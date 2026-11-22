# UI-7 plan — Mimic, alarms, scenarios, export

**Status:** Complete  
**Physics core:** unchanged — plant scenarios drive existing `rod_z` / `rho_ext`; CSV uses trend buffers.

## Scope

- **Thermal mimic:** educational loop schematic (core → pump → SG) with live P/T/rod.
- **Alarms:** HIGH POWER / FUEL T / COOLANT T / SHORT PERIOD / SCRAM — edge-logged, ackable.
- **Scenarios:** `manual`, `rod_step`, `scram_xe`, `doppler` (+ optional YAML plant_scram_xe).
- **Logs:** ring-buffer event list (play/pause/scram/alarm/scenario/export).
- **Export:** CSV of live trends via `reactorlab::io::write_csv`.

## Out of scope

- Full multiphysics scenario runner in the QML shell (Widgets GUI still has it).
- Safety-grade alarm management.
