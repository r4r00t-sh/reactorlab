# Resolve third-party dependencies via find_package, or FetchContent when
# REACTORLAB_FETCH_DEPS is ON (default for Phase 0 bootstrap / no-sudo builds).

include(FetchContent)

function(reactorlab_require_fmt)
  find_package(fmt CONFIG QUIET)
  if(NOT fmt_FOUND AND REACTORLAB_FETCH_DEPS)
    message(STATUS "Fetching fmt via FetchContent")
    FetchContent_Declare(
      fmt
      GIT_REPOSITORY https://github.com/fmtlib/fmt.git
      GIT_TAG 11.1.4
      GIT_SHALLOW TRUE)
    set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(fmt)
  elseif(NOT fmt_FOUND)
    message(FATAL_ERROR "fmt not found. Enable REACTORLAB_FETCH_DEPS or install fmt.")
  endif()
endfunction()

function(reactorlab_require_spdlog)
  find_package(spdlog CONFIG QUIET)
  if(NOT spdlog_FOUND AND REACTORLAB_FETCH_DEPS)
    message(STATUS "Fetching spdlog via FetchContent")
    FetchContent_Declare(
      spdlog
      GIT_REPOSITORY https://github.com/gabime/spdlog.git
      GIT_TAG v1.15.1
      GIT_SHALLOW TRUE)
    set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
  elseif(NOT spdlog_FOUND)
    message(FATAL_ERROR
            "spdlog not found. Enable REACTORLAB_FETCH_DEPS or install spdlog.")
  endif()
endfunction()

function(reactorlab_require_gtest)
  find_package(GTest CONFIG QUIET)
  if(NOT GTest_FOUND)
    find_package(GTest QUIET)
  endif()
  if(NOT GTest_FOUND AND REACTORLAB_FETCH_DEPS)
    message(STATUS "Fetching GoogleTest via FetchContent")
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.15.2
      GIT_SHALLOW TRUE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  elseif(NOT GTest_FOUND)
    message(FATAL_ERROR
            "GTest not found. Enable REACTORLAB_FETCH_DEPS or install gtest.")
  endif()
endfunction()

function(reactorlab_require_eigen)
  find_package(Eigen3 3.4 QUIET NO_MODULE)
  if(NOT Eigen3_FOUND AND REACTORLAB_FETCH_DEPS)
    message(STATUS "Fetching Eigen via FetchContent")
    FetchContent_Declare(
      eigen
      GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
      GIT_TAG 3.4.0
      GIT_SHALLOW TRUE)
    set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(eigen)
  elseif(NOT Eigen3_FOUND)
    message(FATAL_ERROR "Eigen3 not found. Enable REACTORLAB_FETCH_DEPS or install libeigen3-dev.")
  endif()
endfunction()

function(reactorlab_require_sundials)
  find_package(SUNDIALS CONFIG QUIET)
  if(SUNDIALS_FOUND)
    return()
  endif()
  # Some distro packages export individual components without a SUNDIALS config.
  if(TARGET SUNDIALS::cvode)
    return()
  endif()

  if(NOT REACTORLAB_FETCH_DEPS)
    message(FATAL_ERROR
            "SUNDIALS not found. Enable REACTORLAB_FETCH_DEPS or install sundials.")
  endif()

  message(STATUS "Fetching SUNDIALS via FetchContent")
  set(BUILD_ARKODE OFF CACHE BOOL "" FORCE)
  set(BUILD_CVODES OFF CACHE BOOL "" FORCE)
  set(BUILD_IDA OFF CACHE BOOL "" FORCE)
  set(BUILD_IDAS OFF CACHE BOOL "" FORCE)
  set(BUILD_KINSOL OFF CACHE BOOL "" FORCE)
  set(BUILD_CVODE ON CACHE BOOL "" FORCE)
  set(ENABLE_MPI OFF CACHE BOOL "" FORCE)
  set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE)
  set(EXAMPLES_ENABLE_C OFF CACHE BOOL "" FORCE)
  set(EXAMPLES_ENABLE_CXX OFF CACHE BOOL "" FORCE)
  set(EXAMPLES_ENABLE_F2003 OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(SUNDIALS_BUILD_WITH_PROFILING OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_TEST_UNITTESTS OFF CACHE BOOL "" FORCE)
  set(SUNDIALS_ENABLE_ERROR_CHECKS ON CACHE BOOL "" FORCE)

  FetchContent_Declare(
    sundials
    GIT_REPOSITORY https://github.com/LLNL/sundials.git
    GIT_TAG v7.1.1
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(sundials)
endfunction()
