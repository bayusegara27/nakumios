/**
 * Settings Page Base Component
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0

Item {
    id: root
    
    property string title: ""
    property string subtitle: ""
    
    default property alias content: contentColumn.children
    
    ScrollView {
        anchors.fill: parent
        clip: true
        
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        
        ColumnLayout {
            id: contentColumn
            width: root.width
            spacing: DesignTokens.spacingLarge
            
            // Page header
            ColumnLayout {
                Layout.fillWidth: true
                spacing: DesignTokens.spacingSmall
                
                Text {
                    text: root.title
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeTitle
                    font.weight: Font.Bold
                    color: DesignTokens.textPrimary
                    visible: root.title !== ""
                }
                
                Text {
                    text: root.subtitle
                    font.family: DesignTokens.fontFamily
                    font.pixelSize: DesignTokens.fontSizeNormal
                    color: DesignTokens.textSecondary
                    visible: root.subtitle !== ""
                }
            }
        }
    }
}
