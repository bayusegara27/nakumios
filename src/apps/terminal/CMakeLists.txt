# NakumiOS Terminal Application

qt_add_executable(nakumi-terminal
    main.cpp
    terminalcontroller.cpp
)

qt_add_qml_module(nakumi-terminal
    URI NakumiOS.Terminal
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/TerminalView.qml
    SOURCES
        terminalcontroller.h
)

target_link_libraries(nakumi-terminal PRIVATE
    nakumios_common
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

install(TARGETS nakumi-terminal
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Desktop file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/org.nakumios.terminal.desktop.in
    ${CMAKE_BINARY_DIR}/org.nakumios.terminal.desktop
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/org.nakumios.terminal.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
