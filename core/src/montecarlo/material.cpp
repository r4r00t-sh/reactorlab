#include "reactorlab/montecarlo/material.hpp"

namespace reactorlab::montecarlo {

MgMaterial material_2g_infinite() {
  MgMaterial m;
  m.n_groups = 2;
  m.Sigma_t = {0.06, 0.08};
  m.Sigma_a = {0.02, 0.08};
  m.nuSigma_f = {0.0, 0.10};
  m.Sigma_s = {};
  m.Sigma_s[0 * 2 + 1] = 0.04;
  return m;
}

double analytic_k_inf_2g(const MgMaterial& mat) {
  if (mat.n_groups != 2) {
    throw std::invalid_argument("analytic_k_inf_2g requires 2 groups");
  }
  const double Sr1 = mat.Sigma_a[0] + mat.Sigma_s_at(0, 1);
  return (mat.nuSigma_f[0] + mat.nuSigma_f[1] * mat.Sigma_s_at(0, 1) / mat.Sigma_a[1]) / Sr1;
}

}  // namespace reactorlab::montecarlo
