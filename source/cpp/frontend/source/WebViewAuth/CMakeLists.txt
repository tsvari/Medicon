cmake_minimum_required(VERSION 3.23)
project(WebViewAuth VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
include(global-settings)

find_package(Qt6 COMPONENTS Core Gui Widgets Network NetworkAuth QuickWidgets WebView)


# Library
add_library(WebViewAuth STATIC)
target_sources(WebViewAuth
    PUBLIC
        AuthSession.h
    PRIVATE
        AuthSession.cpp
)
target_link_libraries(WebViewAuth
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::NetworkAuth
        Qt6::QuickWidgets
        Qt6::WebView
)

# Test App
add_executable(TestWebViewAuth
    main.cpp
)
target_link_libraries(TestWebViewAuth
    PRIVATE
        Qt6::Core
        WebViewAuth
)


include(GNUInstallDirs)
install(TARGETS WebViewAuth
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
