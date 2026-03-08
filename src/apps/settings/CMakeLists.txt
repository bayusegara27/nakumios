# NakumiOS Settings Application

qt_add_executable(nakumi-settings
    main.cpp
    settingscontroller.cpp
)

qt_add_qml_module(nakumi-settings
    URI NakumiOS.Settings
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/SettingsPage.qml
        qml/pages/AppearancePage.qml
        qml/pages/DisplayPage.qml
        qml/pages/SoundPage.qml
        qml/pages/NetworkPage.qml
        qml/pages/AboutPage.qml
    SOURCES
        settingscontroller.h
)

target_link_libraries(nakumi-settings PRIVATE
    nakumios_common
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
)

install(TARGETS nakumi-settings
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Desktop file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/org.nakumios.settings.desktop.in
    ${CMAKE_BINARY_DIR}/org.nakumios.settings.desktop
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/org.nakumios.settings.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
