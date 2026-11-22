"""Keepin U-233 thermal delayed-neutron set (educational).

Primary: Keepin, Wimett, Zeigler, Phys. Rev. 107, 1044 (1957).
Definitions: docs/theory/delayed-neutrons-beta.md
Provenance: data/PROVENANCE.md

β = Σ βᵢ ≈ 0.002778; ν_d = 0.0070; ν = 2.52; β ≈ ν_d/ν.
"""

from __future__ import annotations

import numpy as np

from reference.point_kinetics import BETA_I as U235_BETA_I
from reference.point_kinetics import KineticsParams

_LN2 = float(np.log(2.0))
_T_HALF = np.array([55.11, 20.74, 5.30, 2.29, 0.546, 0.221], dtype=float)
LAMBDA_I_U233 = _LN2 / _T_HALF
BETA_I_U233 = np.array(
    [0.000238, 0.000762, 0.000631, 0.000881, 0.000202, 0.000064], dtype=float
)
BETA_U233 = float(np.sum(BETA_I_U233))
NU_U233 = 2.52
NU_D_U233 = 0.0070


def u233_thermal_keepin() -> KineticsParams:
    return KineticsParams(lambda_i=LAMBDA_I_U233.copy(), beta_i=BETA_I_U233.copy(), Lambda=1.0e-4)


def dollars(rho_dk: float, beta_eff: float) -> float:
    if beta_eff <= 0.0:
        return 0.0
    return rho_dk / beta_eff


def fission_weighted_beta_eff(rates: np.ndarray, betas: np.ndarray) -> float:
    rates = np.asarray(rates, dtype=float)
    betas = np.asarray(betas, dtype=float)
    den = float(np.sum(np.maximum(rates, 0.0)))
    if den <= 0.0:
        return 0.0
    return float(np.sum(np.maximum(rates, 0.0) * betas) / den)


assert abs(BETA_U233 - NU_D_U233 / NU_U233) < 5e-5
assert BETA_U233 < 0.5 * float(np.sum(U235_BETA_I))
