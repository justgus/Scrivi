#include "AppSupport.hpp"

#include <QDir>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace scrivi::linux_app {

QString appSupportRoot()
{
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // XDG spec: $XDG_DATA_HOME/Scrivi when set and non-empty, else
    // $HOME/.local/share/Scrivi. We test the raw string (not just presence) so
    // an exported-but-empty XDG_DATA_HOME correctly falls through to $HOME.
    QString base;
    const QString xdg = env.value(QStringLiteral("XDG_DATA_HOME"));
    if (!xdg.isEmpty()) {
        base = xdg;
    } else {
        const QString home = env.value(QStringLiteral("HOME"));
        if (!home.isEmpty()) {
            base = home + QStringLiteral("/.local/share");
        } else {
            // Neither XDG_DATA_HOME nor HOME set — fall back to Qt's writable
            // data location so a usable path is always returned. Qt already
            // appends the org/app, so use it as-is rather than adding /Scrivi.
            const QString qtLoc =
                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            if (!qtLoc.isEmpty()) {
                QDir().mkpath(qtLoc);
                return QDir(qtLoc).absolutePath();
            }
            base = QDir::tempPath() + QStringLiteral("/.local/share");
        }
    }

    const QString root = base + QStringLiteral("/Scrivi");
    QDir().mkpath(root);
    return QDir(root).absolutePath();
}

} // namespace scrivi::linux_app
