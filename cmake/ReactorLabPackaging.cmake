# Install + CPack packaging for ReactorLab (Phase 6).

include(GNUInstallDirs)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/scenarios/"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/reactorlab/scenarios"
  FILES_MATCHING PATTERN "*.yaml"
)

install(FILES
  "${CMAKE_SOURCE_DIR}/README.md"
  "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
  "${CMAKE_SOURCE_DIR}/LICENSE"
  "${CMAKE_SOURCE_DIR}/docs/user-guide.md"
  DESTINATION "${CMAKE_INSTALL_DOCDIR}"
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs/theory"
  DESTINATION "${CMAKE_INSTALL_DOCDIR}"
)

# Desktop entry for DEB/TGZ installs
configure_file(
  "${CMAKE_SOURCE_DIR}/packaging/reactorlab.desktop.in"
  "${CMAKE_BINARY_DIR}/reactorlab.desktop"
  @ONLY
)
install(FILES "${CMAKE_BINARY_DIR}/reactorlab.desktop"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)

set(CPACK_PACKAGE_NAME "reactorlab")
set(CPACK_PACKAGE_VENDOR "ReactorLab")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Educational nuclear reactor simulation (not safety-grade)")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "ReactorLab maintainers")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ReactorLab <reactorlab@example.invalid>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 | libqt6widgets6t64")
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-linux-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  "/build/;/\\.git/;/venv/;/third_party/Qt/;/\\.venv/;/dist/")

include(CPack)
