# ReactorLab

Professional-grade **educational / research** nuclear reactor simulation for Linux (Ubuntu 22.04 / 24.04).

> **Disclaimer:** ReactorLab is **not** licensed, safety-grade, or suitable for real reactor design, licensing, or plant operations. Results must not be used for safety decisions.

## Phase status

| Phase | Scope | Status |
|-------|--------|--------|
| 0 | Infrastructure (build, CI, hello library, bindings, Qt shell) | Done |
| 1 | Point kinetics (6 delayed groups) | Done |
| 2 | Feedback, poisons, lumped thermal | Done |
| 3 | Spatial diffusion (TWIGL V&V) | Done |
| 4 | Monte Carlo transport (MG) | Done |
| 5 | Coupled multiphysics (Picard + IAPWS Region 1) | Done |
| 6 | Productization / packaging | Done |

## Architecture

- **`libreactorlab` (C++20):** physics core — no GUI, no Python includes
- **Python (nanobind):** scripting, reference implementations, plotting/reporting
- **Qt 6 Widgets GUI:** visualization and controls only; no physics logic
- **Build:** CMake ≥ 3.25, Ninja; deps via vcpkg manifest or FetchContent fallback

Stack decisions: [docs/adr/0001-dependency-and-gui-stack.md](docs/adr/0001-dependency-and-gui-stack.md).

## Quick start (Ubuntu)

See [docs/ubuntu-setup.md](docs/ubuntu-setup.md) for full package lists.

```bash
# With sudo: install system toolchain (recommended)
./scripts/ubuntu-install-deps.sh

# Or: use the project venv + FetchContent (no sudo for C++ deps; Qt still needed)
python3 -m venv venv
source venv/bin/activate
pip install -U pip -r requirements-dev.txt

cmake --preset dev \
  -DPython_EXECUTABLE=$PWD/venv/bin/python \
  -DQt6_DIR=$PWD/third_party/Qt/6.8.3/gcc_64/lib/cmake/Qt6   # if using local Qt
cmake --build --preset dev
ctest --preset dev --output-on-failure

# Python reference + C++ agreement tests
PYTHONPATH=python:build/dev/python pytest python/tests -q

# GUI (needs display / Qt)
./build/dev/gui/reactorlab_gui

# Packages (DEB + TGZ → dist/)
./scripts/package.sh
```

User guide: [docs/user-guide.md](docs/user-guide.md).

### Offline / no-sudo Qt

If `qt6-base-dev` cannot be installed system-wide:

```bash
source venv/bin/activate
pip install aqtinstall
aqt install-qt linux desktop 6.8.3 linux_gcc_64 -O third_party/Qt
cmake --preset dev -DQt6_DIR=$PWD/third_party/Qt/6.8.3/gcc_64/lib/cmake/Qt6
```

## Development standards

1. Equations & assumptions in `docs/` before code
2. Python reference in `python/reference/` before C++
3. Automated C++ ↔ Python / benchmark tests with documented tolerances
4. Nuclear data only from documented sources (`data/PROVENANCE.md`)
5. SI units internally

## License / use

Educational and research use only. See disclaimer above.
