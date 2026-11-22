#include "reactorlab/diffusion/two_group.hpp"

#include <gtest/gtest.h>

#include <cmath>

TEST(Diffusion, TwiglKeff) {
  const auto res = reactorlab::diffusion::solve_twigl(40, 1e-7);
  // Reference 0.91321; 40×40 mesh ~10 pcm in Python reference.
  EXPECT_NEAR(res.keff, 0.91321, 5e-4);
  EXPECT_GT(res.iterations, 1);
  ASSERT_EQ(static_cast<int>(res.phi2.size()), 40 * 40);
  double max_phi2 = 0.0;
  for (double v : res.phi2) {
    EXPECT_GE(v, -1e-12);
    max_phi2 = std::max(max_phi2, v);
  }
  EXPECT_GT(max_phi2, 0.0);
}

TEST(Diffusion, TwiglMeshConvergence) {
  const auto coarse = reactorlab::diffusion::solve_twigl(20, 1e-6);
  const auto fine = reactorlab::diffusion::solve_twigl(40, 1e-7);
  EXPECT_LT(std::abs(fine.keff - 0.91321), std::abs(coarse.keff - 0.91321) + 1e-4);
}
