#include "reactorlab/diffusion/two_group.hpp"

#include <Eigen/SparseLU>
#include <cmath>

namespace reactorlab::diffusion {
namespace {

double face_D(double a, double b) {
  if (a <= 0.0 || b <= 0.0) {
    return 0.0;
  }
  return 2.0 * a * b / (a + b);
}

}  // namespace

std::vector<int> twigl_material_map(int nx, int ny) {
  std::vector<int> map(static_cast<std::size_t>(nx * ny), 3);
  const double dx = 80.0 / static_cast<double>(nx);
  const double dy = 80.0 / static_cast<double>(ny);
  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
      const double x = (static_cast<double>(i) + 0.5) * dx;
      const double y = (static_cast<double>(j) + 0.5) * dy;
      const bool in_cx = x >= 24.0 && x < 56.0;
      const bool in_cy = y >= 24.0 && y < 56.0;
      int id = 3;
      if (in_cx && in_cy) {
        id = 1;
      } else if ((x < 24.0 && in_cy) || (in_cx && y < 24.0)) {
        id = 2;
      }
      map[static_cast<std::size_t>(j * nx + i)] = id;
    }
  }
  return map;
}

DiffusionResult solve_2d(const std::vector<int>& material_map,
                         int nx,
                         int ny,
                         const std::vector<TwoGroupXS>& materials_by_id,
                         double dx,
                         double dy,
                         BoundaryCondition bc,
                         double tol,
                         int max_iter) {
  if (nx <= 0 || ny <= 0 ||
      material_map.size() != static_cast<std::size_t>(nx * ny)) {
    throw DiffusionError("invalid mesh / material map");
  }
  const int N = nx * ny;
  const int n_unk = 2 * N;

  auto idx = [nx](int i, int j) { return j * nx + i; };
  auto xs_of = [&](int i, int j) -> const TwoGroupXS& {
    const int id = material_map[static_cast<std::size_t>(idx(i, j))];
    if (id < 0 || id >= static_cast<int>(materials_by_id.size())) {
      throw DiffusionError("material id out of range");
    }
    return materials_by_id[static_cast<std::size_t>(id)];
  };

  std::vector<Eigen::Triplet<double>> trips_M;
  std::vector<Eigen::Triplet<double>> trips_F;
  trips_M.reserve(static_cast<std::size_t>(n_unk * 6));
  trips_F.reserve(static_cast<std::size_t>(N * 2));

  const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
      const auto& mat = xs_of(i, j);
      const int p = idx(i, j);
      const int r1 = p;
      const int r2 = N + p;
      double diag1 = mat.Sigma_r1();
      double diag2 = mat.Sigma_a[1];

      for (const auto& d : dirs) {
        const int ii = i + d[0];
        const int jj = j + d[1];
        const double h = (d[0] != 0) ? dx : dy;
        if (ii >= 0 && ii < nx && jj >= 0 && jj < ny) {
          const auto& mat_n = xs_of(ii, jj);
          const double a1 = face_D(mat.D[0], mat_n.D[0]) / (h * h);
          const double a2 = face_D(mat.D[1], mat_n.D[1]) / (h * h);
          const int q = idx(ii, jj);
          trips_M.emplace_back(r1, q, -a1);
          trips_M.emplace_back(r2, N + q, -a2);
          diag1 += a1;
          diag2 += a2;
        } else {
          bool reflective = false;
          if (bc == BoundaryCondition::MixedReflectiveZero) {
            if ((d[0] == -1 && i == 0) || (d[1] == -1 && j == 0)) {
              reflective = true;
            }
          }
          if (!reflective) {
            diag1 += 2.0 * mat.D[0] / (h * h);
            diag2 += 2.0 * mat.D[1] / (h * h);
          }
        }
      }

      trips_M.emplace_back(r1, r1, diag1);
      trips_M.emplace_back(r2, r2, diag2);
      trips_M.emplace_back(r2, r1, -mat.Sigma_s12);

      if (mat.nuSigma_f[0] != 0.0) {
        trips_F.emplace_back(r1, r1, mat.nuSigma_f[0]);
      }
      if (mat.nuSigma_f[1] != 0.0) {
        trips_F.emplace_back(r1, r2, mat.nuSigma_f[1]);
      }
    }
  }

  Eigen::SparseMatrix<double> M(n_unk, n_unk);
  Eigen::SparseMatrix<double> F(n_unk, n_unk);
  M.setFromTriplets(trips_M.begin(), trips_M.end());
  F.setFromTriplets(trips_F.begin(), trips_F.end());

  Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
  solver.compute(M);
  if (solver.info() != Eigen::Success) {
    throw DiffusionError("SparseLU factorization failed");
  }

  Eigen::VectorXd phi = Eigen::VectorXd::Ones(n_unk);
  Eigen::VectorXd s = F * phi;
  double snorm = s.norm();
  if (snorm == 0.0) {
    throw DiffusionError("zero fission source");
  }
  s /= snorm;
  double k = 1.0;
  int it = 0;
  for (it = 1; it <= max_iter; ++it) {
    phi = solver.solve(s);
    Eigen::VectorXd s_new = F * phi;
    const double sn_new = s_new.norm();
    const double k_new = sn_new;
    s_new /= sn_new;
    if (std::abs(k_new - k) / std::max(std::abs(k_new), 1e-12) < tol &&
        (s_new - s).norm() < tol) {
      k = k_new;
      break;
    }
    k = k_new;
    s = s_new;
  }

  DiffusionResult out;
  out.keff = k;
  out.iterations = it;
  out.nx = nx;
  out.ny = ny;
  out.phi1.resize(static_cast<std::size_t>(N));
  out.phi2.resize(static_cast<std::size_t>(N));
  for (int i = 0; i < N; ++i) {
    out.phi1[static_cast<std::size_t>(i)] = phi[i];
    out.phi2[static_cast<std::size_t>(i)] = phi[N + i];
  }
  return out;
}

DiffusionResult solve_twigl(int n, double tol) {
  TwoGroupXS seed;
  seed.D = {1.4, 0.4};
  seed.Sigma_a = {0.01, 0.15};
  seed.nuSigma_f = {0.007, 0.2};
  seed.Sigma_s12 = 0.01;

  TwoGroupXS blanket;
  blanket.D = {1.3, 0.5};
  blanket.Sigma_a = {0.008, 0.05};
  blanket.nuSigma_f = {0.003, 0.06};
  blanket.Sigma_s12 = 0.01;

  // Index materials by id (0 unused)
  std::vector<TwoGroupXS> mats(4);
  mats[1] = seed;
  mats[2] = seed;
  mats[3] = blanket;

  const auto map = twigl_material_map(n, n);
  const double h = 80.0 / static_cast<double>(n);
  return solve_2d(map, n, n, mats, h, h, BoundaryCondition::MixedReflectiveZero, tol);
}

}  // namespace reactorlab::diffusion
