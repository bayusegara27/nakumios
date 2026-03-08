import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import NakumiOS.Panel 1.0

/*
 * NakumiOS Panel - Bottom dock
 *
 * Design Tokens:
 *   Background: #1A1A24
 *   Accent: #6C5CE7
 *   Text: #E2E8F0
 *   Border Radius: 12px
 */

Window {
    id: panelWindow
    width: Screen.width
    height: 48
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    PanelController { id: controller }
    TaskManager { id: taskModel }
    SystemTray { id: sysTray }

    Rectangle {
        anchors.fill: parent
        color: "#1A1A24"
        opacity: 0.92

        RowLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 8

            /* Start Button */
            Rectangle {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: 12
                color: startMa.containsMouse ? "#6C5CE7" : "#2D2D3F"

                Text {
                    anchors.centerIn: parent
                    text: "◆"
                    color: "#E2E8F0"
                    font.pixelSize: 18
                }

                MouseArea {
                    id: startMa
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: controller.toggleLauncher()
                }
            }

            /* Separator */
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                color: "#3D3D5C"
            }

            /* Taskbar */
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                orientation: ListView.Horizontal
                model: taskModel
                spacing: 4
                clip: true

                delegate: Rectangle {
                    width: 140
                    height: 36
                    radius: 8
                    color: model.active ? "#6C5CE7" : "#2D2D3F"

                    Text {
                        anchors.centerIn: parent
                        text: model.title
                        color: "#E2E8F0"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        width: parent.width - 16
                        horizontalAlignment: Text.AlignHCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: taskModel.activate(index)
                    }
                }
            }

            /* Separator */
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                color: "#3D3D5C"
            }

            /* System Tray Area */
            RowLayout {
                spacing: 12

                /* Network */
                Text {
                    text: sysTray.networkConnected ? "󰤨" : "󰤭"
                    color: "#E2E8F0"
                    font.pixelSize: 16
                    font.family: "Noto Sans"
                }

                /* Volume */
                Text {
                    text: sysTray.volume > 50 ? "󰕾" : (sysTray.volume > 0 ? "󰖀" : "󰝟")
                    color: "#E2E8F0"
                    font.pixelSize: 16
                    font.family: "Noto Sans"
                }

                /* Clock */
                Column {
                    Text {
                        text: controller.currentTime
                        color: "#E2E8F0"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        text: controller.currentDate
                        color: "#A0A0B0"
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Item { Layout.preferredWidth: 8 }
        }
    }
}
