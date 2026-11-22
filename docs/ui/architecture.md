# Control-centre architecture

## Module layout

```
gui/control_centre/
  qml/Main.qml                 # ApplicationWindow; binds Shell to Engine
  src/main.cpp                 # injects EngineController as context property
  src/engine/                  # EngineWorker (QThread) + EngineController + snapshot
  run.sh
  imports/ReactorLab/ControlCentre/
    Theme.qml                  # design tokens (singleton)
    LayoutStore.qml            # layout presets / nav (singleton)
    DemoClock.qml              # qml-only fallback when Engine is absent
    Shell.qml
    chrome/                    # NavRail, TopBar, TransportBar
    cards/                     # Card, KpiCard, Sparkline, MultiTrendPanel, …
    layouts/                   # Overview, Operations, Analysis, Radiation
```

## Data path (UI-2)

```
QML  ←  EngineController (UI thread)  ←queued─  EngineWorker (QThread)
                                              └─ plant::advance / total_reactivity
```

Never call libreactorlab from QML. Commands: `play` / `pause` / `reset` / `scram` / `rodZ` / `speed`.

Rated display: `power_MW = P × 3000` (educational).

## Adding a widget

1. Create `cards/MyCard.qml` (or a layout section).
2. Register in `qmldir`.
3. Place it in a layout under `layouts/`.
4. Bind to `Engine` / `clock` properties — never call the core from QML.

## Theme

Edit `Theme.qml` only for global restyle. Dark/light via `Theme.dark`. Font scale via `Theme.fontScale`.

## Spatial maps (UI-4)

Worker solves TWIGL once → form factor; each snapshot scales by plant `P`/`Tf`/`Tc`. QML `HeatmapPanel` renders Power / Flux / Tfuel / Tc-axial with viridis.

## 3D cutaway (UI-5)

`Core3DView` uses Qt Quick 3D (half-core schematic). Offscreen platforms load a 2D fallback. Slice plane + orbit camera; rods / fuel tint follow live engine.

## Radiation (UI-6)

Core `reactorlab::radiation` inventory + γ/n point-source dose; α/β activity only. Radiation page carries a hard educational disclaimer.

## Ops pages (UI-7)

Thermal mimic, ackable alarms, plant scenarios, event log, CSV export (`io::write_csv`).

Control-centre GUI phases UI-0…UI-7 complete for the educational shell.
