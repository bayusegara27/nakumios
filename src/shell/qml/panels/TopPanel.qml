/**
 * TopPanel Component
 * 
 * The main top panel containing activities button, 
 * system tray, and clock.
 */

import QtQuick
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Shell 1.0
import "../components"

BlurPanel {
    id: root
    
    signal launcherClicked()
    
    // Extend slightly for visual effect
    anchors.margins: 0
    radius: 0
    showBorder: false
    
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: DesignTokens.borderDefault
        opacity: 0.5
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: DesignTokens.spacingMedium
        anchors.rightMargin: DesignTokens.spacingMedium
        spacing: DesignTokens.spacingSmall
        
        // Activities / Launcher button
        Item {
            Layout.preferredWidth: activitiesRow.width + DesignTokens.spacingMedium
            Layout.fillHeight: true
            
            Rectangle {
                anchors.fill: parent
                anchors.margins: DesignTokens.spacingSmall / 2
                radius: DesignTokens.borderRadius / 2
                color: activitiesArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                
                Behavior on color {
                    ColorAnimation { duration: DesignTokens.animationDurationFast }
                }
            }
            
            Row {
                id: activitiesRow
                anchors.centerIn: parent
                spacing: DesignTokens.spacingSmall
                
                // NakumiOS logo/icon
                Rectangle {
                    width: 18
                    height: 18
                    radius: 4
                    color: DesignTokens.accentPrimary
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Text {
                        anchors.centerIn: parent
                        text: "N"
                        font.pixelSize: 12
                        font.bold: true
                        color: "white"
                    }
                }
                
                Text {
                    text: "Activities"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    font.weight: Font.Medium
                    color: DesignTokens.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            
            MouseArea {
                id: activitiesArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.launcherClicked()
            }
        }
        
        // Spacer
        Item {
            Layout.fillWidth: true
        }
        
        // Center: Clock
        Clock {
            id: clock
            showDate: true
        }
        
        // Spacer
        Item {
            Layout.fillWidth: true
        }
        
        // System tray area
        Row {
            Layout.alignment: Qt.AlignRight
            spacing: DesignTokens.spacingSmall / 2
            
            // CPU usage indicator
            SystemTrayItem {
                iconText: "⚡"
                label: Math.round(SystemInfo.cpuUsage) + "%"
                showLabel: true
            }
            
            // Memory usage indicator
            SystemTrayItem {
                iconText: "📊"
                label: SystemInfo.formatBytes(SystemInfo.usedMemory)
                showLabel: true
            }
            
            // Battery indicator (if available)
            SystemTrayItem {
                visible: SystemInfo.hasBattery
                iconText: SystemInfo.batteryCharging ? "🔌" : "🔋"
                label: SystemInfo.batteryPercent + "%"
                showLabel: true
            }
            
            // Power menu
            IconButton {
                iconName: "⏻"
                text: "Power"
                onClicked: powerMenu.open()
                
                Menu {
                    id: powerMenu
                    y: parent.height
                    
                    background: BlurPanel {
                        implicitWidth: 200
                        implicitHeight: contentItem.implicitHeight
                    }
                    
                    MenuItem {
                        text: "Lock Screen"
                        onClicked: ShellController.lockScreen()
                    }
                    MenuItem {
                        text: "Suspend"
                        onClicked: ShellController.suspend()
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: "Log Out"
                        onClicked: ShellController.logout()
                    }
                    MenuItem {
                        text: "Restart"
                        onClicked: ShellController.reboot()
                    }
                    MenuItem {
                        text: "Shut Down"
                        onClicked: ShellController.shutdown()
                    }
                }
            }
        }
    }
}
