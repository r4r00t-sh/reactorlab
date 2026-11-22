# UI-5 plan — Quick3D core cutaway

**Status:** Complete  
**Physics core:** unchanged — geometry is educational; colors follow live `P` / `rod_z` / `Tf`.

## Scope

- Qt Quick 3D half-core cutaway with vessel shell, fuel columns, synced control rods.
- Movable horizontal slice plane.
- Orbit camera (drag); gentle auto-rotate when idle.
- Analysis page replaces the UI-5 placeholder.
- Offscreen / no-GL: Loader falls back to a 2D cutaway card (smoke-safe).

## Out of scope

- CAD-accurate internals, radiation (UI-6), process mimic (UI-7).

## Acceptance

- With a normal display, Analysis shows an interactive 3D core tinted by live power; rods track `rod_z` / SCRAM.
- Slice slider moves the plane.
- Banner/copy: educational geometry — not as-built plant.
