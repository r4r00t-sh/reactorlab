"""Phase 0 smoke test: import the nanobind module and call hello()."""

from __future__ import annotations

import importlib
import sys
from pathlib import Path


def _ensure_extension_on_path() -> None:
    root = Path(__file__).resolve().parents[2]
    candidates = [
        root / "build" / "dev" / "python",
        root / "build" / "release" / "python",
        root / "build" / "python",
    ]
    for path in candidates:
        if path.is_dir():
            sys.path.insert(0, str(path))
            return


def test_reactorlab_core_binding_smoke() -> None:
    _ensure_extension_on_path()
    mod = importlib.import_module("reactorlab_core")

    version = mod.version()
    assert version == "0.7.0"

    disclaimer = mod.educational_disclaimer()
    assert "educational/research" in disclaimer
    assert "safety-grade" in disclaimer

    greeting = mod.hello("pytest")
    assert "pytest" in greeting
    assert "0.7.0" in greeting
