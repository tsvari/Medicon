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
set(ALL_PROJECT_GRPC_PROTOS_PATH ${ALL_PROJECT_PATH}/source/grpc/protos/)
set(ALL_PROJECT_GRPC_CPP_SOURCE ${ALL_PROJECT_PATH}/source/grpc/cpp-source/)

include_directories(${ALL_PROJECT_GRPC_CPP_SOURCE})

add_definitions("-DALL_PROJECT_PATH=\"${ALL_PROJECT_PATH}\"")
add_definitions("-DALL_PROJECT_APPDATA_PATH=\"${ALL_PROJECT_APPDATA_PATH}\"")
add_definitions("-DALL_PROJECT_TEST_APPDATA_PATH=\"${ALL_PROJECT_TEST_APPDATA_PATH}\"")

set(INCLUDE_DIR ${ALL_PROJECT_PATH}/source/source)
set(THIRD_PARTY_INCLUDE_DIR ${INCLUDE_DIR}/3party)

include_directories(${INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR}/markup)
include_directories(${THIRD_PARTY_INCLUDE_DIR}/json-develop/include)

# Backend projects definations and settings
set(BACKEND_INCLUDE_DIR ${ALL_PROJECT_PATH}/source/backend/source)
set(BACKEND_THIRD_PARTY_DIR ${BACKEND_INCLUDE_DIR}/3party)
set(BACKEND_GRPC_DIR ${ALL_PROJECT_PATH}/source/backend/grpc)

include_directories(${BACKEND_INCLUDE_DIR})
include_directories(${BACKEND_THIRD_PARTY_DIR})
include_directories(${BACKEND_GRPC_DIR})

set(ALL_BACKEND_PROJECT_PATH ${ALL_PROJECT_PATH}/source/backend/)
set(ALL_BACKEND_TEST_APPDATA_PATH ${BACKEND_INCLUDE_DIR}/tests/app-data/)

add_definitions("-DALL_BACKEND_PROJECT_PATH=\"${ALL_BACKEND_PROJECT_PATH}\"")
add_definitions("-DALL_BACKEND_TEST_APPDATA_PATH=\"${ALL_BACKEND_TEST_APPDATA_PATH}\"")

# Frontend projects definations and settings
set(FRONTEND_INCLUDE_DIR ${ALL_PROJECT_PATH}/source/frontend/source)
set(FRONTEND_THIRD_PARTY_DIR ${FRONTEND_INCLUDE_DIR}/3party)
set(FRONTEND_GRPC_DIR ${ALL_PROJECT_PATH}/source/frontend/grpc)

include_directories(${FRONTEND_INCLUDE_DIR})
include_directories(${FRONTEND_THIRD_PARTY_DIR})
include_directories(${FRONTEND_GRPC_DIR})

set(ALL_FRONTEND_PROJECT_PATH ${ALL_PROJECT_PATH}/source/backend/)
set(ALL_FRONTEND_TEST_APPDATA_PATH ${FRONTEND_INCLUDE_DIR}/tests/app-data/)

add_definitions("-DALL_FRONTEND_PROJECT_PATH=\"${ALL_FRONTEND_PROJECT_PATH}\"")
add_definitions("-DALL_FRONTEND_TEST_APPDATA_PATH=\"${ALL_FRONTEND_TEST_APPDATA_PATH}\"")


#list(APPEND CMAKE_PREFIX_PATH ${ALL_PROJECT_PATH}/source/source/3party/grpc/build/windows/lib/cmake)

IF (WIN32)
    #if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        include_directories(${ALL_PROJECT_PATH}/source/source/3party/grpc/build/windows/Debug/include/)
        list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/windows/Debug/lib/cmake)
    #elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        #list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/windows/Release/lib/cmake)
    #endif()
ELSE()
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_INCLUDE_DIR}/grpc/build/linux/lib/cmake)
ENDIF()

# This branch assumes that gRPC and all its dependencies are already installed
# on this system, so they can be located by find_package().

find_package(Threads REQUIRED)
# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()
