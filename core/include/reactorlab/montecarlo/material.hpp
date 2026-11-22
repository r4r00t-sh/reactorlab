#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace reactorlab::montecarlo {

inline constexpr std::size_t kMaxGroups = 8;

struct MgMaterial {
  int n_groups{2};
  std::array<double, kMaxGroups> Sigma_t{};
  std::array<double, kMaxGroups> Sigma_a{};
  std::array<double, kMaxGroups> nuSigma_f{};
  // row-major Σs[g_from * n_groups + g_to]
  std::array<double, kMaxGroups * kMaxGroups> Sigma_s{};

  [[nodiscard]] double Sigma_s_at(int g_from, int g_to) const {
    return Sigma_s[static_cast<std::size_t>(g_from * n_groups + g_to)];
  }
};

/// 2-group infinite medium with analytic k_inf = 5/6.
[[nodiscard]] MgMaterial material_2g_infinite();

[[nodiscard]] double analytic_k_inf_2g(const MgMaterial& mat);

}  // namespace reactorlab::montecarlo
