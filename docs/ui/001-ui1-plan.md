# UI-1 plan — Design system & shell

**Status:** Complete  
**Prerequisite:** UI-0 approved (2026-11-23)

## Locked decisions

| Topic | Decision |
|-------|----------|
| GUI stack | QML control centre (ADR 0008 → Accepted) |
| Qt Quick 3D | Install into local aqt tree (for UI-5; shell builds without it) |
| Rods until banks | Single `rod_z`, UI may mirror across control banks **labeled as synced demo** |
| Radiation | Educational inventory + γ/n attenuation approved for **UI-6** (not this phase) |
| Rated power | `P = 1` ↔ **3000 MWₜₕ** (educational constant, documented) |

## Scope (UI-1 only)

1. Promote prototype into `gui/control_centre/` product shell (still **no live engine**).
2. `Theme` singleton: colors, radii, spacing, type, motion, dark/light, font scale.
3. Shell chrome: left nav, top bar, transport bar, page host.
4. Card grid + **saved layout presets** (Overview / Operations / Analysis / Radiation-gated).
5. Dockable/resizable panes via `SplitView` (Qt Quick Controls).
6. `qmllint` clean; offscreen launch smoke; brief architecture note.

## Out of scope

- EngineWorker / real snapshots (UI-2)
- Hex rods, heatmaps, 3D, radiation physics UI (UI-3+)

## Acceptance

- `./gui/control_centre/run.sh` opens dark shell; theme toggle works; nav switches layouts.
- Layout preset buttons rearrange card grid.
- Banner still says educational; Radiation page shows “approved for UI-6 / not wired”.
