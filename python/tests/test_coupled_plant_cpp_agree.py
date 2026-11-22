"""C++ CVODE coupled plant vs Python reference."""

from __future__ import annotations

import importlib
import sys
from pathlib import Path

import numpy as np
import pytest
from reference.coupled_plant import PlantParams, integrate_plant, make_equilibrium_state
from reference.point_kinetics import step_reactivity


def _load_core():
    root = Path(__file__).resolve().parents[2]
    for path in (
        root / "build" / "dev" / "python",
        root / "build" / "release" / "python",
        root / "build" / "python",
    ):
        if path.is_dir():
            sys.path.insert(0, str(path))
            break
    return importlib.import_module("reactorlab_core")


@pytest.fixture(scope="module")
def core():
    return _load_core()


def test_cpp_matches_python_hold(core) -> None:
    params = PlantParams()
    y0 = make_equilibrium_state(1.0, params)
    t_out = np.linspace(0.0, 50.0, 51)
    t_py, y_py = integrate_plant(
        (0.0, 50.0), step_reactivity(0.0), y0=y0, t_eval=t_out, params=params
    )
    t_cpp, P_cpp, X_cpp, Tf_cpp, Tc_cpp, _rho = core.integrate_coupled_plant(
        0.0, 0.0, t_out.tolist(), 1.0
    )
    assert np.allclose(t_py, t_cpp, atol=1e-12)
    assert np.allclose(y_py[0], P_cpp, rtol=5e-3, atol=5e-3)
    assert np.allclose(y_py[8], X_cpp, rtol=5e-3, atol=0)
    assert np.allclose(y_py[11], Tf_cpp, rtol=0, atol=0.5)
    assert np.allclose(y_py[12], Tc_cpp, rtol=0, atol=0.5)


def test_cpp_xenon_reactivity(core) -> None:
    params = core.PlantParams()
    rho = core.equilibrium_xenon_reactivity(1.0, params)
    assert rho < -0.02
    assert core.critical_bias(params) > 0.0
