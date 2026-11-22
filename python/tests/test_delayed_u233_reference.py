"""U-233 Keepin delayed-neutron reference checks."""

from __future__ import annotations

import numpy as np
import pytest
from reference.delayed_u233 import (
    BETA_U233,
    NU_D_U233,
    NU_U233,
    dollars,
    fission_weighted_beta_eff,
    u233_thermal_keepin,
)
from reference.point_kinetics import BETA, integrate, step_reactivity


def test_u233_beta_nu_consistency() -> None:
    assert BETA_U233 == pytest.approx(NU_D_U233 / NU_U233, rel=0, abs=5e-5)
    assert BETA_U233 < 0.5 * BETA


def test_u233_faster_jump_than_u235() -> None:
    rho = 0.0015
    u3 = u233_thermal_keepin()
    from reference.point_kinetics import KineticsParams

    u5 = KineticsParams()
    t_end = 1e-2
    t_eval = np.array([0.0, t_end])
    _, y3 = integrate((0.0, t_end), step_reactivity(rho), params=u3, t_eval=t_eval)
    _, y5 = integrate((0.0, t_end), step_reactivity(rho), params=u5, t_eval=t_eval)
    jump3 = y3[0, -1] / y3[0, 0]
    jump5 = y5[0, -1] / y5[0, 0]
    assert jump3 > jump5


def test_dollars_and_beta_eff() -> None:
    assert dollars(BETA_U233, BETA_U233) == pytest.approx(1.0)
    be = fission_weighted_beta_eff(np.array([1.0, 0.0]), np.array([BETA_U233, BETA]))
    assert be == pytest.approx(BETA_U233)
