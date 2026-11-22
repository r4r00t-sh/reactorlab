#include "reactorlab/montecarlo/eigenvalue.hpp"
#include "reactorlab/montecarlo/material.hpp"

#include <gtest/gtest.h>

#include <cmath>

TEST(MonteCarlo, AnalyticKInf) {
  const auto mat = reactorlab::montecarlo::material_2g_infinite();
  EXPECT_NEAR(reactorlab::montecarlo::analytic_k_inf_2g(mat), 5.0 / 6.0, 1e-15);
}

TEST(MonteCarlo, InfiniteMatchesAnalytic) {
  const auto mat = reactorlab::montecarlo::material_2g_infinite();
  const auto res = reactorlab::montecarlo::infinite_medium_keff(mat, 20000, 7);
  const double ref = reactorlab::montecarlo::analytic_k_inf_2g(mat);
  EXPECT_NEAR(res.keff, ref, 0.02);
  EXPECT_LT(std::abs(res.keff - ref), 3.0 * std::max(res.keff_std, 1e-4));
}

TEST(MonteCarlo, SlabBelowKInf) {
  const auto mat = reactorlab::montecarlo::material_2g_infinite();
  const double k_inf = reactorlab::montecarlo::analytic_k_inf_2g(mat);
  reactorlab::montecarlo::SlabGeometry geom;
  geom.L = 10.0;
  const auto res =
      reactorlab::montecarlo::power_iteration_slab(mat, geom, 1500, 15, 30, 3);
  EXPECT_LT(res.keff, k_inf);
  EXPECT_GT(res.keff, 0.1);
  EXPECT_GT(res.keff_std, 0.0);
}

TEST(MonteCarlo, SeedReproducibleInfinite) {
  const auto mat = reactorlab::montecarlo::material_2g_infinite();
  const auto a = reactorlab::montecarlo::infinite_medium_keff(mat, 5000, 99);
  const auto b = reactorlab::montecarlo::infinite_medium_keff(mat, 5000, 99);
  EXPECT_DOUBLE_EQ(a.keff, b.keff);
}
