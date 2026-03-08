/**
 * Terminal View Component
 */

import QtQuick
import QtQuick.Controls
import NakumiOS 1.0
import NakumiOS.Terminal 1.0

Item {
    id: root
    
    property TerminalController controller
    
    Rectangle {
        anchors.fill: parent
        color: controller ? controller.backgroundColor : DesignTokens.backgroundBase
        radius: DesignTokens.borderRadius
        border.width: 1
        border.color: DesignTokens.borderDefault
        
        clip: true
        
        ScrollView {
            id: scrollView
            anchors.fill: parent
            anchors.margins: DesignTokens.spacingSmall
            
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            
            TextArea {
                id: outputArea
                
                readOnly: false
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                
                font: controller ? controller.terminalFont : Qt.font({ family: "monospace", pointSize: 11 })
                color: controller ? controller.foregroundColor : DesignTokens.textPrimary
                selectionColor: DesignTokens.accentPrimary
                selectedTextColor: DesignTokens.textOnAccent
                
                background: Item {}
                
                text: controller ? controller.output : ""
                
                // Auto-scroll to bottom when new output arrives
                onTextChanged: {
                    cursorPosition = text.length
                }
                
                // Handle keyboard input
                Keys.onPressed: (event) => {
                    if (!controller) return
                    
                    if (event.text.length > 0 && !event.modifiers) {
                        // Regular character input
                        controller.sendInput(event.text)
                        event.accepted = true
                    } else {
                        // Special keys
                        controller.sendKey(event.key, event.modifiers)
                        event.accepted = true
                    }
                }
                
                // Handle enter key specifically
                Keys.onReturnPressed: (event) => {
                    if (controller) {
                        controller.sendKey(Qt.Key_Return, event.modifiers)
                    }
                    event.accepted = true
                }
                
                Keys.onEnterPressed: (event) => {
                    if (controller) {
                        controller.sendKey(Qt.Key_Enter, event.modifiers)
                    }
                    event.accepted = true
                }
            }
        }
        
        // Status bar
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 24
            color: DesignTokens.surfaceBase
            
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: DesignTokens.borderDefault
            }
            
            Row {
                anchors.fill: parent
                anchors.leftMargin: DesignTokens.spacingSmall
                anchors.rightMargin: DesignTokens.spacingSmall
                spacing: DesignTokens.spacingMedium
                
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: controller && controller.running ? DesignTokens.success : DesignTokens.error
                }
                
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: controller && controller.running ? "Running" : "Stopped"
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeSmall
                    color: DesignTokens.textSecondary
                }
            }
        }
    }
    
    // Focus the text area on click
    MouseArea {
        anchors.fill: parent
        onClicked: outputArea.forceActiveFocus()
    }
}
