# Adds system test executable
add_executable("pi-test-system"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-system"
    PRIVATE "pi-lib-common"
)

# Adds test
add_test(
    NAME "TestSystem" COMMAND "pi-test-system"
)
