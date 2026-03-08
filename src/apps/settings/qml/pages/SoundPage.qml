/**
 * Sound Settings Page
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Settings 1.0
import ".."

SettingsPage {
    title: "Sound"
    subtitle: "Configure audio output and input settings"
    
    // Output section
    Rectangle {
        Layout.fillWidth: true
        height: outputColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: outputColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "Output"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            // Volume slider
            ColumnLayout {
                Layout.fillWidth: true
                spacing: DesignTokens.spacingSmall
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: "🔊"
                        font.pixelSize: 20
                    }
                    
                    Text {
                        text: "Master Volume"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textPrimary
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Text {
                        text: SettingsController.masterVolume + "%"
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textSecondary
                    }
                }
                
                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    stepSize: 1
                    value: SettingsController.masterVolume
                    onMoved: SettingsController.masterVolume = value
                }
            }
            
            // Mute toggle
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Mute Audio"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textPrimary
                }
                
                Item { Layout.fillWidth: true }
                
                Switch {
                    checked: SettingsController.muteAudio
                    onToggled: SettingsController.muteAudio = checked
                }
            }
        }
    }
    
    // Sound effects section
    Rectangle {
        Layout.fillWidth: true
        height: effectsColumn.height + DesignTokens.spacingLarge * 2
        color: DesignTokens.surfaceBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        ColumnLayout {
            id: effectsColumn
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingLarge
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: "System Sounds"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                font.weight: Font.Medium
                color: DesignTokens.textPrimary
            }
            
            Text {
                text: "Sound effects for system events and notifications"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeSmall
                color: DesignTokens.textSecondary
            }
            
            // Placeholder for future sound effect settings
            Text {
                text: "Sound effect settings coming soon..."
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeNormal
                font.italic: true
                color: DesignTokens.textDisabled
            }
        }
    }
}
