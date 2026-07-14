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

    // Launch-bootstrap identity (SP-059 / T-0225). Populated by bootstrap();
    // read by createProject() and exposed to QML for display. `ready` flips true
    // once identity is resolved.
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString identityID READ identityID NOTIFY readyChanged)
    Q_PROPERTY(QString personaID READ personaID NOTIFY readyChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY readyChanged)

public:
    explicit ScriviBridge(QObject* parent = nullptr);

    bool ready() const { return ready_; }
    QString identityID() const { return identityID_; }
    QString personaID() const { return personaID_; }
    QString displayName() const { return displayName_; }

    // Launch identity bootstrap (mirrors Apple AppEnvironment.bootstrap()). Calls
    // scrivi_ensure_local_identity(displayName, appSupportRoot) once, stashes the
    // resolved identity/persona/displayName for later create/open calls, and flips
    // `ready`. Idempotent per process. `displayName` is the seed for a *new*
    // identity (hostname on Apple); an existing identity keeps its stored name.
    Q_INVOKABLE void bootstrap(const QString& displayName,
                               const QString& appSupportRoot);

    // Hello-ScriviCore slice (EP-020): resolve (or create) the local identity and
    // return its "result" object to QML as a QVariantMap. Kept reachable for dev
    // sanity; bootstrap() is the launch path. On failure returns an empty map and
    // emits errorOccurred(code, message).
    Q_INVOKABLE QVariantMap ensureLocalIdentity(const QString& displayName,
                                                const QString& appSupportRoot);

    // Creates a .scrivi project on disk (SP-059 / T-0225). Fills the remaining
    // scrivi_create_project args from the bootstrapped identity + appSupportRoot,
    // calls it, and returns {projectID, firstScene:{sceneID, metadataPath,
    // contentPath}} to QML. On failure returns an empty map and emits
    // errorOccurred. Requires bootstrap() to have succeeded first.
    Q_INVOKABLE QVariantMap createProject(const QString& projectRootPath,
                                          const QString& appSupportRoot,
                                          const QString& title,
                                          const QString& slug);

    // Opens a native "choose existing directory" dialog (Qt Widgets QFileDialog)
    // and returns the selected absolute path, or an empty string if cancelled
    // (SP-059 / T-0227 folder-picker fix). This replaces the Qt Quick FolderDialog,
    // which could only select an existing *child* folder — wrong for choosing where
    // to create a project (it couldn't select the folder you were viewing, e.g. an
    // empty /projects). QFileDialog selects the folder itself, needs no
    // Qt.labs.folderlistmodel, and lets the user create a new folder in-dialog.
    // `startDir` is the initial directory (absolute path); empty = platform default.
    Q_INVOKABLE QString chooseFolder(const QString& startDir);

signals:
    void readyChanged();
    void errorOccurred(int code, const QString& message);

private:
    // Parse a scrivi_* envelope string. On ok, returns the "result" object.
    // On error, emits errorOccurred and returns an empty map.
    QVariantMap parseEnvelope(const QString& json);

    bool    ready_ = false;
    QString identityID_;
    QString personaID_;
    QString displayName_;
};
