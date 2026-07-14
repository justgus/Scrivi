import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Scrivi

// Landing.qml — the Linux app's root screen (SP-059 / T-0226), replacing the
// EP-020 hello window. Hosts the app-wide ScriviBridge and RecentsStore, bootstraps
// the local identity at launch, and shows the recent-projects list, a first-launch
// empty state, and the New Project / Open Project actions.
//
// A StackView drives the single-window flow: the landing page is the base; New
// Project pushes NewProjectDialog; a successful create pushes the placeholder
// ProjectWindow (the real editor is EP-022). `appSupportRoot` is a context
// property injected by main.cpp.
ApplicationWindow {
    id: window
    width: 820
    height: 560
    visible: true
    title: qsTr("Scrivi — Linux (alpha)")

    // App-wide backend boundary + recents store, shared with pushed pages.
    ScriviBridge {
        id: bridge
        onErrorOccurred: (code, message) => {
            landingPage.errorText = qsTr("Error %1: %2").arg(code).arg(message)
        }
    }

    RecentsStore {
        id: recents
    }

    // Launch bootstrap: resolve identity + load recents against the injected
    // appSupportRoot, once, before the user acts (mirrors Apple bootstrap()).
    Component.onCompleted: {
        recents.configure(appSupportRoot)
        bridge.bootstrap(Qt.application.name, appSupportRoot)
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: landingPage
    }

    // ---- Landing page -----------------------------------------------------
    Component {
        id: landingPage

        Page {
            property string errorText: ""

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 32
                spacing: 20

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 2
                        Label {
                            text: qsTr("Scrivi")
                            font.pixelSize: 32
                            font.bold: true
                        }
                        Label {
                            text: bridge.ready
                                  ? qsTr("Signed in as %1").arg(bridge.displayName)
                                  : qsTr("Preparing…")
                            opacity: 0.7
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: qsTr("New Project")
                        enabled: bridge.ready
                        onClicked: {
                            errorText = ""
                            stack.push(newProjectDialog)
                        }
                    }
                    Button {
                        text: qsTr("Open Project")
                        // Real open lands in SP-060; present but disabled for now.
                        enabled: false
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Available in the next update")
                    }
                    Button {
                        text: qsTr("Quit")
                        // Clean in-app exit. In the Docker/VNC harness the app is
                        // the container's foreground process, so quitting also
                        // tears the container down (run-vnc.sh waits on the app).
                        onClicked: Qt.quit()
                    }
                }

                Label {
                    text: qsTr("Recent Projects")
                    font.pixelSize: 18
                    font.bold: true
                }

                // Recent-projects list (bound to RecentsStore, newest-first).
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // First-launch / empty state.
                    ColumnLayout {
                        anchors.centerIn: parent
                        visible: recents.entries.length === 0
                        spacing: 8
                        Label {
                            text: qsTr("No recent projects yet")
                            font.pixelSize: 16
                            opacity: 0.7
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Label {
                            text: qsTr("Create a project to get started.")
                            opacity: 0.5
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    ListView {
                        anchors.fill: parent
                        visible: recents.entries.length > 0
                        clip: true
                        model: recents.entries
                        spacing: 4

                        delegate: ItemDelegate {
                            required property var modelData
                            width: ListView.view ? ListView.view.width : 0
                            // Open is wired in SP-060; rows are inert for now.
                            enabled: false

                            contentItem: ColumnLayout {
                                spacing: 2
                                Label {
                                    text: modelData.title && modelData.title.length > 0
                                          ? modelData.title : qsTr("Untitled")
                                    font.bold: true
                                }
                                Label {
                                    text: modelData.path
                                    opacity: 0.6
                                    font.pixelSize: 12
                                    elide: Text.ElideMiddle
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: qsTr("Last opened: %1").arg(modelData.lastOpened)
                                    opacity: 0.5
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: errorText.length > 0
                    text: errorText
                    color: "#c0392b"
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    // ---- New Project dialog (T-0227) --------------------------------------
    Component {
        id: newProjectDialog

        NewProjectDialog {
            // Distinct property names (…Ref) avoid a binding loop: a property named
            // `bridge` here would shadow the outer `id: bridge` on its own RHS.
            bridgeRef: bridge
            recentsRef: recents
            appSupportRootPath: appSupportRoot
            onCancelled: stack.pop()
            onCreated: (projectID, title, path) => {
                // Replace the dialog with the placeholder project window so Back
                // does not return to the create form.
                stack.replace(projectWindow, {
                    "projectID": projectID,
                    "projectTitle": title,
                    "projectPath": path
                })
            }
        }
    }

    // ---- Placeholder project window (real editor is EP-022) ---------------
    Component {
        id: projectWindow
        ProjectWindow {
            onClosed: stack.pop(null)   // back to landing
        }
    }
}
