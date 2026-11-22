#pragma once

#include "reactorlab/kinetics/delayed_groups.hpp"

namespace reactorlab::plant {

/// Educational coupled-plant parameters. Units documented per field; see data/PROVENANCE.md.
struct PlantParams {
  kinetics::DelayedGroupData kinetics{kinetics::u235_thermal_keepin()};

  // I-135 / Xe-135
  double lambda_I{2.87e-5};    // 1/s
  double lambda_Xe{2.09e-5};   // 1/s
  double gamma_I{0.0639};      // -
  double gamma_Xe{0.00237};    // -
  double sigma_a_Xe{2.65e-18}; // cm^2

  // Pm-149 / Sm-149
  double lambda_Pm{3.63e-6};  // 1/s
  double gamma_Pm{0.0113};    // -
  double sigma_a_Sm{4.1e-20}; // cm^2

  double nu{2.43};      // -
  double phi0{5.0e13};  // n/cm^2/s
  double sigma_f0{0.1}; // 1/cm

  double alpha_D{-2.5e-5}; // 1/K
  double alpha_M{-2.0e-5}; // 1/K
  double rod_worth{0.05};  // Δk/k at full insertion

  double Tf0{900.0}; // K
  double Tc0{580.0}; // K
  double Tin{565.0}; // K
  double H{1.5e7};   // W/K
  double Cf{2.0e7};  // J/K
  double Cc{5.0e7};  // J/K

  [[nodiscard]] double F0() const noexcept { return sigma_f0 * phi0; }
  [[nodiscard]] double Q0() const noexcept { return H * (Tf0 - Tc0); }
  [[nodiscard]] double W() const noexcept { return H * (Tf0 - Tc0) / (Tc0 - Tin); }
};

} // namespace reactorlab::plant
