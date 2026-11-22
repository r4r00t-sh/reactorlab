#include "reactorlab/diffusion/two_group.hpp"
#include "reactorlab/hello.hpp"
#include "reactorlab/iapws/if97_region1.hpp"
#include "reactorlab/kinetics/point_kinetics.hpp"
#include "reactorlab/montecarlo/eigenvalue.hpp"
#include "reactorlab/montecarlo/material.hpp"
#include "reactorlab/multiphysics/coupled.hpp"
#include "reactorlab/plant/coupled_plant.hpp"
#include "reactorlab/version.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <utility>

namespace nb = nanobind;

NB_MODULE(reactorlab_core, m) {
  m.doc() = "ReactorLab core physics bindings";

  m.def(
      "version",
      []() { return std::string(reactorlab::version_string()); },
      "Return libreactorlab version string");

  m.def(
      "educational_disclaimer",
      []() { return std::string(reactorlab::educational_disclaimer()); },
      "Return the educational/not-safety-grade disclaimer");

  m.def(
      "hello",
      [](const std::string& name) { return reactorlab::hello(name); },
      nb::arg("name") = "ReactorLab",
      "Phase 0 greeting used as a binding smoke test");

  nb::class_<reactorlab::kinetics::DelayedGroupData>(m, "DelayedGroupData")
      .def(nb::init<>())
      .def_rw("lambda_i", &reactorlab::kinetics::DelayedGroupData::lambda_i)
      .def_rw("beta_i", &reactorlab::kinetics::DelayedGroupData::beta_i)
      .def_rw("Lambda", &reactorlab::kinetics::DelayedGroupData::Lambda)
      .def_prop_ro("beta", &reactorlab::kinetics::DelayedGroupData::beta);

  m.def("u235_thermal_keepin",
        &reactorlab::kinetics::u235_thermal_keepin,
        "Keepin U-235 thermal six-group parameters");

  m.def("prompt_jump_ratio",
        &reactorlab::kinetics::prompt_jump_ratio,
        nb::arg("rho"),
        nb::arg("data"),
        "Analytic prompt-jump P(0+)/P(0-)");

  m.def("inhour_reactivity",
        &reactorlab::kinetics::inhour_reactivity,
        nb::arg("omega"),
        nb::arg("data"),
        "Inhour equation rho(omega)");

  m.def("solve_inhour_omega",
        &reactorlab::kinetics::solve_inhour_omega,
        nb::arg("rho"),
        nb::arg("data"),
        "Dominant inhour root for rho");

  m.def(
      "integrate_point_kinetics",
      [](const reactorlab::kinetics::DelayedGroupData& data,
         double rho_step,
         const std::vector<double>& t_out,
         double P0) {
        auto traj = reactorlab::kinetics::integrate(
            data, reactorlab::kinetics::step_reactivity(rho_step), t_out, P0);
        return nb::make_tuple(std::move(traj.t), std::move(traj.P));
      },
      nb::arg("data"),
      nb::arg("rho_step"),
      nb::arg("t_out"),
      nb::arg("P0") = 1.0,
      "Integrate point kinetics for a constant reactivity step; returns (t, P)");

  // --- Phase 2: coupled plant ---
  nb::class_<reactorlab::plant::PlantParams>(m, "PlantParams").def(nb::init<>());

  m.def("equilibrium_xenon_reactivity",
        &reactorlab::plant::equilibrium_xenon_reactivity,
        nb::arg("P"),
        nb::arg("params"),
        "Equilibrium xenon reactivity at relative power P");

  m.def("critical_bias",
        &reactorlab::plant::critical_bias,
        nb::arg("params"),
        "Positive excess canceling equilibrium Xe+Sm at P=1");

  m.def(
      "integrate_coupled_plant",
      [](double rho_ext, double rod_z, const std::vector<double>& t_out, double P0) {
        reactorlab::plant::PlantParams params;
        auto s0 = reactorlab::plant::make_equilibrium_state(P0, params);
        auto traj = reactorlab::plant::integrate(
            params,
            reactorlab::kinetics::step_reactivity(rho_ext),
            [rod_z](double) { return rod_z; },
            t_out,
            s0);
        return nb::make_tuple(std::move(traj.t),
                              std::move(traj.P),
                              std::move(traj.X),
                              std::move(traj.Tf),
                              std::move(traj.Tc),
                              std::move(traj.rho_total));
      },
      nb::arg("rho_ext") = 0.0,
      nb::arg("rod_z") = 0.0,
      nb::arg("t_out"),
      nb::arg("P0") = 1.0,
      "Integrate coupled plant; returns (t, P, X, Tf, Tc, rho_total)");

  // --- Phase 3: diffusion ---
  m.def(
      "solve_twigl",
      [](int n) {
        auto res = reactorlab::diffusion::solve_twigl(n);
        return nb::make_tuple(res.keff, res.nx, res.ny, res.phi1, res.phi2, res.iterations);
      },
      nb::arg("n") = 40,
      "Solve TWIGL 2D; returns (keff, nx, ny, phi1, phi2, iterations)");

  // --- Phase 4: Monte Carlo ---
  m.def(
      "mc_infinite_keff",
      [](int n_histories, unsigned long long seed) {
        const auto mat = reactorlab::montecarlo::material_2g_infinite();
        auto res = reactorlab::montecarlo::infinite_medium_keff(
            mat, n_histories, static_cast<std::uint64_t>(seed));
        return nb::make_tuple(res.keff, res.keff_std);
      },
      nb::arg("n_histories") = 5000,
      nb::arg("seed") = 1,
      "Infinite-medium collision-estimator k_inf ± σ");

  m.def(
      "mc_slab_keff",
      [](double L, int n_histories, int n_inactive, int n_active, unsigned long long seed) {
        const auto mat = reactorlab::montecarlo::material_2g_infinite();
        reactorlab::montecarlo::SlabGeometry geom;
        geom.L = L;
        auto res = reactorlab::montecarlo::power_iteration_slab(
            mat,
            geom,
            n_histories,
            n_inactive,
            n_active,
            static_cast<std::uint64_t>(seed));
        return nb::make_tuple(res.keff, res.keff_std, res.flux, res.flux_std);
      },
      nb::arg("L") = 20.0,
      nb::arg("n_histories") = 2000,
      nb::arg("n_inactive") = 20,
      nb::arg("n_active") = 40,
      nb::arg("seed") = 42,
      "Homogeneous slab MC; returns (keff, keff_std, flux, flux_std)");

  m.def(
      "analytic_k_inf_2g",
      []() {
        return reactorlab::montecarlo::analytic_k_inf_2g(
            reactorlab::montecarlo::material_2g_infinite());
      },
      "Analytic 2-group infinite-medium k_inf");

  // --- Phase 5: IAPWS + multiphysics ---
  m.def(
      "iapws_region1",
      [](double p_pa, double T_K) {
        const auto prop = reactorlab::iapws::region1(p_pa, T_K);
        return nb::make_tuple(prop.v, prop.rho, prop.cp);
      },
      nb::arg("p_pa"),
      nb::arg("T_K"),
      "IAPWS-IF97 Region 1; returns (v, rho, cp)");

  m.def(
      "integrate_multiphysics",
      [](const std::string& scenario, const std::vector<double>& t_out) {
        reactorlab::multiphysics::MultiParams params;
        reactorlab::multiphysics::ScenarioDrivers sc;
        if (scenario == "rod_ejection") {
          sc = reactorlab::multiphysics::scenario_rod_ejection(1.0, 0.08, 0.2, params.mdot0);
        } else if (scenario == "lof") {
          sc = reactorlab::multiphysics::scenario_loss_of_flow(2.0, 0.4, 5.0, params.mdot0);
        } else if (scenario == "load_follow") {
          sc = reactorlab::multiphysics::scenario_load_follow(100.0, 0.0015, params.mdot0);
        } else {
          const double m0 = params.mdot0;
          sc.rho_ext = [](double) { return 0.0; };
          sc.rod_z = [](double) { return 0.0; };
          sc.mdot = [m0](double) { return m0; };
          sc.rod_z0 = 0.0;
        }
        auto s0 = reactorlab::multiphysics::make_equilibrium_state(1.0, params, sc.rod_z0);
        auto traj = reactorlab::multiphysics::integrate(
            params, sc.rho_ext, sc.rod_z, sc.mdot, t_out, s0);
        return nb::make_tuple(std::move(traj.t),
                              std::move(traj.P),
                              std::move(traj.Tf),
                              std::move(traj.Tc_bar),
                              std::move(traj.Tout),
                              std::move(traj.rho_total),
                              std::move(traj.mdot));
      },
      nb::arg("scenario") = "hold",
      nb::arg("t_out"),
      "Integrate multiphysics scenario; returns (t,P,Tf,Tc_bar,Tout,rho,mdot)");
}
