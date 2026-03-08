/**
 * About Settings Page
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import ".."

SettingsPage {
    title: "About"
    subtitle: "System information and details"
    
    // System info section
    Rectangle {
        Layout.fillWidth: true
        height: infoColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: infoColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            // Logo and name
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: DesignTokens.spacingMedium
                
                Rectangle {
                    width: 80
                    height: 80
                    radius: DesignTokens.borderRadius
                    color: DesignTokens.accentPrimary
                    
                    Text {
                        anchors.centerIn: parent
                        text: "N"
                        font.pixelSize: 48
                        font.bold: true
                        color: "white"
                    }
                }
                
                ColumnLayout {
                    spacing: DesignTokens.spacingSmall / 2
                    
                    Text {
                        text: "NakumiOS"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        color: DesignTokens.textPrimary
                    }
                    
                    Text {
                        text: "Version " + Qt.application.version
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textSecondary
                    }
                }
            }
            
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: DesignTokens.borderDefault
            }
            
            // System details
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: DesignTokens.spacingSmall
                columnSpacing: DesignTokens.spacingLarge
                
                Text { text: "Hostname:"; color: DesignTokens.textSecondary; font.family: DesignTokens.fontFamily }
                Text { text: SystemInfo.hostname; color: DesignTokens.textPrimary; font.family: DesignTokens.fontFamily }
                
                Text { text: "Kernel:"; color: DesignTokens.textSecondary; font.family: DesignTokens.fontFamily }
                Text { text: SystemInfo.kernelVersion; color: DesignTokens.textPrimary; font.family: DesignTokens.fontFamily }
                
                Text { text: "Uptime:"; color: DesignTokens.textSecondary; font.family: DesignTokens.fontFamily }
                Text { text: SystemInfo.formatUptime(SystemInfo.uptime); color: DesignTokens.textPrimary; font.family: DesignTokens.fontFamily }
            }
        }
    }
    
    // Hardware section
    Rectangle {
        Layout.fillWidth: true
        height: hardwareColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: hardwareColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Hardware"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: DesignTokens.spacingSmall
                columnSpacing: DesignTokens.spacingLarge
                
                Text { text: "Memory:"; color: DesignTokens.textSecondary; font.family: DesignTokens.fontFamily }
                Text { text: SystemInfo.formatBytes(SystemInfo.totalMemory); color: DesignTokens.textPrimary; font.family: DesignTokens.fontFamily }
                
                Text { text: "Disk Space:"; color: DesignTokens.textSecondary; font.family: DesignTokens.fontFamily }
                Text { text: SystemInfo.formatBytes(SystemInfo.availableDisk) + " available of " + SystemInfo.formatBytes(SystemInfo.totalDisk); color: DesignTokens.textPrimary; font.family: DesignTokens.fontFamily }
            }
        }
    }
}
