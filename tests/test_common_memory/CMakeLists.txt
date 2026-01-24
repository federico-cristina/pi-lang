# Adds memory test executable
add_executable("pi-test-memory"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-memory"
    PRIVATE "pi-lib-common"
)

# Adds test
add_test(
    NAME "TestMemory" COMMAND "pi-test-memory"
)
