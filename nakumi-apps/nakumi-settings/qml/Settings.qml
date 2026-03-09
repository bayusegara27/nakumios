import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import NakumiOS.Settings 1.0

ApplicationWindow {
    id: settingsWindow
    width: 700
    height: 500
    title: "NakumiOS Settings"
    color: "#1A1A24"

    SettingsManager { id: settings }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        /* Sidebar */
        Rectangle {
            Layout.preferredWidth: 180
            Layout.fillHeight: true
            color: "#12121A"

            ListView {
                id: categoryList
                anchors.fill: parent
                anchors.topMargin: 16
                currentIndex: 0
                model: ListModel {
                    ListElement { name: "Wi-Fi"; icon: "󰤨" }
                    ListElement { name: "Audio"; icon: "󰕾" }
                    ListElement { name: "Appearance"; icon: "🎨" }
                    ListElement { name: "About"; icon: "ℹ" }
                }
                delegate: Rectangle {
                    width: ListView.view.width
                    height: 40
                    color: ListView.isCurrentItem ? "#2D2D3F" : "transparent"
                    radius: 8

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        spacing: 12

                        Text {
                            text: model.icon
                            color: "#E2E8F0"
                            font.pixelSize: 16
                        }
                        Text {
                            text: model.name
                            color: "#E2E8F0"
                            font.pixelSize: 13
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: categoryList.currentIndex = index
                    }
                }
            }
        }

        /* Content area */
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            currentIndex: categoryList.currentIndex

            /* Wi-Fi page */
            ColumnLayout {
                spacing: 16

                Text { text: "Wi-Fi"; color: "#E2E8F0"; font.pixelSize: 20; font.bold: true }

                RowLayout {
                    Text { text: "Wi-Fi"; color: "#E2E8F0"; font.pixelSize: 14 }
                    Item { Layout.fillWidth: true }
                    Switch {
                        checked: settings.wifiEnabled
                        onToggled: settings.wifiEnabled = checked
                    }
                }

                Text {
                    text: settings.currentNetwork
                          ? "Connected to: " + settings.currentNetwork
                          : "Not connected"
                    color: "#A0A0B0"
                    font.pixelSize: 12
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: settings.availableNetworks
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 36
                        color: "#2D2D3F"
                        radius: 8

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: modelData
                            color: "#E2E8F0"
                            font.pixelSize: 13
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: settings.connectToNetwork(modelData, "")
                        }
                    }
                }

                Button {
                    text: "Scan Networks"
                    onClicked: settings.scanNetworks()
                    palette.button: "#6C5CE7"
                    palette.buttonText: "#FFFFFF"
                }
            }

            /* Audio page */
            ColumnLayout {
                spacing: 16

                Text { text: "Audio"; color: "#E2E8F0"; font.pixelSize: 20; font.bold: true }

                RowLayout {
                    Text { text: "Volume"; color: "#E2E8F0"; font.pixelSize: 14 }
                    Slider {
                        Layout.fillWidth: true
                        from: 0; to: 100
                        value: settings.volume
                        onMoved: settings.volume = value
                    }
                    Text {
                        text: Math.round(settings.volume) + "%"
                        color: "#A0A0B0"
                        font.pixelSize: 12
                    }
                }
            }

            /* Appearance page */
            ColumnLayout {
                spacing: 16

                Text { text: "Appearance"; color: "#E2E8F0"; font.pixelSize: 20; font.bold: true }

                Text { text: "Accent Color"; color: "#A0A0B0"; font.pixelSize: 12 }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    Repeater {
                        model: ["#6C5CE7", "#E74C3C", "#2ECC71", "#3498DB", "#F39C12", "#E91E63"]
                        Rectangle {
                            width: 36; height: 36
                            radius: 18
                            color: modelData
                            border.width: settings.accentColor === modelData ? 3 : 0
                            border.color: "#FFFFFF"
                            MouseArea {
                                anchors.fill: parent
                                onClicked: settings.accentColor = modelData
                            }
                        }
                    }
                }
            }

            /* About page */
            ColumnLayout {
                spacing: 16

                Text { text: "About NakumiOS"; color: "#E2E8F0"; font.pixelSize: 20; font.bold: true }

                GridLayout {
                    columns: 2
                    columnSpacing: 16
                    rowSpacing: 8

                    Text { text: "Hostname:"; color: "#A0A0B0"; font.pixelSize: 13 }
                    Text { text: settings.hostname; color: "#E2E8F0"; font.pixelSize: 13 }
                    Text { text: "Kernel:"; color: "#A0A0B0"; font.pixelSize: 13 }
                    Text { text: settings.kernelVersion; color: "#E2E8F0"; font.pixelSize: 13 }
                    Text { text: "CPU:"; color: "#A0A0B0"; font.pixelSize: 13 }
                    Text { text: settings.cpuInfo; color: "#E2E8F0"; font.pixelSize: 13 }
                    Text { text: "Memory:"; color: "#A0A0B0"; font.pixelSize: 13 }
                    Text { text: settings.memoryInfo; color: "#E2E8F0"; font.pixelSize: 13 }
                    Text { text: "Version:"; color: "#A0A0B0"; font.pixelSize: 13 }
                    Text { text: "NakumiOS 1.0.0"; color: "#6C5CE7"; font.pixelSize: 13; font.bold: true }
                }
            }
        }
    }
}
