"""C++ MC vs Python infinite-medium agreement."""

from __future__ import annotations

import importlib
import sys
from pathlib import Path

import pytest
from reference.montecarlo import analytic_k_inf, infinite_medium_keff, material_2g_infinite


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


def test_cpp_infinite_near_python_and_analytic() -> None:
    core = _load_core()
    mat = material_2g_infinite()
    ref = analytic_k_inf(mat)
    k_py, _ = infinite_medium_keff(mat, n_histories=20000, seed=11)
    k_cpp, s_cpp = core.mc_infinite_keff(20000, 11)
    assert core.analytic_k_inf_2g() == pytest.approx(ref, abs=1e-15)
    assert abs(k_cpp - ref) < 0.02
    assert abs(k_py - ref) < 0.02
    # Different RNGs ⇒ not bit-identical; both near analytic
    assert abs(k_cpp - k_py) < 0.03
    assert s_cpp > 0.0
