cmake_minimum_required(VERSION 3.16)

project(GlobalTestProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../cmake/global-settings.cmake)
# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")


# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

set(HEADER_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/configfile.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h
)

set(SOURCE_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/configfile.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp

    ConfigFIleTests.cpp
    TypeToStringFormatterTests.cpp
    UtilFunctionsTests.cpp
)

add_executable(GlobalTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(GlobalTestProject PRIVATE
    TEST_GENERATED_ROOT="${TEST_GENERATED_ROOT}"
    TEST_REFRERENCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/reference"
)

find_package(GTest REQUIRED)

target_link_libraries(GlobalTestProject
    PRIVATE
    gtest::gtest
)
