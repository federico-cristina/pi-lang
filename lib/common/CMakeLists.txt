# Pi Common Library
# Provides core utilities: arena allocator, error handling, memory management, system utilities

# Create library (type controlled by BUILD_SHARED_LIBS option)
add_library(pi-lib-common
	"arena.c"
	"error.c"
	"hash.c"
	"memory.c"
	"string.c"
	"system.c"
)

# Add namespaced alias for clean embedding and modern CMake usage
add_library(pi::common ALIAS pi-lib-common)

# Set target properties
set_target_properties(pi-lib-common PROPERTIES
    OUTPUT_NAME "pi-common"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories with proper usage requirements
# BUILD_INTERFACE: Used when building this project
# INSTALL_INTERFACE: Used when this project is installed and used by others
target_include_directories(pi-lib-common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compile definitions
target_compile_definitions(pi-lib-common
    PRIVATE
        # PI_VERSION is private (only needed in implementation)
        PI_VERSION="${PROJECT_VERSION}"
        # PI_BUILD_DLL is set when building shared libraries
        $<$<BOOL:${PI_BUILD_DLL}>:PI_BUILD_DLL>
    PUBLIC
        # Consumers need to know if they're linking against static libs
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:PI_STATIC>
)

# No dependencies for common library (it's the foundation)

# Installation (only when PI_INSTALL_DEVELOPMENT is ON)
if(PI_INSTALL_DEVELOPMENT)
    install(TARGETS pi-lib-common
            EXPORT PiTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
