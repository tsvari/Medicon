cmake_minimum_required(VERSION 3.16)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

add_executable(integration_tests
    test_main.cpp
    ConfigFileIntegrationTests.cpp
)

find_package(GTest REQUIRED)

target_link_libraries(integration_tests
    PRIVATE
    provider_lib
    easyloggingpp::easyloggingpp
    gtest::gtest
)
