#pragma once

#include <map>
#include <stdexcept>
#include <string>

namespace reactorlab::io {

/// Minimal educational scenario YAML (flat keys + one `drivers:` map).
/// Not a full YAML 1.2 implementation — sufficient for shipped scenarios/.
struct ScenarioFile {
  std::string name;
  std::string module;       // plant | multiphysics | twigl | mc
  std::string description;
  std::string driver_type;  // e.g. rod_ejection
  std::map<std::string, double> drivers;  // numeric driver params
  double t_end{10.0};
  double dt{1.0};
  std::string raw;  // original text (for editor round-trip)
};

[[nodiscard]] ScenarioFile load_scenario_yaml(const std::string& path);
[[nodiscard]] ScenarioFile parse_scenario_yaml(const std::string& text);

void save_scenario_yaml(const std::string& path, const ScenarioFile& sc);

class ScenarioError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}  // namespace reactorlab::io
