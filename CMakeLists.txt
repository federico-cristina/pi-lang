cmake_minimum_required(VERSION 3.21.0)

# Detect if this is a standalone build or embedded as a subdirectory
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(PI_IS_STANDALONE ON)
else()
    set(PI_IS_STANDALONE OFF)
endif()

# Define project (always, but only set policies when standalone)
if(PI_IS_STANDALONE)
    project("Pi"
        VERSION         0.7.3
        DESCRIPTION     "The Pi programming language"
        LANGUAGES       C
    )

    # Remove library prefixes on Windows for cleaner naming
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_IMPORT_LIBRARY_PREFIX "")

    # Set C standard (global is appropriate for this)
    set(CMAKE_C_STANDARD            99)
    set(CMAKE_C_STANDARD_REQUIRED   ON)
    set(CMAKE_C_EXTENSIONS          OFF)
endif()

# Include standard installation directories (provides CMAKE_INSTALL_INCLUDEDIR, etc.)
include(GNUInstallDirs)

# =============================================================================
# Build Options
# =============================================================================

# Option to build shared libraries instead of static (standard CMake variable)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)

# Option to build the test suite (default: only when standalone)
if(DEFINED PROJECT_IS_TOP_LEVEL)
    # CMake 3.21+ provides PROJECT_IS_TOP_LEVEL
    option(PI_BUILD_TESTING "Build the test suite" ${PROJECT_IS_TOP_LEVEL})
else()
    # Fallback for older CMake
    option(PI_BUILD_TESTING "Build the test suite" ${PI_IS_STANDALONE})
endif()

# Option to build examples (not yet implemented, but future-proofing)
option(PI_BUILD_EXAMPLES "Build example programs" OFF)

# Option to install development files (headers, CMake configs)
# Default: only when standalone
if(DEFINED PROJECT_IS_TOP_LEVEL)
    option(PI_INSTALL_DEVELOPMENT "Install headers and CMake config files" ${PROJECT_IS_TOP_LEVEL})
else()
    option(PI_INSTALL_DEVELOPMENT "Install headers and CMake config files" ${PI_IS_STANDALONE})
endif()

# Internal cache variable for DLL build detection
# This will be used to set PI_BUILD_DLL or PI_USE_DLL in target compile definitions
if(BUILD_SHARED_LIBS)
    set(PI_BUILD_DLL ON CACHE INTERNAL "Building shared libraries (DLL)")
else()
    set(PI_BUILD_DLL OFF CACHE INTERNAL "Building static libraries")
endif()

# =============================================================================
# Project Subdirectories
# =============================================================================

# Add library implementations
add_subdirectory("lib")
# Add executable source
add_subdirectory("src")

# Add tests (conditional on PI_BUILD_TESTING)
if(PI_BUILD_TESTING)
    include(CTest)
    add_subdirectory("tests")
endif()

# =============================================================================
# Installation
# =============================================================================

if(PI_INSTALL_DEVELOPMENT)
    # Install public headers preserving directory structure
    install(DIRECTORY "include/pi"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h" PATTERN "*.inc")

    # Install CMake package config files
    install(EXPORT PiTargets
            FILE PiTargets.cmake
            NAMESPACE pi::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pi)

    # Generate and install package config and version files
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PiConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/PiConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pi)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/PiConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/PiConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/PiConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pi)
endif()
