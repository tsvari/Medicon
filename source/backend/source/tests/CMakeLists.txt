cmake_minimum_required(VERSION 3.16)

project(BackendTestProject LANGUAGES CXX)

include(CTest)
enable_testing()

include(../../../cmake/global-settings.cmake)

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

# Import SQLAPI the prebuilt static library
include(../../../cmake/sqlapi-config.cmake)

set(HEADER_FILES
    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/JsonParameterFormatter.h

    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${BACKEND_INCLUDE_DIR}/include_backend_util.h
    ${BACKEND_INCLUDE_DIR}/sqlapplet.h
    ${BACKEND_INCLUDE_DIR}/sqlconnection.h
    ${BACKEND_INCLUDE_DIR}/sqlcommand.h
    ${BACKEND_INCLUDE_DIR}/sqlquery.h
)

set(SOURCE_FILES
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/JsonParameterFormatter.cpp
	
    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.cpp

    ${BACKEND_INCLUDE_DIR}/include_backend_util.cpp
    ${BACKEND_INCLUDE_DIR}/sqlapplet.cpp
    ${BACKEND_INCLUDE_DIR}/sqlconnection.cpp
    ${BACKEND_INCLUDE_DIR}/sqlcommand.cpp
    ${BACKEND_INCLUDE_DIR}/sqlquery.cpp

    SqlAppletTests.cpp
    SqlConnectionTests.cpp
    SqlConnectionIntegrationTests.cpp
    SqlCommandTests.cpp
    SqlCommandIntegrationTests.cpp
    SaBinaryTests.cpp
    SqlQueryTests.cpp
    SqlQueryIntegrationTests.cpp
)

add_executable(BackendTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# easylogging++
find_package(easyloggingpp REQUIRED)

find_package(GTest REQUIRED)


target_link_libraries(BackendTestProject
    PRIVATE
    sqlapi
    gtest::gtest
    easyloggingpp::easyloggingpp
)

gtest_discover_tests(BackendTestProject
    DISCOVERY_TIMEOUT 60
)

IF(WIN32)
target_link_libraries(BackendTestProject
    PRIVATE
        user32.lib
        version.lib
        oleaut32.lib
        ole32.lib
)
endif(WIN32)
