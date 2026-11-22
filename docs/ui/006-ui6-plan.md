# UI-6 plan — Educational radiation

**Status:** Complete  
**Disclaimer (required):** Educational estimates — not for safety or dose assessment.

## Model (core `reactorlab::radiation`)

1. **Inventory:** lumped FP / actinide proxy groups with production ∝ thermal power and exponential decay.
2. **γ dose:** `Ḣ_γ = Σ_i A_i · Γ_i · B · e^{−μx} / r²` (point source + slab shield).
3. **n dose:** fluence→dose proxy ∝ `P · e^{−μ_n x} / r²`.
4. **α/β:** activity totals only — no spatial dose (short range).

## UI

- Hard red banner on Radiation page.
- Inventory bars by group / kind.
- γ/n dose vs distance curve + shield thickness control.
- α/β activity cards with range note.

## Out of scope

- Regulatory dose compliance, transport, cloud shine, UI-7 mimic.
