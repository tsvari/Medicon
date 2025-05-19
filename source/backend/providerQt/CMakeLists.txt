cmake_minimum_required(VERSION 3.16)

project(providerQt LANGUAGES CXX)

include_directories(../../cpp-httplib)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(INCLUDE_DIR "../../include")
set(BACKEND_INCLUDE_DIR "../include")
set(THIRD_PARTY_INCLUDE_DIR ${INCLUDE_DIR}/3party)
set(BACKEND_THIRD_PARTY_DIR ${BACKEND_INCLUDE_DIR}/3party)

include_directories(${INCLUDE_DIR})
include_directories(${BACKEND_INCLUDE_DIR})
include_directories(${THIRD_PARTY_INCLUDE_DIR})
include_directories(${BACKEND_THIRD_PARTY_DIR})
include_directories(${BACKEND_THIRD_PARTY_DIR}/cpp-httplib)
include_directories(${BACKEND_THIRD_PARTY_DIR}/json-develop/include)

set(HEADER_FILES
)

set(SOURCE_FILES
    main.cpp
)

add_executable(providerQt
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_link_libraries(providerQt Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS providerQt
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
