# ADR 0002: Point-kinetics solver (Phase 1)

- **Status:** Accepted
- **Date:** 2026-11-23

## Context

Six-group point kinetics is a stiff ODE system (prompt timescale ≪ delayed).
We need a reproducible C++ solver that matches a clear Python reference and
classic analytic checks (prompt jump, inhour period, subcritical decay).

## Decision

- Use **SUNDIALS CVODE** (BDF + Newton) in `libreactorlab` for production solves.
- Keep a **scipy BDF/Radau** Python reference in `python/reference/` for cross-checks.
- External reactivity is a pure function `ρ(t)` (step/ramp/sin/scram); no feedback yet.
- Default delayed-neutron data: **Keepin U-235 thermal** (see `data/PROVENANCE.md`).

## Consequences

- Phase 0 FetchContent/vcpkg must provide SUNDIALS (CVODE + NVector_serial).
- GUI only schedules reactivity and plots `P(t)`; integration stays in the core.
- Tolerances and parameter sets are recorded in `docs/vv/` for each benchmark.
