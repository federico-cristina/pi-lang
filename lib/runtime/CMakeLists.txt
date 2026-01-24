# Pi Runtime Library
# Provides runtime values, objects, and environment for the Pi language

# Create library (type controlled by BUILD_SHARED_LIBS option)
add_library(pi-lib-runtime
    "value.c"
    "env.c"
)

# Add namespaced alias for clean embedding and modern CMake usage
add_library(pi::runtime ALIAS pi-lib-runtime)

# Set target properties
set_target_properties(pi-lib-runtime PROPERTIES
    OUTPUT_NAME "pi-runtime"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories with proper usage requirements
target_include_directories(pi-lib-runtime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compile definitions
target_compile_definitions(pi-lib-runtime
    PRIVATE
        PI_VERSION="${PROJECT_VERSION}"
        $<$<BOOL:${PI_BUILD_DLL}>:PI_BUILD_DLL>
    PUBLIC
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:PI_STATIC>
)

# Link dependencies
# PUBLIC because runtime's API uses common's types (PiArena, error handling, etc.)
target_link_libraries(pi-lib-runtime
    PUBLIC pi::common
)

# Installation (only when PI_INSTALL_DEVELOPMENT is ON)
if(PI_INSTALL_DEVELOPMENT)
    install(TARGETS pi-lib-runtime
            EXPORT PiTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
