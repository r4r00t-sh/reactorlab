#pragma once

#include "reactorlab/montecarlo/material.hpp"

#include <cstdint>
#include <vector>

namespace reactorlab::montecarlo {

struct McResult {
  double keff{0.0};
  double keff_std{0.0};
  std::vector<double> cycle_keff;
  std::vector<double> flux;      // by group
  std::vector<double> flux_std;
  int n_active{0};
  std::uint64_t seed{0};
};

struct SlabGeometry {
  double L{20.0};  // cm
};

/// Collision-estimator k_inf (infinite homogeneous), χ₁=1.
[[nodiscard]] McResult infinite_medium_keff(const MgMaterial& mat,
                                            int n_histories = 5000,
                                            std::uint64_t seed = 1);

/// Analog power iteration in a homogeneous 1D slab with vacuum BC.
[[nodiscard]] McResult power_iteration_slab(const MgMaterial& mat,
                                            const SlabGeometry& geom,
                                            int n_histories = 2000,
                                            int n_inactive = 20,
                                            int n_active = 40,
                                            std::uint64_t seed = 42);

}  // namespace reactorlab::montecarlo
