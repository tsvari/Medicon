cmake_minimum_required(VERSION 3.16)

project(FrontendIntegrationTests LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/global-settings.cmake)
# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

find_package(GTest REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/app-data/)
include_directories(${TEST_DATA_DIR})
add_definitions("-DTEST_DATA_DIR=\"${TEST_DATA_DIR}\"")


set(HEADER_FILES
    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/JsonParameterFormatter.h

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.h
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.h

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.h

    ${FRONTEND_GRPC_DIR}/front_common.h
)

set(SOURCE_FILES
    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${INCLUDE_DIR}/JsonParameterFormatter.cpp

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.cpp

    ${FRONTEND_GRPC_DIR}/company_client.hpp

    GrpcCompanyTests.cpp
)

add_executable(FrontendTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(FrontendTestProject PRIVATE
    TEST_GENERATED_ROOT="${TEST_GENERATED_ROOT}"
    TEST_REFRERENCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/reference"
)


target_link_libraries(FrontendTestProject
    PRIVATE
    gtest::gtest

    absl::check
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}

    Qt6::Core

)


