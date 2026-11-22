# V&V — Multiphysics (Phase 5)

**Date:** 2026-11-23  
**Disclaimer:** Educational/research only.

## Property check (IAPWS-IF97 Region 1)

| State | Quantity | Reference (IAPWS) | Our result | Gate |
|-------|----------|-------------------|------------|------|
| 3 MPa, 300 K | \(v\) [m³/kg] | 0.001002151 | match within 1e−9 rel | PASS |
| 80 MPa, 300 K | \(c_p\) order | ~4 kJ/kg/K | within few % of published | PASS |

(Exact table values recorded in unit tests against IAPWS sample points.)

## Coupled behaviour

| Scenario | Expected qualitative | Status |
|----------|----------------------|--------|
| Steady hold | \(P\approx1\), \(T\) near setpoints | PASS |
| Rod ejection | \(P\) rises then Doppler/moderator limit | PASS |
| Loss of flow | \(\bar T_c\) rises as \(\dot m\) falls | PASS |
| Load-follow | \(P\) tracks slow reactivity schedule | PASS |
| C++ vs Python | trajectories agree within tol | PASS |

## Coupling numerics

Picard residual and iteration counts logged; default max 8 iters, tol 1e−4 K.
