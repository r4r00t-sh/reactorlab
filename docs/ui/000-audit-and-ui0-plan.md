# Step 0 audit + UI-0 plan — ReactorLab control centre

**Date:** 2026-11-23  
**Status:** Awaiting approval before UI-1  
**Disclaimer:** Educational/research UI. Not for operations or dose assessment.

---

## Step 0 — What exists today

### Repository / build

| Item | Finding |
|------|---------|
| Core | `libreactorlab` C++20 static lib — kinetics, plant, diffusion, MC, multiphysics, I/O |
| GUI | `gui/` — **Qt 6 Widgets** + vendored **QCustomPlot** |
| Binding | nanobind `reactorlab_core` (scripting; GUI does not use it) |
| ADR 0001 | Explicitly chose **Widgets** for Phases 0–2; QML deferred |
| Local Qt 6.8.3 | Has **Quick / Qml / Controls2 / OpenGL**; **no Quick3D**, **no Qt Graphs** |

### How the GUI talks to the engine (today)

- `MainWindow` **owns** `PlantParams` / `PlantState` / `MultiParams` / `MultiState` and calls `plant::advance` / `multiphysics::advance` / `diffusion::solve_*` / MC APIs **directly on the UI thread**.
- Drive loop: `QTimer` 50 ms → advance sim by `dt_sim_` (often accelerated) → append to `QCustomPlot` graphs.
- Commands: rod slider, ρ_ext spin, scenario combo, scram-like rod insert — all mutate local state then call `advance`.
- Persistence: `.rlab.json` session, CSV export, YAML scenario editor (text only).
- **No** view-model, **no** thread-safe snapshot queue, **no** separation of sim rate vs UI refresh.

### Engine data already exposed (usable by a thin GUI)

| Domain | State / outputs | Spatial? |
|--------|-----------------|----------|
| Point kinetics | `P`, `C[6]`, ρ schedules, inhour/prompt-jump helpers | No |
| Plant | `P`, poisons I/X/Pm/Sm, `Tf`, `Tc` (lumped), ρ_total, single `rod_z ∈ [0,1]` | No |
| Multiphysics | `P`, `Tf`, axial `Tc[N]`, `mdot`, Picard diagnostics, single `rod_z` | **1D axial coolant only** |
| Diffusion (TWIGL) | `keff`, `phi1`/`phi2` on **Cartesian** nx×ny | 2D flux only |
| Monte Carlo | `keff±σ`, group flux (slab/infinite) | 1D slab flux |
| IAPWS | ρ, cp Region 1 | Property table |
| Scenarios | YAML drivers + built-in rod ejection / LOF / load-follow | — |

### Gaps vs the desired control-centre views

| Desired view | Engine support today | Notes |
|--------------|----------------------|-------|
| KPI: power %, ρ pcm/$, period, k_eff, flow, P | Partial | `P` relative; ρ as Δk/k; period via inhour; k_eff from TWIGL/MC not live plant; **no pressure transient**; Q0→MW needs rated power convention |
| Multi-rod hex core + banks + scram animation | **Missing** | Only **one scalar** rod insertion `z` |
| Power / Tfuel / Xe **2D maps** | Missing (except TWIGL flux) | No assembly-wise power/temp/Xe fields |
| 3D core + slices | Missing | No 3D geometry in core |
| Radiation (α/β/γ/n dose & inventory) | **Absent** | Must not invent in UI |
| Process mimic (loop/pump/SG) | Partial scalars | Flow/T from multi; no nodal process model |
| Timeline replay of recorded runs | Partial | Trajectories exist in memory/CSV; no ring-buffer recorder API |
| Burnup / fission-rate maps | Missing | |

### Reuse vs change

**Reuse**

- All of `libreactorlab` physics APIs (call sites move behind a facade).
- Scenario YAML I/O, CSV export, educational disclaimer.
- QCustomPlot can remain as a fallback chart backend if Graphs unavailable.
- Design docs / V&V / provenance.

**Must change (GUI layer)**

- Replace Widgets monolith with QML shell + C++ view-models.
- Introduce `EngineFacade` / `ReactorSnapshot` + worker thread (sim ≠ UI Hz).
- Design-token Theme singleton; dockable card layouts.
- Install extra Qt modules for Quick3D (and optionally Graphs) via aqt.

**Must not change without approval**

- Physics equations / solvers inside `core/`.

---

## Tech choices (justified)

| Choice | Decision | Why |
|--------|----------|-----|
| UI framework | **Qt 6 Quick / QML** | Matches brief (cards, dock, 3D, shaders). Overrides ADR 0001 for visualization-heavy shell; keep Widgets only if embedding a legacy plot. |
| 3D | **Qt Quick 3D** | Native QML cutaways/slices; needs **aqtinstall** of `qtquick3d` (not in current prefix). |
| Charts | **Custom Scene Graph / Canvas chart first**; evaluate **Qt Graphs** after install | Graphs not in current Qt tree; QCustomPlot via `QQuickPaintedItem` is a proven backup for 100k-point trends with decimation. |
| Heatmaps | **GPU texture + fragment shader** (`ShaderEffect` / custom QSG) | Avoid thousands of QML items. |
| Hex map | **Single Scene Graph / Canvas item** with hit-testing | Interactive assemblies without one Item per cell. |
| Theme | **QML singleton `Theme`** (+ optional JSON tokens) | One place for colors/radii/type/motion. |
| Data path | **C++ `EngineWorker` → queued `ReactorSnapshot` → QObject view-models** | UI thread never calls CVODE. |

**Challenge to the brief (honest):** a full industrial hex-core + multi-bank rod UI **implies** engine state the project does not have. Shipping a beautiful hex map driven by one scalar `z` (all control rods synced) is acceptable for education **only if labeled**; true per-rod worth needs an approved engine extension.

---

## Minimal engine-side additions (ASK BEFORE IMPLEMENTING)

### A. Snapshot facade (recommended for UI-2; thin, no new physics)

```text
ReactorSnapshot {
  t, running, scenario_id,
  P, rho_pcm, period_s,  // period from inhour on current ρ
  Tf, Tc_bar, Tout, mdot, p_sys,
  X, Sm, xenon_worth,
  rod_z,               // [0,1] until multi-rod exists
  Tc_axial[],          // from multiphysics
  optional: last DiffusionResult / McResult fields
}
```

Commands: `setRodZ`, `scram`, `setRhoExt`, `play/pause/step`, `loadScenario`.

### B. Multi-rod / banks (needed for honest UI-3)

- Vector of bank positions + worths; `ρ_rod = −Σ worth_b · z_b`.
- Hex lattice metadata (pitch, bank map) as **data**, not solved neutronics.

### C. Radiation (needed for UI-6) — **STOP until approved**

Proposed minimal educational model (not for dose compliance):

1. **Inventory:** point/region fission-product groups with simple decay (γ/β emitters; α from actinide proxy groups).
2. **Dose (γ/n only):** `Ḣ = Σ_i A_i · Γ_i · B(E) · e^{−μx}/r²` with documented constants; neutron via simple fluence→dose factor.
3. **α/β:** inventory + activity only (no spatial dose map), with UI copy explaining short range.
4. Hard banner: *“Educational estimates — not for safety or dose assessment.”*

### D. Spatial thermal/power maps (UI-4)

- Either shape TWIGL φ into a relative power map, or a coarse assembly-wise overlay driven by point kinetics × form factors (labeled approximation).

---

## Proposed architecture

```text
┌─────────────────────────────────────────────────────────┐
│  QML Shell (Theme, NavRail, TopBar, Transport, Cards)   │
│     ↕ context properties / models                       │
│  ViewModels (KpiModel, TrendModel, CoreMapModel, …)     │
│     ↕ queued signals                                    │
│  EngineWorker (QThread)  ←→  libreactorlab (unchanged)  │
│     snapshots + command queue                           │
└─────────────────────────────────────────────────────────┘
```

Layouts: Overview / Operations / Analysis / Radiation (Radiation gated on C).

---

## UI-0 deliverables (this phase)

1. This audit + plan document.
2. ADR 0008 (QML control-centre direction).
3. **Clickable QML prototype** with **fake data** (`gui/proto/`):
   - Dark theme tokens
   - Left nav, top bar, bottom transport
   - KPI card row (sparklines)
   - One multi-parameter trend panel
   - Placeholder hex strip (non-engine)
4. How to run the prototype (below).

**Acceptance for UI-0:** you can open the prototype, switch nav pages, play/pause fake timeline, toggle theme — and we wait for your approval + answers on engine additions A–D before UI-1.

---

## Open questions for you

1. Approve **QML migration** (superseding ADR 0001 Widgets default)?
2. Approve installing **Qt Quick 3D** (+ optional Graphs) into the local aqt tree?
3. For rods until multi-bank exists: **synced single-z** (labeled) vs **block UI-3 until B**?
4. Approve radiation proposal **C**, reject, or defer UI-6 indefinitely?
5. Rated power for MW display (e.g. educational 3000 MWₜₕ ↔ `P=1`)?
