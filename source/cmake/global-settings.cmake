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
