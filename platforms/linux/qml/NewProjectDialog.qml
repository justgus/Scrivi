import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Scrivi

// NewProjectDialog.qml — the New Project flow (SP-059 / T-0227), the Qt analogue
// of Apple's NewProjectSheet. Collects a title, an editable auto-derived slug, and
// a parent folder (via FolderDialog), composes <folder>/<slug>.scrivi, calls
// bridge.createProject, and on success adds the project to recents and emits
// created(). Bridge errors surface inline.
//
// Pushed onto Landing's StackView; the host owns the ScriviBridge/RecentsStore and
// passes them in, along with appSupportRoot.
Page {
    id: root

    // Injected by the host (Landing.qml). Typed concretely (not `var`) so the
    // Connections block below can resolve the bridge's errorOccurred signal, and
    // named …Ref/…Path so they don't shadow the outer ids of the same name on
    // their own binding right-hand side (which caused a binding loop).
    property ScriviBridge bridgeRef
    property RecentsStore recentsRef
    property string appSupportRootPath: ""

    signal cancelled()
    // Emitted on a successful create; args mirror the create result + inputs.
    signal created(string projectID, string title, string path)

    property string chosenFolder: ""     // parent directory (file:// URL → local path)
    property string errorText: ""

    // Title → slug auto-derivation (mirrors NewProjectSheet): lowercase, spaces to
    // hyphens, keep [a-z0-9-]. User can override the slug field afterward.
    function deriveSlug(text) {
        return text.toLowerCase()
                   .replace(/ /g, "-")
                   .replace(/[^a-z0-9-]/g, "")
    }

    function composedPath() {
        if (chosenFolder.length === 0 || slugField.text.length === 0)
            return ""
        return chosenFolder + "/" + slugField.text + ".scrivi"
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: qsTr("New Project")
                font.pixelSize: 18
                font.bold: true
                Layout.leftMargin: 12
            }
            Item { Layout.fillWidth: true }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 18

        GridLayout {
            columns: 2
            columnSpacing: 12
            rowSpacing: 12
            Layout.fillWidth: true

            Label { text: qsTr("Title:") }
            TextField {
                id: titleField
                Layout.fillWidth: true
                placeholderText: qsTr("My Novel")
                onTextChanged: {
                    if (!slugField.userEdited)
                        slugField.text = root.deriveSlug(text)
                }
            }

            Label { text: qsTr("Slug:") }
            TextField {
                id: slugField
                Layout.fillWidth: true
                placeholderText: qsTr("my-novel")
                property bool userEdited: false
                onTextEdited: userEdited = true
            }

            Label { text: qsTr("Location:") }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Label {
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                    opacity: root.chosenFolder.length > 0 ? 1.0 : 0.5
                    text: root.chosenFolder.length > 0
                          ? root.composedPath()
                          : qsTr("Not chosen")
                }
                Button {
                    text: qsTr("Choose…")
                    onClicked: {
                        // Native Qt Widgets directory picker via the bridge. Starts
                        // in the already-chosen folder, else the shared/default
                        // folder from main.cpp. Returns "" on cancel.
                        var start = root.chosenFolder.length > 0
                                    ? root.chosenFolder : defaultProjectsFolder
                        var picked = bridgeRef.chooseFolder(start)
                        if (picked.length > 0)
                            root.chosenFolder = picked
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.errorText.length > 0
            text: root.errorText
            color: "#c0392b"
            wrapMode: Text.Wrap
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                onClicked: root.cancelled()
            }
            Button {
                text: qsTr("Create")
                highlighted: true
                enabled: titleField.text.length > 0
                         && slugField.text.trim().length > 0
                         && root.chosenFolder.length > 0
                onClicked: root.doCreate()
            }
        }
    }

    function doCreate() {
        errorText = ""
        var slug = slugField.text.trim()
        if (slug.length === 0) {
            errorText = qsTr("Slug cannot be empty.")
            return
        }
        var path = composedPath()
        var result = bridgeRef.createProject(path, appSupportRootPath, titleField.text, slug)
        if (result && result.projectID !== undefined) {
            recentsRef.addOrUpdate(path, titleField.text)
            root.created(result.projectID, titleField.text, path)
        } else {
            // bridge emitted errorOccurred; reflect it inline if not already shown.
            if (errorText.length === 0)
                errorText = qsTr("Could not create the project.")
        }
    }

    // Surface bridge errors raised during this flow inline.
    Connections {
        target: bridgeRef
        function onErrorOccurred(code, message) {
            root.errorText = qsTr("Error %1: %2").arg(code).arg(message)
        }
    }
}
