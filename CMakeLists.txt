cmake_minimum_required(VERSION 3.10.0)

# Adds a new project or embed into an existing one
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project("Pi"
        VERSION         0.7.2
        DESCRIPTION     "The Pi programming language"
        LANGUAGES       C
    )

    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_IMPORT_LIBRARY_PREFIX "")
endif()

include(CTest)

# Defines `PI_VERSION` constant
add_compile_definitions(
    "PI_VERSION=\"${CMAKE_PROJECT_VERSION}\""
)

# Sets headers inclusion directory
include_directories(
    "include"
)

# Includes project subdirectories
add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("tests")
