# Changelog

All notable changes to ReactorLab will be documented here.

## [0.8.0] — QML control centre (UI-0…UI-7)

### Added

- Qt Quick control centre (`gui/control_centre/`) with live `EngineWorker` / plant ODE facade
- Design system shell, KPIs, multi-trends, hex core + synced rod banks
- TWIGL×plant heatmaps, Quick3D cutaway, educational radiation inventory + γ/n dose
- Process mimic, ackable alarms, plant scenarios, event log, CSV export
- ADR 0008 (QML control-centre direction) + `docs/ui/` phase plans
- Core `reactorlab::radiation` educational dose helpers

### Notes

- Widgets GUI remains available; control centre is the primary interactive shell
- Radiation and spatial maps are educational approximations — not for safety/dose assessment

## [0.7.0] — Phase 6

### Added

- Session save/load (`.rlab.json`) and CSV trajectory export
- YAML scenarios under `scenarios/` + GUI scenario editor (text only)
- About / References / user guide (`docs/user-guide.md`)
- CMake install rules, desktop entry, CPack DEB+TGZ (`scripts/package.sh`)
- Optional AppImage helper (`scripts/package-appimage.sh`)
- Minimal scenario YAML parser + CSV writer in `libreactorlab` I/O

## [0.6.0] — Phase 5

### Added

- IAPWS-IF97 Region 1 liquid properties (ρ, cp) with Table 5 sample V&V
- 1D single-phase channel TH + lumped fuel + point kinetics
- Serial Picard (N→TH) coupling each Δt
- Scenarios: rod ejection, loss of flow, load-follow
- Python reference + C++ CVODE/Picard, bindings, GUI multiphysics panel

## [0.5.0] — Phase 4

### Added

- Multigroup analog Monte Carlo \(k\)-eigenvalue (infinite + 1D slab)
- OpenMP history-level parallelism with seeded per-history RNG streams
- Collision \(k\) estimator, track-length flux tallies, cycle \(\sigma\)
- Analytic \(k_\infty\) V&V; C++↔Python agreement; GUI MC panel
- C5G7 / OpenMC documented as optional follow-up (not build deps)

## [0.4.0] — Phase 3

### Added

- Two-group FD diffusion eigenvalue solver (Eigen / scipy)
- TWIGL 2D benchmark with ~10 pcm error at 40×40 vs reference 0.91321
- Python↔C++ TWIGL agreement tests
- GUI module switch: plant dashboard or TWIGL thermal-flux heatmap + rod toggle
- Benchmark YAML stubs for IAEA 2D PWR and LRA (maps flagged pending audit)

## [0.3.0] — Phase 2

### Added

- Coupled plant ODE (13 states): kinetics + I/Xe + Pm/Sm + fuel/coolant temperatures
- Doppler / moderator feedback, linear rod worth, critical bias for Xe equilibrium
- Python reference + xenon dip/peak and Doppler tests
- C++ CVODE plant solver, bindings, C++↔Python agreement tests
- GUI dashboard: P / T / Xe plots, rod slider, scenarios, alarms

## [0.2.0] — Phase 1

### Added

- Six-group point kinetics (Keepin U-235 thermal) with SUNDIALS CVODE
- Python reference (`python/reference/point_kinetics.py`) and analytic tests
- C++ ↔ Python agreement tests via nanobind
- Reactivity schedules: step, ramp, sinusoid, scram
- Qt Widgets GUI with QCustomPlot power trace, log/linear scale, run/pause/reset
- Theory note, ADR 0002, V&V report, provenance for Keepin parameters

## [0.1.0] — Phase 0

### Added

- CMake project with presets (`dev`, `release`, `asan`, `core-only`)
- `libreactorlab` hello/version API and GoogleTest suite
- nanobind module `reactorlab_core` with Python smoke test
- Qt 6 Widgets shell with educational disclaimer / About content
- vcpkg manifest, pre-commit hooks, clang-format/tidy, GitHub Actions CI
- Ubuntu setup docs and install script
- ADR 0001: dependency manager, Eigen-first solvers, Widgets + QCustomPlot plan
