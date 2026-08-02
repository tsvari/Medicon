# Company domain tests — standalone test target for Company Service/Repository/Server
#
# These tests live WITH the company domain code (grpc/company/) because the
# domain types are shared between backend and frontend consumers. They must
# NOT be tied to a single app (provider, frontend, etc.).
#
# Added as a subdirectory of the provider project (which builds provider_lib —
# the full backend library containing company code + SQL infrastructure).

cmake_minimum_required(VERSION 3.16)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

add_executable(company_tests
    test_main.cpp
    CompanyServiceTests.cpp
    CompanyServiceImplTests.cpp
)

find_package(GTest REQUIRED)

target_link_libraries(company_tests
    PRIVATE
    provider_lib
    easyloggingpp::easyloggingpp
    gtest::gtest
)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
    gtest_discover_tests(company_tests
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
