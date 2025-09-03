cmake_minimum_required(VERSION 3.16)

project(ItemModelTestUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../../../cmake/global-settings.cmake)

# Use project name in c++ code
add_definitions("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(HEADER_FILES
    ${INCLUDE_DIR}/include_util.h
    ${INCLUDE_DIR}/TypeToStringFormatter.h
    ${INCLUDE_DIR}/JsonParameterFormatter.h

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.h
    ${FRONTEND_INCLUDE_DIR}/GrpcObjectTableModel.h
    ${FRONTEND_INCLUDE_DIR}/GrpcDataContainer.hpp
    ${FRONTEND_INCLUDE_DIR}/GrpcProxySortFilterModel.h
    ${FRONTEND_INCLUDE_DIR}/GrpcForm.h
    ${FRONTEND_INCLUDE_DIR}/GrpcTemplateController.h
    ${FRONTEND_INCLUDE_DIR}/GrpcMasterSlaveController.h
    ${FRONTEND_INCLUDE_DIR}/GrpcObjectWrapper.hpp
    ${FRONTEND_INCLUDE_DIR}/GrpcSearchForm.h


    ${FRONTEND_INCLUDE_DIR}/TestSharedUtility.h
    ${FRONTEND_GRPC_DIR}/front_common.h


)

set(SOURCE_FILES
    ${INCLUDE_DIR}/include_util.cpp
    ${INCLUDE_DIR}/TypeToStringFormatter.cpp
    ${INCLUDE_DIR}/JsonParameterFormatter.cpp

    ${FRONTEND_INCLUDE_DIR}/include_frontend_util.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcObjectTableModel.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcProxySortFilterModel.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcForm.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcTemplateController.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcMasterSlaveController.cpp
    ${FRONTEND_INCLUDE_DIR}/GrpcSearchForm.cpp

    ${FRONTEND_INCLUDE_DIR}/TestSharedUtility.cpp
)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
)

set(RESOURCE_FILES
    ${FRONTEND_INCLUDE_DIR}/Frontend.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ItemModelTestUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${HEADER_FILES}
        ${SOURCE_FILES}
        ${RESOURCE_FILES}
    )
endif()

target_link_libraries(ItemModelTestUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ItemModelTestUI)
endif()
set_target_properties(ItemModelTestUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ItemModelTestUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ItemModelTestUI)
endif()
