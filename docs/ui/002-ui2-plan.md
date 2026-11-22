# UI-2 plan — Live engine plumbing + KPI / trends

**Status:** Complete  
**Physics core:** unchanged — GUI calls existing `plant::` / kinetics APIs only.

## Data path

```
QML  ←  EngineController (QObject)  ←queued─  EngineWorker (QThread)
                                              └─ plant::advance / total_reactivity
```

- Sim rate: worker timer (default 20 Hz physics steps with `dt` / speed).
- UI refresh: snapshots at ≤ 30 Hz via queued signal.
- Commands: play/pause/reset/scram/setRodZ/setRhoExt/setSpeed — never call CVODE on UI thread.

## Snapshot fields

`t, P, power_MW, rho, rho_pcm, period_s, Tf, Tc, X, xenon_worth_pcm, rod_z, running, state, master_alarm`

Rated: `power_MW = P × 3000` (educational).

## Acceptance

- Control centre binary links `libreactorlab` + Qt Quick.
- Play advances **real** plant ODE; KPIs and trend charts update.
- Scram sets `rod_z=1` and reflects in ρ / power.
- Banner no longer says “demo clock”.
