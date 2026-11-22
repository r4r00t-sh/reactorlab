"""Six-group point kinetics reference (educational).

Equations and assumptions: docs/theory/point-kinetics.md
Delayed-neutron data provenance: data/PROVENANCE.md (Keepin U-235 thermal).

Units: t [s], P relative [-], rho [Delta-k/k], Lambda [s], lambda_i [1/s], beta_i [-].
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

import numpy as np
from scipy.integrate import solve_ivp

# Keepin, G.R., Physics of Nuclear Kinetics (1965) — U-235 thermal six-group set.
LAMBDA_I = np.array([0.0124, 0.0305, 0.111, 0.301, 1.14, 3.01], dtype=float)  # 1/s
BETA_I = np.array(
    [0.000215, 0.001424, 0.001274, 0.002568, 0.000748, 0.000273], dtype=float
)
BETA = float(np.sum(BETA_I))  # 0.006502
DEFAULT_LAMBDA_PROMPT = 1.0e-4  # s (educational thermal default)


@dataclass(frozen=True)
class KineticsParams:
    """Point-kinetics parameters (SI)."""

    lambda_i: np.ndarray = field(default_factory=lambda: LAMBDA_I.copy())
    beta_i: np.ndarray = field(default_factory=lambda: BETA_I.copy())
    Lambda: float = DEFAULT_LAMBDA_PROMPT

    def __post_init__(self) -> None:
        object.__setattr__(self, "lambda_i", np.asarray(self.lambda_i, dtype=float))
        object.__setattr__(self, "beta_i", np.asarray(self.beta_i, dtype=float))
        if self.lambda_i.shape != (6,) or self.beta_i.shape != (6,):
            raise ValueError("expected six delayed groups")

    @property
    def beta(self) -> float:
        return float(np.sum(self.beta_i))


def equilibrium_precursors(P0: float, params: KineticsParams) -> np.ndarray:
    """C_i(0) for rho=0 steady state. Units: same relative units as P."""
    return params.beta_i / (params.lambda_i * params.Lambda) * P0


def rhs(
    t: float,
    y: np.ndarray,
    params: KineticsParams,
    rho_fn: Callable[[float], float],
) -> np.ndarray:
    P = y[0]
    C = y[1:]
    rho = rho_fn(t)
    beta = params.beta
    dP = ((rho - beta) / params.Lambda) * P + float(np.dot(params.lambda_i, C))
    dC = (params.beta_i / params.Lambda) * P - params.lambda_i * C
    out = np.empty(7, dtype=float)
    out[0] = dP
    out[1:] = dC
    return out


def step_reactivity(rho0: float) -> Callable[[float], float]:
    return lambda _t: rho0


def ramp_reactivity(rate: float, rho_max: float | None = None) -> Callable[[float], float]:
    """rho(t) = rate * t, optionally capped at rho_max."""

    def rho(t: float) -> float:
        value = rate * t
        if rho_max is None:
            return value
        if rate >= 0.0:
            return min(value, rho_max)
        return max(value, rho_max)

    return rho


def sinusoidal_reactivity(amplitude: float, frequency_hz: float) -> Callable[[float], float]:
    omega = 2.0 * np.pi * frequency_hz
    return lambda t: amplitude * float(np.sin(omega * t))


def scram_reactivity(
    rho_before: float, rho_scram: float, t_scram: float
) -> Callable[[float], float]:
    def rho(t: float) -> float:
        return rho_before if t < t_scram else rho_scram

    return rho


def integrate(
    t_span: tuple[float, float],
    rho_fn: Callable[[float], float],
    params: KineticsParams | None = None,
    P0: float = 1.0,
    t_eval: np.ndarray | None = None,
    rtol: float = 1e-8,
    atol: float = 1e-10,
) -> tuple[np.ndarray, np.ndarray]:
    """Integrate point kinetics. Returns (t, y) with y shape (7, n)."""
    params = params or KineticsParams()
    y0 = np.empty(7, dtype=float)
    y0[0] = P0
    y0[1:] = equilibrium_precursors(P0, params)

    sol = solve_ivp(
        fun=lambda t, y: rhs(t, y, params, rho_fn),
        t_span=t_span,
        y0=y0,
        method="BDF",
        t_eval=t_eval,
        rtol=rtol,
        atol=atol,
        dense_output=False,
    )
    if not sol.success:
        raise RuntimeError(f"solve_ivp failed: {sol.message}")
    return sol.t, sol.y


def prompt_jump_ratio(rho: float, params: KineticsParams | None = None) -> float:
    """Analytic prompt-jump P(0+)/P(0-) for 0 < rho < beta."""
    params = params or KineticsParams()
    if not (0.0 < rho < params.beta):
        raise ValueError("prompt jump formula requires 0 < rho < beta")
    return params.beta / (params.beta - rho)


def inhour_reactivity(omega: float, params: KineticsParams | None = None) -> float:
    """Inhour equation: rho(omega) for asymptotic e^{omega t}."""
    params = params or KineticsParams()
    return float(
        params.Lambda * omega + np.sum(params.beta_i * omega / (omega + params.lambda_i))
    )


def solve_inhour_omega(rho: float, params: KineticsParams | None = None) -> float:
    """Find the dominant (least negative / positive) inhour root for rho."""
    params = params or KineticsParams()
    lam_min = float(np.min(params.lambda_i))
    # Dominant root lies in (-λ_min, +∞), excluding poles at -λ_i.
    if rho < 0.0:
        lo = -lam_min + 1e-12
        hi = -1e-15
    else:
        lo = 1e-15
        hi = 100.0
    f_lo = inhour_reactivity(lo, params) - rho
    f_hi = inhour_reactivity(hi, params) - rho
    if rho >= 0.0 and f_lo * f_hi > 0:
        for _ in range(40):
            hi *= 2.0
            f_hi = inhour_reactivity(hi, params) - rho
            if f_lo * f_hi <= 0:
                break
        else:
            raise RuntimeError("failed to bracket inhour root")
    if f_lo * f_hi > 0:
        raise RuntimeError("failed to bracket inhour root")
    for _ in range(80):
        mid = 0.5 * (lo + hi)
        f_mid = inhour_reactivity(mid, params) - rho
        if abs(f_mid) < 1e-14:
            return mid
        if f_lo * f_mid <= 0:
            hi = mid
            f_hi = f_mid
        else:
            lo = mid
            f_lo = f_mid
    return 0.5 * (lo + hi)
