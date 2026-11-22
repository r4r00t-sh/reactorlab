#include "reactorlab/kinetics/point_kinetics.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using reactorlab::kinetics::DelayedGroupData;
using reactorlab::kinetics::inhour_reactivity;
using reactorlab::kinetics::integrate;
using reactorlab::kinetics::prompt_jump_ratio;
using reactorlab::kinetics::solve_inhour_omega;
using reactorlab::kinetics::step_reactivity;
using reactorlab::kinetics::u235_thermal_keepin;

TEST(PointKinetics, KeepinBetaSum) {
  const auto data = u235_thermal_keepin();
  EXPECT_NEAR(data.beta(), 0.006502, 1e-12);
}

TEST(PointKinetics, PromptJump) {
  const auto data = u235_thermal_keepin();
  const double rho = 0.0025;
  const double analytic = prompt_jump_ratio(rho, data);
  const double t_jump = 10.0 * data.Lambda / (data.beta() - rho);
  const auto traj = integrate(data, step_reactivity(rho), {0.0, t_jump});
  const double numerical = traj.P.back() / traj.P.front();
  EXPECT_NEAR(numerical, analytic, 0.05 * analytic);
}

TEST(PointKinetics, InhourStablePeriod) {
  const auto data = u235_thermal_keepin();
  const double rho = 0.001;
  const double omega = solve_inhour_omega(rho, data);
  EXPECT_NEAR(inhour_reactivity(omega, data), rho, 1e-12);
  ASSERT_GT(omega, 0.0);
  const double period = 1.0 / omega;
  const double t_end = std::max(5.0 * period, 20.0);

  std::vector<double> t_out;
  for (int i = 0; i < 50; ++i) {
    const double frac = static_cast<double>(i) / 49.0;
    t_out.push_back(0.5 * t_end + frac * (0.5 * t_end));
  }
  // Include start so CVODE initializes from equilibrium.
  std::vector<double> times{0.0};
  times.insert(times.end(), t_out.begin(), t_out.end());

  const auto traj = integrate(data, step_reactivity(rho), times);
  // Linear regression of ln P on the asymptotic half.
  double sum_t = 0.0;
  double sum_y = 0.0;
  double sum_tt = 0.0;
  double sum_ty = 0.0;
  const std::size_t n = traj.t.size() - 1; // skip t=0
  for (std::size_t i = 1; i < traj.t.size(); ++i) {
    const double t = traj.t[i];
    const double y = std::log(traj.P[i]);
    sum_t += t;
    sum_y += y;
    sum_tt += t * t;
    sum_ty += t * y;
  }
  const double denom = static_cast<double>(n) * sum_tt - sum_t * sum_t;
  const double slope = (static_cast<double>(n) * sum_ty - sum_t * sum_y) / denom;
  EXPECT_NEAR(slope, omega, 0.05 * omega);
}

TEST(PointKinetics, SubcriticalDecay) {
  const auto data = u235_thermal_keepin();
  const double rho = -0.002;
  const double omega = solve_inhour_omega(rho, data);
  ASSERT_LT(omega, 0.0);

  std::vector<double> times;
  for (int i = 0; i <= 800; ++i) {
    times.push_back(static_cast<double>(i));
  }
  const auto traj = integrate(data, step_reactivity(rho), times);
  EXPECT_LT(traj.P[1], traj.P[0]);
  EXPECT_LT(traj.P.back(), traj.P.front());

  double sum_t = 0.0;
  double sum_y = 0.0;
  double sum_tt = 0.0;
  double sum_ty = 0.0;
  int n = 0;
  for (std::size_t i = 0; i < traj.t.size(); ++i) {
    if (traj.t[i] < 500.0) {
      continue;
    }
    const double t = traj.t[i];
    const double y = std::log(traj.P[i]);
    sum_t += t;
    sum_y += y;
    sum_tt += t * t;
    sum_ty += t * y;
    ++n;
  }
  ASSERT_GT(n, 10);
  const double denom = static_cast<double>(n) * sum_tt - sum_t * sum_t;
  const double slope = (static_cast<double>(n) * sum_ty - sum_t * sum_y) / denom;
  EXPECT_NEAR(slope, omega, 0.02 * std::abs(omega));
}

TEST(PointKinetics, ZeroReactivityFlat) {
  const auto data = u235_thermal_keepin();
  std::vector<double> times;
  for (int i = 0; i <= 10; ++i) {
    times.push_back(static_cast<double>(i));
  }
  const auto traj = integrate(data, step_reactivity(0.0), times);
  for (double P : traj.P) {
    EXPECT_NEAR(P, 1.0, 1e-6);
  }
}
