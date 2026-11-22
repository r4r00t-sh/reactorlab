#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

namespace reactorlab::ui {

/// Thread-safe POD snapshot for the QML layer (SI + display helpers).
struct ReactorSnapshot {
  double t{0.0};             // s
  double P{1.0};             // relative [-]
  double power_MW{3000.0};   // educational MW_th = P * 3000
  double rho{0.0};           // Δk/k
  double rho_pcm{0.0};       // pcm
  double period_s{1.0e6};    // asymptotic period 1/ω
  double Tf{900.0};          // K
  double Tc{580.0};          // K
  double X{0.0};             // 1/cm^3
  double xenon_worth_pcm{0.0};
  double rod_z{0.0};         // [0,1] insertion
  double rho_ext{0.0};
  double speed{1.0};
  bool running{false};
  bool master_alarm{false};
  QString state{"Paused"};  // Running | Paused | Scram
  QString scenario{"plant · manual"};

  // UI-4 spatial maps (TWIGL form × plant scalars — educational)
  int map_nx{0};
  int map_ny{0};
  QVector<double> power_rel;     // FF × P
  QVector<double> flux_rel;      // FF × P (fluence proxy)
  QVector<double> tfuel_map;     // K
  QVector<double> tcool_axial;   // K, inlet→outlet
  QString map_note{QStringLiteral("TWIGL φ₂ form × plant P/T — educational")};

  // UI-6 educational radiation
  QVector<QString> rad_names;
  QVector<QString> rad_kinds;
  QVector<double> rad_activity_Bq;
  double rad_alpha_Bq{0.0};
  double rad_beta_Bq{0.0};
  double rad_gamma_Bq{0.0};
  double shield_cm{50.0};
  QVector<double> dose_r_m;
  QVector<double> dose_gamma_uSvh;
  QVector<double> dose_neutron_uSvh;
  QString rad_note{QStringLiteral(
      "Educational estimates — not for safety or dose assessment.")};

  // UI-7 alarms / logs / mimic helpers
  QVector<QString> alarm_ids;
  QVector<QString> alarm_msgs;
  QVector<QString> alarm_severities;  // "warn" | "alarm"
  int alarm_unacked{0};
  QVector<QString> event_log;         // newest last
  double mdot_proxy{2.9e4};           // educational kg/s (plant has no ṁ state)
  QString last_export{};

  // Thorium-chain inventory (+ v1.1 quasi-static coupling fields)
  double N_Th232{0.0};
  double N_Pa233{0.0};
  double N_U233{0.0};
  double N_Pu239{0.0};
  double beta_eff{0.0065};
  double rho_dollars{0.0};
  double rho_inventory{0.0};
  bool fuel_cycle_coupling{false};
  QString time_mode{QStringLiteral("transient")};
  bool nuclear_data_placeholders{false};
  QString placeholder_note{};
};

struct TrendBuffers {
  QVector<double> t;
  QVector<double> P;
  QVector<double> rho_pcm;
  QVector<double> Tf;
  QVector<double> Tc;
  QVector<double> X;
};

}  // namespace reactorlab::ui

Q_DECLARE_METATYPE(reactorlab::ui::ReactorSnapshot)
