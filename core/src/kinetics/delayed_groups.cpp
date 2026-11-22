#include "reactorlab/kinetics/delayed_groups.hpp"

#include <cmath>

namespace reactorlab::kinetics {
namespace {

constexpr double ln2 = 0.6931471805599453;

}  // namespace

DelayedGroupData u235_thermal_keepin() noexcept {
  // Keepin (1965) U-235 thermal — see data/PROVENANCE.md
  DelayedGroupData d;
  d.lambda_i = {0.0124, 0.0305, 0.111, 0.301, 1.14, 3.01};
  d.beta_i = {0.000215, 0.001424, 0.001274, 0.002568, 0.000748, 0.000273};
  d.Lambda = 1.0e-4;
  d.nu = 2.43;
  d.nu_d = d.beta() * d.nu;  // consistent with shipped β; Keepin ν_d ~0.0158 thermal U-235 order
  d.label = "Keepin U-235 thermal";
  return d;
}

DelayedGroupData u233_thermal_keepin() noexcept {
  // Keepin, Wimett, Zeigler, Phys. Rev. 107, 1044 (1957) — U-233 thermal.
  // Half-lives [s] and β_i as tabulated e.g. in CETR physics design (UNT) citing Keepin.
  // ν_d = 0.0070 delayed n/fission; ν = 2.52 → β = ν_d/ν ≈ 0.00278.
  DelayedGroupData d;
  const std::array<double, kNumDelayedGroups> T_half = {55.11, 20.74, 5.30, 2.29, 0.546, 0.221};
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    d.lambda_i[i] = ln2 / T_half[i];
  }
  d.beta_i = {0.000238, 0.000762, 0.000631, 0.000881, 0.000202, 0.000064};
  d.Lambda = 1.0e-4;  // educational thermal Λ (same order as U-235 set; not AHWR-specific)
  d.nu = 2.52;
  d.nu_d = 0.0070;
  d.label = "Keepin U-233 thermal";
  return d;
}

DelayedGroupData pu239_fast_keepin() noexcept {
  // Keepin et al. Pu-239 fast-neutron fission 6-group (Phys. Rev. 107, 1957).
  // Absolute group yields [% of fissions] → β_i = yield/100 / ν with ν≈2.94 fast Pu
  // Using Keepin absolute delayed yield ν_d≈0.0063 and recommended group structure
  // as commonly tabulated for fast Pu-239 (see PROVENANCE.md).
  DelayedGroupData d;
  const std::array<double, kNumDelayedGroups> T_half = {54.28, 23.04, 5.60, 2.13, 0.618, 0.257};
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    d.lambda_i[i] = ln2 / T_half[i];
  }
  // β_i from Keepin relative abundances × β with β≈0.0020–0.0021 for fast Pu-239
  d.beta_i = {0.000081, 0.000629, 0.000444, 0.000685, 0.000180, 0.000093};
  d.Lambda = 4.0e-7;  // fast spectrum generation time (educational order)
  d.nu = 2.94;
  d.nu_d = d.beta() * d.nu;
  d.label = "Keepin Pu-239 fast";
  return d;
}

double fission_weighted_beta_eff(const double* fission_rates,
                                 const double* betas,
                                 std::size_t n) noexcept {
  double num = 0.0;
  double den = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    const double F = fission_rates[i] > 0.0 ? fission_rates[i] : 0.0;
    num += F * betas[i];
    den += F;
  }
  if (den <= 0.0) {
    return 0.0;
  }
  return num / den;
}

}  // namespace reactorlab::kinetics
