# backend global_settings.cmake
# Backend globals

include(../../cmake/global-settings.cmake)

set(BACKEND_INCLUDE_DIR "../include")
set(THIRD_PARTY_BACKEND_DIR ${BACKEND_INCLUDE_DIR}/3party)

include_directories(${BACKEND_INCLUDE_DIR})
include_directories(${THIRD_PARTY_BACKEND_DIR})
include_directories(${THIRD_PARTY_BACKEND_DIR}/easylogging)
