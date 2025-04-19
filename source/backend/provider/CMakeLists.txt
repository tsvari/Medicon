cmake_minimum_required(VERSION 3.16)

project(provider LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../cmake/global-settings.cmake)
include(../cmake/global-settings.cmake)

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
get_filename_component(PARENT_CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

# Import SQLAPI the prebuilt static library
add_library(sqlapi STATIC IMPORTED)
IF (WIN32)
    include_directories(${THIRD_PARTY_BACKEND_DIR}/SQLAPI/windows/include)
    # Set SQLAPI paths for debug and release configurations
    set_target_properties(sqlapi PROPERTIES
        IMPORTED_LOCATION_DEBUG "${PARENT_CURRENT_SOURCE_DIR}/include/3party/SQLAPI/windows/vs2022/x86_64/lib/sqlapisd.lib"
        IMPORTED_LOCATION_RELEASE "${PARENT_CURRENT_SOURCE_DIR}/include/3party/SQLAPI/windows/vs2022/x86_64/lib/sqlapis.lib"
    )
ELSE()
  # set stuff for other systems
ENDIF()

set(HEADER_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/markup/Markup.h

    ${BACKEND_INCLUDE_DIR}/include_backend_util.h
    ${BACKEND_INCLUDE_DIR}/sqlapplet.h

    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/configfile.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h
)

set(SOURCE_FILES
    ${THIRD_PARTY_INCLUDE_DIR}/markup/Markup.cpp

    ${BACKEND_INCLUDE_DIR}/include_backend_util.cpp
    ${BACKEND_INCLUDE_DIR}/sqlapplet.cpp

    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/configfile.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
)

# easylogging++
find_package(easyloggingpp REQUIRED)

# sentry-native
find_package(sentry REQUIRED)

# gRPC library
list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/windows/lib/cmake)
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
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
        sentry-native::sentry-native
        gtest::gtest
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
        sentry-native::sentry-native
)

include(GNUInstallDirs)
install(TARGETS provider_lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_TESTS)
    enable_testing()
    set(CMAKE_CTEST_ARGUMENTS "--no-compress-output;--interactive-debug-mode;0;-LE;Online;-L;GoogleTest;--verbose")
    add_subdirectory(tests)
endif()
