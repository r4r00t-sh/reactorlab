"""Coupled plant reference: kinetics + Xe/Sm + lumped thermal + feedback.

Theory: docs/theory/coupled-plant-phase2.md
Provenance: data/PROVENANCE.md

Units: t [s], P [-], T [K], rho [Delta-k/k], concentrations [1/cm^3].
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

import numpy as np
from scipy.integrate import solve_ivp

from reference.point_kinetics import KineticsParams

# --- Documented defaults (data/PROVENANCE.md) ---
LAMBDA_I_POISON = 2.87e-5  # 1/s
LAMBDA_XE = 2.09e-5  # 1/s
GAMMA_I = 0.0639
GAMMA_XE = 0.00237
SIGMA_A_XE = 2.65e-18  # cm^2 (2.65e6 barn)

LAMBDA_PM = 3.63e-6  # 1/s
GAMMA_PM = 0.0113
SIGMA_A_SM = 4.1e-20  # cm^2 (4.1e4 barn)

NU = 2.43
PHI0 = 5.0e13  # n/cm^2/s
SIGMA_F0 = 0.1  # 1/cm
F0 = SIGMA_F0 * PHI0  # fissions/cm^3/s at P=1

ALPHA_D = -2.5e-5  # 1/K
ALPHA_M = -2.0e-5  # 1/K
ROD_WORTH = 0.05  # Delta-k/k fully inserted

TF0 = 900.0  # K
TC0 = 580.0  # K
TIN = 565.0  # K
# Heat balance at P=1: Q0 = H(Tf0-Tc0), W(Tc0-Tin)=H(Tf0-Tc0)
H_HT = 1.5e7  # W/K (educational)
Q0 = H_HT * (TF0 - TC0)  # W at P=1
W_FLOW = H_HT * (TF0 - TC0) / (TC0 - TIN)  # W/K
CF = 2.0e7  # J/K
CC = 5.0e7  # J/K

N_STATE = 13  # P, C[6], I, X, Pm, Sm, Tf, Tc


@dataclass
class PlantParams:
    kinetics: KineticsParams = field(default_factory=KineticsParams)
    lambda_I: float = LAMBDA_I_POISON
    lambda_Xe: float = LAMBDA_XE
    gamma_I: float = GAMMA_I
    gamma_Xe: float = GAMMA_XE
    sigma_a_Xe: float = SIGMA_A_XE
    lambda_Pm: float = LAMBDA_PM
    gamma_Pm: float = GAMMA_PM
    sigma_a_Sm: float = SIGMA_A_SM
    nu: float = NU
    phi0: float = PHI0
    sigma_f0: float = SIGMA_F0
    alpha_D: float = ALPHA_D
    alpha_M: float = ALPHA_M
    rod_worth: float = ROD_WORTH
    Tf0: float = TF0
    Tc0: float = TC0
    Tin: float = TIN
    H: float = H_HT
    Q0: float = Q0
    W: float = W_FLOW
    Cf: float = CF
    Cc: float = CC

    @property
    def F0(self) -> float:
        return self.sigma_f0 * self.phi0


def equilibrium_poisons(P: float, params: PlantParams) -> tuple[float, float, float, float]:
    """Steady I, Xe, Pm, Sm at constant relative power P (>=0)."""
    F = params.F0 * max(P, 0.0)
    phi = params.phi0 * max(P, 0.0)
    I = params.gamma_I * F / params.lambda_I if params.lambda_I > 0 else 0.0
    denom_x = params.lambda_Xe + params.sigma_a_Xe * phi
    X = (params.gamma_Xe * F + params.lambda_I * I) / denom_x if denom_x > 0 else 0.0
    Pm = params.gamma_Pm * F / params.lambda_Pm if params.lambda_Pm > 0 else 0.0
    denom_s = params.sigma_a_Sm * phi
    Sm = (params.lambda_Pm * Pm) / denom_s if denom_s > 0 else 0.0
    return I, X, Pm, Sm


def equilibrium_xenon_reactivity(P: float, params: PlantParams | None = None) -> float:
    """rho_Xe at equilibrium for constant P (high-flux limit ~ -(gI+gX)/nu)."""
    params = params or PlantParams()
    _I, X, _Pm, _Sm = equilibrium_poisons(P, params)
    return -params.sigma_a_Xe * X / (params.nu * params.sigma_f0)


def poison_reactivity(X: float, Sm: float, params: PlantParams) -> tuple[float, float]:
    rho_xe = -params.sigma_a_Xe * X / (params.nu * params.sigma_f0)
    rho_sm = -params.sigma_a_Sm * Sm / (params.nu * params.sigma_f0)
    return rho_xe, rho_sm


def critical_bias(params: PlantParams) -> float:
    """Positive excess that cancels equilibrium Xe+Sm at P=1, Tf0/Tc0, rods out.

    Stands in for critical boron / fuel excess in this educational model.
    """
    _I, X, _Pm, Sm = equilibrium_poisons(1.0, params)
    rho_xe, rho_sm = poison_reactivity(X, Sm, params)
    return -(rho_xe + rho_sm)


def total_reactivity(
    t: float,
    y: np.ndarray,
    params: PlantParams,
    rho_ext_fn: Callable[[float], float],
    rod_fn: Callable[[float], float],
) -> float:
    """rod_fn returns insertion z in [0,1]."""
    Tf = y[11]
    Tc = y[12]
    X = y[8]
    Sm = y[10]
    rho_xe, rho_sm = poison_reactivity(X, Sm, params)
    z = float(np.clip(rod_fn(t), 0.0, 1.0))
    return (
        critical_bias(params)
        + rho_ext_fn(t)
        - params.rod_worth * z
        + params.alpha_D * (Tf - params.Tf0)
        + params.alpha_M * (Tc - params.Tc0)
        + rho_xe
        + rho_sm
    )


def make_equilibrium_state(P0: float, params: PlantParams | None = None) -> np.ndarray:
    params = params or PlantParams()
    y = np.zeros(N_STATE, dtype=float)
    y[0] = P0
    for i in range(6):
        y[1 + i] = params.kinetics.beta_i[i] / (params.kinetics.lambda_i[i] * params.kinetics.Lambda) * P0
    I, X, Pm, Sm = equilibrium_poisons(P0, params)
    y[7] = I
    y[8] = X
    y[9] = Pm
    y[10] = Sm
    y[11] = params.Tf0
    y[12] = params.Tc0
    return y


def rhs(
    t: float,
    y: np.ndarray,
    params: PlantParams,
    rho_ext_fn: Callable[[float], float],
    rod_fn: Callable[[float], float],
) -> np.ndarray:
    P = max(y[0], 0.0)
    C = y[1:7]
    I, X, Pm, Sm = y[7], y[8], y[9], y[10]
    Tf, Tc = y[11], y[12]
    kin = params.kinetics
    beta = kin.beta
    rho = total_reactivity(t, y, params, rho_ext_fn, rod_fn)

    out = np.zeros(N_STATE, dtype=float)
    out[0] = ((rho - beta) / kin.Lambda) * P + float(np.dot(kin.lambda_i, C))
    out[1:7] = (kin.beta_i / kin.Lambda) * P - kin.lambda_i * C

    F = params.F0 * P
    phi = params.phi0 * P
    out[7] = params.gamma_I * F - params.lambda_I * I
    out[8] = params.gamma_Xe * F + params.lambda_I * I - params.lambda_Xe * X - params.sigma_a_Xe * phi * X
    out[9] = params.gamma_Pm * F - params.lambda_Pm * Pm
    out[10] = params.lambda_Pm * Pm - params.sigma_a_Sm * phi * Sm

    out[11] = (P * params.Q0 - params.H * (Tf - Tc)) / params.Cf
    out[12] = (params.H * (Tf - Tc) - params.W * (Tc - params.Tin)) / params.Cc
    return out


def integrate_plant(
    t_span: tuple[float, float],
    rho_ext_fn: Callable[[float], float],
    rod_fn: Callable[[float], float] | None = None,
    params: PlantParams | None = None,
    y0: np.ndarray | None = None,
    t_eval: np.ndarray | None = None,
    rtol: float = 1e-7,
    atol: float = 1e-9,
) -> tuple[np.ndarray, np.ndarray]:
    params = params or PlantParams()
    rod_fn = rod_fn or (lambda _t: 0.0)
    y0 = make_equilibrium_state(1.0, params) if y0 is None else np.asarray(y0, dtype=float)
    sol = solve_ivp(
        fun=lambda t, y: rhs(t, y, params, rho_ext_fn, rod_fn),
        t_span=t_span,
        y0=y0,
        method="BDF",
        t_eval=t_eval,
        rtol=rtol,
        atol=atol,
    )
    if not sol.success:
        raise RuntimeError(sol.message)
    return sol.t, sol.y
