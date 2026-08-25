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
                            "" /* beforeSceneID — Apple-only for now (EP-026 will surface it) */,
                            identityID_.toUtf8().constData(),
                            personaID_.toUtf8().constData(),
                            displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createChapter(const QString& projectRootPath,
                                        const QString& appSupportRoot,
                                        const QString& projectID,
                                        const QString& afterChapterID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // afterChapterID positions the new chapter right after that chapter (born in place,
    // no reorder needed — EP-027 P6). Empty → append at the manuscript end.
    const ScriviString envelope(
        scrivi_create_chapter(projectRootPath.toUtf8().constData(),
                              appSupportRoot.toUtf8().constData(),
                              projectID.toUtf8().constData(),
                              identityID_.toUtf8().constData(),
                              personaID_.toUtf8().constData(),
                              displayName_.toUtf8().constData(),
                              afterChapterID.toUtf8().constData()));
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

QVariantMap ScriviBridge::mergeScene(const QString& projectRootPath,
                                     const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Joins this scene into the previous scene of the same chapter (survivor keeps its
    // files; this scene's files are removed). No author identity — a merge is a
    // structural op, not an authored edit. The caller guards the no-op cases.
    const ScriviString envelope(
        scrivi_merge_scene(projectRootPath.toUtf8().constData(),
                           sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::mergeChapter(const QString& projectRootPath,
                                       const QString& chapterID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Relocates every scene file of this chapter into the predecessor's folder, then
    // removes the emptied chapter — the atomic I-0083 fix (no scene loss on reopen).
    // No author identity. The caller guards the no-op cases.
    const ScriviString envelope(
        scrivi_merge_chapter(projectRootPath.toUtf8().constData(),
                             chapterID.toUtf8().constData()));
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

QVariantMap ScriviBridge::getTimeline(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Read-only; no author identity. Returns {timelineID, epochLabel, projectID,
    // createdAt}. The timeline C ABI is already exported (EP-016) — scrivi.h untouched.
    const ScriviString envelope(
        scrivi_get_timeline(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::getSceneStoryTime(const QString& projectRootPath,
                                            const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Read-only; no author identity. Returns the scene's story-time block
    // (gapMs/durationMs drive the dot chain; offsetSource/bandID for later sprints).
    const ScriviString envelope(
        scrivi_get_scene_story_time(projectRootPath.toUtf8().constData(),
                                    sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setSceneStoryTime(const QString& projectRootPath,
                                            const QString& sceneID,
                                            long long offsetMs,
                                            const QString& source,
                                            long long gapMs,
                                            long long durationMs,
                                            const QString& durationSource)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }

    // Structural (timeline placement), no author identity. `gapMs` is canonical; the
    // endpoint recomputes/persists this scene's block. Chain propagation to later
    // scenes is the caller's job (EditorShell, T-0328), one setSceneStoryTime each.
    const ScriviString envelope(
        scrivi_set_scene_story_time(projectRootPath.toUtf8().constData(),
                                    sceneID.toUtf8().constData(),
                                    offsetMs,
                                    source.toUtf8().constData(),
                                    gapMs,
                                    durationMs,
                                    durationSource.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::getStoryStructure(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_get_story_structure(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setStoryStructure(const QString& projectRootPath,
                                            const QString& structureID,
                                            const QString& bandLayoutJSON)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_set_story_structure(projectRootPath.toUtf8().constData(),
                                   structureID.toUtf8().constData(),
                                   bandLayoutJSON.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::updateBandLayout(const QString& projectRootPath,
                                           const QString& bandLayoutJSON)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_update_band_layout(projectRootPath.toUtf8().constData(),
                                  bandLayoutJSON.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::removeStoryStructure(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_remove_story_structure(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::assignSceneToBand(const QString& projectRootPath,
                                            const QString& sceneID,
                                            const QString& bandID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_assign_scene_to_band(projectRootPath.toUtf8().constData(),
                                    sceneID.toUtf8().constData(),
                                    bandID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::unassignSceneFromBand(const QString& projectRootPath,
                                                const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_unassign_scene_from_band(projectRootPath.toUtf8().constData(),
                                        sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

// --- Historical events + imported timelines + export (EP-025 / SP-082, T-0340) ---
// Thin wrappers over the timeline-events C ABI (scrivi.h 270-291, complete since
// EP-016; untouched). Same shape as the story-structure wrappers above: ready_ guard,
// RAII ScriviString, parseEnvelope. No author identity (empty identity args) — a
// historical event is worldbuilding data, not an authored persona artefact, matching
// how story-time is written (T-0325).

QVariantMap ScriviBridge::createHistoricalEvent(const QString& projectRootPath,
                                                const QString& title,
                                                long long offsetMs,
                                                const QString& description,
                                                const QString& tagsJSON)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_create_historical_event(projectRootPath.toUtf8().constData(),
                                       title.toUtf8().constData(),
                                       static_cast<int64_t>(offsetMs),
                                       description.toUtf8().constData(),
                                       tagsJSON.toUtf8().constData(),
                                       "", "", ""));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::updateHistoricalEvent(const QString& projectRootPath,
                                                const QString& eventID,
                                                const QString& title,
                                                long long offsetMs,
                                                const QString& description,
                                                const QString& tagsJSON)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_update_historical_event(projectRootPath.toUtf8().constData(),
                                       eventID.toUtf8().constData(),
                                       title.toUtf8().constData(),
                                       static_cast<int64_t>(offsetMs),
                                       description.toUtf8().constData(),
                                       tagsJSON.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::deleteHistoricalEvent(const QString& projectRootPath,
                                                const QString& eventID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_delete_historical_event(projectRootPath.toUtf8().constData(),
                                       eventID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listHistoricalEvents(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_list_historical_events(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::importExternalTimeline(const QString& projectRootPath,
                                                 const QString& timelineJSON,
                                                 long long epochOffsetMs,
                                                 const QString& assignedGreyShade)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_import_external_timeline(projectRootPath.toUtf8().constData(),
                                        timelineJSON.toUtf8().constData(),
                                        static_cast<int64_t>(epochOffsetMs),
                                        assignedGreyShade.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::updateImportedTimelineOffset(const QString& projectRootPath,
                                                       const QString& timelineID,
                                                       long long epochOffsetMs)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_update_imported_timeline_offset(projectRootPath.toUtf8().constData(),
                                               timelineID.toUtf8().constData(),
                                               static_cast<int64_t>(epochOffsetMs)));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setImportedTimelineVisible(const QString& projectRootPath,
                                                     const QString& timelineID,
                                                     bool visible)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_set_imported_timeline_visible(projectRootPath.toUtf8().constData(),
                                             timelineID.toUtf8().constData(),
                                             visible ? 1 : 0));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listImportedTimelines(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_list_imported_timelines(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::removeImportedTimeline(const QString& projectRootPath,
                                                 const QString& timelineID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_remove_imported_timeline(projectRootPath.toUtf8().constData(),
                                        timelineID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::exportProjectTimeline(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(
        scrivi_export_project_timeline(projectRootPath.toUtf8().constData()));
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

// ====================================================================
// EP-034 SP-121 — ABI parity completion (T-0461 / T-0462 / T-0463)
//
// See the block comment in ScriviBridge.hpp for why these ship without a
// UI consumer, and for the identity/`scrivi_free` contracts they follow.
// ====================================================================

QVariantMap ScriviBridge::addComment(const QString& projectRootPath, const QString& scopeKind, const QString& targetID, const QString& body)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_add_comment(projectRootPath.toUtf8().constData(),
                                        scopeKind.toUtf8().constData(),
                                        targetID.toUtf8().constData(),
                                        body.toUtf8().constData(),
                                        identityID_.toUtf8().constData(),
                                        personaID_.toUtf8().constData(),
                                        displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::addWorld(const QString& projectRootPath, const QString& packagePath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_add_world(projectRootPath.toUtf8().constData(),
                                      packagePath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::applyRepair(const QString& issueID, const QString& projectRootPath, const QString& appSupportRoot, const QString& actionKind, const QString& targetPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_apply_repair(issueID.toUtf8().constData(),
                                         projectRootPath.toUtf8().constData(),
                                         appSupportRoot.toUtf8().constData(),
                                         actionKind.toUtf8().constData(),
                                         targetPath.toUtf8().constData(),
                                         identityID_.toUtf8().constData(),
                                         personaID_.toUtf8().constData(),
                                         displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::clearSceneStoryTime(const QString& projectRootPath, const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_clear_scene_story_time(projectRootPath.toUtf8().constData(),
                                                   sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createEdge(const QString& projectRootPath, const QString& fromID, const QString& toID, const QString& relationTypeCode, const QString& note)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_create_edge(projectRootPath.toUtf8().constData(),
                                        fromID.toUtf8().constData(),
                                        toID.toUtf8().constData(),
                                        relationTypeCode.toUtf8().constData(),
                                        note.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createObject(const QString& projectRootPath, const QString& objectKind, const QString& displayName, const QString& slug, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_create_object(projectRootPath.toUtf8().constData(),
                                          objectKind.toUtf8().constData(),
                                          displayName.toUtf8().constData(),
                                          slug.toUtf8().constData(),
                                          identityID_.toUtf8().constData(),
                                          personaID_.toUtf8().constData(),
                                          displayName_.toUtf8().constData(),
                                          worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createSnapshot(const QString& projectRootPath, const QString& displayName, const QString& label, const QString& note)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_create_snapshot(projectRootPath.toUtf8().constData(),
                                            identityID_.toUtf8().constData(),
                                            personaID_.toUtf8().constData(),
                                            displayName.toUtf8().constData(),
                                            label.toUtf8().constData(),
                                            note.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::createWorld(const QString& projectRootPath, const QString& packagePath, const QString& displayName, const QString& epochLabel)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_create_world(projectRootPath.toUtf8().constData(),
                                         packagePath.toUtf8().constData(),
                                         displayName.toUtf8().constData(),
                                         epochLabel.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::deleteEdge(const QString& projectRootPath, const QString& edgeID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_delete_edge(projectRootPath.toUtf8().constData(),
                                        edgeID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::deleteObject(const QString& projectRootPath, const QString& objectKind, const QString& objectID, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_delete_object(projectRootPath.toUtf8().constData(),
                                          objectKind.toUtf8().constData(),
                                          objectID.toUtf8().constData(),
                                          worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::enableGitSnapshots(const QString& projectRootPath, const QString& displayName, const QString& initialSnapshotLabel)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_enable_git_snapshots(projectRootPath.toUtf8().constData(),
                                                 identityID_.toUtf8().constData(),
                                                 personaID_.toUtf8().constData(),
                                                 displayName.toUtf8().constData(),
                                                 initialSnapshotLabel.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::extractSearchableText(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_extract_searchable_text(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::fragmentCut(const QString& projectRootPath, const QString& spansJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_fragment_cut(projectRootPath.toUtf8().constData(),
                                         spansJson.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::fragmentExtract(const QString& projectRootPath, const QString& spansJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_fragment_extract(projectRootPath.toUtf8().constData(),
                                             spansJson.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::fragmentPaste(const QString& projectRootPath, const QString& appSupportRoot, const QString& projectID, const QString& fragmentJson, const QString& caretSceneID, long long caretByteOffset, const QString& displayName)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_fragment_paste(projectRootPath.toUtf8().constData(),
                                           appSupportRoot.toUtf8().constData(),
                                           projectID.toUtf8().constData(),
                                           fragmentJson.toUtf8().constData(),
                                           caretSceneID.toUtf8().constData(),
                                           caretByteOffset,
                                           identityID_.toUtf8().constData(),
                                           personaID_.toUtf8().constData(),
                                           displayName.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::fragmentUncutPaste(const QString& projectRootPath, const QString& fragmentJson, const QString& targetSceneID, const QString& createdIDsJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_fragment_uncut_paste(projectRootPath.toUtf8().constData(),
                                                 fragmentJson.toUtf8().constData(),
                                                 targetSceneID.toUtf8().constData(),
                                                 createdIDsJson.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::getSceneNotes(const QString& projectRootPath, const QString& sceneID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_get_scene_notes(projectRootPath.toUtf8().constData(),
                                            sceneID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::getWorldBinding(const QString& projectRootPath, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_get_world_binding(projectRootPath.toUtf8().constData(),
                                              worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::getWorldStatus(const QString& projectRootPath, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_get_world_status(projectRootPath.toUtf8().constData(),
                                             worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::importAsset(const QString& projectRootPath, const QString& sourcePath, const QString& category, const QString& title, const QString& worldID, const QString& projectID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_import_asset(projectRootPath.toUtf8().constData(),
                                         sourcePath.toUtf8().constData(),
                                         category.toUtf8().constData(),
                                         title.toUtf8().constData(),
                                         identityID_.toUtf8().constData(),
                                         personaID_.toUtf8().constData(),
                                         displayName_.toUtf8().constData(),
                                         worldID.toUtf8().constData(),
                                         projectID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::importFromInbox(const QString& projectRootPath, const QString& filename, const QString& action, const QString& category)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_import_from_inbox(projectRootPath.toUtf8().constData(),
                                              filename.toUtf8().constData(),
                                              action.toUtf8().constData(),
                                              category.toUtf8().constData(),
                                              identityID_.toUtf8().constData(),
                                              personaID_.toUtf8().constData(),
                                              displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listAssets(const QString& projectRootPath, const QString& category, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_assets(projectRootPath.toUtf8().constData(),
                                        category.toUtf8().constData(),
                                        worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listComments(const QString& projectRootPath, const QString& scopeKind, const QString& targetID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_comments(projectRootPath.toUtf8().constData(),
                                          scopeKind.toUtf8().constData(),
                                          targetID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listEdgesFor(const QString& projectRootPath, const QString& endpointID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_edges_for(projectRootPath.toUtf8().constData(),
                                           endpointID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listInbox(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_inbox(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listObjectKinds()
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_object_kinds());
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listObjects(const QString& projectRootPath, const QString& kindOrNull)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_objects(projectRootPath.toUtf8().constData(),
                                         kindOrNull.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listOrphanedObjects(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_orphaned_objects(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listPendingEdges(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_pending_edges(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listRelationTypes(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_relation_types(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::listWorlds(const QString& projectRootPath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_list_worlds(projectRootPath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::openObject(const QString& projectRootPath, const QString& objectKind, const QString& objectID, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_open_object(projectRootPath.toUtf8().constData(),
                                        objectKind.toUtf8().constData(),
                                        objectID.toUtf8().constData(),
                                        worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::promoteObject(const QString& projectRootPath, const QString& objectID, const QString& targetKind, const QString& worldIDOrNull)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_promote_object(projectRootPath.toUtf8().constData(),
                                           objectID.toUtf8().constData(),
                                           targetKind.toUtf8().constData(),
                                           worldIDOrNull.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::relinkWorld(const QString& projectRootPath, const QString& worldID, const QString& newPackagePath)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_relink_world(projectRootPath.toUtf8().constData(),
                                         worldID.toUtf8().constData(),
                                         newPackagePath.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::removeAsset(const QString& projectRootPath, const QString& assetID, const QString& worldID, const QString& projectID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_remove_asset(projectRootPath.toUtf8().constData(),
                                         assetID.toUtf8().constData(),
                                         worldID.toUtf8().constData(),
                                         projectID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::removeWorldReference(const QString& projectRootPath, const QString& worldID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_remove_world_reference(projectRootPath.toUtf8().constData(),
                                                   worldID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::resolveComment(const QString& projectRootPath, const QString& scopeKind, const QString& targetID, const QString& commentID, const QString& resolverDisplayName)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_resolve_comment(projectRootPath.toUtf8().constData(),
                                            scopeKind.toUtf8().constData(),
                                            targetID.toUtf8().constData(),
                                            commentID.toUtf8().constData(),
                                            identityID_.toUtf8().constData(),
                                            personaID_.toUtf8().constData(),
                                            resolverDisplayName.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::resolveTimelineProjectTimes(const QString& projectRootPath, const QString& worldID, const QString& timelineID)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_resolve_timeline_project_times(projectRootPath.toUtf8().constData(),
                                                           worldID.toUtf8().constData(),
                                                           timelineID.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::saveObject(const QString& projectRootPath, const QString& objectKind, const QString& objectJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_save_object(projectRootPath.toUtf8().constData(),
                                        objectKind.toUtf8().constData(),
                                        objectJson.toUtf8().constData(),
                                        identityID_.toUtf8().constData(),
                                        personaID_.toUtf8().constData(),
                                        displayName_.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::scanForExternalChanges(const QString& projectRootPath, const QString& appSupportRoot, int includeGitStatus)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_scan_for_external_changes(projectRootPath.toUtf8().constData(),
                                                      appSupportRoot.toUtf8().constData(),
                                                      includeGitStatus));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setSceneOutline(const QString& projectRootPath, const QString& sceneID, const QString& outline)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_set_scene_outline(projectRootPath.toUtf8().constData(),
                                              sceneID.toUtf8().constData(),
                                              outline.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setSceneTags(const QString& projectRootPath, const QString& sceneID, const QString& tagsJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_set_scene_tags(projectRootPath.toUtf8().constData(),
                                           sceneID.toUtf8().constData(),
                                           tagsJson.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setSceneTodo(const QString& projectRootPath, const QString& sceneID, const QString& todoJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_set_scene_todo(projectRootPath.toUtf8().constData(),
                                           sceneID.toUtf8().constData(),
                                           todoJson.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setTimelineEpochLabel(const QString& projectRootPath, const QString& label)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_set_timeline_epoch_label(projectRootPath.toUtf8().constData(),
                                                     label.toUtf8().constData()));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setTimelineEpochOffset(const QString& projectRootPath, const QString& worldID, const QString& timelineID, long long epochOffsetMs)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_set_timeline_epoch_offset(projectRootPath.toUtf8().constData(),
                                                      worldID.toUtf8().constData(),
                                                      timelineID.toUtf8().constData(),
                                                      epochOffsetMs));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::setWorldEpochOffset(const QString& projectRootPath, const QString& worldID, long long epochOffsetMs)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_set_world_epoch_offset(projectRootPath.toUtf8().constData(),
                                                   worldID.toUtf8().constData(),
                                                   epochOffsetMs));
    return parseEnvelope(envelope.toQString());
}

QVariantMap ScriviBridge::upsertRelationType(const QString& projectRootPath, const QString& relationTypeJson)
{
    if (!ready_) {
        emit errorOccurred(-1, QStringLiteral("Identity not bootstrapped"));
        return {};
    }
    const ScriviString envelope(scrivi_upsert_relation_type(projectRootPath.toUtf8().constData(),
                                                 relationTypeJson.toUtf8().constData()));
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
