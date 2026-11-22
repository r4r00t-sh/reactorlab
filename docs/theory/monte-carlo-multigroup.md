# Multigroup Monte Carlo eigenvalue (Phase 4)

**Status:** Educational — not for design or licensing  
**Units:** length cm, Σ cm⁻¹, flux arbitrary (track-length), \(k\) dimensionless

## Transport model

Analog, multigroup, isotropic scattering in the lab frame, no continuous energy.
A history is a sequence of free flights and collisions until absorption or leakage.

Free path:

\[
\ell = -\frac{\ln\xi}{\Sigma_{t,g}(\mathbf{r})}
\]

At collision in group \(g\), sample reaction with probabilities
\(\Sigma_{a,g}/\Sigma_{t,g}\), \(\Sigma_{s,g\to g'}/\Sigma_{t,g}\), fission via \(\nu\Sigma_{f,g}/\Sigma_{t,g}\)
(analog: fission sites banked for the next cycle; history may terminate on absorption).

## \(k\)-eigenvalue power iteration

Inactive cycles discard tallies while the fission source converges.
Active cycles accumulate:

- **Collision estimator** for \(k\): ratio of fission neutrons produced to source size
- **Track-length flux** tallies by group (and region, when geometry has cells)

Cycle \(k\) estimates \(\hat k_i\); report

\[
\bar k=\frac1N\sum_i\hat k_i,\qquad
\sigma_{\bar k}=\sqrt{\frac{1}{N(N-1)}\sum_i(\hat k_i-\bar k)^2}.
\]

## Parallelism & determinism

- **History-level OpenMP**: each history draws from an independent RNG stream
  seeded by `(run_seed, cycle, history_id)`.
- Same seed ⇒ reproducible tallies (bit-identical on same OpenMP thread count
  is not guaranteed if reductions reorder; cycle \(k\) sequence is seeded).

## Validation targets

| Case | Reference | Notes |
|------|-----------|-------|
| Infinite homogeneous MG | Analytic \(k_\infty\) | Zero leakage |
| 1D slab, vacuum BC | Diffusion / fine MC reference | Leakage |
| C5G7 | OECD/NEA | Deferred / simplified pin if full model not ready |
| OpenMC | Same XS/geometry | Optional external cross-check |

## Assumptions

1. Multigroup only; isotropic scatter; no unresolved resonances.
2. No temperature / Doppler broadening.
3. Vacuum or reflective BC on axis-aligned boxes.
