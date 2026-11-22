"""Analytic and smoke tests for the Python MC reference."""

from __future__ import annotations

import pytest
from reference.montecarlo import (
    SlabGeometry,
    analytic_k_inf,
    infinite_medium_keff,
    material_2g_infinite,
    power_iteration_slab,
)


def test_analytic_k_inf_value() -> None:
    mat = material_2g_infinite()
    assert analytic_k_inf(mat) == pytest.approx(5.0 / 6.0, rel=0, abs=1e-15)


def test_infinite_mc_matches_analytic() -> None:
    mat = material_2g_infinite()
    k, sk = infinite_medium_keff(mat, n_histories=20000, seed=7)
    ref = analytic_k_inf(mat)
    # 3-sigma gate
    assert abs(k - ref) < 3.0 * max(sk, 1e-4)
    assert abs(k - ref) < 0.02


def test_slab_keff_below_kinf() -> None:
    mat = material_2g_infinite()
    k_inf = analytic_k_inf(mat)
    res = power_iteration_slab(
        mat,
        SlabGeometry(L=10.0),
        n_histories=1500,
        n_inactive=15,
        n_active=30,
        seed=3,
    )
    assert res.keff < k_inf
    assert res.keff > 0.1  # thin slab leaks strongly
    assert res.keff_std > 0.0
