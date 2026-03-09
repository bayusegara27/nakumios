import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import NakumiOS.Files 1.0

ApplicationWindow {
    id: filesWindow
    width: 900
    height: 600
    title: "NakumiFiles - " + fileModel.currentPath
    color: "#1A1A24"

    FileManager { id: fileModel }
    MountManager { id: mountMgr }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        /* Toolbar */
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#12121A"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 8

                Button {
                    text: "←"
                    enabled: fileModel.canGoBack
                    onClicked: fileModel.goBack()
                    palette.button: "#2D2D3F"
                    palette.buttonText: "#E2E8F0"
                }
                Button {
                    text: "↑"
                    onClicked: fileModel.goUp()
                    palette.button: "#2D2D3F"
                    palette.buttonText: "#E2E8F0"
                }
                Button {
                    text: "⌂"
                    onClicked: fileModel.goHome()
                    palette.button: "#2D2D3F"
                    palette.buttonText: "#E2E8F0"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    radius: 6
                    color: "#2D2D3F"

                    TextInput {
                        anchors.fill: parent
                        anchors.margins: 6
                        text: fileModel.currentPath
                        color: "#E2E8F0"
                        font.pixelSize: 12
                        clip: true
                        onAccepted: fileModel.currentPath = text
                    }
                }

                Button {
                    text: "⟳"
                    onClicked: fileModel.refresh()
                    palette.button: "#2D2D3F"
                    palette.buttonText: "#E2E8F0"
                }
            }
        }

        /* File list */
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: fileModel
            clip: true

            delegate: Rectangle {
                width: ListView.view.width
                height: 36
                color: delegateFileMa.containsMouse ? "#2D2D3F" : "transparent"
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 12

                    Text {
                        text: model.isDir ? "📁" : "📄"
                        font.pixelSize: 16
                    }

                    Text {
                        Layout.fillWidth: true
                        text: model.fileName
                        color: "#E2E8F0"
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }

                    Text {
                        text: model.fileSize
                        color: "#A0A0B0"
                        font.pixelSize: 11
                        Layout.preferredWidth: 80
                        horizontalAlignment: Text.AlignRight
                    }

                    Text {
                        text: model.fileModified
                        color: "#A0A0B0"
                        font.pixelSize: 11
                        Layout.preferredWidth: 130
                    }
                }

                MouseArea {
                    id: delegateFileMa
                    anchors.fill: parent
                    hoverEnabled: true
                    onDoubleClicked: fileModel.open(index)
                }
            }
        }

        /* Status bar */
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 24
            color: "#12121A"

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 12
                text: fileModel.currentPath
                color: "#A0A0B0"
                font.pixelSize: 11
            }
        }
    }
}
