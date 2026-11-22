#pragma once

#include "reactorlab/kinetics/delayed_groups.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace reactorlab::fuelcycle {

/// Heavy-metal nuclides tracked in the Th-chain inventory [atoms/cm³].
struct ChainState {
  double Th232{0.0};
  double Th233{0.0};
  double Pa233{0.0};
  double U233{0.0};
  double U234{0.0};
  double U235{0.0};
  double U236{0.0};
  double Pu239{0.0};
  double fissioned{0.0};  // cumulative atoms removed by fission (bookkeeping sink)

  [[nodiscard]] double heavy_metal_atoms() const noexcept {
    return Th232 + Th233 + Pa233 + U233 + U234 + U235 + U236 + Pu239;
  }

  [[nodiscard]] double accounted_atoms() const noexcept {
    return heavy_metal_atoms() + fissioned;
  }
};

struct ChainParams {
  // Decay constants [1/s] — from IAEA/ENSDF (not placeholders)
  double lambda_Th233{0.0};
  double lambda_Pa233{0.0};
  double lambda_U233{0.0};
  double lambda_U234{0.0};
  double lambda_U235{0.0};
  double lambda_U236{0.0};

  // Microscopic cross sections [cm²]
  double sigma_c_Th232{0.0};
  double sigma_c_Pa233{0.0};
  double sigma_f_U233{0.0};
  double sigma_c_U233{0.0};
  double sigma_f_U235{0.0};
  double sigma_c_U235{0.0};
  double sigma_f_Pu239{0.0};
  double sigma_c_Pu239{0.0};

  double nu_U233{2.52};
  double nu_U235{2.43};
  double nu_Pu239{2.88};
  double phi0{5.0e13};  // n/cm²/s at P=1

  /// Macroscopic fission scale used to normalize inventory reactivity [1/cm].
  double sigma_f0_norm{0.1};

  bool has_placeholders{true};
  std::vector<std::string> placeholder_ids{};
  std::string xs_provenance{"unspecified"};

  kinetics::DelayedGroupData delayed_U233{kinetics::u233_thermal_keepin()};
  kinetics::DelayedGroupData delayed_U235{kinetics::u235_thermal_keepin()};
  kinetics::DelayedGroupData delayed_Pu239{kinetics::pu239_fast_keepin()};
};

/// Transient: plant ODE + chain with same φ(P). Depletion: chain-focused larger steps.
enum class TimeMode { Transient, Depletion };

struct CouplingSettings {
  bool enabled{false};           // v1.1 quasi-static two-way toggle
  double update_period_s{1.0};   // how often to refresh ρ_inv / β_eff
  TimeMode time_mode{TimeMode::Transient};
  /// Depletion mode: multiply chain dt relative to plant dt (plant may freeze).
  double depletion_chain_dt_scale{100.0};
};

/// Load educational AHWR-style defaults (YAML-backed constants mirrored in code).
[[nodiscard]] ChainParams ahwr_edu_v1_params();

[[nodiscard]] ChainState ahwr_edu_v1_initial_state();

/// Advance chain by dt [s] at relative power P (φ = φ0·P).
void advance_chain(ChainState& s, const ChainParams& p, double P, double dt_s);

/// Analytic Bateman solution for Th232→Th233→Pa233→U233 at constant φ,
/// neglecting Pa capture and all fission (validation path only).
[[nodiscard]] ChainState bateman_th_pa_u_constant_flux(double N_Th232_0,
                                                       double phi,
                                                       double t,
                                                       const ChainParams& p);

struct FissionShares {
  double F_U233{0.0};
  double F_U235{0.0};
  double F_Pu239{0.0};
  double beta_eff{0.0};
};

[[nodiscard]] FissionShares fission_shares(const ChainState& s,
                                           const ChainParams& p,
                                           double P);

/// Neutron-economy proxy: Σ Nᵢ(νᵢσ_f,ᵢ − σ_a,ᵢ) / (ν_ref Σ_f0).
/// Pa capture is a pure sink; U-233 fission adds (ν−1)σ_f − σ_c.
[[nodiscard]] double inventory_reactivity(const ChainState& s,
                                          const ChainParams& p) noexcept;

/// Δρ = inventory_reactivity(s) − inventory_reactivity(ref).
[[nodiscard]] double inventory_reactivity_bias(const ChainState& s,
                                               const ChainState& ref,
                                               const ChainParams& p) noexcept;

/// Fission-rate weighted delayed-group set (βᵢ, λᵢ, Λ, ν) for kinetics ODE.
[[nodiscard]] kinetics::DelayedGroupData fission_weighted_delayed(
    const ChainState& s, const ChainParams& p, double P);

/// Apply quasi-static coupling outputs: inventory ρ bias + β_eff-scaled kinetics.
struct CouplingOutputs {
  double rho_inventory{0.0};
  double beta_eff{0.0};
  kinetics::DelayedGroupData delayed{};
};

[[nodiscard]] CouplingOutputs evaluate_coupling(const ChainState& s,
                                                const ChainState& ref,
                                                const ChainParams& p,
                                                double P);

class FuelcycleError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}  // namespace reactorlab::fuelcycle
