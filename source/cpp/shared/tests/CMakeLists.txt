cmake_minimum_required(VERSION 3.16)

project(GlobalTestProject LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(global-settings)
# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

set(HEADER_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/JsonParameterFormatter.h
)

set(SOURCE_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/configfile.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${INCLUDE_DIR}/JsonParameterFormatter.cpp

    ConfigFIleTests.cpp
    ConfigFileIntegrationTests.cpp
    TypeToStringFormatterTests.cpp
    JsonParameterFormatterTests.cpp
    IncludeUtilTests.cpp
)

add_executable(GlobalTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

find_package(GTest REQUIRED)

target_link_libraries(GlobalTestProject
    PRIVATE
    gtest::gtest
)
