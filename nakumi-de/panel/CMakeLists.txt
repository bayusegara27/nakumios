cmake_minimum_required(VERSION 3.20)
project(nakumi-panel VERSION 1.0.0 LANGUAGES C CXX)

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

# qtwaylandscanner lives in Qt6's libexec directory, which is not in PATH
# on most distributions. Add common install paths as hints.
find_program(QT_WAYLAND_SCANNER
    NAMES qtwaylandscanner qt6-qtwaylandscanner
    HINTS
        /usr/lib/qt6/libexec
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt6/libexec
    REQUIRED
)

set(LAYER_SHELL_XML_ORIG
    "${CMAKE_SOURCE_DIR}/../../nakumi-wm/protocols/wlr-layer-shell-unstable-v1.xml")

# ---- Pre-process protocol XML ------------------------------------------------
# The protocol XML uses "namespace" as a parameter name (<arg name="namespace">).
# Since "namespace" is a C++ reserved keyword, we create a modified copy of the
# XML that renames the argument to "name_space" BEFORE feeding it to any code
# generator.  This is more robust than post-processing generated files because
# it avoids shell-escaping issues with sed word-boundary patterns (\b) across
# CMake → Makefile → shell.
# ------------------------------------------------------------------------------
set(LAYER_SHELL_XML "${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1.xml")

# cmake -P script that copies a file then does an in-place sed replacement.
# Using a script avoids any \b / quoting issues in Makefile recipe lines.
set(PREPROCESS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/preprocess_xml.cmake")
file(WRITE "${PREPROCESS_SCRIPT}" [=[
file(READ "${SRC}" content)
string(REPLACE "name=\"namespace\"" "name=\"name_space\"" content "${content}")
file(WRITE "${DST}" "${content}")
]=])

add_custom_command(
    OUTPUT ${LAYER_SHELL_XML}
    COMMAND ${CMAKE_COMMAND}
        "-DSRC=${LAYER_SHELL_XML_ORIG}"
        "-DDST=${LAYER_SHELL_XML}"
        -P "${PREPROCESS_SCRIPT}"
    DEPENDS ${LAYER_SHELL_XML_ORIG}
    COMMENT "Pre-processing layer-shell protocol XML (rename C++ keyword)"
)

# ---- qtwaylandscanner (C++ bindings) ----------------------------------------
# qtwaylandscanner writes to stdout.  We capture it via a cmake -P script that
# uses execute_process(OUTPUT_FILE ...).
set(CAPTURE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/capture_stdout.cmake")
file(WRITE "${CAPTURE_SCRIPT}" [=[
separate_arguments(ARG_LIST UNIX_COMMAND "${ARGS}")
execute_process(COMMAND "${CMD}" ${ARG_LIST} OUTPUT_FILE "${OUT}" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${CMD} ${ARGS} failed with exit code ${rc}")
endif()
]=])

set(LAYER_SHELL_CLIENT_HEADER
    "${CMAKE_CURRENT_BINARY_DIR}/qwayland-wlr-layer-shell-unstable-v1.h")
set(LAYER_SHELL_CLIENT_SOURCE
    "${CMAKE_CURRENT_BINARY_DIR}/qwayland-wlr-layer-shell-unstable-v1.cpp")

add_custom_command(
    OUTPUT ${LAYER_SHELL_CLIENT_HEADER}
    COMMAND ${CMAKE_COMMAND}
        "-DCMD=${QT_WAYLAND_SCANNER}"
        "-DARGS=client-header ${LAYER_SHELL_XML}"
        "-DOUT=${LAYER_SHELL_CLIENT_HEADER}"
        -P "${CAPTURE_SCRIPT}"
    DEPENDS ${LAYER_SHELL_XML}
    COMMENT "Generating layer-shell client header with qtwaylandscanner"
)

add_custom_command(
    OUTPUT ${LAYER_SHELL_CLIENT_SOURCE}
    COMMAND ${CMAKE_COMMAND}
        "-DCMD=${QT_WAYLAND_SCANNER}"
        "-DARGS=client-code ${LAYER_SHELL_XML}"
        "-DOUT=${LAYER_SHELL_CLIENT_SOURCE}"
        -P "${CAPTURE_SCRIPT}"
    DEPENDS ${LAYER_SHELL_XML} ${LAYER_SHELL_CLIENT_HEADER}
    COMMENT "Generating layer-shell client code with qtwaylandscanner"
)

# ---- wayland-scanner (low-level C bindings) ----------------------------------
# qtwaylandscanner-generated headers expect the wayland- prefix in the include
# filename (wayland-<protocol>-client-protocol.h), so we must use matching
# output names here.
set(LAYER_SHELL_PROTOCOL_HEADER
    "${CMAKE_CURRENT_BINARY_DIR}/wayland-wlr-layer-shell-unstable-v1-client-protocol.h")
set(LAYER_SHELL_PROTOCOL_SOURCE
    "${CMAKE_CURRENT_BINARY_DIR}/wayland-wlr-layer-shell-unstable-v1-protocol.c")

add_custom_command(
    OUTPUT ${LAYER_SHELL_PROTOCOL_HEADER}
    COMMAND ${WAYLAND_SCANNER} client-header ${LAYER_SHELL_XML} ${LAYER_SHELL_PROTOCOL_HEADER}
    DEPENDS ${LAYER_SHELL_XML}
    COMMENT "Generating layer-shell wayland client protocol header"
)

add_custom_command(
    OUTPUT ${LAYER_SHELL_PROTOCOL_SOURCE}
    COMMAND ${WAYLAND_SCANNER} private-code ${LAYER_SHELL_XML} ${LAYER_SHELL_PROTOCOL_SOURCE}
    DEPENDS ${LAYER_SHELL_XML}
    COMMENT "Generating layer-shell wayland protocol code"
)

# ---- xdg-shell protocol (dependency of wlr-layer-shell) ---------------------
# The wlr-layer-shell protocol references xdg_popup, so we must also generate
# and compile the xdg-shell protocol code to provide the xdg_popup_interface
# symbol at link time.
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
if(NOT WAYLAND_PROTOCOLS_DIR)
    message(FATAL_ERROR "Could not find wayland-protocols pkgdatadir. Is wayland-protocols installed?")
endif()
set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")

set(XDG_SHELL_PROTOCOL_HEADER
    "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h")
set(XDG_SHELL_PROTOCOL_SOURCE
    "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c")

add_custom_command(
    OUTPUT ${XDG_SHELL_PROTOCOL_HEADER}
    COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_XML} ${XDG_SHELL_PROTOCOL_HEADER}
    DEPENDS ${XDG_SHELL_XML}
    COMMENT "Generating xdg-shell wayland client protocol header"
)

add_custom_command(
    OUTPUT ${XDG_SHELL_PROTOCOL_SOURCE}
    COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_XML} ${XDG_SHELL_PROTOCOL_SOURCE}
    DEPENDS ${XDG_SHELL_XML}
    COMMENT "Generating xdg-shell wayland protocol code"
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
    ${XDG_SHELL_PROTOCOL_HEADER}
    ${XDG_SHELL_PROTOCOL_SOURCE}
    qml.qrc
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
