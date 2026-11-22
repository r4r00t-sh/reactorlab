#pragma once

#include "engine/reactor_snapshot.hpp"

#include "reactorlab/plant/coupled_plant.hpp"
#include "reactorlab/radiation/edu_dose.hpp"

#include <QObject>
#include <QSet>
#include <QTimer>
#include <QVector>
#include <mutex>

namespace reactorlab::ui {

inline constexpr double kRatedMWth = 3000.0;
inline constexpr int kTrendCap = 2400;
inline constexpr int kMapN = 24;
inline constexpr int kAxialNodes = 20;
inline constexpr int kEventCap = 250;

class EngineWorker final : public QObject {
  Q_OBJECT

public:
  explicit EngineWorker(QObject* parent = nullptr);

public slots:
  void start();
  void play();
  void pause();
  void reset();
  void scram();
  void setRodZ(double z);
  void setRhoExt(double rho);
  void setSpeed(double speed);
  void setDt(double dt_s);
  void setShieldCm(double cm);
  void setScenario(const QString& id);
  void acknowledgeAlarms();
  void exportCsv(const QString& path);

signals:
  void snapshotReady(reactorlab::ui::ReactorSnapshot snap);
  void trendsReady(QVector<double> t,
                   QVector<double> P,
                   QVector<double> rho_pcm,
                   QVector<double> Tf,
                   QVector<double> Tc,
                   QVector<double> X);
  void exportFinished(bool ok, QString message);

private slots:
  void onTick();

private:
  void ensureFormFactorLocked();
  void fillMapsLocked(ReactorSnapshot& s) const;
  void fillRadiationLocked(ReactorSnapshot& s) const;
  void fillAlarmsAndLogsLocked(ReactorSnapshot& s);
  void applyScenarioDriversLocked();
  void pushEventLocked(const QString& msg);
  void evaluateAlarmsLocked(double period_s);
  ReactorSnapshot buildSnapshotLocked();
  void pushTrendLocked(const ReactorSnapshot& s);
  void emitTrendsLocked();

  mutable std::mutex mu_;
  plant::PlantParams params_{};
  plant::PlantState state_{};
  double rod_z_{0.0};
  double rho_ext_{0.0};
  double speed_{1.0};
  double dt_{0.25};
  bool running_{false};
  bool scrammed_{false};
  QString scenario_id_{QStringLiteral("manual")};
  QString scenario_{QStringLiteral("plant · manual")};

  QVector<double> form_;
  int map_nx_{0};
  int map_ny_{0};
  bool form_ready_{false};

  radiation::Inventory rad_inv_{};
  radiation::DoseParams rad_params_{};

  QSet<QString> alarm_active_;
  QSet<QString> alarm_unacked_;
  QVector<QString> event_log_;
  QString last_export_;

  TrendBuffers trends_;
  QTimer* timer_{nullptr};
};

}  // namespace reactorlab::ui
