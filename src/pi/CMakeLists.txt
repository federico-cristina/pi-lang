# Pi Executable
# Main entry point for the Pi language interpreter/compiler

# Create executable
add_executable(pi
    "pi.c"
)

# Link libraries (only link top-level dependencies)
# Common and runtime are automatically included via transitive PUBLIC dependencies
target_link_libraries(pi
    PRIVATE
        pi::compiler
        pi::svm
)

# Compile definitions
target_compile_definitions(pi
    PRIVATE
        PI_VERSION="${PROJECT_VERSION}"
        # Set PI_USE_DLL when linking against shared libraries
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Installation - always install the executable
install(TARGETS pi
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
