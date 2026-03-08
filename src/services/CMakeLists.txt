# NakumiOS System Services

add_library(nakumios_services STATIC
    sessionmanager.cpp
    notificationservice.cpp
)

target_include_directories(nakumios_services PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nakumios_services PUBLIC
    nakumios_common
    Qt6::Core
    Qt6::DBus
)
