cmake_minimum_required(VERSION 3.16)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

add_executable(unit_tests
    test_main.cpp
    SentryTests.cpp
)

find_package(GTest REQUIRED)

target_link_libraries(unit_tests
    PRIVATE
    provider_lib
    easyloggingpp::easyloggingpp
    gtest::gtest
)
