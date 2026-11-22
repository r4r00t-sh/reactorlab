# Thorium chain — inventory integrator + v1.1 coupling

**Educational AHWR-style representative model. Not for design, licensing, or safety.**

## Scope

### v1 — one-way side integrator

Driven by relative power \(P\) via \(\phi = \phi_0 P\):

\[
{}^{232}\mathrm{Th}(n,\gamma)\to{}^{233}\mathrm{Th}\xrightarrow{\beta}{}^{233}\mathrm{Pa}\xrightarrow{\beta}{}^{233}\mathrm{U}
\]

with Pa capture loss toward U-234 and U-233 fission/capture feeding U-234→U-235→U-236 (simplified).

### v1.1 — quasi-static two-way coupling (toggle)

Behind **FC couple** in the control centre:

1. Periodically recompute a fission-weighted delayed set and \(\beta_\mathrm{eff}\) from current inventory.
2. Feed \(\rho_\mathrm{inventory}\) (neutron-economy bias vs a reference inventory) into the plant ODE as a slowly varying parameter (`PlantParams::rho_inventory`).
3. Dollars use the current \(\beta_\mathrm{eff}\): \(\rho_\$ = \rho / \beta_\mathrm{eff}\).

Uncoupled mode keeps plant kinetics on the U-235 Keepin set and forces \(\rho_\mathrm{inventory}=0\); the side chain still advances.

Shutdown educational signature: as Pa-233 decays to U-233 at \(P=0\), \(\rho_\mathrm{inventory}\) **rises** (absorber → fissile).

## Time modes

| Mode | Behaviour |
|------|-----------|
| `transient` | Plant ODE + chain share \(\phi(P)\) and wall-clock \(\Delta t\) |
| `depletion` | Plant state frozen; chain advances with \(\Delta t_\mathrm{chain} = s\cdot\Delta t\) (`s` default 100) |

## Cross sections

Spectrum-averaged \(\sigma_c,\sigma_f\) for Th-232, Pa-233, U-233 (and probe U-235/Pu-239) come from OpenMC collapse on an educational AHWR-style pin cell using ENDF/B-VIII.0 (`scripts/openmc/`, `data/thorium/openmc_collapse/`).

Until the collapse artifact is compiled in (`REACTORLAB_AHWR_COLLAPSED_XS`), labeled **PLACEHOLDER** barns remain and release builds fail.

Literature cross-check (geometry + ENDF 2200 m/s sanity; no invented σ): see `data/thorium/openmc_collapse/CROSSCHECK.md`.

## Tests

- Analytic Bateman vs ODE at constant \(\phi\)
- Heavy-metal atom bookkeeping
- Inventory \(\rho\) rise on Pa→U-233 shutdown
- Coupled vs uncoupled plant \(\rho\) regression
- Fission-weighted \(\beta_\mathrm{eff}\) between parent sets
