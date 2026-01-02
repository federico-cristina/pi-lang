# Adds svm disassembler test executable
add_executable("pi-test-svm-disasm"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-svm-disasm"
    PRIVATE pi-lib-common
    PRIVATE pi-lib-svm
)

# Adds test
add_test(
    NAME "TestSvmDisasm" COMMAND "pi-test-svm-disasm"
)
