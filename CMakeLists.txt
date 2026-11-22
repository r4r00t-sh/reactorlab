cmake_minimum_required(VERSION 3.25)

project(ReactorLab
  VERSION 0.8.0
  DESCRIPTION "Educational nuclear reactor simulation (not safety-grade)"
  LANGUAGES CXX
)

option(REACTORLAB_BUILD_TESTS "Build C++ unit tests" ON)
option(REACTORLAB_BUILD_BINDINGS "Build Python nanobind module" ON)
option(REACTORLAB_BUILD_GUI "Build Qt 6 desktop application" ON)
option(REACTORLAB_FETCH_DEPS "Fetch missing C++ deps with FetchContent" ON)
option(REACTORLAB_ENABLE_SANITIZERS "Enable ASan/UBSan" OFF)
option(REACTORLAB_ENABLE_WARNINGS "Enable strict warnings" ON)
option(REACTORLAB_RELEASE_BUILD
  "Fail configure if PLACEHOLDER nuclear data remain (release gate)" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Required so static deps (fmt/spdlog) can link into the nanobind shared module.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ReactorLabDependencies)
include(ReactorLabSanitizers)

reactorlab_require_fmt()
reactorlab_require_spdlog()
reactorlab_require_eigen()
reactorlab_require_sundials()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  message(STATUS "OpenMP found — enabling history-level MC parallelism")
else()
  message(WARNING "OpenMP not found — Monte Carlo will run single-threaded")
endif()

if(REACTORLAB_BUILD_TESTS)
  enable_testing()
  reactorlab_require_gtest()
endif()

if(REACTORLAB_ENABLE_WARNINGS)
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_subdirectory(core)

if(REACTORLAB_BUILD_BINDINGS)
  add_subdirectory(bindings)
endif()

if(REACTORLAB_BUILD_GUI)
  add_subdirectory(gui)
endif()

include(ReactorLabPackaging)

if(REACTORLAB_RELEASE_BUILD)
  execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/check-no-placeholders.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE _rl_ph_rc
  )
  if(NOT _rl_ph_rc EQUAL 0)
    message(FATAL_ERROR
      "REACTORLAB_RELEASE_BUILD=ON but PLACEHOLDER nuclear data remain. "
      "Replace OpenMC-collapsed rates and clear flags before release.")
  endif()
endif()

message(STATUS "ReactorLab ${PROJECT_VERSION}")
message(STATUS "  tests=${REACTORLAB_BUILD_TESTS} bindings=${REACTORLAB_BUILD_BINDINGS} gui=${REACTORLAB_BUILD_GUI}")
message(STATUS "  fetch_deps=${REACTORLAB_FETCH_DEPS} sanitizers=${REACTORLAB_ENABLE_SANITIZERS}")
message(STATUS "  release_build=${REACTORLAB_RELEASE_BUILD}")
