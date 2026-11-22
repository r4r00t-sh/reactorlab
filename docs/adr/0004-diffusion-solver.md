# ADR 0004: Two-group diffusion solver (Phase 3)

- **Status:** Accepted
- **Date:** 2026-11-23

## Context

Spatial neutronics for educational benchmarks (TWIGL, IAEA 2D PWR, LRA static)
needs a clear, testable diffusion eigenvalue solver without PETSc yet.

## Decision

- **Finite-difference** 1D/2D Cartesian, two-group, power iteration.
- **Eigen** sparse (`SparseLU` / `BiCGSTAB`) in C++; `scipy.sparse` in Python reference.
- Benchmark cross sections live under `data/benchmarks/` with PROVENANCE citations.
- GUI shows thermal-flux heatmap and simple rod material toggles; no physics in QML/Widgets.

## Consequences

- Adequate for coarse-mesh benchmark \(k_{\mathrm{eff}}\) within documented tolerances.
- Fine-mesh IAEA agreement may need mesh refinement studies recorded in `docs/vv/`.
- PETSc deferred until larger 3D / multiphysics (Phase 5+).
