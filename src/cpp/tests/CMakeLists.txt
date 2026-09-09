find_package(GTest CONFIG REQUIRED)

add_executable(lob_tests
    OrderBook_test.cpp
    OrderGateway_test.cpp
)

target_link_libraries(lob_tests PRIVATE lob_core GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lob_tests)
