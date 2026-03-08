import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import NakumiOS.Launcher 1.0

/*
 * NakumiOS Launcher - Fullscreen application menu with blurred background
 *
 * Design Tokens:
 *   Background: #1A1A24
 *   Accent: #6C5CE7
 *   Text: #E2E8F0
 *   Border Radius: 12px
 */

Window {
    id: launcherWindow
    visibility: Window.FullScreen
    color: "#CC1A1A24"
    flags: Qt.FramelessWindowHint

    AppModel { id: appModel }

    AppFilterModel {
        id: filterModel
        sourceModel: appModel
    }

    /* Close on Escape */
    Shortcut {
        sequence: "Escape"
        onActivated: Qt.quit()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: Qt.quit()
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.7, 900)
        spacing: 24

        /* Search bar */
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            radius: 12
            color: "#2D2D3F"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Text {
                    text: "🔍"
                    color: "#A0A0B0"
                    font.pixelSize: 16
                }

                TextInput {
                    id: searchInput
                    Layout.fillWidth: true
                    color: "#E2E8F0"
                    font.pixelSize: 16
                    clip: true

                    onTextChanged: filterModel.filterText = text

                    Text {
                        anchors.fill: parent
                        text: "Search applications..."
                        color: "#606080"
                        font.pixelSize: 16
                        visible: !searchInput.text
                    }
                }
            }
        }

        /* Application grid */
        GridView {
            id: appGrid
            Layout.fillWidth: true
            Layout.preferredHeight: launcherWindow.height * 0.6
            cellWidth: 140
            cellHeight: 140
            clip: true
            model: filterModel

            delegate: Item {
                width: 140
                height: 140

                Rectangle {
                    anchors.centerIn: parent
                    width: 120
                    height: 120
                    radius: 12
                    color: delegateMa.containsMouse ? "#6C5CE7" : "transparent"

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        /* App icon placeholder */
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 48
                            height: 48
                            radius: 12
                            color: "#3D3D5C"

                            Text {
                                anchors.centerIn: parent
                                text: model.name ? model.name.charAt(0).toUpperCase() : "?"
                                color: "#6C5CE7"
                                font.pixelSize: 20
                                font.bold: true
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: model.name || ""
                            color: "#E2E8F0"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            width: 100
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    MouseArea {
                        id: delegateMa
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            appModel.launch(filterModel.mapToSource(
                                filterModel.index(index, 0)).row)
                            Qt.quit()
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        searchInput.forceActiveFocus()
    }
}
