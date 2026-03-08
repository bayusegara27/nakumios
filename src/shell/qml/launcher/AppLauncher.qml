/**
 * AppLauncher Component
 * 
 * Full-screen application launcher with search and grid view.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Shell 1.0
import "../components"

Item {
    id: root
    
    signal closed()
    
    // Semi-transparent background
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.7)
        
        MouseArea {
            anchors.fill: parent
            onClicked: root.closed()
        }
    }
    
    // Main launcher content
    BlurPanel {
        id: launcherPanel
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 900)
        height: Math.min(parent.height * 0.8, 700)
        
        // Enter animation
        opacity: root.visible ? 1.0 : 0.0
        scale: root.visible ? 1.0 : 0.95
        
        Behavior on opacity {
            NumberAnimation { duration: DesignTokens.animationDurationNormal }
        }
        
        Behavior on scale {
            NumberAnimation { 
                duration: DesignTokens.animationDurationNormal 
                easing.type: Easing.OutQuad
            }
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            // Search bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                radius: DesignTokens.borderRadius
                color: DesignTokens.surfaceBase
                border.width: searchField.activeFocus ? 2 : 1
                border.color: searchField.activeFocus ? 
                              DesignTokens.accentPrimary : DesignTokens.borderDefault
                
                Behavior on border.color {
                    ColorAnimation { duration: DesignTokens.animationDurationFast }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: DesignTokens.spacingSmall
                    spacing: DesignTokens.spacingSmall
                    
                    // Search icon
                    Text {
                        text: "🔍"
                        font.pixelSize: 18
                        color: DesignTokens.textSecondary
                    }
                    
                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        
                        placeholderText: "Search applications..."
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textPrimary
                        placeholderTextColor: DesignTokens.textSecondary
                        
                        background: Item {}
                        
                        onTextChanged: {
                            ShellController.searchApplications(text)
                        }
                        
                        Keys.onEscapePressed: {
                            if (text !== "") {
                                text = ""
                            } else {
                                root.closed()
                            }
                        }
                        
                        Component.onCompleted: {
                            forceActiveFocus()
                        }
                    }
                    
                    // Clear button
                    IconButton {
                        visible: searchField.text !== ""
                        iconName: "✕"
                        iconSize: 16
                        onClicked: searchField.text = ""
                    }
                }
            }
            
            // Application grid
            AppGrid {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                model: ShellController.applications
                
                onAppClicked: (desktopFile) => {
                    ShellController.launchApplication(desktopFile)
                }
                
                onAppRightClicked: (desktopFile, appName) => {
                    appContextMenu.desktopFile = desktopFile
                    appContextMenu.appName = appName
                    appContextMenu.popup()
                }
            }
            
            // Categories bar (optional)
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: DesignTokens.spacingSmall
                
                Repeater {
                    model: ["All", "Favorites", "Development", "Graphics", "Internet", "Office", "System"]
                    
                    delegate: AbstractButton {
                        width: categoryText.width + DesignTokens.spacingMedium
                        height: 32
                        
                        background: Rectangle {
                            radius: height / 2
                            color: hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            border.width: index === 0 ? 1 : 0
                            border.color: DesignTokens.accentPrimary
                        }
                        
                        Text {
                            id: categoryText
                            anchors.centerIn: parent
                            text: modelData
                            font.family: DesignTokens.fontFamily
                            font.pixelSize: DesignTokens.fontSizeSmall
                            color: index === 0 ? DesignTokens.accentPrimary : DesignTokens.textSecondary
                        }
                    }
                }
            }
        }
    }
    
    // Context menu for apps
    Menu {
        id: appContextMenu
        
        property string desktopFile: ""
        property string appName: ""
        
        background: BlurPanel {
            implicitWidth: 180
        }
        
        MenuItem {
            text: "Launch"
            onClicked: ShellController.launchApplication(appContextMenu.desktopFile)
        }
        MenuItem {
            text: "Pin to Dock"
            onClicked: ShellController.pinToDock(appContextMenu.desktopFile)
        }
    }
    
    // Focus on search when shown
    onVisibleChanged: {
        if (visible) {
            searchField.text = ""
            searchField.forceActiveFocus()
        }
    }
}
