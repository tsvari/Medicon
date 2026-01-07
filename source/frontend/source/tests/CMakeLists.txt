cmake_minimum_required(VERSION 3.16)

project(FrontendTestProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/global-settings.cmake)
# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

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

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.h
    ${FRONTEND_INCLUDE_DIR}/GrpcObjectTableModel.h
    ${FRONTEND_INCLUDE_DIR}/GrpcDataContainer.hpp
    ${FRONTEND_INCLUDE_DIR}/GrpcObjectWrapper.hpp
    ${FRONTEND_INCLUDE_DIR}/TestSharedUtility.h
)

set(SOURCE_FILES
    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcObjectTableModel.cpp
    ${FRONTEND_INCLUDE_DIR}/TestSharedUtility.cpp
)

add_executable(FrontendTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}

    #GrpcObjectTableModelTests.cpp
    GrpcDataContainerTests.cpp
    #GrpcObjectWrapperTests.cpp
    TestSharedUtilityTests.cpp
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
    Qt6::Widgets
    Qt6::Gui
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FrontendTestProject)
endif()

