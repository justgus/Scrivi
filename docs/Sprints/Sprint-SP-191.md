---
sprint: SP-191
epic: EP-053
status: Active
platform: Cross
created: 2026-10-20
---

# SP-191 — `[Apple]` + `[Linux]` [EP-053] **S2b**: copy, paste and its undo, split and merge — as deltas, off the main thread, no reload

**Status:** 🟡 **ACTIVE 2026-10-21** (user: *"activate SP-191"*) — ✅ all questions ruled 2026-10-21 (Q1–Q6, Q3a–Q3e, Q4a) · created 2026-10-20 by [SP-187]'s Q1 ruling (user: split S2). ✅ Its questions were ruled in
[SP-187] (Q2: old history nodes convert; Q3: `ViewportSceneLoader` applies the delta).
**Carries:** [I-0314] (Q6) · [T-0619] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md) · [SP-185] AC4 (user, 2026-10-20: *"AC4 goes
to EP-053 S2"*) · [I-0297]'s reload half · [I-0311] (the apps reach `scrivi_range_paste`).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1, the endpoints) · [SP-187] (S2a: the list as the model of record, the bindings, the delta types,
the partial re-render, the baseline counts).
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) — R1,
R3, R5, R7, P3, P5, §4.2b, §4.4, §4.5, Q5.
**Size:** XL (core changes in Plan 1–2, then both apps).

---

## What reading the code found (re-read 2026-10-21, after [SP-187])

**Apple** (`ManuscriptTextView.swift`)
- **Paste** (`:2774`): `fragmentPaste` on the **main thread**, then `reloadManuscriptFromDisk` (`:2485`), which re-opens
  the project and re-renders everything.
- **Paste's undo and redo** (`applyStructuralInverse`, `:615`): `fragmentPaste` or `fragmentUncutPaste` on the main thread,
  then the same reload (`:639`, `:656`).
- **Copy** (`structuredCopyIfCrossBoundary`, `:2549`) and **copy into a buffer** (`copyIntoBuffer`, `:977`):
  `fragmentExtract` reads the scenes from disk.
- **Split** (`:1253`): `createScene` plus two saves, **merge** (`:1572`) and **chapter merge** (`:1665`): each on the main
  actor, already applied in memory (no reload) and run-rendered since [SP-187].
- ⛔ **Chapter split** (`:1412`): `createChapter` WITHOUT `afterChapterID`, so the chapter is appended at the manuscript END
  on disk, and the following scenes move only in memory (`loader.splitChapter`, `:1484`) → **[I-0314]**, filed today,
  proven by an interop probe (memory and disk disagree right after the split).

**Linux** (`EditorShell.cpp`)
- **Paste** (`:2545`): `fragmentPaste` on a worker (`runStructural`, `:2920`), then `reloadThenPlaceCaret` → `reloadThen`
  (`:2980`): a full `load()`, ~1,700 file calls over the share even with SP-185's `reloadTexts_`.
- **Paste's undo and redo** (`applyStructuralInverse`, `:2574`): the same, after `saveDirtyScenes()` on the GUI thread.
- **Split** (`onCreateSceneRequested`, `:1940`): `createScene` and two saves on the GUI thread, then a reload (`:2011`).
- **Chapter split** (`onCreateChapterRequested`, `:2039`): `createChapter(afterChapterID = C)` (`:2098`), then one
  `reorderScene` PER following scene (`:2159`) and two saves, all on the GUI thread, then a reload (`:2180`).
- **Merge** (`:2193`) and **chapter merge** (`:2273`): on the GUI thread, then a reload (`:2241`, `:2322`).

**ScriviCore**
- `scrivi_range_paste` already does what a split needs. A Range of two entries with empty text, pasted at the caret, splits
  the caret scene there. If the two entries' `chapterID`s differ, it also creates the chapter in place and moves the caret
  chapter's following scenes into it ([I-0311]'s promotion): exactly a chapter split, in one call, under S1's budget
  guards, with the delta.
- A merge is NOT a range cut: a cut joins the two texts directly, while a merge joins them with a blank line (the
  `SceneMerger.joinBodies` ruling). A chapter merge keeps every scene. So the merges keep their own endpoints. Their results
  today are IDs and paths, not the delta.
- `Range::fromFragment` / `toFragment` exist (`Range.hpp:73`) but are not on the C ABI. Today's copy buffers
  (`BufferStore`, `scrivi.buffers.v1`) and old history nodes hold `scrivi.fragment.v1`.
- `scrivi_range_paste` refuses a `caretSceneText` that is not what is on disk, so a dirty caret scene is saved first.

## Questions — ✅ all ruled 2026-10-21

User, 2026-10-21: *"1: as recommended. 2: as recommended. 3: we need to start preparing for a design change here. Currently
if I ctrl-backspace from the start of a scene or Ctrl-Shift-Backspace from the start of a chapter I will merge the scene or
chapter with the previous one. In fact, merge chapter potentially pulls all of the subsequent chapter's scenes into the
merged one. However, I can easily select text around a chapter or scene break, cut, copy, and/or paste it somewhere else
which will create new split or merged Chapters and/or Scenes, and I can undo those in one go. which undoes the structure
changes cleanly. Therefore there is now no reason for the restriction that prevent us from "undoing past the split or merge
of a scene". So, while I believe this must be a range operation, I do not believe that we should keep the "Can't undo past:
stops. 4: No, this is not necessary because this app is not in release yet. The only older readers are the onse on the rig
or this mac. So, we should detect the older structures and silently remove them when necessary. 5: as recommended, as long
as the initial operation response is nearly instantaneous. 6: yes."*

| # | Question | Ruling |
| - | - | - |
| **Q1** | Split and chapter split | ✅ **Through `rangePaste`**, with a two-entry Range of empty text; an entry with an empty `sceneID` / `chapterID` means "mint a fresh one". |
| **Q2** | Merge and chapter merge: off the main thread, returning the delta | ✅ **As recommended** (off the main thread, the delta, count guards). ⚠️ Q3 adds that a merge must be a RANGE OPERATION, so whether the two merge endpoints survive is Q3c. |
| **Q3** | History for split and merge | ✅ **No "Can't undo past…" stops.** Split, merge, chapter split and chapter merge each undo and redo in one step, like a cut or a paste across a break. Each must be a range operation. ⏳ Q3a–Q3d follow. |
| **Q4** | Clipboard and copy buffers | ✅ **The Range, no compatibility layer for the clipboard and copy buffers** (not released; the only older data is on the rig and this Mac). Clarified by Q4a: "removed" applies to the clipboard and buffers only. **History keeps [SP-187] Q2** (old nodes convert to the delta path). |
| **Q5** | While a structural call is in flight | ✅ **As recommended:** an immediate cue, then read-only; repeated gestures, typing and undo are ignored, not queued; the progress row after 400 ms. ✅ **Condition:** the first response is nearly instantaneous, so the cue lands in the same event-loop turn as the key press, before any core call. |
| **Q6** | [I-0314] into this Sprint | ✅ **Yes**, fixed by Q1's route, with a list-against-disk check after every structural operation. |

### Follow-ups from Q3 and Q4 — ✅ all ruled 2026-10-21

User, 2026-10-21: *"Q3c: agreed. Q3c: no change. Q4a: I was applying "Removed" only to clipboard and copy buffers. I wan't
asking you to remove the whole compatibility layer. Only that, if you found an old style object in either the Copy buffers
or the clipboard, remove it (i.e. pretend the buffer is empty). Clipboard and Copy Buffers should store ranges just like
history does."* The second "Q3c" is read as **Q3d** (its recommendation was "no change"). Then: *"Q3a: agreed. Q3b: agreed as well."*

| # | Question | Recommendation / Ruling |
| - | - | - |
| **Q3a** | **The blank line a merge inserts.** Today a scene merge joins with a blank line (`SceneMerger.joinBodies`, elided when either side is empty). A range cut over the break joins directly, which is what selecting across the break and pressing Backspace gives. | ✅ **AGREED. Drop the blank line:** a merge becomes exactly a cut of the break, the same result as the selection route, and its undo (a paste of the break) is exact. If you keep it, the merge's history step must also record the inserted blank line. |
| **Q3b** | ⚠️ **What an undo restores.** A Range entry carries a scene's ID, chapter, titles and text, but not the rest of its sidecar: tags, story time, timeline band, outline, to-dos, status. A merge (and [SP-188]'s cut) deletes the removed scene's sidecar, so undoing it would bring the scene back with its text but without those. | ✅ **AGREED. The Range carries each whole scene's sidecar** (and the chapter's metadata, where a chapter is removed). A paste of a copy ignores it (the new scene starts fresh, as today); the undo of a removal restores it exactly. This also decides [SP-188]'s cut undo. |
| **Q3c** | ⚠️ **Superseded by Q3e.** **A chapter merge as a range operation** removes the chapter break but keeps the scene break, while a cut over the break removes both. | ✅ **AGREED.** **The core gains a break-only form of the range cut and the range paste:** a two-entry Range of zero length at the break says "remove (or insert) the break between these two scenes", and the scene IDs on either side stay. Merge, chapter merge and their undos then all use `rangeCut` / `rangePaste`, and `scrivi_merge_scene` / `scrivi_merge_chapter` retire. |
| **Q3d** | **A chapter merge pulls the next chapter's scenes into the merged one.** You noted this; today it is intended (the whole chapter is merged). | ✅ **RULED: no change.** **No change:** a chapter merge removes one chapter break, so every scene of the later chapter joins the earlier one, and its undo puts them back. |
| **Q3e** | **The manuscript is itself a Range** (user, 2026-10-21: *"the full manuscript is also a range, and each operation operates on subranges of the full manuscript range"*). Q3c gave a zero-length Range a different meaning at a chapter break (merge the chapters, keep the scenes) than at a scene break (join the scenes), while a cut of a sub-range must mean one thing. | ✅ **APPROVED (all three):** (1) a scene merge is a general `rangeCut` of the zero-length break, with no special form; (2) a chapter merge is a **relabel** of the later chapter's scenes (new `scrivi_range_relabel`), and its undo is the inverse relabel; (3) no general replace endpoint, so the ABI goes 100 → 99. User: *"In memory it is true that a chapter merge would be a relabel, not a cut, but that will require in ScriviCore that the scene files are moved and possibly new chapter folders be created when the in memory model is written to disk … Q3e: Approved for all three."* |
| **Q4a** | **What "removed" means** for an older structure. | ✅ **RULED: the clipboard and copy buffers only.** An old-style object found on the clipboard or in a copy buffer (a `scrivi.fragment.v1` fragment rather than a Range) is discarded: the clipboard or buffer reads as **empty**, so paste does nothing; no flat-text fallback. **The clipboard and copy buffers store Ranges, as history does.** History is NOT affected: old `structuredPaste` / `structuredCut` nodes still convert to the delta path ([SP-187] Q2). |

## Goal

✅ On the Mac and on Linux, every structural edit in this Sprint is **one range call** (`rangePaste` or `rangeCut`), made **off
the main thread**, with the returned delta applied through [SP-187]'s partial re-render. These are copy, paste, split,
chapter split, merge and chapter merge.
- **Nothing reloads the manuscript.**
- Each edit **undoes and redoes in one step**: no "Can't undo past…" stops.
- An undo restores the removed scenes and chapters **exactly**, sidecars included.
- The scene list equals the disk after every edit, and [I-0314] is gone.

## Plan

**ScriviCore**
1. **The Range carries sidecars (Q3b).** A `RangeEntry` (`Range.hpp:33`) gains:
   - `sceneMeta`: the scene's whole sidecar as stored, on an entry that covers its whole scene;
   - `chapterMeta`: the chapter's metadata, on the entry that opens a chapter the operation removes.

   Both are additive fields in `scrivi.range.v1`. The core fills them when it reads or cuts. A Range the editor builds leaves
   them empty. `rangePaste` restores a sidecar **exactly when it keeps the entry's ID**, which happens when the manuscript no
   longer has that ID (an undo of a removal, `RangeOperations.cpp:278`). A fresh ID gets a fresh sidecar (`newSceneMeta`,
   `:75`), so the paste of a copy starts clean, as it does today.
2. **Breaks through the general cut and paste (Q3e).** The manuscript is one Range, and every edit works on a sub-range of it.
   A zero-length sub-range spanning a break is an ordinary sub-range, so it needs no special form.
   - **`rangeCut`** of it is a scene merge. The two scenes join **directly**, with no blank line (Q3a). It obeys the same
     rule as every cut (design §4.3).
   - **`rangePaste`** of an empty two-entry Range is Q1's split, or its undo:
     - empty IDs mint fresh ones (a split);
     - the cut's own IDs and sidecars bring the removed scene back (an undo).
   - A chapter split is the same paste with two chapter IDs, and its undo is the cut of that break. The following scenes
     move with it, as in every cut and paste.
3. **Chapter merge is a relabel (Q3e).** In the Range, a chapter break is only neighbouring entries with different
   `chapterID`s, so a chapter merge removes nothing. It changes the chapter of a run of whole scenes.
   - **New endpoint, `scrivi_range_relabel`.** It takes a Range of whole scenes, by ID and without their text, plus the
     target chapter. No text crosses the ABI.
   - **On disk it is a move, not an edit.** The scene files are relocated into the target chapter's folder under new order
     keys, and the chapter left empty is deleted, folder and metadata. This keeps [I-0083]'s order: relocate first, delete
     the folder last (`ChapterMerger.cpp:142`). The texts are not rewritten.
   - **Its undo is the inverse relabel.** It re-creates the chapter's folder and metadata from the Range's `chapterMeta`
     (Q3b), then moves the scenes back.
   - It returns the delta (`Moved` changes, `removedChapterIDs`, `createdChapters`) and the Range for the inverse.
   - The move is held to a count guard: N scenes = N relocations, one chapter metadata write, and one folder created or
     removed.
   - The scene moves inside a chapter split ([I-0311]'s promotion) share this code.
4. **Endpoints.** Retire `scrivi_merge_scene` and `scrivi_merge_chapter` (`scrivi.h:548`, `:559`) and add
   `scrivi_range_relabel`, which takes the ABI from **100 to 99**; update the gap audit to match. ⛔ There is no general
   "replace" endpoint (Q3e): cut, paste and relabel cover every operation. Remove `SceneMerger`, and fold `ChapterMerger`'s
   relocation into the relabel, unless `FragmentCutter` still needs them. Boundary tests go through `scrivi_*`.

**Both apps**

5. **Split and chapter split (Q1)** → `rangePaste` of the empty two-entry Range at the caret.
   - Mac: this fixes **[I-0314]** (`handleCreateChapter`, `ManuscriptTextView.swift:1412`).
   - Linux: this replaces `createChapter` plus one `reorderScene` per following scene (`EditorShell.cpp:2098`, `:2159`).
6. **Merge** (Ctrl-Backspace at a scene's start) → `rangeCut` of the break. **Chapter merge** (Ctrl-Shift-Backspace at a
   chapter's start) → `rangeRelabel` of the later chapter's scenes.
7. **Copy and copy into a buffer** build the Range from the list, with no core call and no disk read (§4.2b).
   - The clipboard's held structure and the copy buffers **store Ranges**, as history does (Q4a).
   - `scrivi.buffers.v1` slots gain `range`. A slot that still holds an old `fragment` reads as **empty**.
   - The system clipboard still carries flat text only (T2=A). The held structure lives in memory only, so in practice an
     old object can only be in a buffer.
8. **Paste** → `rangePaste` of the held Range.
9. **History.** Every operation above records a `rangePaste` / `rangeCut` / `rangeRelabel` node with its Range. Undo is the inverse call;
   redo is the call again.
   - **The barriers are removed:**
     - Apple `sceneSplit` `:1266`, `chapterSplit` `:1425`, `sceneMerge` `:1584`, `chapterMerge` `:1654`, and the paste's barrier-with-payload
       `:2791`;
     - Linux `:1980`, `:2116`, `:2231`, `:2307` and `:2565`.
   - Old `structuredPaste` / `structuredCut` nodes convert to a Range and take the same path ([SP-187] Q2).
   - The delete barriers (scene and chapter delete) stay; they are out of scope.
10. **Off the main thread, with Q5's cue.**
   - Apple uses a detached task; Linux uses `runStructural`.
   - The cue lands in the same event-loop turn as the key press, before any core call. The editor is then read-only.
   - Repeated gestures, typing and undo are ignored, not queued.
   - A progress row appears after 400 ms.
11. **Retire** Apple's `reloadManuscriptFromDisk`, Linux's structural `reloadThen` and SP-185's `reloadTexts_` (design §4.6).
   This also covers [I-0297]'s reload half.

**Docs and proof**

12. **The design doc moves to v0.3.** It records Q1, Q3, Q3a–Q3e, Q4a, the sidecar fields and the relabel.
13. **Tests on both**, against [SP-187]'s baseline. Each check below runs after every operation and after every undo and redo:
    - the list equals the disk (the check that would have caught [I-0314]);
    - the sidecar survives a round trip: tags, story time, band, outline, to-dos and status come back after merge-and-undo,
      and a removed chapter's metadata comes back after chapter-merge-and-undo;
    - an old fragment in a buffer reads as empty;
    - zero re-opens and zero main-thread core calls.

    All of it is mutation-checked, with ctest on the Mac and Linux, interop tests, and the Linux smokes.
14. **Live passes (user):** Mac and rig, over the share, each operation timed by the diagnostic logs.

⛔ **NOT in this Sprint:**
- structural cut and Backspace / Delete over scenes ([SP-188], which inherits items 1, 2 and 9);
- the background sync verification ([SP-189]);
- EP-052's write queue (W7-Q2 unruled);
- the delete barriers.

## Acceptance Criteria

- [ ] AC1 — **Core:** `RangeEntry` carries `sceneMeta` / `chapterMeta`. A paste that keeps an ID restores its sidecar
      exactly, and a fresh ID gets a fresh one. Boundary-tested through `scrivi_*`.
- [ ] AC2 — **Core:** breaks and relabels.
      - A scene merge is a general `rangeCut` of the zero-length break, joining with no blank line.
      - A chapter merge is `scrivi_range_relabel`: the scene files move to the earlier chapter's folder and the empty
        chapter is deleted, with no text rewritten, held to its count guard.
      - Each undo brings back the IDs, the sidecars, the chapter's folder and its metadata.
      - `scrivi_merge_scene` / `scrivi_merge_chapter` are retired, and the gap audit reads 99.
- [ ] AC3 — Copy and copy-into-buffer build the Range from memory, with zero core calls and zero disk reads, on both. The
      clipboard and buffers hold Ranges, and an old fragment in a buffer reads as empty.
- [ ] AC4 — Paste across scenes and chapters is one `rangePaste` off the main thread, with the delta applied and no reload, on
      both. [I-0311]'s order is right in the editor and on disk.
- [ ] AC5 — Split, chapter split, merge and chapter merge are each one range call off the main thread, with the delta applied
      and no reload, on both. The list equals the disk afterwards ([I-0314]).
- [ ] AC6 — Each of these operations undoes and redoes in one step:
      - no "Can't undo past…" stop remains for split, merge or paste;
      - old `structuredPaste` / `structuredCut` nodes still undo, through the same path.
- [ ] AC7 — Q5 on both: the cue comes in the key press's own event-loop turn; repeats, typing and undo are ignored while the
      operation is in flight; a progress row appears after 400 ms.
- [ ] AC8 — [SP-185] AC4: a structured paste's undo over the share makes no project re-open. The three reload paths are gone.
- [ ] AC9 — Tests on both: list = disk after every operation, undo and redo; the sidecar round trip; zero re-opens; zero
      main-thread core calls. All mutation-checked, with ctest (Mac and Linux), interop and the Linux smokes green.
- [ ] AC10 — The design doc is at v0.3 with these rulings.
- [ ] AC11 — ✅ **Live passes (user):** Mac and rig, over the share, each operation timed.

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- From [SP-187]'s Q1 ruling (user: split S2). Issued via `next-id.py`; [T-0619] filed. Steps 3–8 of the original S2 plan
  and its AC2–AC8 moved here.

### 🔵 2026-10-21 — Planning: the code re-read; [I-0314] found; Q1–Q6 raised
- User: *"start planning SP-191"*.
- The code reading was re-checked against the code after [SP-187] (line numbers updated). Apple's split and merge already
  apply in memory; Linux reloads after every structural operation.
- Found: on the Mac a chapter split appends the new chapter at the manuscript's end on disk → **[I-0314]** `[Apple]`, filed to
  the backlog. Proven by a temporary interop probe (memory `Alpha one. | Alpha two. | Beta.`, disk
  `Alpha one. | Beta. | Alpha two.`); the probe was removed after it ran.
- Found: a split and a chapter split are each a `rangePaste` of an empty two-entry Range (Q1); a merge is not a range
  operation (Q2).
- ⏳ Q1–Q6 for ruling. The Plan and ACs are updated once they are ruled.

### 🔵 2026-10-21 — Q1–Q6 ruled; Q3a–Q3d and Q4a raised
- The rulings are recorded under Questions. Q3 changes the Sprint's shape: split and merge (scene and chapter) become
  undoable range operations, and the barriers go.
- Reading for Q3 found that a Range carries no sidecar data (`Range.hpp:33`, `RangeEntry`), while the sidecar holds tags,
  story time, the band, the outline, to-dos and status (`SceneMetaJson.cpp`). An undo through the Range would lose them →
  Q3b, which [SP-188] shares.
- ⏳ The Plan and ACs are rewritten once the follow-ups are ruled.

### 🔵 2026-10-21 — Q3c, Q3d, Q4a ruled; ⏳ Q3a, Q3b
- Q3c agreed (break-only range cut/paste; the merge endpoints retire). Q3d: no change.
- Q4a narrows Q4: only the clipboard and copy buffers drop old-style objects (read as empty) and store Ranges. History keeps
  [SP-187] Q2's conversion, so Plan 3 and AC3 stand. [SP-188]'s note corrected.
- ⏳ Q3a (the merge's blank line) and Q3b (the sidecar in the Range) remain; the Plan and ACs are rewritten after them.

### 🔵 2026-10-21 — Q3a, Q3b ruled; Goal, Plan and ACs rewritten; [I-0314] taken in
- Q3a agreed: no blank line. Q3b agreed: the sidecar travels in the Range.
- Reading `RangeOperations.cpp` gave Q3b its rule. `pickSceneID` (`:278`) keeps a Range's ID only when the manuscript lacks
  it, so restoring the sidecar exactly when the ID is kept covers every undo and leaves a copy's paste fresh. ⚠️ The
  consequence for [SP-188]: a cut-then-paste elsewhere keeps the IDs, so the scenes move WITH their sidecars.
- Read while rewriting: on both platforms the cross-boundary paste is itself recorded as a barrier that carries the
  structural payload (Apple `ManuscriptTextView.swift:2791`, Linux `EditorShell.cpp:2565`). It becomes a `rangePaste` node.
- [I-0314] moved from the backlog to `Issue-active.md`, assigned here.
- ⏳ Ready for activation on the user's approval.

### 🔵 2026-10-21 — Q3e ruled: the manuscript is a Range; chapter merge is a relabel
- User: the full manuscript is a Range, and every operation works on a sub-range of it. That exposed a conflict in Q3c: the
  same zero-length Range meant two things at the two kinds of break.
- ✅ Q3e approved: scene merge = the general cut; chapter merge = `scrivi_range_relabel`, which moves the scene files and
  deletes or re-creates the chapter folder (the user's point: in memory it is a relabel, on disk it is a move); no general
  replace endpoint. The Plan is now items 1–14 and AC2 is rewritten. ABI count 99.
- ⏳ Ready for activation on the user's approval.

### 🟡 2026-10-21 — ACTIVATED
- User: *"activate SP-191"*. [T-0619] → active; [I-0314] and [I-0311] assigned here.
