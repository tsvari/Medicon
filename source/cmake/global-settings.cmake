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
    set(ALL_PROJECT_PATH "")
endif()

set(ALL_PROJECT_APPDATA_PATH ${ALL_PROJECT_PATH}/assets/app-data/)
set(ALL_PROJECT_TEST_PATH ${ALL_PROJECT_PATH}/source/include/tests/app-data/)

add_definitions("-DALL_PROJECT_APPDATA_PATH=\"${ALL_PROJECT_APPDATA_PATH}\"")
add_definitions("-DALL_PROJECT_TEST_PATH=\"${ALL_PROJECT_TEST_PATH}\"")

set(TEST_PROJECT "AllTestProject")
add_definitions("-DTEST_PROJECT=\"${TEST_PROJECT}\"")

set(INCLUDE_DIR ${ALL_PROJECT_PATH}/source/include)
set(THIRD_PARTY_INCLUDE_DIR ${INCLUDE_DIR}/3party)

include_directories(${INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR}/markup)
include_directories(${THIRD_PARTY_INCLUDE_DIR}/json-develop/include)

# Backend settings
set(BACKEND_INCLUDE_DIR ${ALL_PROJECT_PATH}/source/backend/include)
set(THIRD_PARTY_BACKEND_DIR ${BACKEND_INCLUDE_DIR}/3party)

include_directories(${BACKEND_INCLUDE_DIR})
include_directories(${THIRD_PARTY_BACKEND_DIR})
include_directories(${THIRD_PARTY_BACKEND_DIR}/easylogging)

set(BACKEND_TEST_PROJECT "BackendTestProject")
add_definitions("-DBACKEND_TEST_PROJECT=\"${BACKEND_TEST_PROJECT}\"")


