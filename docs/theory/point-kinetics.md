# Point kinetics (6 delayed groups)

**Phase:** 1  
**Status:** Educational model — not for design or operations  
**Units:** SI throughout (time in s, power dimensionless relative to initial, reactivity in Δk/k)

## Assumptions

1. Spatially lumped (zero-dimensional) reactor; flux shape fixed.
2. One energy group effective; delayed neutrons represented by six precursor groups.
3. Prompt neutron generation time `Λ` and delayed fractions `βᵢ` are constant (no feedback in Phase 1).
4. Reactivity `ρ(t)` is an external schedule (step / ramp / sinusoid / scram).
5. Power `P` is normalized so `P(0) = 1` unless otherwise stated (relative power).

## Equations

With `β = Σᵢ βᵢ`:

\[
\frac{\mathrm{d}P}{\mathrm{d}t}
  = \frac{\rho(t) - \beta}{\Lambda}\, P
  + \sum_{i=1}^{6} \lambda_i C_i
\]

\[
\frac{\mathrm{d}C_i}{\mathrm{d}t}
  = \frac{\beta_i}{\Lambda}\, P - \lambda_i C_i,
  \qquad i = 1,\ldots,6
\]

Initial equilibrium at `ρ = 0`, `P(0) = P_0`:

\[
C_i(0) = \frac{\beta_i}{\lambda_i \Lambda}\, P_0
\]

State vector (7 ODEs): `[P, C₁, …, C₆]`.

## Reactivity schedules

| Name | Definition |
|------|------------|
| Step | `ρ(t) = ρ₀` for `t ≥ t₀` (usually `t₀ = 0`) |
| Ramp | `ρ(t) = ρ̇ · t` until optional cap, then hold |
| Sinusoidal | `ρ(t) = A sin(2π f t)` |
| Scram | From current `ρ`, insert `ρ_scram` (large negative) for `t ≥ t_scram` |

Reactivity is absolute `Δk/k` (not dollars) in the solver; the GUI may display `$ = ρ/β`.

## Analytic checks used for V&V

### Prompt jump (step `0 < ρ < β`)

Immediately after a positive step (prompt neutrons adjust; precursors frozen):

\[
\frac{P(0^+)}{P(0^-)} \approx \frac{\beta}{\beta - \rho}
\]

### Inhour equation (asymptotic period)

For constant `ρ` and asymptotic `P(t) ∝ e^{ω t}` with period `T = 1/ω`:

\[
\rho = \Lambda \omega + \sum_{i=1}^{6} \frac{\beta_i \omega}{\omega + \lambda_i}
\]

### Subcritical decay

For a large negative step (or scram) from equilibrium, power falls promptly then decays on delayed-neutron timescales; long-term slope tracks the slowest precursor group (`λ₁`).

## Parameters (default)

U-235 thermal six-group Keepin set — see `data/PROVENANCE.md` and `reactorlab::kinetics::u235_thermal_keepin()`.

Default prompt generation time: `Λ = 1×10⁻⁴ s` (educational thermal-reactor order of magnitude; documented in PROVENANCE).

## Numerical method

- **Python reference:** `scipy.integrate.solve_ivp` (BDF or Radau), tight tolerances.
- **C++:** SUNDIALS CVODE (BDF + Newton), absolute/relative tolerances documented in V&V reports.
