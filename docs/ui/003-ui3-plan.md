# UI-3 plan — Hex core map + synced rod banks

**Status:** Complete  
**Physics core:** unchanged — still one engine `rod_z`; banks are **UI mirrors**.

## Scope

- Educational hex lattice (fixed EDU-1 metadata in QML — not solved neutronics).
- Control / safety cells tinted by live `rod_z`.
- Rod bank panel: banks A/B/C/D/SA elevators all drive the same `Engine.rodZ`.
- Clear **SYNCED · educational** labeling until multi-bank API exists.
- Wire into **Operations** (primary) and **Overview** (compact map).

## Out of scope

- Multi-bank worth / independent z (engine addition B).
- Heatmaps / 3D / radiation / process mimic (UI-4+).

## Acceptance

- Operations shows interactive hex map + bank panel bound to live engine.
- Moving any bank slider or master control updates all banks and hex control cells.
- Scram drives all banks to full-in and map reflects it.
- Banner / copy never imply independent bank physics.
