"""Cross-check C++ CVODE point kinetics against the Python reference."""

from __future__ import annotations

import importlib
import sys
from pathlib import Path

import numpy as np
import pytest
from reference.point_kinetics import KineticsParams, integrate, step_reactivity


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


def test_cpp_matches_python_step(core) -> None:
    params = KineticsParams()
    data = core.u235_thermal_keepin()
    assert data.beta == pytest.approx(params.beta, rel=0, abs=1e-15)
    assert data.Lambda == pytest.approx(params.Lambda)

    rho = 0.0015
    t_out = np.linspace(0.0, 30.0, 301)
    t_py, y_py = integrate((0.0, 30.0), step_reactivity(rho), params=params, t_eval=t_out)
    t_cpp, P_cpp = core.integrate_point_kinetics(data, rho, t_out.tolist())

    assert np.allclose(t_py, t_cpp, rtol=0, atol=1e-12)
    # Relative agreement on power history (stiff ODE; rtol/atol matched at 1e-8/1e-10).
    assert np.allclose(y_py[0], P_cpp, rtol=2e-4, atol=2e-5)


def test_cpp_prompt_jump_helper(core) -> None:
    data = core.u235_thermal_keepin()
    rho = 0.002
    assert core.prompt_jump_ratio(rho, data) == pytest.approx(data.beta / (data.beta - rho))
