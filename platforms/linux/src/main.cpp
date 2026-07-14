#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QUrl>

#include "AppSupport.hpp"
#include "RecentsStore.hpp"
#include "ScriviWindow.hpp"

// Scrivi Linux app entry point.
//
// SP-061 (EP-022, T-0234) flips the app from a QQmlApplicationEngine loading a
// top-level QML ApplicationWindow to a Qt Widgets host: a QApplication + a native
// QMainWindow (ScriviWindow) whose central widget stacks the landing QML — hosted
// in a QQuickWidget — over the native EditorShell. This is forced by EP-022's
// QPlainTextEdit editor choice: on the pinned Qt 6.4 a QWidget can't embed cleanly
// inside a QML window, so QML lives inside Widgets, not the reverse.
//
// The QML module (URI "Scrivi") still registers ScriviBridge and RecentsStore as
// QML elements via QML_ELEMENT, so Landing.qml instantiates them directly. The QML
// requests the landing→editor swap through the "shell" context property
// (ShellController), replacing the old placeholder ProjectWindow push.
//
// appSupportRoot (SP-059 / T-0223) is resolved here and injected as a context
// property so QML passes the same stable path into every scrivi_* call and the
// recents store — no path logic in QML.
int main(int argc, char* argv[])
{
    // QApplication (Widgets): the host shell is a QMainWindow, and the New Project
    // folder picker uses a Qt Widgets QFileDialog (ScriviBridge::chooseFolder).
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Scrivi"));
    QApplication::setOrganizationName(QStringLiteral("Caposoft"));

    const QString appSupportRoot = scrivi::linux_app::appSupportRoot();

    // Where the New Project folder picker opens by default. In the Docker/VNC
    // harness, /projects is a host-shared bind mount (see build-and-run.sh) — a
    // folder reachable from both macOS and the container. Prefer it when present
    // so users land in the shared drop-zone instead of the app's CWD; otherwise
    // fall back to $HOME.
    QString defaultProjectsFolder = QDir::homePath();
    if (QFileInfo("/projects").isDir()) {
        defaultProjectsFolder = QStringLiteral("/projects");
    }

    // The landing QML, hosted in a QQuickWidget instead of a top-level window.
    auto* landing = new QQuickWidget;
    landing->rootContext()->setContextProperty(QStringLiteral("appSupportRoot"),
                                               appSupportRoot);
    landing->rootContext()->setContextProperty(QStringLiteral("defaultProjectsFolder"),
                                               defaultProjectsFolder);

    // The window owns the stacked landing/editor central widget.
    ScriviWindow window(landing, appSupportRoot);

    // The landing's Quit button calls Qt.quit(), which emits QQmlEngine::quit().
    // Under the old QQmlApplicationEngine bootstrap that signal was auto-wired to
    // QCoreApplication::quit(); a QQuickWidget's engine is NOT, so we connect it
    // ourselves (both quit() and the exit(int) variant), or the Quit button does
    // nothing ("Signal QQmlEngine::quit() emitted, but no receivers connected").
    QObject::connect(landing->engine(), &QQmlEngine::quit,
                     &app, &QApplication::quit);
    QObject::connect(landing->engine(), &QQmlEngine::exit,
                     &app, [](int code) { QApplication::exit(code); });

    // Flush pending editor edits on every quit path (T-0239): the landing Quit
    // button's Qt.quit() reaches QApplication::quit() (above) → aboutToQuit fires
    // → we flush here. This is the reliable hook for the Docker/VNC foreground quit;
    // the window's closeEvent covers the window-X path too.
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &window, &ScriviWindow::flushEditor);

    // ShellController is the QML → shell boundary for the landing→editor swap.
    // Parented to the window; exposed to the landing QML as "shell".
    auto* shell = new ShellController(&window, appSupportRoot);
    landing->rootContext()->setContextProperty(QStringLiteral("shell"), shell);

    // QQuickWidget can't loadFromModule on Qt 6.4; load the module's Landing.qml
    // by its qrc URL (CMake pins RESOURCE_PREFIX "/" → qrc:/<URI>/<QML_FILES path>).
    landing->setSource(QUrl(QStringLiteral("qrc:/Scrivi/qml/Landing.qml")));
    if (landing->status() == QQuickWidget::Error) {
        return -1;
    }

    window.show();
    return app.exec();
}
