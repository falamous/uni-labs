enable_testing()

add_executable(SpmTest ../spm.cc spm.cc)
target_link_libraries(
    SpmTest
    gtest_main
    gtest
    )

include(GoogleTest)
gtest_discover_tests(SpmTest)
