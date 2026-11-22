"""IAPWS-IF97 Region 1 checks against published sample points."""

from __future__ import annotations

import numpy as np

from reference.iapws_if97 import SAMPLE_V_3MPA_300K, region1


def test_region1_table5_sample() -> None:
    """IAPWS R7-97 Table 5: p=3 MPa, T=300 K → v = 1.00215168e-3 m³/kg."""
    prop = region1(3.0e6, 300.0)
    assert abs(prop.v - SAMPLE_V_3MPA_300K) < 1.0e-11
    assert abs(prop.rho - 1.0 / SAMPLE_V_3MPA_300K) < 1.0e-5


def test_region1_pwr_like_range() -> None:
    prop = region1(15.5e6, 580.0)
    assert 650.0 < prop.rho < 800.0
    assert 4000.0 < prop.cp < 8000.0


def test_region1_density_decreases_with_T() -> None:
    r1 = region1(15.0e6, 560.0).rho
    r2 = region1(15.0e6, 600.0).rho
    assert r2 < r1
