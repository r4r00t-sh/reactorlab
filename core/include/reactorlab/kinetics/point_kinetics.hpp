#pragma once

#include "reactorlab/kinetics/delayed_groups.hpp"
#include "reactorlab/kinetics/reactivity.hpp"

#include <array>
#include <stdexcept>
#include <vector>

namespace reactorlab::kinetics {

inline constexpr std::size_t kStateSize = 1 + kNumDelayedGroups; // P + C_i

struct Trajectory {
  std::vector<double> t;                                // [s]
  std::vector<double> P;                                // relative power [-]
  std::vector<std::array<double, kNumDelayedGroups>> C; // precursor densities
};

struct State {
  double t{0.0}; // [s]
  double P{1.0}; // relative power [-]
  std::array<double, kNumDelayedGroups> C{};
};

struct SolverTolerances {
  double rtol{1.0e-8};
  double atol{1.0e-10};
};

/// Equilibrium precursors at ρ=0: C_i = β_i P0 / (λ_i Λ).
[[nodiscard]] std::array<double, kNumDelayedGroups>
equilibrium_precursors(double P0, const DelayedGroupData& data);

/// Analytic prompt-jump ratio β/(β-ρ) for 0 < ρ < β.
[[nodiscard]] double prompt_jump_ratio(double rho, const DelayedGroupData& data);

/// Inhour equation ρ(ω).
[[nodiscard]] double inhour_reactivity(double omega, const DelayedGroupData& data);

/// Dominant inhour root ω for given ρ.
[[nodiscard]] double solve_inhour_omega(double rho, const DelayedGroupData& data);

/// Integrate six-group point kinetics with SUNDIALS CVODE (BDF).
/// @param t_out Output sample times [s], must be non-decreasing and t_out.front() >= t0.
/// @param rho Reactivity schedule [Δk/k].
/// Units: see docs/theory/point-kinetics.md
[[nodiscard]] Trajectory integrate(const DelayedGroupData& data,
                                   const ReactivityFn& rho,
                                   const std::vector<double>& t_out,
                                   double P0 = 1.0,
                                   double t0 = 0.0,
                                   SolverTolerances tol = {});

/// Advance from an arbitrary state to time t1 (CVODE). Used by the GUI stepper.
[[nodiscard]] State advance(const DelayedGroupData& data,
                            const ReactivityFn& rho,
                            const State& s0,
                            double t1,
                            SolverTolerances tol = {});

/// Equilibrium initial state at t=0, ρ=0.
[[nodiscard]] State make_equilibrium_state(double P0, const DelayedGroupData& data);

class KineticsError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace reactorlab::kinetics
