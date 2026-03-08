/**
 * AppIcon Component
 * 
 * Individual application icon for the launcher grid.
 */

import QtQuick
import QtQuick.Controls
import NakumiOS 1.0

Item {
    id: root
    
    property string appName: ""
    property string iconSource: ""
    property string desktopFile: ""
    
    signal clicked()
    signal rightClicked()
    
    Column {
        anchors.centerIn: parent
        spacing: DesignTokens.spacingSmall
        
        // Icon container
        Rectangle {
            id: iconContainer
            width: 64
            height: 64
            radius: DesignTokens.borderRadius
            anchors.horizontalCenter: parent.horizontalCenter
            
            color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
            border.width: mouseArea.containsMouse ? 1 : 0
            border.color: DesignTokens.borderDefault
            
            Behavior on color {
                ColorAnimation { duration: DesignTokens.animationDurationFast }
            }
            
            // Application icon
            Image {
                id: iconImage
                anchors.centerIn: parent
                width: 48
                height: 48
                source: root.iconSource
                visible: status === Image.Ready
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
            
            // Fallback icon
            Rectangle {
                anchors.centerIn: parent
                width: 48
                height: 48
                radius: DesignTokens.borderRadius
                color: DesignTokens.accentPrimary
                visible: iconImage.status !== Image.Ready
                
                Text {
                    anchors.centerIn: parent
                    text: root.appName.charAt(0).toUpperCase()
                    font.pixelSize: 24
                    font.bold: true
                    color: "white"
                }
            }
        }
        
        // App name
        Text {
            width: 100
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.appName
            font.family: DesignTokens.fontFamily
            font.pixelSize: DesignTokens.fontSizeSmall
            color: DesignTokens.textPrimary
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            maximumLineCount: 2
            wrapMode: Text.WordWrap
        }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                root.rightClicked()
            } else {
                root.clicked()
            }
        }
    }
    
    // Scale animation
    scale: mouseArea.pressed ? 0.95 : 1.0
    
    Behavior on scale {
        NumberAnimation {
            duration: DesignTokens.animationDurationFast
            easing.type: Easing.OutQuad
        }
    }
    
    // Tooltip for long names
    ToolTip.visible: mouseArea.containsMouse && appName.length > 15
    ToolTip.text: appName
    ToolTip.delay: 1000
}
