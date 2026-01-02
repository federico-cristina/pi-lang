# Adds stack vm library target
add_library("pi-lib-compiler"
    "source.c"
)

# Links stack vm library to common library
target_link_libraries("pi-lib-compiler"
    PRIVATE "pi-lib-common"
)
