#pragma once

#include <cmath>
#include <functional>
#include <numbers>

namespace reactorlab::kinetics {

/// Reactivity schedule ρ(t) in absolute Δk/k (not dollars).
using ReactivityFn = std::function<double(double t_s)>;

[[nodiscard]] inline ReactivityFn step_reactivity(double rho0) {
  return [rho0](double /*t*/) { return rho0; };
}

[[nodiscard]] inline ReactivityFn ramp_reactivity(double rate_per_s, double rho_cap) {
  return [rate_per_s, rho_cap](double t) {
    const double value = rate_per_s * t;
    if (rate_per_s >= 0.0) {
      return value < rho_cap ? value : rho_cap;
    }
    return value > rho_cap ? value : rho_cap;
  };
}

[[nodiscard]] inline ReactivityFn sinusoidal_reactivity(double amplitude, double frequency_hz) {
  const double omega = 2.0 * std::numbers::pi * frequency_hz;
  return [amplitude, omega](double t) { return amplitude * std::sin(omega * t); };
}

[[nodiscard]] inline ReactivityFn
scram_reactivity(double rho_before, double rho_scram, double t_scram_s) {
  return [rho_before, rho_scram, t_scram_s](double t) {
    return t < t_scram_s ? rho_before : rho_scram;
  };
}

} // namespace reactorlab::kinetics
