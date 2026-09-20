import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Scrivi

// Landing.qml — the Linux app's landing screen (SP-059 / T-0226), replacing the
// EP-020 hello window. Hosts the app-wide ScriviBridge and RecentsStore, bootstraps
// the local identity at launch, and shows the recent-projects list, a first-launch
// empty state, and the New Project / Open Project actions.
//
// SP-061 (EP-022 / T-0234): the app flipped to a Qt Widgets host — this QML is now
// embedded in a QQuickWidget inside a QMainWindow, so the root is a plain Item (not
// a top-level ApplicationWindow). A StackView drives the create/open flow: the
// landing page is the base; New Project pushes NewProjectDialog. Opening a "ready"
// project no longer pushes a QML placeholder — it calls shell.openEditor(path,
// title) so the native editor (navigator + viewport) takes over the window.
// `appSupportRoot`, `defaultProjectsFolder`, and `shell` are context properties
// injected by main.cpp.
Item {
    id: window
    width: 820
    height: 560

    // Landing-view error text, held at window scope so it is reachable from the
    // shared open flow (openPath) and the bridge error handler alike. (It must NOT
    // live only on the landing Page component — `landingPage` is a Component, not
    // the instantiated Page, so writing a property on it throws and aborts the
    // caller. That bug silently broke Open/recents-click in the first cut.)
    property string landingError: ""

    // App-wide backend boundary + recents store, shared with pushed pages.
    ScriviBridge {
        id: bridge
        onErrorOccurred: (code, message) => {
            window.landingError = qsTr("Error %1: %2").arg(code).arg(message)
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

    // Shared Open flow (SP-060 / T-0231), reachable from the Open Project button
    // and from a recents-row click. Calls bridge.openProject and branches the three
    // core open modes:
    //   • ready          → record in recents + hand off to the native editor (shell)
    //   • repairRequired → list the issues in a dialog; DO NOT enter the project
    //   • cannotOpen / error → the bridge emits errorOccurred (shown inline); {}
    // ⚠️ SP-144 / [I-0232] — THIS IS ASYNCHRONOUS NOW. It STARTS the open and
    // returns immediately; the branching happens in the Connections block below.
    //
    // ⚠️ WHAT IT USED TO DO: `var result = bridge.openProject(path, ...)`, a
    // BLOCKING call on the UI thread. MEASURED on the rig 2026-09-18 against
    // `the-stairs-of-tintagael.scrivi` on a `cache=none` CIFS mount: 155 s frozen
    // on this line, launch screen silent, "Scrivi is not Responding" every 5 s.
    // ✅ [I-0195] had already moved `EditorShell::load` off the thread — but that
    // runs AFTER this returns, so its progress bar could not appear until the
    // freeze was over.
    //
    // ⚠️ THE PROJECT IS STILL OPENED TWICE (here, then in `EditorShell::load`).
    // ✅ That is a RULED, DELIBERATE narrowing (SP-144 AC5, user 2026-09-20):
    // handing this result to `EditorShell` would rework the one half that
    // currently works, and [I-0231] made each open ~3x cheaper. ⚠️ Both opens are
    // now off the UI thread, so neither freezes — the remaining cost is
    // duplicated WORK, not duplicated FREEZE.
    property bool opening: false
    property string openingPath: ""

    function openPath(path) {
        if (!path || path.length === 0)
            return
        if (window.opening)
            return   // already opening; never fire a second blocking read
        window.landingError = ""
        window.openingPath = path
        window.opening = true
        bridge.openProjectAsync(path, appSupportRoot)
    }

    // ⚠️ Exactly ONE of these fires per `openProjectAsync` (AsyncCall guarantees
    // it), so every branch must clear `opening` or the landing stays stuck.
    Connections {
        target: bridge

        function onProjectOpened(result) {
            var path = window.openingPath
            window.opening = false
            window.openingPath = ""

            if (!result || result.mode === undefined)
                return

            if (result.mode === "repairRequired") {
                repairDialog.projectPath = path
                repairDialog.issues = result.repairIssues || []
                repairDialog.open()
                return
            }

            // ready — record in recents (moves to front) and hand off to the
            // native editor shell (SP-061 / T-0234).
            var title = recentTitleFor(path)
            recents.addOrUpdate(path, title)
            shell.openEditor(path, title)
        }

        function onProjectOpenFailed() {
            // `errorOccurred` already set landingError with the detail.
            window.opening = false
            window.openingPath = ""
        }

        function onProjectOpenTimedOut() {
            window.opening = false
            window.openingPath = ""
            // ⚠️ Honest wording: the worker is still parked inside the read and
            // the project is NOT known to be broken. ⛔ Never say "missing" or
            // "damaged" here — a wrong-but-confident status invites destructive
            // remedies ([I-0115]).
            window.landingError =
                qsTr("This project is taking longer than expected to open and was left alone. It may be on a slow or unreachable drive.")
        }
    }

    // Best-effort display title for a path: the existing recents title if we have
    // one, else the folder name minus the .scrivi extension.
    function recentTitleFor(path) {
        for (var i = 0; i < recents.entries.length; ++i) {
            if (recents.entries[i].path === path) {
                var t = recents.entries[i].title
                if (t && t.length > 0)
                    return t
            }
        }
        var name = path.substring(path.lastIndexOf("/") + 1)
        return name.replace(/\.scrivi$/, "")
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: landingPage
    }

    // File ▸ New Project (SP-077, T-0314): the native menu can't reach the landing's
    // StackView directly, so ScriviWindow's menu action emits shell.newProjectRequested
    // and we open the New Project panel here — the same push the New Project button does.
    // Pop back to the landing root first so it opens from a known state (e.g. if a dialog
    // was already pushed).
    Connections {
        target: shell
        function onNewProjectRequested() {
            window.landingError = ""
            stack.pop(landingPage)
            stack.push(newProjectDialog)
        }
        // File ▸ Open Project (SP-077, T-0315): run the same folder-picker + open the
        // landing's Open Project button does. Pop to the landing root first so we open
        // from a known state.
        function onOpenProjectRequested() {
            window.landingError = ""
            stack.pop(landingPage)
            var picked = bridge.choosePackage(defaultProjectsFolder)
            window.openPath(picked)
        }
    }

    // repairRequired: surface the issue titles and stay on the landing view
    // (surface + block — full repair workflow is a later Epic). SP-060 / T-0231.
    Dialog {
        id: repairDialog
        property string projectPath: ""
        property var issues: []
        anchors.centerIn: parent
        width: Math.min(parent ? parent.width - 80 : 480, 520)
        modal: true
        title: qsTr("This project needs repair")
        standardButtons: Dialog.Ok

        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: qsTr("Scrivi can't open this project yet — the following need attention. "
                           + "Repairing projects arrives in a later update.")
                opacity: 0.85
            }
            Repeater {
                model: repairDialog.issues
                delegate: RowLayout {
                    Layout.fillWidth: true
                    required property var modelData
                    Label { text: "•"; opacity: 0.6 }
                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        text: (modelData.title && modelData.title.length > 0)
                              ? modelData.title : qsTr("Unknown issue")
                    }
                }
            }
        }
    }

    // ---- Landing page -----------------------------------------------------
    Component {
        id: landingPage

        Page {
            // Mirrors the window-level error so the label below can bind to it.
            readonly property string errorText: window.landingError

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
                        enabled: bridge.ready && !window.opening
                        onClicked: {
                            window.landingError = ""
                            stack.push(newProjectDialog)
                        }
                    }
                    Button {
                        text: qsTr("Open Project")
                        enabled: bridge.ready && !window.opening
                        onClicked: {
                            window.landingError = ""
                            // Pick an existing .scrivi directory with the Widgets
                            // QFileDialog (reused from the create flow), then open it.
                            var picked = bridge.choosePackage(defaultProjectsFolder)
                            window.openPath(picked)
                        }
                    }
                    Button {
                        text: qsTr("Quit")
                        // Clean in-app exit. In the Docker/VNC harness the app is
                        // the container's foreground process, so quitting also
                        // tears the container down (run-vnc.sh waits on the app).
                        onClicked: Qt.quit()
                    }
                }

                // ⚠️ SP-144 / [I-0232] — the launch screen's HONEST WAIT.
                //
                // ⚠️ Before this, opening a project from here froze the whole
                // window for the entire read with NOTHING shown — 155 s of it on
                // the rig. ✅ The bar is INDETERMINATE on purpose: the landing has
                // no scene count to divide by (that arrives WITH the open it is
                // waiting for), and [I-0195]'s ruling was that a spinner must not
                // pretend to be a percentage. ✅ The DETERMINATE `n of m scenes`
                // bar appears next, in EditorShell, once the count is real.
                RowLayout {
                    Layout.fillWidth: true
                    visible: window.opening
                    spacing: 12

                    BusyIndicator {
                        running: window.opening
                        implicitWidth: 24
                        implicitHeight: 24
                    }
                    Label {
                        text: qsTr("Opening this project…")
                        opacity: 0.8
                    }
                    ProgressBar {
                        Layout.fillWidth: true
                        indeterminate: true
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
                            // ⚠️ SP-144 / [I-0232]: also disabled DURING an open.
                            // `openPath` guards re-entry anyway, but a row that
                            // still looks clickable while nothing happens reads as
                            // a hang — which is the defect this Sprint is about.
                            enabled: bridge.ready && !window.opening
                            // Click a recent to open it (SP-060 / T-0231).
                            onClicked: window.openPath(modelData.path)

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
                // Pop the create dialog back to the landing base, then hand off to
                // the native editor shell (SP-061 / T-0234). The just-created
                // project opens straight into the navigator + viewport.
                stack.pop(null)
                shell.openEditor(path, title)
            }
        }
    }
}
