# Arena allocator test

# Create test executable
add_executable(pi-test-common-arena
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-common-arena
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-common-arena
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-common-arena COMMAND pi-test-common-arena)
