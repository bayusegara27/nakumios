/**
 * Clock Component
 * 
 * Displays current time and date with formatting.
 */

import QtQuick
import NakumiOS 1.0

Item {
    id: root
    
    property bool showDate: false
    property bool use24Hour: true
    
    implicitWidth: clockText.width + DesignTokens.spacingMedium
    implicitHeight: DesignTokens.panelHeight - DesignTokens.spacingSmall * 2
    
    Rectangle {
        anchors.fill: parent
        radius: DesignTokens.borderRadius / 2
        color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
        
        Behavior on color {
            ColorAnimation { duration: DesignTokens.animationDurationFast }
        }
    }
    
    Text {
        id: clockText
        anchors.centerIn: parent
        font.family: DesignTokens.fontFamily
        font.pixelSize: DesignTokens.fontSizeNormal
        font.weight: Font.Medium
        color: DesignTokens.textPrimary
        
        text: {
            var now = new Date()
            var timeStr = root.use24Hour 
                ? Qt.formatTime(now, "hh:mm")
                : Qt.formatTime(now, "h:mm AP")
            
            if (root.showDate) {
                return Qt.formatDate(now, "ddd, MMM d") + "  " + timeStr
            }
            return timeStr
        }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            root.showDate = !root.showDate
        }
    }
    
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            clockText.text = clockText.text // Force re-evaluation
        }
    }
}
