#include "reactorlab/kinetics/delayed_groups.hpp"

namespace reactorlab::kinetics {

DelayedGroupData u235_thermal_keepin() noexcept {
  // Keepin (1965) U-235 thermal — see data/PROVENANCE.md
  DelayedGroupData d;
  d.lambda_i = {0.0124, 0.0305, 0.111, 0.301, 1.14, 3.01};
  d.beta_i = {0.000215, 0.001424, 0.001274, 0.002568, 0.000748, 0.000273};
  d.Lambda = 1.0e-4;
  return d;
}

} // namespace reactorlab::kinetics
