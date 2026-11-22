# Educational radiation model

**Not for safety, licensing, or personal dose assessment.**

## Inventory

Lumped groups with production rate `Y_i · F` and decay `λ_i`:

\[
\frac{dA_i}{dt} = Y_i F - \lambda_i A_i
\]

where `F` is an educational fission rate from thermal power (`P × 3000 MWₜₕ`).

## γ dose rate

Point-source estimate with a slab shield of thickness `x`:

\[
\dot H_\gamma(r) = \sum_i A_i\,\Gamma_i\,B\,e^{-\mu x}/r^2
\]

Constants `Γ`, `B`, `μ` are illustrative, not nuclide-evaluated libraries.

## Neutron dose rate

\[
\dot H_n(r) \propto P\,e^{-\mu_n x}/r^2
\]

## α / β

Reported as **activity only**. No spatial dose map — range in air is centimetres or less.
