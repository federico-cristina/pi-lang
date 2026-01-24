# Arena allocator test

# Create test executable
add_executable(pi-test-arena
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-arena
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-arena
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-arena COMMAND pi-test-arena)
