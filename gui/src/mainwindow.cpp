#include "mainwindow.hpp"

#include "qcustomplot.h"
#include "reactorlab/diffusion/two_group.hpp"
#include "reactorlab/io/csv_export.hpp"
#include "reactorlab/io/scenario_yaml.hpp"
#include "reactorlab/montecarlo/eigenvalue.hpp"
#include "reactorlab/montecarlo/material.hpp"
#include "reactorlab/version.hpp"

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <vector>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle(QStringLiteral("ReactorLab"));
  resize(1100, 780);

  auto* central = new QWidget(this);
  auto* root = new QVBoxLayout(central);

  module_ = new QComboBox(central);
  module_->addItem(QStringLiteral("Coupled plant (Phase 2)"), QStringLiteral("plant"));
  module_->addItem(QStringLiteral("TWIGL diffusion (Phase 3)"), QStringLiteral("twigl"));
  module_->addItem(QStringLiteral("Monte Carlo (Phase 4)"), QStringLiteral("mc"));
  module_->addItem(QStringLiteral("Multiphysics (Phase 5)"), QStringLiteral("multi"));
  root->addWidget(module_);

  auto* stack = new QStackedWidget(central);

  // ---- Plant panel ----
  plant_panel_ = new QWidget(stack);
  auto* plant_root = new QVBoxLayout(plant_panel_);
  auto* plots = new QHBoxLayout();
  plot_power_ = new QCustomPlot(plant_panel_);
  plot_power_->addGraph();
  plot_power_->xAxis->setLabel(QStringLiteral("t [s]"));
  plot_power_->yAxis->setLabel(QStringLiteral("P [-]"));
  plot_temp_ = new QCustomPlot(plant_panel_);
  plot_temp_->addGraph();
  plot_temp_->addGraph();
  plot_temp_->graph(1)->setPen(QPen(Qt::darkCyan));
  plot_temp_->xAxis->setLabel(QStringLiteral("t [s]"));
  plot_temp_->yAxis->setLabel(QStringLiteral("T [K]"));
  plot_xe_ = new QCustomPlot(plant_panel_);
  plot_xe_->addGraph();
  plot_xe_->xAxis->setLabel(QStringLiteral("t [s]"));
  plot_xe_->yAxis->setLabel(QStringLiteral("Xe [1/cm³]"));
  plots->addWidget(plot_power_);
  plots->addWidget(plot_temp_);
  plots->addWidget(plot_xe_);
  plant_root->addLayout(plots, 1);

  auto* controls = new QHBoxLayout();
  scenario_ = new QComboBox(plant_panel_);
  scenario_->addItem(QStringLiteral("Manual"), QStringLiteral("manual"));
  scenario_->addItem(QStringLiteral("Rod step (power dip)"), QStringLiteral("rod_step"));
  scenario_->addItem(QStringLiteral("Scram → Xe peak"), QStringLiteral("scram_xe"));
  scenario_->addItem(QStringLiteral("ρ step + Doppler"), QStringLiteral("doppler"));
  rho_spin_ = new QDoubleSpinBox(plant_panel_);
  rho_spin_->setDecimals(5);
  rho_spin_->setRange(-0.02, 0.01);
  rho_spin_->setSingleStep(0.0001);
  rho_spin_->setPrefix(QStringLiteral("ρ_ext="));
  rod_slider_ = new QSlider(Qt::Horizontal, plant_panel_);
  rod_slider_->setRange(0, 1000);
  rod_label_ = new QLabel(QStringLiteral("Rod 0%"), plant_panel_);
  run_btn_ = new QPushButton(QStringLiteral("Run"), plant_panel_);
  pause_btn_ = new QPushButton(QStringLiteral("Pause"), plant_panel_);
  reset_btn_ = new QPushButton(QStringLiteral("Reset"), plant_panel_);
  log_check_ = new QCheckBox(QStringLiteral("Log P"), plant_panel_);
  controls->addWidget(scenario_);
  controls->addWidget(rho_spin_);
  controls->addWidget(new QLabel(QStringLiteral("Rods:"), plant_panel_));
  controls->addWidget(rod_slider_, 1);
  controls->addWidget(rod_label_);
  controls->addWidget(run_btn_);
  controls->addWidget(pause_btn_);
  controls->addWidget(reset_btn_);
  controls->addWidget(log_check_);
  plant_root->addLayout(controls);

  // ---- Diffusion panel ----
  diffusion_panel_ = new QWidget(stack);
  auto* diff_root = new QVBoxLayout(diffusion_panel_);
  plot_flux_ = new QCustomPlot(diffusion_panel_);
  flux_map_ = new QCPColorMap(plot_flux_->xAxis, plot_flux_->yAxis);
  auto* color_scale = new QCPColorScale(plot_flux_);
  plot_flux_->plotLayout()->addElement(0, 1, color_scale);
  color_scale->setType(QCPAxis::atRight);
  flux_map_->setColorScale(color_scale);
  flux_map_->setGradient(QCPColorGradient::gpJet);
  plot_flux_->xAxis->setLabel(QStringLiteral("x [cm]"));
  plot_flux_->yAxis->setLabel(QStringLiteral("y [cm]"));
  color_scale->axis()->setLabel(QStringLiteral("φ₂"));
  diff_root->addWidget(plot_flux_, 1);

  auto* dcontrols = new QHBoxLayout();
  twigl_rod_check_ = new QCheckBox(QStringLiteral("Insert rods (↑ Σa₂ in seed)"), diffusion_panel_);
  solve_twigl_btn_ = new QPushButton(QStringLiteral("Solve TWIGL"), diffusion_panel_);
  dcontrols->addWidget(twigl_rod_check_);
  dcontrols->addWidget(solve_twigl_btn_);
  dcontrols->addStretch();
  diff_root->addLayout(dcontrols);

  // ---- Monte Carlo panel ----
  mc_panel_ = new QWidget(stack);
  auto* mc_root = new QVBoxLayout(mc_panel_);
  plot_mc_flux_ = new QCustomPlot(mc_panel_);
  plot_mc_flux_->addGraph();
  plot_mc_flux_->xAxis->setLabel(QStringLiteral("Group"));
  plot_mc_flux_->yAxis->setLabel(QStringLiteral("Track-length flux (rel.)"));
  mc_root->addWidget(plot_mc_flux_, 1);
  auto* mc_controls = new QHBoxLayout();
  mc_histories_ = new QSpinBox(mc_panel_);
  mc_histories_->setRange(100, 200000);
  mc_histories_->setValue(4000);
  mc_histories_->setPrefix(QStringLiteral("N="));
  mc_seed_ = new QSpinBox(mc_panel_);
  mc_seed_->setRange(1, 2000000000);
  mc_seed_->setValue(42);
  mc_seed_->setPrefix(QStringLiteral("seed="));
  mc_slab_L_ = new QDoubleSpinBox(mc_panel_);
  mc_slab_L_->setRange(1.0, 200.0);
  mc_slab_L_->setValue(20.0);
  mc_slab_L_->setPrefix(QStringLiteral("L="));
  mc_slab_L_->setSuffix(QStringLiteral(" cm"));
  mc_run_btn_ = new QPushButton(QStringLiteral("Run slab MC"), mc_panel_);
  mc_result_ = new QLabel(QStringLiteral("k_eff = —"), mc_panel_);
  mc_controls->addWidget(mc_histories_);
  mc_controls->addWidget(mc_seed_);
  mc_controls->addWidget(mc_slab_L_);
  mc_controls->addWidget(mc_run_btn_);
  mc_controls->addWidget(mc_result_, 1);
  mc_root->addLayout(mc_controls);

  // ---- Multiphysics panel ----
  multi_panel_ = new QWidget(stack);
  auto* multi_root = new QVBoxLayout(multi_panel_);
  auto* mplots = new QHBoxLayout();
  plot_multi_power_ = new QCustomPlot(multi_panel_);
  plot_multi_power_->addGraph();
  plot_multi_power_->xAxis->setLabel(QStringLiteral("t [s]"));
  plot_multi_power_->yAxis->setLabel(QStringLiteral("P [-]"));
  plot_multi_temp_ = new QCustomPlot(multi_panel_);
  plot_multi_temp_->addGraph();
  plot_multi_temp_->addGraph();
  plot_multi_temp_->addGraph();
  plot_multi_temp_->graph(1)->setPen(QPen(Qt::darkCyan));
  plot_multi_temp_->graph(2)->setPen(QPen(Qt::darkYellow));
  plot_multi_temp_->xAxis->setLabel(QStringLiteral("t [s]"));
  plot_multi_temp_->yAxis->setLabel(QStringLiteral("T [K]"));
  plot_multi_mdot_ = new QCustomPlot(multi_panel_);
  plot_multi_mdot_->addGraph();
  plot_multi_mdot_->xAxis->setLabel(QStringLiteral("t [s]"));
  plot_multi_mdot_->yAxis->setLabel(QStringLiteral("ṁ [kg/s]"));
  mplots->addWidget(plot_multi_power_);
  mplots->addWidget(plot_multi_temp_);
  mplots->addWidget(plot_multi_mdot_);
  multi_root->addLayout(mplots, 1);

  auto* mcontrols = new QHBoxLayout();
  multi_scenario_ = new QComboBox(multi_panel_);
  multi_scenario_->addItem(QStringLiteral("Rod ejection"), QStringLiteral("rod_ejection"));
  multi_scenario_->addItem(QStringLiteral("Loss of flow"), QStringLiteral("lof"));
  multi_scenario_->addItem(QStringLiteral("Load-follow"), QStringLiteral("load_follow"));
  multi_run_btn_ = new QPushButton(QStringLiteral("Run"), multi_panel_);
  multi_pause_btn_ = new QPushButton(QStringLiteral("Pause"), multi_panel_);
  multi_reset_btn_ = new QPushButton(QStringLiteral("Reset"), multi_panel_);
  mcontrols->addWidget(multi_scenario_);
  mcontrols->addWidget(multi_run_btn_);
  mcontrols->addWidget(multi_pause_btn_);
  mcontrols->addWidget(multi_reset_btn_);
  mcontrols->addStretch();
  multi_root->addLayout(mcontrols);

  stack->addWidget(plant_panel_);
  stack->addWidget(diffusion_panel_);
  stack->addWidget(mc_panel_);
  stack->addWidget(multi_panel_);
  root->addWidget(stack, 1);

  status_ = new QLabel(QStringLiteral("Ready"), central);
  alarms_ = new QLabel(QStringLiteral("Alarms: none"), central);
  alarms_->setStyleSheet(QStringLiteral("font-weight:600;"));
  root->addWidget(status_);
  root->addWidget(alarms_);

  auto* disclaimer = new QLabel(
      QString::fromUtf8(reactorlab::educational_disclaimer().data(),
                        static_cast<int>(reactorlab::educational_disclaimer().size())),
      central);
  disclaimer->setWordWrap(true);
  disclaimer->setStyleSheet(QStringLiteral("color:#555; font-size:11px;"));
  root->addWidget(disclaimer);
  setCentralWidget(central);

  auto* file_menu = menuBar()->addMenu(QStringLiteral("&File"));
  file_menu->addAction(QStringLiteral("Save session…"), this, &MainWindow::onSaveSession);
  file_menu->addAction(QStringLiteral("Load session…"), this, &MainWindow::onLoadSession);
  file_menu->addSeparator();
  file_menu->addAction(QStringLiteral("Export CSV…"), this, &MainWindow::onExportCsv);
  file_menu->addAction(QStringLiteral("Scenarios…"), this, &MainWindow::onScenarioEditor);
  file_menu->addSeparator();
  file_menu->addAction(QStringLiteral("Quit"), this, &QWidget::close);

  auto* help_menu = menuBar()->addMenu(QStringLiteral("&Help"));
  help_menu->addAction(QStringLiteral("User guide…"), this, [this]() {
    QMessageBox::information(
        this, QStringLiteral("User guide"),
        QStringLiteral("See docs/user-guide.md in the source tree, or the installed "
                       "documentation under share/doc/ReactorLab.\n\n"
                       "Educational/research tool — not safety-grade."));
  });
  help_menu->addAction(QStringLiteral("References…"), this, &MainWindow::onReferences);
  help_menu->addAction(QStringLiteral("About ReactorLab"), this, &MainWindow::onAbout);

  timer_ = new QTimer(this);
  timer_->setInterval(50);
  connect(timer_, &QTimer::timeout, this, &MainWindow::onTick);
  connect(run_btn_, &QPushButton::clicked, this, &MainWindow::onRun);
  connect(pause_btn_, &QPushButton::clicked, this, &MainWindow::onPause);
  connect(reset_btn_, &QPushButton::clicked, this, &MainWindow::onReset);
  connect(log_check_, &QCheckBox::toggled, this, &MainWindow::onLogScaleToggled);
  connect(rod_slider_, &QSlider::valueChanged, this, &MainWindow::onRodChanged);
  connect(scenario_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          &MainWindow::onScenarioChanged);
  connect(module_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          &MainWindow::onModuleChanged);
  connect(solve_twigl_btn_, &QPushButton::clicked, this, &MainWindow::onSolveTwigl);
  connect(twigl_rod_check_, &QCheckBox::toggled, this, &MainWindow::onTwiglRodToggled);
  connect(mc_run_btn_, &QPushButton::clicked, this, &MainWindow::onRunMc);
  connect(multi_scenario_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          &MainWindow::onMultiScenarioChanged);
  connect(multi_run_btn_, &QPushButton::clicked, this, [this]() {
    multi_running_ = true;
    running_ = false;
    timer_->start();
  });
  connect(multi_pause_btn_, &QPushButton::clicked, this, [this]() {
    multi_running_ = false;
    if (!running_) {
      timer_->stop();
    }
  });
  connect(multi_reset_btn_, &QPushButton::clicked, this, [this]() {
    multi_running_ = false;
    if (!running_) {
      timer_->stop();
    }
    multiReset();
  });

  onReset();
  onSolveTwigl();
  multiReset();
}

void MainWindow::showPlantUi(bool on) {
  Q_UNUSED(on);
}

void MainWindow::onModuleChanged(int index) {
  auto* stack = qobject_cast<QStackedWidget*>(plant_panel_->parentWidget());
  if (!stack) {
    return;
  }
  stack->setCurrentIndex(index);
  if (index != 0) {
    onPause();
  }
  if (index != 3) {
    multi_running_ = false;
  }
}

void MainWindow::setupMultiScenario() {
  multi_scenario_id_ = multi_scenario_->currentData().toString().toStdString();
  if (multi_scenario_id_ == "rod_ejection") {
    multi_sc_ = reactorlab::multiphysics::scenario_rod_ejection(1.0, 0.08, 0.2, multi_params_.mdot0);
    multi_dt_ = 0.1;
  } else if (multi_scenario_id_ == "lof") {
    multi_sc_ = reactorlab::multiphysics::scenario_loss_of_flow(2.0, 0.4, 5.0, multi_params_.mdot0);
    multi_dt_ = 0.25;
  } else {
    multi_sc_ = reactorlab::multiphysics::scenario_load_follow(100.0, 0.0015, multi_params_.mdot0);
    multi_dt_ = 1.0;
  }
}

void MainWindow::multiReset() {
  multi_params_ = reactorlab::multiphysics::MultiParams{};
  setupMultiScenario();
  multi_state_ = reactorlab::multiphysics::make_equilibrium_state(1.0, multi_params_, multi_sc_.rod_z0);
  multiClearPlots();
  const double rho = reactorlab::multiphysics::total_reactivity(
      multi_state_.t, multi_state_.P, multi_state_.Tf, multi_state_.Tc_bar(), multi_params_,
      multi_sc_.rho_ext, multi_sc_.rod_z);
  const double mdot = multi_sc_.mdot(multi_state_.t);
  multiAppend(multi_state_, rho, mdot);
  multiUpdateStatus(multi_state_, rho, mdot);
  multiUpdateAlarms(multi_state_);
}

void MainWindow::multiClearPlots() {
  export_t_.clear();
  export_P_.clear();
  export_Tf_.clear();
  export_Tc_.clear();
  export_extra_.clear();
  plot_multi_power_->graph(0)->data()->clear();
  plot_multi_temp_->graph(0)->data()->clear();
  plot_multi_temp_->graph(1)->data()->clear();
  plot_multi_temp_->graph(2)->data()->clear();
  plot_multi_mdot_->graph(0)->data()->clear();
  plot_multi_power_->replot();
  plot_multi_temp_->replot();
  plot_multi_mdot_->replot();
}

void MainWindow::multiAppend(const reactorlab::multiphysics::MultiState& s, double /*rho*/,
                             double mdot) {
  const double t = s.t;
  export_t_.push_back(t);
  export_P_.push_back(s.P);
  export_Tf_.push_back(s.Tf);
  export_Tc_.push_back(s.Tc_bar());
  export_extra_.push_back(mdot);
  plot_multi_power_->graph(0)->addData(t, s.P);
  plot_multi_temp_->graph(0)->addData(t, s.Tf);
  plot_multi_temp_->graph(1)->addData(t, s.Tc_bar());
  plot_multi_temp_->graph(2)->addData(t, s.Tout());
  plot_multi_mdot_->graph(0)->addData(t, mdot);
  const double xmax = std::max(20.0, t * 1.05);
  plot_multi_power_->xAxis->setRange(0.0, xmax);
  plot_multi_temp_->xAxis->setRange(0.0, xmax);
  plot_multi_mdot_->xAxis->setRange(0.0, xmax);
  plot_multi_power_->yAxis->setRange(0.0, std::max(2.0, s.P * 1.2));
  plot_multi_temp_->yAxis->setRange(std::min(500.0, s.Tc_bar() - 20.0),
                                    std::max(1100.0, s.Tf + 20.0));
  plot_multi_mdot_->rescaleAxes(true);
  plot_multi_power_->replot();
  plot_multi_temp_->replot();
  plot_multi_mdot_->replot();
}

void MainWindow::multiUpdateStatus(const reactorlab::multiphysics::MultiState& s, double rho,
                                   double mdot) {
  status_->setText(
      QStringLiteral("MULTI t=%1 s  P=%2  Tf=%3 K  Tc̄=%4 K  Tout=%5 K  ṁ=%6  ρ=%7  Picard=%8")
          .arg(s.t, 0, 'f', 2)
          .arg(s.P, 0, 'e', 3)
          .arg(s.Tf, 0, 'f', 1)
          .arg(s.Tc_bar(), 0, 'f', 1)
          .arg(s.Tout(), 0, 'f', 1)
          .arg(mdot, 0, 'f', 0)
          .arg(rho, 0, 'e', 3)
          .arg(s.picard_iters));
}

void MainWindow::multiUpdateAlarms(const reactorlab::multiphysics::MultiState& s) {
  QStringList hits;
  if (s.P > 1.2) hits << QStringLiteral("HIGH POWER");
  if (s.Tf > 1200.0) hits << QStringLiteral("HIGH FUEL T");
  if (s.Tout() > 620.0) hits << QStringLiteral("HIGH OUTLET T");
  if (hits.isEmpty()) {
    alarms_->setText(QStringLiteral("Alarms: none"));
    alarms_->setStyleSheet(QStringLiteral("font-weight:600; color:#2a2;"));
  } else {
    alarms_->setText(QStringLiteral("Alarms: ") + hits.join(QStringLiteral(", ")));
    alarms_->setStyleSheet(QStringLiteral("font-weight:600; color:#c22;"));
  }
}

void MainWindow::onMultiScenarioChanged(int) {
  multi_running_ = false;
  if (!running_) {
    timer_->stop();
  }
  multiReset();
}

void MainWindow::renderTwiglHeatmap(const std::vector<double>& phi2, int n, double keff) {
  flux_map_->data()->setSize(n, n);
  flux_map_->data()->setRange(QCPRange(0.0, 80.0), QCPRange(0.0, 80.0));
  double vmax = 0.0;
  for (int j = 0; j < n; ++j) {
    for (int i = 0; i < n; ++i) {
      const double v = phi2[static_cast<std::size_t>(j * n + i)];
      flux_map_->data()->setCell(i, j, v);
      vmax = std::max(vmax, v);
    }
  }
  flux_map_->setDataRange(QCPRange(0.0, std::max(vmax, 1e-30)));
  plot_flux_->rescaleAxes();
  plot_flux_->replot();
  status_->setText(QStringLiteral("TWIGL  %1×%2  k_eff=%3  rods=%4")
                       .arg(n)
                       .arg(n)
                       .arg(keff, 0, 'f', 5)
                       .arg(twigl_rods_ ? QStringLiteral("IN") : QStringLiteral("OUT")));
  alarms_->setText(twigl_rods_ ? QStringLiteral("Alarms: RODS INSERTED (seed Σa↑)")
                               : QStringLiteral("Alarms: none"));
  alarms_->setStyleSheet(twigl_rods_ ? QStringLiteral("font-weight:600; color:#c22;")
                                     : QStringLiteral("font-weight:600; color:#2a2;"));
}

void MainWindow::onSolveTwigl() {
  constexpr int n = 40;
  try {
    reactorlab::diffusion::TwoGroupXS seed;
    seed.D = {1.4, 0.4};
    seed.Sigma_a = {0.01, 0.15};
    seed.nuSigma_f = {0.007, 0.2};
    seed.Sigma_s12 = 0.01;
    if (twigl_rods_) {
      seed.Sigma_a[1] += 0.0034;
    }
    reactorlab::diffusion::TwoGroupXS blanket;
    blanket.D = {1.3, 0.5};
    blanket.Sigma_a = {0.008, 0.05};
    blanket.nuSigma_f = {0.003, 0.06};
    blanket.Sigma_s12 = 0.01;
    std::vector<reactorlab::diffusion::TwoGroupXS> mats(4);
    mats[1] = seed;
    mats[2] = seed;
    mats[3] = blanket;
    const auto map = reactorlab::diffusion::twigl_material_map(n, n);
    const double h = 80.0 / static_cast<double>(n);
    const auto res = reactorlab::diffusion::solve_2d(
        map, n, n, mats, h, h, reactorlab::diffusion::BoundaryCondition::MixedReflectiveZero);
    renderTwiglHeatmap(res.phi2, n, res.keff);
  } catch (const std::exception& ex) {
    QMessageBox::warning(this, QStringLiteral("Diffusion error"), QString::fromUtf8(ex.what()));
  }
}

void MainWindow::onTwiglRodToggled(bool checked) {
  twigl_rods_ = checked;
  onSolveTwigl();
}

void MainWindow::clearPlots() {
  export_t_.clear();
  export_P_.clear();
  export_Tf_.clear();
  export_Tc_.clear();
  export_extra_.clear();
  plot_power_->graph(0)->data()->clear();
  plot_temp_->graph(0)->data()->clear();
  plot_temp_->graph(1)->data()->clear();
  plot_xe_->graph(0)->data()->clear();
  plot_power_->replot();
  plot_temp_->replot();
  plot_xe_->replot();
}

void MainWindow::appendPoint(const reactorlab::plant::PlantState& s, double /*rho*/) {
  const double t = s.t;
  export_t_.push_back(t);
  export_P_.push_back(s.P);
  export_Tf_.push_back(s.Tf);
  export_Tc_.push_back(s.Tc);
  export_extra_.push_back(s.X);
  plot_power_->graph(0)->addData(t, s.P);
  plot_temp_->graph(0)->addData(t, s.Tf);
  plot_temp_->graph(1)->addData(t, s.Tc);
  plot_xe_->graph(0)->addData(t, s.X);
  const double xmax = std::max(20.0, t * 1.05);
  plot_power_->xAxis->setRange(0.0, xmax);
  plot_temp_->xAxis->setRange(0.0, xmax);
  plot_xe_->xAxis->setRange(0.0, xmax);
  if (!log_check_->isChecked()) {
    plot_power_->yAxis->setRange(0.0, std::max(2.0, s.P * 1.2));
  }
  plot_temp_->yAxis->setRange(std::min(500.0, s.Tc - 20.0), std::max(1100.0, s.Tf + 20.0));
  plot_xe_->rescaleAxes(true);
  plot_power_->replot();
  plot_temp_->replot();
  plot_xe_->replot();
}

void MainWindow::updateStatus(const reactorlab::plant::PlantState& s, double rho) {
  status_->setText(QStringLiteral("t=%1 s (%2 h)  P=%3  Tf=%4 K  Tc=%5 K  ρ=%6  rod=%7%")
                       .arg(s.t, 0, 'f', 1)
                       .arg(s.t / 3600.0, 0, 'f', 2)
                       .arg(s.P, 0, 'e', 3)
                       .arg(s.Tf, 0, 'f', 1)
                       .arg(s.Tc, 0, 'f', 1)
                       .arg(rho, 0, 'e', 3)
                       .arg(rod_z_ * 100.0, 0, 'f', 1));
}

void MainWindow::updateAlarms(const reactorlab::plant::PlantState& s) {
  QStringList hits;
  if (s.P > 1.2) hits << QStringLiteral("HIGH POWER");
  if (s.Tf > 1200.0) hits << QStringLiteral("HIGH FUEL T");
  const double Xfp = reactorlab::plant::make_equilibrium_state(1.0, params_).X;
  if (s.P < 0.05 && s.X > 1.2 * Xfp) hits << QStringLiteral("XE PEAK / POST-SHUTDOWN");
  if (hits.isEmpty()) {
    alarms_->setText(QStringLiteral("Alarms: none"));
    alarms_->setStyleSheet(QStringLiteral("font-weight:600; color:#2a2;"));
  } else {
    alarms_->setText(QStringLiteral("Alarms: ") + hits.join(QStringLiteral(", ")));
    alarms_->setStyleSheet(QStringLiteral("font-weight:600; color:#c22;"));
  }
}

void MainWindow::onRodChanged(int value) {
  rod_z_ = static_cast<double>(value) / 1000.0;
  rod_label_->setText(QStringLiteral("Rod %1%").arg(rod_z_ * 100.0, 0, 'f', 1));
}

void MainWindow::onScenarioChanged(int index) {
  Q_UNUSED(index);
  const QString id = scenario_->currentData().toString();
  if (id == QLatin1String("scram_xe")) {
    time_accel_ = 3600.0;
    dt_sim_ = time_accel_;
  } else if (id == QLatin1String("rod_step")) {
    time_accel_ = 600.0;
    dt_sim_ = time_accel_;
  } else {
    time_accel_ = 60.0;
    dt_sim_ = time_accel_;
  }
}

void MainWindow::onRun() {
  rho_ext_ = rho_spin_->value();
  onScenarioChanged(scenario_->currentIndex());
  multi_running_ = false;
  running_ = true;
  timer_->start();
}

void MainWindow::onPause() {
  running_ = false;
  if (!multi_running_) {
    timer_->stop();
  }
}

void MainWindow::onReset() {
  onPause();
  state_ = reactorlab::plant::make_equilibrium_state(1.0, params_);
  rod_slider_->setValue(0);
  rod_z_ = 0.0;
  rho_spin_->setValue(0.0);
  rho_ext_ = 0.0;
  clearPlots();
  const double rho = reactorlab::plant::total_reactivity(
      0.0, state_, params_, reactorlab::kinetics::step_reactivity(0.0),
      [this](double) { return rod_z_; });
  appendPoint(state_, rho);
  updateStatus(state_, rho);
  updateAlarms(state_);
}

void MainWindow::onTick() {
  if (multi_running_) {
    try {
      multi_state_ = reactorlab::multiphysics::advance(
          multi_params_, multi_sc_.rho_ext, multi_sc_.rod_z, multi_sc_.mdot, multi_state_,
          multi_state_.t + multi_dt_);
      const double rho = reactorlab::multiphysics::total_reactivity(
          multi_state_.t, multi_state_.P, multi_state_.Tf, multi_state_.Tc_bar(), multi_params_,
          multi_sc_.rho_ext, multi_sc_.rod_z);
      const double mdot = multi_sc_.mdot(multi_state_.t);
      multiAppend(multi_state_, rho, mdot);
      multiUpdateStatus(multi_state_, rho, mdot);
      multiUpdateAlarms(multi_state_);
    } catch (const std::exception& ex) {
      multi_running_ = false;
      timer_->stop();
      QMessageBox::warning(this, QStringLiteral("Multiphysics error"),
                           QString::fromUtf8(ex.what()));
    }
    return;
  }
  if (!running_) return;
  const QString id = scenario_->currentData().toString();
  if (id == QLatin1String("scram_xe") && state_.t >= 1.0) {
    rod_slider_->setValue(1000);
    rod_z_ = 1.0;
  } else if (id == QLatin1String("rod_step") && state_.t >= 60.0) {
    rod_slider_->setValue(150);
    rod_z_ = 0.15;
  } else if (id == QLatin1String("doppler")) {
    rho_ext_ = 0.0015;
    rho_spin_->setValue(rho_ext_);
  } else {
    rho_ext_ = rho_spin_->value();
  }
  try {
    state_ = reactorlab::plant::advance(params_, reactorlab::kinetics::step_reactivity(rho_ext_),
                                        [this](double) { return rod_z_; }, state_,
                                        state_.t + dt_sim_);
    const double rho = reactorlab::plant::total_reactivity(
        state_.t, state_, params_, reactorlab::kinetics::step_reactivity(rho_ext_),
        [this](double) { return rod_z_; });
    appendPoint(state_, rho);
    updateStatus(state_, rho);
    updateAlarms(state_);
  } catch (const std::exception& ex) {
    onPause();
    QMessageBox::warning(this, QStringLiteral("Solver error"), QString::fromUtf8(ex.what()));
  }
}

void MainWindow::onLogScaleToggled(bool checked) {
  plot_power_->yAxis->setScaleType(checked ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
  plot_power_->yAxis->setRange(checked ? 1e-3 : 0.0, checked ? 10.0 : 2.0);
  plot_power_->replot();
}

void MainWindow::onRunMc() {
  try {
    const auto mat = reactorlab::montecarlo::material_2g_infinite();
    reactorlab::montecarlo::SlabGeometry geom;
    geom.L = mc_slab_L_->value();
    const auto res = reactorlab::montecarlo::power_iteration_slab(
        mat, geom, mc_histories_->value(), 15, 30,
        static_cast<std::uint64_t>(mc_seed_->value()));
    const double kinf = reactorlab::montecarlo::analytic_k_inf_2g(mat);
    mc_result_->setText(QStringLiteral("k_eff = %1 ± %2   (k_inf=%3)")
                            .arg(res.keff, 0, 'f', 5)
                            .arg(res.keff_std, 0, 'f', 5)
                            .arg(kinf, 0, 'f', 5));
    plot_mc_flux_->graph(0)->data()->clear();
    for (std::size_t g = 0; g < res.flux.size(); ++g) {
      plot_mc_flux_->graph(0)->addData(static_cast<double>(g), res.flux[g]);
    }
    plot_mc_flux_->rescaleAxes();
    plot_mc_flux_->replot();
    status_->setText(QStringLiteral("MC slab done"));
    alarms_->setText(res.keff < kinf ? QStringLiteral("Alarms: none (k < k_inf OK)")
                                     : QStringLiteral("Alarms: check (k >= k_inf)"));
  } catch (const std::exception& ex) {
    QMessageBox::warning(this, QStringLiteral("MC error"), QString::fromUtf8(ex.what()));
  }
}

void MainWindow::onAbout() {
  QMessageBox::about(
      this, QStringLiteral("About ReactorLab"),
      QStringLiteral("<h3>ReactorLab %1</h3><p>%2</p>"
                     "<p>Phase 6 productization: sessions, CSV export, scenario editor, "
                     "packaging.</p><p>See Help → References and docs/user-guide.md.</p>")
          .arg(QString::fromUtf8(reactorlab::version_string().data(),
                                 static_cast<int>(reactorlab::version_string().size())))
          .arg(QString::fromUtf8(reactorlab::educational_disclaimer().data(),
                                 static_cast<int>(reactorlab::educational_disclaimer().size()))));
}

void MainWindow::onReferences() {
  QMessageBox::information(
      this, QStringLiteral("References"),
      QStringLiteral(
          "<p><b>Nuclear data &amp; benchmarks</b> — see <code>data/PROVENANCE.md</code>.</p>"
          "<ul>"
          "<li>Keepin, G.R., <i>Physics of Nuclear Kinetics</i> (U-235 thermal 6-group).</li>"
          "<li>IAPWS R7-97 (IF97) Region 1 water properties.</li>"
          "<li>TWIGL 2D benchmark (OFELIA / standard literature values).</li>"
          "</ul>"
          "<p>Theory notes: <code>docs/theory/</code>. V&amp;V: <code>docs/vv/</code>.</p>"
          "<p><b>Disclaimer:</b> educational/research only — not safety-grade.</p>"));
}

QString MainWindow::scenariosDir() const {
  const QString src = QStringLiteral(REACTORLAB_SOURCE_DIR) + QStringLiteral("/scenarios");
  if (QDir(src).exists()) {
    return src;
  }
  return QStringLiteral("/usr/local/share/reactorlab/scenarios");
}

void MainWindow::onSaveSession() {
  const QString path = QFileDialog::getSaveFileName(
      this, QStringLiteral("Save session"), QStringLiteral("session.rlab.json"),
      QStringLiteral("ReactorLab session (*.rlab.json)"));
  if (path.isEmpty()) {
    return;
  }
  QJsonObject root;
  root.insert(QStringLiteral("version"),
              QString::fromUtf8(reactorlab::version_string().data(),
                                static_cast<int>(reactorlab::version_string().size())));
  root.insert(QStringLiteral("module_index"), module_->currentIndex());
  root.insert(QStringLiteral("plant_scenario"), scenario_->currentData().toString());
  root.insert(QStringLiteral("multi_scenario"), multi_scenario_->currentData().toString());
  root.insert(QStringLiteral("rod_z"), rod_z_);
  root.insert(QStringLiteral("rho_ext"), rho_spin_->value());
  root.insert(QStringLiteral("geometry"), QString::fromUtf8(saveGeometry().toBase64()));
  root.insert(QStringLiteral("loaded_scenario"), loaded_scenario_path_);
  root.insert(QStringLiteral("disclaimer"),
              QStringLiteral("Educational/research — not safety-grade"));
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(this, QStringLiteral("Save failed"), f.errorString());
    return;
  }
  f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  status_->setText(QStringLiteral("Session saved: %1").arg(path));
}

void MainWindow::onLoadSession() {
  const QString path = QFileDialog::getOpenFileName(
      this, QStringLiteral("Load session"), QString(),
      QStringLiteral("ReactorLab session (*.rlab.json)"));
  if (path.isEmpty()) {
    return;
  }
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, QStringLiteral("Load failed"), f.errorString());
    return;
  }
  const auto doc = QJsonDocument::fromJson(f.readAll());
  if (!doc.isObject()) {
    QMessageBox::warning(this, QStringLiteral("Load failed"), QStringLiteral("Invalid JSON"));
    return;
  }
  const QJsonObject root = doc.object();
  module_->setCurrentIndex(root.value(QStringLiteral("module_index")).toInt(0));
  const QString plant_sc = root.value(QStringLiteral("plant_scenario")).toString();
  for (int i = 0; i < scenario_->count(); ++i) {
    if (scenario_->itemData(i).toString() == plant_sc) {
      scenario_->setCurrentIndex(i);
      break;
    }
  }
  const QString multi_sc = root.value(QStringLiteral("multi_scenario")).toString();
  for (int i = 0; i < multi_scenario_->count(); ++i) {
    if (multi_scenario_->itemData(i).toString() == multi_sc) {
      multi_scenario_->setCurrentIndex(i);
      break;
    }
  }
  rod_slider_->setValue(static_cast<int>(root.value(QStringLiteral("rod_z")).toDouble() * 1000.0));
  rho_spin_->setValue(root.value(QStringLiteral("rho_ext")).toDouble());
  const QByteArray geom =
      QByteArray::fromBase64(root.value(QStringLiteral("geometry")).toString().toUtf8());
  if (!geom.isEmpty()) {
    restoreGeometry(geom);
  }
  loaded_scenario_path_ = root.value(QStringLiteral("loaded_scenario")).toString();
  if (!loaded_scenario_path_.isEmpty()) {
    applyLoadedScenario(loaded_scenario_path_);
  }
  status_->setText(QStringLiteral("Session loaded: %1").arg(path));
}

void MainWindow::onExportCsv() {
  if (export_t_.isEmpty()) {
    QMessageBox::information(this, QStringLiteral("Export CSV"),
                             QStringLiteral("No time series to export yet. Run a simulation first."));
    return;
  }
  const QString path = QFileDialog::getSaveFileName(
      this, QStringLiteral("Export CSV"), QStringLiteral("reactorlab_export.csv"),
      QStringLiteral("CSV (*.csv)"));
  if (path.isEmpty()) {
    return;
  }
  try {
    std::vector<reactorlab::io::CsvColumn> cols;
    cols.push_back({"t[s]", {export_t_.begin(), export_t_.end()}});
    cols.push_back({"P[-]", {export_P_.begin(), export_P_.end()}});
    cols.push_back({"Tf[K]", {export_Tf_.begin(), export_Tf_.end()}});
    cols.push_back({"Tc[K]", {export_Tc_.begin(), export_Tc_.end()}});
    const QString mod = module_->currentData().toString();
    if (mod == QLatin1String("multi")) {
      cols.push_back({"mdot[kg/s]", {export_extra_.begin(), export_extra_.end()}});
    } else {
      cols.push_back({"Xe[1/cm3]", {export_extra_.begin(), export_extra_.end()}});
    }
    reactorlab::io::write_csv(
        path.toStdString(), cols,
        std::string("module=") + mod.toStdString() + " version=" +
            std::string(reactorlab::version_string()));
    status_->setText(QStringLiteral("CSV exported: %1").arg(path));
  } catch (const std::exception& ex) {
    QMessageBox::warning(this, QStringLiteral("Export failed"), QString::fromUtf8(ex.what()));
  }
}

void MainWindow::applyLoadedScenario(const QString& path) {
  try {
    const auto sc = reactorlab::io::load_scenario_yaml(path.toStdString());
    loaded_scenario_path_ = path;
    if (sc.module == "multiphysics") {
      module_->setCurrentIndex(3);
      if (sc.driver_type == "rod_ejection") {
        multi_scenario_->setCurrentIndex(0);
      } else if (sc.driver_type == "loss_of_flow") {
        multi_scenario_->setCurrentIndex(1);
      } else if (sc.driver_type == "load_follow") {
        multi_scenario_->setCurrentIndex(2);
      }
      // Override drivers from YAML numerics when present
      multi_params_ = reactorlab::multiphysics::MultiParams{};
      if (sc.driver_type == "rod_ejection") {
        multi_sc_ = reactorlab::multiphysics::scenario_rod_ejection(
            sc.drivers.count("t_eject") ? sc.drivers.at("t_eject") : 1.0,
            sc.drivers.count("z0") ? sc.drivers.at("z0") : 0.08,
            sc.drivers.count("duration") ? sc.drivers.at("duration") : 0.2,
            multi_params_.mdot0);
        multi_dt_ = sc.dt > 0 ? sc.dt : 0.1;
      } else if (sc.driver_type == "loss_of_flow") {
        multi_sc_ = reactorlab::multiphysics::scenario_loss_of_flow(
            sc.drivers.count("t_start") ? sc.drivers.at("t_start") : 2.0,
            sc.drivers.count("fraction") ? sc.drivers.at("fraction") : 0.4,
            sc.drivers.count("ramp") ? sc.drivers.at("ramp") : 5.0, multi_params_.mdot0);
        multi_dt_ = sc.dt > 0 ? sc.dt : 0.25;
      } else {
        multi_sc_ = reactorlab::multiphysics::scenario_load_follow(
            sc.drivers.count("period") ? sc.drivers.at("period") : 100.0,
            sc.drivers.count("amp") ? sc.drivers.at("amp") : 0.0015, multi_params_.mdot0);
        multi_dt_ = sc.dt > 0 ? sc.dt : 1.0;
      }
      multi_state_ =
          reactorlab::multiphysics::make_equilibrium_state(1.0, multi_params_, multi_sc_.rod_z0);
      multiClearPlots();
      const double rho = reactorlab::multiphysics::total_reactivity(
          multi_state_.t, multi_state_.P, multi_state_.Tf, multi_state_.Tc_bar(), multi_params_,
          multi_sc_.rho_ext, multi_sc_.rod_z);
      multiAppend(multi_state_, rho, multi_sc_.mdot(multi_state_.t));
    } else if (sc.module == "plant") {
      module_->setCurrentIndex(0);
      if (sc.driver_type == "scram_xe") {
        scenario_->setCurrentIndex(2);
      }
      onReset();
    }
    status_->setText(QStringLiteral("Loaded scenario: %1").arg(QString::fromStdString(sc.name)));
  } catch (const std::exception& ex) {
    QMessageBox::warning(this, QStringLiteral("Scenario error"), QString::fromUtf8(ex.what()));
  }
}

void MainWindow::onScenarioEditor() {
  QDialog dlg(this);
  dlg.setWindowTitle(QStringLiteral("Scenario editor"));
  dlg.resize(720, 480);
  auto* root = new QVBoxLayout(&dlg);
  auto* split = new QHBoxLayout();
  auto* list = new QListWidget(&dlg);
  auto* edit = new QPlainTextEdit(&dlg);
  edit->setPlaceholderText(QStringLiteral("YAML scenario text…"));
  const QDir dir(scenariosDir());
  const auto files = dir.entryList(QStringList() << QStringLiteral("*.yaml"), QDir::Files);
  for (const QString& f : files) {
    list->addItem(f);
  }
  split->addWidget(list, 1);
  split->addWidget(edit, 2);
  root->addLayout(split, 1);
  auto* note = new QLabel(
      QStringLiteral("Edit YAML only — physics stays in libreactorlab. Educational use."), &dlg);
  note->setWordWrap(true);
  root->addWidget(note);
  auto* buttons = new QDialogButtonBox(
      QDialogButtonBox::Open | QDialogButtonBox::Save | QDialogButtonBox::Close, &dlg);
  buttons->button(QDialogButtonBox::Open)->setText(QStringLiteral("Apply"));
  root->addWidget(buttons);

  auto load_selected = [&]() {
    if (!list->currentItem()) return;
    const QString path = dir.filePath(list->currentItem()->text());
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
      edit->setPlainText(QString::fromUtf8(f.readAll()));
    }
  };
  connect(list, &QListWidget::currentRowChanged, &dlg, [&](int) { load_selected(); });
  if (list->count() > 0) {
    list->setCurrentRow(0);
  }
  connect(buttons, &QDialogButtonBox::clicked, &dlg, [&](QAbstractButton* btn) {
    if (buttons->buttonRole(btn) == QDialogButtonBox::AcceptRole ||
        btn == buttons->button(QDialogButtonBox::Open)) {
      // Apply
      const QString tmp = dir.filePath(QStringLiteral("_editor_apply.yaml"));
      {
        QFile f(tmp);
        if (f.open(QIODevice::WriteOnly)) {
          f.write(edit->toPlainText().toUtf8());
        }
      }
      applyLoadedScenario(tmp);
      dlg.accept();
    } else if (btn == buttons->button(QDialogButtonBox::Save)) {
      QString path;
      if (list->currentItem()) {
        path = dir.filePath(list->currentItem()->text());
      } else {
        path = QFileDialog::getSaveFileName(&dlg, QStringLiteral("Save scenario"),
                                            dir.filePath(QStringLiteral("custom.yaml")),
                                            QStringLiteral("YAML (*.yaml)"));
      }
      if (path.isEmpty()) return;
      QFile f(path);
      if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(&dlg, QStringLiteral("Save failed"), f.errorString());
        return;
      }
      f.write(edit->toPlainText().toUtf8());
      status_->setText(QStringLiteral("Scenario saved: %1").arg(path));
    } else {
      dlg.reject();
    }
  });
  dlg.exec();
}
