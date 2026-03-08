/**
 * NakumiOS Settings Application Main Window
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Settings 1.0
import "pages"

ApplicationWindow {
    id: root
    
    visible: true
    width: 1000
    height: 700
    minimumWidth: 800
    minimumHeight: 600
    
    title: "Settings"
    color: DesignTokens.backgroundBase
    
    // Navigation categories
    property var categories: [
        { name: "Appearance", icon: "🎨", page: "AppearancePage.qml" },
        { name: "Display", icon: "🖥️", page: "DisplayPage.qml" },
        { name: "Sound", icon: "🔊", page: "SoundPage.qml" },
        { name: "Network", icon: "🌐", page: "NetworkPage.qml" },
        { name: "About", icon: "ℹ️", page: "AboutPage.qml" }
    ]
    
    property int currentIndex: 0
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Sidebar
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 250
            color: DesignTokens.surfaceBase
            
            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: DesignTokens.borderDefault
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: DesignTokens.spacingMedium
                spacing: DesignTokens.spacingSmall
                
                // Header
                Text {
                    text: "Settings"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeTitle
                    font.weight: Font.Bold
                    color: DesignTokens.textPrimary
                }
                
                Item { height: DesignTokens.spacingMedium }
                
                // Categories
                Repeater {
                    model: root.categories
                    
                    delegate: AbstractButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        
                        property bool isActive: index === root.currentIndex
                        
                        background: Rectangle {
                            radius: DesignTokens.borderRadius
                            color: isActive ? DesignTokens.accentPrimary :
                                   hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                            
                            Behavior on color {
                                ColorAnimation { duration: DesignTokens.animationDurationFast }
                            }
                        }
                        
                        contentItem: Row {
                            spacing: DesignTokens.spacingMedium
                            leftPadding: DesignTokens.spacingMedium
                            
                            Text {
                                text: modelData.icon
                                font.pixelSize: 18
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            
                            Text {
                                text: modelData.name
                                font.family: DesignTokens.fontFamily
                                font.pixelSize: DesignTokens.fontSizeNormal
                                font.weight: isActive ? Font.Medium : Font.Normal
                                color: isActive ? DesignTokens.textOnAccent : DesignTokens.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                        
                        onClicked: root.currentIndex = index
                    }
                }
                
                Item { Layout.fillHeight: true }
                
                // Reset button
                Button {
                    Layout.fillWidth: true
                    text: "Reset to Defaults"
                    
                    background: Rectangle {
                        radius: DesignTokens.borderRadius
                        color: parent.hovered ? Qt.rgba(1, 0.3, 0.3, 0.3) : "transparent"
                        border.width: 1
                        border.color: DesignTokens.error
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.error
                        horizontalAlignment: Text.AlignHCenter
                    }
                    
                    onClicked: {
                        SettingsController.resetToDefaults()
                    }
                }
            }
        }
        
        // Content area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: DesignTokens.backgroundBase
            
            StackLayout {
                anchors.fill: parent
                anchors.margins: DesignTokens.spacingLarge
                currentIndex: root.currentIndex
                
                AppearancePage {}
                DisplayPage {}
                SoundPage {}
                NetworkPage {}
                AboutPage {}
            }
        }
    }
}
