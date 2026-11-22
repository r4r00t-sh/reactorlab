# ADR 0001: Dependency manager, solvers, GUI, and plotting

- **Status:** Accepted (Phase 0)
- **Date:** 2026-11-23

## Context

ReactorLab needs reproducible scientific builds on Ubuntu 22.04/24.04, a Qt 6 desktop GUI with real-time plots, and a C++20 physics core that will later use sparse linear algebra and stiff ODE solvers.

## Decisions

### Dependency manager: **vcpkg (manifest mode)**

- `vcpkg.json` at the repo root is the source of truth for third-party C++ libraries.
- CMake consumes the vcpkg toolchain when `CMAKE_TOOLCHAIN_FILE` points at vcpkg.
- Phase 0 also supports a **FetchContent fallback** (`REACTORLAB_FETCH_DEPS=ON`) so developers without a bootstrapped vcpkg (or without `sudo` for system packages) can still build core + tests.
- Prefer system Qt when available (`find_package(Qt6)`); document local/offline Qt prefixes for CI and no-sudo machines.

**Why not Conan:** vcpkg integrates cleanly with CMake toolchain files and GitHub Actions caches; one less Python/tooling surface for C++ deps.

### Linear algebra / sparse solvers: **Eigen first; PETSc later (optional)**

- Phases 1–3 (point kinetics, lumped models, small diffusion meshes) are well served by Eigen dense/sparse + iterative solvers.
- PETSc is deferred until mesh size / multiphysics coupling justifies the MPI/install cost (likely Phase 3+ or Phase 5). Introduce behind a CMake option when needed.

### GUI: **Qt 6 Widgets (not QML)**

- Engineering panels (run/pause, tables, docked plots, dialogs) map naturally to Widgets.
- Physics stays in `libreactorlab`; the GUI only calls the public C++ API.
- QML remains an option for a future visualization-heavy shell; it is not the Phase 0–2 path.

### Plotting: **QCustomPlot for Phase 1+**

- Mature, lightweight, excellent for real-time 2D engineering traces (power, reactivity, xenon).
- Vendor as a single `.cpp`/`.h` pair under `gui/third_party/qcustomplot/` when Phase 1 needs plots.
- Phase 0 ships a Widgets shell only (no plot dependency yet).
- Qt Graphs is newer and QML-oriented; revisit if we adopt a QML visualization layer.

### Python bindings: **nanobind**

- Faster compile times and smaller binaries than pybind11 for a growing API surface.

### Units

- SI internally in the core. Public APIs document units in comments/Doxygen. Strong typing can be added later if misuse becomes a problem; avoid a heavy units library in Phase 0.

## Consequences

- CI builds with vcpkg (or FetchContent for a fast smoke job) + optional sanitizer presets.
- GUI developers need Qt 6 Widgets development packages or a local Qt prefix.
- No physics logic in `gui/`.
