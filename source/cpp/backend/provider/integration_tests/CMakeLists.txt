cmake_minimum_required(VERSION 3.16)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

add_executable(integration_tests
    ConfigFileIntegrationTests.cpp
    CompanySqlTemplateTests.cpp
)

find_package(GTest REQUIRED)

target_link_libraries(integration_tests
    PRIVATE
    provider_lib
    gtest::gtest
)
