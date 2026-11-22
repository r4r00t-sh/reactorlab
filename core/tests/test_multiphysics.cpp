#include "reactorlab/multiphysics/coupled.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace reactorlab::multiphysics;

TEST(Multiphysics, EquilibriumNearCritical) {
  MultiParams params;
  auto s = make_equilibrium_state(1.0, params, 0.0);
  const double rho =
      total_reactivity(0.0, s.P, s.Tf, s.Tc_bar(), params, [](double) { return 0.0; },
                       [](double) { return 0.0; });
  EXPECT_NEAR(rho, 0.0, 1e-6);
}

TEST(Multiphysics, HoldSteady) {
  MultiParams params;
  auto s0 = make_equilibrium_state(1.0, params, 0.0);
  std::vector<double> t_out;
  for (int i = 0; i <= 10; ++i) {
    t_out.push_back(0.5 * static_cast<double>(i));
  }
  auto traj = integrate(params, [](double) { return 0.0; }, [](double) { return 0.0; },
                        [&](double) { return params.mdot0; }, t_out, s0);
  EXPECT_NEAR(traj.P.back(), 1.0, 0.02);
  EXPECT_NEAR(traj.Tf.back(), params.Tf0, 5.0);
}

TEST(Multiphysics, RodEjectionPowerRises) {
  MultiParams params;
  // z0·worth = 0.004 < β — delayed supercritical, educational (not prompt).
  auto sc = scenario_rod_ejection(1.0, 0.08, 0.2, params.mdot0);
  auto s0 = make_equilibrium_state(1.0, params, sc.rod_z0);
  std::vector<double> t_out;
  for (int i = 0; i <= 50; ++i) {
    t_out.push_back(0.1 * static_cast<double>(i));
  }
  auto traj = integrate(params, sc.rho_ext, sc.rod_z, sc.mdot, t_out, s0);
  EXPECT_GT(traj.P.back(), 1.05);
  EXPECT_GT(traj.Tf.back(), params.Tf0);
}

TEST(Multiphysics, LossOfFlowHeatsOutlet) {
  MultiParams params;
  auto sc = scenario_loss_of_flow(2.0, 0.4, 5.0, params.mdot0);
  auto s0 = make_equilibrium_state(1.0, params, sc.rod_z0);
  std::vector<double> t_out;
  for (int i = 0; i <= 40; ++i) {
    t_out.push_back(0.5 * static_cast<double>(i));
  }
  auto traj = integrate(params, sc.rho_ext, sc.rod_z, sc.mdot, t_out, s0);
  EXPECT_GT(traj.Tout.back(), traj.Tout.front() + 2.0);
  EXPECT_LT(traj.mdot.back(), 0.45 * params.mdot0);
}

TEST(Multiphysics, LoadFollowOscillates) {
  MultiParams params;
  auto sc = scenario_load_follow(100.0, 0.0015, params.mdot0);
  auto s0 = make_equilibrium_state(1.0, params, sc.rod_z0);
  std::vector<double> t_out;
  for (int i = 0; i <= 75; ++i) {
    t_out.push_back(2.0 * static_cast<double>(i));
  }
  auto traj = integrate(params, sc.rho_ext, sc.rod_z, sc.mdot, t_out, s0);
  const double pmax = *std::max_element(traj.P.begin(), traj.P.end());
  const double pmin = *std::min_element(traj.P.begin(), traj.P.end());
  EXPECT_GT(pmax, 1.02);
  EXPECT_LT(pmin, 0.98);
}
