#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

// Scrivi Linux app entry point. The QML module (URI "Scrivi") registers
// ScriviBridge as a QML element via QML_ELEMENT, so Main.qml can instantiate it
// directly — no manual qmlRegisterType needed.
int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Scrivi"));
    QGuiApplication::setOrganizationName(QStringLiteral("Caposoft"));

    QQmlApplicationEngine engine;
    // engine.loadFromModule() is Qt 6.5+; the toolchain is pinned to Qt 6.4
    // (Ubuntu 24.04 apt), so load the module's Main.qml by its qrc URL instead.
    // CMake pins RESOURCE_PREFIX "/", so the file is at qrc:/<URI>/<QML_FILES path>.
    engine.load(QUrl(QStringLiteral("qrc:/Scrivi/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
