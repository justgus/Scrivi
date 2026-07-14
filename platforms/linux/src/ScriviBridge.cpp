#include "ScriviBridge.hpp"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <scrivi/scrivi.h>

// RAII guard for the heap-allocated JSON strings ScriviCore returns. Every
// scrivi_* return value MUST reach scrivi_free() exactly once; this makes that
// automatic across early returns.
namespace {
class ScriviString
{
public:
    explicit ScriviString(const char* p) : ptr_(p) {}
    ~ScriviString() { scrivi_free(ptr_); }
    ScriviString(const ScriviString&) = delete;
    ScriviString& operator=(const ScriviString&) = delete;

    [[nodiscard]] QString toQString() const
    {
        return ptr_ ? QString::fromUtf8(ptr_) : QString();
    }

private:
    const char* ptr_;
};
} // namespace

ScriviBridge::ScriviBridge(QObject* parent) : QObject(parent) {}

void ScriviBridge::bootstrap(const QString& displayName,
                             const QString& appSupportRoot)
{
    if (ready_) {
        return;   // idempotent — identity is resolved once per process
    }

    const QVariantMap result = ensureLocalIdentity(displayName, appSupportRoot);
    if (result.isEmpty() || !result.contains(QStringLiteral("identityID"))) {
        // ensureLocalIdentity already emitted errorOccurred; leave ready_ false.
        return;
    }

    identityID_  = result.value(QStringLiteral("identityID")).toString();
    personaID_   = result.value(QStringLiteral("defaultPersonaID")).toString();
    displayName_ = result.value(QStringLiteral("displayName")).toString();
    ready_       = true;
    emit readyChanged();
}

QVariantMap ScriviBridge::ensureLocalIdentity(const QString& displayName,
                                              const QString& appSupportRoot)
{
    const ScriviString envelope(
        scrivi_ensure_local_identity(displayName.toUtf8().constData(),
                                     appSupportRoot.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createProject(const QString& projectRootPath,
                                        const QString& appSupportRoot,
                                        const QString& title,
                                        const QString& slug)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    const ScriviString envelope(
        scrivi_create_project(projectRootPath.toUtf8().constData(),
                              appSupportRoot.toUtf8().constData(),
                              title.toUtf8().constData(),
                              slug.toUtf8().constData(),
                              identityID_.toUtf8().constData(),
                              personaID_.toUtf8().constData(),
                              displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QString ScriviBridge::chooseFolder(const QString& startDir)
{
    // Widgets QFileDialog in directory mode: selects the folder itself (not a
    // child) and offers "New Folder", so the user can pick an empty dir or make
    // one. ShowDirsOnly keeps files out of the view. Returns "" on cancel.
    const QString dir = QFileDialog::getExistingDirectory(
        nullptr,
        QStringLiteral("Choose Project Location"),
        startDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    return dir;   // already an absolute local path (not a URL)
}

QVariantMap ScriviBridge::parseEnvelope(const QString& json)
{
    QJsonParseError parseError;
    const QJsonDocument doc =
        QJsonDocument::fromJson(json.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit errorOccurred(-1, QStringLiteral("Malformed ScriviCore envelope: %1")
                                   .arg(parseError.errorString()));
        return {};
    }

    const QJsonObject root = doc.object();

    if (!root.value(QStringLiteral("ok")).toBool()) {
        const QJsonObject err = root.value(QStringLiteral("error")).toObject();
        emit errorOccurred(err.value(QStringLiteral("code")).toInt(),
                           err.value(QStringLiteral("message")).toString(
                               QStringLiteral("Unknown ScriviCore error")));
        return {};
    }

    return root.value(QStringLiteral("result")).toObject().toVariantMap();
}
