# ADR 0008: QML control-centre GUI (supersedes Widgets default)

- **Status:** Accepted (UI-0 approved 2026-11-23)
- **Date:** 2026-11-23
- **Supersedes (GUI portion):** ADR 0001 “GUI: Qt 6 Widgets”

## Context

Phases 0–6 shipped a Widgets + QCustomPlot dashboard. The product goal is now a
modern reactor control-centre with hex maps, GPU heatmaps, 3D cutaways, and a
tokenized dark theme. That stack is a poor fit for Widgets alone.

## Decision

- Adopt **Qt Quick / QML** for the product GUI shell.
- Keep **physics in libreactorlab**; GUI consumes snapshots / sends commands only.
- Add **Qt Quick 3D** when building UI-5 (install via aqt if missing).
- Charts: custom high-performance item or Qt Graphs; QCustomPlot as fallback bridge.
- Retain the existing Widgets binary as `reactorlab_gui_legacy` until the QML app
  reaches feature parity for plant/multi modules (optional; can drop later).

## Consequences

- New module set and CI packages (Quick, QuickControls2, Quick3D).
- Steeper QML skill need; better animation/layout for control-room UX.
- Requires EngineWorker + snapshot API before live data (UI-2).
