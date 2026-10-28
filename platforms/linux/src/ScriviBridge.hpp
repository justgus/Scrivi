#pragma once

#include <QObject>

#include <functional>
#include <QString>
#include <QVariantMap>
#include <QStringList>
#include <QHash>
#include <QJsonObject>
#include <memory>
#include <optional>
#include <utility>
#include <qqmlintegration.h>

#include "MarkdownAnalysis.hpp"

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

    // ✅ [I-0309] What a bridge carries between calls: only who is calling. A worker builds its OWN bridge from a copy of
    // this (`WorkerBridge`), so it never borrows one a window owns (closing the window freed it under a running load).
    struct Identity {
        bool ready = false;
        QString identityID;
        QString personaID;
        QString displayName;
    };
    Identity identity() const { return {ready_, identityID_, personaID_, displayName_}; }
    explicit ScriviBridge(const Identity& identity, QObject* parent = nullptr);

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

    // SP-144 / [I-0232] — the SAME call, OFF THE UI THREAD.
    //
    // ⚠️ WHY THIS EXISTS. `openProject` above is blocking, and `Landing.qml`
    // called it directly. MEASURED on the rig 2026-09-18 (build 43, `cache=none`
    // CIFS, sampling `/proc/<pid>` every 2 s): 155 s in state `D` on ONE thread
    // — the launch screen frozen and silent, "Scrivi is not Responding" every
    // 5 s — BEFORE [I-0195]'s progress bar could appear, because the chain
    // `openPath → openEditor → showEditor → EditorShell::load` only goes async
    // AFTER this call has already returned.
    //
    // ✅ [I-0195] built the machinery and converted `EditorShell::load`.
    // ⚠️ [I-0232] IS NOT A MISSING MECHANISM; IT IS AN UNCONVERTED CALL SITE.
    //
    // Emits EXACTLY ONE of:
    //   • projectOpened(result)  — the same map `openProject` returns
    //   • projectOpenFailed()    — `errorOccurred` has already fired with detail
    //   • projectOpenTimedOut()  — the budget elapsed; the worker is still parked
    //
    // ⚠️ THE BUDGET IS `EditorShell`'s 10 MINUTES, NOT `AsyncCall`'s 5 s DEFAULT.
    // 5 s is tuned to abort a DEAD share ([I-0193]); applying it here would abort
    // the legitimate slow load that [I-0195] exists to support. A 371 s honest
    // open is MEASURED, so the budget must sit well above it.
    Q_INVOKABLE void openProjectAsync(const QString& projectRootPath,
                                      const QString& appSupportRoot);

    // Releases the core's in-memory state for a project (EP-039 T-0512) — today the
    // ProjectIndex that accelerates scene lookup.
    //
    // ⚠️ WITHOUT THIS THE CORE LEAKS ONE INDEX PER PROJECT OPENED: nothing else
    // removes a registry entry, and this app can close a project and open another in
    // the same process ("Close Project" → landing → Open).
    // ✅ Closing a project the core never indexed is a NO-OP, not an error, so this is
    // safe to call unconditionally on a teardown path. Never writes to the project.
    Q_INVOKABLE void closeProject(const QString& projectRootPath);

    // Loads a single scene's body (SP-061 / T-0235). Calls scrivi_open_scene and
    // returns its ok "result": {scene{sceneID,chapterID,title,slug,metadataPath,
    // contentPath}, markdown}. Used by the editor shell to fetch the bodies of the
    // non-active scenes when assembling the continuous viewport (the active scene's
    // body already comes back inside openProject). On failure emits errorOccurred
    // and returns {}. Requires bootstrap() to have succeeded first.
    // SP-144 — `openScene` for the viewport's BULK LOAD. Identical, except it
    // does NOT record the scene as the last writing surface.
    //
    // ⚠️ `EditorShell::load` calls this once per scene to assemble the continuous
    // editor. Through plain `openScene` that was 61 atomic read-modify-writes of
    // `workspace-state.json` for a 61-scene project — MEASURED as 62 opens + 61
    // `.tmp` opens + 61 renames of a 1,662-syscall load, while every manuscript
    // sidecar was opened just 3 times. ⚠️ Each write is a temp+write+rename
    // round-trip on a network volume, which is why a SMALL project loaded slowly.
    //
    // ⛔ Use plain `openScene` for a scene the writer NAVIGATED TO — that call is
    // what records where they are. ✅ The restore is unaffected either way.
    Q_INVOKABLE QVariantMap openSceneForBulkLoad(const QString& projectRootPath,
                                                 const QString& appSupportRoot,
                                                 const QString& projectID,
                                                 const QString& sceneID);

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
    // EP-052 ([SP-195]): queues scene saves — each `scenes` entry {sceneID, metadataPath, contentPath, markdown,
    // selectionAnchor, selectionFocus, scroll}. A memory call: it returns at once with {tickets: [{sceneID, ticket}],
    // ticket}. The core tells history the bytes it wrote.
    QVariantMap enqueueSaveScenes(const QString& projectID, const QString& projectRootPath, const QString& appSupportRoot,
                                  const QVariantList& scenes);
    // EP-052: waits (OFF the GUI thread) for the queued writes: {landed, held, heldReason?, pending, outcomes?}. A HELD
    // queue ([SP-195] Q1 (a)) answers at once.
    QVariantMap waitForWrites(const QString& projectRootPath, qint64 ticket, qint64 timeoutMs);
    // EP-052 [SP-196] W7a: `rangePaste` / `rangeCut` / `rangeRelabel` QUEUED (`scrivi_enqueue_range_op`) — run, and checked,
    // when they reach the head of the queue (W7c). A memory call: returns the job's ticket (0 on failure, with
    // lastCallFailed()); its result, the delta included, comes back in its outcome from `waitForWrites`.
    qint64 enqueueRangePaste(const QString& projectRootPath, const QString& rangeJson, const QString& caretSceneID,
                             long long caretByteOffset, const QString& caretSceneText, const QString& displayName);
    qint64 enqueueRangeCut(const QString& projectRootPath, const QString& rangeJson, const QString& displayName);
    qint64 enqueueRangeRelabel(const QString& projectRootPath, const QString& rangeJson, const QString& targetChapterID,
                               const QString& displayName);

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

    // ⛔ [SP-191] `mergeScene` / `mergeChapter` RETIRED with `scrivi_merge_scene` / `scrivi_merge_chapter`: a scene merge is
    // `rangeCut` of the zero-length range across the break, a chapter merge `rangeRelabel`.

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

    // SP-173 / I-0285 (D3): all five timeline reads in one call, under one project lock; worker-safe. See the .cpp.
    QVariantMap loadTimeline(const QString& projectRootPath);
    // ✅ [SP-191]: every scene's EXPLICIT story time, one call, worker-safe — all a structural edit changes on the timeline
    // (Apple: `listStoryTimes`). Returns {storyTimes:[…]}; on failure emits errorOccurred, returns {}.
    QVariantMap listStoryTimes(const QString& projectRootPath);
    // ✅ SP-197 (EP-052 S3): everything the Scene Inspector shows for one scene, one call under one project lock; worker-safe.
    // Result: {notes, edges, objects, worlds[, citations]}, a failing part replaced by "<part>Error"; lastRevision() is the
    // state they describe.
    QVariantMap loadSceneInspector(const QString& projectRootPath, const QString& sceneID, bool includeCitations);
    // SP-173 (D4): the project's current revision (-1 on failure); touches no file.
    qint64 projectRevision(const QString& projectRootPath);
    // SP-173 (fix 3): many scenes' texts in one call, read in parallel by the core; sceneID → markdown. Worker-safe.
    QHash<QString, QString> readSceneTexts(const QString& projectRootPath, const QStringList& sceneIDs);

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

    // Inspector layout (EP-041 / SP-142, T-0537) — `inspector-layout.json`, owned by
    // ScriviCore since [SP-141].
    //
    // ⚠️ getInspectorLayout ALWAYS SUCCEEDS and reports what it found:
    //   status "ok"         → `documentJSON` holds the document
    //   status "absent"     → no file. ⚠️ NORMAL — the app applies its own defaults.
    //   status "unreadable" → corrupt. ⛔ The core did NOT overwrite it; `message` says why.
    // ⛔ Do not collapse `absent` and `unreadable`: the app defaults for both but can
    // only WARN about the second.
    //
    // ⚠️ The document is OPAQUE across the boundary — the core never interprets it, so
    // keys this build does not understand survive BY CONSTRUCTION ([I-0215]).
    Q_INVOKABLE QVariantMap getInspectorLayout(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap putInspectorLayout(const QString& projectRootPath,
                                               const QString& documentJson);

    // EP-047 AC1/AC2 (SP-167) — settings that TRAVEL with the project ([I-0278]): the inspector
    // layout's opaque contract for `project-settings.json` (status ok | absent | unreadable; PUT
    // replaces the whole document — read, merge your keys, write), and the project rename, which
    // writes `project.json`'s `title`. ⚠️ No Linux UI reads them yet (EP-048 L10).
    Q_INVOKABLE QVariantMap getProjectSettings(const QString& projectRootPath);
    Q_INVOKABLE QVariantMap putProjectSettings(const QString& projectRootPath, const QString& documentJson);
    Q_INVOKABLE QVariantMap setProjectTitle(const QString& projectRootPath, const QString& title);

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
    // ✅ **The undo history is bound by [EP-026]** (its Linux story, ruled 2026-10-11) — Sprint by Sprint, never ahead
    // of its surface (the lesson of I-0144): S2 ([SP-178]) binds the history lifecycle and the 12 `scrivi_capture_*`
    // below (`historyOpen` … `captureState`). ⛔ Still absent: `scrivi_history_get_tree`, `_list_stale_branches`,
    // `_purge_branch`, `_select_branch`, `_get_settings`/`_set_settings` (S3). ✅ S5 ([SP-179]) binds the four
    // `scrivi_buffers_*` (`buffersLoad` … `buffersClear`, below).

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
    // EP-053 S1 endpoints, bound in S2a (SP-187 / T-0615). Offsets are scene-local UTF-8 bytes. `rangeJson` is a
    // scrivi.range.v1 object. Paste and cut answer { delta, range } — decode the delta with `RangeDelta::fromVariant`.
    Q_INVOKABLE QVariantMap rangeRead(const QString& projectRootPath, const QString& startSceneID, long long startByte,
                                      const QString& endSceneID, long long endByte);
    Q_INVOKABLE QVariantMap rangePaste(const QString& projectRootPath, const QString& rangeJson, const QString& caretSceneID,
                                       long long caretByteOffset, const QString& caretSceneText, const QString& displayName);
    Q_INVOKABLE QVariantMap rangeCut(const QString& projectRootPath, const QString& rangeJson, const QString& displayName);
    // [SP-191] Q3e: a chapter merge, and its undo — the run of whole scenes in `rangeJson` moves into `targetChapterID` (the
    // chapter before, after, or a new one; "" = fresh). Answers { delta, range, fromChapterID }.
    Q_INVOKABLE QVariantMap rangeRelabel(const QString& projectRootPath, const QString& rangeJson,
                                         const QString& targetChapterID, const QString& displayName);
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

    // EP-048 L3 (SP-166) — `scrivi_analyze_markdown`: what one Markdown block is, in UTF-16 offsets.
    // ✅ STATIC: the endpoint is stateless (no project), and the manuscript presenter calls it per block
    // without a bridge instance. A failed call returns an empty analysis — the block is drawn as stored.
    static MarkdownAnalysis analyzeMarkdown(const QString& block);

    // SP-175 / T-0602 (EP-048 S3b) — the Format commands and balanced edits (`scrivi_markdown_*`, built in SP-174): pure
    // functions on ONE SCENE's text, offsets in UTF-16 units (QString indices). ✅ STATIC, as `analyzeMarkdown`. A refusal —
    // or a failed call — is `std::nullopt` (the caller beeps, or applies the plain edit).
    static std::optional<MarkdownRange> markdownWordRange(const QString& scene, int caret);
    static std::optional<MarkdownEdit> markdownEmphasisEdit(const QString& scene, int start, int end, int bit, int caret = -1);
    // `format`: body | heading1 | heading2 | heading3 | bulletList | numberedList.
    static std::optional<MarkdownEdit> markdownParagraphEdit(const QString& scene, int start, int end, const QString& format);
    static std::optional<MarkdownEdit> markdownListReturnEdit(const QString& scene, int caret);
    // The balanced form of replacing [start, end) by `replacement`; nullopt = the plain edit is already balanced.
    // The result's `selection` is the caret after it (empty).
    static std::optional<MarkdownEdit> markdownBalancedEdit(const QString& scene, int start, int end,
                                                            const QString& replacement, bool keepsOwnFormatting);
    // {source, presented} of a copy of [start, end).
    static std::pair<QString, QString> markdownBalancedCopy(const QString& scene, int start, int end);
    // EP-026 S4 (SP-180): a cross-scene paste's balancing (Apple: `styleAtCaret` / `balancePastePieces`). The style typing
    // at `caret` would take (bits: 1 italic, 2 bold); 0 when the call fails.
    static int markdownStyleAtCaret(const QString& scene, int caret);
    // The pieces' texts balanced inside a span of `caretStyle` — the first piece starts with it open, the last re-opens it.
    // Unchanged when the call fails.
    static QStringList markdownBalancePastePieces(const QStringList& pieces, int caretStyle);

    // ── EP-026 S2 (SP-178 / T-0605): the undo history and its capture rules (`scrivi_history_*`, `scrivi_capture_*`) ──
    // ✅ STATIC, as the markdown calls: no identity is threaded, and the load worker calls `historyOpen` /
    // `historyValidateScene` off the UI thread. ✅ BEST-EFFORT, as Apple's `try?` (`HistoryCapture.swift`): a failure is
    // logged and returns an empty map — ⛔ never `errorOccurred`, so a history fault can never interrupt writing.
    // Carets are UTF-16 units of the scene text (SP-177 Q2) — `QString` indices. `nowMs` is a monotonic clock (SP-177 Q1).
    static QVariantMap historyOpen(const QString& projectRootPath);
    static QVariantMap historyValidateScene(const QString& projectRootPath, const QString& sceneID, const QString& diskText);
    static QVariantMap historyNoteScenePersisted(const QString& projectRootPath, const QString& sceneID,
                                                 const QString& diskText);
    static QVariantMap historyClose(const QString& projectRootPath);
    static QVariantMap captureNoteEdit(const QString& projectRootPath, const QString& sceneID, const QString& text,
                                       int caretUtf16, const QString& textBefore, qint64 nowMs);
    static QVariantMap captureCaretMoved(const QString& projectRootPath, const QString& sceneID, bool editInFlight,
                                         bool applying);
    static QVariantMap captureBegin(const QString& projectRootPath, const QString& kind, const QString& bufferID = {});
    static QVariantMap captureFlush(const QString& projectRootPath, const QString& trigger,
                                    const QString& kind = QStringLiteral("typing"));
    static QVariantMap captureBeforeSave(const QString& projectRootPath, qint64 nowMs);
    static QVariantMap captureIdle(const QString& projectRootPath, qint64 nowMs);
    // `editsJson`: [{sceneID, textBefore, textAfter, caret}] — one undo step across several scenes ([I-0270]).
    static QVariantMap captureGroupedEdit(const QString& projectRootPath, const QString& editsJson, const QString& kind);
    // `paramsJson`: {barrierKind, note, sceneID?} (as `scrivi_history_record_barrier`).
    static QVariantMap captureRecordBarrier(const QString& projectRootPath, const QString& paramsJson);
    static QVariantMap captureUndo(const QString& projectRootPath);
    static QVariantMap captureRedo(const QString& projectRootPath);
    static QVariantMap captureNoteApplied(const QString& projectRootPath, const QString& sceneID, const QString& text);
    static QVariantMap captureState(const QString& projectRootPath);

    // ── EP-026 S5 (SP-179 / T-0606): the copy buffers 1–9 (`scrivi_buffers_*`, `history/buffers.json`) ──
    // ✅ Static and best-effort, as the capture calls above: a failure is logged and returns {} (⛔ never `errorOccurred`).
    // Stateless in the core — no open / close. A bufferID outside "1"–"9" fails. Text is STORED text (escapes included).
    // `rangeJson` "" = a plain-text slot; a cross-scene copy stores its `scrivi.range.v1` too ([SP-191] Q4a — anything
    // else is refused, and a slot written before that reads as empty).
    static QVariantMap buffersLoad(const QString& projectRootPath, const QString& bufferID, const QString& text,
                                   const QString& rangeJson = {});
    // {bufferID, text, updatedAt, present[, range]}
    static QVariantMap buffersGet(const QString& projectRootPath, const QString& bufferID);
    // {count, buffers: [{bufferID, text, updatedAt[, range]}]} — ⚠️ `buffers` is ABSENT for an empty list.
    static QVariantMap buffersList(const QString& projectRootPath);
    // {bufferID, cleared}
    static QVariantMap buffersClear(const QString& projectRootPath, const QString& bufferID);

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
    //
    // ✅ SP-173 (D5, Scrivi_Core_Concurrency_Design_v0_1.md): PER THREAD, not per bridge. Background reads (SceneInspector,
    // WorldsDialog, the project load) call through the same bridge as the main thread; a shared flag let one thread's call
    // overwrite the other's answer between its call and its read (a data race, and a wrong answer). Each thread now sees
    // only its own last call.
    bool lastCallFailed() const;
    // SP-173 (D2): the project revision THIS thread's last call reported, or -1. Read immediately after the call.
    qint64 lastRevision() const;
    // SP-173 (Q1): the project-lock waits the core has reported in this process (each ≥ 1 ms), across all threads.
    struct LockWaitStats { qint64 totalMs = 0; qint64 count = 0; qint64 maxMs = 0; };
    static LockWaitStats lockWaitStats();
    // EP-053 S2a (SP-187): core calls made ON THE GUI THREAD, process-wide — the baseline S2b removes. Read by smokes.
    static qint64 guiThreadCallCount();
    // [SP-188] AC11 / EP-052 inventory: each GUI-thread project call as the core traced it (endpoint, time after the lock,
    // file operations) and its lock wait. The app logs them (`main.cpp`); smokes tabulate them. Set on the GUI thread only.
    struct GuiCoreCall { QString endpoint; qint64 ms = 0; qint64 fileOps = 0; qint64 lockWaitMs = 0; };
    static std::function<void(const GuiCoreCall&)>& guiCoreCallObserver();
    // [SP-197] rig: the same for every OTHER thread's call, so a log shows which background read held the project's lock.
    // Set once, before any worker runs; called on the worker.
    static std::function<void(const GuiCoreCall&)>& workerCoreCallObserver();

signals:
    void readyChanged();
    void errorOccurred(int code, const QString& message);

    // SP-144 / [I-0232] — outcomes of `openProjectAsync`. Exactly one fires.
    void projectOpened(const QVariantMap& result);
    void projectOpenFailed();
    void projectOpenTimedOut();

private:
    // Parse a scrivi_* envelope string. On ok, returns the "result" object.
    // On error, emits errorOccurred and returns an empty map.
    QVariantMap parseEnvelope(const QString& json);
    // [SP-196] `op` + the range, the identity and `displayName` → `scrivi_enqueue_range_op`; the ticket, 0 on failure.
    qint64 enqueueRangeOp(const QString& projectRootPath, QJsonObject op, const QString& rangeJson,
                          const QString& displayName);

    bool    ready_ = false;
    QString identityID_;
    QString personaID_;
    QString displayName_;
};

// ✅ [I-0309] A worker's link back to the GUI: its bridge's errors (and a load's progress) cross through this, never through
// an object the window owns. Owned jointly by the window and the worker (`WorkerBridge::Handle`), deleted on the GUI thread
// (`deleteLater`) when the last of them lets go. If the window closes first, Qt drops its connections, and the worker's
// emits go nowhere — harmlessly.
class ScriviBridgeRelay : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

signals:
    void errorOccurred(int code, const QString& message);
    void progress(int done, int total, int scenes);   // a load's (EditorShell::loadProgress)
};

// ✅ [I-0309] THE ONLY WAY A WORKER CALLS THE CORE. Build the handle on the GUI thread (`WorkerBridge::handleFor`), capture
// it BY VALUE, and build the `WorkerBridge` INSIDE the worker: it lives and dies on the worker thread, and the bridge's
// failure flag is per thread anyway (SP-173 D5). Errors reach the source bridge's `errorOccurred` listeners exactly as
// before (queued), while the source exists.
class WorkerBridge
{
public:
    struct Handle {
        ScriviBridge::Identity identity;
        std::shared_ptr<ScriviBridgeRelay> relay;
    };
    // GUI thread. `source` is the bridge whose `errorOccurred` the worker's errors are forwarded to.
    static Handle handleFor(ScriviBridge* source);

    explicit WorkerBridge(const Handle& handle);
    ScriviBridge* operator->() { return &bridge_; }
    ScriviBridge& operator*() { return bridge_; }
    ScriviBridgeRelay* relay() const { return relay_.get(); }

private:
    std::shared_ptr<ScriviBridgeRelay> relay_;
    ScriviBridge bridge_;
};
