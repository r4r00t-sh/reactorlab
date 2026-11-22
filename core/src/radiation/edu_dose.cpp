#include "reactorlab/radiation/edu_dose.hpp"

#include <algorithm>
#include <cmath>

namespace reactorlab::radiation {
namespace {

double fission_rate(double P) {
  // Educational: thermal power → fission rate.
  const double watts = std::max(0.0, P) * kRatedWatts;
  return watts * kFissionsPerJoule;
}

}  // namespace

const std::array<GroupDef, kNumGroups>& default_groups() noexcept {
  // λ = ln2 / T½. Yields / Γ are educational proxies, not evaluated nuclear data.
  // Yields include an educational "release-to-point-source" scale (~1e-12) so
  // µSv/h readouts stay human-scale — not a source-term analysis.
  static const std::array<GroupDef, kNumGroups> g{{
      {"I-131·proxy", GroupKind::Gamma, std::log(2.0) / (8.02 * 86400.0), 2.0e-12, 0.22, 0.12},
      {"Cs-137·proxy", GroupKind::Gamma, std::log(2.0) / (30.17 * 365.25 * 86400.0), 1.2e-12, 0.10,
       0.10},
      {"noble-γ·proxy", GroupKind::Gamma, std::log(2.0) / (2.8 * 3600.0), 3.0e-12, 0.18, 0.08},
      {"Sr-90·proxy", GroupKind::Beta, std::log(2.0) / (28.8 * 365.25 * 86400.0), 1.5e-12, 0.0, 0.0},
      {"actinide-α·proxy", GroupKind::Alpha, std::log(2.0) / (432.0 * 365.25 * 86400.0), 4.0e-14, 0.0,
       0.0},
  }};
  return g;
}

std::string kind_name(GroupKind k) {
  switch (k) {
    case GroupKind::Gamma:
      return "gamma";
    case GroupKind::Beta:
      return "beta";
    case GroupKind::Alpha:
      return "alpha";
  }
  return "unknown";
}

void advance_inventory(Inventory& inv,
                       double P,
                       double dt_s,
                       const std::array<GroupDef, kNumGroups>& groups) {
  if (dt_s <= 0.0) {
    return;
  }
  const double F = fission_rate(P);
  for (std::size_t i = 0; i < kNumGroups; ++i) {
    const auto& g = groups[i];
    // A = λN, N' = Y F − λ N  →  A' = λ Y F − λ A  →  A_eq = Y F
    const double A_eq = g.yield * F;
    const double A0 = inv.activity_Bq[i];
    if (g.lambda <= 0.0) {
      inv.activity_Bq[i] = A0 + A_eq * dt_s;
      continue;
    }
    const double e = std::exp(-g.lambda * dt_s);
    inv.activity_Bq[i] = A0 * e + A_eq * (1.0 - e);
  }
}

Inventory make_equilibrium_inventory(double P, const std::array<GroupDef, kNumGroups>& groups) {
  Inventory inv;
  const double F = fission_rate(P);
  for (std::size_t i = 0; i < kNumGroups; ++i) {
    const auto& g = groups[i];
    if (g.lambda > 0.0) {
      inv.activity_Bq[i] = (g.yield * F) / g.lambda;
    } else {
      inv.activity_Bq[i] = 0.0;
    }
  }
  return inv;
}

DoseSample dose_at(const Inventory& inv,
                   double P,
                   double r_m,
                   const DoseParams& params,
                   const std::array<GroupDef, kNumGroups>& groups) {
  DoseSample out;
  out.r_m = std::max(0.1, r_m);
  const double r2 = out.r_m * out.r_m;
  double H_g = 0.0;
  for (std::size_t i = 0; i < kNumGroups; ++i) {
    const auto& g = groups[i];
    if (g.kind != GroupKind::Gamma || g.gamma_Gamma <= 0.0) {
      continue;
    }
    const double A_MBq = inv.activity_Bq[i] * 1.0e-6;
    const double atten = std::exp(-g.mu * params.shield_cm);
    H_g += A_MBq * g.gamma_Gamma * params.buildup * atten / r2;
  }
  out.H_gamma_uSvh = H_g;
  const double atten_n = std::exp(-params.mu_n * params.shield_cm);
  out.H_neutron_uSvh = params.neutron_k * std::max(0.0, P) * atten_n / r2;
  return out;
}

std::vector<DoseSample> dose_curve(const Inventory& inv,
                                   double P,
                                   const std::vector<double>& r_m,
                                   const DoseParams& params,
                                   const std::array<GroupDef, kNumGroups>& groups) {
  std::vector<DoseSample> out;
  out.reserve(r_m.size());
  for (double r : r_m) {
    out.push_back(dose_at(inv, P, r, params, groups));
  }
  return out;
}

double activity_by_kind(const Inventory& inv,
                        GroupKind kind,
                        const std::array<GroupDef, kNumGroups>& groups) {
  double sum = 0.0;
  for (std::size_t i = 0; i < kNumGroups; ++i) {
    if (groups[i].kind == kind) {
      sum += inv.activity_Bq[i];
    }
  }
  return sum;
}

}  // namespace reactorlab::radiation
