#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "AppSupport.hpp"
#include "RecentsStore.hpp"

// Scrivi Linux app entry point. The QML module (URI "Scrivi") registers
// ScriviBridge and RecentsStore as QML elements via QML_ELEMENT, so the QML can
// instantiate them directly — no manual qmlRegisterType needed.
//
// appSupportRoot (SP-059 / T-0223) is resolved here and injected as a context
// property so QML passes the same stable path into every scrivi_* call and the
// recents store — no path logic in QML.
int main(int argc, char* argv[])
{
    // QApplication (not QGuiApplication): the New Project folder picker uses a Qt
    // Widgets QFileDialog (via ScriviBridge::chooseFolder), which requires the
    // widgets application instance.
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

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appSupportRoot"),
                                             appSupportRoot);
    engine.rootContext()->setContextProperty(QStringLiteral("defaultProjectsFolder"),
                                             defaultProjectsFolder);

    // engine.loadFromModule() is Qt 6.5+; the toolchain is pinned to Qt 6.4
    // (Ubuntu 24.04 apt), so load the module's Landing.qml by its qrc URL instead.
    // CMake pins RESOURCE_PREFIX "/", so the file is at qrc:/<URI>/<QML_FILES path>.
    engine.load(QUrl(QStringLiteral("qrc:/Scrivi/qml/Landing.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
