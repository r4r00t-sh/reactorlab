add_library(reactorlab_core
  src/hello.cpp
  src/kinetics/delayed_groups.cpp
  src/kinetics/point_kinetics.cpp
  src/plant/coupled_plant.cpp
  src/diffusion/two_group.cpp
  src/montecarlo/material.cpp
  src/montecarlo/eigenvalue.cpp
  src/iapws/if97_region1.cpp
  src/multiphysics/coupled.cpp
  src/io/csv_export.cpp
  src/io/scenario_yaml.cpp
  src/radiation/edu_dose.cpp
  src/fuelcycle/thorium_chain.cpp
)

add_library(reactorlab::core ALIAS reactorlab_core)

target_include_directories(reactorlab_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(reactorlab_core
  PUBLIC
    fmt::fmt
    Eigen3::Eigen
  PRIVATE
    spdlog::spdlog
    SUNDIALS::cvode
    SUNDIALS::nvecserial
    SUNDIALS::sunmatrixdense
    SUNDIALS::sunlinsoldense
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(reactorlab_core PUBLIC OpenMP::OpenMP_CXX)
endif()

target_compile_features(reactorlab_core PUBLIC cxx_std_20)

reactorlab_enable_sanitizers(reactorlab_core)

if(REACTORLAB_BUILD_TESTS)
  add_executable(reactorlab_core_tests
    tests/test_hello.cpp
    tests/test_point_kinetics.cpp
    tests/test_coupled_plant.cpp
    tests/test_diffusion.cpp
    tests/test_montecarlo.cpp
    tests/test_iapws.cpp
    tests/test_multiphysics.cpp
    tests/test_io.cpp
    tests/test_radiation.cpp
    tests/test_delayed_u233.cpp
    tests/test_thorium_chain.cpp
    tests/test_thorium_coupling.cpp
  )
  target_link_libraries(reactorlab_core_tests
    PRIVATE
      reactorlab::core
      GTest::gtest_main
  )
  target_compile_definitions(reactorlab_core_tests
    PRIVATE REACTORLAB_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  reactorlab_enable_sanitizers(reactorlab_core_tests)
  include(GoogleTest)
  gtest_discover_tests(reactorlab_core_tests)
endif()

# Install library + public headers
include(GNUInstallDirs)
install(TARGETS reactorlab_core
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/reactorlab
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
