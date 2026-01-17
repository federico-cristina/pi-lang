# Adds stack vm library target
add_library("pi-lib-svm"
    "chunk.c"
    "disasm.c"
    "svm.c"
)

# Links stack vm library to common library
target_link_libraries("pi-lib-svm"
    PRIVATE "pi-lib-common"
    PRIVATE "pi-lib-runtime"
)
