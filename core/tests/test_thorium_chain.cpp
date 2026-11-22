#include "reactorlab/fuelcycle/thorium_chain.hpp"

#include <cmath>
#include <gtest/gtest.h>

using reactorlab::fuelcycle::advance_chain;
using reactorlab::fuelcycle::ahwr_edu_v1_initial_state;
using reactorlab::fuelcycle::ahwr_edu_v1_params;
using reactorlab::fuelcycle::bateman_th_pa_u_constant_flux;
using reactorlab::fuelcycle::ChainParams;
using reactorlab::fuelcycle::ChainState;
using reactorlab::fuelcycle::fission_shares;

namespace {

ChainParams bateman_params() {
  auto p = ahwr_edu_v1_params();
  // Validation path: no fission, no Pa capture — pure Th→Pa→U feed.
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

TEST(ThoriumChain, PlaceholdersOrCollapsedConsistent) {
  const auto p = ahwr_edu_v1_params();
  if (p.has_placeholders) {
    EXPECT_FALSE(p.placeholder_ids.empty());
  } else {
    EXPECT_TRUE(p.placeholder_ids.empty());
    EXPECT_NE(p.xs_provenance.find("OpenMC"), std::string::npos);
  }
  EXPECT_GT(p.sigma_c_Th232, 0.0);
  EXPECT_GT(p.sigma_f_U233, 0.0);
}

TEST(ThoriumChain, MassBalanceWithFission) {
  auto p = ahwr_edu_v1_params();
  auto s = ahwr_edu_v1_initial_state();
  const double A0 = s.accounted_atoms();
  const double dt = 1.0;
  for (int i = 0; i < 1000; ++i) {
    advance_chain(s, p, 1.0, dt);
  }
  EXPECT_NEAR(s.accounted_atoms(), A0, 1e-6 * A0);
  EXPECT_GT(s.fissioned, 0.0);
  EXPECT_GT(s.Pa233, 0.0);
}

TEST(ThoriumChain, BatemanConstantFlux) {
  const auto p = bateman_params();
  const double N0 = 1.0e22;
  const double phi = p.phi0;
  const double t = 3600.0 * 24.0;  // 1 day

  ChainState num;
  num.Th232 = N0;
  const int steps = 5000;
  const double dt = t / static_cast<double>(steps);
  for (int i = 0; i < steps; ++i) {
    advance_chain(num, p, 1.0, dt);
  }
  const auto ana = bateman_th_pa_u_constant_flux(N0, phi, t, p);

  EXPECT_NEAR(num.Th232, ana.Th232, 2e-3 * N0);
  EXPECT_NEAR(num.Th233, ana.Th233, std::max(1e10, 0.05 * std::abs(ana.Th233)));
  EXPECT_NEAR(num.Pa233, ana.Pa233, std::max(1e10, 0.05 * std::abs(ana.Pa233)));
  EXPECT_NEAR(num.U233, ana.U233, std::max(1e12, 0.08 * std::abs(ana.U233)));
}

TEST(ThoriumChain, ShutdownPaBreedsU233) {
  auto p = bateman_params();
  auto s = ahwr_edu_v1_initial_state();
  // Irradiate to build Pa
  for (int i = 0; i < 2000; ++i) {
    advance_chain(s, p, 1.0, 100.0);
  }
  const double Pa_after = s.Pa233;
  const double U_after_irr = s.U233;
  EXPECT_GT(Pa_after, 0.0);

  // Shutdown: P=0, Pa decays → U233 rises
  for (int i = 0; i < 5000; ++i) {
    advance_chain(s, p, 0.0, 1000.0);
  }
  EXPECT_LT(s.Pa233, Pa_after);
  EXPECT_GT(s.U233, U_after_irr);
}

TEST(ThoriumChain, BetaEffFromInventory) {
  const auto p = ahwr_edu_v1_params();
  const auto s = ahwr_edu_v1_initial_state();
  const auto sh = fission_shares(s, p, 1.0);
  EXPECT_GT(sh.F_U233, 0.0);
  EXPECT_GT(sh.F_Pu239, 0.0);
  EXPECT_GT(sh.beta_eff, 0.002);
  EXPECT_LT(sh.beta_eff, 0.0065);
}
