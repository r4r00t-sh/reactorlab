# ADR 0007: Productization (Phase 6)

- **Status:** Accepted
- **Date:** 2026-11-23

## Context

Phases 0–5 delivered physics and GUI modules. Users need persistence,
export, documentation, and installable packages for Ubuntu.

## Decision

- **Session:** Qt `QJsonDocument` files (`.rlab.json`) — GUI concern only.
- **Export:** CSV trajectories (SI columns, documented headers); HDF5 deferred.
- **Scenarios:** YAML under `scenarios/`, loaded via **yaml-cpp**; GUI editor
  is a plain-text YAML editor with load/save (no physics in the editor).
- **Packaging:** CMake `install()` + CPack `DEB`/`TGZ`; optional AppImage script
  via linuxdeploy when available.
- **Version:** `0.7.0` for Phase 6 (first productized educational release).

## Consequences

- No new physics; product features wrap existing solvers.
- yaml-cpp becomes a required FetchContent/vcpkg dependency for scenario I/O.
- AppImage may need a local Qt tree; DEB assumes system Qt 6.
