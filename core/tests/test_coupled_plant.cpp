#include "reactorlab/plant/coupled_plant.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using reactorlab::kinetics::step_reactivity;
using reactorlab::plant::critical_bias;
using reactorlab::plant::equilibrium_xenon_reactivity;
using reactorlab::plant::integrate;
using reactorlab::plant::make_equilibrium_state;
using reactorlab::plant::PlantParams;
using reactorlab::plant::total_reactivity;

TEST(CoupledPlant, CriticalBiasZeroNetRho) {
  const PlantParams params;
  const auto s = make_equilibrium_state(1.0, params);
  const double rho =
      total_reactivity(0.0, s, params, step_reactivity(0.0), [](double) { return 0.0; });
  EXPECT_NEAR(rho, 0.0, 1e-12);
  EXPECT_GT(critical_bias(params), 0.0);
  EXPECT_LT(equilibrium_xenon_reactivity(1.0, params), -0.02);
}

TEST(CoupledPlant, FlatFullPowerHold) {
  const PlantParams params;
  auto s0 = make_equilibrium_state(1.0, params);
  std::vector<double> times;
  for (int i = 0; i <= 20; ++i) {
    times.push_back(5.0 * static_cast<double>(i));
  }
  const auto traj = integrate(params, step_reactivity(0.0), [](double) { return 0.0; }, times, s0);
  for (double P : traj.P) {
    EXPECT_NEAR(P, 1.0, 2e-3);
  }
  EXPECT_NEAR(traj.Tf.back(), params.Tf0, 1.0);
  EXPECT_NEAR(traj.Tc.back(), params.Tc0, 1.0);
}

TEST(CoupledPlant, XenonPeakAfterShutdown) {
  const PlantParams params;
  auto s0 = make_equilibrium_state(1.0, params);
  const double X0 = s0.X;

  std::vector<double> times;
  const double t_end = 40.0 * 3600.0;
  for (int i = 0; i <= 200; ++i) {
    times.push_back(t_end * static_cast<double>(i) / 200.0);
  }

  reactorlab::plant::SolverTolerances tol;
  tol.rtol = 1e-6;
  tol.atol = 1e-8;
  const auto traj =
      integrate(params, step_reactivity(0.0), [](double) { return 1.0; }, times, s0, tol);

  double Xmax = 0.0;
  double t_peak = 0.0;
  for (std::size_t i = 0; i < traj.X.size(); ++i) {
    if (traj.X[i] > Xmax) {
      Xmax = traj.X[i];
      t_peak = traj.t[i];
    }
  }
  EXPECT_LT(traj.P[10], 0.05);
  EXPECT_GT(Xmax, 1.05 * X0);
  EXPECT_GT(t_peak, 2.0 * 3600.0);
  EXPECT_LT(t_peak, 15.0 * 3600.0);
  EXPECT_LT(traj.X.back(), Xmax);
}

TEST(CoupledPlant, DopplerLimitsPowerRise) {
  const PlantParams params;
  auto s0 = make_equilibrium_state(1.0, params);
  std::vector<double> times;
  for (int i = 0; i <= 60; ++i) {
    times.push_back(static_cast<double>(i));
  }
  const auto traj =
      integrate(params, step_reactivity(0.0015), [](double) { return 0.0; }, times, s0);
  EXPECT_GT(traj.P.back(), 1.0);
  EXPECT_GT(traj.Tf.back(), params.Tf0);
  EXPECT_LT(traj.P.back(), 5.0);
}
