set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport Quick QuickControls2 Gui)

qt_standard_project_setup()

qt_add_executable(reactorlab_gui
  src/main.cpp
  src/mainwindow.cpp
  include/mainwindow.hpp
  third_party/qcustomplot/qcustomplot.cpp
  third_party/qcustomplot/qcustomplot.h
)

target_include_directories(reactorlab_gui
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/qcustomplot
)

target_link_libraries(reactorlab_gui
  PRIVATE
    reactorlab::core
    Qt6::Widgets
    Qt6::PrintSupport
)

# QCustomPlot is third-party; keep warnings local to our sources.
target_compile_options(reactorlab_gui PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-conversion -Wno-shadow -Wno-float-conversion>
)

reactorlab_enable_sanitizers(reactorlab_gui)

set_target_properties(reactorlab_gui PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE ON
)

if(DEFINED Qt6_DIR)
  get_filename_component(_qt6_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
  if(EXISTS "${_qt6_prefix}/plugins")
    set_target_properties(reactorlab_gui PROPERTIES
      BUILD_RPATH "${_qt6_prefix}/lib"
      INSTALL_RPATH "${_qt6_prefix}/lib"
    )
  endif()
endif()

include(GNUInstallDirs)
install(TARGETS reactorlab_gui
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION .
)
# Default scenarios path hint for installed app
target_compile_definitions(reactorlab_gui PRIVATE
  REACTORLAB_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# QML control centre (live engine facade — UI-2+)
add_subdirectory(control_centre)
