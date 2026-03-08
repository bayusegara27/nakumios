cmake_minimum_required(VERSION 3.20)
project(nakumi-launcher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
)

add_executable(nakumi-launcher
    src/main.cpp
    src/appmodel.cpp
    src/appmodel.h
    qml.qrc
)

target_link_libraries(nakumi-launcher PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

install(TARGETS nakumi-launcher DESTINATION bin)
