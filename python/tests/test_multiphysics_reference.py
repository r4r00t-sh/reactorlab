"""Phase 5 multiphysics reference scenarios."""

from __future__ import annotations

import numpy as np

from reference.multiphysics import (
    MultiParams,
    integrate,
    make_equilibrium_state,
    scenario_load_follow,
    scenario_loss_of_flow,
    scenario_rod_ejection,
    total_reactivity,
)


def test_equilibrium_near_critical() -> None:
    params = MultiParams()
    s = make_equilibrium_state(1.0, params, rod_z0=0.0)
    rho = total_reactivity(
        0.0, s.P, s.Tf, s.Tc_bar, params, lambda _t: 0.0, lambda _t: 0.0
    )
    assert abs(rho) < 1.0e-6
    assert abs(s.P - 1.0) < 1e-12


def test_hold_steady_short() -> None:
    params = MultiParams()
    s0 = make_equilibrium_state(1.0, params)
    t = np.linspace(0.0, 5.0, 11)
    traj = integrate(
        params,
        lambda _t: 0.0,
        lambda _t: 0.0,
        lambda _t: params.mdot0,
        t,
        s0,
    )
    assert abs(traj.P[-1] - 1.0) < 0.02
    assert abs(traj.Tf[-1] - params.Tf0) < 5.0


def test_rod_ejection_power_rises() -> None:
    params = MultiParams()
    rho_ext, rod, mdot, z0 = scenario_rod_ejection(t_eject=1.0, z0=0.08, duration=0.2)
    s0 = make_equilibrium_state(1.0, params, rod_z0=z0)
    t = np.linspace(0.0, 5.0, 51)
    traj = integrate(params, rho_ext, rod, mdot, t, s0)
    assert traj.P[-1] > 1.05
    assert traj.Tf[-1] > params.Tf0


def test_lof_coolant_heats() -> None:
    params = MultiParams()
    rho_ext, rod, mdot, z0 = scenario_loss_of_flow(t_start=2.0, fraction=0.4, ramp=5.0)
    s0 = make_equilibrium_state(1.0, params, rod_z0=z0)
    t = np.linspace(0.0, 20.0, 41)
    traj = integrate(params, rho_ext, rod, mdot, t, s0)
    assert traj.Tout[-1] > traj.Tout[0] + 2.0
    assert traj.mdot[-1] < 0.45 * params.mdot0


def test_load_follow_power_oscillates() -> None:
    params = MultiParams()
    rho_ext, rod, mdot, z0 = scenario_load_follow(period=100.0, amp=0.0015)
    s0 = make_equilibrium_state(1.0, params, rod_z0=z0)
    t = np.linspace(0.0, 150.0, 76)
    traj = integrate(params, rho_ext, rod, mdot, t, s0)
    assert traj.P.max() > 1.02
    assert traj.P.min() < 0.98
