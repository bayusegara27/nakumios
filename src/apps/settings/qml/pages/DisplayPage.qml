/**
 * Display Settings Page
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Settings 1.0
import ".."

SettingsPage {
    title: "Display"
    subtitle: "Configure display and screen settings"
    
    // Resolution section
    Rectangle {
        Layout.fillWidth: true
        height: displayColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: displayColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Resolution"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Screen Resolution"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
                
                Item { Layout.fillWidth: true }
                
                ComboBox {
                    model: SettingsController.getAvailableResolutions()
                    currentIndex: model.indexOf(SettingsController.resolution)
                    onActivated: SettingsController.resolution = currentText
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        color: DesignTokens.surfaceElevated
                        radius: DesignTokens.borderRadius / 2
                        border.width: 1
                        border.color: DesignTokens.borderDefault
                    }
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Refresh Rate"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
                
                Item { Layout.fillWidth: true }
                
                ComboBox {
                    model: SettingsController.getAvailableRefreshRates()
                    currentIndex: model.indexOf(SettingsController.refreshRate)
                    onActivated: SettingsController.refreshRate = model[currentIndex]
                    displayText: currentText + " Hz"
                    
                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 40
                        color: DesignTokens.surfaceElevated
                        radius: DesignTokens.borderRadius / 2
                        border.width: 1
                        border.color: DesignTokens.borderDefault
                    }
                }
            }
        }
    }
    
    // Brightness section
    Rectangle {
        Layout.fillWidth: true
        height: brightnessColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: brightnessColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Brightness & Color"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            ColumnLayout {
                Layout.fillWidth: true
                spacing: DesignTokens.spacingSmall
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Brightness"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textPrimary
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Text {
                        text: Math.round(SettingsController.brightness * 100) + "%"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textSecondary
                    }
                }
                
                Slider {
                    Layout.fillWidth: true
                    from: 0.1
                    to: 1.0
                    value: SettingsController.brightness
                    onMoved: SettingsController.brightness = value
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Night Light"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
                
                Item { Layout.fillWidth: true }
                
                Switch {
                    checked: SettingsController.nightLight
                    onToggled: SettingsController.nightLight = checked
                }
            }
        }
    }
}
