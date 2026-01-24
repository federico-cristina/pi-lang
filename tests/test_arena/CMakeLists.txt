# Adds arena test executable
add_executable("pi-test-arena"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-arena"
    PRIVATE "pi-lib-common"
)

# Adds test
add_test(
    NAME "TestArena" COMMAND "pi-test-arena"
)
