#pragma once

#include <QList>
#include <QMainWindow>
#include <QObject>
#include <QString>
#include <QVariantMap>

class AppEnvironment;
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
    // ⚠️ [SP-146] T-0560 — `env` carries the R3 check; nullptr in tests.
    explicit ShellController(ScriviWindow* window, QString appSupportRoot,
                             AppEnvironment* env = nullptr);

    // Called from Landing.qml on a successful "ready" open, in place of pushing the
    // old placeholder ProjectWindow. Swaps the central stack to the editor and
    // loads the project into it.
    // ⚠️ SP-144 ([I-0232] AC5): `openedProject` is the envelope Landing.qml
    // ALREADY obtained from `openProjectAsync`. Handing it over is what stops the
    // editor opening the same project a SECOND time (~79% of the first open's
    // cost). ✅ This mirrors Apple's `ProjectSession.loadAsync`, which opens once
    // and passes `result.scenes` into the scene loop.
    // ⚠️ QML passes it as a plain JS object; an empty map means "not supplied"
    // and the editor opens the project itself (the reload path).
    Q_INVOKABLE void openEditor(const QString& projectPath, const QString& title,
                                const QVariantMap& openedProject = {});

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

    // ⚠️ [SP-146] T-0560 — the app-global owner, for the R3 check. ⛔ NOT owned.
    AppEnvironment* env_ = nullptr;
};

class ScriviWindow : public QMainWindow
{
    Q_OBJECT

public:
    // `landing` is the QQuickWidget hosting Landing.qml (page 0). `appSupportRoot`
    // is forwarded to the editor when a project opens.
    // ⚠️ [SP-146] T-0559 — `env` is the app-global owner, constructed in `main()`.
    // ⛔ NOT owned here. ⚠️ nullptr is tolerated so tests can build a window without
    // an app environment; the registry is simply not consulted then.
    ScriviWindow(QQuickWidget* landing, QString appSupportRoot,
                 AppEnvironment* env = nullptr);

    // Build (lazily) + show the editor page for `projectPath`. Returns to landing
    // if the load fails.
    void showEditor(const QString& projectPath, const QString& title,
                    const QVariantMap& openedProject = {});

    // Return the central stack to the landing page (editor Close). File ▸ Open/Close
    // route here — the landing page hosts the full Open UI + the ScriviBridge, so the
    // menu doesn't reimplement that plumbing. File ▸ New additionally asks the landing
    // to open the New Project panel (see setShellController).
    void showLanding();

    // Wire the QML↔C++ boundary controller (created in main after the window) so File ▸
    // New can ask the landing QML to open its New Project panel (SP-077, T-0314).
    void setShellController(ShellController* shell) { shell_ = shell; }

    // Flush any pending editor edits to disk (T-0239).
    // ⚠️ [SP-146] T-0560 — NO LONGER WIRED DIRECTLY TO `aboutToQuit`.
    // ⛔ That hook was SINGULAR BY CONSTRUCTION (one window built by value in
    // `main()`), so with N windows it would flush one and drop the rest.
    // ✅ `AppEnvironment::flushAllWindows()` now calls this on EVERY open window.
    // No-op if no project is open.
    void flushEditor();

    // ⚠️ [SP-146] T-0560 — the projectID this window is showing, or empty.
    // ✅ Used by the window manager and by teardown.
    [[nodiscard]] QString shownProjectID() const;

    // ⚠️ [SP-147] T-0567 — apply this project's saved splitter proportions.
    // ✅ Safe with empty lists (nothing stored) — the defaults stay.
    void applySplitterSizes(const QList<int>& paneSizes, const QList<int>& outerSizes);

    // ⚠️ [SP-146] T-0560 — bring this window to the front (R3).
    // ✅ Called when the writer asks to open a project that is ALREADY open.
    void raiseToFront();

    ~ScriviWindow() override;

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
    QString           title_;   // ⚠️ [SP-147] T-0567 — recorded so restore can show it

    // ⚠️ [SP-146] T-0559 — the app-global owner, handed down from `main()`.
    // ⛔ NOT owned: it outlives this window. ✅ Passed on to each `EditorShell`.
    AppEnvironment*   env_ = nullptr;

    // Actions that are only meaningful with a project open in the editor.
    QList<QAction*> editorOnlyActions_;

    // View ▸ Show Inspector (SP-078, T-0320) — checkable; its check-state is
    // synced to the editor's actual inspector visibility on every page swap.
    QAction* showInspectorAction_ = nullptr;

    // View ▸ Show Timeline (SP-079, T-0323) — checkable; check-state synced to the
    // editor's actual timeline visibility on every page swap.
    QAction* showTimelineAction_ = nullptr;

    // ⚠️ [I-0256] — View ▸ Show Scene Navigator. ✅ Check-state synced per window in
    // `updateMenuState()`, exactly as the inspector and timeline actions are.
    QAction* showNavigatorAction_ = nullptr;
};
