# Delayed neutrons: β, ν_d, and β_eff

**Educational — not for design, licensing, or safety assessment.**

## Definitions

| Symbol | Meaning | Units |
|--------|---------|--------|
| \(\nu\) | Average neutrons emitted per fission (prompt + delayed) | — |
| \(\nu_d\) | Average **delayed** neutrons per fission | — |
| \(\beta\) | Delayed **fraction** of all neutrons from that fissile: \(\beta = \nu_d / \nu\) | — |
| \(\beta_i\) | Group delayed fractions with \(\sum_i \beta_i = \beta\) | — |
| \(\beta_{\mathrm{eff}}\) | **Effective** delayed fraction for the **core**, fission-rate weighted over fissiles present | — |

Point-kinetics equations in ReactorLab use \(\beta_i\) (and \(\beta=\sum\beta_i\)) in the usual Keepin form. **Dollars** of reactivity:

\[
\rho_\$ = \frac{\rho}{\beta_{\mathrm{eff}}}
\]

where \(\rho\) is in \(\Delta k/k\). Always state which \(\beta_{\mathrm{eff}}\) was used.

## Shipped sets

| Set | Spectrum | Primary table | Notes |
|-----|----------|---------------|-------|
| `u235_thermal_keepin` | thermal | Keepin U-235 | Existing Phase 1 |
| `u233_thermal_keepin` | thermal | Keepin U-233 (Phys. Rev. 107, 1957) | β ≈ 0.0028 ≪ U-235 |
| `pu239_fast_keepin` | fast | Keepin Pu-239 fast | PFBR-style stage |

Cross-checks vs ENDF/B and JEFF delayed-neutron evaluations are logged in `data/PROVENANCE.md` (differences expected; Keepin remains the **kinetics** primary for educational PK).

## Dynamic β_eff (inventory-weighted)

With fission rates \(F_j = N_j\,\sigma_{f,j}\,\phi\) for fissiles \(j\):

\[
\beta_{\mathrm{eff}} = \frac{\sum_j F_j\,\beta_j}{\sum_j F_j}
\]

v1: computed from the Th-chain side inventory (one-way). v1.1: may feed kinetics as a slowly varying parameter behind a toggle.
