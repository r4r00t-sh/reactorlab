"""Analytic and numerical tests for the Python point-kinetics reference."""

from __future__ import annotations

import numpy as np
import pytest
from reference.point_kinetics import (
    BETA,
    KineticsParams,
    inhour_reactivity,
    integrate,
    prompt_jump_ratio,
    solve_inhour_omega,
    step_reactivity,
)


def test_keepin_beta_sum() -> None:
    params = KineticsParams()
    assert params.beta == pytest.approx(0.006502, rel=0, abs=1e-12)
    assert BETA == pytest.approx(params.beta)


def test_prompt_jump_matches_analytic() -> None:
    params = KineticsParams()
    rho = 0.0025  # ~0.38 $
    analytic = prompt_jump_ratio(rho, params)

    # After ~few prompt time constants Λ/(β-ρ), before delayed groups move much.
    t_jump = 10.0 * params.Lambda / (params.beta - rho)
    t_eval = np.array([0.0, t_jump])
    _t, y = integrate((0.0, t_jump), step_reactivity(rho), params=params, t_eval=t_eval)
    numerical = y[0, -1] / y[0, 0]
    assert numerical == pytest.approx(analytic, rel=5e-2)


def test_inhour_stable_period() -> None:
    params = KineticsParams()
    rho = 0.001  # small positive step
    omega = solve_inhour_omega(rho, params)
    assert inhour_reactivity(omega, params) == pytest.approx(rho, rel=0, abs=1e-12)
    assert omega > 0.0
    period = 1.0 / omega

    # Integrate long enough to enter asymptotic regime; fit ln P vs t.
    t_end = max(5.0 * period, 20.0)
    t_eval = np.linspace(0.5 * t_end, t_end, 50)
    t, y = integrate((0.0, t_end), step_reactivity(rho), params=params, t_eval=t_eval)
    log_p = np.log(y[0])
    slope, _intercept = np.polyfit(t, log_p, 1)
    assert slope == pytest.approx(omega, rel=5e-2)


def test_subcritical_decay() -> None:
    params = KineticsParams()
    rho = -0.002  # subcritical step (~-0.31 $)
    omega = solve_inhour_omega(rho, params)
    assert omega < 0.0

    # Higher modes die slowly; fit only the late asymptotic window.
    t_end = 800.0
    t_eval = np.linspace(0.0, t_end, 801)
    t, y = integrate((0.0, t_end), step_reactivity(rho), params=params, t_eval=t_eval)
    P = y[0]
    assert P[1] < P[0]  # prompt drop
    assert P[-1] < P[0]

    late = t >= 500.0
    slope, _ = np.polyfit(t[late], np.log(P[late]), 1)
    assert slope == pytest.approx(omega, rel=2e-2)


def test_zero_reactivity_stays_flat() -> None:
    t_eval = np.linspace(0.0, 10.0, 21)
    _t, y = integrate((0.0, 10.0), step_reactivity(0.0), t_eval=t_eval)
    assert y[0] == pytest.approx(1.0, rel=0, abs=1e-6)
