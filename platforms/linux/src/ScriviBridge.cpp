#include "ScriviBridge.hpp"

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

QVariantMap ScriviBridge::ensureLocalIdentity(const QString& displayName,
                                              const QString& appSupportRoot)
{
    const ScriviString envelope(
        scrivi_ensure_local_identity(displayName.toUtf8().constData(),
                                     appSupportRoot.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
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
