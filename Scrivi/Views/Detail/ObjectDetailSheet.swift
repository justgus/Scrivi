import SwiftUI

/// The Object Detail Sheet — a first-class, non-modal surface for seeing and
/// editing what an object actually *is* (EP-034 SP-117; trades **D1-E**, **D2-B**,
/// **D3-A**).
///
/// > *"This is not a popup anymore."* — the writer's brief, R1.
///
/// ## ⚠️ Host-independence is a requirement, not a style (S8)
///
/// D1-E ruled this a **non-modal editor-level pane** rather than a window, and
/// bound the ruling to a condition: it *"must be built as a self-contained,
/// navigable component that does not depend on its host. That is what keeps B (a
/// window) available later without a rewrite."*
///
/// ⚠️ So this view reads **nothing** from its host: no `focusedSession`, no
/// editor environment, no inspector geometry. Everything arrives as a parameter.
/// A `#Preview` with no host at all is the standing proof — if it stops
/// compiling, host-independence has been lost.
///
/// ⚠️ **There is no auxiliary window type in Scrivi** (grep: `.sheet`/`.popover`
/// only), and EP-018's registry, dead-window cache, `.onOpenURL` unreliability
/// and terminate-on-last-window override are the receipts for what adding one
/// costs. D1-E exists to avoid paying that inside an Epic already carrying nine
/// trades.
struct ObjectDetailSheet: View {

    let engine: ScriviEngine
    let projectRootPath: String
    let authorshipRef: AuthorshipRef
    /// Worlds as the app already knows them — passed in so this view never
    /// resolves world state itself (and so a caller can supply fixtures).
    let worlds: [WorldEntry]
    /// Writer-facing scene names, keyed by sceneID (I-0151). Supplied by the host,
    /// which already resolves them for the Navigator — ⚠️ **the pane must never
    /// show a raw `scene_…` ID**, and must not reach into the loader to avoid it
    /// (S8 host-independence). Missing keys fall back inside the section.
    var sceneNames: [String: String] = [:]
    /// I-0151 — take the writer to a related SCENE in the manuscript. A scene has
    /// no Detail Sheet, so the sheet hands this back to its host.
    var onSelectScene: (String) -> Void = { _ in }
    /// I-0155 — announces a successful save so the host can refresh other surfaces
    /// showing the same object (the Scene Inspector's cards).
    ///
    /// ⚠️ A callback, not a direct refresh: this pane must not know the inspector
    /// exists (S8 host-independence).
    var onDidSave: () -> Void = {}
    /// Needed for world-package writes: the core records which project holds the
    /// lock, so a shared world can name the holder rather than "unknown".
    var projectID: String = ""
    /// ⚠️ I-0162: bumped when a world's availability changes (mount/eject).
    ///
    /// The sheet re-reads on it, because `imagePath` and the read-only banner are
    /// both **snapshots taken at load time** and a drive can vanish afterwards.
    /// ⚠️ Without this the sheet kept a path to bytes that were no longer
    /// reachable and reported *"This image could not be read from disk"* — a
    /// damaged-file message for what is actually an outage.
    ///
    /// ⚠️ `session.worldRevision` ALREADY EXISTED for this (I-0128/I-0129); the
    /// inspector's cards have watched it all along. This surface simply did not.
    var worldRevision: Int = 0
    /// I-0160 — bumped when the object is edited OUTSIDE this sheet (the Scene
    /// Inspector's inline rename), so the sheet re-reads.
    ///
    /// ⚠️ **The mirror of `onDidSave`.** I-0155 made a sheet save refresh the
    /// inspector and stopped there — the reverse direction was left broken, so an
    /// inspector rename left the sheet showing the old name. ⚠️ **The same two
    /// surfaces, the same stale-view bug, fixed in only one direction**, which is
    /// the `feedback_verify_each_half_separately` trap: I tested the half I built.
    var objectRevision: Int = 0
    let onClose: () -> Void

    /// Navigation state. Owned by the caller so it survives the pane being
    /// rebuilt, and so a window host could own it identically later.
    /// ⚠️ [T-0547] — THE HOST'S HANDOFF ONLY. ⛔ No longer the navigation model.
    ///
    /// ✅ `EditorView` writes the first object here before presenting; this sheet seeds
    /// its `trail` from it on appear and the STACK owns everything after.
    /// ⚠️ Kept as the handoff rather than a plain `Entry` parameter because the host
    /// also calls `reset()` on close, and [I-0168]'s guarded re-entry depends on it.
    @Bindable var history: ObjectDetailHistory

    @State private var detail: ObjectDetail?
    @State private var loadError: String?
    @State private var saveError: String?

    // Edit buffers. Separate from `detail` so a failed save never discards what
    // the writer typed — the same rule the inline editor follows (I-0126).
    @State private var draftName = ""
    @State private var draftSubtitle = ""
    @State private var draftNotes = ""
    @State private var draftTags: [String] = []
    @State private var newTag = ""
    /// ⚠️ T-0452: a pending exit that would discard unsaved edits. Carries WHAT
    /// the writer was trying to do, so answering the prompt completes it rather
    /// than merely dismissing.
    @State private var pendingExit: PendingExit?

    /// ⚠️ I-0168: a navigation requested from OUTSIDE the sheet — the Scene
    /// Inspector double-click, which calls the host directly.
    ///
    /// ⚠️ **The guard cannot live only in the sheet**, because the sheet is not
    /// on that path: the host owns `ObjectDetailHistory` and the inspector asks
    /// the host. So the host now hands the request HERE instead of applying it,
    /// and the sheet either performs it or prompts first. Same veto, one owner.
    var externalNavigation: ObjectDetailHistory.Entry?
    /// Cleared by the sheet once the request has been consumed.
    var onExternalNavigationHandled: () -> Void = {}

    /// Where the writer was heading when unsaved edits stopped her.
    private enum PendingExit: Equatable {
        case close
        case navigate(ObjectDetailHistory.Entry)
        /// ⚠️ [T-0547] — THE POP HAS ALREADY HAPPENED. This is not a request to
        /// navigate; it is a decision owed about the drafts left behind.
        /// ⛔ Two-way only (user ruling): Save or Discard, never Cancel.
        case navigatedAway(from: ObjectDetailHistory.Entry)
    }
    @State private var isSaving = false

    /// ⚠️ [I-0246] — WHAT JUST HAPPENED, because the sheet no longer closes to say it.
    ///
    /// ⛔ Save and Cancel deliberately keep the sheet open ([I-0245]): Save writes and
    /// stays so the writer can keep working; Cancel REVERTS (T-0452). ⚠️ With no exit
    /// to mark the moment, **a Save that worked and a Save that did nothing look
    /// identical** — the user's own observation.
    ///
    /// ✅ Cleared the instant `hasChanges` goes true again, so the banner can never
    /// describe a state the writer has already typed past.
    @State private var actionStatus: ActionStatus?

    /// The two outcomes worth reporting. ⛔ NOT a general message channel — failures
    /// have their own surface (`saveError`), which is louder and stays put.
    private enum ActionStatus {
        case saved, reverted

        var text: String {
            switch self {
            case .saved:    "Changes saved"
            case .reverted: "Changes reverted"
            }
        }

        var symbol: String {
            switch self {
            case .saved:    "checkmark.circle.fill"
            case .reverted: "arrow.uturn.backward.circle.fill"
            }
        }
    }
    /// ⚠️ T-0447: the object's image path, resolved by the CORE at list time
    /// (T-0446) rather than by this view. `ObjectDetail` carries only the
    /// assetID — a path is not in the object file, and deliberately is not.
    @State private var imagePath: String?
    /// ⚠️ Set when the object could not be opened because its WORLD is away
    /// (I-0166). Distinct from `loadError`, which means a real failure.
    @State private var unavailableStatus: WorldStatus?

    /// ⚠️ [T-0547] — THE STACK OWNS NAVIGATION. The path is the source of truth.
    ///
    /// ⛔ This replaced a hand-rolled cursor. `NavigationStack(path:)` draws the bar,
    /// the title and the back chevron; we contribute only the actions that are ours.
    @State private var path = NavigationPath()

    /// ⚠️ The labels behind the path, kept in step with it.
    ///
    /// ⛔ `NavigationPath` is TYPE-ERASED — `count` is readable, the VALUES are not — so
    /// nothing in SwiftUI can say what the object one step back is CALLED. ✅ This can.
    /// ⛔ It decides nothing: a pop is already a fact by the time we consult it.
    @State private var trail: [ObjectDetailHistory.Entry] = []

    var body: some View {
        // ⚠️ [T-0547] — THE `NavigationStack` OWNS THE BAR, THE TITLE AND THE CHEVRON.
        //
        // ⚠️ **THIS TOOK FOUR ATTEMPTS. The wrong turns, so they are not re-earned:**
        // ⛔ (1) `.toolbar` on a bare sheet — the SHEET WINDOW has no `NSToolbar`
        //        (`w.toolbar == nil`, measured), so every item was dropped SILENTLY,
        //        taking `.keyboardShortcut(.cancelAction)` with it → [I-0248].
        // ⛔ (2) A hand-built `VStack` bar — ⚠️ it rendered, but it hand-built chrome
        //        the platform provides, in the Epic that exists to STOP that.
        // ⛔ (3) A `NavigationStack` whose bar we still populated entirely ourselves —
        //        ⚠️ the stack was present but was never allowed to DO anything.
        // ✅ (4) This: the stack owns navigation. We push an `Entry`; it draws the
        //        title and the back chevron and manages the transition.
        //
        // ✅ PLACEMENT IS THE PLATFORM'S (user: *"put them where they are supposed to
        // go"*). On macOS: `.cancellationAction` sits trailing BEFORE confirmation, and
        // `.confirmationAction` is TRAILING-MOST.
        // ⚠️ **CLOSE IS THE CONFIRMATION ACTION** (user ruling) — it is what DISMISSES.
        // ⛔ Save does not dismiss ([I-0245]), so it sits BESIDE the dismissing item.
        // ⚠️ **[I-0249] — THE EDITOR RENDERS AT EVERY LEVEL; THE OBSERVERS RUN ONCE.**
        //
        // ⛔ TWO WRONG SHAPES BEFORE THIS ONE, both recorded because each looked right:
        //   (1) `sheetContent` — the editor AND its whole `.onChange` chain — returned
        //       from BOTH the root and the destination. Two live copies, each calling
        //       `load()` on a pop.
        //   (2) A `Color.clear` destination, to stop the duplication. ⛔ THAT BLANKED
        //       THE PUSHED OBJECT: the stack shows the DESTINATION, so pushing Colm
        //       displayed nothing. ⚠️ A fix built on an unconfirmed theory, which broke
        //       a working surface.
        // ✅ THE SPLIT: `editorSurface` is the visible editor and renders at both
        // levels — the writer must see Colm when she pushes him. The OBSERVERS
        // (`load`, the banner rules, the exit prompts) hang on the STACK, so they exist
        // exactly once no matter how deep the trail goes.
        sheetObservers(
        NavigationStack(path: $path) {
            editorSurface
                .navigationTitle(currentEntry?.displayName ?? "Object")
                .navigationDestination(for: ObjectDetailHistory.Entry.self) { entry in
                    // ✅ The SAME editor. ⚠️ It reads `detail`/the drafts, which live on
                    // this sheet above the stack — so both levels show the object the
                    // trail currently names, and neither owns state.
                    editorSurface
                        .navigationTitle(entry.displayName)
                }
                .toolbar { detailToolbar }
        }
        // ⚠️ THE OBSERVER CHAIN — ⛔ DELIBERATELY OUT HERE, not on `editorSurface`.
        // ✅ One instance, whatever the stack depth. See [I-0249].
        )

    }

    /// ⚠️ THE VISIBLE EDITOR — ⛔ NO observers. See `SheetObservers`.
    private var editorSurface: some View {
        VStack(alignment: .leading, spacing: 0) {
            if let unavailableStatus {
                // ⚠️ Named and explained, never a code. The writer asked for a
                // specific object; tell her about THAT object.
                unavailableView(unavailableStatus)
            } else if let loadError {
                message(loadError, systemImage: "exclamationmark.triangle")
            } else if let detail {
                content(for: detail)
            } else {
                message("Select an object to see its details.",
                        systemImage: "square.dashed")
            }

            // ⚠️ [I-0246] — bottom status banner. See `actionStatus`.
            //
            // ⚠️ OUTSIDE the content branch deliberately: it belongs to the SHEET, so
            // it pins to the bottom edge whatever the body is showing, and it is the
            // last thing in reading order — where the writer's eye lands after
            // pressing a button in the toolbar above.
            if let actionStatus {
                Divider()
                Label(actionStatus.text, systemImage: actionStatus.symbol)
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding(.horizontal, 12)
                    .padding(.vertical, 8)
                    .transition(.opacity)
                    // ⚠️ Announced, not just drawn — the writer may not be looking
                    // at the bottom of the sheet when she hits ⌘S.
                    .accessibilityAddTraits(.updatesFrequently)
            }
        }
        .animation(.easeOut(duration: 0.15), value: actionStatus)
    }

    /// ⚠️ [I-0249] — every `.onChange`/`.alert` the sheet needs, applied ONCE.
    ///
    /// ⛔ These used to hang on the editor itself, which the stack renders at EVERY
    /// level — so a two-deep trail ran each of them twice, and a pop produced two
    /// synchronous `load()` calls inside one layout pass.
    /// ✅ Applied to the `NavigationStack`, they exist once whatever the depth.
    @ViewBuilder
    private func sheetObservers(_ content: some View) -> some View {
        content
        // ⚠️ **THE TWO-WAY PROMPT — THE USER'S RULING, 2026-09-23:**
        // *"This confirmation dialog shouldn't attempt to 'Cancel' the navigation …
        // rather it should require either a save or a revert."*
        //
        // ✅ **THAT RULING IS WHAT MAKES THIS POSSIBLE AT ALL.** ⛔ No SwiftUI hook can
        // ARREST a pop — the macOS 27 SDK has no navigation function taking a closure.
        // ⚠️ An earlier design tried to re-push the popped view to "undo" the
        // navigation; ⛔ that flickers, and it asks a Cancel question about a view the
        // writer has already left. ✅ A settled pop needs no interception — only a
        // decision about the orphaned drafts, and those are still in hand.
        //
        // ⚠️ **ORDERING IS LOAD-BEARING AND MEASURED:** `path.count` changes BEFORE the
        // popped view's `onDisappear`, so the prompt is raised while the drafts live.
        .onChange(of: path.count) { old, new in
            guard new < old else { return }          // a push needs no decision

            // ⚠️ OFF-BY-ONE, CAUGHT BY MEASUREMENT (2026-09-23): the ROOT lives at
            // `path.count == 0` but is `trail[0]`, so the trail is always ONE LONGER
            // than the path. ⛔ `trail[new]` therefore names the object ARRIVED AT, not
            // the one departed — the first run of this prompt said "Myton" when the
            // writer had been editing Brother Colm.
            // ✅ The departed level is the trail's TAIL, before it is trimmed.
            let departed = trail.last
            trail = Array(trail.prefix(new + 1))
            let needsPrompt = hasChanges && !isReadOnlyNow && departed != nil

            // ⚠️ **DEFER OFF THE LAYOUT PASS — [I-0249].**
            //
            // ⛔ This handler runs INSIDE AppKit's layout, and `loadCurrent()` sets
            // `detail = nil` and re-reads from disk. Doing that synchronously mutates
            // state mid-layout, re-dirties constraints, and AppKit loops:
            //   "It's not legal to call -layoutSubtreeIfNeeded on a view which is
            //    already being laid out."
            // ✅ [T-0548]'s DEBUG ASSERTION CAUGHT THIS — 65 Update-Constraints passes
            // in one run-loop turn, trapped before AppKit's own limit. ⚠️ The guard's
            // message names the [I-0245] HStack shape, which is NOT this cause: the
            // guard detects the runaway, it does not diagnose it.
            //
            // ✅ A hop to the next run-loop turn lets the pop's layout FINISH first.
            // ⛔ NOT `withAnimation` or a state flag — the problem is the TIMING, not
            // the animation, and a flag would still mutate inside the same pass.
            // ⛔ NO `loadCurrent()` HERE. ⚠️ Trimming `trail` above changes
            // `currentEntry`, whose own observer re-reads — calling it here too gave
            // TWO loads per pop, which is half of what [I-0249] was.
            if needsPrompt, let departed {
                Task { @MainActor in pendingExit = .navigatedAway(from: departed) }
            }
        }
        // ⚠️ Save-or-revert. ⛔ NO Cancel: the navigation has already happened, and
        // offering to undo it would promise something the platform cannot deliver.
        .alert("Unsaved changes", isPresented: showingNavigatedAwayPrompt) {
            Button("Save Changes") { resolveNavigatedAway(saving: true) }
            Button("Discard Changes", role: .destructive) {
                resolveNavigatedAway(saving: false)
            }
        } message: {
            Text("You have unsaved changes to “\(navigatedAwayName)”.")
        }
        // ⚠️ [I-0246] — THE DISMISSAL RULE, and why it is `hasChanges` and not a timer.
        //
        // ✅ The user's ruling: *"Once more changes are detected … the message should be
        // dismissed. That way the writer is not confused by the message and the changes
        // she is making."*
        //
        // ✅ `hasChanges` is ALREADY the truth here — it is what enables Save and Cancel,
        // and it compares the drafts against `detail`, which `load()` refreshes from disk
        // after a save. So it goes false on save/revert and true again on the next
        // keystroke, with no extra state to drift.
        // ⛔ NOT a timed auto-hide: a banner that vanishes on its own is missed by a
        // writer who looked away, and one that lingers past new typing is the confusion
        // this rule exists to prevent.
        .onChange(of: hasChanges) { _, changed in
            if changed { actionStatus = nil }
        }
        .frame(minWidth: 420, minHeight: 320)
        // ⚠️ T-0452: closing or navigating away used to DISCARD unsaved edits
        // silently — the third route into this Epic's recurring data loss
        // (I-0155, I-0165b), and the only one the writer triggers herself.
        .alert("Save your changes?", isPresented: showingExitPrompt) {
            Button("Save") { resolveExit(saving: true) }
            Button("Discard", role: .destructive) { resolveExit(saving: false) }
            Button("Cancel", role: .cancel) { pendingExit = nil }
        } message: {
            Text("You have unsaved changes to “\(detail?.displayName ?? "this object")”. "
                 + "Closing without saving will discard them.")
        }
        .onChange(of: externalNavigation) { _, entry in
            guard let entry else { return }
            onExternalNavigationHandled()
            requestNavigate(entry)
        }
        // ⚠️ [I-0246]: a status belongs to the object it happened to. Clear it when
        // the sheet moves to a DIFFERENT object, or "Changes saved" would follow the
        // writer onto an object she has not touched.
        // ⛔ NOT inside `load()` — `save()` and `revert()` both call that, so clearing
        // there would wipe the banner before it ever appeared.
        // ⚠️ **[I-0249] — DEFERRED OFF THE LAYOUT PASS.** A pop trims `trail` while
        // AppKit is laying out, so this observer fires mid-pass; `load()` mutates
        // `detail`, which re-dirties constraints and loops
        // (*"not legal to call -layoutSubtreeIfNeeded on a view which is already being
        // laid out"*). ✅ A hop to the next turn lets the pop's layout finish.
        .onChange(of: currentEntry) { _, _ in
            Task { @MainActor in
                actionStatus = nil
                load()
            }
        }
        // ⚠️ I-0160: another surface changed this object — re-read.
        //
        // ⚠️ This comment used to ASSERT that `load()` only overwrote unedited
        // draft fields. It did not — it overwrote all three unconditionally, so
        // every reload discarded the writer's typing (I-0165b). ✅ The property is
        // now actually IMPLEMENTED in `load()`, not merely claimed here.
        // The save path also re-reads and merges per field (I-0155).
        .onChange(of: objectRevision) { _, _ in load() }
        .onChange(of: worldRevision) { _, _ in load() }
        // ⚠️ [T-0547] — SEED THE TRAIL FROM THE HOST'S HANDOFF.
        //
        // ⛔ Without this the stack has no root and the sheet opens blank: `trail` is
        // `@State` and starts empty, while the object the writer double-clicked was
        // recorded on `history` BEFORE the sheet was presented.
        .onAppear {
            if trail.isEmpty, let seed = history.current { trail = [seed] }
            load()
        }
        // ✅ **[I-0250] — ESC, AT THE LEVEL THE KEY ACTUALLY ARRIVES.**
        //
        // ⚠️ *"The user generates an exit command by pressing the Menu button on tvOS,
        // or the escape key on macOS"* — and it fires on the FOCUSED view, which is the
        // sheet's content. ⛔ A `.keyboardShortcut(.cancelAction)` on a toolbar item
        // does NOT get there: SwiftUI's sheet dismissal takes the key first.
        // ✅ Routing through `requestClose()` means Esc honours the unsaved-changes
        // guard exactly as clicking ✕ does.
        .onExitCommand { requestClose() }
        // ⚠️ **THE TRIPWIRE — [I-0250].** ⛔ If the sheet is ever torn down with unsaved
        // edits still present, something bypassed the guard and the writer just lost
        // work SILENTLY. ✅ This session found TWO separate routes into that state
        // ([I-0248], [I-0250]), so the condition is worth naming loudly rather than
        // trusting it cannot recur.
        // ⛔ DELIBERATELY LOGS RATHER THAN AUTOSAVES: a silent write is the opposite
        // failure, and T-0452's whole point is that the writer decides. ⚠️ If this ever
        // appears in a log, it is a DEFECT — find the route, do not add a save here.
        .onDisappear {
            if hasChanges && !isReadOnlyNow {
                NSLog("[SCRIVI-GUARD] ⛔ Detail Sheet dismissed with UNSAVED CHANGES to "
                      + "'\(currentEntry?.displayName ?? "unknown")' — the exit guard was "
                      + "BYPASSED. This is [I-0250]'s class; the writer lost edits.")
            }
        }
    }

    // MARK: — Chrome

    /// Save · Undo (beside the dismissing item) · Close (trailing-most).
    ///
    /// ⛔ **NO BACK ITEM.** ✅ The stack draws its own chevron, hides it at the root,
    /// and labels it with the previous title — all of which this used to hand-roll.
    ///
    /// ⚠️ **ICONS, WORDS ON `.help()`** (user ruling: *"I would now prefer if they use
    /// standard icons"*). ⛔ The labels are not lost — they move to tooltips, so
    /// assistive technology still reads them.
    @ToolbarContentBuilder
    private var detailToolbar: some ToolbarContent {

        // ⚠️ SAVE SITS BESIDE THE DISMISSING ITEM (user ruling). It does NOT dismiss:
        // Save writes, re-reads disk and KEEPS THE SHEET OPEN ([I-0245]) — which is why
        // [I-0246]'s status banner exists to report it.
        // ✅ Enable/disable belongs HERE (user ruling).
        ToolbarItemGroup(placement: .cancellationAction) {
            // ⚠️ `internaldrive` is a DISK (user correction, 2026-09-23).
            // ⛔ NOT `square.and.arrow.down` — that is the DOWNLOAD/export glyph, and
            // it says "bring this in", not "commit this to storage".
            Button { save() } label: {
                Image(systemName: "internaldrive")
            }
            .disabled(isSaving || !hasChanges)
            .keyboardShortcut("s", modifiers: .command)
            .help("Save Changes")

            // ⚠️ T-0452: revert to what is on disk. Deliberately NOT undo — no history,
            // no per-keystroke state. The writer asked for exactly this: *"a Cancel
            // option next to Save would allow me to just revert back to the saved
            // version."* ⚠️ EP-019's sentence-granular history is for the manuscript and
            // stays out of object editing (the D3-C ruling).
            Button { revert() } label: {
                Image(systemName: "arrow.uturn.backward")
            }
            .disabled(isSaving || !hasChanges)
            .help("Undo Changes — discard your edits and return to the saved version")
        }

        // ⚠️ **CLOSE IS THE CONFIRMATION ACTION** — it is what dismisses the sheet.
        //
        // ⚠️ [I-0245] — close KEEPS its THREE-WAY prompt (user ruling): Save · Discard
        // · Cancel. ✅ Cancel is meaningful HERE — she may not want to close at all —
        // ⛔ where on a navigation it would offer to undo something already done.
        //
        // ⛔ **NO `.keyboardShortcut(.cancelAction)` HERE — [I-0250].** ⚠️ MEASURED: a
        // `.cancelAction` on a TOOLBAR item never receives Esc. SwiftUI's own sheet
        // dismissal consumes the key first, and that path bypasses `requestClose()`
        // entirely — the probe logged `sheet DISAPPEARED hasChanges=true` with the
        // button's action NEVER firing, and the writer's edits died with the view.
        // ⚠️ `.interactiveDismissDisabled()` does NOT stop it: that governs INTERACTIVE
        // dismissal (click-outside, drag), not the Escape key.
        // ✅ Esc is handled by `.onExitCommand` on the CONTENT, which is the level the
        // key actually reaches. See `sheetObservers`.
        ToolbarItem(placement: .confirmationAction) {
            Button { requestClose() } label: {
                Image(systemName: "xmark")
            }
            .help("Close the detail view (Esc)")
        }
    }

    // MARK: — Chrome

    // ⚠️⚰️ TOMBSTONE — T-0547 (EP-040 AC8, app-shape §4.4), 2026-09-23.
    //
    // ⛔ REMOVED: `private var toolbar: some View` — an `HStack(spacing: 8)` of
    // `.borderless` buttons (back · forward · Spacer · Cancel · Save · ✕) rendered as
    // the FIRST child of the sheet's `VStack`, above a `Divider()`.
    //
    // ⚠️ WHY IT WAS WRONG: it was WINDOW CHROME INSIDE A CONTENT PANE — navigation
    // history, a save affordance and a CLOSE BUTTON, all hand-drawn where the platform
    // provides `.toolbar`. The same class as [SP-134]'s missing `NSToolbar` and
    // [SP-135]'s `VStack` bars: *the app hand-builds what the platform already gives it.*
    //
    // ✅ REPLACED BY, in two places and deliberately not one:
    //   • `detailToolbar` (above) — Back · Save · Undo · Close, at PLATFORM placements.
    //   • `confirmationFooter` (below) — Cancel · Save, at the CONTENT'S FOOT.
    //
    // ⚠️ THE SPLIT IS THE Q2 RULING (user, 2026-09-23): **Save and Cancel are NOT
    // window chrome.** They are DOCUMENT ACTIONS, and a sheet's convention is
    // confirmation at the foot of its content. ⛔ Moving them into the toolbar would
    // have been a change of behaviour dressed up as a conformance fix.

    // ⚠️⚰️ TOMBSTONE — `confirmationFooter(for:)`, added and removed the same day.
    //
    // ⛔ [T-0547] first placed Save/Cancel at the CONTENT'S FOOT, on the reading that a
    // sheet conventionally puts confirmation there and that they are document actions
    // rather than window chrome.
    // ✅ **THE USER RULED OTHERWISE (2026-09-23):** *"The toolbar should contain the
    // Cancel Save buttons. Although, since they do not dismiss the panel, they should
    // be centered."* ⚠️ The premise was right — they are not dismissal — but the
    // conclusion was placement, and placement was the user's to make.
    // ✅ They now live in `detailToolbar` at `.cancellationAction`, BESIDE the
    // dismissing item, as icons with their words on `.help()`.
    // ⚠️ Two wrong turns on the way, both recorded on `body`: a `.principal` centred
    // pair (dropped because the sheet window has no toolbar — but the NAVIGATION STACK
    // does, which was the real answer), and a hand-built `VStack` bar (⛔ chrome this
    // Epic exists to remove).

    // MARK: — Content

    @ViewBuilder
    private func content(for detail: ObjectDetail) -> some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 14) {

                // ⚠️ R9 / I-0138: a read-only object must say WHY, in words. A
                // greyed-out field that explains nothing is the defect, not the
                // fix — "disabled AND explained" is the rule.
                if let banner = readOnlyBanner(for: detail) {
                    Label(banner, systemImage: "lock")
                        .font(.callout)
                        .foregroundStyle(.secondary)
                        .padding(8)
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
                }

                // ⚠️ T-0450 — AC9's second half: EDITS AT RISK.
                //
                // ✅ The recover-and-save path already worked and was verified by
                // use: the world returns, the fields unlock, Save writes her
                // typing. ⚠️ **What was missing is that she is never TOLD** her
                // unsaved work is held only in memory — and if Scrivi quits before
                // the world returns, it is gone.
                //
                // ⚠️ **Deliberately a SECOND banner, not a longer first one.** The
                // read-only banner states a fact about the object; this states a
                // risk to HER WORK, and folding them together buries the urgent one
                // in the routine one.
                //
                // ⚠️ Shown only when edits actually exist (`hasChanges`) — a
                // warning on an untouched sheet trains her to ignore it.
                if isReadOnly(detail) && hasChanges {
                    Label(
                        "Your unsaved changes are held here only. "
                        + "They will be saved when this object's world is available "
                        + "again — but they will be lost if Scrivi quits first.",
                        systemImage: "exclamationmark.triangle.fill"
                    )
                    .font(.callout)
                    .foregroundStyle(.orange)
                    .padding(8)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .background(.orange.opacity(0.12),
                                in: RoundedRectangle(cornerRadius: 6))
                }

                field("Name") {
                    TextField("Name", text: $draftName)
                        .textFieldStyle(.roundedBorder)
                        .disabled(isReadOnly(detail))
                }

                field("Subtitle") {
                    TextField("A one-line descriptor", text: $draftSubtitle)
                        .textFieldStyle(.roundedBorder)
                        .disabled(isReadOnly(detail))
                }

                // D3-A: a plain TextEditor. ⚠️ D3-C (reusing the manuscript
                // editor) was RULED OUT, not deferred — it would drag EP-019's
                // sentence-granular history engine into object editing.
                field("Notes") {
                    if isReadOnly(detail) {
                        // ⚠️ `.disabled()` DOES NOT MAKE A TextEditor READ-ONLY —
                        // found in the user's live click-through of SP-117. The
                        // two TextFields above honoured it and the TextEditor did
                        // not, so Notes stayed editable directly beneath a banner
                        // saying "read only". This is the app's FIRST disabled
                        // TextEditor; the one other TextEditor in Scrivi
                        // (`WritingToolCards.swift:209`) is never disabled, so
                        // there was no precedent to inherit the problem from.
                        //
                        // ⚠️ Nothing could ever have reached disk — Save is hidden
                        // when read-only — so this was never a write-safety bug.
                        // The real cost was LOST TYPING: `load()` overwrites the
                        // draft from disk, so a paragraph typed during an outage
                        // vanished silently on navigating away and back.
                        //
                        // Rendered as text rather than an inert editor so the
                        // state is visible, not just enforced.
                        ScrollView {
                            Text(draftNotes.isEmpty ? "No notes." : draftNotes)
                                .font(.body)
                                .foregroundStyle(draftNotes.isEmpty ? .tertiary : .secondary)
                                .frame(maxWidth: .infinity, alignment: .leading)
                                .textSelection(.enabled)   // readable ≠ editable
                                .padding(4)
                        }
                        .frame(minHeight: 160)
                        .overlay(RoundedRectangle(cornerRadius: 6)
                            .stroke(.quaternary))
                    } else {
                        TextEditor(text: $draftNotes)
                            .font(.body)
                            .frame(minHeight: 160)
                            .overlay(RoundedRectangle(cornerRadius: 6)
                                .stroke(.quaternary))
                    }
                }

                // ⚠️ T-0449 — EDITABLE at last, closing R2 and AC2.
                //
                // SP-117 deferred this "for want of a chip-editor precedent in the
                // app". ⚠️ **The precedent was `WritingToolCards.swift` all along**
                // — the scene Tags card has had `TagChip` and `FlowLayout` since
                // T-0363. They are reused here rather than rewritten, which is the
                // rule SP-118 paid for four times.
                field("Tags") {
                    VStack(alignment: .leading, spacing: 6) {
                        if draftTags.isEmpty {
                            Text(isReadOnly(detail) ? "No tags." : "No tags yet.")
                                .font(.callout)
                                .foregroundStyle(.secondary)
                        } else {
                            FlowLayout(spacing: 6) {
                                ForEach(draftTags, id: \.self) { tag in
                                    if isReadOnly(detail) {
                                        // ⚠️ I-0148's lesson: read-only must be
                                        // ENFORCED, not merely styled. A chip whose
                                        // ✕ still worked would discard a tag that
                                        // could never be saved.
                                        Text(tag)
                                            .font(.caption)
                                            .padding(.horizontal, 7)
                                            .padding(.vertical, 3)
                                            .background(.quaternary, in: Capsule())
                                    } else {
                                        TagChip(tag: tag) {
                                            draftTags.removeAll { $0 == tag }
                                        }
                                    }
                                }
                            }
                        }

                        if !isReadOnly(detail) {
                            TextField("Add a tag…", text: $newTag)
                                .textFieldStyle(.roundedBorder)
                                .font(.callout)
                                .onSubmit { addTag() }
                        }
                    }
                }

                if let saveError {
                    Label(saveError, systemImage: "exclamationmark.triangle")
                        .font(.callout)
                        .foregroundStyle(.orange)
                }

                Divider()

                // R3/R4/R5/R7 (SP-118): what this object is connected to, and
                // where a writer connects it to something else. Self-contained so
                // the sheet stays host-independent (S8) — it is handed the engine
                // and the worlds the sheet already has, and hands navigation back.
                // AC3 (T-0447): the image. Placed above relationships — what the
                // object IS comes before what it is connected to.
                ObjectImageSection(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    objectID: detail.objectID,
                    objectKind: detail.kind,
                    worldID: detail.worldID,
                    projectID: projectID,
                    authorshipRef: authorshipRef,
                    isReadOnly: isReadOnly(detail),
                    imagePath: imagePath,
                    imageAssetID: detail.imageAssetID,
                    onChanged: {
                        load()        // re-read the object and its image path
                        onDidSave()   // and let the host refresh the inspector
                    }
                )

                Divider()

                // ⚠️ R6 / AC8 (SP-120): the citation surface, placed between what
                // the object IS (image, fields) and what it is connected to.
                //
                // ⚠️ **Deliberately its own section, not a row in the relations
                // list.** A `cites` edge would otherwise appear there as an
                // ordinary relationship — technically true, and useless: a
                // citation is attribution, and burying it among "sibling-of" and
                // "located-at" is what made source creation unreachable for four
                // sprints. ⚠️ Uncited objects still show the relations list
                // unchanged; nothing here replaces it (Q-b's additive rule).
                ObjectSourcesSection(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    objectID: detail.objectID,
                    objectDisplayName: detail.displayName,
                    authorshipRef: authorshipRef,
                    isReadOnly: isReadOnly(detail),
                    onNavigate: { entry in requestNavigate(entry) },
                    onChanged: {
                        // The graph changed — let the host refresh the Scene
                        // Inspector, whose `sources` card reads this same edge.
                        onDidSave()
                    }
                )

                Divider()

                ObjectRelationsSection(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    objectID: detail.objectID,
                    worlds: worlds,
                    sceneNames: sceneNames,
                    isReadOnly: isReadOnly(detail),
                    onSelectScene: onSelectScene,
                    onNavigate: { entry in
                        // ⚠️ `visit` truncates forward history and no-ops on a
                        // re-visit of the object already showing; `load()` then
                        // runs from `.onChange(of: currentEntry)`.
                        requestNavigate(entry)
                    }
                )

                Divider()
                metadata(for: detail)
            }
            .padding(12)
        }
    }

    @ViewBuilder
    private func field<Content: View>(_ label: String,
                                      @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)
            content()
        }
    }

    @ViewBuilder
    private func metadata(for detail: ObjectDetail) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(detail.kind.capitalized)
            if !detail.worldID.isEmpty {
                // ⚠️ I-0142: an object's world is shown as a LABEL, never a
                // picker. A picker implied the world could be changed, and moving
                // objects between worlds is an unruled operation.
                Text("World: \(worldName(for: detail.worldID))")
            }
            if !detail.modifiedAt.isEmpty {
                Text("Modified \(detail.modifiedAt)")
            }
        }
        .font(.caption)
        .foregroundStyle(.secondary)
    }

    /// The object exists; its world is temporarily out of reach (R9, I-0166).
    ///
    /// ⚠️ **Never says "not found" or shows an error code.** Doc 3: absence is
    /// never deletion — and to a writer, "error -1" on a character she wrote is
    /// indistinguishable from having lost her.
    @ViewBuilder
    private func unavailableView(_ status: WorldStatus) -> some View {
        VStack(spacing: 10) {
            Image(systemName: "externaldrive.badge.xmark")
                .font(.title2)
                .foregroundStyle(.secondary)
                .accessibilityHidden(true)

            // ⚠️ The NAME comes from history, which holds it precisely so a
            // writer is never asked to recognise an ID (the AC-A7 rule again).
            Text(currentEntry?.displayName ?? "This object")
                .font(.headline)

            Text(worldSentence(status))
                .font(.callout)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)

            Text("Its details will appear when the world is available again.")
                .font(.caption)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
    }

    /// ⚠️ The status is DIAGNOSTIC, not behavioural (Doc 2 §7.2.1): the behaviour
    /// is identical for every value, but the remedy differs — so name the world
    /// when we can, and never guess at a cause.
    private func worldSentence(_ status: WorldStatus) -> String {
        let name = currentEntry.map { worldName(for: $0.worldID) }
        if let name, !name.isEmpty {
            return "“\(name)” is \(status.writerDescription)."
        }
        return "This object's world is \(status.writerDescription)."
    }

    private func message(_ text: String, systemImage: String) -> some View {
        VStack(spacing: 8) {
            Image(systemName: systemImage).font(.title2)
            Text(text).multilineTextAlignment(.center)
        }
        .foregroundStyle(.secondary)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
    }

    // MARK: — State

    private var hasChanges: Bool {
        guard let detail else { return false }
        return draftName != detail.displayName
            || draftSubtitle != detail.subtitle
            || draftNotes != detail.notes
            || draftTags != detail.tags
    }

    /// ⚠️ R9: an object whose world is away is read-only. Doc 3 — *absence is
    /// never deletion* — so the sheet shows it, names the problem, and refuses to
    /// write. It is never hidden and never silently editable.
    private func isReadOnly(_ detail: ObjectDetail) -> Bool {
        guard !detail.worldID.isEmpty else { return false }
        guard let world = worlds.first(where: { $0.worldID == detail.worldID }) else {
            return true   // bound to a world this project cannot see
        }
        return world.worldStatus != .available
    }

    private func readOnlyBanner(for detail: ObjectDetail) -> String? {
        guard isReadOnly(detail) else { return nil }
        guard let world = worlds.first(where: { $0.worldID == detail.worldID }) else {
            return "This object belongs to a world this project cannot find, "
                 + "so it cannot be edited here."
        }
        // T-0440: when the core told us WHY, say why.
        if let explanation = world.unavailabilityExplanation {
            return explanation
        }
        return "“\(world.displayName)” is \(world.worldStatus.rawValue), "
             + "so this object is read-only until it is available again."
    }

    private func worldName(for worldID: String) -> String {
        worlds.first { $0.worldID == worldID }?.displayName ?? worldID
    }

    // MARK: — Load / save

    /// ⚠️ Duplicates are refused rather than appended — two identical chips are
    /// unremovable individually, since the ✕ matches by value.
    /// Bound rather than a plain Bool so dismissing the alert any other way
    /// clears the pending exit instead of stranding it.
    /// ⚠️ The THREE-WAY prompt — close only (user ruling, 2026-09-23).
    /// ⛔ Deliberately excludes `.navigatedAway`, which gets its own two-way alert:
    /// Cancel is meaningful when closing (she may not want to) and MEANINGLESS after a
    /// pop that has already happened.
    private var showingExitPrompt: Binding<Bool> {
        Binding(get: {
            if case .navigatedAway = pendingExit { return false }
            return pendingExit != nil
        }, set: { if !$0 { pendingExit = nil } })
    }

    /// ⚠️ The TWO-WAY prompt — a navigation that is already a fact.
    private var showingNavigatedAwayPrompt: Binding<Bool> {
        Binding(get: {
            if case .navigatedAway = pendingExit { return true }
            return false
        }, set: { if !$0 { pendingExit = nil } })
    }

    /// The object the drafts belong to — named, never an ID.
    private var navigatedAwayName: String {
        if case .navigatedAway(let entry) = pendingExit { return entry.displayName }
        return "this object"
    }

    /// ⚠️ Save or discard the drafts orphaned by a pop, then adopt the object the
    /// stack has ALREADY moved to. ⛔ Neither branch navigates: that is settled.
    private func resolveNavigatedAway(saving: Bool) {
        pendingExit = nil
        if saving { save() }
        loadCurrent()
    }

    /// ⚠️ WHAT THE STACK IS SHOWING — the trail's last entry.
    ///
    /// ⛔ Replaces `history.current`. The stack owns position; `trail` owns the labels,
    /// and its tail is by construction the level on screen.
    private var currentEntry: ObjectDetailHistory.Entry? { trail.last }

    /// Re-reads whatever the stack is now showing.
    private func loadCurrent() {
        detail = nil          // force `load()` to adopt disk wholesale
        load()
    }

    /// Discards the writer's edits and re-adopts what is on disk.
    ///
    /// ⚠️ Re-reads rather than restoring from the in-memory `detail`: another
    /// surface may have written since (I-0155), and "the saved version" means
    /// what is actually saved, not what this sheet last saw.
    private func revert() {
        guard currentEntry != nil else { return }
        detail = nil          // force load() to adopt disk wholesale
        load()
        saveError = nil
        actionStatus = .reverted   // [I-0246]: the sheet stays open, so it must say so
    }

    /// ⚠️ Every exit routes through here. A close or a navigation with unsaved
    /// edits ASKS; without them it proceeds unchanged.
    private func requestClose() {
        if hasChanges && !isReadOnlyNow {
            pendingExit = .close
        } else {
            onClose()
        }
    }

    /// ⚠️ A double-click on a related object. ⛔ Still guarded THREE-WAY: nothing has
    /// moved yet, so Cancel is meaningful — unlike a pop, which is already a fact.
    private func requestNavigate(_ entry: ObjectDetailHistory.Entry) {
        if hasChanges && !isReadOnlyNow {
            pendingExit = .navigate(entry)
        } else {
            pushEntry(entry)
        }
    }

    /// ⚠️ Read-only objects cannot be saved, so their drafts are already covered
    /// by T-0450's at-risk banner — prompting here would offer a Save that is
    /// disabled two points away.
    private var isReadOnlyNow: Bool {
        guard let detail else { return false }
        return isReadOnly(detail)
    }

    // ⚠️⚰️ TOMBSTONE — `requestStep()`, removed by [T-0547] 2026-09-23.
    //
    // ⛔ It guarded a hand-built Back button, deferring the pop until the writer
    // answered a THREE-WAY prompt. ✅ The stack now owns the chevron, so there is no
    // button to guard and no pop to defer — the pop happens, and `onChange(of:
    // path.count)` raises the TWO-WAY prompt about the drafts left behind.
    // ⚠️ That is the user's ruling: *"This confirmation dialog shouldn't attempt to
    // 'Cancel' the navigation … rather it should require either a save or a revert."*

    /// ⚠️ Resolves the THREE-WAY close prompt only. ⛔ `.navigatedAway` never reaches
    /// here — it has its own two-way alert and `resolveNavigatedAway`.
    private func resolveExit(saving: Bool) {
        let exit = pendingExit
        pendingExit = nil
        if saving { save() }
        switch exit {
        case .close:               onClose()
        case .navigate(let entry): pushEntry(entry)
        case .navigatedAway:       break   // handled by resolveNavigatedAway
        case .none:                break
        }
    }

    /// ⚠️ Pushes onto the STACK, and records the label beside it.
    ///
    /// ⛔ `NavigationPath` is type-erased, so `trail` is the only thing that can
    /// afterwards say what each level is CALLED.
    private func pushEntry(_ entry: ObjectDetailHistory.Entry) {
        guard trail.last?.objectID != entry.objectID else { return }
        if trail.isEmpty {
            // ⚠️ The ROOT is not a push — it is what the stack already shows.
            trail = [entry]
            loadCurrent()
        } else {
            trail.append(entry)
            path.append(entry)
        }
    }

    private func addTag() {
        let tag = newTag.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !tag.isEmpty, !draftTags.contains(tag) else {
            newTag = ""
            return
        }
        draftTags.append(tag)
        newTag = ""
    }

    private func navigate(_ entry: ObjectDetailHistory.Entry?) {
        guard entry != nil else { return }
        load()
    }

    private func load() {
        guard let entry = currentEntry else {
            detail = nil
            return
        }
        loadError = nil
        saveError = nil
        unavailableStatus = nil
        do {
            let opened = try engine.openObject(
                projectRootPath: projectRootPath,
                objectKind: entry.kind,
                objectID: entry.objectID,
                worldID: entry.worldID
            )
            let parsed = try ObjectDetail(json: opened.objectJson, kind: entry.kind)
            // ⚠️ Resolved by the CORE, once, from the listing — not by walking the
            // assets directory here. Empty is normal: no image, or its world is
            // away (T-0446).
            imagePath = (try? engine.listObjects(projectRootPath: projectRootPath)
                .objects.first { $0.objectID == parsed.objectID }?.imagePath) ?? nil
            // ⚠️ I-0165b: adopt disk ONLY for fields the writer has not edited.
            //
            // This used to overwrite all three unconditionally — and a comment on
            // `onChange(of: objectRevision)` claimed the opposite ("load() only
            // overwrites a draft field the writer has not edited"). ⚠️ **The claim
            // was false and nothing enforced it**, so any reload — an Inspector
            // rename (I-0160), a remount (I-0162) — silently discarded her typing.
            // ⚠️ **A comment asserting a safety property is not that property**;
            // the same failure as I-0151.
            //
            // `previous` is what disk said last time we read. A draft still equal
            // to it is untouched, so taking the new value is right. A draft that
            // differs is HER work and is kept — she decides at Save, which already
            // merges per field (I-0155).
            // ⚠️ Only when this is the SAME object. Navigating elsewhere must adopt
            // disk wholesale — the drafts belong to the object we just left, and
            // carrying them across would write one object's text onto another.
            let previous = (detail?.objectID == parsed.objectID) ? detail : nil
            let isSameObject = previous != nil

            if !isSameObject || draftName == (previous?.displayName ?? draftName) {
                draftName = parsed.displayName
            }
            if !isSameObject || draftSubtitle == (previous?.subtitle ?? draftSubtitle) {
                draftSubtitle = parsed.subtitle
            }
            if !isSameObject || draftNotes == (previous?.notes ?? draftNotes) {
                draftNotes = parsed.notes
            }
            if !isSameObject || draftTags == (previous?.tags ?? draftTags) {
                draftTags = parsed.tags
            }
            detail = parsed
        } catch {
            // ⚠️ I-0165: a failed re-read must NOT throw the sheet away.
            //
            // R9 requires that an object whose world is away stays **shown,
            // read-only and explained** — never hidden. But `load()` also runs on
            // `worldRevision` (I-0162), and ejecting the drive makes `openObject`
            // fail — so this branch replaced the whole sheet with a raw
            // "ScriviError -1", ⚠️ **and `detail = nil` DISCARDED THE WRITER'S
            // UNSAVED EDITS**, which is the more serious half.
            //
            // ⚠️ The distinction that matters: **losing the ability to re-read is
            // not the same as having nothing to show.** If we already have the
            // object, keep it — `isReadOnly` will report the outage in words, the
            // banner explains it, and her drafts survive until the world returns.
            if detail != nil {
                imagePath = nil   // the bytes are unreachable; stop pointing at them
                // Deliberately NOT setting loadError: the read-only banner already
                // says why, and two explanations of one outage read as two faults.
            } else {
                // ⚠️ I-0166: NOT "nothing to show" — the earlier comment here said
                // that and it was wrong.
                //
                // `currentEntry` carries the object's NAME, kind and world, so
                // we know exactly what she asked for and why it will not open.
                // R9 requires it be shown, named and explained; a raw
                // "ScriviApp:Scrivi Error -1" is none of those.
                //
                // ⚠️ This is the COLD-OPEN case: the drive was already away when
                // she clicked Show. The earlier fix only covered a sheet that was
                // already displaying an object, which is why this survived it.
                detail = nil
                if (error as? ScriviError)?.isWorldUnavailable == true {
                    unavailableStatus =
                        (error as? ScriviError)?.unavailableWorldStatus ?? .unavailable
                    loadError = nil
                } else {
                    unavailableStatus = nil
                    loadError = error.localizedDescription
                }
            }
        }
    }

    private func save() {
        guard let detail, !isReadOnly(detail) else { return }
        isSaving = true
        defer { isSaving = false }
        do {
            // ⚠️ I-0155: PATCH THE DOCUMENT AS IT IS ON DISK RIGHT NOW, not the
            // snapshot taken when this sheet opened.
            //
            // `detail.sourceJson` was read at open. If anything else edited the
            // object since — the Scene Inspector's rename is the reachable case,
            // because BOTH surfaces can be open on the same object at once —
            // patching the stale copy writes the OLD value back over the newer one.
            // Saving a note silently reverted a rename, and the sheet then showed
            // the reverted name as though nothing had happened.
            //
            // ⚠️ Re-reading is what `ObjectCardModel.rename` already does
            // (`ObjectCard.swift:280`) — it opens the object immediately before
            // patching. This surface simply did not follow the same rule.
            //
            // ⚠️ Still a PATCH, never a reconstruction (ObjectDetail.applyingEdits):
            // a rebuilt document would drop `image`, `attributes` and every field a
            // later core adds.
            let onDisk = try engine.openObject(
                projectRootPath: projectRootPath,
                objectKind: detail.kind,
                objectID: detail.objectID,
                worldID: detail.worldID
            )
            let current = try ObjectDetail(json: onDisk.objectJson, kind: detail.kind)

            // ⚠️ Re-reading fixes the OVERWRITE, but it must not silently swap the
            // conflict's direction. If a field changed on disk since this sheet
            // opened AND the writer did not touch it here, her draft still holds
            // the old value — patching it back is the same loss with the roles
            // reversed. Take what she did not edit from disk instead.
            let nameToWrite = (draftName == detail.displayName)
                ? current.displayName : draftName
            let subtitleToWrite = (draftSubtitle == detail.subtitle)
                ? current.subtitle : draftSubtitle
            let notesToWrite = (draftNotes == detail.notes)
                ? current.notes : draftNotes
            let tagsToWrite = (draftTags == detail.tags)
                ? current.tags : draftTags

            let patched = try current.applyingEdits(
                displayName: nameToWrite,
                subtitle: subtitleToWrite,
                notes: notesToWrite,
                tags: tagsToWrite
            )
            _ = try engine.saveObject(
                projectRootPath: projectRootPath,
                objectKind: detail.kind,
                objectJson: patched,
                authorshipRef: authorshipRef
            )
            saveError = nil
            load()        // re-read so the view shows what is actually on disk
            onDidSave()   // I-0155: and tell the host, so the inspector agrees
            actionStatus = .saved   // [I-0246]: the sheet stays open, so it must say so
        } catch {
            // Keep the drafts — her typing is not thrown away on a failed save.
            saveError = error.localizedDescription
        }
    }
}
