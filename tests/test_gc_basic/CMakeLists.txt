# Basic GC test

# Create test executable
add_executable(pi-test-gc-basic
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-gc-basic
    PRIVATE pi::runtime
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-gc-basic
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-gc-basic COMMAND pi-test-gc-basic)
