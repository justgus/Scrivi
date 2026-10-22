# Verified Issues: I-0311 – I-0320

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0301–I-0310 are in [`Issue-verified-0301-0310.md`](Issue-verified-0301-0310.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0311** | `[ScriviCore]` ⛔ **A structured paste that opens a CHAPTER in the middle of a chapter scrambles the reading order: the target scene's remaining text lands AFTER the scenes that followed it.** Found 2026-10-17 by reading the core while measuring [I-0297]; ⚠️ the new `[I-0297]` ABI test asserts the wrong order (expectations taken from the code). → [details](#i-0311) | **High** | [SP-191] | ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21) |
| **I-0314** | `[Apple]` ⛔ **A chapter split on the Mac puts the scenes out of order ON DISK: the new chapter is appended at the manuscript END, and the scenes after the split stay in the old chapter.** The editor shows the right order until the project is reopened. Linux had the same defect and fixed it in July ([I-0074]: create in place, then move the following scenes). Found 2026-10-21 planning [SP-191] (an interop probe). → [details](#i-0314) | **High** | [SP-191] | ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21) |
| **I-0315** | `[Linux]` ⚠️ **Every move of the caret into another scene WRITES the previous scene's outline and to-do list, changed or not — two sidecar writes on the GUI thread per scene switch.** Found 2026-10-21 tracing [SP-191]'s GUI-thread core calls (`structural_baseline_smoke`). → [details](#i-0315) | Medium | [SP-191] | ✅ **VERIFIED 2026-10-22** (user, 2026-10-22: *"I-0315 is verified"*) |

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

*Last Updated: 2026-10-22 — **I-0315** archived (user-verified).*

*Last Updated: 2026-10-22 — **I-0311** and **I-0314** archived (user-verified, [SP-191]'s close).*
