"""IAPWS-IF97 Region 1 (liquid) — educational subset.

Coefficients: IAPWS Revised Release on IF97 (R7-97), Table 2.
Verification: Table 5 sample (3 MPa, 300 K) → v = 1.00215168e-3 m³/kg.

Units: p [Pa], T [K]; returns v [m³/kg], rho [kg/m³], cp [J/(kg·K)].
"""

from __future__ import annotations

import math
from dataclasses import dataclass

# Table 2: I_i, J_i, n_i
_I = (
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29, 30, 31, 32,
)
_J = (
    -2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3, 0, 1,
    3, 17, -4, 0, 6, -5, -2, 10, -8, -11, -6, -29, -31, -38, -39, -40, -41,
)
_N = (
    0.14632971213167,
    -0.84548187169114,
    -3.7563603672040,
    3.3855169168385,
    -0.95791963387872,
    0.15772038513228,
    -0.016616417199501,
    0.00081214629983568,
    0.00028319080123804,
    -0.00060706301565874,
    -0.018990068218419,
    -0.032529748770505,
    -0.021841717175414,
    -0.000052838357969930,
    -0.00047184321073267,
    -0.00030001780793026,
    0.000047661393906987,
    -0.0000044141845330846,
    -7.2694996297594e-16,
    -0.000031679644845054,
    -0.0000028270797985312,
    -8.5205128120103e-10,
    -0.0000022425281908000,
    -6.5171222895601e-07,
    -1.4341729937924e-13,
    -4.0516996860117e-07,
    -1.2734301741641e-09,
    -1.7424871230634e-10,
    -6.8762131295531e-19,
    1.4478307828521e-20,
    2.6335781662795e-23,
    -1.1947622640071e-23,
    1.8228094581404e-24,
    -9.3537087292458e-26,
)

_R = 0.461526  # kJ/(kg·K)
_PSTAR = 16.53  # MPa
_TSTAR = 1386.0  # K

# Published check (IAPWS Table 5)
SAMPLE_V_3MPA_300K = 1.00215168e-3  # m³/kg


@dataclass(frozen=True)
class Region1Properties:
    v: float  # m³/kg
    rho: float  # kg/m³
    cp: float  # J/(kg·K)


def region1(p_pa: float, T_K: float) -> Region1Properties:
    """Specific volume, density, and cp for Region 1 liquid water."""
    if p_pa <= 0.0:
        raise ValueError("IF97 Region 1: invalid pressure")
    # Educational clamp to Region 1 formal range (273.15–623.15 K).
    if T_K < 273.15:
        T_K = 273.15
    elif T_K > 623.15:
        T_K = 623.15
    p_MPa = p_pa * 1.0e-6
    pi = p_MPa / _PSTAR
    tau = _TSTAR / T_K
    g71 = 7.1 - pi
    gt = tau - 1.222
    gamma_pi = 0.0
    gamma_tautau = 0.0
    for Ii, Ji, ni in zip(_I, _J, _N, strict=True):
        if Ii != 0:
            gamma_pi += -ni * Ii * (g71 ** (Ii - 1)) * (gt**Ji)
        if Ji != 0 and Ji != 1:
            gamma_tautau += ni * (g71**Ii) * Ji * (Ji - 1) * (gt ** (Ji - 2))
    v = pi * gamma_pi * _R * T_K / p_MPa / 1000.0
    cp_kj = -_R * tau * tau * gamma_tautau
    return Region1Properties(v=v, rho=1.0 / v, cp=cp_kj * 1000.0)


def density(p_pa: float, T_K: float) -> float:
    return region1(p_pa, T_K).rho


def cp(p_pa: float, T_K: float) -> float:
    return region1(p_pa, T_K).cp
