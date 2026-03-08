# NakumiOS Common Library
# Shared utilities and design tokens

add_library(nakumios_common STATIC
    designtokens.cpp
    systeminfo.cpp
)

target_include_directories(nakumios_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nakumios_common PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)
