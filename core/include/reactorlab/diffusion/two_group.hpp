#pragma once

#include <Eigen/Sparse>
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace reactorlab::diffusion {

struct TwoGroupXS {
  std::array<double, 2> D{1.0, 0.5};          // cm
  std::array<double, 2> Sigma_a{0.01, 0.1};   // 1/cm
  std::array<double, 2> nuSigma_f{0.0, 0.1};  // 1/cm
  double Sigma_s12{0.01};                     // 1/cm

  [[nodiscard]] double Sigma_r1() const noexcept { return Sigma_a[0] + Sigma_s12; }
};

struct DiffusionResult {
  double keff{0.0};
  int iterations{0};
  int nx{0};
  int ny{0};
  std::vector<double> phi1;  // row-major ny*nx
  std::vector<double> phi2;
};

enum class BoundaryCondition { ZeroFlux, MixedReflectiveZero };

[[nodiscard]] DiffusionResult solve_2d(const std::vector<int>& material_map,
                                       int nx,
                                       int ny,
                                       const std::vector<TwoGroupXS>& materials_by_id,
                                       double dx,
                                       double dy,
                                       BoundaryCondition bc = BoundaryCondition::ZeroFlux,
                                       double tol = 1e-7,
                                       int max_iter = 400);

/// Build classic TWIGL material map (ids 1,2,3) on an nx×ny mesh of the 80 cm domain.
[[nodiscard]] std::vector<int> twigl_material_map(int nx, int ny);

[[nodiscard]] DiffusionResult solve_twigl(int n = 40, double tol = 1e-7);

class DiffusionError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}  // namespace reactorlab::diffusion
