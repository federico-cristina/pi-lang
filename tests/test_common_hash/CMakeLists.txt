# Common Hash Table Library Test

# Create test executable
add_executable(pi-test-common-hash
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-common-hash
    PRIVATE pi::common
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-common-hash
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-common-hash COMMAND pi-test-common-hash)
