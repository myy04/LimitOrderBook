add_executable(lob_tests EXCLUDE_FROM_ALL
    OrderBook_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/OrderBook.cpp
)

target_include_directories(lob_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(lob_tests PRIVATE GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lob_tests)
