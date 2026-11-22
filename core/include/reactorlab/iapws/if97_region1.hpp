#pragma once

#include <stdexcept>

namespace reactorlab::iapws {

/// IAPWS-IF97 Region 1 (liquid) properties.
/// Coefficients: IAPWS Revised Release on IF97 (R7-97), Table 2.
/// Units: p [Pa], T [K], returns v [m³/kg], rho [kg/m³], cp [J/(kg·K)].

struct Region1Properties {
  double v{0.0};    // m^3/kg
  double rho{0.0};  // kg/m^3
  double cp{0.0};   // J/(kg·K)
};

/// Evaluate Region 1 at absolute pressure p [Pa] and temperature T [K].
[[nodiscard]] Region1Properties region1(double p_pa, double T_K);

/// Convenience aliases.
[[nodiscard]] inline double density(double p_pa, double T_K) {
  return region1(p_pa, T_K).rho;
}
[[nodiscard]] inline double cp(double p_pa, double T_K) {
  return region1(p_pa, T_K).cp;
}

class IapwsError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}  // namespace reactorlab::iapws
