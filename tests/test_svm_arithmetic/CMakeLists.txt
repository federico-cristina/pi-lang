# Stack-VM arithmetic operations test

# Create test executable
add_executable(pi-test-svm-arithmetic
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-svm-arithmetic
    PRIVATE pi::runtime
    PRIVATE pi::svm
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-svm-arithmetic
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-svm-arithmetic COMMAND pi-test-svm-arithmetic)
