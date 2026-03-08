/**
 * WindowButton Component
 * 
 * Window control button (close, minimize, maximize) with 
 * colored backgrounds matching system theme.
 */

import QtQuick
import QtQuick.Controls
import NakumiOS 1.0

AbstractButton {
    id: root
    
    enum ButtonType {
        Close,
        Minimize,
        Maximize
    }
    
    property int buttonType: WindowButton.Close
    property int size: 14
    
    implicitWidth: size
    implicitHeight: size
    
    background: Rectangle {
        radius: width / 2
        color: {
            switch (root.buttonType) {
                case WindowButton.Close:
                    return root.pressed ? "#cc4444" : 
                           root.hovered ? "#ff5555" : "#ff5555"
                case WindowButton.Minimize:
                    return root.pressed ? "#cc9900" :
                           root.hovered ? "#ffbb00" : "#ffbb00"
                case WindowButton.Maximize:
                    return root.pressed ? "#009933" :
                           root.hovered ? "#00cc44" : "#00cc44"
                default:
                    return DesignTokens.surfaceBase
            }
        }
        
        opacity: root.hovered ? 1.0 : 0.7
        
        Behavior on color {
            ColorAnimation { duration: DesignTokens.animationDurationFast }
        }
        
        Behavior on opacity {
            NumberAnimation { duration: DesignTokens.animationDurationFast }
        }
    }
    
    contentItem: Item {
        // Icon drawn inside the button
        Canvas {
            anchors.centerIn: parent
            width: parent.width * 0.6
            height: parent.height * 0.6
            visible: root.hovered
            
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = "white"
                ctx.lineWidth = 1.5
                ctx.lineCap = "round"
                
                switch (root.buttonType) {
                    case WindowButton.Close:
                        ctx.beginPath()
                        ctx.moveTo(0, 0)
                        ctx.lineTo(width, height)
                        ctx.moveTo(width, 0)
                        ctx.lineTo(0, height)
                        ctx.stroke()
                        break
                    case WindowButton.Minimize:
                        ctx.beginPath()
                        ctx.moveTo(0, height / 2)
                        ctx.lineTo(width, height / 2)
                        ctx.stroke()
                        break
                    case WindowButton.Maximize:
                        ctx.beginPath()
                        ctx.rect(0, 0, width, height)
                        ctx.stroke()
                        break
                }
            }
        }
    }
    
    scale: pressed ? 0.9 : 1.0
    
    Behavior on scale {
        NumberAnimation {
            duration: DesignTokens.animationDurationFast
            easing.type: Easing.OutQuad
        }
    }
}
