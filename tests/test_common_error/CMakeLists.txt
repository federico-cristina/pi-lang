# Error handling test

# Create test executable
add_executable(pi-test-error
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-error
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-error
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-error COMMAND pi-test-error)
