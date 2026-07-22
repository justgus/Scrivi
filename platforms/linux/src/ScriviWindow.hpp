#pragma once

#include <QList>
#include <QMainWindow>
#include <QObject>
#include <QString>

class QAction;
class QQuickWidget;
class QStackedWidget;
class EditorShell;

// ScriviWindow — the Qt Widgets host shell (SP-061, EP-022 T-0234).
//
// EP-020/021 ran the app as a QQmlApplicationEngine loading a top-level QML
// ApplicationWindow. EP-022 locked QPlainTextEdit for the writing surface, and on
// the pinned Qt 6.4 a QWidget cannot embed cleanly inside a QML window — so the
// integration direction inverts: a native QMainWindow hosts the QML.
//
// The window's central widget is a QStackedWidget with two pages:
//   • page 0 — the LANDING QML (Landing.qml) hosted in a QQuickWidget. All of the
//     EP-020/021 flow (create / open / close / recents / identity / QFileDialog)
//     runs unchanged inside it.
//   • page 1 — the native EditorShell (navigator + read-only viewport).
//
// The QML calls into ShellController (a context property "shell") to request the
// swap: shell.openEditor(path, title) shows the editor; the editor's Close returns
// to landing. appSupportRoot is passed through so the editor opens against the same
// stable path the landing used.
class ScriviWindow;

class ShellController : public QObject
{
    Q_OBJECT

public:
    explicit ShellController(ScriviWindow* window, QString appSupportRoot);

    // Called from Landing.qml on a successful "ready" open, in place of pushing the
    // old placeholder ProjectWindow. Swaps the central stack to the editor and
    // loads the project into it.
    Q_INVOKABLE void openEditor(const QString& projectPath, const QString& title);

    // Ask the landing QML to open the New Project panel (SP-077, T-0314). The File ▸
    // New Project menu action emits this; Landing.qml listens via a Connections block
    // and pushes newProjectDialog. A signal (not an invokable) because the C++ side
    // drives the QML, which owns the StackView.
    void requestNewProject() { emit newProjectRequested(); }

    // Ask the landing QML to run the Open Project flow (SP-077, T-0315): its folder
    // picker + open, the same as the landing's Open Project button. File ▸ Open emits
    // this; Landing.qml listens and calls bridge.chooseFolder + openPath.
    void requestOpenProject() { emit openProjectRequested(); }

signals:
    void newProjectRequested();
    void openProjectRequested();

private:
    ScriviWindow* window_ = nullptr;
    QString appSupportRoot_;
};

class ScriviWindow : public QMainWindow
{
    Q_OBJECT

public:
    // `landing` is the QQuickWidget hosting Landing.qml (page 0). `appSupportRoot`
    // is forwarded to the editor when a project opens.
    ScriviWindow(QQuickWidget* landing, QString appSupportRoot);

    // Build (lazily) + show the editor page for `projectPath`. Returns to landing
    // if the load fails.
    void showEditor(const QString& projectPath, const QString& title);

    // Return the central stack to the landing page (editor Close). File ▸ Open/Close
    // route here — the landing page hosts the full Open UI + the ScriviBridge, so the
    // menu doesn't reimplement that plumbing. File ▸ New additionally asks the landing
    // to open the New Project panel (see setShellController).
    void showLanding();

    // Wire the QML↔C++ boundary controller (created in main after the window) so File ▸
    // New can ask the landing QML to open its New Project panel (SP-077, T-0314).
    void setShellController(ShellController* shell) { shell_ = shell; }

    // Flush any pending editor edits to disk (T-0239). Wired to
    // QCoreApplication::aboutToQuit in main() so the quit legs of the auto-save
    // cadence fire on every exit path — the landing Quit button (Qt.quit()), the
    // window close, and a normal app termination (incl. the Docker/VNC
    // foreground-process quit). No-op if no project is open. Public so main() can
    // connect it.
    void flushEditor();

protected:
    // The window-close (X) path also flushes before the app tears down.
    void closeEvent(QCloseEvent* event) override;

private:
    // Build the native menu bar (SP-077, T-0310/T-0311/T-0312) and stash the actions
    // whose enabled state depends on whether the editor page is active.
    void buildMenuBar();
    // Enable/disable the editor-only actions (Edit, Scene, Chapter, Project, File▸Close)
    // for the current page. Called from showEditor/showLanding.
    void updateMenuState(bool editorActive);

    QStackedWidget*   stack_       = nullptr;
    QQuickWidget*     landing_     = nullptr;
    EditorShell*      editor_      = nullptr;
    ShellController*  shell_       = nullptr;   // QML boundary (New Project panel)
    QString           appSupportRoot_;

    // Actions that are only meaningful with a project open in the editor.
    QList<QAction*> editorOnlyActions_;

    // View ▸ Show Inspector (SP-078, T-0320) — checkable; its check-state is
    // synced to the editor's actual inspector visibility on every page swap.
    QAction* showInspectorAction_ = nullptr;

    // View ▸ Show Timeline (SP-079, T-0323) — checkable; check-state synced to the
    // editor's actual timeline visibility on every page swap.
    QAction* showTimelineAction_ = nullptr;
};
