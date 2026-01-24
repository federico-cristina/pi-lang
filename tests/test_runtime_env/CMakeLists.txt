# Runtime environment test

# Create test executable
add_executable(pi-test-runtime-env
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-runtime-env
    PRIVATE pi::runtime
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-runtime-env
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-runtime-env COMMAND pi-test-runtime-env)
