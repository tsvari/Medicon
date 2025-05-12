cmake_minimum_required(VERSION 3.16)

project(FrontendTestProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/global-settings.cmake)

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)

# gRPC library
IF (WIN32)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/windows/lib/cmake)
    #list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/windows/lib)
ELSE()
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/linux/lib/cmake)
ENDIF()

find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

set(HEADER_FILES
    ${INCLUDE_DIR}/include_util.h
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.h
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.h
)

set(SOURCE_FILES
    ${INCLUDE_DIR}/include_util.cpp	
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc
    ${FRONTEND_GRPC_DIR}/company_client.hpp

    GrpcTests.cpp
)

add_executable(FrontendTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(FrontendTestProject PRIVATE
    TEST_GENERATED_ROOT="${TEST_GENERATED_ROOT}"
    TEST_REFRERENCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/reference"
)

find_package(GTest REQUIRED)

target_link_libraries(FrontendTestProject
    PRIVATE
    gtest::gtest

    absl::check
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}

    #user32.lib
    #version.lib
    #oleaut32.lib
    #ole32.lib
    #ws2_32.lib
)

