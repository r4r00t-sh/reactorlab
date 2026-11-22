"""Reference tests for Phase 2 coupled plant (Xe dip/peak, feedback)."""

from __future__ import annotations

import numpy as np
import pytest
from reference.coupled_plant import (
    GAMMA_I,
    GAMMA_XE,
    NU,
    PlantParams,
    critical_bias,
    equilibrium_poisons,
    equilibrium_xenon_reactivity,
    integrate_plant,
    make_equilibrium_state,
    poison_reactivity,
    total_reactivity,
)
from reference.point_kinetics import step_reactivity


def test_equilibrium_xenon_worth_formula() -> None:
    params = PlantParams()
    rho_x = equilibrium_xenon_reactivity(1.0, params)
    # Exact: ρ = -(γ_I+γ_X)/ν * (σφ)/(λ_X+σφ)
    sigphi = params.sigma_a_Xe * params.phi0
    exact = -(GAMMA_I + GAMMA_XE) / NU * sigphi / (params.lambda_Xe + sigphi)
    assert rho_x == pytest.approx(exact, rel=0, abs=1e-12)
    # At this educational φ0, burnup is strong but not fully asymptotic.
    asymptotic = -(GAMMA_I + GAMMA_XE) / NU
    assert abs(rho_x) > 0.8 * abs(asymptotic)
    _I, X, _Pm, Sm = equilibrium_poisons(1.0, params)
    _rx, rho_sm = poison_reactivity(X, Sm, params)
    assert critical_bias(params) == pytest.approx(-(rho_x + rho_sm), rel=0, abs=1e-15)


def test_critical_bias_makes_zero_net_rho() -> None:
    params = PlantParams()
    y = make_equilibrium_state(1.0, params)
    rho = total_reactivity(0.0, y, params, step_reactivity(0.0), lambda _t: 0.0)
    assert rho == pytest.approx(0.0, abs=1e-12)


def test_flat_full_power_hold() -> None:
    params = PlantParams()
    y0 = make_equilibrium_state(1.0, params)
    t_eval = np.linspace(0.0, 100.0, 21)
    _t, y = integrate_plant((0.0, 100.0), step_reactivity(0.0), y0=y0, t_eval=t_eval, params=params)
    assert y[0] == pytest.approx(1.0, rel=0, abs=2e-3)
    assert y[11] == pytest.approx(params.Tf0, rel=0, abs=1.0)
    assert y[12] == pytest.approx(params.Tc0, rel=0, abs=1.0)


def test_xenon_peak_after_shutdown() -> None:
    """After scram to P~0, Xe rises above full-power eq then decays (classic peak)."""
    params = PlantParams()
    y0 = make_equilibrium_state(1.0, params)
    X0 = y0[8]

    # Instant rod insertion + hold; power collapses, Xe evolves for many hours.
    def rods(t: float) -> float:
        return 1.0 if t >= 0.0 else 0.0

    # Use large negative external as scram backup with rods.
    t_end = 40.0 * 3600.0  # 40 h
    t_eval = np.linspace(0.0, t_end, 401)
    t, y = integrate_plant(
        (0.0, t_end),
        step_reactivity(0.0),
        rod_fn=rods,
        y0=y0,
        t_eval=t_eval,
        params=params,
        rtol=1e-6,
        atol=1e-8,
    )
    X = y[8]
    P = y[0]
    assert P[10] < 0.05  # scrammed
    assert np.max(X) > 1.05 * X0
    # Peak should occur hours after shutdown, not immediately.
    t_peak = t[int(np.argmax(X))]
    assert 2.0 * 3600.0 < t_peak < 15.0 * 3600.0
    assert X[-1] < np.max(X)


def test_xenon_dip_after_power_reduction() -> None:
    """Step power demand via rods: after reducing power, Xe transiently rises."""
    params = PlantParams()
    y0 = make_equilibrium_state(1.0, params)
    X0 = y0[8]

    # Insert rods partially to drop power; hold.
    def rods(t: float) -> float:
        return 0.15 if t >= 10.0 else 0.0

    t_end = 15.0 * 3600.0
    t_eval = np.linspace(0.0, t_end, 301)
    t, y = integrate_plant(
        (0.0, t_end),
        step_reactivity(0.0),
        rod_fn=rods,
        y0=y0,
        t_eval=t_eval,
        params=params,
        rtol=1e-6,
        atol=1e-8,
    )
    # After power drop, Xe should overshoot its new (lower) equilibrium.
    late = t > 10.0 * 3600.0
    X_new_eq = np.mean(y[8, late])
    X_max_after = np.max(y[8, t >= 10.0])
    assert y[0, -1] < 0.95  # power reduced
    assert X_max_after > X_new_eq * 1.02
    assert X_max_after > 0.9 * X0  # transient hole relative to new eq


def test_doppler_limits_power_rise() -> None:
    params = PlantParams()
    y0 = make_equilibrium_state(1.0, params)
    rho_step = 0.0015
    t_eval = np.linspace(0.0, 60.0, 121)
    _t, y = integrate_plant(
        (0.0, 60.0),
        step_reactivity(rho_step),
        y0=y0,
        t_eval=t_eval,
        params=params,
    )
    assert y[0, -1] > 1.0
    assert y[11, -1] > params.Tf0  # fuel heated
    # Without feedback, inhour would run away more; with Doppler, net rho shrinks.
    assert y[0, -1] < 5.0
