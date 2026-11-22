# ReactorLab user guide

**Educational / research tool only.** Not licensed or safety-grade. Do not use
results for design, licensing, or plant operations.

## Install (from source)

```bash
./scripts/ubuntu-install-deps.sh   # or local Qt via aqtinstall
cmake --preset release -DPython_EXECUTABLE=$PWD/venv/bin/python
cmake --build --preset release
sudo cmake --install build/release --prefix /usr/local
```

Or build packages:

```bash
./scripts/package.sh          # → dist/*.deb and dist/*.tar.gz
# optional AppImage (needs linuxdeploy + local Qt):
./scripts/package-appimage.sh
```

Launch: `reactorlab_gui` (if installed) or `./build/release/gui/reactorlab_gui`.

## Modules

| Module | What it does |
|--------|----------------|
| Coupled plant | Point kinetics + Xe/Sm + lumped TH + feedback |
| TWIGL diffusion | 2D two-group eigenvalue / flux heatmap |
| Monte Carlo | Multigroup slab / infinite \(k\) |
| Multiphysics | Kinetics + 1D channel + IAPWS Region 1 (Picard) |

## Sessions & export

- **File → Save session / Load session** — restores module, scenario choice,
  window geometry (`.rlab.json`).
- **File → Export CSV** — writes the current plot time series (SI units).
- **File → Scenarios…** — browse, edit, and save YAML under `scenarios/`.

## Multiphysics scenarios

Built-ins: rod ejection (delayed supercritical), loss of flow, load-follow.
Drivers are also shipped as YAML in `scenarios/` for editing.

## Python

```bash
PYTHONPATH=python:build/release/python pytest python/tests -q
python -c "import reactorlab_core as rl; print(rl.version())"
```

## References

See **Help → References** in the GUI and `data/PROVENANCE.md` for nuclear data
sources. Theory notes live under `docs/theory/`; V&V under `docs/vv/`.
