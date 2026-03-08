/**
 * BlurPanel Component
 * 
 * A semi-transparent panel with background blur effect
 * following NakumiOS design tokens.
 */

import QtQuick
import QtQuick.Effects
import NakumiOS 1.0

Rectangle {
    id: root
    
    property real blurRadius: DesignTokens.blurRadius
    property bool showBorder: true
    property color borderColor: DesignTokens.borderDefault
    
    color: Qt.rgba(
        DesignTokens.surfaceBase.r,
        DesignTokens.surfaceBase.g,
        DesignTokens.surfaceBase.b,
        DesignTokens.panelOpacity
    )
    radius: DesignTokens.borderRadius
    
    border.width: showBorder ? DesignTokens.borderWidth : 0
    border.color: borderColor
    
    // Background blur effect
    layer.enabled: true
    layer.effect: MultiEffect {
        source: root
        blurEnabled: true
        blur: root.blurRadius / 64.0
        blurMax: 64
        saturation: 0.1
    }
    
    // Inner shadow for depth
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.05) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
}
