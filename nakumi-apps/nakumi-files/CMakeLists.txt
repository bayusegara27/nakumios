cmake_minimum_required(VERSION 3.20)
project(nakumi-files VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    DBus
)

add_executable(nakumi-files
    src/main.cpp
    src/filemanager.cpp
    src/filemanager.h
    src/mountmanager.cpp
    src/mountmanager.h
    qml/qml.qrc
)

target_link_libraries(nakumi-files PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::DBus
)

install(TARGETS nakumi-files DESTINATION bin)
