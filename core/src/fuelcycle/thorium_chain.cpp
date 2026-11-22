#include "reactorlab/fuelcycle/thorium_chain.hpp"

#include <algorithm>
#include <cmath>

namespace reactorlab::fuelcycle {
namespace {

constexpr double kBarnCm2 = 1.0e-24;
constexpr double kLn2 = 0.6931471805599453;

double clamp_nonneg(double x) {
  return x < 0.0 ? 0.0 : x;
}

}  // namespace

ChainParams ahwr_edu_v1_params() {
  ChainParams p;
  // Decay: IAEA LiveChart / ENSDF — see data/PROVENANCE.md and ahwr_edu_chain_v1.yaml
  p.lambda_Th233 = kLn2 / 1309.8;
  p.lambda_Pa233 = kLn2 / 2331072.0;
  p.lambda_U233 = kLn2 / 5.024e12;
  p.lambda_U234 = kLn2 / 7.747e12;
  p.lambda_U235 = kLn2 / 2.221e16;
  p.lambda_U236 = kLn2 / 7.391e14;

  // One-group microscopic XS [cm²] from OpenMC ENDF/B-VIII.0 collapse
  // (data/thorium/openmc_collapse/). Generated .inc is required for release.
#if __has_include("reactorlab/fuelcycle/ahwr_collapsed_xs.inc")
#include "reactorlab/fuelcycle/ahwr_collapsed_xs.inc"
  p.sigma_c_Th232 = REACTORLAB_XS_Th232_c * kBarnCm2;
  p.sigma_c_Pa233 = REACTORLAB_XS_Pa233_c * kBarnCm2;
  p.sigma_f_U233 = REACTORLAB_XS_U233_f * kBarnCm2;
  p.sigma_c_U233 = REACTORLAB_XS_U233_c * kBarnCm2;
  p.sigma_f_U235 = REACTORLAB_XS_U235_f * kBarnCm2;
  p.sigma_c_U235 = REACTORLAB_XS_U235_c * kBarnCm2;
  p.sigma_f_Pu239 = REACTORLAB_XS_Pu239_f * kBarnCm2;
  p.sigma_c_Pu239 = REACTORLAB_XS_Pu239_c * kBarnCm2;
  p.has_placeholders = false;
  p.placeholder_ids.clear();
  p.xs_provenance =
      "OpenMC ENDF/B-VIII.0 collapse — data/thorium/openmc_collapse/";
#else
#error "Missing ahwr_collapsed_xs.inc — run scripts/openmc/collapse + apply_collapse_to_tree.py"
#endif

  p.nu_U233 = 2.52;
  p.nu_U235 = 2.43;
  p.nu_Pu239 = 2.88;
  p.phi0 = 5.0e13;
  p.sigma_f0_norm = 0.1;
  return p;
}

ChainState ahwr_edu_v1_initial_state() {
  ChainState s;
  s.Th232 = 1.0e22;
  s.U233 = 2.0e20;
  s.Pu239 = 5.0e19;
  return s;
}

FissionShares fission_shares(const ChainState& s, const ChainParams& p, double P) {
  const double phi = p.phi0 * std::max(0.0, P);
  FissionShares out;
  out.F_U233 = s.U233 * p.sigma_f_U233 * phi;
  out.F_U235 = s.U235 * p.sigma_f_U235 * phi;
  out.F_Pu239 = s.Pu239 * p.sigma_f_Pu239 * phi;
  const double rates[3] = {out.F_U233, out.F_U235, out.F_Pu239};
  const double betas[3] = {p.delayed_U233.beta(), p.delayed_U235.beta(), p.delayed_Pu239.beta()};
  out.beta_eff = kinetics::fission_weighted_beta_eff(rates, betas, 3);
  return out;
}

double inventory_reactivity(const ChainState& s, const ChainParams& p) noexcept {
  // η_i = ν σ_f − σ_a  with σ_a = σ_f + σ_c for fissile, σ_c for fertile/Pa
  const double eta =
      s.Th232 * (0.0 - p.sigma_c_Th232) + s.Th233 * (0.0 - 0.0) +
      s.Pa233 * (0.0 - p.sigma_c_Pa233) +
      s.U233 * (p.nu_U233 * p.sigma_f_U233 - p.sigma_f_U233 - p.sigma_c_U233) +
      s.U234 * (0.0 - 0.0) +
      s.U235 * (p.nu_U235 * p.sigma_f_U235 - p.sigma_f_U235 - p.sigma_c_U235) +
      s.U236 * (0.0 - 0.0) +
      s.Pu239 * (p.nu_Pu239 * p.sigma_f_Pu239 - p.sigma_f_Pu239 - p.sigma_c_Pu239);
  const double denom = p.nu_U233 * p.sigma_f0_norm;
  if (denom <= 0.0) {
    return 0.0;
  }
  return eta / denom;
}

double inventory_reactivity_bias(const ChainState& s,
                                 const ChainState& ref,
                                 const ChainParams& p) noexcept {
  return inventory_reactivity(s, p) - inventory_reactivity(ref, p);
}

kinetics::DelayedGroupData fission_weighted_delayed(const ChainState& s,
                                                    const ChainParams& p,
                                                    double P) {
  const auto sh = fission_shares(s, p, P);
  const double w[3] = {sh.F_U233, sh.F_U235, sh.F_Pu239};
  double wsum = w[0] + w[1] + w[2];
  const kinetics::DelayedGroupData* sets[3] = {&p.delayed_U233, &p.delayed_U235,
                                               &p.delayed_Pu239};
  kinetics::DelayedGroupData out = p.delayed_U233;
  if (wsum <= 0.0) {
    // Shutdown / no fission: keep U-233 thermal set (primary Th-cycle), β unchanged
    out = p.delayed_U233;
    out.label = "U-233 thermal (no fission weight)";
    return out;
  }
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    double beta = 0.0;
    double lam = 0.0;
    for (int j = 0; j < 3; ++j) {
      const double wj = w[j] / wsum;
      beta += wj * sets[j]->beta_i[i];
      lam += wj * sets[j]->lambda_i[i];
    }
    out.beta_i[i] = beta;
    out.lambda_i[i] = lam;
  }
  out.Lambda = 0.0;
  out.nu = 0.0;
  for (int j = 0; j < 3; ++j) {
    const double wj = w[j] / wsum;
    out.Lambda += wj * sets[j]->Lambda;
    out.nu += wj * sets[j]->nu;
  }
  out.nu_d = out.beta() * out.nu;
  out.label = "fission-weighted (U233/U235/Pu239)";
  return out;
}

CouplingOutputs evaluate_coupling(const ChainState& s,
                                  const ChainState& ref,
                                  const ChainParams& p,
                                  double P) {
  CouplingOutputs o;
  o.rho_inventory = inventory_reactivity_bias(s, ref, p);
  o.delayed = fission_weighted_delayed(s, p, std::max(P, 1.0e-12));
  // At shutdown use last positive-power weights via P_floor for β_eff continuity:
  // prefer fission_shares at max(P, eps) already done; beta_eff from delayed sum.
  o.beta_eff = o.delayed.beta();
  if (o.beta_eff <= 0.0) {
    o.beta_eff = p.delayed_U233.beta();
    o.delayed = p.delayed_U233;
  }
  return o;
}

void advance_chain(ChainState& s, const ChainParams& p, double P, double dt_s) {
  if (dt_s <= 0.0) {
    return;
  }
  const double phi = p.phi0 * std::max(0.0, P);

  const double N_Th232 = s.Th232;
  const double N_Th233 = s.Th233;
  const double N_Pa233 = s.Pa233;
  const double N_U233 = s.U233;
  const double N_U234 = s.U234;
  const double N_U235 = s.U235;
  const double N_U236 = s.U236;
  const double N_Pu239 = s.Pu239;

  const double capt_Th232 = p.sigma_c_Th232 * phi * N_Th232;
  const double capt_Pa233 = p.sigma_c_Pa233 * phi * N_Pa233;
  const double fis_U233 = p.sigma_f_U233 * phi * N_U233;
  const double capt_U233 = p.sigma_c_U233 * phi * N_U233;
  const double fis_U235 = p.sigma_f_U235 * phi * N_U235;
  const double capt_U235 = p.sigma_c_U235 * phi * N_U235;
  const double fis_Pu239 = p.sigma_f_Pu239 * phi * N_Pu239;
  const double capt_Pu239 = p.sigma_c_Pu239 * phi * N_Pu239;

  const double dTh232 = -capt_Th232;
  const double dTh233 = capt_Th232 - p.lambda_Th233 * N_Th233;
  const double dPa233 = p.lambda_Th233 * N_Th233 - p.lambda_Pa233 * N_Pa233 - capt_Pa233;
  const double dU233 =
      p.lambda_Pa233 * N_Pa233 - fis_U233 - capt_U233 - p.lambda_U233 * N_U233;
  const double dU234 =
      capt_Pa233 + capt_U233 + p.lambda_U233 * N_U233 - p.lambda_U234 * N_U234;
  const double dU235 =
      p.lambda_U234 * N_U234 - fis_U235 - capt_U235 - p.lambda_U235 * N_U235;
  const double dU236 = capt_U235 + p.lambda_U235 * N_U235 - p.lambda_U236 * N_U236;
  const double dPu239 = -fis_Pu239 - capt_Pu239;

  const double fiss_removed = (fis_U233 + fis_U235 + fis_Pu239) * dt_s;

  s.Th232 = clamp_nonneg(N_Th232 + dTh232 * dt_s);
  s.Th233 = clamp_nonneg(N_Th233 + dTh233 * dt_s);
  s.Pa233 = clamp_nonneg(N_Pa233 + dPa233 * dt_s);
  s.U233 = clamp_nonneg(N_U233 + dU233 * dt_s);
  s.U234 = clamp_nonneg(N_U234 + dU234 * dt_s);
  s.U235 = clamp_nonneg(N_U235 + dU235 * dt_s);
  s.U236 = clamp_nonneg(N_U236 + dU236 * dt_s);
  s.Pu239 = clamp_nonneg(N_Pu239 + dPu239 * dt_s);
  s.fissioned += fiss_removed;
}

ChainState bateman_th_pa_u_constant_flux(double N_Th232_0,
                                         double phi,
                                         double t,
                                         const ChainParams& p) {
  ChainState out;
  const double a = p.sigma_c_Th232 * phi;
  const double l1 = p.lambda_Th233;
  const double l2 = p.lambda_Pa233;

  out.Th232 = N_Th232_0 * std::exp(-a * t);

  if (t <= 0.0) {
    out.Th232 = N_Th232_0;
    return out;
  }

  const double N0 = N_Th232_0;
  double N2 = 0.0;
  if (std::abs(l1 - a) < 1e-30) {
    N2 = a * N0 * t * std::exp(-a * t);
  } else {
    N2 = a * N0 * (std::exp(-a * t) - std::exp(-l1 * t)) / (l1 - a);
  }

  const double ea = std::exp(-a * t);
  const double e1 = std::exp(-l1 * t);
  const double e2 = std::exp(-l2 * t);
  const double d_a1 = l1 - a;
  const double d_a2 = l2 - a;
  const double d_12 = l2 - l1;
  double N3 = 0.0;
  if (std::abs(d_a1) > 1e-30 && std::abs(d_a2) > 1e-30 && std::abs(d_12) > 1e-30) {
    N3 = a * l1 * N0 *
         (ea / (d_a1 * d_a2) + e1 / ((a - l1) * d_12) + e2 / ((a - l2) * (l1 - l2)));
  }

  double N4 = 0.0;
  if (std::abs(d_a1) > 1e-30 && std::abs(d_a2) > 1e-30 && std::abs(d_12) > 1e-30) {
    auto denom = [&](double lk, double lother1, double lother2) {
      return (lother1 - lk) * (lother2 - lk);
    };
    const double A_a = 1.0 / denom(a, l1, l2);
    const double A_1 = 1.0 / denom(l1, a, l2);
    const double A_2 = 1.0 / denom(l2, a, l1);
    N4 = a * l1 * l2 * N0 *
         (A_a * (1.0 - ea) / a + A_1 * (1.0 - e1) / l1 + A_2 * (1.0 - e2) / l2);
  }

  out.Th233 = clamp_nonneg(N2);
  out.Pa233 = clamp_nonneg(N3);
  out.U233 = clamp_nonneg(N4);
  return out;
}

}  // namespace reactorlab::fuelcycle
