"""C++/Python agreement for IAPWS and multiphysics hold / rod-ejection."""

from __future__ import annotations

import numpy as np
import pytest

pytest.importorskip("reactorlab_core")
import reactorlab_core as rl

from reference.iapws_if97 import region1
from reference.multiphysics import (
    MultiParams,
    integrate,
    make_equilibrium_state,
    scenario_rod_ejection,
)


def test_iapws_region1_agree() -> None:
    for p, T in [(3.0e6, 300.0), (15.5e6, 580.0), (10.0e6, 560.0)]:
        py = region1(p, T)
        v, rho, cp = rl.iapws_region1(p, T)
        assert abs(v - py.v) / py.v < 1e-12
        assert abs(rho - py.rho) / py.rho < 1e-12
        assert abs(cp - py.cp) / py.cp < 1e-10


def test_hold_agree() -> None:
    t = np.linspace(0.0, 5.0, 11)
    t_cpp, P_cpp, Tf_cpp, *_ = rl.integrate_multiphysics("hold", t.tolist())
    params = MultiParams()
    s0 = make_equilibrium_state(1.0, params)
    traj = integrate(
        params,
        lambda _t: 0.0,
        lambda _t: 0.0,
        lambda _t: params.mdot0,
        t,
        s0,
    )
    assert np.max(np.abs(np.asarray(P_cpp) - traj.P)) < 0.05
    assert np.max(np.abs(np.asarray(Tf_cpp) - traj.Tf)) < 10.0


def test_rod_ejection_agree() -> None:
    t = np.linspace(0.0, 5.0, 26)
    t_cpp, P_cpp, Tf_cpp, *_ = rl.integrate_multiphysics("rod_ejection", t.tolist())
    params = MultiParams()
    rho_ext, rod, mdot, z0 = scenario_rod_ejection(1.0, 0.08, 0.2)
    s0 = make_equilibrium_state(1.0, params, rod_z0=z0)
    traj = integrate(params, rho_ext, rod, mdot, t, s0)
    # Relative tolerance — Picard / CVODE vs BDF may differ slightly
    assert abs(P_cpp[-1] - traj.P[-1]) / max(traj.P[-1], 1.0) < 0.15
    assert abs(Tf_cpp[-1] - traj.Tf[-1]) < 30.0
