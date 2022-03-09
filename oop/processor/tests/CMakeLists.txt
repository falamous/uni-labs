enable_testing()

add_executable(BinaryOperationOpcodeTest ../opcode.cc ../register.cc ../memory.cc ../error.cc ../util.cc bin_operation_opcode_tests.cc)
target_link_libraries(
    BinaryOperationOpcodeTest
    gtest_main
    gtest
    )

include(GoogleTest)
gtest_discover_tests(BinaryOperationOpcodeTest)
