set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

add_executable(connection_tests
    ConnectionTests.cpp
)

target_compile_definitions(connection_tests PRIVATE
    TEST_GENERATED_ROOT="${TEST_GENERATED_ROOT}"
    TEST_REFRERENCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/reference"
)

find_package(GTest REQUIRED)

target_link_libraries(connection_tests
    PRIVATE
    provider_lib
    gtest::gtest
)
