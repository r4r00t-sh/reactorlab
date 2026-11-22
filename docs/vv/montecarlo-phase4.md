# V&V — Multigroup Monte Carlo (Phase 4)

**Date:** 2026-11-23  
**Disclaimer:** Educational/research only — not safety-grade.

## Configuration

| Item | Value |
|------|-------|
| Method | Analog multigroup; collision \(k\) estimator; track-length flux |
| Parallelism | OpenMP history-level; per-history RNG streams |
| Seed | User-specified `uint64` |

## Results

| Test | Reference | Result / gate | Status |
|------|-----------|---------------|--------|
| Analytic \(k_\infty\) (2G) | \(5/6\) | Exact helper | PASS |
| Infinite MC vs analytic | \(5/6\) | \|Δk\| < 0.02 and < 3σ | PASS |
| Slab \(L=10\) cm | \(k < k_\infty\) | \(k \in (0.1, k_\infty)\) | PASS |
| Seed reproducibility | same seed | identical \(k\) (C++) | PASS |
| C++ vs Python infinite | both near analytic | \|Δ\| < 0.03 | PASS |

## Deferred / optional

| Item | Note |
|------|------|
| OECD/NEA **C5G7** | Full 7-group pin-cell model not gated; XS/geometry follow-up |
| **OpenMC** cross-check | Optional external; not a build dependency. Use same XS when available |

## Known limitations

- Multigroup isotropic only; no continuous-energy / unresolved resonance.
- Slab is 1D homogeneous; no pin-resolved lattice yet.
- OpenMP reductions may change cycle sample paths vs serial, but streams are seeded per history.
