#include "reactorlab/io/scenario_yaml.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

namespace reactorlab::io {
namespace {

std::string trim(std::string s) {
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
    s.erase(s.begin());
  }
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
    s.pop_back();
  }
  // strip matching quotes
  if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                        (s.front() == '\'' && s.back() == '\''))) {
    s = s.substr(1, s.size() - 2);
  }
  return s;
}

bool is_comment_or_empty(const std::string& line) {
  const auto t = trim(line);
  return t.empty() || t[0] == '#';
}

int indent_of(const std::string& line) {
  int n = 0;
  for (char c : line) {
    if (c == ' ') {
      ++n;
    } else if (c == '\t') {
      n += 2;
    } else {
      break;
    }
  }
  return n;
}

}  // namespace

ScenarioFile parse_scenario_yaml(const std::string& text) {
  ScenarioFile sc;
  sc.raw = text;
  std::istringstream in(text);
  std::string line;
  bool in_drivers = false;
  while (std::getline(in, line)) {
    if (is_comment_or_empty(line)) {
      continue;
    }
    const int ind = indent_of(line);
    const std::string body = trim(line);
    const auto colon = body.find(':');
    if (colon == std::string::npos) {
      continue;
    }
    const std::string key = trim(body.substr(0, colon));
    std::string val = trim(body.substr(colon + 1));

    if (ind == 0 && key == "drivers") {
      in_drivers = true;
      continue;
    }
    if (ind == 0) {
      in_drivers = false;
    }

    if (in_drivers && ind > 0) {
      if (key == "type") {
        sc.driver_type = val;
      } else if (!val.empty()) {
        try {
          sc.drivers[key] = std::stod(val);
        } catch (...) {
          throw ScenarioError("bad driver numeric value for key: " + key);
        }
      }
      continue;
    }

    if (key == "name") {
      sc.name = val;
    } else if (key == "module") {
      sc.module = val;
    } else if (key == "description") {
      sc.description = val;
    } else if (key == "t_end") {
      sc.t_end = std::stod(val);
    } else if (key == "dt") {
      sc.dt = std::stod(val);
    } else if (key == "t_insert" || key == "rod_z_after") {
      sc.drivers[key] = std::stod(val);
    }
  }
  if (sc.name.empty()) {
    throw ScenarioError("scenario missing name");
  }
  if (sc.module.empty()) {
    throw ScenarioError("scenario missing module");
  }
  return sc;
}

ScenarioFile load_scenario_yaml(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    throw ScenarioError("cannot open scenario: " + path);
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return parse_scenario_yaml(ss.str());
}

void save_scenario_yaml(const std::string& path, const ScenarioFile& sc) {
  std::ofstream out(path);
  if (!out) {
    throw ScenarioError("cannot write scenario: " + path);
  }
  if (!sc.raw.empty()) {
    out << sc.raw;
    if (sc.raw.back() != '\n') {
      out << '\n';
    }
    return;
  }
  out << "name: " << sc.name << '\n';
  out << "module: " << sc.module << '\n';
  if (!sc.description.empty()) {
    out << "description: " << sc.description << '\n';
  }
  out << "drivers:\n";
  out << "  type: " << sc.driver_type << '\n';
  for (const auto& [k, v] : sc.drivers) {
    out << "  " << k << ": " << v << '\n';
  }
  out << "t_end: " << sc.t_end << '\n';
  out << "dt: " << sc.dt << '\n';
}

}  // namespace reactorlab::io
