#pragma once

#include <array>
#include <cstddef>

namespace reactorlab::kinetics {

inline constexpr std::size_t kNumDelayedGroups = 6;

/// Six-group delayed-neutron parameters. Units: lambda_i [1/s], beta_i [-], Lambda [s].
struct DelayedGroupData {
  std::array<double, kNumDelayedGroups> lambda_i{};
  std::array<double, kNumDelayedGroups> beta_i{};
  double Lambda{1.0e-4}; // prompt neutron generation time [s]

  [[nodiscard]] double beta() const noexcept {
    double sum = 0.0;
    for (double b : beta_i) {
      sum += b;
    }
    return sum;
  }
};

/// Keepin U-235 thermal set (see data/PROVENANCE.md).
[[nodiscard]] DelayedGroupData u235_thermal_keepin() noexcept;

} // namespace reactorlab::kinetics
