# Adds test executable
add_executable("pi-test-runtime-env"
	"test.c"
)

# Links libraries to the executable
target_link_libraries("pi-test-runtime-env"
    PRIVATE "pi-lib-runtime"
)

# Adds test
add_test(
    NAME "TestRuntimeEnv" COMMAND "pi-test-runtime-env"
)
