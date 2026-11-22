#pragma once

#include "reactorlab/kinetics/delayed_groups.hpp"

#include <cstddef>

namespace reactorlab::multiphysics {

/// Educational multiphysics parameters (Phase 5). See docs/theory/multiphysics-coupling.md.
struct MultiParams {
  kinetics::DelayedGroupData kinetics{kinetics::u235_thermal_keepin()};

  double alpha_D{-2.5e-5};  // 1/K
  double alpha_M{-2.0e-5};  // 1/K
  double rod_worth{0.05};  // Δk/k at full insertion

  double Tf0{900.0};  // K
  double Tc0{580.0};  // K
  double Tin{565.0};  // K
  double H{1.5e7};    // W/K
  double Cf{2.0e7};   // J/K

  double p_sys{15.5e6};  // Pa
  double L{3.5};         // m
  double A{5.0};         // m²
  std::size_t n_nodes{10};
  double mdot0{2.9e4};  // kg/s

  double rho_bias{0.0};
  int picard_max{8};
  double picard_tol{1.0e-4};  // K

  [[nodiscard]] double Q0() const noexcept { return H * (Tf0 - Tc0); }
  [[nodiscard]] double dz() const noexcept {
    return L / static_cast<double>(n_nodes);
  }
};

}  // namespace reactorlab::multiphysics
