# NakumiOS Wayland Compositor

add_executable(nakumi-compositor
    main.cpp
    compositor.cpp
    windowmanager.cpp
)

target_link_libraries(nakumi-compositor PRIVATE
    nakumios_common
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WaylandCompositor
)

# Install
install(TARGETS nakumi-compositor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
