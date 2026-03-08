/**
 * SystemTrayItem Component
 * 
 * Individual system tray indicator (battery, wifi, etc.)
 */

import QtQuick
import QtQuick.Controls
import NakumiOS 1.0

Item {
    id: root
    
    property string iconSource: ""
    property string iconText: ""
    property string label: ""
    property bool showLabel: true
    property alias hovered: mouseArea.containsMouse
    
    signal clicked()
    
    implicitWidth: row.width + DesignTokens.spacingSmall * 2
    implicitHeight: DesignTokens.panelHeight - DesignTokens.spacingSmall * 2
    
    Rectangle {
        anchors.fill: parent
        radius: DesignTokens.borderRadius / 2
        color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
        
        Behavior on color {
            ColorAnimation { duration: DesignTokens.animationDurationFast }
        }
    }
    
    Row {
        id: row
        anchors.centerIn: parent
        spacing: DesignTokens.spacingSmall / 2
        
        Image {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            width: 16
            height: 16
            source: root.iconSource
            visible: root.iconSource !== ""
            fillMode: Image.PreserveAspectFit
        }
        
        Text {
            id: iconTextItem
            anchors.verticalCenter: parent.verticalCenter
            text: root.iconText
            font.pixelSize: 14
            color: DesignTokens.textPrimary
            visible: root.iconSource === "" && root.iconText !== ""
        }
        
        Text {
            id: labelText
            anchors.verticalCenter: parent.verticalCenter
            text: root.label
            font.pixelSize: DesignTokens.fontSizeSmall
            font.family: DesignTokens.fontFamily
            color: DesignTokens.textPrimary
            visible: root.showLabel && root.label !== ""
        }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
