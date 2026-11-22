# ADR 0005: Multigroup Monte Carlo (Phase 4)

- **Status:** Accepted
- **Date:** 2026-11-23

## Context

Need an educational \(k\)-eigenvalue Monte Carlo with uncertainties, OpenMP, and
reproducible seeds — without requiring OpenMC at build time.

## Decision

- Analog multigroup power iteration in `libreactorlab` (`reactorlab::montecarlo`).
- History-level OpenMP; per-history RNG streams from a master seed.
- Python reference mirrors the algorithm for cross-checks.
- Full **C5G7** pin-by-pin and live **OpenMC** comparison are optional/follow-up;
  Phase 4 gate is analytic \(k_\infty\) + slab + C++↔Python agreement.

## Consequences

- GUI shows \(\bar k\pm\sigma\) and group flux bars; no physics in Widgets.
- OpenMC remains a recommended external verifier, not a hard dependency.
