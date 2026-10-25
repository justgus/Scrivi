---
sprint: SP-188
epic: EP-053
status: Active
platform: Cross
created: 2026-10-20
---

# SP-188 — `[Apple]` + `[Linux]` [EP-053] **S3**: structural cut, Backspace / Delete over scenes, and their undo

**Status:** 🟡 **ACTIVE** 2026-10-22 (user: *"activate and implement SP-188"*) — created 2026-10-20 (user: *"complete planning for EP-053"*); code re-read 2026-10-22 after
[SP-191]. ✅ Q1–Q6 ruled 2026-10-22 (user: *"Q1–Q6 agreed as recommended"*).
**Carries:** [T-0616] (🟠 Implemented - Not Verified) → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md). · [I-0319] (🟠 Resolved - Not Verified), [I-0320] and [I-0321] (✅ Verified), added 2026-10-24 → [`../Issues/Issue-active.md`](../Issues/Issue-active.md).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1, `scrivi_range_cut`) · [SP-187] + [SP-191] (S2a + S2b: the scene list as the model of record, the delta applied,
`rangePaste` / `rangeCut` history, the one off-main path).
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_4.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_4.md) (v0.4, 2026-10-22, this Sprint's
Q1–Q6 in §1c; supersedes v0.3) — R4, §4.3, §4.5, Q2, Q4. ⛔ R4 **reverses** the 2026-10-02 ruling ([I-0270]: a cross-scene cut deletes text and keeps scenes).
**Ruling that shaped it (user, 2026-10-20):** ✅ each operation ships with its undo; ✅ the `rangeCut` history op name is
approved, backward-compatible.
**Size:** M (was L: [SP-191] built the `rangeCut` node, its undo and redo, and the off-main path — see below).

---

## What reading the code found (re-read 2026-10-22, after [SP-191])

**Still the switch point — every cross-scene cut and delete funnels through ONE function per platform:**
- **Apple** `deleteAcrossScenes` (`ManuscriptTextView.swift:2361`), reached from ⌘X (`cut`, `:3034`), ⌥N (`cutIntoBuffer`,
  `:1059`), ⌫ (`deleteBackward`, `:3188`), ⌦ (`deleteForward`, `:3257`) — and ⚠️ **one caller the 2026-10-20 reading missed:**
  a structured paste over a selection ([I-0295], `pasteStructuredRange`, `:2441`) removes the selection through it first.
- **Linux** `ManuscriptEditor::deleteAcrossScenes` (`ManuscriptEditor.cpp:1185`), reached from the key handler (`:498`),
  `cutSelection` (`:1093`), `cutSelectionIntoBuffer` (`:1133`) and — the same missed caller — `removeSelectionForPaste`
  (`:969`, from `EditorShell.cpp:2418`).

**Already built by [SP-191] — this Sprint calls it, it does not build it:**
- The cut call with its history node: Apple `cut(_:label:)` (`:1482`), Linux `EditorShell::cutRange` (`EditorShell.cpp:2223`),
  each a `rangeCut` through the one off-main path (`runStructuralEdit` `:1293` / `runRangeEdit` `EditorShell.cpp:1892`): the
  cue in the key press's turn, read-only, the progress row after 400 ms, the delta applied, no reload. Today only the merge
  (⌘⌫ / Ctrl+Backspace) uses it.
- The `rangeCut` undo (a paste at `undoCaretSceneID` / `undoCaretByte`, sidecars restored) and redo (the cut again):
  Apple `applyStructuralInverse` (`:610`), Linux (`EditorShell.cpp:2541`). Old `structuredCut` nodes convert to a Range
  (Apple `:647`, Linux the same function). So Plan items 3 and 4 of the original plan are **done**; they only need tests
  against a cut that is not a merge.
- The test harnesses: Apple's `StructuralCallSink` (main-thread call count) and `listMatchesDisk`; Linux's
  `structural_baseline_smoke` (`listMatchesDisk`, no load, no document build).

**Found, and raised as questions:**
- ⚠️ **The core checks the cut's text against the disk** (`RangeOperations.cpp:829`: `body.compare(e.start, e.length,
  e.text)`), but the copy Range on both platforms is **balanced** (Apple `copiedRange` `:2260`, Linux `crossSceneCopy`
  `ManuscriptEditor.cpp:886`): a selection that starts or ends inside bold carries markers the disk does not have. So the
  copy's Range **cannot** be the cut's Range — the core would refuse it. → **Q3**, **Q4**.
- ⚠️ **Whole scenes leave an empty scene.** The core removes the scenes *between* the ends; the surviving end keeps its
  identity (Q7). A selection of scenes 2–3's whole text leaves an EMPTY scene 3. Both platforms add a scene's start to the
  Range only when the selection reaches **past** it (Apple `:2270`, Linux `:909`), so a selection ending at the divider
  never takes the next scene. → **Q5**.
- ⚠️ **The [I-0295] caller** (paste over a cross-scene selection) becomes two structural calls in sequence, and the one
  path ignores a second call while the first is in flight (Q5 of [SP-191]). → **Q6**.
- ✅ A selection of exactly one divider is still refused on both (Apple's divider guard `:2563`; Linux `crossSceneCuts`
  empty → no-op) — Q2 is unchanged by [SP-191].
- ✅ [I-0270]'s grouped-delete history nodes are ordinary edit groups (`recordGroupedEdit`) and undo as text — untouched
  by this Sprint. Replace All also records one (`:2288`) and is not a cut: it stays.

## Goal

✅ A cut or a Backspace / Delete across scene or chapter boundaries **removes the structure** on both platforms: the start
scene's head joins the end scene's tail directly, the scenes between and the end scene go, emptied chapters go, and the end
chapter's later scenes join the start scene's chapter (Q2). One `scrivi_range_cut` off the main thread, the delta applied,
no reload. Undo is a range paste; redo is the cut again.

## Plan

0. **Red tests first:** a cross-scene cut on each platform must leave the scene count lower and make zero core calls of its
   own on the main thread. Both fail against `deleteAcrossScenes` today.
1. **Core — the balanced join (Q3).** `RangeOperations::cut` balances the survivor's join with the core's markdown balancer
   and returns the markers it added (a new optional field on the cut result, `scrivi.range` unchanged; no new endpoint, ABI
   count unchanged). The `rangeCut` undo removes them before pasting the Range back; the redo cuts again and re-adds them.
   An old node without the field undoes as today. Boundary-tested through `scrivi_*`; budget unchanged (the join is in
   memory).
2. **The cut's Range** is built from the scene list with each piece's **stored** text (not the balanced copy) — one
   builder per platform beside `copiedRange` / `crossSceneCopy`:
   - **Q5:** a selection that crosses a scene's divider reaches that scene at its start, so whole scenes go and none is
     left empty. The same rule in `copiedRange` / `crossSceneCopy`, so a copy of whole scenes pastes as whole scenes.
   - **Q1:** a chapter title in the selection is skipped — the Range starts (or ends) at the first (last) scene body the
     selection reaches — unless the selection covers the whole chapter, when the chapter's scenes are all in the Range
     (and the core removes the emptied chapter).
3. **Cut** (⌘X / Ctrl+X): other apps get the presented text at the key press (unchanged); then `cut(_:label:)` /
   `cutRange`. **Q4:** when the cut lands, Scrivi's clipboard takes the cut's **returned** Range (sidecars included) with
   its edge pieces balanced. **The buffer cut** (⌥N / Alt+N): the same, that Range in the slot.
4. **Backspace / Delete over scenes = Cut minus the clipboard** (design Q4): the same call; the clipboard and buffers are
   left alone. **Q2:** a selection of exactly one scene divider is the scene merge (⌘⌫ / Ctrl+Backspace's zero-length
   `rangeCut`), no longer refused.
5. **Paste over a cross-scene selection (Q6, [I-0295]):** the `rangeCut` of the selection, then — when it lands, inside
   the same read-only period, one cue, one progress row — the paste (a structured `rangePaste`, or the flat text inserted).
   Two nodes, two undo steps.
6. **Retire** `deleteAcrossScenes` on both, with all four callers moved. A selection inside one scene keeps the ordinary
   path. ⚠️ On Linux the call moves from the editor to the shell: `ManuscriptEditor` emits, `EditorShell` runs it (as
   `bufferCutRequested` already does).
7. **The design doc moves to v0.4** with Q1–Q6 (§4.3's balanced join and whole-scene reach; the clipboard after a cut;
   Backspace over a divider; paste over a selection).
8. **Tests** on both: cut, then undo, then redo — the disk and the list equal after each (`listMatchesDisk`); a cut that
   empties a chapter removes it, and its undo restores the chapter's metadata; Q2's promotion; a cut inside bold restores
   exactly (Q3); Backspace leaves the clipboard unchanged; the buffer cut fills the slot; a cut then a paste elsewhere keeps
   the scenes' sidecars (Q4); whole scenes (Q5); a chapter title in the selection (Q1); Backspace over one divider merges
   (Q2); paste over a selection, and its two undos (Q6); an old `structuredCut` node and an old [I-0270] grouped delete
   still undo; zero main-thread core calls. Mutation-checked.
9. **Live passes (user):** Mac and rig, over the share.

## Questions — ✅ all ruled 2026-10-22 (as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **A selection that starts or ends on a chapter title** (Apple draws it; Linux draws it when titles are on, [SP-184]). Does the cut take the chapter? | ✅ **Only when the selection covers the whole chapter.** Otherwise the range starts at the first scene body the selection reaches. A title is presentation (it is not in any scene's text). |
| **Q2** | **Backspace over a selection of exactly one scene divider** (no text either side): merge the scenes, or refuse? | ✅ **Merge**, the same as ⌘⌫ / Ctrl+Backspace's scene merge, recorded as a `rangeCut` of an empty range. Today both platforms refuse to remove a divider by an ordinary edit ([I-0270]). |
| **Q3** | ⚠️ **A cut that starts or ends inside bold** (or any emphasis span). The core removes exactly the stored text and joins head to tail, so `**Alpha be|ta.**` cut to `|Gamma.` leaves `**Alpha beGamma.` — unbalanced. Today's [I-0270] path balances each scene's removal (T-0591), but that is not a removal of exactly the Range, so its undo would not be exact. | ✅ **The core balances the join, and the result records the markers it added** — the undo takes them away before pasting the Range back, so cut ∘ undo is exact. The markdown balancer is already in the core (`scrivi_markdown_*`, stateless). ⚠️ The alternative — the editor widens the selection to the span's edge — changes what the writer selected. |
| **Q4** | **What Scrivi's clipboard (and a buffer) holds after a cut.** The copy's Range has balanced texts but no sidecars; the cut's **returned** Range has the sidecars (Q3b) but raw edge pieces. | ✅ **The returned Range, with its edge pieces balanced** (only the texts change). Then a cut-then-paste elsewhere moves the scenes WITH their tags, story time, band, outline, to-dos and status ([SP-191]'s noted consequence) — what a writer means by *moving* scenes. Other apps still get the presented text. ⚠️ This means the clipboard is filled when the cut **lands**, not at the key press. |
| **Q5** | **A selection of whole scenes.** Selecting all of scenes 2 and 3 and cutting leaves an EMPTY scene 3 (the surviving end keeps its identity). | ✅ **A selection that crosses a scene's divider reaches that scene, at its start** — so selecting through to the next divider takes scenes 2 and 3 whole and leaves none empty. The same rule for the copy, so a copy of whole scenes pastes as whole scenes. A selection that stops at the end of scene 3's text (before its divider) keeps today's result. |
| **Q6** | **Paste over a cross-scene selection** ([I-0295]: *the selection is removed first, as its own undo step*). That becomes a `rangeCut`, then the paste — two structural calls in a row. | ✅ **Chained: the paste starts when the cut lands**, inside the same read-only period (one cue, one progress row), and records its own node — two undo steps, as [I-0295] ruled. A flat paste over a cross-scene selection the same, its text inserted when the cut lands. |

⛔ **NOT in this Sprint:** the background sync verification ([SP-189]).

## Acceptance Criteria

- [x] AC1 — Cut across scenes and chapters removes the structure (§4.3, Q2 of the design), one `scrivi_range_cut` off the
      main thread, delta applied, no reload, on both
- [x] AC2 — **Core (Q3):** a cut that starts or ends inside an emphasis span leaves the survivor balanced; its undo
      restores the disk exactly, its redo re-applies the same result; an old node without the markers field still undoes.
      Boundary-tested through `scrivi_*`; the cut's disk budget unchanged
- [x] AC3 — Backspace / Delete over scenes: the same cut, with the clipboard and buffers untouched (design Q4); a selection
      of exactly one scene divider merges the two scenes (Q2)
- [x] AC4 — After a cut, Scrivi's clipboard holds the cut's returned Range (sidecars included, edges balanced): a paste
      elsewhere brings the scenes back with their tags, story time, band, outline, to-dos and status (Q4). The buffer cut
      (⌥N / Alt+N) puts the same Range in the slot
- [x] AC5 — A selection crossing a scene's divider takes that scene whole — no empty scene is left — for cut and copy alike
      (Q5); a chapter title is skipped unless the whole chapter is selected, which removes the chapter (Q1)
- [x] AC6 — Paste over a cross-scene selection: the cut, then the paste when it lands, in one read-only period; two undo
      steps (Q6, [I-0295])
- [x] AC7 — Undo = range paste, redo = cut, `rangeCut` node; cut∘undo restores disk and memory exactly; old `structuredCut`
      nodes and [I-0270] grouped deletes still undo on both
- [x] AC8 — `deleteAcrossScenes` retired on both (all four callers moved)
- [x] AC9 — Tests on both, mutation-checked; ctest (Mac and Linux), interop and Linux smokes green; zero main-thread core
      calls of the cut's own
- [x] AC10 — The design doc is at v0.4 with Q1–Q6
- [ ] AC11 — ✅ **Live passes (user):** Mac and rig, over the share, each operation timed

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-187], [SP-189], [SP-190] via `next-id.py`; [T-0616] filed.
- Q1–Q2 await rulings.

### 🔵 2026-10-21 — Affected by [SP-191]'s Q3 and Q4
- [SP-191] Q3 (ruled): split and merge become undoable range operations, with no barriers. Its Q3b (✅ agreed 2026-10-21)
  puts each whole scene's sidecar (and a removed chapter's metadata) in the Range, built in SP-191's Plan 1. This Sprint's
  cut undo uses it, so an undone cut restores the tags, story time, band, outline, to-dos and status. ⚠️ A consequence: a
  cut-then-paste elsewhere keeps the scenes' IDs (`pickSceneID`), so the scenes move WITH their sidecars.
- [SP-191] Q3e (ruled): the manuscript is one Range, and a cut of any sub-range joins the scenes at its ends, including a
  zero-length one at a chapter break. This Sprint's cut follows the same rule (design §4.3). Chapter merge is a relabel.
- [SP-191] Q4/Q4a (ruled): only the clipboard and copy buffers drop old-style objects (they read as empty) and store Ranges.
  History keeps [SP-187] Q2's conversion, so this Sprint's "old `structuredCut` nodes still undo" (Plan 4, AC5) stands.

### 🔵 2026-10-22 — The code re-read after [SP-191]; Plan revised; Q3–Q6 raised
- [SP-191] closed; this Sprint no longer waits on it. Line numbers re-checked on both platforms.
- [SP-191] already built the `rangeCut` call, node, undo, redo and the old-node conversion; the original Plan 3–4 are done
  and the Plan is renumbered. Size L → M.
- Found: the core refuses a balanced copy Range as a cut (Q3, Q4); whole scenes leave an empty scene (Q5); a fourth caller
  of `deleteAcrossScenes`, paste over a selection (Q6).
- ⏳ Q1–Q6 for ruling. The ACs are revised once they are ruled.

### 🔵 2026-10-22 — Q1–Q6 ruled; Plan and ACs rewritten
- User: *"Q1–Q6 agreed as recommended."* Recorded under Questions.
- Q3 adds a core change (Plan 1, AC2): the cut balances the join and returns the markers it added, so its undo is exact.
- The Plan is items 0–9; the ACs are AC1–AC11. ⚠️ Not ruled and not in scope: a selection of exactly a CHAPTER break
  (Q2 covers a scene divider) — it stays refused, as today.
- ⏳ Ready for activation on the user's approval.

### 🟡 2026-10-22 — ACTIVATED
- User: *"activate and implement SP-188"*. [T-0616] → active.

### 🟡 2026-10-22 — IMPLEMENTED (AC1–AC10 met); ⏳ AC11 live passes
- **Core (Q3):** `RangeJoin` on the Range (`ScriviCore/src/manuscript/Range.hpp`), an optional top-level `"join"` in its JSON.
  `RangeOperations.cpp` `balanceJoin()` balances the survivor with `markdown::editing::balancedEdit` over head + removed
  pieces + tail; `cut()` records the join on the returned Range (never keeping one it was given); `paste()` puts the plain
  join back first, refused anywhere but at its survivor as the cut left it. Budget unchanged (`RangeOperationBudgetTests`
  5 reads / 5 writes, green). `scrivi.h` documents the field; no new endpoint.
- **Mac:** `ManuscriptTextView.swift` `selectionRun` (Q5 / Q2 / Q1) and `cutAcrossScenes(_:keeping:then:)` — ⌘X, ⌥N,
  ⌫ / ⌦ and both pastes over a cross-scene selection (Q6, chained by `then:`). The clipboard gets the presented text at the
  key press and the cut's Range (join dropped, the copy's balanced pieces) when it lands, only if nothing was copied since.
  `deleteAcrossScenes` retired. `ScriviRange.join` in `ScriviEngine.swift`.
- **Linux:** the same shape — `ManuscriptEditor::selectionRun` / `crossSceneCutRange` / `crossSceneCutRequested`,
  `EditorShell::cutAcrossScenes(keeping, bufferID, then)`; `deleteAcrossScenes` and the grouped-edit signals of the
  editor retired (Replace All keeps `begin/endGroupedEdit`). ⚠️ `SceneDocument::crossSceneCuts` is now unused by the app
  (still covered by `editor_map_smoke`); left in place.
- **Design doc:** v0.4 (§1c Q1–Q6, §4.2b, §4.3, §4.5, §4.6).
- **Tests:** core 4 `[sp188]` cases; interop `StructuralCutTests` (9) + 2 revised; Linux `structural_baseline_smoke`
  SP-188 block (+ a bold project for Q3 / Q4) and 5 smokes revised. Measured cut on Linux: ≤ 2 GUI-thread core calls, both
  the Inspector's `scrivi_get_scene_notes` (traced) — none the cut's.
- **Counts (final tree):** ctest Mac 763/763, Linux 767/767; interop 311/311; Linux smokes 38/38.
- **Mutations (in a copy of the repo):** core 6/6, Mac 8/8, Linux 7/7 caught. Linux first showed a survivor (the clipboard
  keeping the stored pieces): the Q4 check pasted at the survivor, where stored and balanced pieces read alike. ✅ Fixed —
  the check is now exact (`**ta.**`, not `ta.**`) and catches it.
- **Harness fixes found by the mutations:** a failing test could HANG instead of fail — a modal notice blocked the host.
  Mac: the interop harness records an unexpected barrier notice as an issue; Linux: the bold Rig arms the confirmer.
- **Flakes fixed:** `FragmentPasteTests` and `ReorderTests` `TempDir` named by the clock alone collided across ctest's
  parallel processes (each failed once on Linux); now pid + counter + clock. ⚠️ 36 other test files use the same naming.
- [T-0616] → 🟠 Implemented - Not Verified.


### 🟡 2026-10-23 — Rig live pass: ✅ all 9 steps (user); ⏳ Mac
- User: *"I did the rig first while I still have it. All 9 steps passed."* Build 79, on the share, against the
  `dumas-prose-timelines` fixture (1,180 scenes) rather than a scratch project.
- Timings from the rig log (`~/.local/state/scrivi/scrivi.log`): each step's cut / undo / redo landed in 0.3–0.6 s
  (worker), applied in 50–320 ms. The last step (Q6) pasted over most of the document: the cut took 49 s, all of it in
  the range call, and the paste 131 s (range call 105 s, history node 26 s). Both ran off the main thread, and the window
  answered throughout.
- ⚠️ Not this Sprint's code: while the user drag-selected toward the top / bottom (autoscroll), the window stopped
  answering for up to 25.6 s (14.5 s, 8.1 s, 6.9 s ...). Every autoscroll step that enters a new scene runs
  `EditorShell::promoteActiveScene` → `SceneInspector::setScene` synchronously (50–290 ms each, back to back), so the
  event loop never idles. ⏳ To be filed as an Issue.
- ⚠️ One `QTextCursor::setPosition: Position '1846043' out of range` warning, right after the 49 s cut was applied: the
  caret was restored to a position past the end of the shortened document. ⏳ To be filed with it.

### 🟡 2026-10-24 — Mac live pass: 8 of 9 (user); step 6 fixed, ⏳ re-run
- User: steps 2, 3, 4, 5, 7 and 8 passed. Step 9: the same large cut and paste as on the rig, then undone, *"without
  serious delay or lag"*, and no freeze during the drag-selection. ⚠️ On a LOCAL disk, not over the share (AC11 says the
  share).
- ❌ **Step 6:** with only the divider selected, ⌫ did nothing (nor ⌘⌫; ⌘⌫ with the caret at a scene's start still merges).
  Cause: `selectionRun` (`ManuscriptTextView.swift:2168`) took the divider only when the selection also covered the `\n`
  after it, which is what the Q2 test selected. A click or drag over the divider selects the divider alone. ✅ Fixed: a
  selection that reaches past the scene's end (the divider) takes both scenes. New interop test
  `backspaceOverTheDividerAlone`; `StructuralCutTests` 10/10. Linux unchanged (its separator is one block; step 6 passed on
  the rig). ⏳ Step 6 to re-run on the Mac.
- ⚠️ **Step 1:** the first ⌘Z after the ⌘X showed *"Undoing changes from a previous session"* (after the undo, OK only).
  Not this Sprint's code, but surfaced by it. ✅ User ruling: a question only for an undo of an earlier session's change,
  BEFORE it, with Cancel. *"Ownership of an issue does not absolve a work product from the need to fix it."* → [I-0319],
  assigned here, fixed in the core and on both platforms. ⏳ Step 1 to re-run on the Mac.
- [I-0318]: the user may have caused it by pasting at the document's end after the cut. A smoke for that added
  (`structural_baseline_smoke`).
- Xcode warning fixed: `ViewportSceneLoader.swift:585` (a `[weak self]` inside a closure that held `self` strongly); the outer
  closure now captures `self` weakly. Clean build, no warnings.
- [I-0317] (autoscroll freeze) and [I-0318] (out-of-range caret) filed `[Linux]` from the rig log (user).

### 🟡 2026-10-24 — Mac re-run: steps 1 and 6 pass (user); [I-0319] revised, [I-0320] added
- User: step 1 passes as described, ⚠️ but the next ⌘Z (into the previous session) asked nothing. The first fix asked
  once per session per run. ✅ Ruling: ask at EVERY session border an undo crosses, going back, and again on re-crossing
  one. Core `undoSession_` replaces `warnedSessions_`. Step 6 passes.
- User: the branch chooser must appear on ⇧⌘Z, never after ⌘Z → [I-0320], assigned here and fixed (core `redoFork`,
  Apple `performRedo`). A visible branch-point marker in the manuscript is open for discussion; nothing decided.
- Counts: ctest Mac 765/765; interop 313/313.

### 🟡 2026-10-24 — Mac checks pass (user); [I-0320] verified; [I-0321] added
- User: *"I did the checks on the Mac. Both steps pass."* [I-0320] ✅ Verified (Mac only, as it is Mac-only).
  [I-0319]: the Mac passed; ⏳ the Linux half waits for the rig.
- User: a choice with no text in the chooser → [I-0321]. The project's history log showed a branch that only typed two
  spaces. The core now folds a branch with no text into its siblings and shows each branch's first text. Core
  `[I-0321]`; ctest 766/766; interop 313/313.
- Not this Sprint: the branch-point marker → [T-0620]; the whole-history view → [T-0621] (both 🔵 backlog).

### 🟡 2026-10-24 — [I-0321] verified (user)
- User: *"I-0321 passes, mark it verified"* → archived (`Issues/Verified/Issue-verified-0321-0330.md`). Open here:
  [I-0319]'s Linux half and AC11 (rig).
