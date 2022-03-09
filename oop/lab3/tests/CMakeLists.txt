enable_testing()

# add_executable(DynamicTest ../poly-dynamic.cc dynamic_test.cc)
# target_link_libraries(
#     DynamicTest
#     gtest_main
#     gtest
#     )

add_executable(StaticTest static_test.cc)
target_link_libraries(
    StaticTest
    gtest_main
    gtest
    )

include(GoogleTest)
# gtest_discover_tests(DynamicTest)
gtest_discover_tests(StaticTest)
