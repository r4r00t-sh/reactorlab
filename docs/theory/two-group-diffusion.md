# Two-group neutron diffusion (Phase 3)

**Status:** Educational — not for design or licensing  
**Units:** SI / cm convention for nuclear data as published (length cm, Σ in cm⁻¹, D in cm)

## Steady-state equations

For groups \(g=1\) (fast) and \(g=2\) (thermal), with fission spectrum \(\chi_1=1\), \(\chi_2=0\)
and downscatter only \(1\to2\):

\[
-\nabla\cdot D_1\nabla\phi_1
+\bigl(\Sigma_{a1}+\Sigma_{s}^{1\to2}\bigr)\phi_1
=\frac{1}{k}\bigl(\nu\Sigma_{f1}\phi_1+\nu\Sigma_{f2}\phi_2\bigr)
\]

\[
-\nabla\cdot D_2\nabla\phi_2
+\Sigma_{a2}\phi_2
-\Sigma_{s}^{1\to2}\phi_1
=0
\]

(Removal form: \(\Sigma_{r1}=\Sigma_{a1}+\Sigma_{s}^{1\to2}\).)

## Discretization

- **1D slab / 2D Cartesian** finite differences on a uniform mesh.
- Cell-centered fluxes; material properties piecewise constant per region.
- Outer boundary: zero-flux (\(\phi=0\)) unless a benchmark specifies otherwise.
- Internal interfaces: continuity of \(\phi\) and normal current (harmonic \(D\) at faces).

## Eigenvalue solution

Power iteration on

\[
\mathbf{M}\,\boldsymbol{\phi}=\frac{1}{k}\mathbf{F}\,\boldsymbol{\phi}
\]

with \(\mathbf{M}\) the migration–removal–scatter operator and \(\mathbf{F}\) the fission operator.
Normalize \(\|\phi_2\|_2=1\) (or total fission source) each iteration; converge on \(k\) and flux.

## Benchmarks (see `data/PROVENANCE.md`)

| ID | Dim | Reference \(k_{\mathrm{eff}}\) | Notes |
|----|-----|----------------------------------|-------|
| Homogeneous 1D | 1D | Analytic / fine-mesh | Method of manufactured / buckling check |
| TWIGL seed-blanket | 2D | ~0.9132 (zero-flux) | Classic ANL space-time precursor; static here |
| IAEA 2D PWR | 2D | 1.029585 | Argonne Benchmark Problem Book |
| LRA BWR (static) | 2D | published static \(k\) | Static eigenvalue only in Phase 3 |

## Assumptions

1. Two energy groups; isotropic diffusion; no upscatter.
2. Steady state (no delayed precursors in the diffusion solve — kinetics remain Phase 1–2).
3. Rods modeled as material swaps / Σ perturbations in designated assemblies.
