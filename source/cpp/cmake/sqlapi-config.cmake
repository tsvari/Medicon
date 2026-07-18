# sqlapi-config.cmake
# Find-module for the prebuilt SQLAPI++ library.
# Defines the IMPORTED target `sqlapi`.
#
# Requires these variables to be set first (by global-settings.cmake):
#   BACKEND_INCLUDE_DIR    - path to backend/source (e.g. .../source/backend/source)
#   BACKEND_THIRD_PARTY_DIR - path to backend 3party dir

if(NOT TARGET sqlapi)
    add_library(sqlapi STATIC IMPORTED)

    if(WIN32)
        set(_SQLAPI_SUBDIR "windows/vs2022/x86_64")
        set(_SQLAPI_LIB_DEBUG "sqlapisd.lib")
        set(_SQLAPI_LIB_RELEASE "sqlapis.lib")
        set(_SQLAPI_INCLUDE_DIR "${BACKEND_THIRD_PARTY_DIR}/SQLAPI/windows/include")
    elseif(UNIX AND NOT APPLE)
        set(_SQLAPI_SUBDIR "linux/sqlapi-5.3.5")
        set(_SQLAPI_LIB_DEBUG "libsqlapi.a")
        set(_SQLAPI_LIB_RELEASE "libsqlapi.a")
        set(_SQLAPI_INCLUDE_DIR "${BACKEND_THIRD_PARTY_DIR}/SQLAPI/linux/sqlapi-5.3.5/include")
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}. Medicon only supports Windows and Linux.")
    endif()

    set_target_properties(sqlapi PROPERTIES
        IMPORTED_LOCATION_DEBUG            "${BACKEND_INCLUDE_DIR}/3party/SQLAPI/${_SQLAPI_SUBDIR}/lib/${_SQLAPI_LIB_DEBUG}"
        IMPORTED_LOCATION_RELEASE          "${BACKEND_INCLUDE_DIR}/3party/SQLAPI/${_SQLAPI_SUBDIR}/lib/${_SQLAPI_LIB_RELEASE}"
        INTERFACE_INCLUDE_DIRECTORIES      "${_SQLAPI_INCLUDE_DIR}"
    )

    mark_as_advanced(_SQLAPI_SUBDIR _SQLAPI_LIB_DEBUG _SQLAPI_LIB_RELEASE _SQLAPI_INCLUDE_DIR)
endif()
