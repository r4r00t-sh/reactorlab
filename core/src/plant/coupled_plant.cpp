#include "reactorlab/plant/coupled_plant.hpp"

#include "reactorlab/kinetics/point_kinetics.hpp"

#include <algorithm>
#include <cmath>
#include <cvode/cvode.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>

namespace reactorlab::plant {
namespace {

struct Ctx {
  const PlantParams* params{nullptr};
  const kinetics::ReactivityFn* rho_ext{nullptr};
  const RodFn* rod{nullptr};
};

void check(int flag, const char* what) {
  if (flag < 0) {
    throw PlantError(std::string("SUNDIALS failure in ") + what);
  }
}

double poison_rho_xe(double X, const PlantParams& p) {
  return -p.sigma_a_Xe * X / (p.nu * p.sigma_f0);
}

double poison_rho_sm(double Sm, const PlantParams& p) {
  return -p.sigma_a_Sm * Sm / (p.nu * p.sigma_f0);
}

int rhs_fn(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data) {
  auto* ctx = static_cast<Ctx*>(user_data);
  const auto& p = *ctx->params;
  const sunrealtype* yy = N_VGetArrayPointer(y);
  sunrealtype* yd = N_VGetArrayPointer(ydot);

  PlantState s = PlantState::from_array(static_cast<double>(t), {});
  // Fill from yy manually (from_array needs full array)
  std::array<double, kPlantStateSize> arr{};
  for (std::size_t i = 0; i < kPlantStateSize; ++i) {
    arr[i] = static_cast<double>(yy[i]);
  }
  s = PlantState::from_array(static_cast<double>(t), arr);

  const double P = std::max(s.P, 0.0);
  const double rho = total_reactivity(static_cast<double>(t), s, p, *ctx->rho_ext, *ctx->rod);
  const double beta = p.kinetics.beta();

  double precursor_src = 0.0;
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    precursor_src += p.kinetics.lambda_i[i] * s.C[i];
  }

  yd[0] = static_cast<sunrealtype>(((rho - beta) / p.kinetics.Lambda) * P + precursor_src);
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    yd[1 + i] = static_cast<sunrealtype>((p.kinetics.beta_i[i] / p.kinetics.Lambda) * P -
                                         p.kinetics.lambda_i[i] * s.C[i]);
  }

  const double F = p.F0() * P;
  const double phi = p.phi0 * P;
  yd[7] = static_cast<sunrealtype>(p.gamma_I * F - p.lambda_I * s.I);
  yd[8] = static_cast<sunrealtype>(p.gamma_Xe * F + p.lambda_I * s.I - p.lambda_Xe * s.X -
                                   p.sigma_a_Xe * phi * s.X);
  yd[9] = static_cast<sunrealtype>(p.gamma_Pm * F - p.lambda_Pm * s.Pm);
  yd[10] = static_cast<sunrealtype>(p.lambda_Pm * s.Pm - p.sigma_a_Sm * phi * s.Sm);
  yd[11] = static_cast<sunrealtype>((P * p.Q0() - p.H * (s.Tf - s.Tc)) / p.Cf);
  yd[12] = static_cast<sunrealtype>((p.H * (s.Tf - s.Tc) - p.W() * (s.Tc - p.Tin)) / p.Cc);
  return 0;
}

} // namespace

std::array<double, kPlantStateSize> PlantState::to_array() const {
  std::array<double, kPlantStateSize> y{};
  y[0] = P;
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    y[1 + i] = C[i];
  }
  y[7] = I;
  y[8] = X;
  y[9] = Pm;
  y[10] = Sm;
  y[11] = Tf;
  y[12] = Tc;
  return y;
}

PlantState PlantState::from_array(double t, const std::array<double, kPlantStateSize>& y) {
  PlantState s;
  s.t = t;
  s.P = y[0];
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    s.C[i] = y[1 + i];
  }
  s.I = y[7];
  s.X = y[8];
  s.Pm = y[9];
  s.Sm = y[10];
  s.Tf = y[11];
  s.Tc = y[12];
  return s;
}

void equilibrium_poisons(
    double P, const PlantParams& params, double& I, double& X, double& Pm, double& Sm) {
  const double Prest = std::max(P, 0.0);
  const double F = params.F0() * Prest;
  const double phi = params.phi0 * Prest;
  I = params.lambda_I > 0.0 ? params.gamma_I * F / params.lambda_I : 0.0;
  const double denom_x = params.lambda_Xe + params.sigma_a_Xe * phi;
  X = denom_x > 0.0 ? (params.gamma_Xe * F + params.lambda_I * I) / denom_x : 0.0;
  Pm = params.lambda_Pm > 0.0 ? params.gamma_Pm * F / params.lambda_Pm : 0.0;
  const double denom_s = params.sigma_a_Sm * phi;
  Sm = denom_s > 0.0 ? (params.lambda_Pm * Pm) / denom_s : 0.0;
}

double equilibrium_xenon_reactivity(double P, const PlantParams& params) {
  double I = 0.0;
  double X = 0.0;
  double Pm = 0.0;
  double Sm = 0.0;
  equilibrium_poisons(P, params, I, X, Pm, Sm);
  return poison_rho_xe(X, params);
}

double critical_bias(const PlantParams& params) {
  double I = 0.0;
  double X = 0.0;
  double Pm = 0.0;
  double Sm = 0.0;
  equilibrium_poisons(1.0, params, I, X, Pm, Sm);
  return -(poison_rho_xe(X, params) + poison_rho_sm(Sm, params));
}

double total_reactivity(double t,
                        const PlantState& s,
                        const PlantParams& params,
                        const kinetics::ReactivityFn& rho_ext,
                        const RodFn& rod) {
  const double z = std::clamp(rod(t), 0.0, 1.0);
  return critical_bias(params) + rho_ext(t) - params.rod_worth * z +
         params.alpha_D * (s.Tf - params.Tf0) + params.alpha_M * (s.Tc - params.Tc0) +
         poison_rho_xe(s.X, params) + poison_rho_sm(s.Sm, params);
}

PlantState make_equilibrium_state(double P0, const PlantParams& params) {
  PlantState s;
  s.t = 0.0;
  s.P = P0;
  s.C = kinetics::equilibrium_precursors(P0, params.kinetics);
  equilibrium_poisons(P0, params, s.I, s.X, s.Pm, s.Sm);
  s.Tf = params.Tf0;
  s.Tc = params.Tc0;
  return s;
}

PlantState advance(const PlantParams& params,
                   const kinetics::ReactivityFn& rho_ext,
                   const RodFn& rod,
                   const PlantState& s0,
                   double t1,
                   SolverTolerances tol) {
  if (t1 < s0.t) {
    throw PlantError("advance: t1 < s0.t");
  }
  if (t1 == s0.t) {
    return s0;
  }

  SUNContext sunctx = nullptr;
  check(SUNContext_Create(SUN_COMM_NULL, &sunctx), "SUNContext_Create");
  N_Vector y = N_VNew_Serial(static_cast<sunindextype>(kPlantStateSize), sunctx);
  if (!y) {
    SUNContext_Free(&sunctx);
    throw PlantError("N_VNew_Serial failed");
  }

  const auto arr0 = s0.to_array();
  sunrealtype* yy = N_VGetArrayPointer(y);
  for (std::size_t i = 0; i < kPlantStateSize; ++i) {
    yy[i] = static_cast<sunrealtype>(arr0[i]);
  }

  void* cvode_mem = CVodeCreate(CV_BDF, sunctx);
  Ctx ctx{&params, &rho_ext, &rod};
  check(CVodeInit(cvode_mem, rhs_fn, static_cast<sunrealtype>(s0.t), y), "CVodeInit");
  check(CVodeSetUserData(cvode_mem, &ctx), "CVodeSetUserData");
  check(CVodeSStolerances(cvode_mem, tol.rtol, tol.atol), "CVodeSStolerances");

  SUNMatrix A = SUNDenseMatrix(static_cast<sunindextype>(kPlantStateSize),
                               static_cast<sunindextype>(kPlantStateSize),
                               sunctx);
  SUNLinearSolver LS = SUNLinSol_Dense(y, A, sunctx);
  check(CVodeSetLinearSolver(cvode_mem, LS, A), "CVodeSetLinearSolver");

  sunrealtype t_current = static_cast<sunrealtype>(s0.t);
  const int flag = CVode(cvode_mem, static_cast<sunrealtype>(t1), y, &t_current, CV_NORMAL);

  PlantState out;
  if (flag >= 0) {
    std::array<double, kPlantStateSize> arr{};
    for (std::size_t i = 0; i < kPlantStateSize; ++i) {
      arr[i] = static_cast<double>(yy[i]);
    }
    out = PlantState::from_array(static_cast<double>(t_current), arr);
  }

  SUNLinSolFree(LS);
  SUNMatDestroy(A);
  CVodeFree(&cvode_mem);
  N_VDestroy(y);
  SUNContext_Free(&sunctx);

  if (flag < 0) {
    throw PlantError("CVode step failed");
  }
  return out;
}

PlantTrajectory integrate(const PlantParams& params,
                          const kinetics::ReactivityFn& rho_ext,
                          const RodFn& rod,
                          const std::vector<double>& t_out,
                          const PlantState& s0,
                          SolverTolerances tol) {
  if (t_out.empty()) {
    throw PlantError("t_out empty");
  }
  PlantState state = s0;
  PlantTrajectory traj;
  traj.t.reserve(t_out.size());
  traj.P.reserve(t_out.size());
  traj.X.reserve(t_out.size());
  traj.Tf.reserve(t_out.size());
  traj.Tc.reserve(t_out.size());
  traj.rho_total.reserve(t_out.size());

  for (double t_target : t_out) {
    state = advance(params, rho_ext, rod, state, t_target, tol);
    traj.t.push_back(state.t);
    traj.P.push_back(state.P);
    traj.X.push_back(state.X);
    traj.Tf.push_back(state.Tf);
    traj.Tc.push_back(state.Tc);
    traj.rho_total.push_back(total_reactivity(state.t, state, params, rho_ext, rod));
  }
  return traj;
}

} // namespace reactorlab::plant
