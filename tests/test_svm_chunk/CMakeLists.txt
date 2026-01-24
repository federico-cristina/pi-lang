# SVM chunk test

# Create test executable
add_executable(pi-test-svm-chunk
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-svm-chunk
    PRIVATE pi::svm
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-svm-chunk
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-svm-chunk COMMAND pi-test-svm-chunk)
