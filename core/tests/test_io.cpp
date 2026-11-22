#include "reactorlab/io/csv_export.hpp"
#include "reactorlab/io/scenario_yaml.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST(IoCsv, RoundTripShape) {
  const auto path = fs::temp_directory_path() / "reactorlab_test.csv";
  reactorlab::io::write_csv(
      path.string(),
      {{"t[s]", {0.0, 1.0, 2.0}}, {"P[-]", {1.0, 1.1, 1.2}}},
      "unit test");
  std::ifstream in(path);
  ASSERT_TRUE(in.good());
  std::string line;
  std::getline(in, line);
  EXPECT_EQ(line.front(), '#');
  std::getline(in, line);
  EXPECT_EQ(line.front(), '#');
  std::getline(in, line);
  EXPECT_NE(line.find("t[s]"), std::string::npos);
  std::remove(path.string().c_str());
}

TEST(IoScenario, ParseRodEjection) {
  const char* yaml = R"(
name: multiphysics_rod_ejection
module: multiphysics
description: test
drivers:
  type: rod_ejection
  t_eject: 1.0
  z0: 0.08
  duration: 0.2
t_end: 20.0
dt: 0.1
)";
  const auto sc = reactorlab::io::parse_scenario_yaml(yaml);
  EXPECT_EQ(sc.name, "multiphysics_rod_ejection");
  EXPECT_EQ(sc.module, "multiphysics");
  EXPECT_EQ(sc.driver_type, "rod_ejection");
  EXPECT_DOUBLE_EQ(sc.drivers.at("z0"), 0.08);
  EXPECT_DOUBLE_EQ(sc.t_end, 20.0);
}

TEST(IoScenario, LoadShippedFile) {
  const std::string path =
      std::string(REACTORLAB_SOURCE_DIR) + "/scenarios/multiphysics_rod_ejection.yaml";
  const auto sc = reactorlab::io::load_scenario_yaml(path);
  EXPECT_EQ(sc.module, "multiphysics");
  EXPECT_EQ(sc.driver_type, "rod_ejection");
  EXPECT_DOUBLE_EQ(sc.drivers.at("z0"), 0.08);
}
