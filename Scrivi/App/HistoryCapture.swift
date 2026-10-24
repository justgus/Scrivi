import Foundation

// HistoryCapture — the Swift side of EP-019 undo/redo (SP-053, T-0204/T-0205/T-0206). One instance per open project,
// owned by ProjectSession.
//
// ✅ SP-177 / T-0604 (EP-026 S1): WHEN an undo step ends is ScriviCore's (`scrivi_capture_*`, `history/CaptureSession`) —
// the latch, floor seeding, the 45 s idle rule, the sentence boundary, the soft whitespace deferral, scene switch vs caret
// move, paste/cut labels, autosave-never-seals, grouped edits. They were this file's (and the text view's
// `isCommitBoundary`) until SP-177; the parity fixture recorded from them is replayed by a core test
// (`ScriviCore/tests/fixtures/history_capture_parity.jsonl`). This type keeps only PLUMBING:
//  - it reports what happened (an edit, a caret move, a paste/cut, a save, an applied undo) with a monotonic `nowMs`;
//  - it runs the idle TIMER at the deadline the core returns (SP-177 Q1: the core never owns a clock or timer);
//  - `isApplying` around an undo apply, so the text change it causes is not reported as an edit;
//  - `revision` for the history card; the read-only history calls (tree, stale branches, purge, select branch).
// Carets are UTF-16 units of the scene text (SP-177 Q2).
//
// Concurrency: @MainActor — created and used entirely on the main actor. The C side is thread-safe.
@MainActor
final class HistoryCapture {

    private let engine: ScriviEngine
    private let projectRootPath: String

    // True once historyOpen has succeeded and false after close, so notifications and undo/redo become no-ops when
    // history is unavailable.
    private(set) var isOpen: Bool = false

    // Guards re-entrancy: while an undo/redo apply is in flight we must not treat the resulting textDidChange as a new
    // user edit.
    private(set) var isApplying: Bool = false

    // I-0105: bumped on every mutation of the history tree (commit, undo, redo, barrier, purge). The history card folds
    // this into its `.task(id:)` so a commit re-fetches the tree.
    private(set) var revision: Int = 0

    private func bumpRevision() { revision &+= 1 }

    // The core's last report: can-undo includes a pending edit; the idle deadline drives the timer.
    private var state: CaptureState?
    var canUndoNow: Bool { state?.canUndo ?? false }
    /// [I-0302]: the newest step is a hard barrier — Undo stays available so ⌘Z shows its notice.
    var atBarrierNow: Bool { state?.atBarrier ?? false }
    var canRedoNow: Bool { state?.canRedo ?? false }
    var hasPendingChanges: Bool { state?.hasPending ?? false }
    /// [SP-192]: the branch point the last report named (nil while an edit is pending) — the margin mark's.
    var redoForkNow: HistoryForkAhead? { state?.redoFork }
    /// [SP-192]: told after every report, so the margin mark follows the history pointer.
    var onReport: (@MainActor () -> Void)?

    // A monotonic millisecond clock (SP-177 Q1) — the same one the deadline is in.
    private static func nowMs() -> Int64 { Int64(ProcessInfo.processInfo.systemUptime * 1000) }

    // Fires the idle boundary on its own, at the core's deadline — so a writer who stops mid-sentence and walks away
    // still gets their last sentence sealed. Re-armed after every report; a stale one does nothing (the core checks).
    private var idleTask: Task<Void, Never>?

    init(engine: ScriviEngine, projectRootPath: String) {
        self.engine = engine
        self.projectRootPath = projectRootPath
    }

    /// Takes the core's report: re-arms the idle timer and bumps the revision when an event was committed.
    @discardableResult
    private func take(_ r: CaptureState?) -> Bool {
        guard let r else { return false }
        state = r
        onReport?()
        if r.committed { bumpRevision() }
        idleTask?.cancel()
        idleTask = nil
        if r.idleDeadlineMs >= 0 {
            let delay = max(0, r.idleDeadlineMs - Self.nowMs())
            idleTask = Task { @MainActor [weak self] in
                try? await Task.sleep(nanoseconds: UInt64(delay) * 1_000_000)
                guard !Task.isCancelled, let self, self.isOpen else { return }
                self.take(try? self.engine.captureIdle(projectRootPath: self.projectRootPath, nowMs: Self.nowMs()))
            }
        }
        return r.committed
    }

    // MARK: — Lifecycle

    func open() {
        guard !isOpen else { return }
        do {
            _ = try engine.historyOpen(projectRootPath: projectRootPath)
            isOpen = true
            take(try? engine.captureState(projectRootPath: projectRootPath))
        } catch {
            // History is best-effort; a failure here must never block editing.
            isOpen = false
            print("[Scrivi] historyOpen failed: \(error)")
        }
    }

    // Validates each loaded scene's on-disk text against the head hash persisted at last close (§6.b). A scene edited
    // outside Scrivi between sessions gets an externalChange barrier + floor re-seed in the engine — never touching the
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
        // `scrivi_history_close` commits the pending typing first, so the final sentence is not lost (SP-177).
        //
        // ⚠️ DO NOT call noteScenePersisted() here with the pending text. That was tried on 2026-08-10 and made things
        // strictly worse (the writer's barrier count went 7 → 8 on a single relaunch): on quit the pending text may
        // never have been written at all, so recording its hash guarantees a mismatch → barrier, permanently.
        // `noteScenePersisted` has exactly one correct caller: the save path, AFTER a successful write, with the bytes it
        // actually wrote (`ViewportSceneLoader.saveScene`).
        idleTask?.cancel()
        idleTask = nil
        _ = try? engine.historyClose(projectRootPath: projectRootPath)
        isOpen = false
        state = nil
    }

    // MARK: — Reports (called by the text coordinator)

    // The writer changed `sceneID`; `text` is its full current text, `caretUTF16` the caret after the edit,
    // `textBefore` the scene before it (its floor, the first time). The core latches it and decides whether it ends a
    // step (a sentence terminator or Return; a long idle gap before it).
    func noteEdit(sceneID: String, text: String, caretUTF16: Int, textBefore: String) {
        guard isOpen, !isApplying else { return }
        take(try? engine.captureNoteEdit(projectRootPath: projectRootPath, sceneID: sceneID, text: text,
                                         caretUTF16: caretUTF16, textBefore: textBefore, nowMs: Self.nowMs()))
    }

    // The caret moved into `sceneID` (another scene commits; the same scene commits unless only whitespace changed).
    // `editInFlight`: the selection change came from an edit this turn. `applying`: an undo/redo apply moved it.
    func caretMoved(sceneID: String, editInFlight: Bool, applying: Bool) {
        guard isOpen else { return }
        take(try? engine.captureCaretMoved(projectRootPath: projectRootPath, sceneID: sceneID,
                                           editInFlight: editInFlight, applying: applying))
    }

    // Paste/cut boundary (§4.a triggers #3/#4): call immediately BEFORE a paste or cut mutates text; follow the change
    // with `flush(trigger: kind, kind: kind)`.
    func beginPasteOrCut(kind: String) {
        guard isOpen else { return }
        take(try? engine.captureBegin(projectRootPath: projectRootPath, kind: kind))
    }

    // Cut-into-buffer boundary (EP-019 SP-056, Trade T3): as a cut, tagged with the copy-buffer slot it fed.
    func beginCutIntoBuffer(bufferID: String) {
        guard isOpen else { return }
        take(try? engine.captureBegin(projectRootPath: projectRootPath, kind: "cut", bufferID: bufferID))
    }

    // Commits the pending edit (if any) now, as `kind` unless a paste/cut label is waiting.
    @discardableResult
    func flush(trigger: String, kind: String = "typing") -> Bool {
        guard isOpen else { return false }
        return take(try? engine.captureFlush(projectRootPath: projectRootPath, trigger: trigger, kind: kind))
    }

    /// One scene's part of an edit that changed several scenes at once (I-0270). `caret`: UTF-16 in `textBefore`,
    /// where the edit began.
    struct GroupedSceneEdit {
        let sceneID: String
        let textBefore: String
        let textAfter: String
        let caret: Int
    }

    /// ONE edit that changed several scenes (⌫ / ⌦ / ⌘X across scene breaks, Replace All): one undo step.
    func recordGroupedEdit(_ edits: [GroupedSceneEdit], kind: String) {
        guard isOpen, !isApplying, !edits.isEmpty else { return }
        take(try? engine.captureGroupedEdit(
            projectRootPath: projectRootPath,
            edits: edits.map { CaptureGroupEdit(sceneID: $0.sceneID, textBefore: $0.textBefore, textAfter: $0.textAfter,
                                                caret: $0.caret) },
            kind: kind))
    }

    // An autosave is about to write. T-0396: it never seals the typing session unless the writer is already idle
    // (design §4.a.1, §4.d — disk may lead history by at most one open session; close and the idle timer commit it).
    func flushThenSave() {
        guard isOpen else { return }
        take(try? engine.captureBeforeSave(projectRootPath: projectRootPath, nowMs: Self.nowMs()))
    }

    // MARK: — Undo / Redo (T-0205 — the editor applies the returned change)

    /// [I-0319]: when the next undo would undo an earlier session's change, that change's time (nil: it would not). Asked
    /// fresh — a structural step records outside `scrivi_capture_*`, so the last report can be stale.
    func undoEntersEarlierSession() -> String? {
        guard isOpen else { return nil }
        take(try? engine.captureState(projectRootPath: projectRootPath))
        return state?.undoCrossesSessionAt
    }

    /// [I-0320]: the branches the next redo could take, when there are two or more (nil: a plain redo). Asked fresh, as above.
    func redoFork() -> HistoryForkAhead? {
        guard isOpen else { return nil }
        take(try? engine.captureState(projectRootPath: projectRootPath))
        return state?.redoFork
    }

    func undo() -> HistoryStepResult? {
        guard isOpen else { return nil }
        // The core commits any pending edit first, so the newest keystrokes are not discarded.
        do {
            let r = try engine.captureUndo(projectRootPath: projectRootPath)
            take(try? engine.captureState(projectRootPath: projectRootPath))
            bumpRevision()   // I-0105: the current-node pointer moved
            return r
        } catch { print("[Scrivi] captureUndo failed: \(error)"); return nil }
    }

    func redo() -> HistoryStepResult? {
        guard isOpen else { return nil }
        do {
            let r = try engine.captureRedo(projectRootPath: projectRootPath)
            take(try? engine.captureState(projectRootPath: projectRootPath))
            bumpRevision()   // I-0105: the current-node pointer moved
            return r
        } catch { print("[Scrivi] captureRedo failed: \(error)"); return nil }
    }

    // After applying an undo/redo: the caret rests in `sceneID`, which now reads `text` (the whitespace reference).
    func noteApplied(sceneID: String, text: String) {
        guard isOpen else { return }
        take(try? engine.captureNoteApplied(projectRootPath: projectRootPath, sceneID: sceneID, text: text))
    }

    // Re-primaries a fork so the next redo walks `childEventID`'s branch (SP-055 / §5, §7, T-0211 fork popover). Does not
    // move the pointer; the caller redoes afterwards. Returns false on failure.
    @discardableResult
    func selectBranch(forkNodeID: String, childEventID: String) -> Bool {
        guard isOpen else { return false }
        do {
            let r = try engine.historySelectBranch(
                projectRootPath: projectRootPath, forkNodeID: forkNodeID, childEventID: childEventID)
            take(try? engine.captureState(projectRootPath: projectRootPath))
            if r.ok { bumpRevision() }   // I-0105: the primary spine changed
            return r.ok
        } catch { print("[Scrivi] historySelectBranch failed: \(error)"); return false }
    }

    // MARK: — Save-path notification (I-0104)

    // Record the exact bytes the save path just wrote for `sceneID`, so the head hash persisted at close describes the
    // file. Deliberately NOT a commit: it does not seal the pending typing entry (T-0396).
    func noteScenePersisted(sceneID: String, diskText: String) {
        guard isOpen, !sceneID.isEmpty else { return }
        do {
            try engine.historyNoteScenePersisted(projectRootPath: projectRootPath, sceneID: sceneID, diskText: diskText)
        } catch { print("[Scrivi] historyNoteScenePersisted failed: \(error)") }
    }

    // MARK: — History tree (EP-030 SP-092, T-0395)

    // A windowed projection of the history graph for the history card. nil when history is closed or the call fails.
    func tree(aroundNodeID: String? = nil, maxNodes: Int = 0) -> HistoryTreeResult? {
        guard isOpen else { return nil }
        do {
            return try engine.historyGetTree(projectRootPath: projectRootPath, aroundNodeID: aroundNodeID, maxNodes: maxNodes)
        } catch { print("[Scrivi] historyGetTree failed: \(error)"); return nil }
    }

    // MARK: — Stale branches (T-0212, §5)

    func listStaleBranches() -> [HistoryStaleBranch] {
        guard isOpen else { return [] }
        do { return try engine.historyListStaleBranches(projectRootPath: projectRootPath).branches }
        catch { print("[Scrivi] historyListStaleBranches failed: \(error)"); return [] }
    }

    @discardableResult
    func purgeStaleBranch(branchRootEventID: String) -> Bool {
        guard isOpen else { return false }
        do {
            let r = try engine.historyPurgeBranch(projectRootPath: projectRootPath, branchRootEventID: branchRootEventID)
            if r.ok {
                take(try? engine.captureState(projectRootPath: projectRootPath))
                bumpRevision()   // I-0105: a subtree was erased
            }
            return r.ok
        } catch { print("[Scrivi] historyPurgeBranch failed: \(error)"); return false }
    }

    // The can-state is current from the last report; exposed so the editor can trigger a menu-validation refresh after
    // an apply that returned a barrier (which does not change the pointer).
    func refreshCanState() { /* state already current from the last report */ }

    // Brackets the editor's storage mutation so the resulting textDidChange is not mistaken for a user edit.
    func withApplying(_ body: () -> Void) {
        isApplying = true
        body()
        isApplying = false
    }

    // MARK: — [SP-191] A structural edit records its node OFF the main actor

    /// The project root a structural edit records its history node against, off the main actor (the C side is
    /// thread-safe); nil while history is not open.
    var offMainRoot: String? { isOpen ? projectRootPath : nil }

    /// Takes the core's report of a node recorded off the main actor, as `recordBarrier` takes its own.
    func adopt(_ r: CaptureState) {
        guard isOpen else { return }
        take(r)
        bumpRevision()
    }

    // MARK: — Barriers (T-0206, §4.5)

    // Records a structural barrier; the core commits any pending edit first and, with no `sceneID`, attributes it to the
    // pending scene (I-0102). A non-nil `structuralPayload` records a REVERSIBLE structural node (T-0356 / AC6).
    // `sceneTexts` ([I-0287]): sceneID → the text the op left in each scene it rewrote or created, exactly as the editor
    // will next report it — so the next edit's undo returns to THAT text, not the pre-op one. ⚠️ Record it AFTER the op.
    // `sceneTextsBefore` ([I-0293], a reversible structural node only): what each scene that EXISTED before the op held
    // then — undo crosses the node, and the scene reads that text again.
    func recordBarrier(kind: String, note: String,
                       sceneID: String? = nil,
                       structuralPayload: HistoryStructuralPayload? = nil,
                       sceneTexts: [String: String] = [:],
                       sceneTextsBefore: [String: String] = [:]) {
        guard isOpen else { return }
        do {
            take(try engine.captureRecordBarrier(projectRootPath: projectRootPath, barrierKind: kind, note: note,
                                                 sceneID: sceneID, structuralPayload: structuralPayload,
                                                 sceneTexts: sceneTexts, sceneTextsBefore: sceneTextsBefore))
            bumpRevision()   // I-0105: a barrier node was appended
        } catch { print("[Scrivi] captureRecordBarrier failed: \(error)") }
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
