import Foundation

// Which end of a scene or chapter a navigation command targets.
enum ManuscriptEdge {
    case start
    case end
}

// A scene segment held in memory by the viewport loader.
struct SceneSegment: Identifiable {
    let id: String          // == sceneID
    let sceneID: String
    let chapterID: String
    let metadataPath: String
    let contentPath: String
    var text: String        // mutable — edited by the author
    var isDirty: Bool = false
}

// ViewportSceneLoader owns the ordered array of loaded SceneSegments.
//
// All scenes are loaded into memory at once on project open (all-in-memory model).
// The dynamic fill/release cycle is not used.
//
// Thread-safety: all mutations happen on @MainActor via AppEnvironment.
@Observable @MainActor final class ViewportSceneLoader {

    // All scene segments in manuscript order.
    private(set) var segments: [SceneSegment] = []

    // Index into `segments` of the scene the cursor is currently in.
    // @ObservationIgnored so cursor movement does not trigger SwiftUI view updates.
    @ObservationIgnored private(set) var currentIndex: Int = 0

    private var engine: ScriviEngine
    private var projectRootPath: String
    private var appSupportRoot: String
    private(set) var projectID: String

    // The project's history capture, when open (I-0104). Set by ProjectSession after
    // both objects exist. `weak` because HistoryCapture is owned by the session, which
    // also owns this loader — a strong reference here would be a retain cycle.
    // @ObservationIgnored: wiring, not view state.
    @ObservationIgnored weak var historyCapture: HistoryCapture?

    // ⚠️ [T-0570] — called when a scene write FAILS, so the session can find out WHY
    // (the project's drive gone) and warn the writer. Wiring, not view state.
    @ObservationIgnored var onSaveFailed: (() -> Void)?

    // ✅ [I-0303]: how many scenes a bulk save is writing OFF the main actor right now. Above zero, the manuscript is locked
    // and the editor shows progress (Replace All).
    private(set) var scenesWritingOffMain = 0
    /// [I-0305] — run once no off-main write is left (`whenOffMainWritesLand`).
    @ObservationIgnored private var offMainWaiters: [@MainActor () -> Void] = []

    // Tests ([I-0303]): told of every scene write that succeeded — its sceneID and whether it ran on the main thread.
    @ObservationIgnored var onSceneWritten: (@Sendable (_ sceneID: String, _ onMainThread: Bool) -> Void)?

    // [I-0303]: the scenes an off-main write holds, and those a main-actor save found in flight. ⚠️ A main save of a scene in
    // flight is DEFERRED until the flight lands, so an older write can never land over a newer one.
    @ObservationIgnored private var inFlight: Set<String> = []
    @ObservationIgnored private var saveAfterFlight: Set<String> = []
    // Left when an off-main write's last file is written (on the worker, not after the main-actor hop), so the quit path
    // can wait for it without needing the main thread.
    @ObservationIgnored private let offMainWrites = DispatchGroup()

    // Full ordered scene list. Used by SceneNavigatorView to build the sidebar.
    private(set) var allScenes: [SceneInfo]

    // Live first-line titles keyed by sceneID.
    private(set) var liveTitles: [String: String] = [:]

    // Manuscript-coordinate start position for each scene (separator chars excluded).
    // Built by rebuildSceneStartMap(); updated after every structural change.
    private(set) var sceneStartMap: [String: Int] = [:]

    // Storage-coordinate start offset for each scene (separator chars included).
    // Used by ManuscriptTextView to place the cursor after navigate/delete.
    private(set) var sceneStorageOffsetMap: [String: Int] = [:]

    // The sceneID the cursor (insertion point) is currently in.
    // @ObservationIgnored — must not trigger SwiftUI view updates on every keystroke.
    @ObservationIgnored private(set) var cursorSceneID: String?

    // The sceneID currently visible in the viewport — drives the Navigator highlight.
    // Updates on scroll (debounced) and on cursor movement.
    // Only this property is observable by SwiftUI.
    private(set) var viewportSceneID: String?

    // Current cursor position in manuscript coordinates (separator chars excluded).
    // @ObservationIgnored so cursor movement does not trigger SwiftUI view updates.
    @ObservationIgnored private(set) var manuscriptCursorPosition: Int = 0

    // Scene-local cursor offset within the current scene (I-0058). Persisted on
    // save as WorkspaceState selection so the cursor is restored on next open.
    @ObservationIgnored private(set) var currentSceneCursorOffset: Int = 0

    // Document scroll fraction (0.0–1.0), updated on scroll (I-0058). Persisted on
    // save as WorkspaceState scroll so scroll position is restored on next open.
    @ObservationIgnored private(set) var scrollFraction: Double = 0

    // Restored writing surface from the last session (I-0058), consumed once by the
    // editor on first appear to place the cursor. nil after consumption.
    //
    // ⚠️ No scroll fraction here (I-0133, ruled 2026-08-18): the Apple layer centres the
    // restored scene instead, so a document-wide fraction has nothing to apply to. The
    // field still exists in the schema and open envelope — `[Linux]` consumes it — it is
    // simply not carried into this layer.
    @ObservationIgnored var restoredSelectionOffset: Int?

    // Set by ManuscriptTextView.Coordinator.makeNSView; used to forward takeFocus calls.
    var takeFocusHandler: (() -> Void)?

    func takeFocus() { takeFocusHandler?() }

    // ⚠️ **Echo suppression for the navigator↔manuscript cycle (I-0132).**
    //
    // The two views form a loop by design: the manuscript scrolls → `setViewportScene` →
    // the navigator mirrors it into its `selection` → `EditorView.onChange(selection)` →
    // navigate the manuscript. Left open, a scroll would re-scroll itself and yank focus
    // mid-gesture.
    //
    // This is set while the loader is pushing a viewport change OUT to the navigator, and
    // marks any selection write that arrives during that window as an **echo** rather than
    // a writer's choice. Suspending the notification is the fix; comparing values is not —
    // an equality guard silently depends on which view happens to write first, and would
    // start looping the day that order changed.
    @ObservationIgnored private(set) var isMirroringViewportToSelection = false

    // Called by the Coordinator when the visible scene changes (scroll or cursor movement).
    // This is the only path that updates the Navigator highlight.
    func setViewportScene(_ sceneID: String?) {
        if viewportSceneID != sceneID {
            NSLog("[SCRIVI-DIAG] setViewportScene: \(viewportSceneID ?? "nil") -> \(sceneID ?? "nil")")
        }
        // The flag must be raised BEFORE the observable write, because the navigator's
        // `onChange` (and the selection write it makes) run synchronously off it.
        isMirroringViewportToSelection = true
        viewportSceneID = sceneID
        // Lower it after the resulting view update has drained, so an echo that arrives on
        // the next runloop pass is still recognised as one.
        DispatchQueue.main.async { [weak self] in
            self?.isMirroringViewportToSelection = false
        }
    }

    init(
        engine: ScriviEngine,
        projectRootPath: String,
        appSupportRoot: String,
        projectID: String,
        allScenes: [SceneInfo]
    ) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        self.appSupportRoot = appSupportRoot
        self.projectID = projectID
        self.allScenes = allScenes
    }

    // Load all scenes into memory at once. Called once on project open.
    //
    // `activeSceneID` (I-0058) is the scene the writer last edited, as returned by the
    // backend openProject flow. When present and still in the manuscript, the loader
    // resumes there: currentIndex/cursorSceneID/viewportSceneID all point at it, so the
    // Navigator highlights it and the editor scrolls to it on first appear. When nil or
    // stale, behaviour is unchanged — resume at the first scene, no pre-selection.
    //
    // `restoredSelection` is the scene-local cursor offset from the last session; the
    // editor consumes it once on appear to place the cursor. The envelope's
    // `restored.scroll` is deliberately not taken (I-0133) — see `restoredSelectionOffset`.
    /// T-0523 / AC6 — the off-main-thread half of `loadAll`.
    ///
    /// ⚠️ **WHY THIS IS A `nonisolated static` FUNCTION AND NOT A METHOD.**
    /// `ViewportSceneLoader` is `@MainActor` and `@Observable`: touching `self`
    /// from a worker would either fail to compile or (worse, the [I-0198] shape)
    /// mutate observable state off the UI thread. So the expensive part takes only
    /// `Sendable` inputs, returns plain values, and touches NOTHING on the loader.
    /// The caller publishes the result on the main actor in one assignment.
    ///
    /// ⚠️ `ScriviEngine` is `@unchecked Sendable` and each `openScene` is an
    /// independent C ABI read, which is what makes this legal at all.
    ///
    /// `onProgress` is called from the WORKER — it must not touch UI directly.
    nonisolated static func loadSegmentsOffMain(
        engine: ScriviEngine,
        projectRootPath: String,
        appSupportRoot: String,
        projectID: String,
        allScenes: [SceneInfo],
        onProgress: @Sendable (Int, Int) -> Void
    ) -> (segments: [SceneSegment], liveTitles: [String: String]) {
        var segments: [SceneSegment] = []
        var liveTitles: [String: String] = [:]
        segments.reserveCapacity(allScenes.count)

        // ✅ SP-173 / I-0285 (fix 3): the texts in CHUNKS through `readSceneTexts`, which reads each chunk IN PARALLEL inside
        // the core (one `openSceneForBulkLoad` per scene was one network round trip after another on a share). Read-only,
        // like the bulk variant before it (SP-144: no workspace-state write). Progress is reported per CHUNK, so the bar
        // still moves on a slow mount.
        let chunk = 64
        var start = 0
        while start < allScenes.count {
            let slice = allScenes[start ..< min(start + chunk, allScenes.count)]
            let read = try? engine.readSceneTexts(projectRootPath: projectRootPath, sceneIDs: slice.map(\.sceneID))
            var textByID: [String: String] = [:]
            for s in read?.scenes ?? [] { textByID[s.sceneID] = s.markdown }
            for info in slice {
                let text = textByID[info.sceneID] ?? ""
                segments.append(SceneSegment(
                    id: info.sceneID,
                    sceneID: info.sceneID,
                    chapterID: info.chapterID,
                    metadataPath: info.metadataPath,
                    contentPath: info.contentPath,
                    text: text
                ))
                let firstLine = text.components(separatedBy: .newlines)
                    .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
                if !firstLine.isEmpty { liveTitles[info.sceneID] = firstLine }
            }
            start += slice.count
            onProgress(start, allScenes.count)
        }
        return (segments, liveTitles)
    }

    /// T-0523 — publishes a set of segments computed off-main, then finishes the
    /// same bookkeeping `loadAll` does. ⚠️ Everything here is main-actor work and
    /// is deliberately kept SMALL: it is the part the writer waits through after
    /// the bar reaches 100%.
    func adoptLoadedSegments(
        _ loaded: (segments: [SceneSegment], liveTitles: [String: String]),
        activeSceneID: String? = nil,
        restoredSelection: Int? = nil
    ) {
        segments = loaded.segments
        liveTitles = loaded.liveTitles
        rebuildSceneStartMap()
        resolveActiveScene(activeSceneID: activeSceneID, restoredSelection: restoredSelection)
    }

    /// Shared by `loadAll` and `adoptLoadedSegments` so the resume-at-last-scene
    /// rule cannot drift between the sync and async paths.
    private func resolveActiveScene(activeSceneID: String?, restoredSelection: Int?) {
        NSLog("[SCRIVI-DIAG] loadAll: activeSceneID=\(activeSceneID ?? "nil") restoredSel=\(String(describing: restoredSelection)) segments=\(segments.count)")
        if let activeSceneID,
           let idx = segments.firstIndex(where: { $0.sceneID == activeSceneID }) {
            NSLog("[SCRIVI-DIAG] loadAll: RESOLVED activeScene to index \(idx)")
            currentIndex = idx
            cursorSceneID = activeSceneID
            viewportSceneID = activeSceneID
            restoredSelectionOffset = restoredSelection
        } else {
            currentIndex = 0
            if let firstID = segments.first?.sceneID {
                cursorSceneID = firstID
                // viewportSceneID intentionally left nil — no scene is pre-selected
                // in the Navigator on load. The scroll observer sets it on first scroll.
            }
        }
    }

    func loadAll(
        activeSceneID: String? = nil,
        restoredSelection: Int? = nil
    ) {
        // ⚠️ [I-0196] instrumentation. This loop is ONE `scrivi_open_scene` PER
        // SCENE, and the tick below reports the RATE, not just the total —
        // a rising avg is what proves the cost is quadratic, and a total alone
        // is what made the first (wrong) diagnosis possible.
        ScriviDiag.reset()

        // ⚠️ AC9 / T-0524 — NOTIFICATION BATCHING.
        //
        // This loop called `loadScene`, which writes BOTH `segments` and `liveTitles`
        // — two `@Observable` properties — ONCE PER SCENE. On the 1,174-scene fixture
        // that is ~2,348 observation notifications during a single open, which is the
        // measured `1135.66 notifications/second` and the `harmful notification post
        // rate` warning.
        //
        // ✅ Accumulate into LOCALS, then assign ONCE. Two notifications for the whole
        // load instead of two per scene — so volume no longer scales with scene count,
        // which is the AC's actual requirement (the AC is the BATCHING, not the number).
        //
        // ⚠️ `segments` is `private(set)` and observation fires on the SETTER, so the
        // local array must be built and assigned wholesale; appending to `self.segments`
        // in the loop is what produced the storm.
        var builtSegments: [SceneSegment] = []
        builtSegments.reserveCapacity(allScenes.count)
        var builtTitles: [String: String] = [:]

        ScriviDiag.measure("loadAll: scene loop") {
            for i in allScenes.indices {
                buildScene(at: i, into: &builtSegments, titles: &builtTitles)
                ScriviDiag.tick("scenes loaded", i, of: allScenes.count)
            }
        }

        segments = builtSegments
        // ⚠️ MERGE, do not replace. The original loop wrote `liveTitles[id] = firstLine`
        // per scene and never cleared the dictionary, so a title already present for a
        // scene this pass derives no first line for SURVIVED. Both of today's callers
        // happen to enter with it empty (`replaceScenes` clears it at :289; a fresh open
        // starts empty), which makes replace and merge equivalent RIGHT NOW — but that is
        // incidental, and a future caller would silently lose titles. Keep the original
        // semantics: one notification, same result.
        liveTitles.merge(builtTitles) { _, new in new }
        ScriviDiag.measure("loadAll: rebuildSceneStartMap") {
            rebuildSceneStartMap()
        }

        // Resume at the last-edited scene when the backend supplied one and it still exists.
        resolveActiveScene(activeSceneID: activeSceneID, restoredSelection: restoredSelection)
    }

    var currentSegment: SceneSegment? {
        guard segments.indices.contains(currentIndex) else { return nil }
        return segments[currentIndex]
    }

    // Scene-local UTF-8 byte offset of the caret within `cursorSceneID`. Observed (not
    // @ObservationIgnored) so the history card re-highlights as the caret moves.
    // -1 when unknown.
    private(set) var cursorByteOffset: Int = -1

    // Called by ManuscriptTextView on every caret move, so the history card can bold the
    // entry whose change the caret is sitting inside (user request, 2026-08-06).
    func setCursorByteOffset(_ offset: Int, sceneID: String) {
        if cursorSceneID != sceneID {
            NSLog("[SCRIVI-DIAG] cursorScene: \(cursorSceneID ?? "nil") -> \(sceneID)")
        }
        cursorSceneID = sceneID
        cursorByteOffset = offset
    }

    // Called by ManuscriptTextView when the author's cursor moves into a different segment.
    func setCurrentIndex(_ index: Int) {
        if index != currentIndex { NSLog("[SCRIVI-DIAG] setCurrentIndex: \(currentIndex) -> \(index)") }
        currentIndex = index
        if segments.indices.contains(index) {
            cursorSceneID = segments[index].sceneID
        }
    }

    // ⚠️ [I-0259] — a caller whose OWN write failed hands the scene back as dirty, so
    // the next save retries it instead of the edit living only in memory.
    func markDirty(at index: Int) {
        guard segments.indices.contains(index) else { return }
        segments[index].isDirty = true
    }

    // Called by ManuscriptTextView when the author edits text in a segment.
    func updateText(_ text: String, at index: Int) {
        guard segments.indices.contains(index) else { return }
        segments[index].text = text
        segments[index].isDirty = true
    }

    /// T-0571 — a navigator click made while searching: the manuscript puts the caret at the
    /// query's first match in that scene instead of the scene start. Read by
    /// `navigateToScene`; ⚠️ NOT observed, so writing it never re-renders anything.
    struct SearchCaretHint: Equatable {
        let sceneID: String
        let query: String
    }
    @ObservationIgnored var searchCaretHint: SearchCaretHint? = nil

    // Update the live navigator title for a scene (debounced by caller).
    func updateLiveTitle(_ title: String, forSceneID sceneID: String) {
        liveTitles[sceneID] = title
    }

    // Save the segment at `index` without removing it from memory.
    //
    // Only the *current* scene's save carries the live cursor offset + scroll fraction
    // (I-0058); the backend stamps WorkspaceState.lastWritingSurface on every save, so
    // the current scene must be the last write for resume to land on it (see saveAllDirty).
    func saveScene(at index: Int, engine: ScriviEngine, ref: AuthorshipRef) async {
        saveSceneBlocking(at: index, engine: engine, ref: ref)
    }

    // The actual write. Synchronous because every engine call it makes is a synchronous
    // C ABI call — there is nothing to await. Having it callable without `await` is what
    // lets `applicationWillTerminate` flush safely (I-0104/I-0108); that callback does
    // not wait for async work, so a Task would be cut off mid-write.
    func saveSceneBlocking(at index: Int, engine: ScriviEngine, ref: AuthorshipRef) {
        guard segments.indices.contains(index), segments[index].isDirty else { return }
        let seg = segments[index]
        guard !inFlight.contains(seg.sceneID) else {
            saveAfterFlight.insert(seg.sceneID)   // [I-0303]: written once the off-main write of it lands
            return
        }
        let isCurrent = index == currentIndex
        // ⛔ [I-0259] — A FAILED WRITE MUST LEAVE THE SCENE DIRTY.
        //
        // This was `_ = try? engine.saveScene(…)` followed UNCONDITIONALLY by
        // `isDirty = false`, a "WROTE" log line and `noteScenePersisted`. ⚠️ Found
        // 2026-09-30 with the project's own drive pulled: every save failed
        // ("Operation not permitted"), yet each scene was marked clean and history
        // was told its bytes were on disk — so NOTHING retried when the drive came
        // back, and the edits lived only in memory until quit discarded them.
        // ✅ Now: on failure the scene stays dirty (the next save cadence retries
        // it), the failure is logged as a failure, and history is NOT told.
        do {
            _ = try engine.saveScene(
                projectID: projectID,
                projectRootPath: projectRootPath,
                appSupportRoot: appSupportRoot,
                sceneID: seg.sceneID,
                sceneMetadataPath: seg.metadataPath,
                sceneContentPath: seg.contentPath,
                markdown: seg.text,
                selectionAnchor: isCurrent ? currentSceneCursorOffset : 0,
                selectionFocus: isCurrent ? currentSceneCursorOffset : 0,
                scroll: isCurrent ? scrollFraction : 0,
                authorshipRef: ref
            )
        } catch {
            NSLog("[SCRIVI-DIAG] saveSceneBlocking FAILED scene=\(seg.sceneID) — kept DIRTY: \(error)")
            onSaveFailed?()
            return
        }
        NSLog("[SCRIVI-DIAG] saveSceneBlocking WROTE scene=\(seg.sceneID) isCurrent=\(isCurrent)")
        onSceneWritten?(seg.sceneID, pthread_main_np() != 0)
        segments[index].isDirty = false

        // I-0104: tell history the bytes we just wrote. The head hash persisted at
        // close must describe the FILE, not the replayed history head — otherwise
        // the next open compares two different artifacts and raises a spurious
        // `externalChange` barrier for a scene edited only inside Scrivi.
        //
        // ⚠️ This is the ONLY correct place for this call: right after a successful
        // write, with the exact bytes written. Calling it anywhere that merely *has*
        // scene text (e.g. HistoryCapture.close with its pending latch) records a hash
        // for text that may never have reached disk — tried 2026-08-10, and it made
        // the barrier count worse rather than better.
        if let historyCapture {
            historyCapture.noteScenePersisted(sceneID: seg.sceneID, diskText: seg.text)
        }
    }

    // Save the current segment (used by debounce auto-save and app resign).
    func saveCurrentIfDirty(engine: ScriviEngine, ref: AuthorshipRef) async {
        await saveScene(at: currentIndex, engine: engine, ref: ref)
    }

    // Save all dirty segments (used on app resign to flush everything).
    // The current scene is saved LAST so its writing-surface state (cursor + scroll)
    // is the one the backend records as lastWritingSurface for next-session resume.
    func saveAllDirty(engine: ScriviEngine, ref: AuthorshipRef) async {
        saveAllDirtyBlocking(engine: engine, ref: ref)
    }

    /// One scene's write, taken on the main actor and done off it ([I-0303]).
    private struct OffMainWrite: Sendable {
        let sceneID, metadataPath, contentPath, text: String
        let cursor: Int
        let scroll: Double
    }

    // ✅ [I-0303] — writes the dirty scenes at `indices` OFF the main actor (a Replace All's: only the scenes holding a
    // match). Their texts are taken now; each written scene is marked clean and its bytes told to history when the writes
    // land, unless the writer changed it meanwhile (then it is saved again). ⛔ [I-0259]: a failed write leaves it dirty.
    // `whenLanded` runs on the main actor in the same step that brings `scenesWritingOffMain` back down, so a lock it lifts
    // never outlives the count. Returns false (and never calls it) when nothing was dirty.
    @discardableResult
    func saveScenesOffMain(_ indices: some Sequence<Int>, engine: ScriviEngine, ref: AuthorshipRef,
                           whenLanded: @escaping @MainActor () -> Void = {}) -> Bool {
        let jobs: [OffMainWrite] = Set(indices).sorted().compactMap { i in
            guard segments.indices.contains(i), segments[i].isDirty, !inFlight.contains(segments[i].sceneID) else { return nil }
            let seg = segments[i]
            let isCurrent = i == currentIndex
            return OffMainWrite(sceneID: seg.sceneID, metadataPath: seg.metadataPath, contentPath: seg.contentPath, text: seg.text,
                       cursor: isCurrent ? currentSceneCursorOffset : 0, scroll: isCurrent ? scrollFraction : 0)
        }
        guard !jobs.isEmpty else { return false }
        inFlight.formUnion(jobs.map(\.sceneID))
        scenesWritingOffMain += jobs.count
        let projectID = projectID, root = projectRootPath, support = appSupportRoot
        let observe = onSceneWritten
        let group = offMainWrites
        group.enter()
        Task { @MainActor in
            let written: [Bool] = await Task.detached(priority: .userInitiated) {
                defer { group.leave() }
                return jobs.map { job in
                    do {
                        _ = try engine.saveScene(projectID: projectID, projectRootPath: root, appSupportRoot: support,
                                                 sceneID: job.sceneID, sceneMetadataPath: job.metadataPath,
                                                 sceneContentPath: job.contentPath, markdown: job.text,
                                                 selectionAnchor: job.cursor, selectionFocus: job.cursor, scroll: job.scroll,
                                                 authorshipRef: ref)
                    } catch {
                        NSLog("[SCRIVI-DIAG] saveScenesOffMain FAILED scene=\(job.sceneID) — kept DIRTY: \(error)")
                        return false
                    }
                    observe?(job.sceneID, pthread_main_np() != 0)
                    return true
                }
            }.value
            // ---- main actor from here down ----
            inFlight.subtract(jobs.map(\.sceneID))
            scenesWritingOffMain -= jobs.count
            whenLanded()
            var again: [Int] = []
            for (job, ok) in zip(jobs, written) {
                guard let i = segments.firstIndex(where: { $0.sceneID == job.sceneID }) else { continue }
                if ok {
                    NSLog("[SCRIVI-DIAG] saveScenesOffMain WROTE scene=\(job.sceneID)")
                    historyCapture?.noteScenePersisted(sceneID: job.sceneID, diskText: job.text)
                    if segments[i].text == job.text { segments[i].isDirty = false }
                }
                if saveAfterFlight.remove(job.sceneID) != nil || (ok && segments[i].isDirty) { again.append(i) }
            }
            if written.contains(false) { onSaveFailed?() }
            for i in again { saveSceneBlocking(at: i, engine: engine, ref: ref) }
            if scenesWritingOffMain == 0 {
                let waiters = offMainWaiters
                offMainWaiters = []
                for w in waiters { w() }
            }
        }
        return true
    }

    // ✅ [I-0305] — runs `then` on the main actor once every off-main write has landed (at once when none is running). A
    // close or quit waits on it (user, 2026-10-15: the close "must WAIT until the background process is completed").
    // ✅ [SP-191]: a structural edit in flight is an off-main write too.
    func whenOffMainWritesLand(_ then: @escaping @MainActor () -> Void) {
        whenScenesLand { [weak self] in
            guard let self else { return then() }
            if self.structuralInFlight { self.structuralWaiters.append(then) } else { then() }
        }
    }

    /// The scene saves alone (not a structural edit): what a structural edit itself waits for before its core call.
    private func whenScenesLand(_ then: @escaping @MainActor () -> Void) {
        if scenesWritingOffMain == 0 { then() } else { offMainWaiters.append(then) }
    }

    // MARK: — [SP-191] Structural edits: one range call off the main actor, the delta applied

    /// ✅ [SP-191] Q5 (user, 2026-10-21): from the key press until the delta is applied the editor is read-only, and repeated
    /// gestures, typing and undo are IGNORED, not queued — *"as long as the initial operation response is nearly
    /// instantaneous"*: the caller's cue lands in the same event-loop turn, before any core call.
    private(set) var structuralInFlight = false
    /// Q5: the progress row's text, once a structural edit has run for 400 ms; nil before that and after it lands.
    private(set) var structuralProgress: String?
    @ObservationIgnored private var structuralWaiters: [@MainActor () -> Void] = []
    @ObservationIgnored private var structuralGeneration = 0
    /// Tests: the main-thread core calls this editor's structural edits made (AC: none of their own).
    @ObservationIgnored let structuralCalls = StructuralCallSink()

    /// Runs ONE structural edit: every unsaved scene is written first (the core refuses a caret scene that is not what is on
    /// disk), then `call` runs OFF the main actor, then `landed` runs on it with the outcome. ⛔ Returns false, doing
    /// nothing, while another structural edit is in flight (Q5: ignored, not queued).
    @discardableResult
    func runStructural<T: Sendable>(_ label: String, ref: AuthorshipRef,
                                    _ call: @escaping @Sendable (ScriviEngine) throws -> T,
                                    landed: @escaping @MainActor (Result<T, Error>) -> Void) -> Bool {
        guard !structuralInFlight else { return false }
        return StructuralCallAccounting.$sink.withValue(structuralCalls) {
            startStructural(label, ref: ref, call, landed: landed)
        }
    }

    private func startStructural<T: Sendable>(_ label: String, ref: AuthorshipRef,
                                              _ call: @escaping @Sendable (ScriviEngine) throws -> T,
                                              landed: @escaping @MainActor (Result<T, Error>) -> Void) -> Bool {
        structuralInFlight = true
        structuralGeneration += 1
        let generation = structuralGeneration
        Task { @MainActor [weak self] in
            try? await Task.sleep(for: .milliseconds(400))
            guard let self, self.structuralInFlight, self.structuralGeneration == generation else { return }
            self.structuralProgress = label
        }
        _ = saveScenesOffMain(segments.indices, engine: engine, ref: ref)
        let engine = engine
        let group = offMainWrites
        whenScenesLand { [weak self] in
            guard let self else { return }
            group.enter()   // the quit path waits on this group (`saveAllDirtyBlocking`)
            // ⚠️ Re-marked: this closure can run later, from a save's landing, outside the caller's task.
            StructuralCallAccounting.$sink.withValue(self.structuralCalls) { Task { @MainActor [weak self] in
                let outcome: Result<T, Error> = await Task.detached(priority: .userInitiated) {
                    defer { group.leave() }
                    return Result { try call(engine) }
                }.value
                guard let self else { return }
                self.structuralInFlight = false
                landed(outcome)
                // ✅ [SP-188] Q6: an edit that starts another as it lands (a paste over a cut selection) keeps the row.
                if !self.structuralInFlight { self.structuralProgress = nil }
                let waiters = self.structuralWaiters
                self.structuralWaiters = []
                for w in waiters { w() }
            } }
        }
        return true
    }

    /// ✅ [SP-191] (design P3): apply a range operation's DELTA to the scene list — the model of record — instead of
    /// re-opening the project. Every scene it names is placed after its `afterSceneID` with its new chapter, paths and (when
    /// written) text; the removed ones leave. Returns the run that changed — where it sat before and where it sits now,
    /// starting at the same index — for `renderRun`; nil when the delta names nothing.
    @discardableResult
    func apply(_ delta: RangeDelta) -> (old: Range<Int>, new: Range<Int>)? {
        let removed = Set(delta.removedSceneIDs)
        let named = Set(delta.scenes.map(\.sceneID))
        let oldTouched = segments.indices.filter { removed.contains(segments[$0].sceneID) || named.contains(segments[$0].sceneID) }

        var segs = segments.filter { !removed.contains($0.sceneID) }
        var all = allScenes.filter { !removed.contains($0.sceneID) }
        var titles = liveTitles
        for id in removed { titles.removeValue(forKey: id) }
        for change in delta.scenes {
            let text = change.text ?? segs.first(where: { $0.sceneID == change.sceneID })?.text ?? ""
            let slug = all.first(where: { $0.sceneID == change.sceneID })?.slug ?? ""
            Self.place(SceneSegment(id: change.sceneID, sceneID: change.sceneID, chapterID: change.chapterID,
                                    metadataPath: change.metadataPath, contentPath: change.contentPath,
                                    text: text, isDirty: false),
                       in: &segs, id: \.sceneID, after: change.afterSceneID)
            Self.place(SceneInfo(sceneID: change.sceneID, chapterID: change.chapterID, title: change.title,
                                 chapterTitle: change.chapterTitle, slug: slug, metadataPath: change.metadataPath,
                                 contentPath: change.contentPath, chapterMetadataPath: change.chapterMetadataPath),
                       in: &all, id: \.sceneID, after: change.afterSceneID)
            if let written = change.text {
                let firstLine = written.components(separatedBy: .newlines)
                    .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
                if firstLine.isEmpty { titles.removeValue(forKey: change.sceneID) } else { titles[change.sceneID] = firstLine }
            }
        }
        let newTouched = segs.indices.filter { named.contains(segs[$0].sceneID) }
        segments = segs
        allScenes = all
        liveTitles = titles
        currentIndex = min(currentIndex, max(0, segments.count - 1))
        rebuildSceneStartMap()
        guard let oldLo = oldTouched.first, let oldHi = oldTouched.last,
              let newLo = newTouched.first, let newHi = newTouched.last else { return nil }
        return (oldLo..<oldHi + 1, newLo..<newHi + 1)
    }

    /// Puts `item` immediately after the element whose ID is `after` ("" = first), removing it from where it was.
    private static func place<E>(_ item: E, in list: inout [E], id: KeyPath<E, String>, after: String) {
        list.removeAll { $0[keyPath: id] == item[keyPath: id] }
        let at = after.isEmpty ? 0 : (list.firstIndex { $0[keyPath: id] == after }.map { $0 + 1 } ?? list.endIndex)
        list.insert(item, at: at)
    }

    // Synchronous counterpart for the quit path (I-0104/I-0108).
    func saveAllDirtyBlocking(engine: ScriviEngine, ref: AuthorshipRef) {
        // ✅ [I-0303]: let an off-main write finish first — its main-actor hop will never run during quit, so its scenes
        // (still dirty) are written again below, with their newest text. ✅ [SP-191]: a structural edit's core call too — it
        // is in the same group, and a quit must not cut it off half-written.
        if !inFlight.isEmpty || structuralInFlight {
            _ = offMainWrites.wait(timeout: .now() + 30)
            inFlight.removeAll()
            saveAfterFlight.removeAll()
        }
        NSLog("[SCRIVI-DIAG] quit: currentIndex=\(currentIndex) cursorScene=\(cursorSceneID ?? "nil") viewportScene=\(viewportSceneID ?? "nil") dirty=\(segments.filter{$0.isDirty}.count) cursorOffset=\(currentSceneCursorOffset) scroll=\(scrollFraction)")
        for i in segments.indices where segments[i].isDirty && i != currentIndex {
            saveSceneBlocking(at: i, engine: engine, ref: ref)
        }
        saveSceneBlocking(at: currentIndex, engine: engine, ref: ref)
        // Then stamp the scrolled-to scene as the resume point (I-0058 follow-up),
        // so a scene the writer scrolled to but never edited still resumes correctly.
        stampWritingSurfaceBlocking(engine: engine, ref: ref)
    }

    // Record the scrolled-to (viewport) scene as the backend's lastWritingSurface, even
    // when nothing is dirty (I-0058 follow-up). The backend stamps lastWritingSurface on
    // every saveScene; this forces one final save of the viewport scene so resume lands
    // on the scene the writer was *looking at*, not just the last one they edited.
    //
    // No-op only when the viewport scene isn't loaded.
    func stampWritingSurface(engine: ScriviEngine, ref: AuthorshipRef) async {
        stampWritingSurfaceBlocking(engine: engine, ref: ref)
    }

    func stampWritingSurfaceBlocking(engine: ScriviEngine, ref: AuthorshipRef) {
        guard let vpID = viewportSceneID,
              let seg = segments.first(where: { $0.sceneID == vpID }) else { return }

        // ⚠️ **I-0131: this must NOT be conditional on the cursor being elsewhere.**
        //
        // The guard here used to be `vpID != cursorSceneID`, on the reasoning that a
        // viewport scene which already holds the cursor was stamped by the
        // current-scene save in `saveAllDirtyBlocking`. That is true only when that
        // scene is DIRTY — `saveSceneBlocking` returns early otherwise, so for a clean
        // scene both paths declined and **nothing was written at all**.
        //
        // ⚠️ That was a real hole but it was NOT the reported bug — see the root cause
        // in `ManuscriptTextView.navigateToScene`. A navigator click does not move the
        // caret (by design, SP-063's ruling is about click-to-place *within* the
        // manuscript), so `viewportSceneID` and `cursorSceneID` genuinely differ and
        // the old guard did fire. The scene it stamped was wrong because the debounced
        // scroll handler had already overwritten `viewportSceneID`. Both are fixed;
        // this one closes the clean-scene hole so the stamp cannot silently no-op.
        //
        // Writing unconditionally is safe: `saveScene` is idempotent for unchanged
        // bytes, and this is one write on the quit path.
        let cursorIsHere = (vpID == cursorSceneID)

        // When the cursor IS in this scene, persist its real offset — sending 0 would
        // throw away the caret position this call exists to preserve. When it is not,
        // 0 is correct per §9.3 ("scene changed / cursor not here" ⇒ restore the scene
        // and place the cursor safely).
        let offset = cursorIsHere ? currentSceneCursorOffset : 0

        // ⛔ [I-0259] — only a write that SUCCEEDED may be reported to history.
        do {
            _ = try engine.saveScene(
                projectID: projectID,
                projectRootPath: projectRootPath,
                appSupportRoot: appSupportRoot,
                sceneID: seg.sceneID,
                sceneMetadataPath: seg.metadataPath,
                sceneContentPath: seg.contentPath,
                markdown: seg.text,
                selectionAnchor: offset,
                selectionFocus: offset,
                scroll: scrollFraction,
                authorshipRef: ref
            )
        } catch {
            NSLog("[SCRIVI-DIAG] stampWritingSurface FAILED scene=\(seg.sceneID): \(error)")
            return
        }
        NSLog("[SCRIVI-DIAG] stampWritingSurface WROTE scene=\(seg.sceneID) cursorIsHere=\(cursorIsHere) offset=\(offset)")
        // I-0104: this path writes the scene file too, so it must report the bytes it
        // wrote — otherwise a scene stamped here keeps a stale baseline and re-flags as
        // externally changed on the next open.
        historyCapture?.noteScenePersisted(sceneID: seg.sceneID, diskText: seg.text)
    }

    // Called by ManuscriptTextView.Coordinator.textViewDidChangeSelection.
    // Only updates the non-observable cursor position. Scene tracking is done
    // via setCurrentIndex() and setHighlightedScene() from the coordinator.
    func updateCursorPosition(_ manuscriptPosition: Int) {
        manuscriptCursorPosition = manuscriptPosition
    }

    // Record the scene-local cursor offset within the current scene (I-0058).
    // Sent as the WorkspaceState selection on the next save so it can be restored.
    func updateSceneCursorOffset(_ offset: Int) {
        currentSceneCursorOffset = max(0, offset)
    }

    // Record the current document scroll fraction (I-0058), sent on the next save.
    func updateScrollFraction(_ fraction: Double) {
        scrollFraction = min(max(fraction, 0), 1)
    }

    // Return the NSTextStorage offset for the start of a scene (separator chars included).
    func storageOffset(forSceneID sceneID: String) -> Int? {
        sceneStorageOffsetMap[sceneID]
    }

    // Return the NSTextStorage offset for cursor placement after a delete.
    // Returns the storage start of the scene at `segmentIndex`, or end-of-storage if out of range.
    func storageOffsetForCursorAfterDelete(nextSegmentIndex: Int, totalStorageLength: Int) -> Int {
        guard segments.indices.contains(nextSegmentIndex),
              let offset = sceneStorageOffsetMap[segments[nextSegmentIndex].sceneID]
        else { return totalStorageLength }
        return offset
    }

    // Update the stored title for a scene in allScenes (called after a successful renameScene).
    func updateSceneTitle(_ newTitle: String, forMetadataPath metadataPath: String) {
        guard let idx = allScenes.firstIndex(where: { $0.metadataPath == metadataPath }) else { return }
        let old = allScenes[idx]
        allScenes[idx] = SceneInfo(
            sceneID: old.sceneID,
            chapterID: old.chapterID,
            title: newTitle,
            chapterTitle: old.chapterTitle,
            slug: old.slug,
            metadataPath: old.metadataPath,
            contentPath: old.contentPath,
            chapterMetadataPath: old.chapterMetadataPath
        )
    }

    // Update the stored chapter title for all scenes in a chapter (called after renameChapter).
    func updateChapterTitle(_ newTitle: String, forChapterMetadataPath chapterMetadataPath: String) {
        for idx in allScenes.indices where allScenes[idx].chapterMetadataPath == chapterMetadataPath {
            let old = allScenes[idx]
            allScenes[idx] = SceneInfo(
                sceneID: old.sceneID,
                chapterID: old.chapterID,
                title: old.title,
                chapterTitle: newTitle,
                slug: old.slug,
                metadataPath: old.metadataPath,
                contentPath: old.contentPath,
                chapterMetadataPath: old.chapterMetadataPath
            )
        }
    }

    // Reorder a scene within allScenes to reflect a successful reorderScene call.
    // `targetChapterID` is the chapter the scene now belongs to.
    // `afterSceneID` is the scene it should follow; empty means "insert at beginning of chapter".
    func reorderScene(
        sceneID: String,
        targetChapterID: String,
        afterSceneID: String,
        targetChapterMetadataPath: String,
        targetChapterTitle: String
    ) {
        guard let fromIdx = allScenes.firstIndex(where: { $0.sceneID == sceneID }) else { return }
        var scene = allScenes.remove(at: fromIdx)
        // Update chapter membership fields if the scene moved across chapters.
        if scene.chapterID != targetChapterID {
            scene = SceneInfo(
                sceneID: scene.sceneID,
                chapterID: targetChapterID,
                title: scene.title,
                chapterTitle: targetChapterTitle,
                slug: scene.slug,
                metadataPath: scene.metadataPath,
                contentPath: scene.contentPath,
                chapterMetadataPath: targetChapterMetadataPath
            )
        }
        if afterSceneID.isEmpty {
            // Insert at the beginning of the target chapter's block.
            let insertAt = allScenes.firstIndex(where: { $0.chapterID == targetChapterID }) ?? allScenes.endIndex
            allScenes.insert(scene, at: insertAt)
        } else {
            let afterIdx = allScenes.firstIndex(where: { $0.sceneID == afterSceneID }) ?? allScenes.endIndex - 1
            allScenes.insert(scene, at: afterIdx + 1)
        }
        // Keep segments in sync.
        if let segIdx = segments.firstIndex(where: { $0.sceneID == sceneID }) {
            var seg = segments.remove(at: segIdx)
            seg = SceneSegment(
                id: seg.id,
                sceneID: seg.sceneID,
                chapterID: targetChapterID,
                metadataPath: seg.metadataPath,
                contentPath: seg.contentPath,
                text: seg.text,
                isDirty: seg.isDirty
            )
            if afterSceneID.isEmpty {
                let insertAt = segments.firstIndex(where: { $0.chapterID == targetChapterID }) ?? segments.endIndex
                segments.insert(seg, at: insertAt)
            } else {
                let afterIdx = segments.firstIndex(where: { $0.sceneID == afterSceneID }) ?? segments.endIndex - 1
                segments.insert(seg, at: afterIdx + 1)
            }
            currentIndex = segments.firstIndex(where: { $0.sceneID == currentSegment?.sceneID }) ?? 0
        }
    }

    // Reorder a chapter within allScenes to reflect a successful reorderChapter call.
    // `afterChapterID` is the chapter the moved chapter should follow; empty means first.
    func reorderChapter(chapterID: String, afterChapterID: String) {
        // Extract all scenes belonging to the moved chapter, preserving order.
        let movedScenes = allScenes.filter { $0.chapterID == chapterID }
        allScenes.removeAll { $0.chapterID == chapterID }

        let insertAt: Int
        if afterChapterID.isEmpty {
            insertAt = 0
        } else if let afterIdx = allScenes.lastIndex(where: { $0.chapterID == afterChapterID }) {
            insertAt = afterIdx + 1
        } else {
            insertAt = allScenes.endIndex
        }
        allScenes.insert(contentsOf: movedScenes, at: insertAt)

        // Keep segments in sync.
        let movedSegs = segments.filter { $0.chapterID == chapterID }
        segments.removeAll { $0.chapterID == chapterID }
        let segInsertAt: Int
        if afterChapterID.isEmpty {
            segInsertAt = 0
        } else if let afterIdx = segments.lastIndex(where: { $0.chapterID == afterChapterID }) {
            segInsertAt = afterIdx + 1
        } else {
            segInsertAt = segments.endIndex
        }
        segments.insert(contentsOf: movedSegs, at: segInsertAt)
        currentIndex = segments.firstIndex(where: { $0.sceneID == currentSegment?.sceneID }) ?? 0
    }

    // Remove a scene from allScenes and segments after a successful deleteScene call.
    // Returns the sceneID to navigate to next (nearest remaining scene), or nil if none left.
    func removeScene(sceneID: String) -> String? {
        allScenes.removeAll { $0.sceneID == sceneID }
        segments.removeAll { $0.sceneID == sceneID }
        liveTitles.removeValue(forKey: sceneID)

        if !segments.isEmpty {
            currentIndex = min(currentIndex, segments.count - 1)
            rebuildSceneStartMap()
            return segments[currentIndex].sceneID
        }
        rebuildSceneStartMap()
        return nil
    }

    // Remove all scenes belonging to chapterID from allScenes and segments.
    // Returns the sceneID to navigate to next (nearest remaining scene), or nil if none left.
    func removeChapter(chapterID: String) -> String? {
        let removedIDs = Set(allScenes.filter { $0.chapterID == chapterID }.map(\.sceneID))
        allScenes.removeAll { $0.chapterID == chapterID }
        segments.removeAll { removedIDs.contains($0.sceneID) }
        removedIDs.forEach { liveTitles.removeValue(forKey: $0) }

        if !segments.isEmpty {
            currentIndex = min(currentIndex, segments.count - 1)
            rebuildSceneStartMap()
            return segments[currentIndex].sceneID
        }
        rebuildSceneStartMap()
        return nil
    }

    // MARK: — Private helpers

    // Build sceneStartMap and sceneStorageOffsetMap from the current segments array.
    // sceneStartMap: manuscript-coordinate positions (separator chars excluded).
    // sceneStorageOffsetMap: NSTextStorage offsets (separator chars included, 2 chars each).
    // Called after loadAll() and after any structural change.
    func rebuildSceneStartMap() {
        var manuscriptOffset = 0
        var storageOffset = 0
        var newStartMap: [String: Int] = [:]
        var newStorageMap: [String: Int] = [:]

        for (i, seg) in segments.enumerated() {
            if i > 0 {
                storageOffset += 2 // attachment char + \n for separator
            }
            newStartMap[seg.sceneID] = manuscriptOffset
            newStorageMap[seg.sceneID] = storageOffset

            manuscriptOffset += seg.text.count
            storageOffset += seg.text.count
        }
        sceneStartMap = newStartMap
        sceneStorageOffsetMap = newStorageMap
    }

    /// AC9 — the bulk-load counterpart of `loadScene`: does the SAME per-scene work but
    /// writes into caller-owned storage, so a full load posts no observation notification
    /// until the caller assigns. `loadScene` is retained for incremental single-scene
    /// inserts, where one notification is correct.
    private func buildScene(at allIdx: Int,
                            into out: inout [SceneSegment],
                            titles: inout [String: String]) {
        let info = allScenes[allIdx]
        // ⚠️ SP-144 — BULK variant (this IS the bulk-load counterpart of
        // `loadScene`, per the doc comment above): no per-scene workspace write.
        let loaded = ScriviDiag.measure("  engine.openSceneForBulkLoad (C ABI)") {
            try? engine.openSceneForBulkLoad(
                projectRootPath: projectRootPath,
                appSupportRoot: appSupportRoot,
                projectID: projectID,
                sceneID: info.sceneID
            )
        }
        let text = loaded?.markdown ?? ""
        out.append(SceneSegment(
            id: info.sceneID,
            sceneID: info.sceneID,
            chapterID: info.chapterID,
            metadataPath: info.metadataPath,
            contentPath: info.contentPath,
            text: text
        ))
        let firstLine = ScriviDiag.measure("  firstLine scan") {
            text.components(separatedBy: .newlines)
                .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
        }
        if !firstLine.isEmpty {
            titles[info.sceneID] = firstLine
        }
    }


}
