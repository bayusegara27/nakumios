/**
 * Network Settings Page
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import ".."

SettingsPage {
    title: "Network"
    subtitle: "Manage network connections and settings"
    
    // Wi-Fi section
    Rectangle {
        Layout.fillWidth: true
        height: wifiColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: wifiColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Wi-Fi"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeLarge
                    font.weight: Font.Medium
                    color: DesignTokens.textPrimary
                }
                
                Item { Layout.fillWidth: true }
                
                Switch {
                    id: wifiSwitch
                    checked: true
                }
            }
            
            // Placeholder for network list
            Rectangle {
                Layout.fillWidth: true
                height: 150
                color: DesignTokens.surfaceElevated
                radius: DesignTokens.borderRadius / 2
                
                Text {
                    anchors.centerIn: parent
                    text: wifiSwitch.checked ? "Scanning for networks..." : "Wi-Fi is disabled"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textSecondary
                }
            }
        }
    }
    
    // Ethernet section
    Rectangle {
        Layout.fillWidth: true
        height: ethernetColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: ethernetColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Wired Connection"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: DesignTokens.spacingMedium
                
                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: DesignTokens.success
                }
                
                Text {
                    text: "Connected"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
            }
        }
    }
}
