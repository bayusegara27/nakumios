/**
 * Dock Component
 * 
 * The application dock at the bottom of the screen with
 * pinned apps and running applications.
 */

import QtQuick
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Shell 1.0
import "../components"

BlurPanel {
    id: root
    
    implicitWidth: dockContent.width + DesignTokens.spacingMedium * 2
    implicitHeight: 64
    
    Row {
        id: dockContent
        anchors.centerIn: parent
        spacing: DesignTokens.spacingSmall
        
        // Pinned applications
        Repeater {
            model: ShellController.pinnedApps
            
            delegate: DockItem {
                iconSource: modelData.icon ? "image://icons/" + modelData.icon : ""
                iconFallback: modelData.name.charAt(0).toUpperCase()
                appName: modelData.name
                desktopFile: modelData.desktopFile
                
                onClicked: {
                    ShellController.launchApplication(desktopFile)
                }
                
                onRightClicked: {
                    contextMenu.desktopFile = desktopFile
                    contextMenu.appName = appName
                    contextMenu.popup()
                }
            }
        }
        
        // Separator if we have pinned apps
        Rectangle {
            visible: ShellController.pinnedApps.length > 0
            width: 1
            height: 40
            anchors.verticalCenter: parent.verticalCenter
            color: DesignTokens.borderDefault
            opacity: 0.5
        }
        
        // App Launcher button
        DockItem {
            iconFallback: "+"
            appName: "Show Apps"
            accentColor: DesignTokens.accentPrimary
            
            onClicked: {
                ShellController.launcherVisible = !ShellController.launcherVisible
            }
        }
    }
    
    // Context menu for dock items
    Menu {
        id: contextMenu
        
        property string desktopFile: ""
        property string appName: ""
        
        background: BlurPanel {
            implicitWidth: 180
        }
        
        MenuItem {
            text: "Launch " + contextMenu.appName
            onClicked: ShellController.launchApplication(contextMenu.desktopFile)
        }
        MenuSeparator {}
        MenuItem {
            text: "Unpin from Dock"
            onClicked: ShellController.unpinFromDock(contextMenu.desktopFile)
        }
    }
}

// Dock item component
component DockItem: Item {
    id: dockItem
    
    property string iconSource: ""
    property string iconFallback: ""
    property string appName: ""
    property string desktopFile: ""
    property color accentColor: DesignTokens.surfaceElevated
    property bool isRunning: false
    
    signal clicked()
    signal rightClicked()
    
    width: 48
    height: 48
    
    Rectangle {
        id: background
        anchors.fill: parent
        radius: DesignTokens.borderRadius
        color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.15) : 
               dockItem.accentColor
        border.width: 1
        border.color: mouseArea.containsMouse ? DesignTokens.borderFocused : 
                      DesignTokens.borderDefault
        
        Behavior on color {
            ColorAnimation { duration: DesignTokens.animationDurationFast }
        }
        
        Behavior on border.color {
            ColorAnimation { duration: DesignTokens.animationDurationFast }
        }
    }
    
    // Icon or fallback text
    Image {
        id: dockItemIcon
        anchors.centerIn: parent
        width: 28
        height: 28
        source: dockItem.iconSource
        visible: dockItem.iconSource !== "" && status === Image.Ready
        fillMode: Image.PreserveAspectFit
    }
    
    Text {
        anchors.centerIn: parent
        text: dockItem.iconFallback
        font.pixelSize: 20
        font.weight: Font.Bold
        color: DesignTokens.textPrimary
        visible: dockItem.iconSource === "" || dockItemIcon.status !== Image.Ready
    }
    
    // Running indicator dot
    Rectangle {
        visible: dockItem.isRunning
        width: 4
        height: 4
        radius: 2
        color: DesignTokens.accentPrimary
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -8
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                dockItem.rightClicked()
            } else {
                dockItem.clicked()
            }
        }
    }
    
    // Bounce animation on click
    scale: mouseArea.pressed ? 0.9 : 1.0
    
    Behavior on scale {
        NumberAnimation {
            duration: DesignTokens.animationDurationFast
            easing.type: Easing.OutBack
        }
    }
    
    // Tooltip
    ToolTip.visible: mouseArea.containsMouse && appName !== ""
    ToolTip.text: appName
    ToolTip.delay: 500
}
