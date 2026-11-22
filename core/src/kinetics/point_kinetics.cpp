#include "reactorlab/kinetics/point_kinetics.hpp"

#include <algorithm>
#include <cmath>
#include <cvode/cvode.h>
#include <nvector/nvector_serial.h>
#include <stdexcept>
#include <sundials/sundials_types.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>

namespace reactorlab::kinetics {
namespace {

struct CvodeContext {
  const DelayedGroupData* data{nullptr};
  const ReactivityFn* rho{nullptr};
};

int rhs_fn(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data) {
  auto* ctx = static_cast<CvodeContext*>(user_data);
  const auto& d = *ctx->data;
  const double rho = (*ctx->rho)(static_cast<double>(t));
  const double beta = d.beta();

  const sunrealtype* yy = N_VGetArrayPointer(y);
  sunrealtype* yd = N_VGetArrayPointer(ydot);

  const double P = static_cast<double>(yy[0]);
  double precursor_source = 0.0;
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    precursor_source += d.lambda_i[i] * static_cast<double>(yy[1 + i]);
  }

  yd[0] = static_cast<sunrealtype>(((rho - beta) / d.Lambda) * P + precursor_source);
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    yd[1 + i] = static_cast<sunrealtype>((d.beta_i[i] / d.Lambda) * P -
                                         d.lambda_i[i] * static_cast<double>(yy[1 + i]));
  }
  return 0;
}

void check_sundials(int flag, const char* what) {
  if (flag < 0) {
    throw KineticsError(std::string("SUNDIALS failure in ") + what);
  }
}

} // namespace

std::array<double, kNumDelayedGroups> equilibrium_precursors(double P0,
                                                             const DelayedGroupData& data) {
  std::array<double, kNumDelayedGroups> C{};
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    C[i] = data.beta_i[i] / (data.lambda_i[i] * data.Lambda) * P0;
  }
  return C;
}

double prompt_jump_ratio(double rho, const DelayedGroupData& data) {
  const double beta = data.beta();
  if (!(rho > 0.0 && rho < beta)) {
    throw KineticsError("prompt jump requires 0 < rho < beta");
  }
  return beta / (beta - rho);
}

double inhour_reactivity(double omega, const DelayedGroupData& data) {
  double sum = data.Lambda * omega;
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    sum += data.beta_i[i] * omega / (omega + data.lambda_i[i]);
  }
  return sum;
}

double solve_inhour_omega(double rho, const DelayedGroupData& data) {
  const double lam_min = *std::min_element(data.lambda_i.begin(), data.lambda_i.end());
  double lo = 0.0;
  double hi = 0.0;
  if (rho < 0.0) {
    lo = -lam_min + 1e-12;
    hi = -1e-15;
  } else {
    lo = 1e-15;
    hi = 100.0;
  }

  auto f = [&](double w) { return inhour_reactivity(w, data) - rho; };
  double f_lo = f(lo);
  double f_hi = f(hi);
  if (rho >= 0.0 && f_lo * f_hi > 0.0) {
    for (int i = 0; i < 40; ++i) {
      hi *= 2.0;
      f_hi = f(hi);
      if (f_lo * f_hi <= 0.0) {
        break;
      }
    }
  }
  if (f_lo * f_hi > 0.0) {
    throw KineticsError("failed to bracket inhour root");
  }

  for (int i = 0; i < 80; ++i) {
    const double mid = 0.5 * (lo + hi);
    const double f_mid = f(mid);
    if (std::abs(f_mid) < 1e-14) {
      return mid;
    }
    if (f_lo * f_mid <= 0.0) {
      hi = mid;
      f_hi = f_mid;
    } else {
      lo = mid;
      f_lo = f_mid;
    }
  }
  return 0.5 * (lo + hi);
}

State make_equilibrium_state(double P0, const DelayedGroupData& data) {
  State s;
  s.t = 0.0;
  s.P = P0;
  s.C = equilibrium_precursors(P0, data);
  return s;
}

State advance(const DelayedGroupData& data,
              const ReactivityFn& rho,
              const State& s0,
              double t1,
              SolverTolerances tol) {
  if (t1 < s0.t) {
    throw KineticsError("advance: t1 must be >= s0.t");
  }
  if (t1 == s0.t) {
    return s0;
  }

  SUNContext sunctx = nullptr;
  check_sundials(SUNContext_Create(SUN_COMM_NULL, &sunctx), "SUNContext_Create");

  N_Vector y = N_VNew_Serial(static_cast<sunindextype>(kStateSize), sunctx);
  if (y == nullptr) {
    SUNContext_Free(&sunctx);
    throw KineticsError("N_VNew_Serial failed");
  }

  sunrealtype* yy = N_VGetArrayPointer(y);
  yy[0] = static_cast<sunrealtype>(s0.P);
  for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
    yy[1 + i] = static_cast<sunrealtype>(s0.C[i]);
  }

  void* cvode_mem = CVodeCreate(CV_BDF, sunctx);
  if (cvode_mem == nullptr) {
    N_VDestroy(y);
    SUNContext_Free(&sunctx);
    throw KineticsError("CVodeCreate failed");
  }

  CvodeContext ctx{&data, &rho};
  check_sundials(CVodeInit(cvode_mem, rhs_fn, static_cast<sunrealtype>(s0.t), y), "CVodeInit");
  check_sundials(CVodeSetUserData(cvode_mem, &ctx), "CVodeSetUserData");
  check_sundials(CVodeSStolerances(cvode_mem, tol.rtol, tol.atol), "CVodeSStolerances");

  SUNMatrix A = SUNDenseMatrix(
      static_cast<sunindextype>(kStateSize), static_cast<sunindextype>(kStateSize), sunctx);
  SUNLinearSolver LS = SUNLinSol_Dense(y, A, sunctx);
  check_sundials(CVodeSetLinearSolver(cvode_mem, LS, A), "CVodeSetLinearSolver");

  sunrealtype t_current = static_cast<sunrealtype>(s0.t);
  const int flag = CVode(cvode_mem, static_cast<sunrealtype>(t1), y, &t_current, CV_NORMAL);

  State out;
  if (flag >= 0) {
    out.t = static_cast<double>(t_current);
    out.P = static_cast<double>(yy[0]);
    for (std::size_t i = 0; i < kNumDelayedGroups; ++i) {
      out.C[i] = static_cast<double>(yy[1 + i]);
    }
  }

  SUNLinSolFree(LS);
  SUNMatDestroy(A);
  CVodeFree(&cvode_mem);
  N_VDestroy(y);
  SUNContext_Free(&sunctx);

  if (flag < 0) {
    throw KineticsError("CVode step failed");
  }
  return out;
}

Trajectory integrate(const DelayedGroupData& data,
                     const ReactivityFn& rho,
                     const std::vector<double>& t_out,
                     double P0,
                     double t0,
                     SolverTolerances tol) {
  if (t_out.empty()) {
    throw KineticsError("t_out must be non-empty");
  }
  if (t_out.front() < t0) {
    throw KineticsError("t_out must start at or after t0");
  }

  State state = make_equilibrium_state(P0, data);
  state.t = t0;

  Trajectory traj;
  traj.t.reserve(t_out.size());
  traj.P.reserve(t_out.size());
  traj.C.reserve(t_out.size());

  for (double t_target : t_out) {
    state = advance(data, rho, state, t_target, tol);
    traj.t.push_back(state.t);
    traj.P.push_back(state.P);
    traj.C.push_back(state.C);
  }
  return traj;
}

} // namespace reactorlab::kinetics
