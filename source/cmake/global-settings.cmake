# global_settings.cmake
# Set common compiler flags


# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set(ALL_PROJECT_APPDATA_PATH "C:/projects/MediCon/assets/app-data/")
elseif(APPLE)
    # macOS-specific settings
    set(ALL_PROJECT_APPDATA_PATH "")
else()
    # Linux or other Unix-like systems
    set(ALL_PROJECT_APPDATA_PATH "")
endif()

add_definitions("-DALL_PROJECT_APPDATA_PATH=\"${ALL_PROJECT_APPDATA_PATH}\"")

set(INCLUDE_DIR "../../include")
set(THIRD_PARTY_INCLUDE_DIR ${INCLUDE_DIR}/3party)

include_directories(${INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR}/markup)
include_directories(${THIRD_PARTY_INCLUDE_DIR}/json-develop/include)

