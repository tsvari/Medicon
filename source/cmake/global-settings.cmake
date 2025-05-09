# global_settings.cmake
# Set common compiler flags

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set(ALL_PROJECT_PATH "C:/projects/MediCon")
elseif(APPLE)
    # macOS-specific settings
    set(ALL_PROJECT_PATH "")
else()
    # Linux or other Unix-like systems
    set(ALL_PROJECT_PATH "/home/tsvari/Projects/Medicon")
endif()

# All project definations
set(ALL_PROJECT_PATH ${ALL_PROJECT_PATH})
set(ALL_PROJECT_APPDATA_PATH ${ALL_PROJECT_PATH}/assets/app-data/)
set(ALL_PROJECT_TEST_APPDATA_PATH ${ALL_PROJECT_PATH}/source/source/tests/app-data/)

add_definitions("-DALL_PROJECT_PATH=\"${ALL_PROJECT_PATH}\"")
add_definitions("-DALL_PROJECT_APPDATA_PATH=\"${ALL_PROJECT_APPDATA_PATH}\"")
add_definitions("-DALL_PROJECT_TEST_APPDATA_PATH=\"${ALL_PROJECT_TEST_APPDATA_PATH}\"")


set(TEST_PROJECT "AllTestProject")
add_definitions("-DTEST_PROJECT=\"${TEST_PROJECT}\"")

set(INCLUDE_DIR ${ALL_PROJECT_PATH}/source/source)
set(THIRD_PARTY_INCLUDE_DIR ${INCLUDE_DIR}/3party)

include_directories(${INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR}/markup)
include_directories(${THIRD_PARTY_INCLUDE_DIR}/json-develop/include)

# Backend projects definations and settings
set(BACKEND_TEST_PROJECT "BackendTestProject")
add_definitions("-DBACKEND_TEST_PROJECT=\"${BACKEND_TEST_PROJECT}\"")

set(BACKEND_INCLUDE_DIR ${ALL_PROJECT_PATH}/source/backend/source)
set(THIRD_PARTY_BACKEND_DIR ${BACKEND_INCLUDE_DIR}/3party)

include_directories(${BACKEND_INCLUDE_DIR})
include_directories(${THIRD_PARTY_BACKEND_DIR})
include_directories(${THIRD_PARTY_BACKEND_DIR}/easylogging)

set(ALL_BACKEND_PROJECT_PATH ${ALL_PROJECT_PATH}/source/backend/)
set(ALL_BACKEND_TEST_APPDATA_PATH ${BACKEND_INCLUDE_DIR}/tests/app-data/)

add_definitions("-DALL_BACKEND_PROJECT_PATH=\"${ALL_BACKEND_PROJECT_PATH}\"")
add_definitions("-DALL_BACKEND_TEST_APPDATA_PATH=\"${ALL_BACKEND_TEST_APPDATA_PATH}\"")


