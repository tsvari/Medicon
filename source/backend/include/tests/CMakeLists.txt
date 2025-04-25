cmake_minimum_required(VERSION 3.16)

project(BackendTestProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/global-settings.cmake)

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

set(HEADER_FILES
    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/include_util.h

    ${THIRD_PARTY_INCLUDE_DIR}/markup/Markup.h

    ${BACKEND_INCLUDE_DIR}/include_backend_util.h
    ${BACKEND_INCLUDE_DIR}/sqlapplet.h
)

set(SOURCE_FILES
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${INCLUDE_DIR}/include_util.cpp
	
    ${THIRD_PARTY_INCLUDE_DIR}/markup/Markup.cpp

    ${BACKEND_INCLUDE_DIR}/include_backend_util.cpp
    ${BACKEND_INCLUDE_DIR}/sqlapplet.cpp

    SQLAppletTests.cpp
)

add_executable(BackendTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(BackendTestProject PRIVATE
    TEST_GENERATED_ROOT="${TEST_GENERATED_ROOT}"
    TEST_REFRERENCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/reference"
)

find_package(GTest REQUIRED)

target_link_libraries(BackendTestProject
    PRIVATE
    gtest::gtest
)
