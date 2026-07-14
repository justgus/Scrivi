import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// ProjectWindow.qml — the placeholder project screen (SP-059 / T-0227). Shown
// after a project is created; confirms the .scrivi exists by echoing the
// projectID/title/path returned by createProject. The real editor is EP-022; for
// now this is a stub with a Close action that returns to the landing view.
Page {
    id: root

    property string projectID: ""
    property string projectTitle: ""
    property string projectPath: ""

    signal closed()

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Button {
                text: qsTr("‹ Close")
                flat: true
                onClicked: root.closed()
            }
            Label {
                text: root.projectTitle.length > 0 ? root.projectTitle : qsTr("Project")
                font.bold: true
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            // Balance the Close button so the title stays centered.
            Item { width: 72 }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 16

        Label {
            text: qsTr("Project created")
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: qsTr("The editor lands in a future update. This confirms the "
                       + ".scrivi package was created on disk.")
            opacity: 0.7
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 2
            columnSpacing: 12
            rowSpacing: 6
            Layout.alignment: Qt.AlignHCenter

            Label { text: qsTr("Title:"); opacity: 0.6 }
            Label { text: root.projectTitle }
            Label { text: qsTr("Project ID:"); opacity: 0.6 }
            Label { text: root.projectID }
            Label { text: qsTr("Path:"); opacity: 0.6 }
            Label {
                text: root.projectPath
                elide: Text.ElideMiddle
                Layout.maximumWidth: 480
            }
        }
    }
}
