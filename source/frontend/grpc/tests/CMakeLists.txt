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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

set(HEADER_FILES
    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h

    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.h
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.h

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.h
)

set(SOURCE_FILES
    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp

    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.cpp

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

    Qt${QT_VERSION_MAJOR}::Core

)

