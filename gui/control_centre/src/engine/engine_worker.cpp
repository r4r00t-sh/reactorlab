#include "engine/engine_worker.hpp"

#include "reactorlab/diffusion/two_group.hpp"
#include "reactorlab/io/csv_export.hpp"
#include "reactorlab/kinetics/point_kinetics.hpp"
#include "reactorlab/radiation/edu_dose.hpp"

#include <QDateTime>

#include <algorithm>
#include <cmath>
#include <vector>

namespace reactorlab::ui {
namespace {

double xenon_worth_pcm(const plant::PlantState& s, const plant::PlantParams& p) {
  const double rho = -p.sigma_a_Xe * s.X / (p.nu * p.sigma_f0);
  return rho * 1.0e5;
}

double period_from_rho(double rho, const kinetics::DelayedGroupData& data) {
  try {
    const double omega = kinetics::solve_inhour_omega(rho, data);
    if (std::abs(omega) < 1.0e-12) {
      return 1.0e6;
    }
    return 1.0 / omega;
  } catch (...) {
    return 1.0e6;
  }
}

struct AlarmDef {
  const char* id;
  const char* msg;
  const char* severity;  // warn | alarm
};

}  // namespace

EngineWorker::EngineWorker(QObject* parent) : QObject(parent) {
  state_ = plant::make_equilibrium_state(1.0, params_);
  rad_inv_ = radiation::make_equilibrium_inventory(1.0);
}

void EngineWorker::ensureFormFactorLocked() {
  if (form_ready_) {
    return;
  }
  try {
    const auto res = diffusion::solve_twigl(kMapN, 1.0e-6);
    map_nx_ = res.nx;
    map_ny_ = res.ny;
    form_.resize(static_cast<int>(res.phi2.size()));
    double mx = 0.0;
    for (double v : res.phi2) {
      mx = std::max(mx, v);
    }
    if (mx <= 0.0) {
      mx = 1.0;
    }
    for (int i = 0; i < form_.size(); ++i) {
      form_[i] = res.phi2[static_cast<std::size_t>(i)] / mx;
    }
    form_ready_ = true;
  } catch (...) {
    map_nx_ = kMapN;
    map_ny_ = kMapN;
    form_.fill(1.0, kMapN * kMapN);
    form_ready_ = true;
  }
}

void EngineWorker::fillMapsLocked(ReactorSnapshot& s) const {
  s.map_nx = map_nx_;
  s.map_ny = map_ny_;
  s.map_note = QStringLiteral("TWIGL φ₂ form × plant P/T — educational");
  const auto n = form_.size();
  s.power_rel.resize(n);
  s.flux_rel.resize(n);
  s.tfuel_map.resize(n);
  for (qsizetype i = 0; i < n; ++i) {
    const double ff = form_[i];
    s.power_rel[i] = ff * state_.P;
    s.flux_rel[i] = ff * state_.P;
    s.tfuel_map[i] = state_.Tc + (state_.Tf - state_.Tc) * ff;
  }

  s.tcool_axial.resize(kAxialNodes);
  const double dT = std::max(8.0, 0.12 * (state_.Tf - state_.Tc));
  for (int k = 0; k < kAxialNodes; ++k) {
    const double z = (k + 0.5) / static_cast<double>(kAxialNodes);
    const double shape = std::sin(3.141592653589793 * z);
    s.tcool_axial[k] = state_.Tc - 0.35 * dT + dT * shape * std::max(0.2, state_.P);
  }
}

void EngineWorker::fillRadiationLocked(ReactorSnapshot& s) const {
  const auto& groups = radiation::default_groups();
  s.rad_names.resize(static_cast<int>(radiation::kNumGroups));
  s.rad_kinds.resize(static_cast<int>(radiation::kNumGroups));
  s.rad_activity_Bq.resize(static_cast<int>(radiation::kNumGroups));
  for (std::size_t i = 0; i < radiation::kNumGroups; ++i) {
    s.rad_names[static_cast<int>(i)] = QString::fromUtf8(groups[i].name);
    s.rad_kinds[static_cast<int>(i)] =
        QString::fromStdString(radiation::kind_name(groups[i].kind));
    s.rad_activity_Bq[static_cast<int>(i)] = rad_inv_.activity_Bq[i];
  }
  s.rad_alpha_Bq = radiation::activity_by_kind(rad_inv_, radiation::GroupKind::Alpha);
  s.rad_beta_Bq = radiation::activity_by_kind(rad_inv_, radiation::GroupKind::Beta);
  s.rad_gamma_Bq = radiation::activity_by_kind(rad_inv_, radiation::GroupKind::Gamma);
  s.shield_cm = rad_params_.shield_cm;

  static const std::vector<double> rs{1.0, 2.0, 5.0, 10.0, 20.0, 50.0};
  const auto curve = radiation::dose_curve(rad_inv_, state_.P, rs, rad_params_);
  s.dose_r_m.resize(static_cast<int>(curve.size()));
  s.dose_gamma_uSvh.resize(static_cast<int>(curve.size()));
  s.dose_neutron_uSvh.resize(static_cast<int>(curve.size()));
  for (int i = 0; i < static_cast<int>(curve.size()); ++i) {
    s.dose_r_m[i] = curve[static_cast<std::size_t>(i)].r_m;
    s.dose_gamma_uSvh[i] = curve[static_cast<std::size_t>(i)].H_gamma_uSvh;
    s.dose_neutron_uSvh[i] = curve[static_cast<std::size_t>(i)].H_neutron_uSvh;
  }
  s.rad_note = QStringLiteral("Educational estimates — not for safety or dose assessment.");
}

void EngineWorker::pushEventLocked(const QString& msg) {
  const QString line =
      QStringLiteral("t=%1s · %2").arg(state_.t, 0, 'f', 1).arg(msg);
  event_log_.push_back(line);
  if (event_log_.size() > kEventCap) {
    event_log_.remove(0, event_log_.size() - kEventCap);
  }
}

void EngineWorker::evaluateAlarmsLocked(double period_s) {
  static const AlarmDef defs[] = {
      {"HIGH_POWER", "High thermal power (P > 120%)", "alarm"},
      {"HIGH_FUEL_T", "High fuel temperature (Tf > 1200 K)", "alarm"},
      {"HIGH_COOLANT_T", "High coolant temperature (Tc > 620 K)", "warn"},
      {"SHORT_PERIOD", "Short reactor period (|T| < 10 s)", "warn"},
      {"SCRAM", "Reactor scram — rods fully inserted", "alarm"},
  };

  QSet<QString> now;
  auto consider = [&](const char* id, bool hit) {
    if (hit) {
      now.insert(QString::fromUtf8(id));
    }
  };
  consider("HIGH_POWER", state_.P > 1.2);
  consider("HIGH_FUEL_T", state_.Tf > 1200.0);
  consider("HIGH_COOLANT_T", state_.Tc > 620.0);
  consider("SHORT_PERIOD", std::abs(period_s) < 10.0 && period_s < 1.0e5);
  consider("SCRAM", scrammed_);

  for (const QString& id : now) {
    if (!alarm_active_.contains(id)) {
      alarm_unacked_.insert(id);
      QString msg = id;
      for (const auto& d : defs) {
        if (id == QLatin1String(d.id)) {
          msg = QString::fromUtf8(d.msg);
          break;
        }
      }
      pushEventLocked(QStringLiteral("ALARM · %1").arg(msg));
    }
  }
  // Drop unacked for cleared conditions only when acknowledging; keep active set current.
  alarm_active_ = now;
}

void EngineWorker::fillAlarmsAndLogsLocked(ReactorSnapshot& s) {
  static const AlarmDef defs[] = {
      {"HIGH_POWER", "High thermal power (P > 120%)", "alarm"},
      {"HIGH_FUEL_T", "High fuel temperature (Tf > 1200 K)", "alarm"},
      {"HIGH_COOLANT_T", "High coolant temperature (Tc > 620 K)", "warn"},
      {"SHORT_PERIOD", "Short reactor period (|T| < 10 s)", "warn"},
      {"SCRAM", "Reactor scram — rods fully inserted", "alarm"},
  };

  s.alarm_ids.clear();
  s.alarm_msgs.clear();
  s.alarm_severities.clear();
  for (const auto& d : defs) {
    const QString id = QString::fromUtf8(d.id);
    if (!alarm_active_.contains(id)) {
      continue;
    }
    s.alarm_ids.push_back(id);
    s.alarm_msgs.push_back(QString::fromUtf8(d.msg));
    s.alarm_severities.push_back(QString::fromUtf8(d.severity));
  }
  s.alarm_unacked = static_cast<int>(alarm_unacked_.size());
  s.master_alarm = !alarm_active_.isEmpty() || !alarm_unacked_.isEmpty();
  s.event_log = event_log_;
  s.mdot_proxy = 2.9e4 * std::clamp(state_.P, 0.25, 1.25);
  s.last_export = last_export_;
}

void EngineWorker::applyScenarioDriversLocked() {
  if (scenario_id_ == QLatin1String("scram_xe") && state_.t >= 1.0) {
    if (!scrammed_) {
      pushEventLocked(QStringLiteral("Scenario scram_xe · rods in"));
    }
    rod_z_ = 1.0;
    scrammed_ = true;
  } else if (scenario_id_ == QLatin1String("rod_step") && state_.t >= 60.0) {
    if (rod_z_ < 0.14) {
      pushEventLocked(QStringLiteral("Scenario rod_step · z→0.15"));
    }
    if (!scrammed_) {
      rod_z_ = 0.15;
    }
  } else if (scenario_id_ == QLatin1String("doppler")) {
    rho_ext_ = 0.0015;
  }
}

void EngineWorker::start() {
  if (timer_ == nullptr) {
    timer_ = new QTimer(this);
    timer_->setInterval(50);
    connect(timer_, &QTimer::timeout, this, &EngineWorker::onTick);
  }
  {
    std::lock_guard lock(mu_);
    state_ = plant::make_equilibrium_state(1.0, params_);
    rod_z_ = 0.0;
    rho_ext_ = 0.0;
    scrammed_ = false;
    running_ = false;
    trends_ = {};
    rad_inv_ = radiation::make_equilibrium_inventory(1.0);
    alarm_active_.clear();
    alarm_unacked_.clear();
    event_log_.clear();
    pushEventLocked(QStringLiteral("Engine started"));
    ensureFormFactorLocked();
  }
  timer_->start();
  onTick();
}

void EngineWorker::play() {
  std::lock_guard lock(mu_);
  if (!scrammed_) {
    running_ = true;
    pushEventLocked(QStringLiteral("Play"));
  }
}

void EngineWorker::pause() {
  std::lock_guard lock(mu_);
  running_ = false;
  pushEventLocked(QStringLiteral("Pause"));
}

void EngineWorker::reset() {
  std::lock_guard lock(mu_);
  state_ = plant::make_equilibrium_state(1.0, params_);
  rod_z_ = 0.0;
  rho_ext_ = (scenario_id_ == QLatin1String("doppler")) ? 0.0015 : 0.0;
  scrammed_ = false;
  running_ = false;
  trends_ = {};
  rad_inv_ = radiation::make_equilibrium_inventory(1.0);
  alarm_active_.clear();
  alarm_unacked_.clear();
  pushEventLocked(QStringLiteral("Reset"));
}

void EngineWorker::scram() {
  std::lock_guard lock(mu_);
  rod_z_ = 1.0;
  scrammed_ = true;
  running_ = false;
  pushEventLocked(QStringLiteral("Operator SCRAM"));
}

void EngineWorker::setRodZ(double z) {
  std::lock_guard lock(mu_);
  if (scrammed_) {
    return;
  }
  rod_z_ = std::clamp(z, 0.0, 1.0);
}

void EngineWorker::setRhoExt(double rho) {
  std::lock_guard lock(mu_);
  rho_ext_ = rho;
}

void EngineWorker::setSpeed(double speed) {
  std::lock_guard lock(mu_);
  speed_ = std::clamp(speed, 0.05, 60.0);
}

void EngineWorker::setDt(double dt_s) {
  std::lock_guard lock(mu_);
  dt_ = std::clamp(dt_s, 0.01, 10.0);
}

void EngineWorker::setShieldCm(double cm) {
  std::lock_guard lock(mu_);
  rad_params_.shield_cm = std::clamp(cm, 0.0, 500.0);
}

void EngineWorker::setScenario(const QString& id) {
  std::lock_guard lock(mu_);
  scenario_id_ = id;
  if (id == QLatin1String("scram_xe")) {
    scenario_ = QStringLiteral("plant · scram_xe");
    // Substepped in onTick; raise Speed in the transport bar for Xe-time.
    dt_ = 1.0;
    speed_ = 5.0;
  } else if (id == QLatin1String("rod_step")) {
    scenario_ = QStringLiteral("plant · rod_step");
    dt_ = 1.0;
    speed_ = 20.0;
  } else if (id == QLatin1String("doppler")) {
    scenario_ = QStringLiteral("plant · doppler");
    dt_ = 0.25;
    rho_ext_ = 0.0015;
  } else {
    scenario_id_ = QStringLiteral("manual");
    scenario_ = QStringLiteral("plant · manual");
    dt_ = 0.25;
  }
  state_ = plant::make_equilibrium_state(1.0, params_);
  rod_z_ = 0.0;
  if (scenario_id_ != QLatin1String("doppler")) {
    rho_ext_ = 0.0;
  }
  scrammed_ = false;
  running_ = false;
  trends_ = {};
  rad_inv_ = radiation::make_equilibrium_inventory(1.0);
  alarm_active_.clear();
  alarm_unacked_.clear();
  pushEventLocked(QStringLiteral("Scenario → %1").arg(scenario_));
}

void EngineWorker::acknowledgeAlarms() {
  std::lock_guard lock(mu_);
  alarm_unacked_.clear();
  pushEventLocked(QStringLiteral("Alarms acknowledged"));
}

void EngineWorker::exportCsv(const QString& path) {
  TrendBuffers trends;
  {
    std::lock_guard lock(mu_);
    trends = trends_;
  }
  bool ok = false;
  QString message;
  try {
    std::vector<io::CsvColumn> cols{
        {"t[s]", {trends.t.begin(), trends.t.end()}},
        {"P[-]", {trends.P.begin(), trends.P.end()}},
        {"rho[pcm]", {trends.rho_pcm.begin(), trends.rho_pcm.end()}},
        {"Tf[K]", {trends.Tf.begin(), trends.Tf.end()}},
        {"Tc[K]", {trends.Tc.begin(), trends.Tc.end()}},
        {"X[1/cm3]", {trends.X.begin(), trends.X.end()}},
    };
    const QString header = QStringLiteral("ReactorLab educational export %1")
                               .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    io::write_csv(path.toStdString(), cols, header.toStdString());
    ok = true;
    message = QStringLiteral("Wrote %1").arg(path);
    std::lock_guard lock(mu_);
    last_export_ = path;
    pushEventLocked(QStringLiteral("CSV export · %1").arg(path));
  } catch (const std::exception& ex) {
    message = QString::fromUtf8(ex.what());
  } catch (...) {
    message = QStringLiteral("CSV export failed");
  }
  emit exportFinished(ok, message);
}

ReactorSnapshot EngineWorker::buildSnapshotLocked() {
  const double rod = rod_z_;
  const double rho = plant::total_reactivity(
      state_.t, state_, params_, kinetics::step_reactivity(rho_ext_),
      [rod](double) { return rod; });

  ReactorSnapshot s;
  s.t = state_.t;
  s.P = state_.P;
  s.power_MW = state_.P * kRatedMWth;
  s.rho = rho;
  s.rho_pcm = rho * 1.0e5;
  s.period_s = period_from_rho(rho, params_.kinetics);
  s.Tf = state_.Tf;
  s.Tc = state_.Tc;
  s.X = state_.X;
  s.xenon_worth_pcm = xenon_worth_pcm(state_, params_);
  s.rod_z = rod_z_;
  s.rho_ext = rho_ext_;
  s.speed = speed_;
  s.running = running_;
  if (scrammed_) {
    s.state = QStringLiteral("Scram");
  } else if (running_) {
    s.state = QStringLiteral("Running");
  } else {
    s.state = QStringLiteral("Paused");
  }
  s.scenario = scenario_;
  fillMapsLocked(s);
  fillRadiationLocked(s);
  evaluateAlarmsLocked(s.period_s);
  fillAlarmsAndLogsLocked(s);
  return s;
}

void EngineWorker::pushTrendLocked(const ReactorSnapshot& s) {
  auto push = [](QVector<double>& v, double x) {
    v.push_back(x);
    if (v.size() > kTrendCap) {
      v.remove(0, v.size() - kTrendCap);
    }
  };
  push(trends_.t, s.t);
  push(trends_.P, s.P);
  push(trends_.rho_pcm, s.rho_pcm);
  push(trends_.Tf, s.Tf);
  push(trends_.Tc, s.Tc);
  push(trends_.X, s.X);
}

void EngineWorker::emitTrendsLocked() {
  emit trendsReady(trends_.t, trends_.P, trends_.rho_pcm, trends_.Tf, trends_.Tc, trends_.X);
}

void EngineWorker::onTick() {
  ReactorSnapshot snap;
  bool emit_trends = false;
  {
    std::lock_guard lock(mu_);
    if (running_) {
      applyScenarioDriversLocked();
      const double sim_dt = dt_ * speed_;
      try {
        // Substep large educational leaps so CVODE stays within mxstep.
        double left = sim_dt;
        while (left > 1.0e-12) {
          const double step = std::min(left, 2.0);
          const double next_t = state_.t + step;
          const double z = rod_z_;
          state_ = plant::advance(params_, kinetics::step_reactivity(rho_ext_),
                                  [z](double) { return z; }, state_, next_t);
          radiation::advance_inventory(rad_inv_, state_.P, step);
          left -= step;
          applyScenarioDriversLocked();
        }
      } catch (...) {
        running_ = false;
        pushEventLocked(QStringLiteral("Plant advance failed — paused"));
      }
    }
    snap = buildSnapshotLocked();
    if (running_ || trends_.t.isEmpty()) {
      pushTrendLocked(snap);
      emit_trends = true;
    }
  }
  emit snapshotReady(snap);
  if (emit_trends) {
    std::lock_guard lock(mu_);
    emitTrendsLocked();
  }
}

}  // namespace reactorlab::ui
