# Common String Library Test

# Create test executable
add_executable(pi-test-common-string
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-common-string
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-common-string
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-common-string COMMAND pi-test-common-string)
