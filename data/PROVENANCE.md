# Nuclear data provenance

**Rule:** Never invent nuclear data. Every numeric nuclear parameter shipped in
`data/` or hard-coded as a physics default must cite a textbook, OECD/IAEA
benchmark, or evaluated library (e.g. ENDF/B, OpenMC libraries).

| Dataset / constant | Source | Version / edition | Notes | Added |
|--------------------|--------|-------------------|-------|-------|
| U-235 thermal 6-group `λᵢ`, `βᵢ` | Keepin, G.R., *Physics of Nuclear Kinetics*, Addison-Wesley, 1965; tabulated in Duderstadt & Hamilton | Keepin thermal fission set | `β = Σ βᵢ = 0.0065` | 2026-11-23 |
| Prompt generation time `Λ = 1e-4 s` | Duderstadt & Hamilton; Lamarsh & Baratta (order-of-magnitude thermal) | Educational default | Not plant-specific | 2026-11-23 |
| I-135 / Xe-135: `λ_I=2.87e-5 /s`, `λ_Xe=2.09e-5 /s`, `γ_I=0.0639`, `γ_Xe=0.00237`, `σ_aXe=2.65e6 b` | Lamarsh & Baratta, *Introduction to Nuclear Engineering*; Duderstadt & Hamilton poison tables | Standard textbook thermal set | Half-lives ~6.57 h (I), ~9.14 h (Xe) | 2026-11-23 |
| Pm-149 / Sm-149: `λ_Pm=3.63e-6 /s`, `γ_Pm=0.0113`, `σ_aSm=4.1e4 b` | Lamarsh & Baratta; Stacey, *Nuclear Reactor Physics* | Textbook thermal set | Sm treated as stable | 2026-11-23 |
| `ν = 2.43` | Thermal U-235 average neutrons per fission (textbook) | Educational | Used in poison reactivity conversion | 2026-11-23 |
| `φ0 = 5e13 n/cm²/s` | Order-of-magnitude PWR thermal flux (Lamarsh) | Educational operating point | Sets Xe burnup vs decay | 2026-11-23 |
| `Σ_f0 = 0.1 /cm` | Order-of-magnitude macroscopic fission cross section in fuel (textbook PWR cell scale) | Educational | With `φ0` defines `F0=Σ_f0 φ0` | 2026-11-23 |
| Doppler `α_D = -2.5e-5 /K` | Typical UO₂ Doppler coefficient magnitude (~−2.5 pcm/K) cited in reactor-physics texts | Educational constant | Not enrichment/burnup-specific | 2026-11-23 |
| Moderator `α_M = -2.0e-5 /K` | Representative negative MTC order (PWR H₂O); texts quote tens of pcm/K depending on boron | Softened educational value | Chosen for numerical stability in lumped model; **not** a measured MTC | 2026-11-23 |
| Lumped `C_f, C_c, H, W, T_in, Q0` | Constructed so equilibrium `Tf0≈900 K`, `Tc0≈580 K` at `P=1` (PWR-like order) | Educational heat balance | **Not** a licensed plant thermal-hydraulic model | 2026-11-23 |
| Rod worth `w = 0.05` (Δk/k) | Educational bank worth (~7.7 $ with β=0.0065) | Scenario default | Replace with benchmark worth curves in later phases | 2026-11-23 |
| TWIGL 2G XS + geometry | Hageman & Yasinsky; OFELIA TWIGL2D.geo; Seamplex SP-MI-17-96D3 | Static seed/blanket | `νΣf=(0.007,0.2)` seed; `(0.003,0.06)` blanket; ref \(k=0.91321\) | 2026-11-23 |
| IAEA 2D PWR XS / ref \(k=1.029585\) | Argonne Benchmark Problem Book (ANL-7416) / IAEA 2D PWR | Educational YAML | Assembly map is a **placeholder** pending cell-exact audit | 2026-11-23 |
| LRA BWR static XS / ref \(k≈0.99636\) | Argonne LRA BWR benchmark documentation | Static-only Phase 3 | Map simplified; transient LRA deferred | 2026-11-23 |
| MG infinite 2G analytic \(k_\infty\) | Derived from documented Σ set in `mg_infinite.yaml` | Exact algebraic | Used for MC V&V (zero leakage) | 2026-11-23 |
| C5G7 | OECD/NEA C5G7 MOX benchmark | Phase 4 follow-up | Full pin model not gated in Phase 4 | 2026-11-23 |

When adding data files, update this table in the same commit.
