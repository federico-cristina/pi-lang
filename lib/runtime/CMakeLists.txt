# Adds runtime library target
add_library("pi-lib-runtime"
    "value.c"
    "env.c"
)

# Links runtime library to common library
target_link_libraries("pi-lib-runtime"
    PRIVATE "pi-lib-common"
)
