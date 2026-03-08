# NakumiOS - Custom Linux Desktop Environment
# Copyright (c) 2024 NakumiOS Project
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
project(NakumiOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 dependencies
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    DBus
    WaylandCompositor
    Widgets
)

# Enable Qt Quick Compiler for better performance
if(Qt6_FOUND)
    qt_standard_project_setup()
endif()

# Global compile options
add_compile_options(-Wall -Wextra -Wpedantic)

# Define NakumiOS version macros
add_compile_definitions(
    NAKUMIOS_VERSION="${PROJECT_VERSION}"
    NAKUMIOS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    NAKUMIOS_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    NAKUMIOS_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src/common)

# Sub-projects
add_subdirectory(src/common)
add_subdirectory(src/compositor)
add_subdirectory(src/shell)
add_subdirectory(src/services)
add_subdirectory(src/apps/settings)
add_subdirectory(src/apps/terminal)
add_subdirectory(src/apps/filemanager)

# Tests (optional)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

# Installation rules
include(GNUInstallDirs)

install(DIRECTORY resources/themes/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/nakumios/themes
)

install(DIRECTORY resources/icons/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/nakumios/icons
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "nakumios")
set(CPACK_PACKAGE_VENDOR "NakumiOS Project")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_GENERATOR "TGZ")
include(CPack)
