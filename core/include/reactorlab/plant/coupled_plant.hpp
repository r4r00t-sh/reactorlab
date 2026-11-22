#pragma once

#include "reactorlab/kinetics/reactivity.hpp"
#include "reactorlab/plant/params.hpp"

#include <array>
#include <functional>
#include <stdexcept>
#include <vector>

namespace reactorlab::plant {

inline constexpr std::size_t kPlantStateSize = 13;

struct PlantState {
  double t{0.0};
  double P{1.0};
  std::array<double, kinetics::kNumDelayedGroups> C{};
  double I{0.0};    // 1/cm^3
  double X{0.0};    // 1/cm^3
  double Pm{0.0};   // 1/cm^3
  double Sm{0.0};   // 1/cm^3
  double Tf{900.0}; // K
  double Tc{580.0}; // K

  [[nodiscard]] std::array<double, kPlantStateSize> to_array() const;
  static PlantState from_array(double t, const std::array<double, kPlantStateSize>& y);
};

struct PlantTrajectory {
  std::vector<double> t;
  std::vector<double> P;
  std::vector<double> X;
  std::vector<double> Tf;
  std::vector<double> Tc;
  std::vector<double> rho_total;
};

struct SolverTolerances {
  double rtol{1.0e-7};
  double atol{1.0e-9};
};

using RodFn = std::function<double(double t_s)>; // insertion z in [0,1]

void equilibrium_poisons(
    double P, const PlantParams& params, double& I, double& X, double& Pm, double& Sm);

[[nodiscard]] double equilibrium_xenon_reactivity(double P, const PlantParams& params);

[[nodiscard]] double critical_bias(const PlantParams& params);

[[nodiscard]] double total_reactivity(double t,
                                      const PlantState& s,
                                      const PlantParams& params,
                                      const kinetics::ReactivityFn& rho_ext,
                                      const RodFn& rod);

[[nodiscard]] PlantState make_equilibrium_state(double P0, const PlantParams& params);

[[nodiscard]] PlantState advance(const PlantParams& params,
                                 const kinetics::ReactivityFn& rho_ext,
                                 const RodFn& rod,
                                 const PlantState& s0,
                                 double t1,
                                 SolverTolerances tol = {});

[[nodiscard]] PlantTrajectory integrate(const PlantParams& params,
                                        const kinetics::ReactivityFn& rho_ext,
                                        const RodFn& rod,
                                        const std::vector<double>& t_out,
                                        const PlantState& s0,
                                        SolverTolerances tol = {});

class PlantError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace reactorlab::plant
