# Coupled plant model (Phase 2)

**Status:** Educational — not for design or operations  
**Units:** SI (time s, temperature K, reactivity Δk/k, power relative unless noted)

## State

\[
\mathbf{y}=
\bigl[P,\,C_1,\ldots,C_6,\,I,\,X,\,P_m,\,S_m,\,T_f,\,T_c\bigr]^\mathsf{T}
\]

(13 ODEs). Point kinetics as in `docs/theory/point-kinetics.md`, with **total** reactivity

\[
\rho(t)=\rho_{\mathrm{bias}}+\rho_{\mathrm{ext}}(t)+\rho_{\mathrm{rod}}(z)
+\alpha_D(T_f-T_{f0})+\alpha_M(T_c-T_{c0})
+\rho_{\mathrm{Xe}}(X)+\rho_{\mathrm{Sm}}(S_m).
\]

Here \(\rho_{\mathrm{bias}}=-\rho_{\mathrm{Xe}}^{\mathrm{eq}}(P{=}1)-\rho_{\mathrm{Sm}}^{\mathrm{eq}}(P{=}1)\) is a constant
educational stand-in for critical boron / fuel excess so that full-power equilibrium is critical
at \(T_{f0},T_{c0}\) with rods out.

## Control rods

Insertion fraction \(z\in[0,1]\) (0 = fully withdrawn):

\[
\rho_{\mathrm{rod}}=-w\,z
\]

with rod worth \(w>0\) in Δk/k (documented educational default).

## Xenon / iodine

With fission rate density at nominal power \(F_0=\Sigma_{f0}\varphi_0\) and relative power \(P\):

\[
\frac{\mathrm{d}I}{\mathrm{d}t}=\gamma_I F_0 P-\lambda_I I
\]

\[
\frac{\mathrm{d}X}{\mathrm{d}t}
=\gamma_X F_0 P+\lambda_I I-\lambda_X X-\sigma_{aX}\varphi_0 P\,X
\]

\[
\rho_{\mathrm{Xe}}=-\frac{\sigma_{aX}\,X}{\nu\Sigma_{f0}}
=-\frac{\sigma_{aX}\varphi_0}{\nu F_0}\,X
\]

Equilibrium at constant \(P\) recovers the classic high-flux limit
\(\rho_{X\infty}\approx-(\gamma_I+\gamma_X)/\nu\).

## Promethium / samarium

\[
\frac{\mathrm{d}P_m}{\mathrm{d}t}=\gamma_{Pm} F_0 P-\lambda_{Pm} P_m
\]

\[
\frac{\mathrm{d}S_m}{\mathrm{d}t}=\lambda_{Pm} P_m-\sigma_{aSm}\varphi_0 P\,S_m
\]

\[
\rho_{\mathrm{Sm}}=-\frac{\sigma_{aSm}\varphi_0}{\nu F_0}\,S_m
\]

(Sm-149 treated as stable.)

## Lumped fuel–coolant thermal model

\[
C_f\frac{\mathrm{d}T_f}{\mathrm{d}t}=P\,Q_0-H(T_f-T_c)
\]

\[
C_c\frac{\mathrm{d}T_c}{\mathrm{d}t}=H(T_f-T_c)-W(T_c-T_{\mathrm{in}})
\]

At \(P=1\) steady state, \((T_{f0},T_{c0})\) satisfy the algebraic heat balance.
Capacitances and conductances are **educational lumped values** (not a licensed plant model) — see `data/PROVENANCE.md`.

## Assumptions

1. Spatially lumped; single thermal flux level for poison burnup.
2. Feedback coefficients constant (no boron, no void).
3. Rod worth linear in insertion (no differential worth curve).
4. Decay heats neglected in the thermal nodes (Phase 2).

## V&V targets

1. Equilibrium Xe worth ≈ \(-(\gamma_I+\gamma_X)/\nu\) at high flux.
2. **Xenon dip:** after a step-down in power, Xe rises then falls — power undershoot / reactivity hole.
3. **Post-shutdown Xe peak:** after scram to \(P\approx0\), Xe peaks on the ~5–10 h scale then decays.
4. Doppler: positive step in \(\rho_{\mathrm{ext}}\) with \(\alpha_D<0\) raises \(T_f\) and reduces net \(\rho\).
5. C++ CVODE vs Python scipy BDF agreement.
