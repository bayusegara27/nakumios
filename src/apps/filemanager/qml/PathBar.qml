/**
 * Path Bar Component
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Files 1.0

Rectangle {
    id: root
    
    color: DesignTokens.surfaceElevated
    radius: DesignTokens.borderRadius / 2
    border.width: pathInput.activeFocus ? 2 : 1
    border.color: pathInput.activeFocus ? DesignTokens.accentPrimary : DesignTokens.borderDefault
    
    property bool editMode: false
    
    // Path segments display
    Row {
        id: segmentsRow
        visible: !root.editMode
        anchors.fill: parent
        anchors.leftMargin: DesignTokens.spacingSmall
        anchors.rightMargin: DesignTokens.spacingSmall
        spacing: 2
        clip: true
        
        Repeater {
            model: FileManager.currentPath.split("/").filter(s => s !== "")
            
            delegate: Row {
                spacing: 2
                anchors.verticalCenter: parent.verticalCenter
                
                AbstractButton {
                    height: root.height - DesignTokens.spacingSmall
                    width: segmentText.width + DesignTokens.spacingSmall * 2
                    anchors.verticalCenter: parent.verticalCenter
                    
                    background: Rectangle {
                        radius: 4
                        color: hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                    }
                    
                    Text {
                        id: segmentText
                        anchors.centerIn: parent
                        text: modelData
                        font.family: DesignTokens.fontFamily
                        font.pixelSize: DesignTokens.fontSizeNormal
                        color: DesignTokens.textPrimary
                    }
                    
                    onClicked: {
                        // Build path up to this segment
                        var parts = FileManager.currentPath.split("/").filter(s => s !== "")
                        var newPath = "/" + parts.slice(0, index + 1).join("/")
                        FileManager.currentPath = newPath
                    }
                }
                
                Text {
                    text: "/"
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                    visible: index < FileManager.currentPath.split("/").filter(s => s !== "").length - 1
                }
            }
        }
    }
    
    // Path input field
    TextField {
        id: pathInput
        visible: root.editMode
        anchors.fill: parent
        anchors.margins: 2
        
        text: FileManager.currentPath
        font.family: DesignTokens.fontFamily
        font.pixelSize: DesignTokens.fontSizeNormal
        color: DesignTokens.textPrimary
        
        background: Item {}
        
        onAccepted: {
            FileManager.currentPath = text
            root.editMode = false
        }
        
        Keys.onEscapePressed: {
            text = FileManager.currentPath
            root.editMode = false
        }
    }
    
    MouseArea {
        anchors.fill: parent
        visible: !root.editMode
        
        onDoubleClicked: {
            root.editMode = true
            pathInput.forceActiveFocus()
            pathInput.selectAll()
        }
    }
}
