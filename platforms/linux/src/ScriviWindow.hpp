#pragma once

#include <QMainWindow>
#include <QObject>
#include <QString>

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

    // Return the central stack to the landing page (editor Close).
    void showLanding();

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
    QStackedWidget* stack_       = nullptr;
    QQuickWidget*   landing_     = nullptr;
    EditorShell*    editor_      = nullptr;
    QString         appSupportRoot_;
};
