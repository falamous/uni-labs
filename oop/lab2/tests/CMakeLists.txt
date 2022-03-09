enable_testing()

add_executable(DeltoidTest ../deltoid.cc deltoid_test.cc)
target_link_libraries(
    DeltoidTest
    gtest_main
    gtest
    )

include(GoogleTest)
gtest_discover_tests(DeltoidTest)
