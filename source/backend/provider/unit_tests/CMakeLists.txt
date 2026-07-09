cmake_minimum_required(VERSION 3.16)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(unit_tests
    SentryTests.cpp
)

find_package(GTest REQUIRED)

target_link_libraries(unit_tests
    PRIVATE
    provider_lib
    gtest::gtest
)
