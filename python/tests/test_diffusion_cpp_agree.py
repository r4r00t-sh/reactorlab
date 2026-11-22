"""C++ TWIGL diffusion vs Python reference."""

from __future__ import annotations

import importlib
import sys
from pathlib import Path

import pytest
from reference.diffusion import run_twigl


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


def test_twigl_cpp_matches_python() -> None:
    core = _load_core()
    py = run_twigl(40)
    keff, nx, ny, _phi1, _phi2, _it = core.solve_twigl(40)
    assert nx == 40 and ny == 40
    assert keff == pytest.approx(py.keff, rel=0, abs=2e-5)
