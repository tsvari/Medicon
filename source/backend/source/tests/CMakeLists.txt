cmake_minimum_required(VERSION 3.16)

project(BackendTestProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/global-settings.cmake)

# Markup should compile with STL
add_compile_definitions(MARKUP_STL)

# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVER_MODE PRE_TEST)
include(GoogleTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${TEST_GENERATED_ROOT})

# sqlapi static
add_compile_definitions(SA_STATIC_PQSQL)
# Easylogging should compile with STL
add_compile_definitions(ELPP_STL_LOGGING)
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
    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/JsonParameterFormatter.h

    ${THIRD_PARTY_INCLUDE_DIR}/Markup/Markup.h

    ${BACKEND_INCLUDE_DIR}/include_backend_util.h
    ${BACKEND_INCLUDE_DIR}/sqlapplet.h
    ${BACKEND_INCLUDE_DIR}/sqlconnection.h
    ${BACKEND_INCLUDE_DIR}/sqlcommand.h
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

    SqlAppletTests.cpp
    SqlConnectionTests.cpp
    SqlConnectionIntegrationTests.cpp
    SqlCommandTests.cpp
    SqlCommandIntegrationTests.cpp
    SaBinaryTests.cpp


)

add_executable(BackendTestProject
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(BackendTestProject PRIVATE
    TEST_GENERATED_ROOT="${TEST_GENERATED_ROOT}"
    TEST_REFRERENCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/reference"
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

IF(WIN32)
target_link_libraries(BackendTestProject
    PRIVATE
        user32.lib
        version.lib
        oleaut32.lib
        ole32.lib
)
endif(WIN32)
