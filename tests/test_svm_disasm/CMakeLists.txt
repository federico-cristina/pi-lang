# SVM disassembler test

# Create test executable
add_executable(pi-test-svm-disasm
	"test.c"
)

# Link against library being tested
target_link_libraries(pi-test-svm-disasm
    PRIVATE pi::svm
)

# Compile definitions (use DLL import when linking shared)
target_compile_definitions(pi-test-svm-disasm
    PRIVATE
        $<$<BOOL:${PI_BUILD_DLL}>:PI_USE_DLL>
)

# Register with CTest
add_test(NAME test-svm-disasm COMMAND pi-test-svm-disasm)
