"""Python reference tests for two-group diffusion."""

from __future__ import annotations

import numpy as np
import pytest
from reference.diffusion import (
    BENCH_DIR,
    load_yaml_benchmark,
    run_homogeneous_1d_analytic_check,
    run_twigl,
)


def test_1d_keff_below_kinf() -> None:
    res, k_inf = run_homogeneous_1d_analytic_check(n=100)
    assert k_inf > 0.0
    assert res.keff < k_inf
    assert res.keff > 0.5 * k_inf


def test_twigl_keff() -> None:
    data = load_yaml_benchmark(BENCH_DIR / "twigl.yaml")
    ref = float(data["reference_keff"])
    res = run_twigl(n=40, tol=1e-7)
    # 40×40 (~2 cm): expect ~10 pcm; allow 50 pcm gate
    assert res.keff == pytest.approx(ref, rel=0, abs=5e-4)


def test_twigl_flux_positive() -> None:
    res = run_twigl(n=20, tol=1e-6)
    assert np.all(res.phi2 >= -1e-12)
    assert np.max(res.phi2) > 0.0
