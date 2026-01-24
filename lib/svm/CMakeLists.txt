# Pi Stack VM Library
# Provides stack-based virtual machine, bytecode chunks, disassembler, and code generator

# Create library (type controlled by BUILD_SHARED_LIBS option)
add_library(pi-lib-svm
    "chunk.c"
    "disasm.c"
    "svm.c"
    "codegen.c"
)

# Add namespaced alias for clean embedding and modern CMake usage
add_library(pi::svm ALIAS pi-lib-svm)

# Set target properties
set_target_properties(pi-lib-svm PROPERTIES
    OUTPUT_NAME "pi-svm"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories with proper usage requirements
target_include_directories(pi-lib-svm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compile definitions
target_compile_definitions(pi-lib-svm
    PRIVATE
        PI_VERSION="${PROJECT_VERSION}"
        $<$<BOOL:${PI_BUILD_DLL}>:PI_BUILD_DLL>
    PUBLIC
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:PI_STATIC>
)

# Add compiler-specific features for computed goto (direct threading)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # GCC and Clang support computed goto via &&label extension
    target_compile_definitions(pi-lib-svm
        PRIVATE PI_USE_JUMP_TABLE=1)
endif()

# Link dependencies
# PUBLIC because svm's API uses common's and runtime's types
target_link_libraries(pi-lib-svm
    PUBLIC
        pi::common
        pi::runtime
)

# Installation (only when PI_INSTALL_DEVELOPMENT is ON)
if(PI_INSTALL_DEVELOPMENT)
    install(TARGETS pi-lib-svm
            EXPORT PiTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
