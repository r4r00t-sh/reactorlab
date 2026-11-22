#pragma once

#include "reactorlab/kinetics/reactivity.hpp"
#include "reactorlab/multiphysics/params.hpp"

#include <array>
#include <functional>
#include <stdexcept>
#include <vector>

namespace reactorlab::multiphysics {

inline constexpr std::size_t kNfStateSize = 8;  // P + C[6] + Tf

struct MultiState {
  double t{0.0};
  double P{1.0};
  std::array<double, kinetics::kNumDelayedGroups> C{};
  double Tf{900.0};
  std::vector<double> Tc;  // axial nodes [K]
  int picard_iters{0};
  double picard_res{0.0};

  [[nodiscard]] double Tc_bar() const;
  [[nodiscard]] double Tout() const;
};

struct MultiTrajectory {
  std::vector<double> t;
  std::vector<double> P;
  std::vector<double> Tf;
  std::vector<double> Tc_bar;
  std::vector<double> Tout;
  std::vector<double> rho_total;
  std::vector<double> mdot;
  std::vector<int> picard_iters;
};

struct SolverTolerances {
  double rtol{1.0e-7};
  double atol{1.0e-9};
};

using DriverFn = std::function<double(double t_s)>;

[[nodiscard]] double total_reactivity(double t,
                                      double P,
                                      double Tf,
                                      double Tc_bar,
                                      const MultiParams& params,
                                      const DriverFn& rho_ext,
                                      const DriverFn& rod_z);

/// Build equilibrium state; sets params.rho_bias so ρ≈0 at t=0 with rod_z0.
[[nodiscard]] MultiState make_equilibrium_state(double P0,
                                                MultiParams& params,
                                                double rod_z0 = 0.0);

[[nodiscard]] MultiState advance(const MultiParams& params,
                                 const DriverFn& rho_ext,
                                 const DriverFn& rod_z,
                                 const DriverFn& mdot,
                                 const MultiState& s0,
                                 double t1,
                                 SolverTolerances tol = {});

[[nodiscard]] MultiTrajectory integrate(const MultiParams& params,
                                        const DriverFn& rho_ext,
                                        const DriverFn& rod_z,
                                        const DriverFn& mdot,
                                        const std::vector<double>& t_out,
                                        const MultiState& s0,
                                        SolverTolerances tol = {});

/// Built-in scenario helpers: return (rho_ext, rod_z, mdot, rod_z0).
struct ScenarioDrivers {
  DriverFn rho_ext;
  DriverFn rod_z;
  DriverFn mdot;
  double rod_z0{0.0};
};

[[nodiscard]] ScenarioDrivers scenario_rod_ejection(double t_eject = 1.0,
                                                    double z0 = 0.08,
                                                    double duration = 0.1,
                                                    double mdot0 = 2.9e4);

[[nodiscard]] ScenarioDrivers scenario_loss_of_flow(double t_start = 5.0,
                                                    double fraction = 0.4,
                                                    double ramp = 10.0,
                                                    double mdot0 = 2.9e4);

[[nodiscard]] ScenarioDrivers scenario_load_follow(double period = 200.0,
                                                   double amp = 0.001,
                                                   double mdot0 = 2.9e4);

class MultiError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}  // namespace reactorlab::multiphysics
