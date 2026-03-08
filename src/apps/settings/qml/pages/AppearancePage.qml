/**
 * Appearance Settings Page
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Settings 1.0
import ".."

SettingsPage {
    title: "Appearance"
    subtitle: "Customize the look and feel of NakumiOS"
    
    // Theme section
    Rectangle {
        Layout.fillWidth: true
        height: themeColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: themeColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Theme"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            // Dark mode toggle
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Dark Mode"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
                
                Item { Layout.fillWidth: true }
                
                Switch {
                    checked: SettingsController.darkMode
                    onToggled: SettingsController.darkMode = checked
                }
            }
            
            // Accent color
            ColumnLayout {
                Layout.fillWidth: true
                spacing: DesignTokens.spacingSmall
                
                Text {
                    text: "Accent Color"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
                
                Row {
                    spacing: DesignTokens.spacingSmall
                    
                    Repeater {
                        model: [
                            "#6C5CE7", // Purple (default)
                            "#0984E3", // Blue
                            "#00B894", // Green
                            "#FDCB6E", // Yellow
                            "#E17055", // Orange
                            "#D63031", // Red
                            "#E84393"  // Pink
                        ]
                        
                        delegate: Rectangle {
                            width: 36
                            height: 36
                            radius: 18
                            color: modelData
                            border.width: SettingsController.accentColor === modelData ? 3 : 0
                            border.color: "white"
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: SettingsController.accentColor = modelData
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Interface section
    Rectangle {
        Layout.fillWidth: true
        height: interfaceColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: interfaceColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Interface"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            // Interface scale
            ColumnLayout {
                Layout.fillWidth: true
                spacing: DesignTokens.spacingSmall
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "Interface Scale"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textPrimary
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Text {
                        text: Math.round(SettingsController.interfaceScale * 100) + "%"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textSecondary
                    }
                }
                
                Slider {
                    Layout.fillWidth: true
                    from: 0.75
                    to: 2.0
                    stepSize: 0.25
                    value: SettingsController.interfaceScale
                    onMoved: SettingsController.interfaceScale = value
                }
            }
        }
    }
}
