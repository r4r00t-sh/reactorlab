# V&V — Coupled plant (Phase 2)

**Date:** 2026-11-23  
**Disclaimer:** Educational/research only — not safety-grade.

## Configuration

| Item | Value |
|------|-------|
| Kinetics | Keepin U-235 thermal, `Λ=1e-4 s` |
| Poisons | I/Xe + Pm/Sm textbook set (`data/PROVENANCE.md`) |
| Feedback | `α_D=-2.5e-5 /K`, `α_M=-2.0e-5 /K` |
| Thermal | 2-node lumped fuel/coolant |
| Solver | CVODE BDF (C++) / scipy BDF (Python) |
| Critical bias | Cancels eq. Xe+Sm at P=1 (boron/excess stand-in) |

## Results

| Test | Criterion | Status |
|------|-----------|--------|
| Eq. Xe worth formula | Matches `-(γI+γX)/ν · σφ/(λ+σφ)` | PASS |
| Critical hold P=1 | `|P-1|<2e-3`, T near Tf0/Tc0 over 100 s | PASS |
| Post-shutdown Xe peak | Max Xe > 1.05×X0; peak in 2–15 h | PASS |
| Power-reduction Xe dip | Xe overshoots new equilibrium | PASS |
| Doppler step `ρ=0.0015` | Tf↑, P bounded `<5` | PASS |
| C++ vs Python hold 0–50 s | P, X, Tf, Tc within rtol/atol | PASS (pytest) |

## Known limitations

- No decay heat, boron chemistry, or void coefficient.
- `α_M` softened for lumped stability; not a measured MTC.
- Thermal nodes are educational, not a plant TH model.
- GUI time acceleration is for demonstration only.
