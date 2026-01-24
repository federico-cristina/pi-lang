# System utilities test

# Create test executable
add_executable(pi-test-system
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-system
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-system
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-system COMMAND pi-test-system)
