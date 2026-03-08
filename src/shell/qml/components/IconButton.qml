/**
 * IconButton Component
 * 
 * A clickable icon button with hover and press animations.
 */

import QtQuick
import QtQuick.Controls
import NakumiOS 1.0

AbstractButton {
    id: root
    
    property string iconSource: ""
    property string iconName: ""
    property int iconSize: DesignTokens.iconSize
    property color iconColor: DesignTokens.textPrimary
    property color hoverColor: Qt.rgba(1, 1, 1, 0.1)
    property color pressColor: Qt.rgba(1, 1, 1, 0.15)
    property alias radius: background.radius
    
    implicitWidth: iconSize + DesignTokens.spacingMedium
    implicitHeight: iconSize + DesignTokens.spacingMedium
    
    background: Rectangle {
        id: background
        radius: DesignTokens.borderRadius / 2
        color: root.pressed ? root.pressColor : 
               root.hovered ? root.hoverColor : "transparent"
        
        Behavior on color {
            ColorAnimation {
                duration: DesignTokens.animationDurationFast
            }
        }
    }
    
    contentItem: Item {
        // Use Image for external icons, Text with icon font as fallback
        Image {
            id: iconImage
            anchors.centerIn: parent
            width: root.iconSize
            height: root.iconSize
            source: root.iconSource
            visible: root.iconSource !== ""
            fillMode: Image.PreserveAspectFit
            
            // Apply color tint (works with SVG)
            layer.enabled: true
            layer.effect: Item {
                property color color: root.iconColor
            }
        }
        
        Text {
            id: iconText
            anchors.centerIn: parent
            text: root.iconName
            font.pixelSize: root.iconSize
            color: root.iconColor
            visible: root.iconSource === "" && root.iconName !== ""
        }
    }
    
    // Scale animation on press
    scale: pressed ? 0.92 : 1.0
    
    Behavior on scale {
        NumberAnimation {
            duration: DesignTokens.animationDurationFast
            easing.type: Easing.OutQuad
        }
    }
    
    // Tooltip
    ToolTip.visible: hovered && text !== ""
    ToolTip.text: text
    ToolTip.delay: 500
}
