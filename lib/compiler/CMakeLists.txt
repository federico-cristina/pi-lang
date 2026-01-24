# Pi Compiler Library
# Provides lexer, parser, diagnostic system, and source management for the Pi language

# Create library (type controlled by BUILD_SHARED_LIBS option)
add_library(pi-lib-compiler
    "source.c"
    "token.c"
    "lexer.c"
    "parser.c"
    "diagnostic.c"
)

# Add namespaced alias for clean embedding and modern CMake usage
add_library(pi::compiler ALIAS pi-lib-compiler)

# Set target properties
set_target_properties(pi-lib-compiler PROPERTIES
    OUTPUT_NAME "pi-compiler"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories with proper usage requirements
target_include_directories(pi-lib-compiler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compile definitions
target_compile_definitions(pi-lib-compiler
    PRIVATE
        PI_VERSION="${PROJECT_VERSION}"
        $<$<BOOL:${PI_BUILD_DLL}>:PI_BUILD_DLL>
    PUBLIC
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:PI_STATIC>
)

# Link dependencies
# PUBLIC because compiler's API uses common's types
target_link_libraries(pi-lib-compiler
    PUBLIC pi::common
)

# Installation (only when PI_INSTALL_DEVELOPMENT is ON)
if(PI_INSTALL_DEVELOPMENT)
    install(TARGETS pi-lib-compiler
            EXPORT PiTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
