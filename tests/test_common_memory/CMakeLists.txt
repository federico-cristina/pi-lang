# Memory management test

# Create test executable
add_executable(pi-test-memory
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-memory
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-memory
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-memory COMMAND pi-test-memory)
