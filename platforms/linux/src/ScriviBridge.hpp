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

    // Opens a package-aware chooser for picking an EXISTING `.scrivi` project
    // (I-0185). ⚠️ A package is a DIRECTORY, so `chooseFolder` above walks into
    // it; this one treats it as a leaf and selects it, while still resolving
    // upward if the writer descends anyway. Returns "" on cancel.
    // ⚠️ Use `chooseFolder` for "where should the NEW project go" — descending is
    // correct there.
    Q_INVOKABLE QString choosePackage(const QString& startDir);

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

    // --- Historical events + imported timelines + export (EP-025 / SP-082, T-0340) ---
    //
    // Wrappers over the timeline-events C ABI (all exported since EP-016/SP-039;
    // scrivi.h lines 270-291, untouched). Historical events are worldbuilding moments
    // (not scenes) that appear on the project row as their own dot; imported timelines
    // are external .scrivi-timeline.json files rendered as grey rows below the project
    // row; export produces a .scrivi-timeline.json body.
    //
    // Envelope shapes (confirmed against scrivi_c_api.cpp at planning):
    //   createHistoricalEvent  -> {eventID, slug}
    //   updateHistoricalEvent  -> {eventID, updated}
    //   deleteHistoricalEvent  -> {eventID, deleted}
    //   listHistoricalEvents   -> {count, eventsJSON}      (eventsJSON is a STRING:
    //     {"events":[{eventID,title,offsetMs,offsetSource,description,createdAt,
    //      modifiedAt}, ...]} — no tags/slug in the list projection)
    //   importExternalTimeline -> {timelineID, imported}
    //   updateImportedTimelineOffset / setImportedTimelineVisible -> {timelineID, updated}
    //   removeImportedTimeline -> {timelineID, removed}
    //   listImportedTimelines  -> {count, timelinesJSON}   (timelinesJSON is a STRING:
    //     {"timelines":[{timelineID,sourceProjectTitle,epochLabel,epochOffsetMs,visible,
    //      assignedGreyShade,eventCount}, ...]} — METADATA ONLY, no per-event array. To
    //      render imported dots the shell reads the stored files in
    //      objects/imported-timelines/ directly, exactly as Apple does — there is NO C ABI
    //      gap here, the list endpoint is metadata-only by design.)
    //   exportProjectTimeline  -> {timelineJSON}           (the full .scrivi-timeline.json body)

    // Creates a historical event (empty tags → pass "" for tagsJSON, else
    // {"tags":["a","b"]}). No author identity (empty identity args, like story-time).
    // On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap createHistoricalEvent(const QString& projectRootPath,
                                                  const QString& title,
                                                  long long offsetMs,
                                                  const QString& description,
                                                  const QString& tagsJSON);

    // Updates an existing historical event (all fields overwrite). On failure emits
    // errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap updateHistoricalEvent(const QString& projectRootPath,
                                                  const QString& eventID,
                                                  const QString& title,
                                                  long long offsetMs,
                                                  const QString& description,
                                                  const QString& tagsJSON);

    // Deletes a historical event by ID. On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap deleteHistoricalEvent(const QString& projectRootPath,
                                                  const QString& eventID);

    // Lists the project's historical events → {count, eventsJSON}. On failure emits
    // errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap listHistoricalEvents(const QString& projectRootPath);

    // Imports an external timeline. `timelineJSON` is the file body; `epochOffsetMs`
    // shifts every event; `assignedGreyShade` is the row's per-source grey (hex). On
    // failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap importExternalTimeline(const QString& projectRootPath,
                                                   const QString& timelineJSON,
                                                   long long epochOffsetMs,
                                                   const QString& assignedGreyShade);

    // Changes an imported timeline's epoch offset ("Edit Epoch Offset…"). On failure
    // emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap updateImportedTimelineOffset(const QString& projectRootPath,
                                                         const QString& timelineID,
                                                         long long epochOffsetMs);

    // Shows/hides an imported timeline row (persisted `visible` flag). On failure emits
    // errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap setImportedTimelineVisible(const QString& projectRootPath,
                                                       const QString& timelineID,
                                                       bool visible);

    // Lists imported timelines → {count, timelinesJSON} (metadata only). On failure
    // emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap listImportedTimelines(const QString& projectRootPath);

    // Removes an imported timeline (deletes its stored file). On failure emits
    // errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap removeImportedTimeline(const QString& projectRootPath,
                                                   const QString& timelineID);

    // Exports the project timeline → {timelineJSON} (scene + historical events; no prose
    // or identity). On failure emits errorOccurred, returns {}.
    Q_INVOKABLE QVariantMap exportProjectTimeline(const QString& projectRootPath);


    // ================================================================
    // EP-034 SP-121 (T-0461 / T-0462 / T-0463) — ABI parity completion
    // ================================================================
    //
    // ⚠️ **47 endpoints, bridged so the Linux app CAN reach what ScriviCore
    // already does.** Before this sprint `ScriviBridge` reached 34 of the 100
    // endpoints in `scrivi.h`; Apple's `ScriviEngine` reached 96. The audit is
    // `docs/Scrivi_ABI_Binding_Gap_Audit_v0_1.md` (T-0460).
    //
    // ⚠️ **NO UI CONSUMES THESE YET, AND THAT IS DELIBERATE — user-ruled
    // 2026-08-24:** *"We are aware of the disparity. However, we are planning
    // for that. We'll verify the backend with the ctests and the subsequent
    // Epic will manage the surfacing of the capability in the Linux App."*
    // The consumer is **EP-035**, already open in the Epic backlog carrying
    // EP-034's AC11. ⚠️ This is staged work with a scheduled reader — not
    // `capability_without_surface`, which is a dangling read with NO reader.
    //
    // ⚠️ **Identity is threaded by the BRIDGE, never by the caller.** Every
    // endpoint taking identityID/personaID/authorDisplayName is given the
    // bootstrapped identity here, exactly as Apple's `authorshipRef` does. A
    // QML caller cannot pass the wrong author because it cannot pass one.
    //
    // ⚠️ **`scrivi_free` is handled by `ScriviString` (RAII)** — every one of
    // these returns through it, so a leak is structurally impossible rather
    // than a thing each method must remember.
    //
    // ⚠️ **19 `scrivi_history_*` / `scrivi_buffers_*` endpoints are
    // DELIBERATELY ABSENT** — they belong to EP-019, whose Linux story has
    // never been ruled. Bridging them here would set that direction by
    // accident (the lesson of I-0144).

    Q_INVOKABLE QVariantMap addComment(const QString& projectRootPath, const QString& scopeKind, const QString& targetID, const QString& body);
    Q_INVOKABLE QVariantMap addWorld(const QString& projectRootPath, const QString& packagePath);
    Q_INVOKABLE QVariantMap applyRepair(const QString& issueID, const QString& projectRootPath, const QString& appSupportRoot, const QString& actionKind, const QString& targetPath);
    Q_INVOKABLE QVariantMap clearSceneStoryTime(const QString& projectRootPath, const QString& sceneID);
    Q_INVOKABLE QVariantMap createEdge(const QString& projectRootPath, const QString& fromID, const QString& toID, const QString& relationTypeCode, const QString& note);
    Q_INVOKABLE QVariantMap createObject(const QString& projectRootPath, const QString& objectKind, const QString& displayName, const QString& slug, const QString& worldID);
    Q_INVOKABLE QVariantMap createSnapshot(const QString& projectRootPath, const QString& displayName, const QString& label, const QString& note);
    Q_INVOKABLE QVariantMap createWorld(const QString& projectRootPath, const QString& packagePath, const QString& displayName, const QString& epochLabel);
    Q_INVOKABLE QVariantMap deleteEdge(const QString& projectRootPath, const QString& edgeID);
    Q_INVOKABLE QVariantMap deleteObject(const QString& projectRootPath, const QString& objectKind, const QString& objectID, const QString& worldID);
    Q_INVOKABLE QVariantMap enableGitSnapshots(const QString& projectRootPath, const QString& displayName, const QString& initialSnapshotLabel);
    Q_INVOKABLE QVariantMap extractSearchableText(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap fragmentCut(const QString& projectRootPath, const QString& spansJson);
    Q_INVOKABLE QVariantMap fragmentExtract(const QString& projectRootPath, const QString& spansJson);
    Q_INVOKABLE QVariantMap fragmentPaste(const QString& projectRootPath, const QString& appSupportRoot, const QString& projectID, const QString& fragmentJson, const QString& caretSceneID, long long caretByteOffset, const QString& displayName);
    Q_INVOKABLE QVariantMap fragmentUncutPaste(const QString& projectRootPath, const QString& fragmentJson, const QString& targetSceneID, const QString& createdIDsJson);
    Q_INVOKABLE QVariantMap getSceneNotes(const QString& projectRootPath, const QString& sceneID);
    Q_INVOKABLE QVariantMap getWorldBinding(const QString& projectRootPath, const QString& worldID);
    Q_INVOKABLE QVariantMap getWorldStatus(const QString& projectRootPath, const QString& worldID);
    Q_INVOKABLE QVariantMap importAsset(const QString& projectRootPath, const QString& sourcePath, const QString& category, const QString& title, const QString& worldID, const QString& projectID);
    Q_INVOKABLE QVariantMap importFromInbox(const QString& projectRootPath, const QString& filename, const QString& action, const QString& category);
    Q_INVOKABLE QVariantMap listAssets(const QString& projectRootPath, const QString& category, const QString& worldID);
    Q_INVOKABLE QVariantMap listComments(const QString& projectRootPath, const QString& scopeKind, const QString& targetID);
    Q_INVOKABLE QVariantMap listEdgesFor(const QString& projectRootPath, const QString& endpointID);
    Q_INVOKABLE QVariantMap listInbox(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap listObjectKinds();
    Q_INVOKABLE QVariantMap listObjects(const QString& projectRootPath, const QString& kindOrNull);
    Q_INVOKABLE QVariantMap listOrphanedObjects(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap listPendingEdges(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap listRelationTypes(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap listWorlds(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap openObject(const QString& projectRootPath, const QString& objectKind, const QString& objectID, const QString& worldID);
    Q_INVOKABLE QVariantMap promoteObject(const QString& projectRootPath, const QString& objectID, const QString& targetKind, const QString& worldIDOrNull);
    Q_INVOKABLE QVariantMap relinkWorld(const QString& projectRootPath, const QString& worldID, const QString& newPackagePath);
    Q_INVOKABLE QVariantMap removeAsset(const QString& projectRootPath, const QString& assetID, const QString& worldID, const QString& projectID);
    Q_INVOKABLE QVariantMap removeWorldReference(const QString& projectRootPath, const QString& worldID);
    Q_INVOKABLE QVariantMap resolveComment(const QString& projectRootPath, const QString& scopeKind, const QString& targetID, const QString& commentID, const QString& resolverDisplayName);
    Q_INVOKABLE QVariantMap resolveTimelineProjectTimes(const QString& projectRootPath, const QString& worldID, const QString& timelineID);
    Q_INVOKABLE QVariantMap saveObject(const QString& projectRootPath, const QString& objectKind, const QString& objectJson);
    Q_INVOKABLE QVariantMap scanForExternalChanges(const QString& projectRootPath, const QString& appSupportRoot, int includeGitStatus);
    Q_INVOKABLE QVariantMap setSceneOutline(const QString& projectRootPath, const QString& sceneID, const QString& outline);
    Q_INVOKABLE QVariantMap setSceneTags(const QString& projectRootPath, const QString& sceneID, const QString& tagsJson);
    Q_INVOKABLE QVariantMap setSceneTodo(const QString& projectRootPath, const QString& sceneID, const QString& todoJson);
    Q_INVOKABLE QVariantMap setTimelineEpochLabel(const QString& projectRootPath, const QString& label);
    Q_INVOKABLE QVariantMap setTimelineEpochOffset(const QString& projectRootPath, const QString& worldID, const QString& timelineID, long long epochOffsetMs);
    Q_INVOKABLE QVariantMap setWorldEpochOffset(const QString& projectRootPath, const QString& worldID, long long epochOffsetMs);
    Q_INVOKABLE QVariantMap upsertRelationType(const QString& projectRootPath, const QString& relationTypeJson);

    // Whether the MOST RECENT scrivi_* call through this bridge failed.
    //
    // ⚠️ An empty QVariantMap is AMBIGUOUS and always has been: parseEnvelope
    // returns {} for a failed call, and an ok envelope also decodes to {} when
    // its result carries no keys — which is exactly what every list endpoint
    // emits for an EMPTY list, because JsonDoc::appendToArray only creates the
    // array key when there is a first element to push. So `{}` from
    // listEdgesFor() means EITHER "this scene has no edges" OR "the call
    // failed", and the two are indistinguishable at the call site.
    //
    // Existing callers tolerate that by degrading silently (the timeline drops
    // to scenes-only on a failed historical-events read). ⚠️ A surface that must
    // tell "nothing here" apart from "we could not read it" — EP-035's T-0483,
    // where absence is never deletion — cannot. This flag is how it does.
    //
    // Set by every call that goes through parseEnvelope, so it must be read
    // IMMEDIATELY after the call it refers to.
    bool lastCallFailed() const { return lastCallFailed_; }

signals:
    void readyChanged();
    void errorOccurred(int code, const QString& message);

private:
    // Parse a scrivi_* envelope string. On ok, returns the "result" object.
    // On error, emits errorOccurred and returns an empty map.
    QVariantMap parseEnvelope(const QString& json);

    bool    ready_ = false;
    // See lastCallFailed(). Starts false: no call has failed before the first
    // call is made.
    bool    lastCallFailed_ = false;
    QString identityID_;
    QString personaID_;
    QString displayName_;
};
