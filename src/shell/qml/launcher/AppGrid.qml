/**
 * AppGrid Component
 * 
 * Grid view of application icons for the launcher.
 */

import QtQuick
import QtQuick.Controls
import NakumiOS 1.0

Item {
    id: root
    
    property var model: []
    
    signal appClicked(string desktopFile)
    signal appRightClicked(string desktopFile, string appName)
    
    ScrollView {
        anchors.fill: parent
        clip: true
        
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        
        GridView {
            id: gridView
            anchors.fill: parent
            
            cellWidth: 120
            cellHeight: 120
            
            model: root.model
            
            delegate: AppIcon {
                width: gridView.cellWidth
                height: gridView.cellHeight
                
                appName: modelData.name || ""
                iconSource: modelData.icon ? "image://icons/" + modelData.icon : ""
                desktopFile: modelData.desktopFile || ""
                
                onClicked: root.appClicked(desktopFile)
                onRightClicked: root.appRightClicked(desktopFile, appName)
            }
            
            // Empty state
            Text {
                anchors.centerIn: parent
                visible: root.model.length === 0
                text: "No applications found"
                font.family: DesignTokens.fontFamily
                font.pixelSize: DesignTokens.fontSizeLarge
                color: DesignTokens.textSecondary
            }
        }
    }
}
