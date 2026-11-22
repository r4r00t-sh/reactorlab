#include "reactorlab/radiation/edu_dose.hpp"

#include <gtest/gtest.h>

#include <cmath>

using reactorlab::radiation::DoseParams;
using reactorlab::radiation::GroupKind;
using reactorlab::radiation::Inventory;
using reactorlab::radiation::activity_by_kind;
using reactorlab::radiation::advance_inventory;
using reactorlab::radiation::dose_at;
using reactorlab::radiation::make_equilibrium_inventory;

TEST(RadiationEdu, EquilibriumPositive) {
  const auto inv = make_equilibrium_inventory(1.0);
  double sum = 0.0;
  for (double a : inv.activity_Bq) {
    sum += a;
    EXPECT_GE(a, 0.0);
  }
  EXPECT_GT(sum, 0.0);
}

TEST(RadiationEdu, AdvanceIncreasesFromZero) {
  Inventory inv{};
  advance_inventory(inv, 1.0, 3600.0);
  double sum = 0.0;
  for (double a : inv.activity_Bq) {
    sum += a;
  }
  EXPECT_GT(sum, 0.0);
}

TEST(RadiationEdu, DoseFallsWithDistanceAndShield) {
  const auto inv = make_equilibrium_inventory(1.0);
  DoseParams thin{};
  thin.shield_cm = 0.0;
  DoseParams thick = thin;
  thick.shield_cm = 100.0;

  const auto near = dose_at(inv, 1.0, 1.0, thin);
  const auto far = dose_at(inv, 1.0, 10.0, thin);
  const auto shielded = dose_at(inv, 1.0, 1.0, thick);

  EXPECT_GT(near.H_gamma_uSvh, far.H_gamma_uSvh);
  EXPECT_GT(near.H_neutron_uSvh, far.H_neutron_uSvh);
  EXPECT_GT(near.H_gamma_uSvh, shielded.H_gamma_uSvh);
  EXPECT_GT(near.H_neutron_uSvh, shielded.H_neutron_uSvh);
}

TEST(RadiationEdu, AlphaBetaAreInventoryOnlyTotals) {
  const auto inv = make_equilibrium_inventory(1.0);
  EXPECT_GT(activity_by_kind(inv, GroupKind::Alpha), 0.0);
  EXPECT_GT(activity_by_kind(inv, GroupKind::Beta), 0.0);
  EXPECT_GT(activity_by_kind(inv, GroupKind::Gamma), 0.0);
}
