cmake_minimum_required(VERSION 3.16)

project(provider LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(global-settings)

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)
# Easylogging should compile with STL
add_compile_definitions(ELPP_STL_LOGGING)
# sqlapi static
add_compile_definitions(SA_STATIC_PQSQL)

# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

# Tests turn on/off
set(BUILD_TESTS ON CACHE BOOL "If enabled, build 'provider' unit tests")

# Import SQLAPI the prebuilt static library
include(sqlapi-config)

set(HEADER_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${BACKEND_INCLUDE_DIR}/include_backend_util.h
    ${BACKEND_INCLUDE_DIR}/sqltemplate.h
    ${BACKEND_INCLUDE_DIR}/column_allowlist.h
    ${BACKEND_INCLUDE_DIR}/sqlconnection.h
    ${BACKEND_INCLUDE_DIR}/sqlcommand.h
    ${BACKEND_INCLUDE_DIR}/sqlquery.h

    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/configfile.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/JsonParameterFormatter.h

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.h
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.h

    ${BACKEND_GRPC_DIR}/company/company_types.h
    ${BACKEND_GRPC_DIR}/company/company_repository.h
    ${BACKEND_GRPC_DIR}/company/company_service.h
    ${BACKEND_GRPC_DIR}/company/company_server.h
)

set(SOURCE_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${BACKEND_GRPC_DIR}/company/company_repository.cpp
    ${BACKEND_GRPC_DIR}/company/company_service.cpp
    ${BACKEND_GRPC_DIR}/company/company_server.cpp

    ${BACKEND_INCLUDE_DIR}/include_backend_util.cpp
    ${BACKEND_INCLUDE_DIR}/sqltemplate.cpp
    ${BACKEND_INCLUDE_DIR}/sqlconnection.cpp
    ${BACKEND_INCLUDE_DIR}/sqlcommand.cpp
    ${BACKEND_INCLUDE_DIR}/sqlquery.cpp

    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/configfile.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${INCLUDE_DIR}/JsonParameterFormatter.cpp

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc

    # company_server.hpp is now in company/ subdirectory
)

# easylogging++
find_package(easyloggingpp REQUIRED)

# sentry-native
#find_package(sentry REQUIRED)

find_package(GTest REQUIRED)

add_library(provider_lib STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

IF(WIN32)
target_link_libraries(provider_lib
    PRIVATE
        user32.lib
        version.lib
        oleaut32.lib
        ole32.lib
)
endif(WIN32)

target_link_libraries(provider_lib
    PRIVATE
        # Project-wide includes and definitions
        medicon_global_includes

        # SQL API
        sqlapi

        # Conan packages
        easyloggingpp::easyloggingpp
        #sentry-native::sentry-native
        gtest::gtest

        absl::check
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
)

# Just executable
add_executable(provider
    main.cpp
)

IF(WIN32)
target_link_libraries(provider
    PRIVATE
        user32.lib
        version.lib
        oleaut32.lib
        ole32.lib
)
endif(WIN32)
target_link_libraries(provider
    PRIVATE
        provider_lib
        # Conan packages
        easyloggingpp::easyloggingpp
        #sentry-native::sentry-native
        absl::check
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
)

include(GNUInstallDirs)
install(TARGETS provider_lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_TESTS)
    enable_testing()
    set(CMAKE_CTEST_ARGUMENTS "--no-compress-output;--interactive-debug-mode;0;-LE;Online;-L;GoogleTest;--verbose")
    add_subdirectory(unit_tests)
    add_subdirectory(integration_tests)
    add_subdirectory(connection_tests)
endif()
