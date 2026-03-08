/**
 * NakumiOS Shell Main Window
 * 
 * The root component that composes the desktop shell including
 * the top panel, dock, and application launcher.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import NakumiOS 1.0
import NakumiOS.Shell 1.0
import "panels"
import "launcher"
import "components"

Window {
    id: root
    
    visible: true
    visibility: Window.FullScreen
    color: DesignTokens.backgroundBase
    title: "NakumiOS"
    
    // Desktop wallpaper
    Image {
        id: wallpaper
        anchors.fill: parent
        source: "qrc:/wallpapers/default.jpg"
        fillMode: Image.PreserveAspectCrop
        
        // Fallback gradient if no wallpaper
        Rectangle {
            anchors.fill: parent
            visible: wallpaper.status !== Image.Ready
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1a1a2e" }
                GradientStop { position: 0.5; color: "#16213e" }
                GradientStop { position: 1.0; color: "#0f0f14" }
            }
        }
    }
    
    // Top panel
    TopPanel {
        id: topPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: DesignTokens.panelHeight
        z: 100
        
        onLauncherClicked: {
            ShellController.launcherVisible = !ShellController.launcherVisible
        }
    }
    
    // Desktop area (between panel and dock)
    Item {
        id: desktopArea
        anchors {
            left: parent.left
            right: parent.right
            top: topPanel.bottom
            bottom: dock.top
        }
        
        // Desktop icons could go here
        MouseArea {
            anchors.fill: parent
            onClicked: {
                ShellController.launcherVisible = false
            }
        }
    }
    
    // Bottom dock
    Dock {
        id: dock
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: DesignTokens.spacingMedium
        z: 100
    }
    
    // Application launcher overlay
    AppLauncher {
        id: launcher
        anchors.fill: parent
        visible: ShellController.launcherVisible
        z: 200
        
        onClosed: {
            ShellController.launcherVisible = false
        }
    }
    
    // Start system monitoring
    Component.onCompleted: {
        SystemInfo.startMonitoring(2000)
    }
    
    Component.onDestruction: {
        SystemInfo.stopMonitoring()
    }
    
    // Keyboard shortcuts
    Shortcut {
        sequence: "Meta+Space"
        onActivated: {
            ShellController.launcherVisible = !ShellController.launcherVisible
        }
    }
    
    Shortcut {
        sequence: "Escape"
        onActivated: {
            ShellController.launcherVisible = false
        }
    }
}
