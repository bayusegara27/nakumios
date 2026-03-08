/**
 * NakumiOS File Manager Main Window
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Files 1.0

ApplicationWindow {
    id: root
    
    visible: true
    width: 1100
    height: 700
    minimumWidth: 800
    minimumHeight: 500
    
    title: FileManager.currentPath + " - Files"
    color: DesignTokens.backgroundBase
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Sidebar
        Sidebar {
            Layout.fillHeight: true
            Layout.preferredWidth: 220
        }
        
        // Main content
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: DesignTokens.backgroundBase
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // Toolbar
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    color: DesignTokens.surfaceBase
                    
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: DesignTokens.borderDefault
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: DesignTokens.spacingMedium
                        anchors.rightMargin: DesignTokens.spacingMedium
                        spacing: DesignTokens.spacingSmall
                        
                        // Navigation buttons
                        ToolButton {
                            text: "←"
                            enabled: FileManager.canGoBack
                            onClicked: FileManager.goBack()
                            
                            background: Rectangle {
                                radius: DesignTokens.borderRadius / 2
                                color: parent.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 20
                                color: parent.enabled ? DesignTokens.textPrimary : DesignTokens.textDisabled
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                        
                        ToolButton {
                            text: "→"
                            enabled: FileManager.canGoForward
                            onClicked: FileManager.goForward()
                            
                            background: Rectangle {
                                radius: DesignTokens.borderRadius / 2
                                color: parent.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 20
                                color: parent.enabled ? DesignTokens.textPrimary : DesignTokens.textDisabled
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                        
                        ToolButton {
                            text: "↑"
                            onClicked: FileManager.goUp()
                            
                            background: Rectangle {
                                radius: DesignTokens.borderRadius / 2
                                color: parent.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 20
                                color: DesignTokens.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                        
                        // Path bar
                        PathBar {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.margins: DesignTokens.spacingSmall
                        }
                        
                        // View mode toggle
                        ToolButton {
                            text: FileManager.viewMode === 0 ? "☷" : "▦"
                            onClicked: FileManager.viewMode = FileManager.viewMode === 0 ? 1 : 0
                            
                            background: Rectangle {
                                radius: DesignTokens.borderRadius / 2
                                color: parent.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 20
                                color: DesignTokens.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                        
                        // Refresh button
                        ToolButton {
                            text: "↻"
                            onClicked: FileManager.refresh()
                            
                            background: Rectangle {
                                radius: DesignTokens.borderRadius / 2
                                color: parent.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 20
                                color: DesignTokens.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
                
                // File view
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    clip: true
                    
                    // Icon view
                    GridView {
                        id: iconView
                        visible: FileManager.viewMode === 0
                        anchors.fill: parent
                        anchors.margins: DesignTokens.spacingMedium
                        
                        cellWidth: 100
                        cellHeight: 100
                        
                        model: FileManager.files
                        
                        delegate: FileItem {
                            width: iconView.cellWidth
                            height: iconView.cellHeight
                            viewMode: 0
                            fileData: modelData
                        }
                    }
                    
                    // List view
                    ListView {
                        id: listView
                        visible: FileManager.viewMode === 1
                        anchors.fill: parent
                        
                        model: FileManager.files
                        
                        header: Rectangle {
                            width: listView.width
                            height: 36
                            color: DesignTokens.surfaceBase
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: DesignTokens.spacingMedium
                                anchors.rightMargin: DesignTokens.spacingMedium
                                spacing: DesignTokens.spacingMedium
                                
                                Text {
                                    Layout.fillWidth: true
                                    text: "Name"
                                    font.family: DesignTokens.fontFamily
                                    font.pixelSize: DesignTokens.fontSizeSmall
                                    font.weight: Font.Medium
                                    color: DesignTokens.textSecondary
                                }
                                
                                Text {
                                    Layout.preferredWidth: 100
                                    text: "Size"
                                    font.family: DesignTokens.fontFamily
                                    font.pixelSize: DesignTokens.fontSizeSmall
                                    font.weight: Font.Medium
                                    color: DesignTokens.textSecondary
                                }
                                
                                Text {
                                    Layout.preferredWidth: 150
                                    text: "Modified"
                                    font.family: DesignTokens.fontFamily
                                    font.pixelSize: DesignTokens.fontSizeSmall
                                    font.weight: Font.Medium
                                    color: DesignTokens.textSecondary
                                }
                            }
                        }
                        
                        delegate: FileItem {
                            width: listView.width
                            height: 40
                            viewMode: 1
                            fileData: modelData
                        }
                    }
                }
                
                // Status bar
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    color: DesignTokens.surfaceBase
                    
                    Rectangle {
                        anchors.top: parent.top
                        width: parent.width
                        height: 1
                        color: DesignTokens.borderDefault
                    }
                    
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: DesignTokens.spacingMedium
                        anchors.verticalCenter: parent.verticalCenter
                        text: FileManager.files.length + " items"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeSmall
                        color: DesignTokens.textSecondary
                    }
                }
            }
        }
    }
    
    // Context menu
    Menu {
        id: contextMenu
        
        background: Rectangle {
            implicitWidth: 200
            color: DesignTokens.surfaceBase
            radius: DesignTokens.borderRadius
            border.width: 1
            border.color: DesignTokens.borderDefault
        }
        
        MenuItem {
            text: "New Folder"
            onClicked: newFolderDialog.open()
        }
        MenuItem {
            text: "New File"
            onClicked: newFileDialog.open()
        }
        MenuSeparator {}
        MenuItem {
            text: "Paste"
            onClicked: FileManager.paste()
        }
        MenuSeparator {}
        MenuItem {
            text: "Show Hidden Files"
            checkable: true
            checked: FileManager.showHiddenFiles
            onToggled: FileManager.showHiddenFiles = checked
        }
    }
    
    // New folder dialog
    Dialog {
        id: newFolderDialog
        title: "Create New Folder"
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: parent
        
        TextField {
            id: newFolderName
            placeholderText: "Folder name"
            width: 300
        }
        
        onAccepted: {
            if (newFolderName.text.trim() !== "") {
                FileManager.createFolder(newFolderName.text.trim())
            }
            newFolderName.text = ""
        }
    }
    
    // New file dialog
    Dialog {
        id: newFileDialog
        title: "Create New File"
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: parent
        
        TextField {
            id: newFileName
            placeholderText: "File name"
            width: 300
        }
        
        onAccepted: {
            if (newFileName.text.trim() !== "") {
                FileManager.createFile(newFileName.text.trim())
            }
            newFileName.text = ""
        }
    }
}
