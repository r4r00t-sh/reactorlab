# ADR 0006: Multiphysics coupling (Phase 5)

- **Status:** Accepted
- **Date:** 2026-11-23

## Context

Need neutronics ↔ thermal-hydraulics feedback with documented water properties
and educational transient scenarios, without a full system code.

## Decision

- Point kinetics + lumped fuel + **1D single-phase channel**.
- **IAPWS-IF97 Region 1** for \(\rho\), \(c_p\) (coefficients from IAPWS R7-97).
- **Picard / operator-split** coupling each \(\Delta t\) (serial N→TH).
- Scenarios as exogenous \(z(t)\), \(\dot m(t)\), \(\rho_{\mathrm{ext}}(t)\).

## Consequences

- Suitable for rod-ejection / LOF / load-follow demos with alarms.
- Not a best-estimate LOCA/ATWS tool; no two-phase, no 3D core.
- Full IAPWS regions 2–5 deferred; PWR-like liquid only.
