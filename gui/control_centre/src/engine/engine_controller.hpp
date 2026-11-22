#pragma once

#include "engine/reactor_snapshot.hpp"

#include <QObject>
#include <QThread>
#include <QVariantList>
#include <QVector>

namespace reactorlab::ui {

class EngineWorker;

/// QML-facing facade. Lives on the UI thread; worker runs physics off-thread.
class EngineController final : public QObject {
  Q_OBJECT

  Q_PROPERTY(double t READ t NOTIFY snapshotChanged)
  Q_PROPERTY(double simTime READ t NOTIFY snapshotChanged)
  Q_PROPERTY(double P READ P NOTIFY snapshotChanged)
  Q_PROPERTY(double powerMW READ powerMW NOTIFY snapshotChanged)
  Q_PROPERTY(double powerPct READ powerPct NOTIFY snapshotChanged)
  Q_PROPERTY(double rhoPcm READ rhoPcm NOTIFY snapshotChanged)
  Q_PROPERTY(double periodS READ periodS NOTIFY snapshotChanged)
  Q_PROPERTY(double keff READ keff NOTIFY snapshotChanged)
  Q_PROPERTY(double Tf READ Tf NOTIFY snapshotChanged)
  Q_PROPERTY(double Tc READ Tc NOTIFY snapshotChanged)
  Q_PROPERTY(double tfuel READ Tf NOTIFY snapshotChanged)
  Q_PROPERTY(double tcool READ Tc NOTIFY snapshotChanged)
  Q_PROPERTY(double mdot READ mdot NOTIFY snapshotChanged)
  Q_PROPERTY(double X READ X NOTIFY snapshotChanged)
  Q_PROPERTY(double xenonWorthPcm READ xenonWorthPcm NOTIFY snapshotChanged)
  Q_PROPERTY(double rodZ READ rodZ WRITE setRodZ NOTIFY snapshotChanged)
  Q_PROPERTY(double rhoExt READ rhoExt WRITE setRhoExt NOTIFY snapshotChanged)
  Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY snapshotChanged)
  Q_PROPERTY(bool running READ running NOTIFY snapshotChanged)
  Q_PROPERTY(bool masterAlarm READ masterAlarm NOTIFY snapshotChanged)
  Q_PROPERTY(QString state READ state NOTIFY snapshotChanged)
  Q_PROPERTY(QString simState READ state NOTIFY snapshotChanged)
  Q_PROPERTY(QString scenarioName READ scenarioName NOTIFY snapshotChanged)
  Q_PROPERTY(QString reactorName READ reactorName CONSTANT)

  Q_PROPERTY(QVariantList trendT READ trendT NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList trendP READ trendP NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList trendRho READ trendRho NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList trendTf READ trendTf NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList trendTc READ trendTc NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList trendX READ trendX NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList sparkPower READ sparkPower NOTIFY trendsChanged)
  Q_PROPERTY(QVariantList sparkRho READ sparkRho NOTIFY trendsChanged)

  Q_PROPERTY(int mapNx READ mapNx NOTIFY snapshotChanged)
  Q_PROPERTY(int mapNy READ mapNy NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList mapPower READ mapPower NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList mapFlux READ mapFlux NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList mapTfuel READ mapTfuel NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList mapTcoolAxial READ mapTcoolAxial NOTIFY snapshotChanged)
  Q_PROPERTY(QString mapNote READ mapNote NOTIFY snapshotChanged)

  Q_PROPERTY(QVariantList radNames READ radNames NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList radKinds READ radKinds NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList radActivityBq READ radActivityBq NOTIFY snapshotChanged)
  Q_PROPERTY(double radAlphaBq READ radAlphaBq NOTIFY snapshotChanged)
  Q_PROPERTY(double radBetaBq READ radBetaBq NOTIFY snapshotChanged)
  Q_PROPERTY(double radGammaBq READ radGammaBq NOTIFY snapshotChanged)
  Q_PROPERTY(double shieldCm READ shieldCm WRITE setShieldCm NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList doseR READ doseR NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList doseGamma READ doseGamma NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList doseNeutron READ doseNeutron NOTIFY snapshotChanged)
  Q_PROPERTY(QString radNote READ radNote NOTIFY snapshotChanged)

  Q_PROPERTY(QVariantList alarmIds READ alarmIds NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList alarmMsgs READ alarmMsgs NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList alarmSeverities READ alarmSeverities NOTIFY snapshotChanged)
  Q_PROPERTY(int alarmUnacked READ alarmUnacked NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList eventLog READ eventLog NOTIFY snapshotChanged)
  Q_PROPERTY(double mdotProxy READ mdotProxy NOTIFY snapshotChanged)
  Q_PROPERTY(QString lastExport READ lastExport NOTIFY snapshotChanged)
  Q_PROPERTY(QVariantList scenarioIds READ scenarioIds CONSTANT)
  Q_PROPERTY(QVariantList scenarioLabels READ scenarioLabels CONSTANT)
  Q_PROPERTY(QString exportStatus READ exportStatus NOTIFY exportStatusChanged)

  Q_PROPERTY(double betaEff READ betaEff NOTIFY snapshotChanged)
  Q_PROPERTY(double rhoDollars READ rhoDollars NOTIFY snapshotChanged)
  Q_PROPERTY(double nPa233 READ nPa233 NOTIFY snapshotChanged)
  Q_PROPERTY(double nU233 READ nU233 NOTIFY snapshotChanged)
  Q_PROPERTY(bool nuclearDataPlaceholders READ nuclearDataPlaceholders NOTIFY snapshotChanged)
  Q_PROPERTY(QString placeholderNote READ placeholderNote NOTIFY snapshotChanged)
  Q_PROPERTY(bool fuelCycleCoupling READ fuelCycleCoupling WRITE setFuelCycleCoupling NOTIFY
                 snapshotChanged)
  Q_PROPERTY(double rhoInventory READ rhoInventory NOTIFY snapshotChanged)
  Q_PROPERTY(QString timeMode READ timeMode WRITE setTimeMode NOTIFY snapshotChanged)

public:
  explicit EngineController(QObject* parent = nullptr);
  ~EngineController() override;

  [[nodiscard]] double t() const { return snap_.t; }
  [[nodiscard]] double P() const { return snap_.P; }
  [[nodiscard]] double powerMW() const { return snap_.power_MW; }
  [[nodiscard]] double powerPct() const { return snap_.P * 100.0; }
  [[nodiscard]] double rhoPcm() const { return snap_.rho_pcm; }
  [[nodiscard]] double periodS() const { return snap_.period_s; }
  [[nodiscard]] double keff() const { return 1.0 + snap_.rho; }  // educational k≈1+ρ
  [[nodiscard]] double Tf() const { return snap_.Tf; }
  [[nodiscard]] double Tc() const { return snap_.Tc; }
  [[nodiscard]] double X() const { return snap_.X; }
  [[nodiscard]] double xenonWorthPcm() const { return snap_.xenon_worth_pcm; }
  [[nodiscard]] double rodZ() const { return snap_.rod_z; }
  [[nodiscard]] double rhoExt() const { return snap_.rho_ext; }
  [[nodiscard]] double speed() const { return snap_.speed; }
  [[nodiscard]] bool running() const { return snap_.running; }
  [[nodiscard]] bool masterAlarm() const { return snap_.master_alarm; }
  [[nodiscard]] QString state() const { return snap_.state; }
  [[nodiscard]] QString scenarioName() const { return snap_.scenario; }
  [[nodiscard]] QString reactorName() const {
    return QStringLiteral("ReactorLab · EDU-AHWR (representative)");
  }

  [[nodiscard]] QVariantList trendT() const { return toVariant(trend_t_); }
  [[nodiscard]] QVariantList trendP() const { return toVariant(trend_P_); }
  [[nodiscard]] QVariantList trendRho() const { return toVariant(trend_rho_); }
  [[nodiscard]] QVariantList trendTf() const { return toVariant(trend_Tf_); }
  [[nodiscard]] QVariantList trendTc() const { return toVariant(trend_Tc_); }
  [[nodiscard]] QVariantList trendX() const { return toVariant(trend_X_); }
  [[nodiscard]] QVariantList sparkPower() const { return spark(trend_P_); }
  [[nodiscard]] QVariantList sparkRho() const { return spark(trend_rho_); }

  [[nodiscard]] int mapNx() const { return snap_.map_nx; }
  [[nodiscard]] int mapNy() const { return snap_.map_ny; }
  [[nodiscard]] QVariantList mapPower() const { return toVariant(snap_.power_rel); }
  [[nodiscard]] QVariantList mapFlux() const { return toVariant(snap_.flux_rel); }
  [[nodiscard]] QVariantList mapTfuel() const { return toVariant(snap_.tfuel_map); }
  [[nodiscard]] QVariantList mapTcoolAxial() const { return toVariant(snap_.tcool_axial); }
  [[nodiscard]] QString mapNote() const { return snap_.map_note; }

  [[nodiscard]] QVariantList radNames() const { return toVariantStr(snap_.rad_names); }
  [[nodiscard]] QVariantList radKinds() const { return toVariantStr(snap_.rad_kinds); }
  [[nodiscard]] QVariantList radActivityBq() const { return toVariant(snap_.rad_activity_Bq); }
  [[nodiscard]] double radAlphaBq() const { return snap_.rad_alpha_Bq; }
  [[nodiscard]] double radBetaBq() const { return snap_.rad_beta_Bq; }
  [[nodiscard]] double radGammaBq() const { return snap_.rad_gamma_Bq; }
  [[nodiscard]] double shieldCm() const { return snap_.shield_cm; }
  [[nodiscard]] QVariantList doseR() const { return toVariant(snap_.dose_r_m); }
  [[nodiscard]] QVariantList doseGamma() const { return toVariant(snap_.dose_gamma_uSvh); }
  [[nodiscard]] QVariantList doseNeutron() const { return toVariant(snap_.dose_neutron_uSvh); }
  [[nodiscard]] QString radNote() const { return snap_.rad_note; }

  [[nodiscard]] QVariantList alarmIds() const { return toVariantStr(snap_.alarm_ids); }
  [[nodiscard]] QVariantList alarmMsgs() const { return toVariantStr(snap_.alarm_msgs); }
  [[nodiscard]] QVariantList alarmSeverities() const {
    return toVariantStr(snap_.alarm_severities);
  }
  [[nodiscard]] int alarmUnacked() const { return snap_.alarm_unacked; }
  [[nodiscard]] QVariantList eventLog() const { return toVariantStr(snap_.event_log); }
  [[nodiscard]] double mdotProxy() const { return snap_.mdot_proxy; }
  [[nodiscard]] QString lastExport() const { return snap_.last_export; }
  [[nodiscard]] QVariantList scenarioIds() const;
  [[nodiscard]] QVariantList scenarioLabels() const;
  [[nodiscard]] QString exportStatus() const { return export_status_; }

  [[nodiscard]] double betaEff() const { return snap_.beta_eff; }
  [[nodiscard]] double rhoDollars() const { return snap_.rho_dollars; }
  [[nodiscard]] double nPa233() const { return snap_.N_Pa233; }
  [[nodiscard]] double nU233() const { return snap_.N_U233; }
  [[nodiscard]] bool nuclearDataPlaceholders() const { return snap_.nuclear_data_placeholders; }
  [[nodiscard]] QString placeholderNote() const { return snap_.placeholder_note; }
  [[nodiscard]] bool fuelCycleCoupling() const { return snap_.fuel_cycle_coupling; }
  [[nodiscard]] double rhoInventory() const { return snap_.rho_inventory; }
  [[nodiscard]] QString timeMode() const { return snap_.time_mode; }

  /// Prefer mdotProxy for mimic UI (plant has no true ṁ).
  [[nodiscard]] double mdot() const { return snap_.mdot_proxy; }

public slots:
  void play();
  void pause();
  void reset();
  void scram();
  void setRodZ(double z);
  void setRhoExt(double rho);
  void setSpeed(double speed);
  void setShieldCm(double cm);
  void setScenario(const QString& id);
  void setFuelCycleCoupling(bool enabled);
  void setTimeMode(const QString& mode);
  void acknowledgeAlarms();
  void exportCsv(const QString& path);

signals:
  void snapshotChanged();
  void trendsChanged();
  void exportStatusChanged();

private slots:
  void onSnapshot(reactorlab::ui::ReactorSnapshot snap);
  void onTrends(QVector<double> t,
                QVector<double> P,
                QVector<double> rho_pcm,
                QVector<double> Tf,
                QVector<double> Tc,
                QVector<double> X);
  void onExportFinished(bool ok, QString message);

private:
  static QVariantList toVariant(const QVector<double>& v);
  static QVariantList toVariantStr(const QVector<QString>& v);
  static QVariantList spark(const QVector<double>& v);

  QThread worker_thread_;
  EngineWorker* worker_{nullptr};
  ReactorSnapshot snap_{};
  QVector<double> trend_t_;
  QVector<double> trend_P_;
  QVector<double> trend_rho_;
  QVector<double> trend_Tf_;
  QVector<double> trend_Tc_;
  QVector<double> trend_X_;
  QString export_status_;
};

}  // namespace reactorlab::ui
