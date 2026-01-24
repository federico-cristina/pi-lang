# Adds error test executable
add_executable("pi-test-error"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-error"
    PRIVATE "pi-lib-common"
)

# Adds test
add_test(
    NAME "TestError" COMMAND "pi-test-error"
)
