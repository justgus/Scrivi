import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Scrivi

// Hello-ScriviCore slice (EP-020 / T-0220): a real Qt window that drives the
// full QML → ScriviBridge → C ABI → ScriviCore → JSON → QML round-trip. Clicking
// the button calls scrivi_ensure_local_identity and shows the decoded result.
ApplicationWindow {
    id: window
    width: 640
    height: 420
    visible: true
    title: qsTr("Scrivi — Linux (alpha)")

    ScriviBridge {
        id: bridge
        onErrorOccurred: (code, message) => {
            statusLabel.text = qsTr("Error %1: %2").arg(code).arg(message)
            statusLabel.color = "#c0392b"
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 16

        Label {
            text: qsTr("Scrivi")
            font.pixelSize: 32
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: qsTr("ScriviCore round-trip check")
            opacity: 0.7
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: qsTr("Display name:") }
            TextField {
                id: nameField
                Layout.fillWidth: true
                text: qsTr("Alpha Tester")
            }
        }

        Button {
            text: qsTr("Ensure Local Identity")
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                // /tmp is fine for the hello slice; real appSupportRoot resolution
                // lands with the project-lifecycle Epic (EP-021).
                const result = bridge.ensureLocalIdentity(nameField.text,
                                                          "/tmp/scrivi-linux-appsupport")
                if (result && result.identityID !== undefined) {
                    statusLabel.color = "#27ae60"
                    statusLabel.text =
                        qsTr("identityID: %1\ndefaultPersonaID: %2\ndisplayName: %3\ncreatedNew: %4")
                            .arg(result.identityID)
                            .arg(result.defaultPersonaID)
                            .arg(result.displayName)
                            .arg(result.createdNewIdentity)
                }
            }
        }

        Label {
            id: statusLabel
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: qsTr("Click to call ScriviCore.")
            opacity: 0.9
        }
    }
}
