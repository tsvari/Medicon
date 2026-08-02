# GrpcProtoTests — standalone gRPC domain tests
#
# Tests Company (and future User/License/Audit) domain services independently
# of the provider application. Compiles the domain + SQL infrastructure sources
# directly — no dependency on the provider build.
#
# Build:
#   cd source/cpp/backend/grpc/tests
#   conan install . --output-folder build/x86-64/Debug -pr:b x86_64-windows-msvc194 \
#       -pr:h x86_64-windows-msvc194 -s build_type=Debug --build missing
#   cmake -S . -B build/x86-64/Debug/build -G "Visual Studio 17 2022" \
#       -DCMAKE_TOOLCHAIN_FILE=build/x86-64/Debug/build/generators/conan_toolchain.cmake
#   cmake --build build/x86-64/Debug/build --config Debug

cmake_minimum_required(VERSION 3.16)

project(GrpcProtoTests LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
include(global-settings)

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)
# Easylogging should compile with STL
add_compile_definitions(ELPP_STL_LOGGING)
# sqlapi static
add_compile_definitions(SA_STATIC_PQSQL)

# Use project name in c++ code (PROJECT_NAME from global-settings)
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

# Import SQLAPI the prebuilt static library
include(sqlapi-config)

# All sources needed to test the company domain (self-contained)
set(SOURCE_FILES
    # Shared utilities
    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/JsonParameterFormatter.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    # SQL infrastructure
    ${BACKEND_INCLUDE_DIR}/include_backend_util.cpp
    ${BACKEND_INCLUDE_DIR}/sqltemplate.cpp
    ${BACKEND_INCLUDE_DIR}/sqlconnection.cpp
    ${BACKEND_INCLUDE_DIR}/sqlcommand.cpp
    ${BACKEND_INCLUDE_DIR}/sqlquery.cpp

    # Company domain
    ${BACKEND_GRPC_DIR}/company/company_repository.cpp
    ${BACKEND_GRPC_DIR}/company/company_service.cpp
    ${BACKEND_GRPC_DIR}/company/company_server.cpp

    # Generated protobuf surface
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc

    # Test files
    test_main.cpp
    CompanyServiceTests.cpp
    CompanyServiceImplTests.cpp
)

add_executable(grpc_proto_tests
    ${SOURCE_FILES}
)

find_package(easyloggingpp REQUIRED)
find_package(GTest REQUIRED)

target_link_libraries(grpc_proto_tests
    PRIVATE
        sqlapi
        easyloggingpp::easyloggingpp
        gtest::gtest
        absl::check
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
)

if(WIN32)
    target_link_libraries(grpc_proto_tests
        PRIVATE
            user32.lib
            version.lib
            oleaut32.lib
            ole32.lib
    )
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
    gtest_discover_tests(grpc_proto_tests
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
