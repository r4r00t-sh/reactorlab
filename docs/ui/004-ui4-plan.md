# UI-4 plan — Spatial heatmaps

**Status:** Complete  
**Physics core:** unchanged — one-shot `diffusion::solve_twigl` for a relative form factor; live plant scalars scale the maps.

## Approach

1. On worker start: solve TWIGL (n≈24) once → normalize thermal flux φ₂ to form factor `FF` (max=1).
2. Each snapshot:
   - `power_rel = FF × P`
   - `flux_rel = FF × P` (educational fluence proxy)
   - `tfuel_map = Tc + (Tf − Tc) × FF` (K)
   - `tcool_axial[z]` = synthetic axial coolant profile from plant `Tc` / peaking (labeled)
3. QML `HeatmapPanel`: Power / Flux / Tfuel / Tc-axial with viridis legend + approximation banner.

## Out of scope

- Live multiphysics Picard / assembly-wise neutronics.
- Quick3D (UI-5), radiation (UI-6).

## Acceptance

- Analysis page shows updating heatmaps while Play advances plant ODE.
- Maps resize correctly; legend min/max update.
- Copy states educational TWIGL×plant scaling — not a safety-grade spatial solver.
