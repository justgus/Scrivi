# Verified Issues: I-0311 – I-0320

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0301–I-0310 are in [`Issue-verified-0301-0310.md`](Issue-verified-0301-0310.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0311** | `[ScriviCore]` ⛔ **A structured paste that opens a CHAPTER in the middle of a chapter scrambles the reading order: the target scene's remaining text lands AFTER the scenes that followed it.** Found 2026-10-17 by reading the core while measuring [I-0297]; ⚠️ the new `[I-0297]` ABI test asserts the wrong order (expectations taken from the code). → [details](#i-0311) | **High** | [SP-191] | ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21) |
| **I-0313** | `[Apple]` ⚠️ **Typing into an empty scene that sits right before another empty scene is credited to the NEXT scene: the page shows the letter in the first, the scene list puts a newline in the second, and the caret jumps two characters.** Found 2026-10-20 by [SP-187]'s run-render test; reproduced after a full render too, so not caused by it. ⚠️ Not yet seen in the app. → [details](#i-0313) | **Medium** | [SP-193] | ✅ **VERIFIED 2026-10-25** (user, Mac live pass 2026-10-24: step 1 *"passes"*, step 2 *"passes"*) |
| **I-0314** | `[Apple]` ⛔ **A chapter split on the Mac puts the scenes out of order ON DISK: the new chapter is appended at the manuscript END, and the scenes after the split stay in the old chapter.** The editor shows the right order until the project is reopened. Linux had the same defect and fixed it in July ([I-0074]: create in place, then move the following scenes). Found 2026-10-21 planning [SP-191] (an interop probe). → [details](#i-0314) | **High** | [SP-191] | ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21) |
| **I-0315** | `[Linux]` ⚠️ **Every move of the caret into another scene WRITES the previous scene's outline and to-do list, changed or not — two sidecar writes on the GUI thread per scene switch.** Found 2026-10-21 tracing [SP-191]'s GUI-thread core calls (`structural_baseline_smoke`). → [details](#i-0315) | Medium | [SP-191] | ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"I-0315 is verified"*) |
| **I-0319** | `[ScriviCore]` `[Apple]` `[Linux]` ⚠️ **Undoing a change made in THIS session showed "Undoing changes from a previous session", after the undo, with only an OK button.** Found by the user 2026-10-24 ([SP-188] Mac live pass, step 1). → [details](#i-0319) | **Medium** | [SP-188] | ✅ **VERIFIED 2026-10-25** (Mac and rig) |
| **I-0320** | `[ScriviCore]` `[Apple]` ⚠️ **The branch chooser (a history choice point) pops up as an UNDO lands on a fork; choosing in it redoes, which is never what a writer wants right after an undo.** Reported by the user 2026-10-24 ([SP-188] Mac live pass). → [details](#i-0320) | **Medium** | [SP-188] | ✅ **VERIFIED 2026-10-24** |

---

## I-0311

**Title:** A paste that opens a chapter mid-chapter moves the target's tail past the following scenes
**Status:** ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21). Was 🟠 Resolved - Not Verified (2026-10-21, [SP-191]).
**Platform:** `[ScriviCore]` (both platforms call it)
**Severity:** High — the writer's text comes out in the wrong order; nothing is lost.
**Sprint:** [SP-191] ([EP-053] S2b — the apps switch to `scrivi_range_paste`, which [SP-186] fixed; design v0.2 §4.4)
**Date Identified:** 2026-10-17 (reading `FragmentPaster::paste` while measuring [I-0297])

**What happens:** chapter 1 = [a1 "Alpha. Bravo.", a2 "Charlie."], chapter 2 = [b1]. Paste [head "one", chapter "two", scene
"three"] at "Alpha. |Bravo.". As one flat document the result reads: "Alpha. one" · [new chapter] "two" · "three Bravo." ·
"Charlie." · [chapter 2] … ✅ What the core writes: a1 "Alpha. one" · a2 "Charlie." · [new chapter] "two" · "threeBravo." · b1.
"Bravo." and the new chapter now come AFTER "Charlie.".
**Cause:** the paste creates each new chapter after the target's chapter (`afterChapterID = runChapterID`,
`FragmentPaster.cpp`) but leaves the target chapter's scenes after the caret where they were. A flat insert of a chapter
boundary should carry them into the last created chapter — the mirror of the cut's "promote the survivors" (design §4.4).
⚠️ `CaptureCApiTests` `[I-0297]` encodes the scrambled order as expected; it must change with the fix.


---

## I-0313

**Title:** Typing into an empty scene followed by another empty scene is credited to the next scene
**Status:** ✅ **VERIFIED 2026-10-25** (user, Mac live pass 2026-10-24: step 1 *"passes"*, step 2 *"passes"*). Was 🟠 Resolved - Not Verified (2026-10-24, [SP-193]).
**Platform:** `[Apple]`
**Component:** `ManuscriptTextView` — the edit path (`textDidChange` → `segmentIndex(for:)`) and `SceneBoundaryTable.apply`
**Severity:** Medium — the writer's text lands in the wrong scene's record; on save the wrong scene gets it
**Sprint:** [SP-193] — fix before the alpha (user, 2026-10-24)
**Date Identified:** 2026-10-20 (`RunRenderTests`, [SP-187])

**Measured (interop harness, one chapter: "Alpha one. \n\nAlpha two." · "Beta." · "" · "" · "Gamma."):** the caret at the
first empty scene's start (storage 32, its range `{32, 0}`; the next empty scene is `{34, 0}`), then
`insertText("Z")`:
- the storage reads `Beta.￼\nZ￼\n￼\nGamma.` — the "Z" IS in the first empty scene on the page;
- the boundary table after the edit: `{32, 0}`, `{34, 1}` — it grew the SECOND empty scene;
- the scene list: `["…", "Beta.", "", "\n", "Gamma."]`; the selection moved to 34, not 33.
✅ **Reproduced after a full `rebuildStorage`** (probe, 2026-10-20), so it is not [SP-187]'s run render.
**Lead, not a diagnosis:** two zero-length scenes two characters apart, and an edit whose processed range may be widened
(the note at `SceneBoundaryTable`, *"by `didProcessEditing`, `editedRange` has been WIDENED to whole lines"*). ⚠️ Prove it
by instrument before fixing. Not yet reproduced in the app; it needs two empty scenes in a row (e.g. two quick ⌘↩ at a
scene's end, then type into the first).
**Test:** `RunRenderTests.sceneOperations` types into a scene WITH text, and says why.


**Cause (shown by instrument, 2026-10-24):** the storage reported the edit exactly (`edited={32, 1}, delta=1`), but the
boundary table was already dirty: the typed "Z" carried **`scrivi.divider`**. At a line's start AppKit gives typing
the attributes of the character AFTER the caret, and before an empty scene's divider that character is the divider. The
table's update treated text bringing the divider key as structure and marked itself dirty. The rescan, which finds
dividers by that key, then took the "Z" for a divider: `{32,0}`, `{34,1}`, so the next scene got the text.
**Resolution:** the coordinator implements `textView(_:shouldChangeTypingAttributes:toAttributes:)` (`NSTextViewDelegate`,
`NS_SWIFT_UI_ACTOR` in NSTextView.h). When the new typing attributes carry `.scriviDivider`, typing uses the body's
attributes instead. Interop `typingIntoFirstOfTwoEmptyScenes`: the Z has no divider key, the list is
`["…", "Beta.", "Z", "", "Gamma."]`, the table equals a fresh scan, the caret sits after the Z, and the disk matches.
Mutation-checked (hook removed: 4 expectations fail). ✅ Linux: a `structural_baseline_smoke` block types into the first
of two empty scenes. It passes with no change: Linux never had this.
---

---

## I-0314

**Title:** A chapter split on the Mac writes the scenes out of order on disk
**Status:** ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21). Was 🟠 Resolved - Not Verified (2026-10-21, [SP-191]).
**Platform:** `[Apple]`
**Component:** `ManuscriptTextView.handleCreateChapter` (`ManuscriptTextView.swift:1412`): `engine.createChapter` is called
without `afterChapterID`, so the wrapper's default `""` applies (`ScriviEngine.swift:759`: *"Empty → append at the manuscript
end (the prior macOS behavior, preserved by the default)"*). `loader.splitChapter` (`:1484`) then moves the following scenes
into the new chapter IN MEMORY ONLY.
**Severity:** High. Nothing is lost, but after a reopen the writer's text is out of order: the tail of the split scene
moves to the end of the book, and the scenes that followed it stay in the old chapter.
**Sprint:** [SP-191] ([EP-053] S2b — chapter split through `scrivi_range_paste`; ruled Q6, 2026-10-21)
**Date Identified:** 2026-10-21 (an interop probe while planning [SP-191]; the probe was removed after it ran)

**Measured (interop harness, one chapter: "Alpha one. Alpha two." · "Beta."):** the caret after "Alpha one. ", then
`handleCreateChapter()`:
- the list in memory: `Alpha one. ` · [new chapter] `Alpha two.` · `Beta.`;
- the disk, through `openProject`: `Alpha one. ` · `Beta.` · [new chapter] `Alpha two.`.
With chapters after the split, the new chapter should also land after the LAST of them, per `scrivi_create_chapter`'s
documentation. ⚠️ Read, not measured.

**Why the tests missed it:** `RunRenderTests.chapterOperations` checks the render against the list, never the list against
the disk. SP-187's live pass (AC7) split a chapter and then merged it straight back (steps 6–7), which restores the same
order, so the reopen at step 10 could not show it.

**Linux:** not affected. [I-0074] (verified 2026-07-18) made Linux create the chapter in place (`afterChapterID = C`) and
move each following scene (`EditorShell.cpp:2098`, `:2159`).

**Fix direction:** decided with [SP-191]. That Sprint's plan already moves chapter split to one core call that returns the
delta; a split through `scrivi_range_paste` puts the new chapter in place and moves the following scenes (the [I-0311]
promotion). A test checks the list against the disk after every structural operation.

**Resolution (2026-10-21):** ⇧⌘-Return is now ONE `scrivi_range_paste` of a break whose second entry is in another chapter
(`ManuscriptTextView.handleCreateChapter` → `insertBreak`), so the core creates the chapter IN PLACE and moves the following
scenes into it, on disk as in the editor; at a scene's start it is a relabel ([SP-191] Q8). Tests: the interop
`listMatchesDisk` (a fresh open against the scene list) after every split, chapter split, merge and chapter merge, their
undos and redos (`structuralEditsUndoAndRedo`, `RunRenderTests.chapterOperations`), and the core's
`[SP-191] Q1 + [I-0314]` case (the disk's order after a chapter split).


---

## I-0315

**Title:** A scene switch rewrites the previous scene's outline and to-do list, whether or not they changed
**Status:** ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"I-0315 is verified"*). Was 🟠 Resolved - Not Verified (2026-10-21, [SP-191]).
**Platform:** `[Linux]`
**Component:** `SceneInspector::setScene` → `flushPendingEdits` (`platforms/linux/src/SceneInspector.cpp:335`, `:915`), which calls
`onOutlineEdited` and `onTodoEdited`; each writes its sidecar field unconditionally (`scrivi_set_scene_outline`,
`scrivi_set_scene_todo`, `:948` and below).
**Severity:** Medium. Each write is small, but it runs on the GUI thread on every move of the caret into another scene, so over
the share each costs a network round trip, and it rewrites the scene's sidecar with no change.
**Sprint:** [SP-191] (taken in by the user after the live passes, 2026-10-21)
**Date Identified:** 2026-10-21 (tracing [SP-191]'s remaining GUI-thread core calls in `structural_baseline_smoke`)

**Measured:** after each structural edit places the caret in another scene, the trace shows two `{sceneID, updated: true}`
envelopes (the outline and the to-do writes of the scene the Inspector was showing) and the new scene's notes read. When the
scene the Inspector was showing has just been removed (a merge, an undo of a paste), the writes fail with "Scene not found".
These are 3 of the split's and paste's 6 GUI-thread calls; the structural edit itself makes none.

**Why it matters beyond cost:** a write of an unchanged outline stamps the sidecar. It is not known to have lost anything.
⚠️ Read, not measured: whether the Mac's Inspector writes on a scene switch too.

**Fix direction (not decided):** write only what changed since the Inspector loaded the scene (compare with the loaded value),
as the tags already do through their own `editingFinished`.

**Rig evidence (build 76, 2026-10-21):** after an undo of a split, *"inspector: set scene took 45309 ms"* — the writes to the
scene the undo had just removed, each a failed lookup over the share.

**Resolution (2026-10-21, [SP-191]):** the Mac's shape (`WritingToolCards.saveOutline` guards `text != outline`; to-dos
write only on an explicit add / toggle / remove), adopted on Linux:
- `SceneInspector` keeps the outline and to-do as last loaded or written (`loadedOutline_`, `loadedTodo_`);
  `onOutlineEdited` / `onTodoEdited` write only when the field differs (`SceneInspector.cpp`, both handlers).
- `SceneInspector::forgetScene` drops a REMOVED scene without flushing; `EditorShell::applyRangeResult` calls it for every
  `removedSceneIDs` entry.
- Measured (`structural_baseline_smoke`): a split's GUI-thread core calls 6 → **2**, now guarded (`<= 2`); mutation-checked
  (each compare removed → caught).


---

## I-0319

**Title:** The previous-session warning fires for this session's change, after the undo, with no Cancel
**Status:** ✅ **VERIFIED 2026-10-25** — Mac (user, 2026-10-24: *"Both steps pass."*) and Linux on the rig, build 1.0.297 (user, 2026-10-25: *"Steps 1 - 8 passed. We can mark T-622 and I-0319 verified. I approve closing SP-193."*). Was 🟠 Resolved - Not Verified (2026-10-24).
**Platform:** `[ScriviCore]` `[Apple]` `[Linux]`
**Component:** `HistoryService::undo` (`ScriviCore/src/history/HistoryService.cpp`); Apple `performUndo`
(`Scrivi/Views/ManuscriptTextView.swift:387`); Linux `EditorShell::undo` (`platforms/linux/src/EditorShell.cpp`)
**Severity:** Medium. The warning tells the writer something false (that today's change is yesterday's), and gives no way
to back out of an undo that really would change yesterday's text.
**Sprint:** [SP-188]
**Date Identified:** 2026-10-24, user, [SP-188] Mac live pass step 1 (⌘X across scenes, then ⌘Z)

**What happened (user):** *"If I make a change, and then undo it, I should not get a dialog telling me about last sessions
changes! … no dialog should appear unless I try and undo a change made from yesterday. Then I should see the dialog. Oh,
and the dialog only offers an OK button. I have no way to Cancel yesterday's undo I am about to make today."*

**Cause:** the core flagged `crossedSessionBoundary` when an undo LANDED on an earlier session's node. Undoing this
session's first change lands on the previous session's last one, so it warned, and only after the undo had run. ✅ The
undo design v0.1 ("Sessions") already said *"about to undo"* and *"proceeds only on confirmation"*. The code didn't.

**✅ Ruling (user, 2026-10-24):** a dialog only for an undo that would undo an earlier session's change, shown BEFORE that
undo, with Cancel. *"Ownership of an issue does not absolve a work product from the need to fix it."* It was fixed in
[SP-188], which surfaced it.

**Resolution (2026-10-24):**
- **Core:** the crossing is the undo of a change from a different session than the undos are in (`undoSession_`: this
  session after a new change or an open; else the session of the last change undone or redone), never the node landed
  on. `undoEntersEarlierSession()` puts `undoCrossesSessionAt` (that change's time) in every `scrivi_capture_*` envelope
  when the next undo crosses a border; never while an edit is pending (the undo takes that edit first). `scrivi.h`
  documents it; no new endpoint.
- **Apple / Linux:** before undoing, ask the core afresh. If it reports a crossing: *"Undo a change from a previous
  session?"*, **Undo** / **Cancel**. Cancel undoes nothing. The after-the-undo notice is gone from both.
- **Tests:** core `[I-0319]` (C ABI, three sessions: asked at each border going back and never inside a session; a redo
  forward and an undo back re-asks at the border); interop
  `earlierSessionUndoIsAskedFirst` (replaces [I-0298]'s `structuralUndoAcrossSessionsWarns`: this session's paste
  undone without a question, then yesterday's asked, Cancel undoes nothing, Undo undoes it); Linux `undo_smoke` (this
  session's typing undone without a question).
- ✅ **Ruling (user, 2026-10-24, after the first fix):** *"I do not want it to ask every time; until you reach the border
  of a previous session again."* Asked at EVERY border going back (session 3 → 2, then 2 → 1, … to the project's
  start), and again on re-crossing one. ⛔ The first fix asked once per session per run, so the user's second ⌘Z of
  step 1 (into the next session) asked nothing. Replaced.

---

---

## I-0320

**Title:** The branch chooser appears after an undo instead of on redo
**Status:** ✅ **VERIFIED 2026-10-24** (user, Mac: *"Ok, I did the checks on the Mac. Both steps pass."* — the chooser check: no chooser after ⌘Z, both branches on ⇧⌘Z, Esc redoes nothing, choosing redoes it)
**Platform:** `[ScriviCore]` `[Apple]` (Linux has no branch chooser: its redo follows the primary branch, [SP-178] Q3)
**Component:** Apple `apply(step:)` / `performRedo` / `presentForkPopover` (`Scrivi/Views/ManuscriptTextView.swift`);
the capture envelope (`ScriviCore/src/public_api/scrivi_c_api.cpp`, `withCapture`)
**Severity:** Medium. The popover obstructs the writer after an undo, and its only action (choosing) re-applies text the
writer just undid or undid long ago.
**Sprint:** [SP-188]
**Date Identified:** 2026-10-24, user, during [SP-188]'s Mac live pass

**What happened (user):** *"Let's say I just typed some text, and then I type undo (Cmd-Z) and the choice point appears.
Now if I actually choose something it reapplies my typing, or it reapplies the typing I undid a while ago. In neither
case do I want to do that after an undo! In most cases after an undo what I want to do is either start typing, or undo
again. The historical choice point should not appear until I choose redo (Shift-Cmd-Z). Then I will want to know which
branch to redo into."*

**Cause:** `apply(step:)` showed the popover whenever a step's `forkAhead` was set, which includes every undo that lands
on a fork.

**✅ Ruling (user, 2026-10-24):** the choice point appears on ⇧⌘Z only.

**Resolution (2026-10-24):**
- **Core:** every `scrivi_capture_*` envelope carries `redoFork` (`{nodeID, children}`, as `forkAhead`) when the redo from
  the current node has two or more branches (`HistoryService::redoFork`); never while an edit is pending. `scrivi.h`
  documents it; no new endpoint. `forkAhead` on a step result is unchanged; the app no longer acts on it.
- **Apple:** ⇧⌘Z asks the core afresh. At a fork it shows the chooser instead of redoing: choosing a branch re-primaries
  it and redoes, and dismissing it redoes nothing. Undo never shows it, and any step closes an open one.
- **Tests:** core `[I-0320]` (no `redoFork` after a single-branch undo or at the tip; two children at a real fork);
  interop `branchChooserOnRedoOnly` (no chooser as the undo lands; ⇧⌘Z offers both branches; dismissed = nothing
  redone; choosing the older branch redoes it). The interop harness now fails on an unexpected chooser, as it does on an
  unexpected barrier notice. `structuralEditsUndoAndRedo` builds a fork with each op and now answers the chooser with
  the newest branch.
- Follow-ups: a blank choice in the chooser → [I-0321]; the "will ⇧⌘Z ask here" marker → [T-0620]; the whole-history view → [T-0621].
- ⏳ **Not decided:** a visible marker in the manuscript where a branch point is (user, 2026-10-24: *"I might be able to
  be persuaded … we should explore the concept a bit"*). Discussion only.

---

*Last Updated: 2026-10-24 — **I-0320** archived (user-verified, Mac).*

*Last Updated: 2026-10-22 — **I-0315** archived (user-verified).*

*Last Updated: 2026-10-22 — **I-0311** and **I-0314** archived (user-verified, [SP-191]'s close).*
