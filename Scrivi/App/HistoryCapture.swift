import Foundation

// HistoryCapture — the Swift-side capture/apply driver for EP-019 undo/redo
// (SP-053, T-0204/T-0205/T-0206). One instance per open project, owned by
// ProjectSession.
//
// Responsibilities (design §4, §8):
//  - Detect commit triggers and turn accumulated text changes into history
//    events via engine.historyRecordEvent (snapshot-diff happens in C++; Swift
//    only sends the affected scene's full current text).
//  - Enforce the "history commits before disk writes" invariant (§4.d) via
//    flushThenSave.
//  - Provide undo/redo results (full scene text + cursor) for the editor to
//    apply, and record structural barriers (§4.5).
//
// This type holds NO history state machine beyond a single pending-edit latch:
// the tree, diffing, sessions, and pointers all live in ScriviCore. Swift never
// interprets offsets — undo/redo return full scene text.
//
// Concurrency: @MainActor because it is created and used entirely on the main
// actor (owned by the @MainActor ProjectSession; driven by the @MainActor text
// coordinator). engine.history* calls are synchronous and thread-safe on the
// C side (mutex-guarded registry), so no cross-actor hops are needed.
@MainActor
final class HistoryCapture {

    private let engine: ScriviEngine
    private let projectRootPath: String

    // True once historyOpen has succeeded and false after close, so trigger
    // notifications and undo/redo become no-ops when history is unavailable.
    private(set) var isOpen: Bool = false

    // Pending edit latch (§4.a): the scene that has uncommitted text and the
    // full current text to record at the next trigger. `pendingSceneID == nil`
    // means "no text changed since the last event" — a pure cursor move then
    // produces no event.
    private var pendingSceneID: String?
    private var pendingText: String = ""
    private var pendingCursorBefore: Int = 0
    private var pendingCursorAfter: Int = 0

    // Guards re-entrancy: while an undo/redo apply is in flight we must not
    // treat the resulting textDidChange as a new user edit.
    private(set) var isApplying: Bool = false

    // I-0105: bumped on every mutation of the history tree (commit, undo, redo,
    // barrier, purge). The history card folds this into its `.task(id:)` so a
    // commit re-fetches the tree; without it the card reloaded on scene identity
    // alone and went stale until the project was reopened. A counter rather than
    // a notification because the card only needs "something changed", and the
    // value is cheap to diff in a task id.
    private(set) var revision: Int = 0

    private func bumpRevision() { revision &+= 1 }

    // MARK: — Typing-session coalescing (T-0396)

    // How long the writer must be idle before the next keystroke starts a NEW
    // history entry rather than continuing the open one. Deliberately long
    // (user ruling, 2026-08-07: 30–60 s) — "sometimes we writers ruminate for
    // longer periods", so a pause only ends a session when it means the writer
    // actually stopped, not merely thought mid-sentence. This is a SESSION
    // boundary and is entirely separate from the 1 s autosave cadence, which
    // writes to disk without sealing anything.
    static let idleSessionThreshold: TimeInterval = 45

    // Timestamp of the last text edit, for the idle-session boundary above.
    private var lastActivity: Date?

    // True when the writer has been idle past the threshold, so the open typing
    // session should be considered over.
    private var isSessionIdle: Bool {
        guard let lastActivity else { return false }
        return Date().timeIntervalSince(lastActivity) >= Self.idleSessionThreshold
    }

    // Fires the idle boundary on its own. Without this the session would only end
    // when the writer did something ELSE (typed again, moved the caret, closed the
    // project) — so a writer who simply stops mid-sentence and walks away would
    // leave the entry uncommitted, and the history card would not show their last
    // sentence until they came back. Restarted on every edit; cancelled on commit.
    private var idleTask: Task<Void, Never>?

    private func scheduleIdleCommit() {
        idleTask?.cancel()
        idleTask = Task { @MainActor [weak self] in
            try? await Task.sleep(nanoseconds: UInt64(Self.idleSessionThreshold * 1_000_000_000))
            guard !Task.isCancelled, let self, self.hasPendingChanges else { return }
            self.flush(trigger: "idle")
        }
    }

    // Clear the session clock once an entry has been committed, so a later edit is
    // judged fresh rather than against a stale timestamp, and stand the idle timer
    // down — there is nothing left pending for it to seal.
    private func endContinuation() {
        lastActivity = nil
        idleTask?.cancel()
        idleTask = nil
    }

    // Scenes whose baseline (pre-edit floor text) has been seeded this session,
    // so we seed each scene exactly once on its first edit.
    private var seededScenes: Set<String> = []

    // The scene text as of the last committed event (per pending scene), so a
    // soft trigger can tell whether the pending change is only whitespace — those
    // don't commit on their own; the whitespace rides along with the next real
    // text (§4.a refinement, user-reported 2026-07-07).
    private var lastCommittedText: String = ""

    // Cached enable-state for the Edit ▸ Undo/Redo menu items, updated from every
    // engine response. `validateUserInterfaceItem` reads these. Because a pending
    // (uncommitted) edit means there IS something to undo, canUndoNow also
    // reflects hasPendingChanges.
    private var engineCanUndo: Bool = false
    private var engineCanRedo: Bool = false
    var canUndoNow: Bool { engineCanUndo || hasPendingChanges }
    var canRedoNow: Bool { engineCanRedo }

    init(engine: ScriviEngine, projectRootPath: String) {
        self.engine = engine
        self.projectRootPath = projectRootPath
    }

    // MARK: — Lifecycle

    func open() {
        guard !isOpen else { return }
        do {
            let r = try engine.historyOpen(projectRootPath: projectRootPath)
            engineCanUndo = r.canUndo
            engineCanRedo = r.canRedo
            isOpen = true
        } catch {
            // History is best-effort; a failure here must never block editing.
            isOpen = false
            print("[Scrivi] historyOpen failed: \(error)")
        }
    }

    // Validates each loaded scene's on-disk text against the head hash persisted
    // at last close (§6.b). A scene edited outside Scrivi between sessions gets an
    // externalChange barrier + floor re-seed in the engine — never touching the
    // manuscript. Call once, right after open, with (sceneID, currentText) pairs.
    func validateScenes(_ scenes: [(id: String, text: String)]) {
        guard isOpen else { return }
        for scene in scenes {
            _ = try? engine.historyValidateScene(
                projectRootPath: projectRootPath, sceneID: scene.id, currentDiskText: scene.text)
        }
    }

    func close() {
        guard isOpen else { return }
        // Commit anything outstanding so the final sentence is not lost.
        flush(trigger: "flush")

        // ⚠️ DO NOT call noteScenePersisted() here with the pending text.
        //
        // That was tried on 2026-08-10 and made things strictly worse (the writer's
        // barrier count went 7 → 8 on a single relaunch). The reasoning behind it —
        // "the pending text is already on disk, autosave wrote it" — is FALSE: T-0396
        // defers the save-time commit, and `applicationWillTerminate` does not call
        // `saveAllDirty`, so on quit the pending text may never have been written at
        // all. Recording its hash told the next open that the file should contain text
        // it does not, guaranteeing a mismatch → barrier, permanently.
        //
        // `noteScenePersisted` has exactly one correct caller: the save path, AFTER a
        // successful write, with the bytes it actually wrote
        // (`ViewportSceneLoader.saveScene`). Anywhere else is guessing at disk state.
        _ = try? engine.historyClose(projectRootPath: projectRootPath)
        isOpen = false
        clearPending()
    }

    // MARK: — Edit notifications (called by the text coordinator)

    // The user changed text in `sceneID`; `text` is that scene's full current
    // text. `cursor` is the scene-local UTF-8 byte offset of the insertion
    // point. `baselineIfFirst` is the scene's text *before* this edit — used to
    // seed the scene's history floor the first time it is edited, so undo stops
    // at the pre-existing text rather than emptying the scene. Latches the
    // pending edit; the actual event commits at a trigger.
    func noteEdit(sceneID: String, text: String, cursor: Int, baselineIfFirst: String) {
        guard isOpen, !isApplying else { return }
        seedBaselineIfNeeded(sceneID: sceneID, baseline: baselineIfFirst)

        // T-0396 — an idle gap ends the typing session. If the writer stopped for
        // longer than the threshold, commit what was pending BEFORE folding this
        // keystroke in, so the resumed work starts its own entry. Below the
        // threshold the pending latch simply stays open and this edit joins it —
        // which, with the save-time commit deferred, is what makes a continuously
        // typed sentence record as one event instead of three.
        if pendingSceneID != nil, isSessionIdle {
            flush(trigger: "idle")
        }

        if pendingSceneID == nil {
            // First change since the last commit — remember where the cursor was.
            pendingCursorBefore = cursor
        }
        pendingSceneID = sceneID
        pendingText = text
        pendingCursorAfter = cursor
        lastActivity = Date()
        scheduleIdleCommit()
    }

    // Seeds a scene's ROOT FLOOR text (the state undo stops at, never below):
    // the empty scene for a from-scratch project, or the pre-existing on-disk
    // text for a project that already had content. Conceptually a one-time,
    // project-lifetime seed done at history creation; because the SP-053 engine is
    // in-memory (opens empty each launch), we seed on first edit per session as a
    // behaviourally-identical stand-in. SP-054 moves this to history-creation time
    // and stops re-seeding per session (the on-disk text becomes the replayed
    // HEAD, not the floor; head-hash mismatch → externalChange barrier). Design §5.
    private func seedBaselineIfNeeded(sceneID: String, baseline: String) {
        guard !seededScenes.contains(sceneID) else { return }
        seededScenes.insert(sceneID)
        lastCommittedText = baseline   // whitespace-delta reference starts at the floor
        _ = try? engine.historySeedScene(projectRootPath: projectRootPath, sceneID: sceneID, sceneText: baseline)
    }

    // The editor calls this after applying an undo/redo result so the
    // whitespace-delta reference tracks the newly-restored scene text.
    func syncCommittedText(_ text: String) { lastCommittedText = text }

    // Whether there is an uncommitted edit awaiting a trigger.
    var hasPendingChanges: Bool { pendingSceneID != nil }

    // Paste/cut boundary (§4.a triggers #3/#4). The editor calls this immediately
    // BEFORE a paste or cut mutates text: it commits whatever was pending as its
    // own event, so the paste/cut lands as a distinct, separately-undoable event.
    // The insertion/removal itself is then captured by the next flush. `kind`
    // ("paste" or "cut") labels the resulting event.
    func beginPasteOrCut(kind: String) {
        guard isOpen else { return }
        flush(trigger: kind)      // close out prior typing as its own step
        nextEventKind = kind
    }

    // Cut-into-buffer boundary (EP-019 SP-056, Trade T3). Like beginPasteOrCut with
    // kind "cut", but also tags the resulting event with the copy-buffer slot the
    // cut fed, so the history node records which buffer the text went to. The tag is
    // metadata only — undo still restores the text exactly as a plain cut would.
    func beginCutIntoBuffer(bufferID: String) {
        guard isOpen else { return }
        flush(trigger: "cut")     // close out prior typing as its own step
        nextEventKind = "cut"
        nextBufferID = bufferID
    }

    // Kind label to apply to the next committed event (set by beginPasteOrCut,
    // consumed on the next flush). Defaults back to "typing".
    private var nextEventKind: String?

    // bufferID to tag the next committed event with (set by beginCutIntoBuffer,
    // consumed on the next flush). nil for every event that isn't a cut-into-buffer.
    private var nextBufferID: String?

    // MARK: — Commit triggers (§4.a)

    // Commits the pending edit (if any) as one event with the given trigger
    // label. `kind` distinguishes paste/cut from typing/delete.
    //
    // `soft` triggers (cursor move, scene switch) defer a pending change that is
    // *only whitespace* since the last commit — so double-spacing after a period,
    // or trailing spaces, never become their own undo step; the whitespace rides
    // into the next event once real text follows. `soft == false` (sentence
    // terminator, paste/cut, flush-before-save, close) always commits.
    //
    // A no-op when nothing is pending or history is closed.
    @discardableResult
    func flush(trigger: String, kind: String = "typing", soft: Bool = false) -> Bool {
        guard isOpen, let sceneID = pendingSceneID else {
            nextEventKind = nil   // nothing pending — drop any stale kind hint
            nextBufferID = nil
            return false
        }
        // Defer a whitespace-only pending change on a soft trigger.
        if soft && nextEventKind == nil && isWhitespaceOnlyDelta(from: lastCommittedText, to: pendingText) {
            return false
        }
        // A pending paste/cut kind hint (from beginPasteOrCut) wins over the
        // caller's default "typing".
        let effectiveKind = nextEventKind ?? kind
        let effectiveBufferID = nextBufferID
        nextEventKind = nil
        nextBufferID = nil
        do {
            let r = try engine.historyRecordEvent(
                projectRootPath: projectRootPath,
                sceneID: sceneID,
                newSceneText: pendingText,
                kind: effectiveKind,
                cursorBefore: Int64(pendingCursorBefore),
                cursorAfter: Int64(pendingCursorAfter),
                bufferID: effectiveBufferID)
            engineCanUndo = r.canUndo
            engineCanRedo = r.canRedo
            if !r.noOp {
                lastCommittedText = pendingText
                bumpRevision()   // I-0105: a real commit — the card must re-fetch
            }
            clearPending()
            endContinuation()   // T-0396: entry sealed — the session clock resets
            return !r.noOp
        } catch {
            print("[Scrivi] historyRecordEvent failed: \(error)")
            clearPending()
            return false
        }
    }

    // True if `a` → `b` differs only in whitespace (space/tab/newline) — i.e. the
    // non-whitespace content is identical. Used to defer whitespace-only commits.
    private func isWhitespaceOnlyDelta(from a: String, to b: String) -> Bool {
        func stripWhitespace(_ s: String) -> String {
            String(s.unicodeScalars.filter { !CharacterSet.whitespacesAndNewlines.contains($0) })
        }
        return stripWhitespace(a) == stripWhitespace(b)
    }

    // The §4.d invariant: any code path that writes a scene file must first
    // flush the pending history commit, so the file on disk always equals a
    // recorded history node. Returns after the commit; the caller then saves.
    //
    // T-0396: a save must NOT end the typing session. Before this, the 1 s autosave
    // debounce sealed an entry every time the writer paused for a second, which is
    // what split one continuously-typed sentence into three rows — including a break
    // mid-word ("…made glo" / "rious…"), the signature of a wall-clock timer rather
    // than any intentional boundary.
    //
    // The commit is DEFERRED rather than recorded-and-reopened: `HistoryService::record`
    // always appends a node with no amend path, so recording at save time would still
    // have cost the writer one ⌘Z stop per save. Design §4.a.1 / §4.d.
    func flushThenSave() {
        // T-0396 — the save no longer seals the typing session. While the writer is
        // mid-session the pending edit stays open and NO event is recorded; the
        // scene is still written to disk by the caller. The entry commits once at a
        // genuine boundary: the idle timer, or any of the triggers AC2 names
        // (cursor move, sentence terminator, paste/cut, scene switch, close), all
        // of which are KEPT (user ruling 2026-08-07).
        //
        // If the writer has already gone idle past the threshold, this save IS the
        // boundary — commit now rather than leaving the entry dangling.
        if isSessionIdle {
            flush(trigger: "idle")
            endContinuation()
            return
        }
        // Otherwise: deliberately no flush. See `deferredCommitNote` below for the
        // §4.d consequence and why it is safe.
    }

    // ⚠️ §4.d — relaxed by T-0396, deliberately and with the user's approval.
    //
    // The invariant used to read: disk never contains text that no history node
    // describes. Deferring the commit means disk can now LEAD history by at most
    // one save's worth of typing while a session is open.
    //
    // Why that is safe: the pending text lives in `pendingText` and is committed at
    // every boundary including project close (`close()` flushes). The only window
    // where it is lost is a hard crash mid-session — and in that case the next open
    // compares the scene's on-disk bytes against the persisted head hash (I-0104)
    // and records an `externalChange` barrier, so undo stops at the last node it can
    // honestly describe rather than walking past text it never recorded. The failure
    // mode is "undo stops early", never "undo corrupts the manuscript".
    //
    // T-0217 carries the AC2 + design §4.a/§4.d amendment documenting this.
    private var deferredCommitNote: Void { () }

    // MARK: — Undo / Redo (T-0205 — the editor applies the returned change)

    func undo() -> HistoryStepResult? {
        guard isOpen else { return nil }
        // Any uncommitted edit must become an event before we walk backwards,
        // otherwise the newest keystrokes would be silently discarded.
        flush(trigger: "flush")
        do {
            let r = try engine.historyUndo(projectRootPath: projectRootPath)
            engineCanUndo = r.canUndo
            engineCanRedo = r.canRedo
            bumpRevision()   // I-0105: the current-node pointer moved
            return r
        } catch { print("[Scrivi] historyUndo failed: \(error)"); return nil }
    }

    func redo() -> HistoryStepResult? {
        guard isOpen else { return nil }
        do {
            let r = try engine.historyRedo(projectRootPath: projectRootPath)
            engineCanUndo = r.canUndo
            engineCanRedo = r.canRedo
            bumpRevision()   // I-0105: the current-node pointer moved
            return r
        } catch { print("[Scrivi] historyRedo failed: \(error)"); return nil }
    }

    // Re-primaries a fork so the next redo walks `childEventID`'s branch
    // (SP-055 / §5, §7, T-0211 fork popover). Does not move the pointer; the
    // caller redoes afterwards to actually step onto the chosen branch. Returns
    // false on failure (e.g. childEventID is not a child of the fork).
    @discardableResult
    func selectBranch(forkNodeID: String, childEventID: String) -> Bool {
        guard isOpen else { return false }
        do {
            let r = try engine.historySelectBranch(
                projectRootPath: projectRootPath, forkNodeID: forkNodeID, childEventID: childEventID)
            engineCanRedo = r.canRedo
            if r.ok { bumpRevision() }   // I-0105: the primary spine changed
            return r.ok
        } catch { print("[Scrivi] historySelectBranch failed: \(error)"); return false }
    }

    // MARK: — Save-path notification (I-0104)

    // Record the exact bytes the save path just wrote for `sceneID`, so the head
    // hash persisted at close describes the file rather than the replayed history
    // head. Without this the next open compares head-to-disk and raises a spurious
    // `externalChange` barrier for a scene edited only inside Scrivi.
    //
    // Deliberately NOT a commit: this only tells history what landed on disk. It
    // does not seal the pending typing entry — that distinction is what T-0396
    // builds on.
    func noteScenePersisted(sceneID: String, diskText: String) {
        guard isOpen, !sceneID.isEmpty else { return }
        do {
            try engine.historyNoteScenePersisted(projectRootPath: projectRootPath,
                                                 sceneID: sceneID,
                                                 diskText: diskText)
        } catch { print("[Scrivi] historyNoteScenePersisted failed: \(error)") }
    }

    // MARK: — History tree (EP-030 SP-092, T-0395)

    // A windowed projection of the history graph for the history card. Returns nil
    // when history is closed or the call fails — the card renders its own notice
    // rather than the stack failing (Doc 2 §7.1). Best-effort, like its siblings.
    func tree(aroundNodeID: String? = nil, maxNodes: Int = 0) -> HistoryTreeResult? {
        guard isOpen else { return nil }
        do {
            return try engine.historyGetTree(projectRootPath: projectRootPath,
                                             aroundNodeID: aroundNodeID,
                                             maxNodes: maxNodes)
        } catch { print("[Scrivi] historyGetTree failed: \(error)"); return nil }
    }

    // MARK: — Stale branches (T-0212, §5)

    // Lists stale branches — non-primary subtrees older than the project's
    // staleBranchDays setting. Empty when history is closed, detection is disabled
    // (staleBranchDays <= 0), or nothing is stale. Best-effort.
    func listStaleBranches() -> [HistoryStaleBranch] {
        guard isOpen else { return [] }
        do { return try engine.historyListStaleBranches(projectRootPath: projectRootPath).branches }
        catch { print("[Scrivi] historyListStaleBranches failed: \(error)"); return [] }
    }

    // Purges a stale branch after user confirmation; refreshes redo-state on
    // success (purge can drop the primary line of a fork). Returns false on
    // failure or rejection (e.g. the branch is on the live path). Best-effort.
    @discardableResult
    func purgeStaleBranch(branchRootEventID: String) -> Bool {
        guard isOpen else { return false }
        do {
            let r = try engine.historyPurgeBranch(
                projectRootPath: projectRootPath, branchRootEventID: branchRootEventID)
            if r.ok {
                engineCanUndo = r.canUndo
                engineCanRedo = r.canRedo
                bumpRevision()   // I-0105: a subtree was erased
            }
            return r.ok
        } catch { print("[Scrivi] historyPurgeBranch failed: \(error)"); return false }
    }

    // Recomputes nothing itself — the can-state is refreshed from each engine
    // response above. Exposed so the editor can trigger a menu-validation refresh
    // after an apply that returned a barrier (which does not change the pointer).
    func refreshCanState() { /* state already current from the last engine call */ }

    // Brackets the editor's storage mutation so the resulting textDidChange is
    // not mistaken for a user edit. The closure runs synchronously.
    func withApplying(_ body: () -> Void) {
        isApplying = true
        body()
        isApplying = false
    }

    // MARK: — Barriers (T-0206, §4.5)

    // Records a structural barrier. Commits any pending edit first so the
    // barrier lands after the last text event. Best-effort.
    //
    // A non-nil `structuralPayload` records a REVERSIBLE structural node (T-0356 / AC6)
    // that undo/redo step across, handing the payload back for the editor to replay the
    // inverse fragment op; nil records a hard barrier (undo stops with a notice).
    func recordBarrier(kind: String, note: String,
                       sceneID: String? = nil,
                       structuralPayload: HistoryStructuralPayload? = nil) {
        guard isOpen else { return }
        // Capture the pending scene BEFORE flushing — flush() clears it, and it is the
        // best available attribution when the caller does not name a scene (I-0102).
        let attributed = sceneID ?? pendingSceneID
        flush(trigger: "flush")
        do {
            _ = try engine.historyRecordBarrier(projectRootPath: projectRootPath,
                                                barrierKind: kind, note: note,
                                                sceneID: attributed,
                                                structuralPayload: structuralPayload)
            bumpRevision()   // I-0105: a barrier node was appended
        }
        catch { print("[Scrivi] historyRecordBarrier failed: \(error)") }
    }

    // MARK: — Private

    private func clearPending() {
        pendingSceneID = nil
        pendingText = ""
        pendingCursorBefore = 0
        pendingCursorAfter = 0
    }
}

// Formats an ISO-8601 history timestamp for the session-boundary warning
// (T-0209) — e.g. "yesterday at 9:42 PM" / "on Jul 3 at 2:15 PM".
enum HistoryTimestamp {
    static func friendly(_ iso8601: String) -> String? {
        let parser = ISO8601DateFormatter()
        parser.formatOptions = [.withInternetDateTime]
        guard let date = parser.date(from: iso8601)
            ?? { parser.formatOptions = [.withInternetDateTime, .withFractionalSeconds]; return parser.date(from: iso8601) }()
        else { return nil }

        let cal = Calendar.current
        let time = DateFormatter()
        time.timeStyle = .short
        time.dateStyle = .none
        let clock = time.string(from: date)

        if cal.isDateInToday(date)     { return "earlier today at \(clock)" }
        if cal.isDateInYesterday(date) { return "yesterday at \(clock)" }
        let day = DateFormatter()
        day.dateFormat = "MMM d"
        return "on \(day.string(from: date)) at \(clock)"
    }
}
