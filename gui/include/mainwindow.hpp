#pragma once

#include "reactorlab/multiphysics/coupled.hpp"
#include "reactorlab/plant/coupled_plant.hpp"

#include <QMainWindow>
#include <QVector>
#include <string>

class QCustomPlot;
class QCPColorMap;
class QDoubleSpinBox;
class QSlider;
class QPushButton;
class QCheckBox;
class QLabel;
class QTimer;
class QComboBox;
class QSpinBox;
class QWidget;
class QAction;

class MainWindow final : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);

private slots:
  void onRun();
  void onPause();
  void onReset();
  void onTick();
  void onLogScaleToggled(bool checked);
  void onAbout();
  void onReferences();
  void onRodChanged(int value);
  void onScenarioChanged(int index);
  void onModuleChanged(int index);
  void onSolveTwigl();
  void onTwiglRodToggled(bool checked);
  void onRunMc();
  void onMultiScenarioChanged(int index);
  void onSaveSession();
  void onLoadSession();
  void onExportCsv();
  void onScenarioEditor();

private:
  void appendPoint(const reactorlab::plant::PlantState& s, double rho);
  void clearPlots();
  void updateAlarms(const reactorlab::plant::PlantState& s);
  void updateStatus(const reactorlab::plant::PlantState& s, double rho);
  void showPlantUi(bool on);
  void renderTwiglHeatmap(const std::vector<double>& phi2, int n, double keff);

  void multiReset();
  void multiAppend(const reactorlab::multiphysics::MultiState& s, double rho, double mdot);
  void multiClearPlots();
  void multiUpdateStatus(const reactorlab::multiphysics::MultiState& s, double rho, double mdot);
  void multiUpdateAlarms(const reactorlab::multiphysics::MultiState& s);
  void setupMultiScenario();
  void applyLoadedScenario(const QString& path);
  QString scenariosDir() const;

  reactorlab::plant::PlantParams params_{};
  reactorlab::plant::PlantState state_{};
  double rod_z_{0.0};
  double rho_ext_{0.0};
  bool twigl_rods_{false};

  reactorlab::multiphysics::MultiParams multi_params_{};
  reactorlab::multiphysics::MultiState multi_state_{};
  reactorlab::multiphysics::ScenarioDrivers multi_sc_{};
  std::string multi_scenario_id_{"rod_ejection"};
  QString loaded_scenario_path_;

  // Series buffers for CSV export (plant + multiphysics)
  QVector<double> export_t_;
  QVector<double> export_P_;
  QVector<double> export_Tf_;
  QVector<double> export_Tc_;
  QVector<double> export_extra_;  // Xe or mdot

  QComboBox* module_{nullptr};
  QWidget* plant_panel_{nullptr};
  QWidget* diffusion_panel_{nullptr};
  QWidget* multi_panel_{nullptr};

  QCustomPlot* plot_power_{nullptr};
  QCustomPlot* plot_temp_{nullptr};
  QCustomPlot* plot_xe_{nullptr};
  QCustomPlot* plot_flux_{nullptr};
  QCPColorMap* flux_map_{nullptr};

  QCustomPlot* plot_multi_power_{nullptr};
  QCustomPlot* plot_multi_temp_{nullptr};
  QCustomPlot* plot_multi_mdot_{nullptr};

  QDoubleSpinBox* rho_spin_{nullptr};
  QSlider* rod_slider_{nullptr};
  QLabel* rod_label_{nullptr};
  QComboBox* scenario_{nullptr};
  QComboBox* multi_scenario_{nullptr};
  QPushButton* run_btn_{nullptr};
  QPushButton* pause_btn_{nullptr};
  QPushButton* reset_btn_{nullptr};
  QPushButton* multi_run_btn_{nullptr};
  QPushButton* multi_pause_btn_{nullptr};
  QPushButton* multi_reset_btn_{nullptr};
  QCheckBox* log_check_{nullptr};
  QCheckBox* twigl_rod_check_{nullptr};
  QPushButton* solve_twigl_btn_{nullptr};
  QWidget* mc_panel_{nullptr};
  QCustomPlot* plot_mc_flux_{nullptr};
  QSpinBox* mc_histories_{nullptr};
  QSpinBox* mc_seed_{nullptr};
  QDoubleSpinBox* mc_slab_L_{nullptr};
  QPushButton* mc_run_btn_{nullptr};
  QLabel* mc_result_{nullptr};
  QLabel* status_{nullptr};
  QLabel* alarms_{nullptr};
  QTimer* timer_{nullptr};

  bool running_{false};
  bool multi_running_{false};
  double dt_sim_{1.0};
  double time_accel_{60.0};
  double multi_dt_{0.25};
};
