find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)

execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_VARIABLE nanobind_ROOT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE nanobind_cmake_dir_result
)
if(NOT nanobind_cmake_dir_result EQUAL 0)
  message(FATAL_ERROR
    "nanobind not found for ${Python_EXECUTABLE}. "
    "Install with: ${Python_EXECUTABLE} -m pip install nanobind")
endif()

find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(reactorlab_core_nb module.cpp)
set_target_properties(reactorlab_core_nb PROPERTIES OUTPUT_NAME reactorlab_core)
target_link_libraries(reactorlab_core_nb PRIVATE reactorlab::core)
# nanobind headers trip -Wshadow under GCC; keep project sources clean elsewhere.
target_compile_options(reactorlab_core_nb PRIVATE -Wno-shadow)
reactorlab_enable_sanitizers(reactorlab_core_nb)

# Place the extension next to python tests for easy import during development.
set(_nb_out_dir "${CMAKE_BINARY_DIR}/python")
file(MAKE_DIRECTORY "${_nb_out_dir}")
set_target_properties(reactorlab_core_nb PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${_nb_out_dir}"
  RUNTIME_OUTPUT_DIRECTORY "${_nb_out_dir}"
)
