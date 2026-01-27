# Common tests
add_subdirectory("test_common_arena")
add_subdirectory("test_common_system")
add_subdirectory("test_common_error")
add_subdirectory("test_common_memory")
add_subdirectory("test_common_string")
add_subdirectory("test_common_hash")

# Runtime tests
add_subdirectory("test_gc_basic")

# Stack-VM tests
add_subdirectory("test_svm_arithmetic")
add_subdirectory("test_svm_stack")
add_subdirectory("test_svm_disasm")
