# --- CPack Multi-Platform Configuration (DEB & RPM) ---

# Basic Package Metadata
set(CPACK_PACKAGE_NAME "liblog4cpp")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "log4cpp.org")
set(CPACK_PACKAGE_CONTACT "developer@log4cpp.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Log4j-style C++ logging library")

# License Configuration
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RPM_PACKAGE_LICENSE "LGPLv3")

# Generator & Components
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

# System Architecture Detection
execute_process(COMMAND dpkg --print-architecture OUTPUT_VARIABLE CPACK_SYSTEM_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CPACK_SYSTEM_ARCH}")

# Dependency Logic Optimization
set(STRICT_VERSION_MATCH "= ${CPACK_PACKAGE_VERSION}")

# Maintainer Scripts (ldconfig)
# Created in the binary directory to keep source clean
file(WRITE "${CMAKE_BINARY_DIR}/postinst" "#!/bin/sh\nset -e\nldconfig\n")
file(WRITE "${CMAKE_BINARY_DIR}/postrm"   "#!/bin/sh\nset -e\nldconfig\n")

# --- Component: Runtime (liblog4cpp) ---
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "liblog4cpp")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst;${CMAKE_BINARY_DIR}/postrm")

set(CPACK_RPM_RUNTIME_PACKAGE_NAME "liblog4cpp")
set(CPACK_RPM_RUNTIME_AUTOREQ ON)
set(CPACK_RPM_RUNTIME_POST_INSTALL_SCRIPT_FILE "${CMAKE_BINARY_DIR}/postinst")
set(CPACK_RPM_RUNTIME_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_BINARY_DIR}/postrm")

# --- Component: Development (liblog4cpp-dev) ---
set(CPACK_DEBIAN_DEV_PACKAGE_NAME "liblog4cpp-dev")
set(CPACK_DEBIAN_DEV_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "liblog4cpp (= ${CPACK_PACKAGE_VERSION}), nlohmann-json3-dev (>= 3.0.0)")

set(CPACK_RPM_DEV_PACKAGE_NAME "liblog4cpp-devel")
set(CPACK_RPM_DEV_PACKAGE_SECTION "Development/Libraries")
set(CPACK_RPM_DEV_PACKAGE_REQUIRES "liblog4cpp ${CPACK_PACKAGE_VERSION}, nlohmann-json-devel >= 3.0.0")

# Final Filename Formatting
# Disable default component naming behavior
set(CPACK_DEB_PACKAGE_COMPONENT_PART_NAME OFF)
set(CPACK_RPM_PACKAGE_COMPONENT_PART_NAME OFF)

# Explicitly set filenames for Debian
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_FILE_NAME "liblog4cpp_${CPACK_PACKAGE_VERSION}_${CPACK_SYSTEM_ARCH}.deb")
set(CPACK_DEBIAN_DEV_PACKAGE_FILE_NAME "liblog4cpp-dev_${CPACK_PACKAGE_VERSION}_${CPACK_SYSTEM_ARCH}.deb")

# Explicit Filenames for RPM
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_RUNTIME_PACKAGE_FILE_NAME "liblog4cpp-${CPACK_PACKAGE_VERSION}.${CPACK_SYSTEM_ARCH}.rpm")
set(CPACK_RPM_DEV_PACKAGE_FILE_NAME "liblog4cpp-devel-${CPACK_PACKAGE_VERSION}.${CPACK_SYSTEM_ARCH}.rpm")
set(CPACK_RPM_COMPONENT_INSTALL ON)

include(CPack)
