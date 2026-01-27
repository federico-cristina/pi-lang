# Stack-VM stack operations test

# Create test executable
add_executable(pi-test-svm-stack
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-svm-stack
    PRIVATE pi::runtime
    PRIVATE pi::svm
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-svm-stack
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-svm-stack COMMAND pi-test-svm-stack)
