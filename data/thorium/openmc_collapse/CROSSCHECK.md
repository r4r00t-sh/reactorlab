# OpenMC AHWR-style cell collapse — literature cross-check

## Method

- Transport: OpenMC 0.15.3 with ENDF/B-VIII.0
- Geometry: edu AHWR-style equal-area pin cell (Kumar IAEA-CN-164-3S03 dims)
- Pitch: 3.0619 cm (225 mm lattice / √54 pins)
- k_eff (infinite cell): 1.50863 ± 0.00119

## Citations used for geometry / programme context

1. A. Kumar, R. Srivenkatesan, R. K. Sinha, *On the Physics Design of AHWR*,
   IAEA-CN-164-3S03 — Table I (pitch, pin OD, clad, PT/CT, enrichments).
2. IAEA TECDOC-1450, *Thorium fuel cycle — Potential benefits and challenges*
   — programme context; **does not publish** AHWR one-group microscopic σ.
3. IAEA ARIS AHWR characteristics — high-level plant description.

## What the papers do *not* provide

Kumar et al. and TECDOC-1450 do **not** tabulate collapsed one-group
microscopic capture/fission cross sections for Th-232 / Pa-233 / U-233
suitable for direct numeric substitution. Therefore no invented
"literature σ" values are entered. Cross-check is against:

- ENDF/B-VIII.0 2200 m/s point data (same evaluation family as the transport library)
- Qualitative AHWR lattice facts from Kumar (D2O moderated, LW cooled,
  Th/U-233 MOX ~3–3.75 wt% U-233, pitch 225 mm)

## Collapsed one-group σ [barn] vs ENDF 2200 m/s

| Reaction | OpenMC σ̄ [b] | ENDF 2200 m/s [b] | Ratio σ̄/σ₀ |
|----------|---------------|-------------------|-------------|
| Th232_capture_b | 1.7263 | 7.34 | 0.235 |
| Pa233_capture_b | 31.0505 | 42.49 | 0.731 |
| U233_fission_b | 105.2634 | 533.91 | 0.197 |
| U233_capture_b | 11.6999 | 42.33 | 0.276 |
| U235_fission_b | 93.1327 | 586.44 | 0.159 |
| U235_capture_b | 19.7447 | 99.34 | 0.199 |
| Pu239_fission_b | 234.3945 | 747.46 | 0.314 |
| Pu239_capture_b | 131.9007 | 270.40 | 0.488 |

### Discrepancy notes

- Spectrum-averaged σ̄ for 1/v absorbers is typically **below** 2200 m/s
  values in a hardened AHWR-like spectrum (pitch reduction / LW coolant).
- 2200 m/s points are evaluated from the **same** ENDF/B-VIII.0 HDF5 files
  used in transport (`IncidentNeutron` at 0.0253 eV) — not hand-entered.
- Trace U-235 / Pu-239 probes share the Th/U-233 cell spectrum; rates are
  for that spectrum, not a Pu-MOX pin spectrum (outer AHWR ring differs).
- Infinite-cell \(k_\mathrm{eff}\approx1.51\) is expected for this simplified
  pin cell (no PT/CT/displacer grey absorbers, reflective boundaries); it is
  **not** an AHWR core eigenvalue claim.

## Provenance rule

Shipped YAML/C++ defaults use **only** the OpenMC-collapsed numbers in
`ahwr_edu_cell_onegroup.json`. No hand-tuned σ were introduced.

