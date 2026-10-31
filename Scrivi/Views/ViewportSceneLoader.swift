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

    // ✅ EP-052 ([SP-195]): how many scenes have a queued save not yet landed. Shown as "Saving N scenes…"; never locks
    // the manuscript.
    private(set) var scenesWritingOffMain = 0
    /// [I-0305] — run once no queued save is left, or the queue is HELD (`whenOffMainWritesLand`).
    @ObservationIgnored private var offMainWaiters: [@MainActor () -> Void] = []

    // Tests ([I-0303]): told of every scene write that succeeded — its sceneID and whether it ran on the main thread.
    @ObservationIgnored var onSceneWritten: (@Sendable (_ sceneID: String, _ onMainThread: Bool) -> Void)?

    // EP-052: the scene and text each queued ticket carries. A ticket leaves when it lands, is replaced by a newer save of
    // its scene, or is refused. ONE queue in the core orders every write ([SP-195], W6 retired here).
    @ObservationIgnored private var queued: [Int64: (sceneID: String, text: String)] = [:]
    @ObservationIgnored private var waiting = false
    /// [SP-196] The structural job this editor queued and has not seen land: its ticket, and what runs with its outcome.
    @ObservationIgnored private var structural: (ticket: Int64, landed: @MainActor (WriteQueueWait.WriteOutcome?) -> Void)?
    /// A structural outcome the waiter took before its ticket was known here (it is queued off the main actor).
    @ObservationIgnored private var unclaimedStructural: [Int64: WriteQueueWait.WriteOutcome] = [:]
    /// [SP-195] Q1 (a): a save could not reach the disk; the core keeps it and retries. A close or quit then asks ([T-0570])
    /// instead of waiting.
    private(set) var writesHeld = false

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
        let span = ScriviDiag.begin("viewport scene changed"); defer { ScriviDiag.end(span) }
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
                let firstLine = ManuscriptPresenter.presentedFirstLine(of: text)
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

    // ✅ EP-052 ([SP-195]): QUEUES the save; the core writes it off the main thread, in order with every other write of
    // this project, and the scene is marked clean when its outcome lands (`apply(_ wait:)`). ⛔ [I-0259]: a failed write
    // leaves it dirty. Still callable without `await`, which `applicationWillTerminate` needs (I-0104/I-0108).
    func saveSceneBlocking(at index: Int, engine: ScriviEngine, ref: AuthorshipRef) {
        guard segments.indices.contains(index), segments[index].isDirty else { return }
        let isCurrent = index == currentIndex
        enqueueSaves([(index, isCurrent ? currentSceneCursorOffset : 0, isCurrent ? scrollFraction : 0)], ref: ref)
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

    /// Queues the dirty scenes at `indices` (a Replace All's: only the scenes holding a match). Returns false when nothing
    /// was dirty.
    @discardableResult
    func saveScenesOffMain(_ indices: some Sequence<Int>, engine: ScriviEngine, ref: AuthorshipRef) -> Bool {
        let entries = Set(indices).sorted().filter { segments.indices.contains($0) && segments[$0].isDirty }.map { i in
            (i, i == currentIndex ? currentSceneCursorOffset : 0, i == currentIndex ? scrollFraction : 0)
        }
        return enqueueSaves(entries, ref: ref)
    }

    /// One `scrivi_enqueue_save_scenes` call: each entry is a scene index, its caret and its scroll. A memory call: it
    /// returns at once.
    @discardableResult
    private func enqueueSaves(_ entries: [(index: Int, cursor: Int, scroll: Double)], ref: AuthorshipRef) -> Bool {
        // ⛔ [SP-196] rig: never behind a structural edit in flight. Every dirty scene was queued before it (W7b) and the
        // text takes no typing until it lands, so a save now only repeats one, after the job, with the text from before
        // it: a scene the job removed held the queue for good. The scenes stay dirty; they are queued when it lands.
        guard !structuralInFlight else { return false }
        let saves = entries.filter { segments.indices.contains($0.index) }.map { e in
            let seg = segments[e.index]
            return QueuedSave(sceneID: seg.sceneID, metadataPath: seg.metadataPath, contentPath: seg.contentPath,
                              markdown: seg.text, selectionAnchor: e.cursor, selectionFocus: e.cursor, scroll: e.scroll)
        }
        guard !saves.isEmpty else { return false }
        let tickets: [String: Int64]
        do {
            tickets = try engine.enqueueSaveScenes(projectID: projectID, projectRootPath: projectRootPath,
                                                   appSupportRoot: appSupportRoot, scenes: saves, authorshipRef: ref)
        } catch {
            NSLog("[SCRIVI-DIAG] enqueue save FAILED — kept DIRTY: \(error)")
            onSaveFailed?()
            return false
        }
        for save in saves { if let t = tickets[save.sceneID] { queued[t] = (save.sceneID, save.markdown) } }
        countQueued()
        startWaiting()
        return true
    }

    private func countQueued() {
        let n = Set(queued.values.map(\.sceneID)).count
        if scenesWritingOffMain != n { scenesWritingOffMain = n }
    }

    /// One background waiter at a time: it hands every outcome to the main actor until nothing queued is left.
    private func startWaiting() {
        guard !waiting else { return }
        waiting = true
        let engine = engine, root = projectRootPath
        Task.detached(priority: .utility) { [weak self] in
            while true {
                let wait = try? engine.waitForWrites(projectRootPath: root, timeoutMs: 1000)
                guard let self, await self.apply(wait) else { return }
                // A held queue answers at once; the core retries every 5 s, so asking more often learns nothing.
                if wait?.held ?? true { try? await Task.sleep(for: .seconds(1)) }
            }
        }
    }

    /// Main actor: applies one wait's outcomes. Returns whether to keep waiting.
    private func apply(_ wait: WriteQueueWait?) -> Bool {
        var failed = false
        var structuralOutcome: WriteQueueWait.WriteOutcome?
        for o in wait?.outcomes ?? [] where o.kind == "structural" && o.held != true {   // a held one lands later
            if o.ticket == structural?.ticket { structuralOutcome = o } else { unclaimedStructural[o.ticket] = o }
        }
        for o in wait?.outcomes ?? [] where o.kind == "save" {
            guard let sent = queued[o.ticket] else { continue }
            if o.supersededBy != nil { queued[o.ticket] = nil; continue }   // the newer save's outcome decides
            if o.ok {
                queued[o.ticket] = nil
                NSLog("[SCRIVI-DIAG] queued save WROTE scene=\(sent.sceneID)")
                onSceneWritten?(sent.sceneID, false)
                if let i = segments.firstIndex(where: { $0.sceneID == sent.sceneID }), segments[i].text == sent.text {
                    segments[i].isDirty = false   // unless the writer changed it since
                }
            } else {
                NSLog("[SCRIVI-DIAG] queued save FAILED scene=\(sent.sceneID) held=\(o.held == true) — kept DIRTY")
                if o.held != true { queued[o.ticket] = nil }   // refused: it will not be retried
                failed = true
            }
        }
        // ⚠️ A failed wait is NOT a failed edit: the job is still queued and will run, so keep waiting for its outcome
        // ([SP-196] rig: giving up left the edit on disk and not in the window).
        if let s = structural, structuralOutcome != nil {
            structural = nil
            s.landed(structuralOutcome)
        }
        guard let wait else {   // the wait itself failed: the scenes stay dirty for the next save
            queued.removeAll()
            countQueued()
            return structural != nil || finishWaiting()
        }
        if writesHeld != wait.held { writesHeld = wait.held }
        countQueued()
        if failed { onSaveFailed?() }
        if queued.isEmpty && structural == nil { return finishWaiting() }
        if wait.held { runWaiters() }   // a close or quit asks now instead of waiting ([T-0570])
        return true
    }

    private func finishWaiting() -> Bool {
        waiting = false
        if writesHeld { writesHeld = false }
        runWaiters()
        return false
    }

    private func runWaiters() {
        // A held structural edit cannot land until the drive is back: a close or quit asks rather than waits for it.
        let waiters = offMainWaiters + (writesHeld ? structuralWaiters : [])
        offMainWaiters = []
        if writesHeld { structuralWaiters = [] }
        for w in waiters { w() }
    }

    // ✅ [I-0305] — runs `then` on the main actor once every queued save has landed, or the queue is held (at once when
    // neither). A close or quit waits on it (user, 2026-10-15: the close "must WAIT until the background process is
    // completed"). ✅ [SP-191]: a structural edit in flight is an off-main write too.
    func whenOffMainWritesLand(_ then: @escaping @MainActor () -> Void) {
        whenScenesLand { [weak self] in
            guard let self else { return then() }
            if self.structuralInFlight && !self.writesHeld { self.structuralWaiters.append(then) } else { then() }
        }
    }

    private func whenScenesLand(_ then: @escaping @MainActor () -> Void) {
        if scenesWritingOffMain == 0 || writesHeld { then() } else { offMainWaiters.append(then) }
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

    /// Runs ONE structural edit through the write queue ([SP-196], W7): every unsaved scene is queued first (W7b), then
    /// `enqueue` queues the job OFF the main actor and returns its ticket; the job's result comes back in its outcome
    /// (W7-Q1 (a)), `finish` turns it into `T` off the main actor, and `landed` runs on it. ⛔ Returns false, doing nothing,
    /// while another structural edit is in flight (Q5: ignored, not queued) or the queue is held ([SP-195] Q1 (a)).
    @discardableResult
    func runStructural<T: Sendable>(_ label: String, ref: AuthorshipRef,
                                    enqueue: @escaping @Sendable (ScriviEngine) throws -> Int64,
                                    finish: @escaping @Sendable (ScriviEngine, RangeOperationResult) -> T,
                                    landed: @escaping @MainActor (Result<T, Error>) -> Void) -> Bool {
        guard !structuralInFlight, !writesHeld else { return false }
        return StructuralCallAccounting.$sink.withValue(structuralCalls) {
            startStructural(label, ref: ref, enqueue: enqueue, finish: finish, landed: landed)
        }
    }

    private func startStructural<T: Sendable>(_ label: String, ref: AuthorshipRef,
                                              enqueue: @escaping @Sendable (ScriviEngine) throws -> Int64,
                                              finish: @escaping @Sendable (ScriviEngine, RangeOperationResult) -> T,
                                              landed: @escaping @MainActor (Result<T, Error>) -> Void) -> Bool {
        // W7b: the queue's order replaces "save first" — the job runs on what these saves wrote. Before the flag, which
        // holds back every later save until the job lands.
        _ = saveScenesOffMain(segments.indices, engine: engine, ref: ref)
        structuralInFlight = true
        structuralGeneration += 1
        let began = Date()
        ScriviDiag.note("structural: \(label) started")
        let generation = structuralGeneration
        Task { @MainActor [weak self] in
            try? await Task.sleep(for: .milliseconds(400))
            guard let self, self.structuralInFlight, self.structuralGeneration == generation else { return }
            self.structuralProgress = label
        }
        let done: @MainActor (Result<T, Error>) -> Void = { [weak self] outcome in
            guard let self else { return }
            self.structuralInFlight = false
            _ = self.saveScenesOffMain(self.segments.indices, engine: self.engine, ref: ref)   // what was held back
            StructuralCallAccounting.$sink.withValue(self.structuralCalls) {
                ScriviDiag.span("structural: \(label) applied", always: true) { landed(outcome) }
            }
            ScriviDiag.note(String(format: "structural: %@ %@ after %.0f ms in all", label,
                                   (try? outcome.get()) == nil ? "FAILED" : "landed", Date().timeIntervalSince(began) * 1000))
            // ✅ [SP-188] Q6: an edit that starts another as it lands (a paste over a cut selection) keeps the row.
            if !self.structuralInFlight { self.structuralProgress = nil }
            let waiters = self.structuralWaiters
            self.structuralWaiters = []
            for w in waiters { w() }
        }
        let engine = engine
        Task { @MainActor [weak self] in
            let queued = await Task.detached(priority: .userInitiated) { Result { try enqueue(engine) } }.value
            guard let self else { return }
            let ticket: Int64
            do { ticket = try queued.get() } catch { return done(.failure(error)) }
            ScriviDiag.note("structural: \(label) queued, ticket \(ticket)")
            let landed: @MainActor (WriteQueueWait.WriteOutcome?) -> Void = { outcome in
                guard let outcome, let result = outcome.range else {
                    return done(.failure(outcome?.error ?? ScriviError(code: -1, message: "the write queue could not be read")))
                }
                Task { @MainActor in
                    done(.success(await Task.detached(priority: .userInitiated) { finish(engine, result) }.value))
                }
            }
            if let early = self.unclaimedStructural.removeValue(forKey: ticket) { return landed(early) }
            self.structural = (ticket, landed)
            self.startWaiting()
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
                let firstLine = ManuscriptPresenter.presentedFirstLine(of: written)
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
        // EP-052: queued, in this order, behind every write already queued; the caller waits for them at quit.
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

        // EP-052: queued like any save, even when the scene is clean; its outcome marks nothing dirty or clean that the
        // text does not match.
        guard let index = segments.firstIndex(where: { $0.sceneID == seg.sceneID }) else { return }
        enqueueSaves([(index, offset, scrollFraction)], ref: ref)
        NSLog("[SCRIVI-DIAG] stampWritingSurface QUEUED scene=\(seg.sceneID) cursorIsHere=\(cursorIsHere) offset=\(offset)")
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
            ManuscriptPresenter.presentedFirstLine(of: text)
        }
        if !firstLine.isEmpty {
            titles[info.sceneID] = firstLine
        }
    }


}
