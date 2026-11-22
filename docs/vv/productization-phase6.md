# V&V — Productization (Phase 6)

**Date:** 2026-11-23  
**Disclaimer:** Educational/research only.

| Check | Result |
|-------|--------|
| CSV writer unit test | PASS |
| Scenario YAML parse (rod ejection) | PASS |
| Load shipped `scenarios/*.yaml` | PASS |
| ctest (28) / pytest (35) | PASS |
| `cmake --install` (bin, scenarios, docs, desktop) | PASS |
| CPack DEB + TGZ `0.7.0` | PASS |
| GUI offscreen smoke | PASS (exit 124 = timeout) |

## Known limitations

- Scenario parser is a minimal YAML subset (not full YAML 1.2).
- CSV export only (HDF5 deferred).
- AppImage requires external `linuxdeploy`; otherwise use TGZ.
- Tagged git release is a maintainer step (`git tag v0.7.0`).
