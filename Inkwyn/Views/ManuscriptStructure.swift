import Foundation
#if os(macOS)
import AppKit
#else
import UIKit
#endif

// MARK: — C3: undo/redo, structural edits and the cross-scene clipboard (EP-055 S3a, SP-222, T-0685)
//
// ✅ Q2 (user, 2026-11-18): MOVED out of the Mac `Coordinator`, as C1 and C2 were in [SP-198] — so the Mac and iOS cannot drift
// (the [EP-049] defect). What the history applies, which range call an edit makes and what a copy holds are decided HERE;
// each host only does what its view must: the caret, the scroll, the render, alerts, the pasteboard (`ManuscriptStructureHost`).

/// What a host's view does for `ManuscriptStructure`.
@MainActor
protocol ManuscriptStructureHost: AnyObject {
    var structureStorage: NSTextStorage? { get }
    var structureSelection: NSRange { get }
    /// The cue (SP-191 Q5): the editor stops taking input while a structural edit is in flight.
    func structureSetEditable(_ editable: Bool)
    /// A refused or failed edit (the Mac beeps).
    func structureRefuse()
    /// A paste refused at a chapter title (the Mac flashes).
    func structureFlashRefuse()
    /// Re-render the run `old` → `new` of the scene list (or everything).
    func structureRenderRun(old: Range<Int>, new: Range<Int>)
    /// The caret after a structural edit lands, kept at its height in the viewport and shown.
    func structureRevealCaret(_ loc: Int)
    /// The caret after an undo's text is applied; `reveal` scrolls to it.
    func structurePlaceCaret(_ loc: Int, reveal: Bool)
    /// A disk correction: the scroll kept across `body`.
    func structurePreservingScroll(_ body: () -> Void)
    /// After a structural edit lands (the Mac's branch marks).
    func structureLanded()
    /// [I-0319] Before an undo crosses into an earlier session.
    func structureConfirmEarlierSessionUndo(_ timestamp: String, _ answer: @escaping @MainActor (Bool) -> Void)
    /// [I-0302] The undo stopped at a barrier.
    func structureBarrierNotice(_ message: String)
    /// Q4: redo at a branch asks which; `choose` with the branch's eventID. Dismissed: nothing.
    func structureChooseBranch(_ fork: HistoryForkAhead, _ choose: @escaping @MainActor (String) -> Void)
    func structureCloseBranchChooser()
    /// Delete a one-scene selection as a typed edit would (recorded, autosaved).
    func structureDeleteSelection(_ sel: NSRange)
    var structurePasteboardChangeCount: Int { get }
    /// Other apps get `presented`; returns the pasteboard's change count after the write.
    func structureWritePasteboard(_ presented: String) -> Int
    /// Insert `source` (STORED form, Scrivi's own copy: it keeps its own formatting) over the selection, as a typed edit.
    func structureInsertOwnSource(_ source: String)
}

@MainActor
final class ManuscriptStructure {
    let sync: ManuscriptSceneSync
    weak var host: (any ManuscriptStructureHost)?

    init(sync: ManuscriptSceneSync) { self.sync = sync }

    private var loader: ViewportSceneLoader { sync.loader }
    private var session: ProjectSession { sync.session }
    private var presenter: ManuscriptPresenter { sync.presenter }
    private var sceneBoundaries: [NSRange] {
        guard let storage = host?.structureStorage else { return [] }
        return sync.sceneBoundaries(in: storage)
    }
    private func segmentIndex(for loc: Int) -> Int? {
        guard let storage = host?.structureStorage else { return nil }
        return sync.segmentIndex(for: loc, in: storage)
    }

    // MARK: — Undo / Redo apply path (T-0205, design §8)

    var canUndo: Bool { session.historyCapture?.canUndoNow ?? false }
    /// ✅ [I-0302]: Undo is available at a barrier too — the undo then shows the barrier's notice ("This scene was
    /// changed outside Scrivi; undo stops here"). ⛔ Disabled on `canUndo` alone, ⌘Z only flashed and never said why.
    var undoAvailable: Bool { canUndo || (session.historyCapture?.atBarrierNow ?? false) }
    // EP-052 ([SP-195]): undo and redo are NOT held off while saves are queued — their own writes queue behind them.
    var canRedo: Bool { session.historyCapture?.canRedoNow ?? false }

    /// ✅ [SP-196] v0.2 Q5: an undo or redo pressed while a structural edit is in flight WAITS ITS TURN — it runs once
    /// that edit has landed (never cancels it); a failed edit drops the ones pressed for it.
    private var deferredSteps: [@MainActor () -> Void] = []

    func runDeferredSteps() {
        while !deferredSteps.isEmpty, !loader.structuralInFlight { deferredSteps.removeFirst()() }
    }

    // Performs an undo: asks HistoryService for the prior state, applies the returned full scene text into that scene's
    // storage range, restores the cursor, and saves immediately.
    func performUndo() {
        let span = ScriviDiag.begin("undo"); defer { ScriviDiag.end(span) }
        if loader.structuralInFlight { return deferredSteps.append { [weak self] in self?.performUndo() } }
        guard let capture = session.historyCapture else { return }
        // ✅ [I-0319] (user, 2026-10-24): asked BEFORE an undo of an earlier session's change, with Cancel — never for
        // this session's.
        if let when = capture.undoEntersEarlierSession(), let host {
            host.structureConfirmEarlierSessionUndo(when) { [weak self] ok in
                guard ok, let self, let capture = self.session.historyCapture else { return }
                self.apply(step: capture.undo())
            }
            return
        }
        apply(step: capture.undo())
    }

    func performRedo() {
        let span = ScriviDiag.begin("redo"); defer { ScriviDiag.end(span) }
        if loader.structuralInFlight { return deferredSteps.append { [weak self] in self?.performRedo() } }
        guard let capture = session.historyCapture else { return }
        // ✅ [I-0320] (user, 2026-10-24): the branch chooser is a REDO question — shown here, never as an undo lands.
        if let fork = capture.redoFork(), let host {
            host.structureChooseBranch(fork) { [weak self] child in self?.redoBranch(child, of: fork) }
            return
        }
        apply(step: capture.redo())
    }

    // Re-primary the fork, then redo onto the chosen branch.
    func redoBranch(_ childEventID: String, of fork: HistoryForkAhead) {
        guard let capture = session.historyCapture,
              capture.selectBranch(forkNodeID: fork.nodeID, childEventID: childEventID) else { return }
        apply(step: capture.redo())
    }

    // Applies a step result (undo or redo). A step may be blocked by a structural barrier (§4.5) — in that case we surface
    // the notice and do not mutate text.
    private func apply(step: HistoryStepResult?) {
        guard let step, let capture = session.historyCapture else { return }

        if let barrier = step.stoppedAtBarrier {
            host?.structureBarrierNotice(barrier.note.isEmpty ? "Can't undo past this point." : barrier.note)
            capture.refreshCanState()
            return
        }

        // Reversible structural step: the pointer crossed a structural node. ✅ [SP-191]: the inverse range call, off the
        // main actor, the delta applied — no reload.
        if let inverse = step.structuralInverse {
            host?.structureCloseBranchChooser()
            applyStructuralInverse(inverse)
            capture.refreshCanState()
            return
        }

        guard step.moved, !step.changes.isEmpty, let host, let storage = host.structureStorage else {
            // A step that did not move (nothing left to undo/redo) still means we are no longer on the previous fork.
            host?.structureCloseBranchChooser()
            capture.refreshCanState()
            return
        }

        // ⚠️ [I-0270] A step can carry SEVERAL scene changes — an edit group (a delete or cut across scene breaks) undoes /
        // redoes as ONE step: every change is applied, then the caret goes to the EARLIEST scene the step touched.
        // ⚠️ [SP-164] live pass: undoing a Replace All across ~880 scenes scrolled to each scene in turn — a multi-scene step
        // places the caret once, below.
        let single = step.changes.count == 1
        var placed: [(segIdx: Int, caret: Int)] = []
        for change in step.changes {
            if let p = applySceneChange(change, storage: storage, capture: capture, placesCaret: single) { placed.append(p) }
        }
        if !single, let first = placed.min(by: { $0.segIdx < $1.segIdx }) {
            host.structurePlaceCaret(first.caret, reveal: true)
            if loader.segments.indices.contains(first.segIdx) {
                let seg = loader.segments[first.segIdx]
                capture.noteApplied(sceneID: seg.sceneID, text: seg.text)
            }
        }
        capture.refreshCanState()
        // [I-0320]: a step moves off any fork the chooser was offered for; the next redo asks again.
        host.structureCloseBranchChooser()
    }

    // Applies ONE scene's undo/redo text into its storage range, places the caret, syncs the loader and saves. Returns the
    // segment and caret it used, or nil when the scene is not loaded.
    private func applySceneChange(_ change: HistorySceneChange, storage: NSTextStorage, capture: HistoryCapture,
                                  placesCaret: Bool) -> (segIdx: Int, caret: Int)? {
        guard let segIdx = loader.segments.firstIndex(where: { $0.sceneID == change.sceneID }) else { return nil }
        let bounds = sync.sceneBoundaries(in: storage)
        guard bounds.indices.contains(segIdx) else { return nil }
        var caret = bounds[segIdx].location

        // The full scene text into the scene's range under the apply guard, so the text-changed callback does not record
        // it as an edit. Dividers and headings sit outside the range and are untouched.
        capture.withApplying {
            // ✅ EP-047 AC4: the ONE source of the body attributes (I-0112: they carry the colour — no black in Dark Mode).
            storage.replaceCharacters(in: bounds[segIdx],
                                      with: NSAttributedString(string: change.newText, attributes: presenter.typography.bodyAttributes))
            let after = sync.sceneBoundaries(in: storage)
            if after.indices.contains(segIdx) {
                let sceneText = (storage.string as NSString).substring(with: after[segIdx])
                // SP-177 Q2: the core reports the caret in UTF-16 too.
                let charOffset = change.cursorAfterUtf16.map(Int.init)
                    ?? Self.charOffset(forByteOffset: Int(change.cursorAfter), in: sceneText)
                caret = min(after[segIdx].location + charOffset, storage.length)
                if placesCaret { host?.structurePlaceCaret(caret, reveal: true) }
            }
        }

        // The loader's in-memory text in sync, and saved at once.
        loader.updateText(change.newText, at: segIdx)
        // The restored text is now the whitespace-delta reference (SP-177: told to the core).
        if placesCaret { capture.noteApplied(sceneID: change.sceneID, text: change.newText) }
        let env = sync.env, loader = loader
        if let ref = session.authorshipRef {
            Task { @MainActor in await loader.saveScene(at: segIdx, engine: env.engine, ref: ref) }
        }
        return (segIdx, caret)
    }

    // ✅ [SP-191] Undo and redo of a structural node: the inverse range call, through the same off-main path as the edit
    // itself — the delta applied, one run rendered, nothing re-opened. Each node holds the ONE Range both directions use
    // (see `HistoryStructuralPayload`):
    //   rangePaste    undo = cut what it inserted          redo = paste that again at its caret
    //   rangeCut      undo = paste what it removed back at the survivor's seam (sidecars restored, Q3b)
    //                 redo = cut that again (the undo restored it exactly)
    //   rangeRelabel  undo = relabel the run back into the chapter it left (re-created from its sidecar when it was
    //                 removed)                             redo = into the chapter it went to
    // ✅ [SP-187] Q2: the nodes written before SP-191 (`structuredPaste` / `structuredCut`, a scrivi.fragment.v1) are
    // CONVERTED to a Range and take the same path. ⚠️ An inverse the core refuses steps the history back, so the pointer
    // never says the manuscript is somewhere it is not.
    private func applyStructuralInverse(_ inverse: HistoryStructuralInverse) {
        let p = inverse.payload
        let undo = inverse.direction == "undo"
        let stepBack: @MainActor () -> Void = { [weak self] in
            guard let capture = self?.session.historyCapture else { return }
            _ = undo ? capture.redo() : capture.undo()
        }
        let refuse: @MainActor () -> Void = { [weak self] in self?.host?.structureRefuse(); stepBack() }
        switch p.op {
        case "rangePaste", "rangeCut", "rangeRelabel":
            guard let range = try? JSONDecoder().decode(ScriviRange.self, from: Data(p.rangeJSON.utf8)) else { return refuse() }
            let removes = (p.op == "rangePaste") == undo        // paste's undo and cut's redo both cut
            if p.op == "rangeRelabel" {
                let target = undo ? p.fromChapterID : p.targetChapterID
                let keep = caretInScene()?.seg.sceneID ?? range.scenes.first?.sceneID ?? ""
                inverseEdit({ engine, root, ref in
                    try engine.enqueueRangeRelabel(projectRootPath: root, range: range, targetChapterID: target,
                                                   authorshipRef: ref)
                }, caret: { _ in (keep, 0) }, failed: stepBack)
            } else if removes {
                inverseEdit({ engine, root, ref in
                    try engine.enqueueRangeCut(projectRootPath: root, range: range, authorshipRef: ref)
                }, caret: { r in r.undoCaretSceneID.map { ($0, r.undoCaretByte ?? 0) } }, failed: stepBack)
            } else {
                pasteInverse(range, at: p.caretSceneID, byte: p.caretByte, failed: stepBack)
            }
        case "structuredPaste":
            // The paste's undo is a cut of what it inserted — the fragment's pieces under the IDs it created.
            guard let inserted = oldPasteRange(p) else { return refuse() }
            if undo {
                inverseEdit({ engine, root, ref in
                    try engine.enqueueRangeCut(projectRootPath: root, range: inserted, authorshipRef: ref)
                }, caret: { r in r.undoCaretSceneID.map { ($0, r.undoCaretByte ?? 0) } }, failed: stepBack)
            } else {
                pasteInverse(inserted.normalized(), at: p.caretSceneID, byte: p.caretByte, failed: stepBack)
            }
        case "structuredCut":
            // The collapsing cut (retired by [I-0270]): its undo pastes the fragment back at the survivor; its redo cuts
            // what that paste inserted, remembered for this session.
            let key = "\(p.caretSceneID)|\(p.caretByte)|\(p.fragmentJSON.hashValue)"
            if undo {
                guard let range = Self.rangeFromFragment(p.fragmentJSON) else { return refuse() }
                pasteInverse(range, at: p.caretSceneID, byte: p.caretByte, failed: stepBack) { [weak self] r in
                    self?.oldCutRedo[key] = r.range
                }
            } else {
                guard let inserted = oldCutRedo.removeValue(forKey: key) else { return refuse() }
                inverseEdit({ engine, root, ref in
                    try engine.enqueueRangeCut(projectRootPath: root, range: inserted, authorshipRef: ref)
                }, caret: { r in r.undoCaretSceneID.map { ($0, r.undoCaretByte ?? 0) } }, failed: stepBack)
            }
        default:
            refuse()
        }
    }

    /// An old `structuredCut`'s redo needs what its undo's paste inserted — kept for this session only (as before).
    private var oldCutRedo: [String: ScriviRange] = [:]

    private func inverseEdit(_ call: @escaping StructuralCall,
                             caret: @escaping StructuralCaret, failed: @escaping @MainActor () -> Void) {
        if !runStructuralEdit("Undoing…", call, node: nil, caret: caret, failed: failed) { failed() }
    }

    /// A paste of `range` at (sceneID, byte), the caret at the end of what it put back.
    private func pasteInverse(_ range: ScriviRange, at sceneID: String, byte: Int, failed: @escaping @MainActor () -> Void,
                              inserted: (@MainActor (RangeOperationResult) -> Void)? = nil) {
        guard let text = loader.segments.first(where: { $0.sceneID == sceneID })?.text else { failed(); return }
        inverseEdit({ engine, root, ref in
            try engine.enqueueRangePaste(projectRootPath: root, range: range, caretSceneID: sceneID, caretByteOffset: byte,
                                         caretSceneText: text, authorshipRef: ref)
        }, caret: { r in
            inserted?(r)
            return r.range.scenes.last.map { ($0.sceneID, $0.start + $0.length) }
        }, failed: failed)
    }

    /// [SP-187] Q2: what an old `structuredPaste` inserted, as a Range — the fragment's pieces under the target and the
    /// scenes and chapters it created, each with its length as the scene list holds it now.
    private func oldPasteRange(_ p: HistoryStructuralPayload) -> ScriviRange? {
        guard let frag = try? JSONDecoder().decode(FragmentResult.self, from: Data(p.fragmentJSON.utf8)),
              !frag.pieces.isEmpty, frag.pieces.count == p.createdSceneIDs.count + 1 else { return nil }
        let byID = Dictionary(loader.segments.map { ($0.sceneID, $0) }, uniquingKeysWith: { a, _ in a })
        var chapter = byID[p.caretSceneID]?.chapterID ?? ""
        var created = p.createdChapterIDs.makeIterator()
        var entries: [ScriviRange.Entry] = []
        for (i, piece) in frag.pieces.enumerated() {
            let id = i == 0 ? p.caretSceneID : p.createdSceneIDs[i - 1]
            if piece.opensWith == "chapter" { chapter = created.next() ?? "" }
            let start = i == 0 ? p.caretByte : 0
            let length = piece.text.utf8.count
            entries.append(.init(sceneID: id, chapterID: byID[id]?.chapterID ?? chapter,
                                 chapterTitle: piece.chapterTitle ?? "", sceneTitle: piece.sceneTitle ?? "",
                                 start: start, length: length,
                                 sceneLength: byID[id]?.text.utf8.count ?? start + length, text: piece.text))
        }
        return ScriviRange(scenes: entries)
    }

    /// [SP-187] Q2: an old scrivi.fragment.v1 as a Range with no identity — a paste of it mints fresh IDs.
    static func rangeFromFragment(_ json: String) -> ScriviRange? {
        guard let frag = try? JSONDecoder().decode(FragmentResult.self, from: Data(json.utf8)), !frag.pieces.isEmpty
        else { return nil }
        var chapter = 0
        let entries = frag.pieces.map { piece -> ScriviRange.Entry in
            if piece.opensWith == "chapter" { chapter += 1 }
            return .init(sceneID: "", chapterID: "~fragment-chapter-\(chapter)", chapterTitle: piece.chapterTitle ?? "",
                         sceneTitle: piece.sceneTitle ?? "", start: 0, length: piece.text.utf8.count,
                         sceneLength: piece.text.utf8.count, text: piece.text)
        }
        return ScriviRange(scenes: entries)
    }

    // MARK: — [SP-191] Structural edits: ONE range call, off the main actor, the delta applied

    /// What a structural edit brings back from off the main actor: the core's answer, the history state after its node
    /// was recorded THERE, and the timeline's story times read THERE — so the main actor makes no core call for it.
    struct StructuralLanding: Sendable {
        let result: RangeOperationResult
        let history: CaptureState?
        let storyTimes: StoryTimesResult?
    }

    /// Queues a structural edit's range operation and returns its ticket ([SP-196] W7a): `engine.enqueueRange…`.
    typealias StructuralCall = @Sendable (ScriviEngine, String, AuthorshipRef) throws -> Int64
    /// The history node a structural edit records, built from the core's answer (off the main actor). An undo or redo
    /// passes nil: it crosses a node, it does not add one.
    typealias StructuralNode = @Sendable (RangeOperationResult) -> HistoryStructuralPayload?
    /// Where the caret goes once the delta is applied: a scene and a scene-local UTF-8 byte offset.
    typealias StructuralCaret = @MainActor (RangeOperationResult) -> (sceneID: String, byte: Int)?

    /// ✅ [SP-191] The ONE path every structural edit takes — paste, split, chapter split, merge, chapter merge, and the
    /// undo and redo of each (user, 2026-10-17: *"one range call, apply in memory"*):
    ///  1. the cue, in THIS event-loop turn (Q5): the editor stops taking input, so the caret goes;
    ///  2. off the main actor: the unsaved scenes written, the pending typing committed, the ONE range call, its history
    ///     node, the story times;
    ///  3. back on it: the delta applied to the scene list, ONE run rendered, the caret placed. Nothing re-opens the project.
    /// Returns false, doing nothing, while another is in flight — Q5: ignored, not queued.
    @discardableResult
    func runStructuralEdit(_ label: String,
                           _ call: @escaping StructuralCall,
                           node: StructuralNode?,
                           caret: @escaping StructuralCaret,
                           failed: (@MainActor () -> Void)? = nil,
                           then: (@MainActor (RangeOperationResult) -> Void)? = nil) -> Bool {
        let loader = loader
        guard !loader.structuralInFlight, let host,
              let root = session.projectRootPath, let ref = session.authorshipRef else { return false }
        let historyRoot = node == nil ? nil : session.historyCapture?.offMainRoot
        // What each scene held before — the node needs it for the scenes the edit rewrites or removes ([I-0293]).
        let before = Dictionary(loader.segments.map { ($0.sceneID, $0.text) }, uniquingKeysWith: { a, _ in a })
        host.structureSetEditable(false)                        // ✅ Q5: the cue — at once, before any core call
        let started = loader.runStructural(label, ref: ref, enqueue: { engine in
            if let historyRoot { _ = try? engine.captureFlush(projectRootPath: historyRoot, trigger: "flush") }
            return try call(engine, root, ref)
        }, finish: { engine, result -> StructuralLanding in
            var history: CaptureState?
            if let historyRoot, let payload = node?(result) {
                var after: [String: String] = [:], was: [String: String] = [:]
                for s in result.delta.scenes {
                    guard let text = s.text else { continue }
                    after[s.sceneID] = text
                    if let old = before[s.sceneID] { was[s.sceneID] = old }
                }
                for id in result.delta.removedSceneIDs { was[id] = before[id] }
                history = try? engine.captureRecordBarrier(projectRootPath: historyRoot, barrierKind: payload.op,
                                                           note: "", structuralPayload: payload,
                                                           sceneTexts: after, sceneTextsBefore: was)
            }
            return StructuralLanding(result: result, history: history,
                                     storyTimes: try? engine.listStoryTimes(projectRootPath: root))
        }, landed: { [weak self] outcome in
            self?.structuralEditLanded(outcome, caret: caret, failed: failed, then: then)
        })
        if !started {
            host.structureSetEditable(true)
            if loader.writesHeld { host.structureRefuse() }   // [SP-195] Q1 (a): refused behind a held queue
        }
        return started
    }

    private func structuralEditLanded(_ outcome: Result<StructuralLanding, Error>, caret: StructuralCaret,
                                      failed: (@MainActor () -> Void)?,
                                      then: (@MainActor (RangeOperationResult) -> Void)?) {
        let loader = loader
        guard let host, let storage = host.structureStorage else { return }
        defer { host.structureLanded() }
        host.structureSetEditable(true)
        let landing: StructuralLanding
        switch outcome {
        case .success(let l): landing = l
        case .failure(let error):
            NSLog("[SCRIVI-DIAG] structural edit failed: %@", String(describing: error))   // [SP-188] AC11
            host.structureRefuse()
            deferredSteps.removeAll()
            failed?()
            return
        }
        if let history = landing.history { session.historyCapture?.adopt(history) }
        let delta = landing.result.delta
        let verify = RangeVerify.request(for: delta, before: loader.verifyScenes)   // before: it knows the removed
        defer { loader.verifyAgainstDisk(verify) { [weak self] correction in self?.applyCorrection(correction) } }
        // ✅ [SP-191] Q10: no renumbering — an untitled chapter's "Chapter N" is derived from its position wherever it
        // shows (headings, navigator, timeline), so a chapter inserted or removed renumbers the rest by itself.
        if let run = loader.apply(delta) { host.structureRenderRun(old: run.old, new: run.new) }
        let bounds = sync.sceneBoundaries(in: storage)
        if let target = caret(landing.result),
           let i = loader.segments.firstIndex(where: { $0.sceneID == target.sceneID }),
           bounds.indices.contains(i) {
            let loc = bounds[i].location + Self.charOffset(forByteOffset: target.byte, in: loader.segments[i].text)
            // ⚠️ The caret's move tells the history which scene the caret is in (`captureCaretMoved`, in memory — the
            // edit flushed the pending typing first): every caret move makes it, so it is counted apart.
            StructuralCallAccounting.$placingCaret.withValue(true) {
                host.structureRevealCaret(min(loc, storage.length))
            }
            sync.lastCursorSegmentIndex = i
            loader.setCurrentIndex(i)
            loader.setViewportScene(target.sceneID)
        }
        if let timeline = session.timelineModel {
            timeline.reloadSceneDots(explicitByScene: TimelineViewModel.explicitStoryTimes(landing.storyTimes),
                                     scenes: loader.allScenes)
            timeline.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
        }
        then?(landing.result)   // ✅ [SP-188] after the caret is placed: a cut's clipboard, a paste over a selection (Q4, Q6)
        runDeferredSteps()
    }

    /// ✅ [SP-189]: the disk's correction, rendered as every delta is — the caret and the scroll where they were (AC2).
    private func applyCorrection(_ correction: RangeDelta) {
        let loader = loader
        guard let host, let storage = host.structureStorage else { return }
        let caret = host.structureSelection.location
        let i0 = segmentIndex(for: caret)
        let scene = i0.flatMap { loader.segments.indices.contains($0) ? loader.segments[$0].sceneID : nil }
        let offset = i0.flatMap { sceneBoundaries.indices.contains($0) ? caret - sceneBoundaries[$0].location : nil } ?? 0
        host.structurePreservingScroll {
            if let run = loader.apply(correction) { host.structureRenderRun(old: run.old, new: run.new) }
            let bounds = sync.sceneBoundaries(in: storage)
            if let scene, let i = loader.segments.firstIndex(where: { $0.sceneID == scene }), bounds.indices.contains(i) {
                let length = (loader.segments[i].text as NSString).length
                host.structurePlaceCaret(bounds[i].location + min(max(offset, 0), length), reveal: false)
            }
        }
    }

    /// The caret's scene, its index, and the caret's scene-local UTF-8 byte offset — where a range call is anchored.
    /// ✅ [SP-191] (live pass, user: *"the preceding ## is left behind in the previous chapter"*): the anchor is BEFORE
    /// any hidden markers the caret rests after — a heading's `## `, a list marker, an opening `**` (their caret home is
    /// after them) — so a split, a chapter split, a merge or a paste at the visible start of a line takes the line whole.
    func caretInScene() -> (segIdx: Int, seg: SceneSegment, byte: Int)? {
        guard let host else { return nil }
        let loc = structuralAnchor(host.structureSelection.location)
        let bounds = sceneBoundaries
        guard let i = segmentIndex(for: loc), loader.segments.indices.contains(i), bounds.indices.contains(i) else { return nil }
        let seg = loader.segments[i]
        return (i, seg, Self.byteOffset(charOffset: loc - bounds[i].location, in: seg.text))
    }

    /// Steps back from `loc` over every hidden marker run that ends exactly there and whose caret home is after it.
    func structuralAnchor(_ loc: Int) -> Int { structuralAnchorAndLine(loc).loc }

    /// The same, and whether it stepped back over a LINE-START marker (a heading's `## `, a list marker) — a paste
    /// anchored there must not join its last part onto that line, or the heading or item stops being one.
    func structuralAnchorAndLine(_ loc: Int) -> (loc: Int, beforeLineMarker: Bool) {
        guard let storage = host?.structureStorage else { return (loc, false) }
        let stops = presenter.stopTest(in: storage)
        var at = loc
        var lineMarker = false
        while at > 0, let stop = stops(at - 1), stop.homeAfter, NSMaxRange(stop.range) == at, stop.range.location < at {
            if stop.kind == .prefix || stop.kind == .listPrefix { lineMarker = true }
            at = stop.range.location
        }
        return (at, lineMarker)
    }

    /// A Range entry naming a scene's place without its text — a break's sides, a relabel's run.
    private static func bare(_ seg: SceneSegment, at byte: Int = 0, of whole: Int = 0) -> ScriviRange.Entry {
        .init(sceneID: seg.sceneID, chapterID: seg.chapterID, chapterTitle: "", sceneTitle: "",
              start: byte, length: 0, sceneLength: whole, text: "")
    }

    // ⌘-Return: a new scene at the caret. ✅ [SP-191] Q1: a PASTE OF A BREAK — an empty two-entry Range — so it is one
    // range call off the main actor, and Undo reverses it in one step (Q3: no "Can't undo past…").
    // ✅ Q7 (user, 2026-10-21): at a scene's very start the new scene goes BEFORE it — the scene keeps its text, title and
    // sidecar (I-0096) — and the caret goes into the new scene; elsewhere the scene splits and the caret starts the new
    // one. (A plain Return at a scene's start is just a newline: not this path.)
    func handleCreateScene() {
        insertBreak(newChapter: false, label: "Splitting the scene…")
    }

    // ⇧⌘-Return: a new chapter at the caret — the same paste, its second entry in another chapter, so the core creates
    // the chapter in place and moves the rest of this chapter into it ([I-0311]; ✅ [I-0314]: in place ON DISK too).
    // ✅ Q8: at a scene's very start it is a RELABEL — the scene and the rest of its chapter move into a new chapter; no
    // empty scene, no text moves. At a chapter's first scene a chapter already starts there: nothing happens.
    // ✅ Q9: no confirmation — Undo reverses it.
    func handleCreateChapter() {
        guard let at = caretInScene() else { return }
        guard at.byte == 0 else {
            insertBreak(newChapter: true, label: "Splitting the chapter…")
            return
        }
        let segments = loader.segments
        guard at.segIdx > 0, segments[at.segIdx - 1].chapterID == at.seg.chapterID else { return }
        let run = segments[at.segIdx...].prefix { $0.chapterID == at.seg.chapterID }
        relabel(ScriviRange(scenes: run.map { Self.bare($0) }), into: "", keepingCaretIn: at.seg.sceneID,
                label: "Splitting the chapter…")
    }

    private func insertBreak(newChapter: Bool, label: String) {
        guard let at = caretInScene() else { return }
        var opening = Self.bare(at.seg)
        opening.sceneID = ""                                    // ✅ Q1: an empty ID mints a fresh one
        if newChapter { opening.chapterID = "" }                // another chapter: the paste opens one there
        var first = Self.bare(at.seg)
        first.sceneID = ""
        let range = ScriviRange(scenes: [first, opening])
        let caretSceneID = at.seg.sceneID, caretByte = at.byte, caretText = at.seg.text
        runStructuralEdit(label, { engine, root, ref in
            try engine.enqueueRangePaste(projectRootPath: root, range: range, caretSceneID: caretSceneID,
                                         caretByteOffset: caretByte, caretSceneText: caretText, authorshipRef: ref)
        }, node: { r in
            HistoryStructuralPayload(op: "rangePaste", caretSceneID: caretSceneID, caretByte: caretByte,
                                     rangeJSON: (try? r.range.toJSON()) ?? "")
        }, caret: { r in
            // The new scene — after the caret scene, or before it at a scene's start (Q7).
            r.range.scenes.first { $0.sceneID != caretSceneID }.map { ($0.sceneID, 0) }
        })
    }

    // ⌘-Backspace at a scene's very start: merge it into the scene before (same chapter only). ✅ [SP-191] Q3e: a CUT of
    // the zero-length sub-range across the break — the two texts join DIRECTLY (Q3a: no blank line), the removed scene's
    // sidecar travels in the Range (Q3b), and Undo puts the break back exactly.
    func handleMergeScene() {
        guard let at = caretInScene(), at.byte == 0, at.segIdx > 0 else { return }
        let previous = loader.segments[at.segIdx - 1]
        guard previous.chapterID == at.seg.chapterID else { return }   // the first scene of a chapter: nothing
        let prevBytes = previous.text.utf8.count
        let range = ScriviRange(scenes: [Self.bare(previous, at: prevBytes, of: prevBytes),
                                         Self.bare(at.seg, at: 0, of: at.seg.text.utf8.count)])
        cut(range, label: "Merging the scenes…")
    }

    // ⇧⌘-Backspace at a chapter's first scene: merge the chapter into the one before. ✅ [SP-191] Q3e: a RELABEL of its
    // scenes into that chapter — on disk their files move; no text changes. Undo re-creates the chapter from its sidecar,
    // which the result carries (Q3b). Every scene of the chapter joins the earlier one (Q3d).
    func handleMergeChapter() {
        guard let at = caretInScene(), at.byte == 0, at.segIdx > 0 else { return }
        let segments = loader.segments
        let previous = segments[at.segIdx - 1].chapterID
        guard previous != at.seg.chapterID else { return }    // not a chapter's first scene
        let run = segments[at.segIdx...].prefix { $0.chapterID == at.seg.chapterID }
        relabel(ScriviRange(scenes: run.map { Self.bare($0) }), into: previous, keepingCaretIn: at.seg.sceneID,
                label: "Merging the chapters…")
    }

    /// What the structural edits can do at the caret now — for a menu that offers only these (iOS, Q3).
    var canMergeScene: Bool {
        guard let at = caretInScene(), at.byte == 0, at.segIdx > 0 else { return false }
        return loader.segments[at.segIdx - 1].chapterID == at.seg.chapterID
    }
    var canMergeChapter: Bool {
        guard let at = caretInScene(), at.byte == 0, at.segIdx > 0 else { return false }
        return loader.segments[at.segIdx - 1].chapterID != at.seg.chapterID
    }

    @discardableResult
    private func cut(_ range: ScriviRange, label: String,
                     then: (@MainActor (RangeOperationResult) -> Void)? = nil) -> Bool {
        runStructuralEdit(label, { engine, root, ref in
            try engine.enqueueRangeCut(projectRootPath: root, range: range, authorshipRef: ref)
        }, node: { r in
            HistoryStructuralPayload(op: "rangeCut", caretSceneID: r.undoCaretSceneID ?? "",
                                     caretByte: r.undoCaretByte ?? 0, rangeJSON: (try? r.range.toJSON()) ?? "")
        }, caret: { r in r.undoCaretSceneID.map { ($0, r.undoCaretByte ?? 0) } }, then: then)
    }

    private func relabel(_ run: ScriviRange, into target: String, keepingCaretIn sceneID: String, label: String) {
        runStructuralEdit(label, { engine, root, ref in
            try engine.enqueueRangeRelabel(projectRootPath: root, range: run, targetChapterID: target, authorshipRef: ref)
        }, node: { r in
            HistoryStructuralPayload(op: "rangeRelabel", rangeJSON: (try? r.range.toJSON()) ?? "",
                                     fromChapterID: r.fromChapterID ?? "",
                                     targetChapterID: r.range.scenes.first?.chapterID ?? "")
        }, caret: { _ in (sceneID, 0) })
    }

    // MARK: — Cross-boundary Cut/Copy/Paste (EP-029 SP-089, T-0354)
    //
    // The manuscript is one text storage; sceneBoundaries[i] is scene i's storage range (dividers / headings sit OUTSIDE
    // those ranges). A selection is "cross-boundary" when it takes more than one scene — then cut, copy and paste are
    // range calls. A single-scene selection keeps the host's own fast path (AC5).

    // True if `sel` takes more than one scene.
    func selectionCrossesBoundary(_ sel: NSRange) -> Bool { (selectionRun(sel)?.count ?? 0) > 1 }

    /// ✅ [SP-188] The scenes a selection takes, in reading order, each with the part of it selected (storage ranges) —
    /// what a copy and a cut build their Range from. A scene is in it when the selection overlaps its text, and:
    ///  - Q5 / Q2: a selection that covers a SCENE divider takes the scenes on both sides of it — the one before at its
    ///    end, the one after at its start (zero-length parts) — so whole scenes go whole, and a selection of exactly one
    ///    divider is the merge;
    ///  - Q1: a chapter divider (its title) is skipped — presentation, in no scene's text — unless the selection covers
    ///    the whole chapter, which then takes the break after it (or, at the manuscript's end, the one before), so the cut
    ///    removes the chapter.
    /// nil for an empty selection or one that reaches no scene.
    func selectionRun(_ sel: NSRange) -> [(segIdx: Int, part: NSRange)]? {
        guard sel.length > 0 else { return nil }
        let segments = loader.segments
        let bounds = sceneBoundaries
        let n = min(bounds.count, segments.count)
        var lo = Int.max, hi = Int.min
        func take(_ i: Int) { lo = min(lo, i); hi = max(hi, i) }
        for i in 0..<n where NSIntersectionRange(bounds[i], sel).length > 0 { take(i) }
        for i in 0..<max(0, n - 1) where segments[i].chapterID == segments[i + 1].chapterID
            // The divider is the character at the scene's end: a click or drag over it selects it without its `\n`.
            && sel.location <= NSMaxRange(bounds[i]) && NSMaxRange(sel) > NSMaxRange(bounds[i]) {
            take(i); take(i + 1)
        }
        var c0 = 0
        while c0 < n {
            var c1 = c0
            while c1 + 1 < n, segments[c1 + 1].chapterID == segments[c0].chapterID { c1 += 1 }
            if sel.location <= bounds[c0].location, NSMaxRange(sel) >= NSMaxRange(bounds[c1]) {
                take(c0); take(c1)
                if c1 + 1 < n { take(c1 + 1) } else if c0 > 0 { take(c0 - 1) }
            }
            c0 = c1 + 1
        }
        guard lo <= hi else { return nil }
        return (lo...hi).map { i in
            let part = NSIntersectionRange(bounds[i], sel)
            if part.length > 0 { return (i, part) }
            // Untouched text: the scene before the selection is taken at its end, the one after at its start.
            return (i, NSRange(location: i == lo && bounds[i].location < sel.location ? NSMaxRange(bounds[i])
                                                                                      : bounds[i].location, length: 0))
        }
    }

    // True if the caret at storage offset `loc` sits inside a non-editable chapter title.
    func caretInHeading(_ loc: Int) -> Bool {
        guard let storage = host?.structureStorage, loc < storage.length else { return false }
        return storage.attribute(.scriviHeading, at: loc, effectiveRange: nil) != nil
    }

    // Scene-local UTF-16 char offset → scene-local UTF-8 byte offset (history's convention).
    static func byteOffset(charOffset: Int, in sceneText: String) -> Int {
        let nsText = sceneText as NSString
        let clamped = max(0, min(charOffset, nsText.length))
        return nsText.substring(to: clamped).utf8.count
    }

    // Scene-local UTF-8 byte offset → character offset within `sceneText`.
    static func charOffset(forByteOffset byteOffset: Int, in sceneText: String) -> Int {
        guard byteOffset > 0 else { return 0 }
        var bytes = 0
        var chars = 0
        for scalar in sceneText.unicodeScalars {
            if bytes >= byteOffset { break }
            bytes += String(scalar).utf8.count
            chars += scalar.utf16.count   // NSString char units
        }
        return chars
    }

    // The internal structured clipboard (T2=A): the Range a cross-scene copy or cut placed. The system pasteboard carries
    // only flat text for other apps; the Range lives here so Scrivi's own paste can reconstruct the breaks.
    /// ⛔ [I-0279]: the copy used to be held FOREVER — every later paste pasted it, whatever had been copied since.
    /// ✅ Now it is tied to the system pasteboard's change count.
    private var structuredClipboard = StructuredClipboard()
    private var internalClipboardRange: ScriviRange? {
        structuredClipboard.range(currentChangeCount: host?.structurePasteboardChangeCount ?? -1)
    }
    private func holdClipboardRange(_ range: ScriviRange?, changeCount: Int) {
        structuredClipboard.hold(range, changeCount: changeCount)
    }

    // True when a structured Range is available for paste (drives paste routing).
    var hasInternalStructuredRange: Bool { internalClipboardRange != nil }

    // Cross-scene copy: the flat text on the system pasteboard (other apps get clean text), the Range held for Scrivi's
    // own paste. ✅ [SP-191] (§4.2b): built from the scene list — no core call, no disk read. Returns true if it handled
    // the copy (the selection crossed a break); false → the host uses its normal single-scene copy.
    func structuredCopyIfCrossBoundary() -> Bool {
        guard let host else { return false }
        let sel = host.structureSelection
        guard selectionCrossesBoundary(sel), let copied = copiedRange(sel) else { return false }
        // EP-045 AC4: other apps get what the writer SEES. ⚠️ [I-0279]: held AFTER the write, at its change count.
        holdClipboardRange(copied.range, changeCount: host.structureWritePasteboard(copied.presented))
        return true
    }

    /// ✅ [SP-191] The Range of a cross-scene selection, from the scene list (§4.2b, R6): one entry per scene the
    /// selection touches, with its identity, chapter and titles. ✅ [T-0591]: each piece is BALANCED — one that starts or
    /// ends inside a bold span carries its own markers (AC12) — so its offsets are the balanced piece's (`normalized`).
    /// Other apps get what the writer SEES ([SP-162] Q2), pieces joined by a blank line; `source` is the stored pieces
    /// joined the same way (a buffer slot's flat text). Shared by copy and the copy buffers ([I-0289]).
    func copiedRange(_ sel: NSRange) -> (range: ScriviRange, presented: String, source: String)? {
        guard let storage = host?.structureStorage else { return nil }
        let loader = loader
        let bounds = sceneBoundaries
        let infos = Dictionary(loader.allScenes.map { ($0.sceneID, $0) }, uniquingKeysWith: { a, _ in a })
        var entries: [ScriviRange.Entry] = []
        var presented: [String] = []
        for (i, part) in selectionRun(sel) ?? [] {   // ✅ [SP-188] Q5 / Q1: the same scenes a cut takes
            let seg = loader.segments[i]
            let copy = presenter.balancedCopy(in: storage, part)
            let info = infos[seg.sceneID]
            entries.append(.init(sceneID: seg.sceneID, chapterID: seg.chapterID,
                                 chapterTitle: info?.chapterTitle ?? "", sceneTitle: info?.title ?? "",
                                 start: Self.byteOffset(charOffset: part.location - bounds[i].location, in: seg.text),
                                 length: 0, sceneLength: seg.text.utf8.count, text: copy.source))
            presented.append(copy.presented)
        }
        guard entries.count > 1 else { return nil }
        return (ScriviRange(scenes: entries).normalized(), presented.joined(separator: "\n\n"),
                entries.map(\.text).joined(separator: "\n\n"))
    }

    /// Where a cut across scenes puts what it removed.
    enum CutKeeping { case nothing, clipboard, buffer(String) }

    // ✅ [SP-188] (design R4, §4.3) — A CUT ACROSS SCENES REMOVES THE STRUCTURE: cut, buffer cut, ⌫ and ⌦ over a selection
    // that crosses a break are ONE `rangeCut` of the scenes `selectionRun` takes, off the main actor, the delta applied —
    // the start scene's head joins the end scene's tail (balanced, Q3), the scenes between go, an emptied chapter goes.
    // Undo pastes it back.
    //  - cut / buffer cut: other apps get the presented text at once; when the cut LANDS, Scrivi's clipboard (or the slot)
    //    takes the Range the cut returned — sidecars included, its pieces the balanced copy's, its join dropped (Q4).
    //  - ⌫ / ⌦: the same cut; the clipboard and buffers are left alone (design Q4).
    // `then` runs once it has landed, the caret at the join (Q6: a paste over the selection).
    // Returns false for a selection that takes one scene — the host's ordinary path handles it; true when handled (cut,
    // or ignored while another structural edit is in flight).
    @discardableResult
    func cutAcrossScenes(_ sel: NSRange, keeping: CutKeeping, then: (@MainActor () -> Void)? = nil) -> Bool {
        guard let host, let run = selectionRun(sel), run.count > 1 else { return false }
        if loader.structuralInFlight { return true }   // ✅ Q5: ignored, not queued
        let segments = loader.segments
        let bounds = sceneBoundaries
        // The stored text of each part — the core checks it against the disk (⛔ never the balanced copy).
        let entries: [ScriviRange.Entry] = run.map { i, part in
            let seg = segments[i]
            let a = Self.byteOffset(charOffset: part.location - bounds[i].location, in: seg.text)
            let b = Self.byteOffset(charOffset: NSMaxRange(part) - bounds[i].location, in: seg.text)
            let bytes = Array(seg.text.utf8)
            return .init(sceneID: seg.sceneID, chapterID: seg.chapterID, chapterTitle: "", sceneTitle: "",
                         start: a, length: b - a, sceneLength: bytes.count,
                         text: String(decoding: bytes[a..<b], as: UTF8.self))
        }
        var copy: (range: ScriviRange, presented: String, source: String)?
        var changeCount = 0
        switch keeping {
        case .nothing: break
        case .clipboard:
            copy = copiedRange(sel)
            changeCount = host.structureWritePasteboard(copy?.presented ?? "")   // EP-045 AC4: what the writer SEES
        case .buffer:
            copy = copiedRange(sel)
        }
        let label = if case .nothing = keeping { "Deleting…" } else { "Cutting…" }
        return cut(ScriviRange(scenes: entries), label: label) { [weak self] r in
            guard let self else { return }
            // Q4: the returned Range, the copy's balanced pieces in place of the stored ones (same scenes, same order).
            var held = r.range.withoutJoin
            if let copy, copy.range.scenes.count == held.scenes.count {
                for k in held.scenes.indices { held.scenes[k].text = copy.range.scenes[k].text }
            }
            held = held.normalized()
            switch keeping {
            case .nothing: break
            case .clipboard:
                // ⚠️ [I-0279]: only if nothing else was copied while the cut ran.
                if self.host?.structurePasteboardChangeCount == changeCount { self.holdClipboardRange(held, changeCount: changeCount) }
            case .buffer(let bufferID):
                if let json = try? held.toJSON() {
                    self.session.bufferService?.load(copy?.source ?? "", intoSlot: bufferID, rangeJSON: json)
                }
            }
            then?()
        }
    }

    // Paste when the internal clipboard holds a Range: reconstruct it at the caret. Returns true if handled; false → the
    // host uses its normal pasteboard paste.
    func structuredPasteIfAvailable() -> Bool {
        guard let range = internalClipboardRange else { return false }
        return pasteStructuredRange(range)
    }

    // Reconstruct `range` at the caret — paste of the internal clipboard, a structured buffer slot, and Import ▸
    // Manuscript… Into This Project. ✅ [SP-191]: ONE `rangePaste` off the main actor, the delta applied, no reload; Undo
    // cuts exactly what it inserted. ✅ Q7: at a scene's very start the pasted scenes go before it and the last part joins
    // its start. The caret lands at the end of the pasted block. Refuses if the caret is in a heading. Returns true if
    // handled (pasted, refused, or ignored while another structural edit is in flight).
    func pasteStructuredRange(_ range: ScriviRange) -> Bool {
        guard let host else { return false }
        let sel = host.structureSelection
        if caretInHeading(sel.location) { host.structureFlashRefuse(); return true }   // handled = refused
        if loader.structuralInFlight { return true }                                     // ✅ Q5: ignored, not queued
        // ✅ [I-0295] ([SP-180] Q2): a selection is REPLACED, as a flat paste replaces it — removed first, as its own
        // undo step, then the Range lands where it began. ✅ [SP-188] Q6: across scenes the removal is the structural cut,
        // and the paste follows when it lands (the caret then at the join).
        if sel.length > 0 {
            if cutAcrossScenes(sel, keeping: .nothing, then: { [weak self] in _ = self?.pasteStructuredRange(range) }) {
                return true
            }
            host.structureDeleteSelection(sel)
        }
        guard let at = caretInScene() else { return false }
        var range = range
        // ✅ [SP-191] live pass: anchored before a heading's (or list item's) marker, the last part keeps its own line —
        // joined onto `## Heading` it would turn the heading into body text. The line break is part of what the paste
        // inserts, so its undo takes it back too.
        if structuralAnchorAndLine(host.structureSelection.location).beforeLineMarker, range.scenes.count > 1,
           let last = range.scenes.last, !last.text.isEmpty, !last.text.hasSuffix("\n") {
            range.scenes[range.scenes.count - 1].text += "\n"
            range = range.normalized()
        }
        // ✅ [T-0591]: pasting INSIDE a bold span — the first piece closes it, the last re-opens it for the text after the
        // split. ✅ [SP-163] live pass (user): the pasted text KEEPS ITS OWN formatting (only what was bold is).
        if let storage = host.structureStorage {
            let style = presenter.styleAtCaret(in: storage, host.structureSelection.location)
            let texts = ManuscriptPresenter.balancePastePieces(range.scenes.map(\.text), caretStyle: style)
            if texts != range.scenes.map(\.text) {
                for i in range.scenes.indices { range.scenes[i].text = texts[i] }
                range = range.normalized()
            }
        }
        let pasted = range
        let caretSceneID = at.seg.sceneID, caretByte = at.byte, caretText = at.seg.text
        runStructuralEdit("Pasting…", { engine, root, ref in
            try engine.enqueueRangePaste(projectRootPath: root, range: pasted, caretSceneID: caretSceneID,
                                         caretByteOffset: caretByte, caretSceneText: caretText, authorshipRef: ref)
        }, node: { r in
            HistoryStructuralPayload(op: "rangePaste", caretSceneID: caretSceneID, caretByte: caretByte,
                                     rangeJSON: (try? r.range.toJSON()) ?? "")
        }, caret: { r in r.range.scenes.last.map { ($0.sceneID, $0.start + $0.length) } })
        return true
    }

    // MARK: — The copy buffers (EP-019, T-0214; moved from the Mac host, EP-055 S3b [SP-223])

    /// ⌘N: copy the selection into slot N. Copy-only — NO text changes, so no history event (Trade T3). Across scenes a
    /// STRUCTURED slot (flat text + a Range, ✅ [SP-191] built from the scene list) so ⌃⌘N rebuilds the breaks.
    /// ✅ [I-0289]: a slot holds the BALANCED source, as ⌘C keeps for Scrivi's own paste.
    func copyIntoBuffer(_ bufferID: String) {
        guard let host, let storage = host.structureStorage else { return }
        let sel = host.structureSelection
        guard sel.length > 0 else { return }
        if selectionCrossesBoundary(sel), let copied = copiedRange(sel), let json = try? copied.range.toJSON() {
            session.bufferService?.load(copied.source, intoSlot: bufferID, rangeJSON: json)
            return
        }
        session.bufferService?.load(presenter.balancedCopy(in: storage, sel).source, intoSlot: bufferID)
    }

    /// ⌥N: cut the selection into slot N — a `cut` history event tagged with the slot (Trade T3). The slot is loaded FIRST,
    /// so it holds the text even if the delete is later undone. ✅ [SP-188] Across scenes: the structural cut, its Range into
    /// the slot when it lands (Q4).
    func cutIntoBuffer(_ bufferID: String) {
        guard let host, let storage = host.structureStorage else { return }
        let sel = host.structureSelection
        guard sel.length > 0 else { return }
        if cutAcrossScenes(sel, keeping: .buffer(bufferID)) { return }
        guard session.bufferService?.load(presenter.balancedCopy(in: storage, sel).source, intoSlot: bufferID) == true else { return }
        session.historyCapture?.beginCutIntoBuffer(bufferID: bufferID)
        host.structureDeleteSelection(sel)
        session.historyCapture?.flush(trigger: "cut", kind: "cut")
    }

    /// ⌃⌘N: paste slot N at the caret, replacing any selection — one `paste` history event; the system pasteboard is
    /// untouched. A structured slot rebuilds its scenes, as ⌘V of a cross-scene copy; an empty slot is a silent no-op.
    func pasteFromBuffer(_ bufferID: String) {
        guard let host else { return }
        if let range = session.bufferService?.range(inSlot: bufferID) {
            _ = pasteStructuredRange(range)
            return
        }
        guard let text = session.bufferService?.text(inSlot: bufferID), !text.isEmpty else { return }
        session.historyCapture?.beginPasteOrCut(kind: "paste")
        host.structureInsertOwnSource(text)
        session.historyCapture?.flush(trigger: "paste", kind: "paste")
    }

    // MARK: — Replace All (moved from the Mac host, EP-055 S3b [SP-223])

    /// ✅ [SP-164] Q2: Replace All is ONE undoable step, even across scenes — recorded as ONE grouped edit (as [I-0270]'s
    /// cross-scene delete is). `edits` are storage ranges and UNescaped replacement text.
    /// ✅ [I-0333] (SP-201, Linux's shape, `EditorShell.cpp`): it froze the window 6 s on dumas (41,300 matches), one core
    /// call and one storage edit per match on the main thread. Now: the matches are snapped and grouped by scene here; each
    /// scene's new text is computed OFF the main actor; the scenes go in back to front in slices of ~50 ms with the count in
    /// the progress row. The editor takes no input meanwhile and an undo waits its turn (the structural edits' busy state).
    /// False when there is no history to record it in (a view with no project): the host applies the edits itself.
    func replaceAll(_ edits: [(NSRange, String)]) -> Bool {
        guard let host, let storage = host.structureStorage, let capture = session.historyCapture else { return false }
        guard !edits.isEmpty else { return true }
        let loader = self.loader, presenter = self.presenter
        guard loader.beginLocalEdit("Replacing…") else { host.structureRefuse(); return true }
        let t0 = Date()
        host.structureSetEditable(false)
        let bounds = sceneBoundaries
        let ns = storage.string as NSString
        let runAt = ManuscriptEditRules.runLookup(presenter, storage)   // once: nothing changes the storage in this pass
        var byScene: [Int: [(NSRange, String)]] = [:]
        for (r, typed) in edits {
            var range = r
            if range.length > 0, let pair = MarkdownEscapes.snapSelection(range, in: ns, runAt: runAt) { range = pair }
            guard let i = segmentIndex(for: range.location), bounds.indices.contains(i),
                  loader.segments.indices.contains(i), NSMaxRange(range) <= NSMaxRange(bounds[i]) else { continue }
            byScene[i, default: []].append((NSRange(location: range.location - bounds[i].location, length: range.length),
                                            MarkdownEscapes.escape(typed)))
        }
        // Back to front, so each scene's storage range is still where the boundaries say when it is replaced.
        let jobs = byScene.keys.sorted(by: >).map { i in (index: i, before: ns.substring(with: bounds[i]), edits: byScene[i]!) }
        let selection = host.structureSelection
        capture.flush(trigger: "replace")
        let tSnapped = Date()
        Task { @MainActor [weak self] in
            let after = await Task.detached(priority: .userInitiated) { jobs.map { Self.replaced($0.before, $0.edits) } }.value
            let tComputed = Date()
            guard let self, let host = self.host, let storage = host.structureStorage else { return loader.endLocalEdit() }
            let total = edits.count
            var k = 0, replaced = 0
            var grouped: [HistoryCapture.GroupedSceneEdit] = []
            var touched: [Int] = []
            while k < jobs.count {
                let slice = Date()
                capture.withApplying {
                    // One storage edit per slice: what reacts to an edit (the presenter, the AX map) runs once for it.
                    storage.beginEditing()
                    defer { storage.endEditing() }
                    while k < jobs.count, Date().timeIntervalSince(slice) < 0.05 {
                        let job = jobs[k], text = after[k]
                        k += 1
                        replaced += job.edits.count
                        let bounds = self.sceneBoundaries
                        guard text != job.before, bounds.indices.contains(job.index),
                              loader.segments.indices.contains(job.index) else { continue }
                        storage.replaceCharacters(in: bounds[job.index],
                                                  with: NSAttributedString(string: text, attributes: presenter.typography.bodyAttributes))
                        let seg = loader.segments[job.index]
                        loader.updateText(text, at: job.index)
                        loader.updateLiveTitle(ManuscriptPresenter.presentedFirstLine(of: text), forSceneID: seg.sceneID)
                        grouped.append(.init(sceneID: seg.sceneID, textBefore: job.before, textAfter: text, caret: 0))
                        touched.append(job.index)
                    }
                }
                loader.localEditProgress("Replacing \(replaced.formatted()) of \(total.formatted())…")
                if k < jobs.count { await Task.yield(); try? await Task.sleep(for: .milliseconds(1)) }
            }
            let tApplied = Date()
            capture.recordGroupedEdit(grouped.reversed(), kind: "replace")
            let tRecorded = Date()
            host.structurePlaceCaret(min(selection.location, storage.length), reveal: false)
            host.structureSetEditable(true)
            // ✅ EP-052 ([SP-195]): the touched scenes — ONLY those holding a match — are queued, written off the main
            // thread, as the busy state ends (saves are held while it lasts).
            let env = self.sync.env
            let ref = self.session.authorshipRef
            loader.endLocalEdit {
                if let ref { loader.saveScenesOffMain(touched, engine: env.engine, ref: ref) }
            }
            self.session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            NSLog(String(format: "[SCRIVI-FIND] replaceAll %d edits / %d scenes: snap=%.0f ms  compute=%.0f ms (off main)  apply=%.0f ms  history=%.0f ms",
                         total, jobs.count, tSnapped.timeIntervalSince(t0) * 1000, tComputed.timeIntervalSince(tSnapped) * 1000,
                         tApplied.timeIntervalSince(tComputed) * 1000, tRecorded.timeIntervalSince(tApplied) * 1000))
            self.runDeferredSteps()
        }
        return true
    }

    /// One scene's text with `edits` (back to front, scene-relative, stored form) made, each kept balanced. Off the main
    /// actor: the core reads, nothing here touches the view.
    nonisolated static func replaced(_ scene: String, _ edits: [(NSRange, String)]) -> String {
        let text = NSMutableString(string: scene)
        for (r, t) in edits {
            let w = ScriviEngine.markdownEditWindow(text, around: r)
            let local = NSRange(location: r.location - w.location, length: r.length)
            if let e = try? ScriviEngine.markdownBalancedEditInScene(scene: text.substring(with: w), range: local, replacement: t,
                                                                     keepsOwnFormatting: false) {
                text.replaceCharacters(in: NSRange(location: w.location + e.range.location, length: e.range.length),
                                       with: e.replacement)
            } else {
                text.replaceCharacters(in: r, with: t)
            }
        }
        return text as String
    }
}
