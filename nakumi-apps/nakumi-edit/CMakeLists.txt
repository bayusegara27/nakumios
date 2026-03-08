cmake_minimum_required(VERSION 3.20)
project(nakumi-edit VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

add_executable(nakumi-edit
    src/main.cpp
    src/editor.cpp
    src/editor.h
    src/syntaxhighlighter.cpp
    src/syntaxhighlighter.h
    src/linenumberarea.cpp
    src/linenumberarea.h
)

target_link_libraries(nakumi-edit PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

install(TARGETS nakumi-edit DESTINATION bin)
