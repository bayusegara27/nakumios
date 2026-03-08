/**
 * Sidebar Component
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Files 1.0

Rectangle {
    id: root
    
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
        
        Text {
            text: "Places"
            font.family: DesignTokens.fontFamily
            font.pixelSize: DesignTokens.fontSizeSmall
            font.weight: Font.Medium
            color: DesignTokens.textSecondary
            Layout.leftMargin: DesignTokens.spacingSmall
        }
        
        Repeater {
            model: FileManager.bookmarks
            
            delegate: AbstractButton {
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                
                property bool isActive: FileManager.currentPath === modelData.path
                
                background: Rectangle {
                    radius: DesignTokens.borderRadius / 2
                    color: isActive ? DesignTokens.accentPrimary :
                           hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                    
                    Behavior on color {
                        ColorAnimation { duration: DesignTokens.animationDurationFast }
                    }
                }
                
                contentItem: Row {
                    spacing: DesignTokens.spacingSmall
                    leftPadding: DesignTokens.spacingSmall
                    
                    Text {
                        text: modelData.icon
                        font.pixelSize: 16
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    Text {
                        text: modelData.name
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: isActive ? DesignTokens.textOnAccent : DesignTokens.textPrimary
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                    }
                }
                
                onClicked: {
                    FileManager.currentPath = modelData.path
                }
            }
        }
        
        Item { Layout.fillHeight: true }
        
        // Add bookmark button
        AbstractButton {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            
            background: Rectangle {
                radius: DesignTokens.borderRadius / 2
                color: hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                border.width: 1
                border.color: DesignTokens.borderDefault
            }
            
            contentItem: Row {
                spacing: DesignTokens.spacingSmall
                leftPadding: DesignTokens.spacingSmall
                
                Text {
                    text: "+"
                    font.pixelSize: 16
                    color: DesignTokens.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Text {
                    text: "Add Bookmark"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeSmall
                    color: DesignTokens.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            
            onClicked: {
                FileManager.addBookmark(FileManager.currentPath)
            }
        }
    }
}
