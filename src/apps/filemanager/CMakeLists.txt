# NakumiOS File Manager Application

qt_add_executable(nakumi-files
    main.cpp
    filemanagercontroller.cpp
)

qt_add_qml_module(nakumi-files
    URI NakumiOS.Files
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/FileItem.qml
        qml/Sidebar.qml
        qml/PathBar.qml
    SOURCES
        filemanagercontroller.h
)

target_link_libraries(nakumi-files PRIVATE
    nakumios_common
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
)

install(TARGETS nakumi-files
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Desktop file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/org.nakumios.files.desktop.in
    ${CMAKE_BINARY_DIR}/org.nakumios.files.desktop
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/org.nakumios.files.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
