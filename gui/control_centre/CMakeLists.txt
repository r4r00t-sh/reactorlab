# ReactorLab QML control centre (UI-1 shell + UI-2 live engine facade)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Gui Quick3D)

qt_add_executable(reactorlab_control_centre
  src/main.cpp
  src/engine/engine_worker.cpp
  src/engine/engine_controller.cpp
  src/engine/engine_worker.hpp
  src/engine/engine_controller.hpp
  src/engine/reactor_snapshot.hpp
)

target_include_directories(reactorlab_control_centre
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(reactorlab_control_centre
  PRIVATE
    REACTORLAB_CC_IMPORTS="${CMAKE_CURRENT_SOURCE_DIR}/imports"
    REACTORLAB_CC_MAIN_QML="${CMAKE_CURRENT_SOURCE_DIR}/qml/Main.qml"
    REACTORLAB_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(reactorlab_control_centre
  PRIVATE
    reactorlab::core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Gui
    Qt6::Quick3D
)

reactorlab_enable_sanitizers(reactorlab_control_centre)

if(DEFINED Qt6_DIR)
  get_filename_component(_qt6_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
  if(EXISTS "${_qt6_prefix}/lib")
    set_target_properties(reactorlab_control_centre PROPERTIES
      BUILD_RPATH "${_qt6_prefix}/lib"
      INSTALL_RPATH "${_qt6_prefix}/lib"
    )
  endif()
endif()

include(GNUInstallDirs)
install(TARGETS reactorlab_control_centre RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY imports DESTINATION ${CMAKE_INSTALL_DATADIR}/reactorlab/qml)
install(DIRECTORY qml DESTINATION ${CMAKE_INSTALL_DATADIR}/reactorlab)

if(REACTORLAB_BUILD_TESTS)
  find_package(Qt6 REQUIRED COMPONENTS Test)
  qt_add_executable(reactorlab_cc_engine_tests
    tests/test_engine_worker.cpp
    src/engine/engine_worker.cpp
    src/engine/engine_worker.hpp
    src/engine/reactor_snapshot.hpp
  )
  target_include_directories(reactorlab_cc_engine_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(reactorlab_cc_engine_tests PRIVATE reactorlab::core Qt6::Test Qt6::Core)
  if(DEFINED Qt6_DIR)
    get_filename_component(_qt6_prefix_tests "${Qt6_DIR}/../../.." ABSOLUTE)
    set_target_properties(reactorlab_cc_engine_tests PROPERTIES
      BUILD_RPATH "${_qt6_prefix_tests}/lib")
  endif()
  add_test(NAME control_centre.EngineWorker COMMAND reactorlab_cc_engine_tests)
endif()
