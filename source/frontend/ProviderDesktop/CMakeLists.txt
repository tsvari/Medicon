cmake_minimum_required(VERSION 3.19)
project(ProviderDesktop LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(UI_HEADER_FILES
    mainwindow.ui
)

set(HEADER_FILES
    mainwindow.h
)

set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

qt_add_executable(ProviderDesktop
    WIN32 MACOSX_BUNDLE
    ${UI_HEADER_FILES}
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_link_libraries(ProviderDesktop
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS ProviderDesktop
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ProviderDesktop
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
