#include "reactorlab/kinetics/delayed_groups.hpp"
#include "reactorlab/kinetics/point_kinetics.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using reactorlab::kinetics::DelayedGroupData;
using reactorlab::kinetics::fission_weighted_beta_eff;
using reactorlab::kinetics::integrate;
using reactorlab::kinetics::pu239_fast_keepin;
using reactorlab::kinetics::step_reactivity;
using reactorlab::kinetics::u233_thermal_keepin;
using reactorlab::kinetics::u235_thermal_keepin;

// DelayedGroupData::dollars is static method — use that name
namespace {
double rho_dollars(double rho, double beta_eff) {
  return DelayedGroupData::dollars(rho, beta_eff);
}
}  // namespace

TEST(DelayedU233, KeepinBetaAndNuDConsistency) {
  const auto d = u233_thermal_keepin();
  EXPECT_NEAR(d.beta(), 0.002778, 1e-6);
  EXPECT_NEAR(d.nu_d, 0.0070, 1e-12);
  EXPECT_NEAR(d.nu, 2.52, 1e-12);
  // β ≈ ν_d / ν
  EXPECT_NEAR(d.beta(), d.nu_d / d.nu, 5e-5);
  EXPECT_EQ(d.label, "Keepin U-233 thermal");
}

TEST(DelayedU233, MuchSmallerBetaThanU235) {
  EXPECT_LT(u233_thermal_keepin().beta(), 0.5 * u235_thermal_keepin().beta());
}

TEST(DelayedPu239Fast, BetaOrder) {
  const auto d = pu239_fast_keepin();
  EXPECT_GT(d.beta(), 0.0015);
  EXPECT_LT(d.beta(), 0.0035);
  EXPECT_LT(d.Lambda, 1e-5);
}

TEST(DelayedBetaEff, FissionWeighted) {
  const double rates[3] = {1.0, 0.0, 0.0};
  const double betas[3] = {0.002778, 0.006502, 0.0021};
  EXPECT_NEAR(fission_weighted_beta_eff(rates, betas, 3), 0.002778, 1e-12);
  const double mix[3] = {1.0, 1.0, 0.0};
  EXPECT_NEAR(fission_weighted_beta_eff(mix, betas, 3), 0.5 * (0.002778 + 0.006502), 1e-12);
}

TEST(DelayedDollars, UsesBetaEff) {
  const double rho = 0.002778;  // 1$ on U-233 β
  EXPECT_NEAR(rho_dollars(rho, 0.002778), 1.0, 1e-9);
  EXPECT_NEAR(rho_dollars(rho, 0.006502), rho / 0.006502, 1e-9);
}

TEST(DelayedU233, FasterPromptJumpThanU235) {
  // Same ρ (Δk/k): U-233 has smaller β → closer to prompt critical → larger jump.
  const double rho = 0.0015;
  const auto u5 = u235_thermal_keepin();
  const auto u3 = u233_thermal_keepin();
  ASSERT_LT(rho, u3.beta());
  ASSERT_LT(rho, u5.beta());

  const double t_end = 1.0e-2;
  const auto traj5 = integrate(u5, step_reactivity(rho), {0.0, t_end});
  const auto traj3 = integrate(u3, step_reactivity(rho), {0.0, t_end});
  const double jump5 = traj5.P.back() / traj5.P.front();
  const double jump3 = traj3.P.back() / traj3.P.front();
  EXPECT_GT(jump3, jump5);
}
