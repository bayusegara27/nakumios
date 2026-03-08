/**
 * NakumiOS Terminal Main Window
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Terminal 1.0

ApplicationWindow {
    id: root
    
    visible: true
    width: 800
    height: 500
    minimumWidth: 400
    minimumHeight: 300
    
    title: "Terminal"
    color: DesignTokens.backgroundBase
    
    TerminalController {
        id: terminal
    }
    
    TerminalView {
        anchors.fill: parent
        anchors.margins: DesignTokens.spacingSmall
        controller: terminal
    }
    
    Component.onCompleted: {
        terminal.start()
    }
    
    Component.onDestruction: {
        terminal.stop()
    }
    
    // Keyboard shortcuts
    Shortcut {
        sequence: "Ctrl+Shift+C"
        onActivated: terminal.copy()
    }
    
    Shortcut {
        sequence: "Ctrl+Shift+V"
        onActivated: terminal.paste()
    }
    
    Shortcut {
        sequence: "Ctrl+Shift+N"
        onActivated: {
            // Open new terminal window
        }
    }
}
