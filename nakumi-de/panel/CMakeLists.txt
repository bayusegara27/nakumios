cmake_minimum_required(VERSION 3.20)
project(nakumi-panel VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(FindPkgConfig)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    WaylandClient
    DBus
)

# Find wayland-scanner and protocols
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

# ============================================================================
# Generate layer-shell protocol bindings using qtwaylandscanner
# ============================================================================
# CRITICAL: Standard Qt6 does NOT have native layer-shell QML bindings.
# We must generate C++ headers from the protocol XML using qtwaylandscanner.

find_program(QT_WAYLAND_SCANNER NAMES qtwaylandscanner qt6-qtwaylandscanner REQUIRED)

set(LAYER_SHELL_XML
    "${CMAKE_SOURCE_DIR}/../../nakumi-wm/protocols/wlr-layer-shell-unstable-v1.xml")

# Generate client-side C++ header from the layer-shell protocol XML
set(LAYER_SHELL_CLIENT_HEADER
    "${CMAKE_CURRENT_BINARY_DIR}/qwayland-wlr-layer-shell-unstable-v1.h")
set(LAYER_SHELL_CLIENT_SOURCE
    "${CMAKE_CURRENT_BINARY_DIR}/qwayland-wlr-layer-shell-unstable-v1.cpp")

add_custom_command(
    OUTPUT ${LAYER_SHELL_CLIENT_HEADER}
    COMMAND ${QT_WAYLAND_SCANNER} client-header
            ${LAYER_SHELL_XML} > ${LAYER_SHELL_CLIENT_HEADER}
    DEPENDS ${LAYER_SHELL_XML}
    COMMENT "Generating layer-shell client header with qtwaylandscanner"
)

add_custom_command(
    OUTPUT ${LAYER_SHELL_CLIENT_SOURCE}
    COMMAND ${QT_WAYLAND_SCANNER} client-code
            ${LAYER_SHELL_XML} > ${LAYER_SHELL_CLIENT_SOURCE}
    DEPENDS ${LAYER_SHELL_XML} ${LAYER_SHELL_CLIENT_HEADER}
    COMMENT "Generating layer-shell client code with qtwaylandscanner"
)

# Generate wayland-scanner protocol code (low-level C bindings)
set(LAYER_SHELL_PROTOCOL_HEADER
    "${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1-client-protocol.h")
set(LAYER_SHELL_PROTOCOL_SOURCE
    "${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1-protocol.c")

add_custom_command(
    OUTPUT ${LAYER_SHELL_PROTOCOL_HEADER}
    COMMAND ${WAYLAND_SCANNER} client-header
            ${LAYER_SHELL_XML} ${LAYER_SHELL_PROTOCOL_HEADER}
    DEPENDS ${LAYER_SHELL_XML}
    COMMENT "Generating layer-shell wayland client protocol header"
)

add_custom_command(
    OUTPUT ${LAYER_SHELL_PROTOCOL_SOURCE}
    COMMAND ${WAYLAND_SCANNER} private-code
            ${LAYER_SHELL_XML} ${LAYER_SHELL_PROTOCOL_SOURCE}
    DEPENDS ${LAYER_SHELL_XML}
    COMMENT "Generating layer-shell wayland protocol code"
)

# ============================================================================
# Panel executable
# ============================================================================

add_executable(nakumi-panel
    src/main.cpp
    src/layershellwindow.cpp
    src/layershellwindow.h
    src/panelcontroller.cpp
    src/panelcontroller.h
    src/taskmanager.cpp
    src/taskmanager.h
    src/systemtray.cpp
    src/systemtray.h
    ${LAYER_SHELL_CLIENT_HEADER}
    ${LAYER_SHELL_CLIENT_SOURCE}
    ${LAYER_SHELL_PROTOCOL_HEADER}
    ${LAYER_SHELL_PROTOCOL_SOURCE}
    qml/qml.qrc
)

target_include_directories(nakumi-panel PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
)

target_link_libraries(nakumi-panel PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WaylandClient
    Qt6::WaylandClientPrivate
    Qt6::DBus
    ${WAYLAND_CLIENT_LIBRARIES}
)

install(TARGETS nakumi-panel DESTINATION bin)
