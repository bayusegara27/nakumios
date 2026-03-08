cmake_minimum_required(VERSION 3.20)
project(nakumi-tests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Core Test)

# ==============================================================================
# ANSI Parser Tests (NakumiTerm)
# ==============================================================================

add_executable(test_ansiparser
    test_ansiparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../nakumi-apps/nakumi-term/src/ansiparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../nakumi-apps/nakumi-term/src/ansiparser.h
)

target_include_directories(test_ansiparser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../nakumi-apps/nakumi-term/src
)

target_link_libraries(test_ansiparser PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME AnsiParserTest COMMAND test_ansiparser)

# ==============================================================================
# FileManager Tests (NakumiFiles)
# ==============================================================================

add_executable(test_filemanager
    test_filemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../nakumi-apps/nakumi-files/src/filemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../nakumi-apps/nakumi-files/src/filemanager.h
)

target_include_directories(test_filemanager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../nakumi-apps/nakumi-files/src
)

target_link_libraries(test_filemanager PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME FileManagerTest COMMAND test_filemanager)
