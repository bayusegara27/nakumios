# NakumiOS Desktop Shell

qt_add_executable(nakumi-shell
    main.cpp
    shellcontroller.cpp
)

qt_add_qml_module(nakumi-shell
    URI NakumiOS.Shell
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/BlurPanel.qml
        qml/components/IconButton.qml
        qml/components/WindowButton.qml
        qml/components/SystemTrayItem.qml
        qml/components/Clock.qml
        qml/panels/TopPanel.qml
        qml/panels/Dock.qml
        qml/launcher/AppLauncher.qml
        qml/launcher/AppGrid.qml
        qml/launcher/AppIcon.qml
    SOURCES
        shellcontroller.h
)

target_link_libraries(nakumi-shell PRIVATE
    nakumios_common
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
)

# Install
install(TARGETS nakumi-shell
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
