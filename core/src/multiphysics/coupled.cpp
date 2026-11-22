#include "reactorlab/multiphysics/coupled.hpp"

#include "reactorlab/iapws/if97_region1.hpp"
#include "reactorlab/kinetics/point_kinetics.hpp"

#include <algorithm>
#include <cmath>
#include <cvode/cvode.h>
#include <nvector/nvector_serial.h>
#include <numeric>
#include <sundials/sundials_types.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>

namespace reactorlab::multiphysics {
namespace {

struct NfCtx {
  const MultiParams* params{nullptr};
  const DriverFn* rho_ext{nullptr};
  const DriverFn* rod_z{nullptr};
  double Tc_bar{580.0};
};

void check(int flag, const char* what) {
  if (flag < 0) {
    throw MultiError(std::string("SUNDIALS failure in ") + what);
  }
}

int nf_rhs(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data) {
  auto* ctx = static_cast<NfCtx*>(user_data);
  const auto& p = *ctx->params;
  const sunrealtype* yy = N_VGetArrayPointer(y);
  sunrealtype* yd = N_VGetArrayPointer(ydot);

  const double P = std::max(static_cast<double>(yy[0]), 0.0);
  std::array<double, kinetics::kNumDelayedGroups> C{};
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    C[i] = static_cast<double>(yy[1 + i]);
  }
  const double Tf = static_cast<double>(yy[7]);
  const double rho =
      total_reactivity(static_cast<double>(t), P, Tf, ctx->Tc_bar, p, *ctx->rho_ext, *ctx->rod_z);
  const double beta = p.kinetics.beta();

  double precursor_src = 0.0;
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    precursor_src += p.kinetics.lambda_i[i] * C[i];
  }
  yd[0] = static_cast<sunrealtype>(((rho - beta) / p.kinetics.Lambda) * P + precursor_src);
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    yd[1 + i] = static_cast<sunrealtype>((p.kinetics.beta_i[i] / p.kinetics.Lambda) * P -
                                         p.kinetics.lambda_i[i] * C[i]);
  }
  yd[7] = static_cast<sunrealtype>((P * p.Q0() - p.H * (Tf - ctx->Tc_bar)) / p.Cf);
  return 0;
}

void advance_nf(const MultiParams& params,
                const DriverFn& rho_ext,
                const DriverFn& rod_z,
                double Tc_bar,
                double t0,
                double t1,
                double& P,
                std::array<double, kinetics::kNumDelayedGroups>& C,
                double& Tf,
                SolverTolerances tol) {
  if (t1 <= t0) {
    return;
  }
  NfCtx ctx{&params, &rho_ext, &rod_z, Tc_bar};

  SUNContext sunctx = nullptr;
  check(SUNContext_Create(SUN_COMM_NULL, &sunctx), "SUNContext_Create");
  N_Vector y = N_VNew_Serial(static_cast<sunindextype>(kNfStateSize), sunctx);
  if (y == nullptr) {
    SUNContext_Free(&sunctx);
    throw MultiError("N_VNew_Serial failed");
  }
  sunrealtype* yy = N_VGetArrayPointer(y);
  yy[0] = static_cast<sunrealtype>(P);
  for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
    yy[1 + i] = static_cast<sunrealtype>(C[i]);
  }
  yy[7] = static_cast<sunrealtype>(Tf);

  void* cvode = CVodeCreate(CV_BDF, sunctx);
  if (cvode == nullptr) {
    N_VDestroy(y);
    SUNContext_Free(&sunctx);
    throw MultiError("CVodeCreate failed");
  }
  check(CVodeInit(cvode, nf_rhs, static_cast<sunrealtype>(t0), y), "CVodeInit");
  check(CVodeSStolerances(cvode, tol.rtol, tol.atol), "CVodeSStolerances");
  check(CVodeSetUserData(cvode, &ctx), "CVodeSetUserData");

  SUNMatrix A = SUNDenseMatrix(static_cast<sunindextype>(kNfStateSize),
                               static_cast<sunindextype>(kNfStateSize),
                               sunctx);
  SUNLinearSolver LS = SUNLinSol_Dense(y, A, sunctx);
  check(CVodeSetLinearSolver(cvode, LS, A), "CVodeSetLinearSolver");

  sunrealtype tret = static_cast<sunrealtype>(t0);
  const int flag = CVode(cvode, static_cast<sunrealtype>(t1), y, &tret, CV_NORMAL);

  if (flag >= 0) {
    P = std::max(static_cast<double>(yy[0]), 0.0);
    for (std::size_t i = 0; i < kinetics::kNumDelayedGroups; ++i) {
      C[i] = static_cast<double>(yy[1 + i]);
    }
    Tf = static_cast<double>(yy[7]);
  }

  SUNLinSolFree(LS);
  SUNMatDestroy(A);
  CVodeFree(&cvode);
  N_VDestroy(y);
  SUNContext_Free(&sunctx);

  if (flag < 0) {
    throw MultiError("CVode step failed");
  }
}

std::vector<double> update_coolant(const std::vector<double>& Tc,
                                   double Tf,
                                   double mdot,
                                   double dt,
                                   const MultiParams& params) {
  const std::size_t n = params.n_nodes;
  std::vector<double> out(n);
  for (std::size_t i = 0; i < n; ++i) {
    const double T_old = Tc[i];
    const auto prop = iapws::region1(params.p_sys, T_old);
    const double Cap = prop.rho * prop.cp * params.A * params.dz() / dt;
    const double T_up = (i == 0) ? params.Tin : out[i - 1];
    const double m_cp = mdot * prop.cp;
    const double Hn = params.H / static_cast<double>(n);
    const double denom = Cap + m_cp + Hn;
    out[i] = (Cap * T_old + m_cp * T_up + Hn * Tf) / denom;
  }
  return out;
}

}  // namespace

double MultiState::Tc_bar() const {
  if (Tc.empty()) {
    return 0.0;
  }
  return std::accumulate(Tc.begin(), Tc.end(), 0.0) / static_cast<double>(Tc.size());
}

double MultiState::Tout() const {
  return Tc.empty() ? 0.0 : Tc.back();
}

double total_reactivity(double t,
                        double /*P*/,
                        double Tf,
                        double Tc_bar,
                        const MultiParams& params,
                        const DriverFn& rho_ext,
                        const DriverFn& rod_z) {
  return params.rho_bias + rho_ext(t) - params.rod_worth * rod_z(t) +
         params.alpha_D * (Tf - params.Tf0) + params.alpha_M * (Tc_bar - params.Tc0);
}

MultiState make_equilibrium_state(double P0, MultiParams& params, double rod_z0) {
  MultiState s;
  s.t = 0.0;
  s.P = P0;
  s.C = kinetics::equilibrium_precursors(P0, params.kinetics);
  s.Tf = params.Tf0;
  s.Tc.assign(params.n_nodes, 0.0);
  const double Tout = 2.0 * params.Tc0 - params.Tin;
  for (std::size_t i = 0; i < params.n_nodes; ++i) {
    const double frac = (static_cast<double>(i) + 0.5) / static_cast<double>(params.n_nodes);
    s.Tc[i] = params.Tin + frac * (Tout - params.Tin);
  }
  const double mean = s.Tc_bar();
  for (double& T : s.Tc) {
    T += params.Tc0 - mean;
  }
  params.rho_bias = params.rod_worth * rod_z0 - params.alpha_M * (s.Tc_bar() - params.Tc0);
  return s;
}

MultiState advance(const MultiParams& params,
                   const DriverFn& rho_ext,
                   const DriverFn& rod_z,
                   const DriverFn& mdot_fn,
                   const MultiState& s0,
                   double t1,
                   SolverTolerances tol) {
  if (t1 <= s0.t) {
    throw MultiError("t1 must be > state.t");
  }
  const double dt = t1 - s0.t;
  const double mdot = std::max(mdot_fn(s0.t), 1.0);
  std::vector<double> Tc = s0.Tc;
  double P = s0.P;
  auto C = s0.C;
  double Tf = s0.Tf;
  double res = 0.0;
  int it = 0;
  for (it = 1; it <= params.picard_max; ++it) {
    const double Tc_bar = [&] {
      double sum = 0.0;
      for (double T : Tc) {
        sum += T;
      }
      return sum / static_cast<double>(Tc.size());
    }();
    P = s0.P;
    C = s0.C;
    Tf = s0.Tf;
    advance_nf(params, rho_ext, rod_z, Tc_bar, s0.t, t1, P, C, Tf, tol);
    auto Tc_new = update_coolant(s0.Tc, Tf, mdot, dt, params);
    res = 0.0;
    for (std::size_t i = 0; i < Tc_new.size(); ++i) {
      res = std::max(res, std::abs(Tc_new[i] - Tc[i]));
    }
    Tc = std::move(Tc_new);
    if (res < params.picard_tol) {
      break;
    }
  }
  MultiState out;
  out.t = t1;
  out.P = P;
  out.C = C;
  out.Tf = Tf;
  out.Tc = std::move(Tc);
  out.picard_iters = it;
  out.picard_res = res;
  return out;
}

MultiTrajectory integrate(const MultiParams& params,
                          const DriverFn& rho_ext,
                          const DriverFn& rod_z,
                          const DriverFn& mdot_fn,
                          const std::vector<double>& t_out,
                          const MultiState& s0,
                          SolverTolerances tol) {
  MultiTrajectory traj;
  traj.t = t_out;
  traj.P.resize(t_out.size());
  traj.Tf.resize(t_out.size());
  traj.Tc_bar.resize(t_out.size());
  traj.Tout.resize(t_out.size());
  traj.rho_total.resize(t_out.size());
  traj.mdot.resize(t_out.size());
  traj.picard_iters.resize(t_out.size());
  MultiState s = s0;
  for (std::size_t i = 0; i < t_out.size(); ++i) {
    if (t_out[i] > s.t + 1e-15) {
      s = advance(params, rho_ext, rod_z, mdot_fn, s, t_out[i], tol);
    }
    traj.P[i] = s.P;
    traj.Tf[i] = s.Tf;
    traj.Tc_bar[i] = s.Tc_bar();
    traj.Tout[i] = s.Tout();
    traj.rho_total[i] =
        total_reactivity(s.t, s.P, s.Tf, s.Tc_bar(), params, rho_ext, rod_z);
    traj.mdot[i] = mdot_fn(s.t);
    traj.picard_iters[i] = s.picard_iters;
  }
  return traj;
}

ScenarioDrivers scenario_rod_ejection(double t_eject, double z0, double duration, double mdot0) {
  ScenarioDrivers d;
  d.rod_z0 = z0;
  d.rho_ext = [](double) { return 0.0; };
  d.mdot = [mdot0](double) { return mdot0; };
  d.rod_z = [t_eject, z0, duration](double t) {
    if (t < t_eject) {
      return z0;
    }
    if (t >= t_eject + duration) {
      return 0.0;
    }
    return z0 * (1.0 - (t - t_eject) / duration);
  };
  return d;
}

ScenarioDrivers scenario_loss_of_flow(double t_start, double fraction, double ramp, double mdot0) {
  ScenarioDrivers d;
  d.rod_z0 = 0.0;
  d.rho_ext = [](double) { return 0.0; };
  d.rod_z = [](double) { return 0.0; };
  d.mdot = [t_start, fraction, ramp, mdot0](double t) {
    if (t < t_start) {
      return mdot0;
    }
    if (t >= t_start + ramp) {
      return fraction * mdot0;
    }
    const double x = (t - t_start) / ramp;
    return mdot0 * (1.0 - (1.0 - fraction) * x);
  };
  return d;
}

ScenarioDrivers scenario_load_follow(double period, double amp, double mdot0) {
  ScenarioDrivers d;
  d.rod_z0 = 0.0;
  d.rod_z = [](double) { return 0.0; };
  d.mdot = [mdot0](double) { return mdot0; };
  d.rho_ext = [period, amp](double t) {
    return amp * std::sin(2.0 * 3.14159265358979323846 * t / period);
  };
  return d;
}

}  // namespace reactorlab::multiphysics
