/**
 * File Item Component
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Files 1.0

Item {
    id: root
    
    property var fileData
    property int viewMode: 0 // 0 = icon, 1 = list
    
    // Icon view
    Column {
        visible: root.viewMode === 0
        anchors.centerIn: parent
        spacing: DesignTokens.spacingSmall / 2
        
        Rectangle {
            width: 56
            height: 56
            radius: DesignTokens.borderRadius
            color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
            anchors.horizontalCenter: parent.horizontalCenter
            
            Text {
                anchors.centerIn: parent
                text: fileData ? fileData.icon : ""
                font.pixelSize: 32
            }
        }
        
        Text {
            width: root.width - DesignTokens.spacingSmall * 2
            text: fileData ? fileData.name : ""
            font.family: DesignTokens.fontFamily
            font.pixelSize: DesignTokens.fontSizeSmall
            color: DesignTokens.textPrimary
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            maximumLineCount: 2
            wrapMode: Text.WrapAnywhere
        }
    }
    
    // List view
    Rectangle {
        visible: root.viewMode === 1
        anchors.fill: parent
        color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: DesignTokens.spacingMedium
            anchors.rightMargin: DesignTokens.spacingMedium
            spacing: DesignTokens.spacingMedium
            
            Text {
                text: fileData ? fileData.icon : ""
                font.pixelSize: 20
            }
            
            Text {
                Layout.fillWidth: true
                text: fileData ? fileData.name : ""
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeNormal
                color: DesignTokens.textPrimary
                elide: Text.ElideRight
            }
            
            Text {
                Layout.preferredWidth: 100
                text: fileData ? fileData.sizeFormatted : ""
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeSmall
                color: DesignTokens.textSecondary
            }
            
            Text {
                Layout.preferredWidth: 150
                text: fileData ? fileData.modified : ""
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeSmall
                color: DesignTokens.textSecondary
            }
        }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        onDoubleClicked: {
            if (fileData) {
                FileManager.openItem(fileData.path)
            }
        }
        
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton && fileData) {
                itemContextMenu.popup()
            }
        }
    }
    
    Menu {
        id: itemContextMenu
        
        background: Rectangle {
            implicitWidth: 180
            color: DesignTokens.surfaceBase
            radius: DesignTokens.borderRadius
            border.width: 1
            border.color: DesignTokens.borderDefault
        }
        
        MenuItem {
            text: "Open"
            onClicked: FileManager.openItem(fileData.path)
        }
        MenuSeparator {}
        MenuItem {
            text: "Copy"
            onClicked: FileManager.copyItem(fileData.path)
        }
        MenuItem {
            text: "Cut"
            onClicked: FileManager.cutItem(fileData.path)
        }
        MenuSeparator {}
        MenuItem {
            text: "Rename"
            onClicked: renameDialog.open()
        }
        MenuItem {
            text: "Delete"
            onClicked: deleteDialog.open()
        }
    }
    
    Dialog {
        id: renameDialog
        title: "Rename"
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        TextField {
            id: renameField
            text: fileData ? fileData.name : ""
            width: 300
        }
        
        onAccepted: {
            if (renameField.text.trim() !== "" && fileData) {
                FileManager.renameItem(fileData.path, renameField.text.trim())
            }
        }
    }
    
    Dialog {
        id: deleteDialog
        title: "Delete"
        standardButtons: Dialog.Yes | Dialog.No
        
        Text {
            text: "Are you sure you want to delete \"" + (fileData ? fileData.name : "") + "\"?"
            color: DesignTokens.textPrimary
        }
        
        onAccepted: {
            if (fileData) {
                FileManager.deleteItem(fileData.path)
            }
        }
    }
}
