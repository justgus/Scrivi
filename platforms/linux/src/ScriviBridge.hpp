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

    // Opens an existing .scrivi project (SP-060 / T-0230). Fills appSupportRoot +
    // the bootstrapped identityID, calls scrivi_open_project, and returns the ok
    // "result" object to QML — which carries "mode":
    //   • "ready"          → {projectID, mode, activeScene{...}, restored{...}, scenes:[...]}
    //   • "repairRequired" → {projectID, mode, repairIssues:[{...}]}
    // The third core open mode, "cannotOpen", comes back as an *error* envelope, so
    // it is surfaced via errorOccurred and this returns an empty map (same as any
    // other failure). Requires bootstrap() to have succeeded first. No backend logic.
    Q_INVOKABLE QVariantMap openProject(const QString& projectRootPath,
                                        const QString& appSupportRoot);

    // Loads a single scene's body (SP-061 / T-0235). Calls scrivi_open_scene and
    // returns its ok "result": {scene{sceneID,chapterID,title,slug,metadataPath,
    // contentPath}, markdown}. Used by the editor shell to fetch the bodies of the
    // non-active scenes when assembling the continuous viewport (the active scene's
    // body already comes back inside openProject). On failure emits errorOccurred
    // and returns {}. Requires bootstrap() to have succeeded first.
    Q_INVOKABLE QVariantMap openScene(const QString& projectRootPath,
                                      const QString& appSupportRoot,
                                      const QString& projectID,
                                      const QString& sceneID);

    // Persists one scene's Markdown (SP-062 / T-0239). Fills the author args from
    // the bootstrapped identity, calls scrivi_save_scene, and returns its ok
    // "result": {sceneID, saved, wordCount}. `selectionAnchor`/`selectionFocus` are
    // scene-local cursor offsets and `scroll` the scroll fraction — persisted so a
    // later open can restore the surface (used in full by SP-064); pass 0/0/0.0 when
    // not tracking them yet. On failure emits errorOccurred and returns {}. Requires
    // bootstrap() to have succeeded first.
    Q_INVOKABLE QVariantMap saveScene(const QString& projectID,
                                      const QString& projectRootPath,
                                      const QString& appSupportRoot,
                                      const QString& sceneID,
                                      const QString& sceneMetadataPath,
                                      const QString& sceneContentPath,
                                      const QString& markdown,
                                      long long selectionAnchor,
                                      long long selectionFocus,
                                      double scroll);

    // Creates a new scene in `chapterID`, inserted after `afterSceneID` (empty =
    // append to the chapter). Fills the author args from the bootstrapped identity,
    // calls scrivi_create_scene, and returns its ok "result": {sceneID, chapterID,
    // metadataPath, contentPath}. The Linux analogue of Apple's in-editor ⌘↩
    // (SP-062 / T-0240). On failure emits errorOccurred and returns {}.
    Q_INVOKABLE QVariantMap createScene(const QString& projectRootPath,
                                        const QString& appSupportRoot,
                                        const QString& projectID,
                                        const QString& chapterID,
                                        const QString& afterSceneID);

    // Creates a new chapter (with its first scene). Fills the author args from the
    // bootstrapped identity, calls scrivi_create_chapter, and returns its ok
    // "result": {chapterID, chapterMetadataPath, firstSceneID, firstSceneMetadataPath,
    // firstSceneContentPath}. The analogue of ⌘⇧↩ (SP-062 / T-0241). On failure
    // emits errorOccurred and returns {}.
    // `afterChapterID` (optional) positions the new chapter immediately after that
    // chapter — its folder is born in place, no reorder/rename (EP-027 P6). Empty (the
    // default) appends at the manuscript end.
    Q_INVOKABLE QVariantMap createChapter(const QString& projectRootPath,
                                          const QString& appSupportRoot,
                                          const QString& projectID,
                                          const QString& afterChapterID = {});

    // Deletes one scene from disk + the manuscript/chapter index (EP-023 / SP-065,
    // T-0250). Calls scrivi_delete_scene(projectRootPath, sceneID) and returns its ok
    // "result". Delete takes no author identity (it is not an authored edit). On
    // failure emits errorOccurred and returns {}. Requires bootstrap() first (so the
    // bridge is in a consistent state), though the endpoint itself needs no identity.
    Q_INVOKABLE QVariantMap deleteScene(const QString& projectRootPath,
                                        const QString& sceneID);

    // Deletes one chapter AND all its scenes from disk + the index (EP-023 / SP-065,
    // T-0250). Calls scrivi_delete_chapter(projectRootPath, chapterID) and returns its
    // ok "result". On failure emits errorOccurred and returns {}. The caller is
    // responsible for the "chapter + all its scenes" confirmation before invoking.
    Q_INVOKABLE QVariantMap deleteChapter(const QString& projectRootPath,
                                          const QString& chapterID);

    // Moves a scene within its chapter or across chapters (EP-023 / SP-067, T-0258).
    // Calls scrivi_reorder_scene(projectRootPath, sceneID, sourceChapterID,
    // targetChapterID, afterSceneID) and returns its ok "result": {sceneID,
    // sourceChapterID, targetChapterID, reordered}. `afterSceneID` empty = the scene
    // becomes the first scene of the target chapter; otherwise it lands right after
    // that sibling. source == target is a within-chapter reorder. No author identity
    // (structural, not an authored edit). On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap reorderScene(const QString& projectRootPath,
                                         const QString& sceneID,
                                         const QString& sourceChapterID,
                                         const QString& targetChapterID,
                                         const QString& afterSceneID);

    // Moves a whole chapter to sit right after `afterChapterID` (EP-023 / SP-067,
    // T-0258). Calls scrivi_reorder_chapter(projectRootPath, chapterID, afterChapterID)
    // and returns its ok "result": {chapterID, afterChapterID, reordered}. An empty
    // `afterChapterID` moves the chapter to the manuscript's front. Used here by the
    // I-0064 chapter-split orchestration (moving the appended chapter into caret
    // position); full chapter drag-reorder is SP-068. No author identity. On failure
    // emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap reorderChapter(const QString& projectRootPath,
                                           const QString& chapterID,
                                           const QString& afterChapterID);

    // Merge one scene into the scene immediately BEFORE it in the SAME chapter
    // (EP-028 / SP-076, T-0305 — the Linux analogue of Apple's ⌘⌫). Calls
    // scrivi_merge_scene(projectRootPath, sceneID) and returns its ok "result":
    // {survivorSceneID, mergedSceneID, chapterID, survivorMetadataPath,
    // survivorContentPath, chapterMetadataPath, merged}. The survivor keeps its own
    // files; `sceneID`'s body is appended (blank-line join) and its files removed. The
    // caller (EditorShell) enforces the start-of-scene / first-scene-of-chapter no-op
    // BEFORE invoking. No author identity (structural, not an authored edit). On
    // failure emits errorOccurred and returns {}.
    Q_INVOKABLE QVariantMap mergeScene(const QString& projectRootPath,
                                       const QString& sceneID);

    // Merge a whole chapter into the chapter immediately BEFORE it in manuscript order
    // (EP-028 / SP-076, T-0305 — the analogue of ⇧⌘⌫). Calls
    // scrivi_merge_chapter(projectRootPath, chapterID): every scene file of `chapterID`
    // is RELOCATED into the predecessor's folder (order-key files renamed after its last
    // scene), then the emptied chapter is removed. Returns its ok "result":
    // {survivorChapterID, mergedChapterID, survivorChapterMetadataPath, scenesRelocated,
    // merged}. This is the atomic, no-scene-loss fix (I-0083) — the caller does NOT
    // compose it from deleteChapter. The caller enforces the first-scene-of-chapter /
    // manuscript-start no-op. No author identity. On failure emits errorOccurred,
    // returns {}.
    Q_INVOKABLE QVariantMap mergeChapter(const QString& projectRootPath,
                                         const QString& chapterID);

    // Renames one scene — writes the sidecar `title` field (EP-023 / SP-066, T-0254).
    // Calls scrivi_rename_scene(projectRootPath, metadataPath, newTitle) and returns its
    // ok "result": {metadataPath, newTitle, renamed}. `metadataPath` is the scene's own
    // sidecar path; a blank/whitespace `newTitle` clears the custom title (saved empty),
    // and the navigator falls back to the first prose line. On failure emits
    // errorOccurred and returns {}. No author identity (rename is not an authored edit).
    Q_INVOKABLE QVariantMap renameScene(const QString& projectRootPath,
                                        const QString& metadataPath,
                                        const QString& newTitle);

    // Renames one chapter — writes the chapter sidecar `title` (EP-023 / SP-066, T-0254).
    // Calls scrivi_rename_chapter(projectRootPath, metadataPath, newTitle) and returns
    // its ok "result": {metadataPath, newTitle, renamed}. `metadataPath` is the
    // CHAPTER's metadata path (carried in scrivi_open_project's scene entries as
    // `chapterMetadataPath`). Blank/whitespace clears the custom title → navigator +
    // heading fall back to "Chapter N". On failure emits errorOccurred and returns {}.
    Q_INVOKABLE QVariantMap renameChapter(const QString& projectRootPath,
                                          const QString& metadataPath,
                                          const QString& newTitle);

    // Returns the project's timeline meta (EP-025 / SP-079, T-0321). Calls
    // scrivi_get_timeline(projectRootPath) and returns its ok "result":
    // {timelineID, epochLabel, projectID, createdAt}. Used by the Timeline panel to
    // label the story-time origin ("Story Open"). The timeline C ABI already exists
    // (EP-016/SP-039); scrivi.h is untouched. On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap getTimeline(const QString& projectRootPath);

    // Returns one scene's story-time (EP-025 / SP-079, T-0321). Calls
    // scrivi_get_scene_story_time(projectRootPath, sceneID) and returns its ok
    // "result": {sceneID, offsetMs, offsetSource, gapMs, durationMs, durationSource,
    // inferenceHint, inferenceConfidence, bandID, bandAssignedAt}. The Timeline panel
    // reads gapMs + durationMs for each scene and computes the dot offsets via the
    // default gap chain (mirroring Apple's TimelineViewModel.recomputeAllOffsets). On
    // failure emits errorOccurred and returns {}.
    Q_INVOKABLE QVariantMap getSceneStoryTime(const QString& projectRootPath,
                                              const QString& sceneID);

    // Writes one scene's story-time (EP-025 / SP-080, T-0325). Calls
    // scrivi_set_scene_story_time(projectRootPath, sceneID, offsetMs, source, gapMs,
    // durationMs, durationSource) and returns its ok "result" (the updated block).
    // The canonical stored value is `gapMs` (gap from the previous scene's END to this
    // scene's START); `offsetMs` is the derived absolute position. `source` is
    // "manual" for a writer-placed scene or "default" to return it to the gap chain.
    // Used by the Time Delta Picker commit + the chain re-persist (T-0328). Peer to
    // getSceneStoryTime; the endpoint is already exported (EP-016). No author identity.
    // On failure emits errorOccurred and returns {}.
    Q_INVOKABLE QVariantMap setSceneStoryTime(const QString& projectRootPath,
                                              const QString& sceneID,
                                              long long offsetMs,
                                              const QString& source,
                                              long long gapMs,
                                              long long durationMs,
                                              const QString& durationSource);

    // --- Story structure (EP-025 / SP-081, T-0329) ------------------------
    //
    // Wrappers over the story-structure C ABI (all exported since EP-016; scrivi.h
    // untouched). A structure is a named set of proportional colored BANDS painted
    // behind the timeline dots; scenes assign to a band.

    // Returns {hasStructure, structureID, bandLayoutJSON}. bandLayoutJSON is an array
    // of {bandID, label, color, proportion} (proportions sum to 1.0). Empty structure
    // when none set. On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap getStoryStructure(const QString& projectRootPath);

    // Sets the project's story structure (`structureID`, e.g. "three-act") with the
    // given `bandLayoutJSON` (the app supplies the built-in preset layout). Persists
    // + returns the ok result. On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap setStoryStructure(const QString& projectRootPath,
                                              const QString& structureID,
                                              const QString& bandLayoutJSON);

    // Updates only the band proportions/layout of the current structure (border-drag
    // re-proportion, T-0331) — `bandLayoutJSON` is the new full layout. On failure
    // emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap updateBandLayout(const QString& projectRootPath,
                                             const QString& bandLayoutJSON);

    // Removes the story structure. Scene offsets + bandID assignments are PRESERVED
    // (the AC4 contract) — only the band rendering goes away. On failure emits
    // errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap removeStoryStructure(const QString& projectRootPath);

    // Assigns `sceneID` to band `bandID` (drag-up / "Assign to Act…", T-0332). The dot
    // then paints a ring in the band's color. On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap assignSceneToBand(const QString& projectRootPath,
                                              const QString& sceneID,
                                              const QString& bandID);

    // Clears `sceneID`'s band assignment ("Unassign"). On failure emits errorOccurred,
    // returns {}.
    Q_INVOKABLE QVariantMap unassignSceneFromBand(const QString& projectRootPath,
                                                  const QString& sceneID);

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
