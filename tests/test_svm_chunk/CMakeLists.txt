# Adds svm chunk test executable
add_executable("pi-test-svm-chunk"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-svm-chunk"
    PRIVATE "pi-lib-svm"
)

# Adds test
add_test(
    NAME "TestSvmChunk" COMMAND "pi-test-svm-chunk"
)
