#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace reactorlab::radiation {

inline constexpr std::size_t kNumGroups = 5;
inline constexpr double kRatedWatts = 3.0e9;           // educational P=1 → 3000 MWth
inline constexpr double kFissionsPerJoule = 3.1e10;  // ~200 MeV/fission

enum class GroupKind { Gamma, Beta, Alpha };

struct GroupDef {
  const char* name;
  GroupKind kind;
  double lambda;       // 1/s
  double yield;        // atoms per fission (proxy); A_eq [Bq] ≈ yield · F
  double gamma_Gamma;  // (µSv·m²)/(h·MBq) — educational
  double mu;           // 1/cm shield attenuation (γ carriers)
};

[[nodiscard]] const std::array<GroupDef, kNumGroups>& default_groups() noexcept;

struct Inventory {
  std::array<double, kNumGroups> activity_Bq{};
};

struct DoseSample {
  double r_m{1.0};
  double H_gamma_uSvh{0.0};
  double H_neutron_uSvh{0.0};
};

struct DoseParams {
  double buildup{1.5};
  double shield_cm{50.0};
  double mu_n{0.08};            // 1/cm educational neutron attenuation
  double neutron_k{2.5e3};      // µSv/h at 1 m for P=1, x=0
};

/// Advance inventory over dt_s given relative power P ∈ [0,∞).
void advance_inventory(Inventory& inv,
                       double P,
                       double dt_s,
                       const std::array<GroupDef, kNumGroups>& groups = default_groups());

[[nodiscard]] Inventory make_equilibrium_inventory(
    double P,
    const std::array<GroupDef, kNumGroups>& groups = default_groups());

[[nodiscard]] DoseSample dose_at(const Inventory& inv,
                                 double P,
                                 double r_m,
                                 const DoseParams& params = {},
                                 const std::array<GroupDef, kNumGroups>& groups = default_groups());

[[nodiscard]] std::vector<DoseSample> dose_curve(const Inventory& inv,
                                                 double P,
                                                 const std::vector<double>& r_m,
                                                 const DoseParams& params = {},
                                                 const std::array<GroupDef, kNumGroups>& groups = default_groups());

[[nodiscard]] double activity_by_kind(const Inventory& inv,
                                      GroupKind kind,
                                      const std::array<GroupDef, kNumGroups>& groups = default_groups());

[[nodiscard]] std::string kind_name(GroupKind k);

}  // namespace reactorlab::radiation
