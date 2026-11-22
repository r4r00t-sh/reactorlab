# Coupled multiphysics (Phase 5)

**Status:** Educational — not for design, licensing, or operations  
**Units:** SI (Pa, K, kg/s, W, Δk/k)

## Physics pieces

1. **Neutronics:** six-group point kinetics (Phase 1) with total reactivity
   \[
   \rho=\rho_{\mathrm{bias}}+\rho_{\mathrm{ext}}(t)+\rho_{\mathrm{rod}}(z)
   +\alpha_D(T_f-T_{f0})+\alpha_M(\bar T_c-T_{c0}).
   \]
2. **Fuel:** lumped energy balance
   \(C_f\dot T_f = P\,Q_0 - H(T_f-\bar T_c)\).
3. **Coolant:** 1D single-phase channel, \(N\) axial nodes, mass flow \(\dot m(t)\):
   \[
   A\Delta z\,\rho_i c_{p,i}\,\dot T_{c,i}
   = \dot m c_{p}\,(T_{c,i-1}-T_{c,i})
   + \frac{H}{N}(T_f-T_{c,i}).
   \]
   Properties \(\rho(p,T)\), \(c_p(p,T)\) from **IAPWS-IF97 Region 1** (liquid).

## Coupling scheme (operator-split Picard)

Each time step \(\Delta t\):

1. Freeze \(\dot m(t)\), \(z(t)\), \(\rho_{\mathrm{ext}}(t)\).
2. **Picard loop** (max \(K\) iters, tol on \(\|T^{k+1}-T^k\|_\infty\)):
   - Advance kinetics + fuel ODEs over \(\Delta t\) with current \(\bar T_c\).
   - Update axial coolant nodes with new \(T_f\) and IAPWS properties at system pressure.
3. Accept state; log Picard iterations / residual.

This is a **serial** (neutronics → TH) Picard scheme — simple, educational, not a Jacobian-free Newton multiphysics solver.

## Scenarios

| Name | Drivers |
|------|---------|
| Rod ejection | \(z(t)\) drops rapidly (worth inserted → withdrawn) ⇒ positive \(\rho_{\mathrm{rod}}\) step |
| Loss of flow | \(\dot m(t)\) ramps down; power/flow mismatch heats coolant |
| Load-follow | Slow \(\rho_{\mathrm{ext}}(t)\) / rod schedule tracking a power demand band |

## Assumptions

1. Single-phase liquid only (Region 1); no boiling / void feedback.
2. Uniform pressure (imposed); no momentum PDE (prescribed \(\dot m\)).
3. Point kinetics (no spatial neutronics in the coupled loop).
4. Properties evaluated at node \(T\) and fixed \(p_{\mathrm{sys}}\).
