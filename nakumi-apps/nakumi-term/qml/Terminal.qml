import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import NakumiOS.Terminal 1.0

/*
 * NakumiOS Terminal - Custom terminal emulator
 *
 * Design Tokens:
 *   Background: #1A1A24
 *   Accent: #6C5CE7
 *   Text: #E2E8F0
 */

ApplicationWindow {
    id: termWindow
    width: 800
    height: 500
    title: "NakumiTerm"
    color: "#1A1A24"

    TerminalBackend {
        id: terminal
        rows: Math.floor(scrollView.height / 16)
        cols: Math.floor(scrollView.width / 8.4)
    }

    Component.onCompleted: terminal.start()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        /* Title bar */
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: "#12121A"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12

                Text {
                    text: "NakumiTerm"
                    color: "#A0A0B0"
                    font.pixelSize: 12
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                /* Terminal status */
                Text {
                    text: terminal.running ? "●" : "○"
                    color: terminal.running ? "#00CC66" : "#CC3333"
                    font.pixelSize: 12
                    Layout.rightMargin: 12
                }
            }
        }

        /* Terminal output */
        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextEdit {
                id: outputArea
                width: scrollView.width
                text: terminal.outputText
                color: "#E2E8F0"
                font.family: "monospace"
                font.pixelSize: 14
                readOnly: true
                wrapMode: TextEdit.NoWrap
                padding: 8
                selectByMouse: true
                selectedTextColor: "#1A1A24"
                selectionColor: "#6C5CE7"

                onTextChanged: {
                    /* Auto-scroll to bottom */
                    cursorPosition = text.length
                }
            }
        }

        /* Input area */
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#3D3D5C"
        }
    }

    /* Keyboard input handler */
    Item {
        anchors.fill: parent
        focus: true

        Keys.onPressed: function(event) {
            if (event.text.length > 0) {
                terminal.sendInput(event.text)
                event.accepted = true
            } else {
                terminal.sendKey(event.key, event.modifiers)
                event.accepted = true
            }
        }
    }
}
