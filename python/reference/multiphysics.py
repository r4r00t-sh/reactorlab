"""Coupled multiphysics: point kinetics + lumped fuel + 1D channel (Picard).

Theory: docs/theory/multiphysics-coupling.md
IAPWS: reference.iapws_if97

Units: SI (Pa, K, kg/s, W, Δk/k). Educational — not safety-grade.
"""

from __future__ import annotations

import math
from collections.abc import Callable
from dataclasses import dataclass, field

import numpy as np
from scipy.integrate import solve_ivp

from reference.iapws_if97 import cp as iapws_cp
from reference.iapws_if97 import density as iapws_density
from reference.point_kinetics import KineticsParams, equilibrium_precursors

ALPHA_D = -2.5e-5  # 1/K
ALPHA_M = -2.0e-5  # 1/K
ROD_WORTH = 0.05  # Δk/k at full insertion
TF0 = 900.0  # K
TC0 = 580.0  # K
TIN = 565.0  # K
H_HT = 1.5e7  # W/K
CF = 2.0e7  # J/K
Q0 = H_HT * (TF0 - TC0)  # W at P=1
P_SYS = 15.5e6  # Pa
CHANNEL_L = 3.5  # m
CHANNEL_A = 5.0  # m²
N_NODES = 10
MDOT0 = 2.9e4  # kg/s (educational; sized for ~30 K rise)


@dataclass
class MultiParams:
    kinetics: KineticsParams = field(default_factory=KineticsParams)
    alpha_D: float = ALPHA_D
    alpha_M: float = ALPHA_M
    rod_worth: float = ROD_WORTH
    Tf0: float = TF0
    Tc0: float = TC0
    Tin: float = TIN
    H: float = H_HT
    Q0: float = Q0
    Cf: float = CF
    p_sys: float = P_SYS
    L: float = CHANNEL_L
    A: float = CHANNEL_A
    n_nodes: int = N_NODES
    mdot0: float = MDOT0
    rho_bias: float = 0.0
    picard_max: int = 8
    picard_tol: float = 1.0e-4  # K

    @property
    def dz(self) -> float:
        return self.L / float(self.n_nodes)


@dataclass
class MultiState:
    t: float = 0.0
    P: float = 1.0
    C: np.ndarray = field(default_factory=lambda: np.zeros(6))
    Tf: float = TF0
    Tc: np.ndarray = field(default_factory=lambda: np.full(N_NODES, TC0))
    picard_iters: int = 0
    picard_res: float = 0.0

    @property
    def Tc_bar(self) -> float:
        return float(np.mean(self.Tc))

    @property
    def Tout(self) -> float:
        return float(self.Tc[-1])


@dataclass
class MultiTrajectory:
    t: np.ndarray
    P: np.ndarray
    Tf: np.ndarray
    Tc_bar: np.ndarray
    Tout: np.ndarray
    rho_total: np.ndarray
    mdot: np.ndarray
    picard_iters: np.ndarray


DriverFn = Callable[[float], float]  # z(t), mdot(t), or rho_ext(t)


def total_reactivity(
    t: float,
    P: float,
    Tf: float,
    Tc_bar: float,
    params: MultiParams,
    rho_ext: DriverFn,
    rod_z: DriverFn,
) -> float:
    """ρ = bias + ext + rod + Doppler + moderator. Rod: ρ_rod = −worth·z."""
    _ = P
    return (
        params.rho_bias
        + float(rho_ext(t))
        - params.rod_worth * float(rod_z(t))
        + params.alpha_D * (Tf - params.Tf0)
        + params.alpha_M * (Tc_bar - params.Tc0)
    )


def make_equilibrium_state(
    P0: float = 1.0,
    params: MultiParams | None = None,
    rod_z0: float = 0.0,
) -> MultiState:
    """Critical equilibrium at P0 with linear coolant profile; sets rho_bias."""
    params = params or MultiParams()
    Tout = 2.0 * params.Tc0 - params.Tin
    Tc = np.linspace(params.Tin + (Tout - params.Tin) / (2 * params.n_nodes), Tout, params.n_nodes)
    # Adjust outlet mean toward Tc0
    Tc = Tc + (params.Tc0 - float(np.mean(Tc)))
    C = equilibrium_precursors(P0, params.kinetics)
    # Choose bias so ρ≈0 at t=0 with given rod
    bias = (
        params.rod_worth * rod_z0
        - params.alpha_D * 0.0
        - params.alpha_M * (float(np.mean(Tc)) - params.Tc0)
    )
    params.rho_bias = bias
    return MultiState(t=0.0, P=P0, C=C.copy(), Tf=params.Tf0, Tc=Tc.copy())


def _nf_rhs(
    t: float,
    y: np.ndarray,
    params: MultiParams,
    Tc_bar: float,
    rho_ext: DriverFn,
    rod_z: DriverFn,
) -> np.ndarray:
    P = max(y[0], 0.0)
    C = y[1:7]
    Tf = y[7]
    rho = total_reactivity(t, P, Tf, Tc_bar, params, rho_ext, rod_z)
    beta = params.kinetics.beta
    dP = ((rho - beta) / params.kinetics.Lambda) * P + float(np.dot(params.kinetics.lambda_i, C))
    dC = (params.kinetics.beta_i / params.kinetics.Lambda) * P - params.kinetics.lambda_i * C
    dTf = (P * params.Q0 - params.H * (Tf - Tc_bar)) / params.Cf
    out = np.empty(8)
    out[0] = dP
    out[1:7] = dC
    out[7] = dTf
    return out


def _advance_neutronics_fuel(
    state: MultiState,
    t1: float,
    params: MultiParams,
    Tc_bar: float,
    rho_ext: DriverFn,
    rod_z: DriverFn,
) -> tuple[float, np.ndarray, float]:
    y0 = np.empty(8)
    y0[0] = state.P
    y0[1:7] = state.C
    y0[7] = state.Tf
    sol = solve_ivp(
        lambda t, y: _nf_rhs(t, y, params, Tc_bar, rho_ext, rod_z),
        (state.t, t1),
        y0,
        method="BDF",
        rtol=1e-7,
        atol=1e-9,
        dense_output=False,
    )
    if not sol.success:
        raise RuntimeError(f"neutronics/fuel IVP failed: {sol.message}")
    y = sol.y[:, -1]
    return float(y[0]), y[1:7].copy(), float(y[7])


def _update_coolant(
    Tc: np.ndarray,
    Tf: float,
    mdot: float,
    dt: float,
    params: MultiParams,
) -> np.ndarray:
    """Implicit Euler axial sweep with IAPWS ρ, cp at node temperature."""
    n = params.n_nodes
    out = Tc.copy()
    Tin = params.Tin
    for i in range(n):
        T_old = Tc[i]
        props = (
            iapws_density(params.p_sys, T_old),
            iapws_cp(params.p_sys, T_old),
        )
        rho_i, cp_i = props
        Cap = rho_i * cp_i * params.A * params.dz / dt
        T_up = Tin if i == 0 else out[i - 1]
        m_cp = mdot * cp_i
        Hn = params.H / float(n)
        denom = Cap + m_cp + Hn
        out[i] = (Cap * T_old + m_cp * T_up + Hn * Tf) / denom
    return out


def advance(
    params: MultiParams,
    rho_ext: DriverFn,
    rod_z: DriverFn,
    mdot_fn: DriverFn,
    state: MultiState,
    t1: float,
) -> MultiState:
    """One Picard-coupled time step from state.t → t1."""
    if t1 <= state.t:
        raise ValueError("t1 must be > state.t")
    dt = t1 - state.t
    mdot = max(float(mdot_fn(state.t)), 1.0)  # avoid zero-flow singularity
    Tc = state.Tc.copy()
    P, C, Tf = state.P, state.C.copy(), state.Tf
    res = 0.0
    it = 0
    for it in range(1, params.picard_max + 1):
        Tc_bar = float(np.mean(Tc))
        P, C, Tf = _advance_neutronics_fuel(
            MultiState(t=state.t, P=state.P, C=state.C.copy(), Tf=state.Tf, Tc=state.Tc),
            t1,
            params,
            Tc_bar,
            rho_ext,
            rod_z,
        )
        Tc_new = _update_coolant(state.Tc, Tf, mdot, dt, params)
        res = float(np.max(np.abs(Tc_new - Tc)))
        Tc = Tc_new
        if res < params.picard_tol:
            break
    return MultiState(
        t=t1,
        P=max(P, 0.0),
        C=C,
        Tf=Tf,
        Tc=Tc,
        picard_iters=it,
        picard_res=res,
    )


def integrate(
    params: MultiParams,
    rho_ext: DriverFn,
    rod_z: DriverFn,
    mdot_fn: DriverFn,
    t_out: np.ndarray,
    state0: MultiState | None = None,
) -> MultiTrajectory:
    """Advance with fixed outer steps between consecutive t_out samples."""
    t_out = np.asarray(t_out, dtype=float)
    if state0 is None:
        state0 = make_equilibrium_state(1.0, params)
    s = state0
    n = len(t_out)
    P = np.empty(n)
    Tf = np.empty(n)
    Tc_bar = np.empty(n)
    Tout = np.empty(n)
    rho = np.empty(n)
    mdot = np.empty(n)
    pit = np.empty(n, dtype=int)
    for i, ti in enumerate(t_out):
        if ti > s.t + 1e-15:
            s = advance(params, rho_ext, rod_z, mdot_fn, s, float(ti))
        P[i] = s.P
        Tf[i] = s.Tf
        Tc_bar[i] = s.Tc_bar
        Tout[i] = s.Tout
        rho[i] = total_reactivity(s.t, s.P, s.Tf, s.Tc_bar, params, rho_ext, rod_z)
        mdot[i] = float(mdot_fn(s.t))
        pit[i] = s.picard_iters
    return MultiTrajectory(
        t=t_out.copy(),
        P=P,
        Tf=Tf,
        Tc_bar=Tc_bar,
        Tout=Tout,
        rho_total=rho,
        mdot=mdot,
        picard_iters=pit,
    )


# --- Scenario drivers ---


def scenario_rod_ejection(
    t_eject: float = 1.0,
    z0: float = 0.08,
    duration: float = 0.1,
) -> tuple[DriverFn, DriverFn, DriverFn, float]:
    """Rods withdraw from z0→0 around t_eject; constant mdot, ρ_ext=0.

    Default z0·worth ≈ 0.004 < β (delayed supercritical, educational).
    """

    def rod(t: float) -> float:
        if t < t_eject:
            return z0
        if t >= t_eject + duration:
            return 0.0
        return z0 * (1.0 - (t - t_eject) / duration)

    return (lambda _t: 0.0), rod, (lambda _t: MDOT0), z0


def scenario_loss_of_flow(
    t_start: float = 5.0,
    fraction: float = 0.2,
    ramp: float = 10.0,
) -> tuple[DriverFn, DriverFn, DriverFn, float]:
    """Mass flow ramps to fraction·mdot0; rods out."""

    def mdot(t: float) -> float:
        if t < t_start:
            return MDOT0
        if t >= t_start + ramp:
            return fraction * MDOT0
        x = (t - t_start) / ramp
        return MDOT0 * (1.0 - (1.0 - fraction) * x)

    return (lambda _t: 0.0), (lambda _t: 0.0), mdot, 0.0


def scenario_load_follow(
    period: float = 200.0,
    amp: float = 0.001,
) -> tuple[DriverFn, DriverFn, DriverFn, float]:
    """Slow sinusoidal ρ_ext for load-follow demo."""

    def rho_ext(t: float) -> float:
        return amp * math.sin(2.0 * math.pi * t / period)

    return rho_ext, (lambda _t: 0.0), (lambda _t: MDOT0), 0.0
