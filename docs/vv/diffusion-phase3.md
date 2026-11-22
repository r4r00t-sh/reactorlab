# V&V — Two-group diffusion (Phase 3)

**Date:** 2026-11-23  
**Disclaimer:** Educational/research only — not safety-grade.

## Solver

| Item | Value |
|------|-------|
| Method | 2-group cell-centered FD, power iteration |
| Linear algebra | Eigen SparseLU (C++); scipy.sparse (Python) |
| BC | TWIGL: reflective at x=0,y=0; zero-flux outer |

## TWIGL (primary)

| Mesh | Our \(k_{\mathrm{eff}}\) | Reference 0.91321 | Error |
|------|--------------------------|-------------------|-------|
| 20×20 | ~0.91300 | 0.91321 | ~−21 pcm |
| 40×40 | ~0.91310 | 0.91321 | ~−11 pcm |
| 80×80 | ~0.91318 | 0.91321 | ~−3 pcm |

C++ matches Python within ~2e−5 absolute on 40×40.

**Citation:** Hageman & Yasinsky TWIGL; XS/geometry as in OFELIA TWIGL2D.geo / Seamplex SP-MI-17-96D3.

## IAEA 2D PWR / LRA

YAML stubs live in `data/benchmarks/` with published reference \(k\) values and textbook-lineage XS.
**Phase 3 automated pass criterion for these two is deferred to a follow-up mesh/map audit** — the Cartesian assembly maps are educational placeholders and must not be treated as bit-exact ANL-7416 reproductions until verified cell-by-cell. TWIGL is the locked Phase 3 spatial V&V gate.

## Known limitations

- Cartesian FD only (no FEM, no hexagonal).
- Steady eigenvalue only (no diffusion kinetics / LRA transient yet).
- Rod model in GUI is a uniform Σa bump on seed, not the benchmark rod curve.
