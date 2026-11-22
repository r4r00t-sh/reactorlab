# Phase 6 — Definition of done

**Disclaimer:** Educational/research — not safety-grade.

## Deliverables

1. Session save/load (`.rlab.json`) from the GUI File menu.
2. CSV export of the active module’s time series.
3. YAML scenarios under `scenarios/` + load/edit dialog (no physics in editor).
4. About + References dialogs with disclaimer and citations.
5. User guide: `docs/user-guide.md`.
6. `cmake --install` installs GUI binary, headers, library, scenarios, docs.
7. CPack `DEB` + `TGZ` via `scripts/package.sh`.
8. AppImage helper script (uses linuxdeploy when present; else documents TGZ).
9. Version **0.7.0**; CHANGELOG + README phase table updated.
10. Existing ctest + pytest still green; new unit tests for CSV/scenario I/O.

## Out of scope

- Full HDF5 result archive (CSV is the Phase 6 export path).
- Windows/macOS installers.
- Signing / App Store distribution.
