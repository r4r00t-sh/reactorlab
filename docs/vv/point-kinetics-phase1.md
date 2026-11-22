# V&V — Point kinetics (Phase 1)

**Date:** 2026-11-23  
**Commit:** (record at release)  
**Disclaimer:** Educational/research only — not safety-grade.

## Configuration

| Item | Value |
|------|-------|
| Delayed-neutron set | Keepin U-235 thermal (6 groups) |
| `Λ` | `1×10⁻⁴` s |
| C++ solver | SUNDIALS CVODE BDF + dense Newton |
| Python reference | `scipy.integrate.solve_ivp` method=`BDF` |
| Tolerances | rtol=`1e-8`, atol=`1e-10` |

## Results

| Test | Reference | Result | Tolerance | Status |
|------|-----------|--------|-----------|--------|
| Prompt jump `ρ=0.0025` | `β/(β-ρ)=1.6247` | C++ & Python within 5% at `t≈10 Λ/(β-ρ)` | rel 5% | PASS |
| Inhour period `ρ=0.001` | `ω` from inhour eq. | Fitted `d ln P / dt` within 5% | rel 5% | PASS |
| Subcritical `ρ=-0.002` | Dominant inhour `ω` | Late-time slope (`t≥500` s) within 2% | rel 2% | PASS |
| `ρ=0` flat | `P=1` | `|P-1|<1e-6` over 10 s | abs `1e-6` | PASS |
| C++ vs Python step `ρ=0.0015`, 0–30 s | Python BDF | `allclose` rtol=`2e-4`, atol=`2e-5` | — | PASS |

## Known limitations

- No feedback, spatial effects, or temperature dependence.
- Keepin parameters and `Λ` are textbook defaults, not plant-specific data.
- GUI recreates a short CVODE solve each frame (fine for 7 ODEs; not a production stepper).
