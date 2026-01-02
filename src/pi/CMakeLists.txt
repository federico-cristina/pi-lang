# Adds 'pi' executable target
add_executable(pi
    "pi.c"
)

# Links libraries to the executable
target_link_libraries(pi
    PRIVATE pi-lib-common
    PRIVATE pi-lib-compiler
    PRIVATE pi-lib-svm
)

# Installable targets
install(
    TARGETS pi
)
