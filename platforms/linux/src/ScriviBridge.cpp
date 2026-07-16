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

QVariantMap ScriviBridge::openProject(const QString& projectRootPath,
                                      const QString& appSupportRoot)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    const ScriviString envelope(
        scrivi_open_project(projectRootPath.toUtf8().constData(),
                            appSupportRoot.toUtf8().constData(),
                            identityID_.toUtf8().constData()));
    // parseEnvelope returns the ok "result" (carrying "mode" for ready /
    // repairRequired) or, for a cannotOpen / other error envelope, emits
    // errorOccurred and returns {}.
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::openScene(const QString& projectRootPath,
                                    const QString& appSupportRoot,
                                    const QString& projectID,
                                    const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    const ScriviString envelope(
        scrivi_open_scene(projectRootPath.toUtf8().constData(),
                          appSupportRoot.toUtf8().constData(),
                          projectID.toUtf8().constData(),
                          sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::saveScene(const QString& projectID,
                                    const QString& projectRootPath,
                                    const QString& appSupportRoot,
                                    const QString& sceneID,
                                    const QString& sceneMetadataPath,
                                    const QString& sceneContentPath,
                                    const QString& markdown,
                                    long long selectionAnchor,
                                    long long selectionFocus,
                                    double scroll)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    const ScriviString envelope(
        scrivi_save_scene(projectID.toUtf8().constData(),
                          projectRootPath.toUtf8().constData(),
                          appSupportRoot.toUtf8().constData(),
                          sceneID.toUtf8().constData(),
                          sceneMetadataPath.toUtf8().constData(),
                          sceneContentPath.toUtf8().constData(),
                          markdown.toUtf8().constData(),
                          selectionAnchor,
                          selectionFocus,
                          scroll,
                          identityID_.toUtf8().constData(),
                          personaID_.toUtf8().constData(),
                          displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createScene(const QString& projectRootPath,
                                      const QString& appSupportRoot,
                                      const QString& projectID,
                                      const QString& chapterID,
                                      const QString& afterSceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    const ScriviString envelope(
        scrivi_create_scene(projectRootPath.toUtf8().constData(),
                            appSupportRoot.toUtf8().constData(),
                            projectID.toUtf8().constData(),
                            chapterID.toUtf8().constData(),
                            afterSceneID.toUtf8().constData(),
                            identityID_.toUtf8().constData(),
                            personaID_.toUtf8().constData(),
                            displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createChapter(const QString& projectRootPath,
                                        const QString& appSupportRoot,
                                        const QString& projectID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    const ScriviString envelope(
        scrivi_create_chapter(projectRootPath.toUtf8().constData(),
                              appSupportRoot.toUtf8().constData(),
                              projectID.toUtf8().constData(),
                              identityID_.toUtf8().constData(),
                              personaID_.toUtf8().constData(),
                              displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::deleteScene(const QString& projectRootPath,
                                      const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Delete takes no author identity — it is a structural removal, not an authored
    // edit. Just the project root + the target sceneID.
    const ScriviString envelope(
        scrivi_delete_scene(projectRootPath.toUtf8().constData(),
                            sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::deleteChapter(const QString& projectRootPath,
                                        const QString& chapterID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Removes the chapter and every scene it contains. The "chapter + all its scenes"
    // confirmation is the caller's responsibility (EditorShell, T-0251).
    const ScriviString envelope(
        scrivi_delete_chapter(projectRootPath.toUtf8().constData(),
                              chapterID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::reorderScene(const QString& projectRootPath,
                                       const QString& sceneID,
                                       const QString& sourceChapterID,
                                       const QString& targetChapterID,
                                       const QString& afterSceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Moves the scene within its chapter (source == target) or across chapters. No
    // author identity — a reorder is a structural move, not an authored edit. An empty
    // afterSceneID places it first in the target chapter.
    const ScriviString envelope(
        scrivi_reorder_scene(projectRootPath.toUtf8().constData(),
                             sceneID.toUtf8().constData(),
                             sourceChapterID.toUtf8().constData(),
                             targetChapterID.toUtf8().constData(),
                             afterSceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::reorderChapter(const QString& projectRootPath,
                                         const QString& chapterID,
                                         const QString& afterChapterID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Moves the chapter to sit after afterChapterID (empty = front). No author identity.
    const ScriviString envelope(
        scrivi_reorder_chapter(projectRootPath.toUtf8().constData(),
                               chapterID.toUtf8().constData(),
                               afterChapterID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::renameScene(const QString& projectRootPath,
                                      const QString& metadataPath,
                                      const QString& newTitle)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Writes the scene's sidecar `title`. A blank/whitespace title is passed through
    // as-is (ScriviCore stores it empty = "no custom title"); the caller decides the
    // navigator fallback. No author identity — rename is a metadata edit, not authored.
    const ScriviString envelope(
        scrivi_rename_scene(projectRootPath.toUtf8().constData(),
                            metadataPath.toUtf8().constData(),
                            newTitle.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::renameChapter(const QString& projectRootPath,
                                        const QString& metadataPath,
                                        const QString& newTitle)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // `metadataPath` is the CHAPTER's metadata path (open_project's
    // `chapterMetadataPath`). Blank/whitespace clears the custom title.
    const ScriviString envelope(
        scrivi_rename_chapter(projectRootPath.toUtf8().constData(),
                              metadataPath.toUtf8().constData(),
                              newTitle.toUtf8().constData()));
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
