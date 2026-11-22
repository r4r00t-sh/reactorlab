#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace reactorlab::kinetics {

inline constexpr std::size_t kNumDelayedGroups = 6;

/// Six-group delayed-neutron parameters.
/// See docs/theory/delayed-neutrons-beta.md for β vs ν_d vs β_eff.
struct DelayedGroupData {
  std::array<double, kNumDelayedGroups> lambda_i{};
  std::array<double, kNumDelayedGroups> beta_i{};
  double Lambda{1.0e-4};  // prompt neutron generation time [s]
  double nu{0.0};         // average neutrons per fission ν [-]
  double nu_d{0.0};       // delayed neutrons per fission ν_d [-]
  std::string_view label{"unspecified"};

  /// β = Σ βᵢ (delayed fraction). Should satisfy β ≈ ν_d / ν when ν>0.
  [[nodiscard]] double beta() const noexcept {
    double sum = 0.0;
    for (double b : beta_i) {
      sum += b;
    }
    return sum;
  }

  /// ρ_$ = ρ / β_eff with ρ in Δk/k.
  [[nodiscard]] static double dollars(double rho_dk, double beta_eff) noexcept {
    if (beta_eff <= 0.0) {
      return 0.0;
    }
    return rho_dk / beta_eff;
  }
};

/// Keepin U-235 thermal (existing Phase 1). See data/PROVENANCE.md.
[[nodiscard]] DelayedGroupData u235_thermal_keepin() noexcept;

/// Keepin U-233 thermal — primary for Th-cycle kinetics.
/// Keepin, Wimett, Zeigler, Phys. Rev. 107, 1044 (1957). See PROVENANCE.md.
[[nodiscard]] DelayedGroupData u233_thermal_keepin() noexcept;

/// Keepin Pu-239 fast — PFBR-style stage kinetics set.
[[nodiscard]] DelayedGroupData pu239_fast_keepin() noexcept;

/// Fission-rate weighted β_eff = Σ (F_j β_j) / Σ F_j.
[[nodiscard]] double fission_weighted_beta_eff(const double* fission_rates,
                                               const double* betas,
                                               std::size_t n) noexcept;

}  // namespace reactorlab::kinetics
