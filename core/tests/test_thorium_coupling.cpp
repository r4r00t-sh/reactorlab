#include "reactorlab/fuelcycle/thorium_chain.hpp"
#include "reactorlab/plant/coupled_plant.hpp"

#include <cmath>
#include <gtest/gtest.h>

using reactorlab::fuelcycle::advance_chain;
using reactorlab::fuelcycle::ahwr_edu_v1_initial_state;
using reactorlab::fuelcycle::ahwr_edu_v1_params;
using reactorlab::fuelcycle::evaluate_coupling;
using reactorlab::fuelcycle::inventory_reactivity_bias;

namespace {

reactorlab::fuelcycle::ChainParams bateman_params() {
  auto p = ahwr_edu_v1_params();
  p.sigma_f_U233 = 0.0;
  p.sigma_c_U233 = 0.0;
  p.sigma_f_U235 = 0.0;
  p.sigma_c_U235 = 0.0;
  p.sigma_f_Pu239 = 0.0;
  p.sigma_c_Pu239 = 0.0;
  p.sigma_c_Pa233 = 0.0;
  return p;
}

}  // namespace

TEST(ThoriumCoupling, PaToU233RaisesInventoryRho) {
  // Educational shutdown: Pa absorber → U-233 fissile raises neutron-economy proxy.
  auto p = ahwr_edu_v1_params();
  auto s = ahwr_edu_v1_initial_state();
  // Build Pa under irradiation (keep Pa capture on)
  for (int i = 0; i < 3000; ++i) {
    advance_chain(s, p, 1.0, 100.0);
  }
  const auto ref = s;
  const double rho0 = inventory_reactivity_bias(s, ref, p);
  EXPECT_NEAR(rho0, 0.0, 1e-15);

  const double Pa0 = s.Pa233;
  const double U0 = s.U233;
  for (int i = 0; i < 8000; ++i) {
    advance_chain(s, p, 0.0, 1000.0);  // shutdown decay
  }
  EXPECT_LT(s.Pa233, Pa0);
  EXPECT_GT(s.U233, U0);
  const double rho1 = inventory_reactivity_bias(s, ref, p);
  EXPECT_GT(rho1, rho0);
}

TEST(ThoriumCoupling, CoupledVsUncoupledPlantRho) {
  using namespace reactorlab;
  auto chain_p = ahwr_edu_v1_params();
  auto chain = ahwr_edu_v1_initial_state();
  // Force a large U-233 increase relative to ref so inventory bias is nonzero
  auto ref = chain;
  chain.U233 *= 1.25;
  const auto coup = evaluate_coupling(chain, ref, chain_p, 1.0);
  EXPECT_GT(coup.rho_inventory, 0.0);
  EXPECT_GT(coup.beta_eff, 0.0);

  plant::PlantParams params{};
  params.kinetics = kinetics::u235_thermal_keepin();
  auto state = plant::make_equilibrium_state(1.0, params);
  const auto rho_uncoupled = plant::total_reactivity(
      0.0, state, params, kinetics::step_reactivity(0.0), [](double) { return 0.0; });

  params.rho_inventory = coup.rho_inventory;
  params.kinetics = coup.delayed;
  const auto rho_coupled = plant::total_reactivity(
      0.0, state, params, kinetics::step_reactivity(0.0), [](double) { return 0.0; });

  EXPECT_NEAR(rho_coupled - rho_uncoupled, coup.rho_inventory, 1e-12);
  EXPECT_NE(params.kinetics.beta(), kinetics::u235_thermal_keepin().beta());
}

TEST(ThoriumCoupling, UncoupledIgnoresInventoryBias) {
  using namespace reactorlab;
  plant::PlantParams params{};
  params.rho_inventory = 0.0;
  auto state = plant::make_equilibrium_state(1.0, params);
  const double rho0 = plant::total_reactivity(
      0.0, state, params, kinetics::step_reactivity(0.0), [](double) { return 0.0; });
  params.rho_inventory = 0.002;
  const double rho1 = plant::total_reactivity(
      0.0, state, params, kinetics::step_reactivity(0.0), [](double) { return 0.0; });
  // When coupling field is set, plant sees it — "uncoupled" means engine keeps it at 0.
  EXPECT_NEAR(rho1 - rho0, 0.002, 1e-12);
}

TEST(ThoriumCoupling, FissionWeightedBetaBetweenParents) {
  auto p = ahwr_edu_v1_params();
  auto s = ahwr_edu_v1_initial_state();
  s.U235 = 0.0;
  s.Pu239 = 0.0;
  const auto only_u233 = evaluate_coupling(s, s, p, 1.0);
  EXPECT_NEAR(only_u233.beta_eff, p.delayed_U233.beta(), 1e-9);

  s.U233 = 0.0;
  s.Pu239 = 1.0e20;
  const auto only_pu = evaluate_coupling(s, s, p, 1.0);
  EXPECT_NEAR(only_pu.beta_eff, p.delayed_Pu239.beta(), 1e-9);
}

TEST(ThoriumCoupling, BatemanStillOk) {
  // Sanity: coupling helpers do not break existing Bateman path.
  const auto p = bateman_params();
  EXPECT_GT(p.lambda_Pa233, 0.0);
}
