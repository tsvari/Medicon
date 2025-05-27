cmake_minimum_required(VERSION 3.16)

project(provider LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../source/cmake/global-settings.cmake)

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

# one level above CMAKE_CURRENT_SOURCE_DIR
#get_filename_component(PARENT_CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

# Import SQLAPI the prebuilt static library
add_library(sqlapi STATIC IMPORTED)
IF (WIN32)
    include_directories(${BACKEND_THIRD_PARTY_DIR}/SQLAPI/windows/include)
    # Set SQLAPI paths for debug and release configurations
    set_target_properties(sqlapi PROPERTIES
      IMPORTED_LOCATION_DEBUG "${BACKEND_INCLUDE_DIR}/3party/SQLAPI/windows/vs2022/x86_64/lib/sqlapisd.lib"
      IMPORTED_LOCATION_RELEASE "${BACKEND_INCLUDE_DIR}/3party/SQLAPI/windows/vs2022/x86_64/lib/sqlapis.lib"
    )
ELSE()
    include_directories(${BACKEND_THIRD_PARTY_DIR}/SQLAPI/linux/sqlapi-5.3.5/include)
    # set stuff for other systems
    set_target_properties(sqlapi PROPERTIES
      IMPORTED_LOCATION_DEBUG "${BACKEND_INCLUDE_DIR}/3party/SQLAPI/linux/sqlapi-5.3.5/lib/libsqlapi.a"
      IMPORTED_LOCATION_RELEASE "${BACKEND_INCLUDE_DIR}/3party/SQLAPI/linux/sqlapi-5.3.5/lib/libsqlapi.a"
    )
ENDIF()

set(HEADER_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${BACKEND_INCLUDE_DIR}/include_backend_util.h
    ${BACKEND_INCLUDE_DIR}/sqlapplet.h
    ${BACKEND_INCLUDE_DIR}/sqlconnection.h
    ${BACKEND_INCLUDE_DIR}/sqlcommand.h
    ${BACKEND_INCLUDE_DIR}/sqlquery.h

    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/configfile.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.h
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.h
)

set(SOURCE_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${BACKEND_INCLUDE_DIR}/include_backend_util.cpp
    ${BACKEND_INCLUDE_DIR}/sqlapplet.cpp
    ${BACKEND_INCLUDE_DIR}/sqlconnection.cpp
    ${BACKEND_INCLUDE_DIR}/sqlcommand.cpp
    ${BACKEND_INCLUDE_DIR}/sqlquery.cpp

    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/configfile.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp

    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
    ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc

    ${BACKEND_GRPC_DIR}/company_server.hpp
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
endif()
