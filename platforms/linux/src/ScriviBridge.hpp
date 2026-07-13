#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <qqmlintegration.h>

// ScriviBridge — the QML ↔ ScriviCore C ABI boundary for the Linux app.
//
// This is the Qt analogue of ScriviEngine.swift on Apple: it calls the plain-C
// scrivi_* functions (scrivi.h), parses the {"ok":...}/{"error":...} JSON
// envelope, frees every returned pointer with scrivi_free(), and hands typed
// values to QML. It contains NO backend logic — all behavior lives in ScriviCore.
//
// Envelope contract (scrivi.h):
//   {"ok":true,  "result":{...}}
//   {"ok":false, "error":{"code":N,"message":"..."}}
class ScriviBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScriviBridge(QObject* parent = nullptr);

    // Hello-ScriviCore slice: resolve (or create) the local identity under
    // appSupportRoot and return its "result" object to QML as a QVariantMap.
    // On failure, returns an empty map and emits errorOccurred(code, message).
    Q_INVOKABLE QVariantMap ensureLocalIdentity(const QString& displayName,
                                                const QString& appSupportRoot);

signals:
    void errorOccurred(int code, const QString& message);

private:
    // Parse a scrivi_* envelope string. On ok, returns the "result" object.
    // On error, emits errorOccurred and returns an empty map.
    QVariantMap parseEnvelope(const QString& json);
};
