#include "reactorlab/iapws/if97_region1.hpp"

#include <array>
#include <cmath>

namespace reactorlab::iapws {
namespace {

// IAPWS R7-97 Table 2: I_i, J_i, n_i
constexpr int kN = 34;
constexpr std::array<int, kN> kI = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29, 30, 31, 32};
constexpr std::array<int, kN> kJ = {
    -2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3, 0, 1,
    3, 17, -4, 0, 6, -5, -2, 10, -8, -11, -6, -29, -31, -38, -39, -40, -41};
constexpr std::array<double, kN> kNcoef = {
    0.14632971213167,     -0.84548187169114,    -3.7563603672040,     3.3855169168385,
    -0.95791963387872,    0.15772038513228,     -0.016616417199501,   0.00081214629983568,
    0.00028319080123804,  -0.00060706301565874, -0.018990068218419,   -0.032529748770505,
    -0.021841717175414,   -0.000052838357969930, -0.00047184321073267, -0.00030001780793026,
    0.000047661393906987, -0.0000044141845330846, -7.2694996297594e-16, -0.000031679644845054,
    -0.0000028270797985312, -8.5205128120103e-10, -0.0000022425281908000, -6.5171222895601e-07,
    -1.4341729937924e-13, -4.0516996860117e-07, -1.2734301741641e-09, -1.7424871230634e-10,
    -6.8762131295531e-19, 1.4478307828521e-20,  2.6335781662795e-23,  -1.1947622640071e-23,
    1.8228094581404e-24,  -9.3537087292458e-26};

constexpr double kR = 0.461526;      // kJ/(kg·K)
constexpr double kPstar = 16.53;     // MPa
constexpr double kTstar = 1386.0;    // K

}  // namespace

Region1Properties region1(double p_pa, double T_K) {
  if (!(p_pa > 0.0)) {
    throw IapwsError("IF97 Region 1: invalid pressure");
  }
  // Educational clamp: Region 1 formal range is 273.15–623.15 K.
  // Outside that, evaluate at the nearest valid T (single-phase liquid model).
  if (!(T_K >= 273.15 && T_K <= 623.15)) {
    if (T_K < 273.15) {
      T_K = 273.15;
    } else if (T_K > 623.15) {
      T_K = 623.15;
    } else {
      throw IapwsError("IF97 Region 1: invalid temperature");
    }
  }
  const double p_MPa = p_pa * 1.0e-6;
  const double pi = p_MPa / kPstar;
  const double tau = kTstar / T_K;
  const double g71 = 7.1 - pi;
  const double gt = tau - 1.222;

  double gamma_pi = 0.0;
  double gamma_tautau = 0.0;
  for (int i = 0; i < kN; ++i) {
    const double Ii = static_cast<double>(kI[static_cast<std::size_t>(i)]);
    const double Ji = static_cast<double>(kJ[static_cast<std::size_t>(i)]);
    const double ni = kNcoef[static_cast<std::size_t>(i)];
    if (Ii != 0.0) {
      gamma_pi += -ni * Ii * std::pow(g71, Ii - 1.0) * std::pow(gt, Ji);
    }
    if (Ji != 0.0 && Ji != 1.0) {
      gamma_tautau += ni * std::pow(g71, Ii) * Ji * (Ji - 1.0) * std::pow(gt, Ji - 2.0);
    }
  }

  // v = π * γ_π * R * T / p   with R in kJ/(kg·K), p in MPa → m³/kg
  // R*T has units kJ/kg = kPa·m³/kg; divide by p_MPa*1000 to get m³/kg:
  // Actually: (π γ_π) * R * T / p_MPa  with R in kJ/kg/K gives (kJ/kg)/MPa = m³/kg
  // since 1 kJ/MPa = 0.001 m³? Wait: 1 kJ = 10^3 J, 1 MPa = 10^6 Pa, J/Pa = m³
  // so kJ/MPa = 10^3/10^6 m³ = 0.001 m³. The iapws formula uses:
  // v = pi * gp * R * T / P / 1000  with P in MPa, R in kJ/kg/K → divide by 1000.
  const double v = pi * gamma_pi * kR * T_K / p_MPa / 1000.0;
  // cp = -R τ² γ_ττ   (kJ/kg/K) → J/kg/K
  const double cp_kj = -kR * tau * tau * gamma_tautau;

  Region1Properties out;
  out.v = v;
  out.rho = 1.0 / v;
  out.cp = cp_kj * 1000.0;
  return out;
}

}  // namespace reactorlab::iapws
