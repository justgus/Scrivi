---
sprint: SP-191
epic: EP-053
status: Active
platform: Cross
created: 2026-10-20
---

# SP-191 — `[Apple]` + `[Linux]` [EP-053] **S2b**: copy, paste and its undo, split and merge — as deltas, off the main thread, no reload

**Status:** 🟡 **ACTIVE 2026-10-21** (user: *"activate SP-191"*) — ✅ AC1–AC10 met 2026-10-21; ⏳ AC11 live passes (user) — ✅ all questions ruled 2026-10-21 (Q1–Q6, Q3a–Q3e, Q4a) · created 2026-10-20 by [SP-187]'s Q1 ruling (user: split S2). ✅ Its questions were ruled in
[SP-187] (Q2: old history nodes convert; Q3: `ViewportSceneLoader` applies the delta).
**Carries:** [I-0314] (Q6) · [T-0619] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md) · [SP-185] AC4 (user, 2026-10-20: *"AC4 goes
to EP-053 S2"*) · [I-0297]'s reload half · [I-0311] (the apps reach `scrivi_range_paste`).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1, the endpoints) · [SP-187] (S2a: the list as the model of record, the bindings, the delta types,
the partial re-render, the baseline counts).
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_3.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_3.md) (v0.3, 2026-10-21, supersedes v0.2) — R1,
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
| **Q3e** | **The manuscript is itself a Range** (user, 2026-10-21: *"the full manuscript is also a range, and each operation operates on subranges of the full manuscript range"*). Q3c gave a zero-length Range a different meaning at a chapter break (merge the chapters, keep the scenes) than at a scene break (join the scenes), while a cut of a sub-range must mean one thing. | ✅ **APPROVED (all three):** (1) a scene merge is a general `rangeCut` of the zero-length break, with no special form; (2) a chapter merge is a **relabel** of the later chapter's scenes (new `scrivi_range_relabel`), and its undo is the inverse relabel; (3) no general replace endpoint, so the ABI goes 100 → 99 (⚠️ the count was wrong: 136 → 135, Plan 4). User: *"In memory it is true that a chapter merge would be a relabel, not a cut, but that will require in ScriviCore that the scene files are moved and possibly new chapter folders be created when the in memory model is written to disk … Q3e: Approved for all three."* |
| **Q4a** | **What "removed" means** for an older structure. | ✅ **RULED: the clipboard and copy buffers only.** An old-style object found on the clipboard or in a copy buffer (a `scrivi.fragment.v1` fragment rather than a Range) is discarded: the clipboard or buffer reads as **empty**, so paste does nothing; no flat-text fallback. **The clipboard and copy buffers store Ranges, as history does.** History is NOT affected: old `structuredPaste` / `structuredCut` nodes still convert to the delta path ([SP-187] Q2). |

### Raised while implementing — ✅ ruled 2026-10-21

User, Q7: *"Position caret at the first character of a scene and type Return simply adds a new blank line to the beginning of
the Scene. Cmd-Return creates a new scene before the current scene and pushes the caret inside the new scene (the assumption
is that the writer created a new scene and now wants to type in it). A range paste at scene offset 0 should paste everything
before the scene, creating any new scenes before is as are necessary. The last part of the scene is merged with the
beginning of the current scene and the caret is placed at the end of the paste block (the assumption being that the writer
wants to add the copied text to this part of the manuscript and continue on from there)."* Q8, Q9: as recommended.

| # | Question | Ruling |
| - | - | - |
| **Q7** | A split or paste at a scene's very START (offset 0) | ✅ **Before the scene.** The new scenes go before it; the scene keeps its text, identity and sidecar, and takes the last part in front of its text. Cmd-Return there leaves the caret in the new scene; a paste leaves it at the end of the pasted block. ⚠️ **What follows for the cut (so every undo is exact):** a cut that begins at a scene's first character keeps the END scene — the scene whose text survives keeps its identity. [SP-188]'s cuts follow the same rule. |
| **Q8** | Chapter split with the caret at a scene's start | ✅ **Relabel:** the scene and the rest of its chapter move into a new chapter; no empty scene, no text moves (the exact inverse of a chapter merge). At a chapter's first scene it does nothing. |
| **Q9** | The chapter-split confirmation ("… This cannot be undone.") | ✅ **Removed:** the split is immediate and Undo reverses it. |

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
   `scrivi_range_relabel`: **136 endpoints → 135** (⚠️ corrected 2026-10-21 while implementing: "100 → 99" was the gap
   audit's 2026-08-24 snapshot, not today's `scrivi.h`). The gap audit records the retirement. ⛔ There is no general
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

**Live-pass follow-ups (ruled 2026-10-21, after the user's live passes)**

15. **Q10 — chapter numbers derived, never stored, on both.** The core writes untitled chapters (`ChapterCreator`,
    `ProjectCreator`, `RangeOperations` openChapter and relabel); `parseChapterMeta` reads a stored `Chapter <number>` as
    untitled; `extractSearchableText` derives the container title. Linux's `renumberCreatedChapters` (the ~12 s stall) and
    the Mac's `renumberChapterTitlesFrom` are removed; both apps' headings, navigator and timeline dots derive the number.
16. **The heading anchor, on both.** Every structural edit steps back over hidden line-start and opener stops first (Apple
    `structuralAnchorAndLine`, Linux `EditorShell::structuralAnchor`); a multi-entry paste anchored before a heading's or
    list item's marker ends its last part with a line break (⚠️ a judgment call: vetoable).
17. **[I-0315] on Linux** (the Mac already had this shape): the Inspector writes the outline and to-do only when changed,
    and never to a removed scene.
18. **The Linux worker reads only the story times** (`scrivi_list_story_times`, as the Mac does) and logs each phase's time,
    plus the GUI-thread apply.

⛔ **NOT in this Sprint:**
- structural cut and Backspace / Delete over scenes ([SP-188], which inherits items 1, 2 and 9);
- the background sync verification ([SP-189]);
- EP-052's write queue (W7-Q2 unruled);
- the delete barriers.

## Acceptance Criteria

- [x] AC1 — **Core:** `RangeEntry` carries `sceneMeta` / `chapterMeta`. A paste that keeps an ID restores its sidecar
      exactly, and a fresh ID gets a fresh one. Boundary-tested through `scrivi_*`.
- [x] AC2 — **Core:** breaks and relabels.
      - A scene merge is a general `rangeCut` of the zero-length break, joining with no blank line.
      - A chapter merge is `scrivi_range_relabel`: the scene files move to the earlier chapter's folder and the empty
        chapter is deleted, with no text rewritten, held to its count guard.
      - Each undo brings back the IDs, the sidecars, the chapter's folder and its metadata.
      - `scrivi_merge_scene` / `scrivi_merge_chapter` are retired (`scrivi.h` 136 → 135), recorded in the gap audit.
- [x] AC3 — Copy and copy-into-buffer build the Range from memory, with zero core calls and zero disk reads, on both. The
      clipboard and buffers hold Ranges, and an old fragment in a buffer reads as empty.
      ✅ ⚠️ one stateless balancing call per piece (`scrivi_markdown_*`, SP-174) — nothing reads the project.
- [x] AC4 — Paste across scenes and chapters is one `rangePaste` off the main thread, with the delta applied and no reload, on
      both. [I-0311]'s order is right in the editor and on disk.
- [x] AC5 — Split, chapter split, merge and chapter merge are each one range call off the main thread, with the delta applied
      and no reload, on both. The list equals the disk afterwards ([I-0314]).
- [x] AC6 — Each of these operations undoes and redoes in one step:
      - no "Can't undo past…" stop remains for split, merge or paste;
      - old `structuredPaste` / `structuredCut` nodes still undo, through the same path.
- [x] AC7 — Q5 on both: the cue comes in the key press's own event-loop turn; repeats, typing and undo are ignored while the
      operation is in flight; a progress row appears after 400 ms.
- [x] AC8 — [SP-185] AC4: a structured paste's undo over the share makes no project re-open. The three reload paths are gone.
      ✅ by test (no load, no re-open, both); ⏳ over the share: AC11.
- [x] AC9 — Tests on both: list = disk after every operation, undo and redo; the sidecar round trip; zero re-opens; zero
      main-thread core calls. All mutation-checked, with ctest (Mac and Linux), interop and the Linux smokes green.
      ✅ — Mac: 0 own main-thread calls (1 caret bookkeeping); Linux: 0 own (the rest are [I-0315]). ctest 748 Mac / 752 Linux, interop 301, smokes 38; mutations core 13, Mac 10, Linux 11, all caught.
- [x] AC10 — The design doc is at v0.3 with these rulings.
- [ ] AC11 — ✅ **Live passes (user):** Mac and rig, over the share, each operation timed.
      ✅ 2026-10-21: all 12 actions pass on both; the details are AC12–AC15. ✅ Mac re-pass (heading cases, Q10 numbering,
      regression): all steps pass (user). ⏳ The rig re-pass.
- [x] AC12 — Q10: a created chapter is stored untitled; a stored `Chapter 7` reads as untitled, `Chapter of Secrets` is
      kept (core tests, mutation-checked); no app rewrites chapter titles after a structural edit.
- [x] AC13 — The heading anchor: at a scene that starts with `## `, split, chapter split and paste keep the heading with
      its scene, and each undoes exactly (Mac interop `structuralEditsAtAHeading`, Linux `structural_baseline_smoke`;
      mutation-checked on Linux).
- [x] AC14 — [I-0315]: a split's GUI-thread core calls on Linux 6 → 2, guarded; mutation-checked.
- [x] AC15 — The Linux worker reads only the story times and logs each phase. ⏳ The rig's timings: AC11.

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

### 🟡 2026-10-21 — ScriviCore done (Plan 1–3, the buffers half of 7); Q7–Q9 raised and ruled
- User: *"implement SP-191"*.
- **The Range carries sidecars (Q3b):** `RangeEntry::sceneMeta` / `chapterMeta` (`Range.hpp`), optional strings in
  `scrivi.range.v1`; an empty `sceneID` / `chapterID` is now accepted (Q1). A cut fills them for every scene and chapter it
  removes (one sidecar read each); a paste that keeps an entry's ID writes them back, and patches the story-time index.
- **Relabel (Q3e):** `RangeOperations::relabel` + `scrivi_range_relabel` — into the chapter before (append), after
  (prepend), or a new one (from `chapterMeta` on an undo). The files move; texts are never rewritten; an emptied chapter is
  deleted after its scenes are out ([I-0083]) and its sidecar travels for the undo. Result: `{delta, range, fromChapterID}`.
  Shared move helpers (`movedInto`, `moveSceneFiles`, `removeChapterFolder`) now serve cut, paste and relabel.
- **Q7 in the core:** a paste at offset 0 creates its scenes before the caret scene (`ProjectIndex::insertScenesBefore`,
  new); a cut from a scene's first character keeps the end scene, and every cut now returns `undoCaretSceneID` /
  `undoCaretByte` for its inverse paste.
- **Buffers (Q4a):** `scrivi.buffers.v1` slots hold `range`; `scrivi_buffers_load` refuses anything that is not a
  `scrivi.range.v1`; a slot holding an old `fragment` reads as empty and the next write drops it.
- **Tests:** 16 new `[sp191]` C ABI cases (sidecar round trips incl. story time through the patched index, scene merge as a
  zero-length cut at scene and chapter breaks, split and chapter split via paste incl. [I-0314]'s on-disk order, chapter
  merge and its undo and redo, relabel refusals, every Q7 case) + 1 budget case (a 20-scene chapter merge: 23 reads,
  63 writes, no listing; its undo 2 reads, 44 writes) + 2 buffer cases. ⚠️ The two-scene cut's budget is now **5 reads**
  (was 4): the removed scene's sidecar, which Q3b needs. ctest **757/757** (Mac, Debug).
- **Mutations: 13 caught, 1 equivalent** (removed): restored scene/chapter sidecars ignored, sidecar not captured, relabel
  keeping colliding names, leaving the emptied folder, not capturing it, story time not patched, never prepending, never
  keeping the end scene, the wrong index insert at the manuscript's start, the caret scene not opening the last chapter,
  the wrong promotion anchor.
- ⏳ Next: the Mac app, then Linux; then the merge endpoints retire.

### 🟡 2026-10-21 — The Mac app done (Plan 5–11 on Apple); interop 304/304
- **One path for every structural edit** (`Coordinator.runStructuralEdit`, `ViewportSceneLoader.runStructural`): the cue
  in the key press's own turn (the editor goes read-only, so the caret goes; Q5); off the main actor the unsaved scenes
  are written, the pending typing committed, the ONE range call made, its history node recorded and the story times read;
  back on it `ViewportSceneLoader.apply(delta)` updates the scene list (new) and ONE run is rendered. Repeats and undo
  while in flight are ignored; the progress row ("Splitting the scene…") shows after 400 ms (`EditorView`). A close or quit
  waits for it.
- ⌘-Return / ⇧⌘-Return = `rangePaste` of a break (✅ [I-0314]: the chapter in place on disk; Q7 at a scene's start; Q8 a
  relabel; Q9 the confirmation removed); ⌘⌫ = `rangeCut` of the zero-length break (joined directly); ⇧⌘⌫ =
  `rangeRelabel`. ⌘C and copy-into-buffer build the Range from the list (`copiedRange`; ⛔ `fragmentExtract` read every
  selected scene from disk); ⌘V and ⌃⌘N = `rangePaste`. The clipboard holder and the buffers hold Ranges.
- **History:** every edit records ONE `rangePaste` / `rangeCut` / `rangeRelabel` node holding the Range both its undo and
  its redo use; the split/merge/paste barriers are gone. Old `structuredPaste` / `structuredCut` nodes convert to a Range
  ([SP-187] Q2). An inverse the core refuses steps the pointer back.
- ⛔ **Retired:** `reloadManuscriptFromDisk`, the per-op `createScene`/`saveScene`/`createChapter`/`mergeScene`/`mergeChapter`
  sequences, `fragmentSpans`, `balancedFragment`, the structural redo cache.
- **Measured** (interop, `RunRenderTests`, serialized): every structural operation makes **0 core calls of its own on the
  main thread** (was 5–33) and **re-opens nothing** (was up to 1), with ONE run render. ⚠️ Placing the caret afterwards
  makes **1** in-memory history call (`captureCaretMoved`, as every caret move does) — counted apart, per editor
  (`StructuralCallSink`, a task-local, so parallel tests cannot inflate it). A cross-scene ⌘C makes only the stateless
  balancing calls, one per piece (SP-174), and reads nothing from the project.
- **Tests (new / changed):** each of split, split at a scene's start, merge, chapter split, chapter split at a scene's
  start and chapter merge undoes and redoes in ONE step with the list equal to the disk after each (`listMatchesDisk`, a
  fresh open); Q7, Q8; a merge's undo restores the merged-away scene's tags and outline through the history JSON; Q5's cue,
  ignored repeat and undo; the 400 ms progress row; the copy; the buffer Range round trip (and an old fragment refused);
  the structured-paste tests moved to Ranges and now also redo. The merge tests expect no blank line (Q3a). The I-0302
  barrier test uses a scene-delete barrier (a split is no longer one).
- ⏳ Next: mutation checks on the Mac code, then Linux.

### 🟡 2026-10-21 — Linux done (Plan 5–11 on Linux); smokes 38/38; design v0.3
- **The same one path** (`EditorShell::runRangeEdit`, Apple's `runStructuralEdit`): `lockForWork` is the cue in the key
  press's own turn (read-only, the progress row after 400 ms); on the WORKER the unsaved scenes are written, the pending typing
  committed, the ONE range call made, its history node recorded (`HistoryCapture::barrierParams` + the static, thread-safe
  `captureRecordBarrier`) and the timeline read; on the GUI thread `SceneDocument::applyRangeDelta` (new) re-renders only the
  run that changed, and the caret is placed. A repeat while one is in flight is ignored. `structuralEditFinished` (new
  signal) tells the smokes it landed.
- ⌃↩ / ⌃⇧↩ / ⌃⌫ / ⌃⇧⌫, ⌃V and ⌃⇧N are the same range calls as the Mac's, with the same history payloads (`rangePaste` /
  `rangeCut` / `rangeRelabel`, `rangeJSON`); old `structuredPaste` / `structuredCut` nodes convert ([SP-187] Q2). The copy
  builds the Range from the scene list (`ManuscriptEditor::crossSceneCopy`; ⛔ `fragmentExtract` read every selected scene
  from disk on the GUI thread). The clipboard holder and the buffers hold Ranges; an old fragment slot reads empty.
- ⛔ **Retired on Linux:** `reloadThen`, `reloadThenPlaceCaret`, `runStructural`, [I-0297]'s `reloadTexts_` (and the load's
  "known texts" path), `structuralRedoCreated_`, the per-op `createScene`/`saveScene`/`createChapter`/`reorderScene`/
  `mergeScene`/`mergeChapter` sequences and their barriers.
- **Measured** (`structural_baseline_smoke`, Docker Qt 6.4 Debug): every operation now makes **0 project loads and 0
  document builds** (was 1 and 1). GUI-thread core calls: split 6, merge 3, chapter split 6, chapter merge 0, copy 3, paste
  6, undo of the paste 3 (was 9, 5, 10, 4, 4, 9, 6). ⚠️ Traced (a temporary envelope trace, removed): **none of them is
  the edit's own** — every one is the Scene Inspector's reaction to the caret moving into another scene (the copy's too: the
  selection moved it): it reads the new scene's notes and REWRITES the previous scene's outline and to-do whether or not they
  changed → **[I-0315]** `[Linux]` filed to the backlog.
- ⚠️ **A known divergence kept as it was:** after a chapter is created or removed, Linux renames the stored "Chapter N"
  titles on disk (I-0063, `renumberCreatedChapters`, GUI thread); the Mac renumbers them in memory only. Not changed here.
- **Tests (Linux):** `structural_baseline_smoke` now checks, after every operation, undo and redo, that the scene list
  equals the disk (`listMatchesDisk`), no load and no document build; round-trips split, split at a scene's start, merge,
  chapter split, chapter split at a scene's start and chapter merge (each ONE step); Q7, Q8 (and nothing at a chapter's
  first scene), Q5's cue and an ignored repeat. `undo_smoke`: the split is undoable (the I-0302 barrier test uses a scene
  delete barrier), the merge joins directly. `structured_paste_smoke`: waits for the landing, asserts no reload, the buffers
  hold Ranges, a Mac-written Range slot pastes, an old fragment slot reads empty, the nodes are `rangePaste`, and the Mac's
  OLD `structuredPaste` and collapsing `structuredCut` nodes undo and redo converted. `settings_smoke`: the face survives a
  structured paste that no longer reloads.
- **Mac mutations: 10/10 caught** (after two test additions — the loader's own in-flight refusal, the caret after a
  mid-scene split): the delta ignoring `afterSceneID`, no cue, no refusal in flight, no history node, a cut's redo pasting,
  an unbalanced copy, the progress row at once, a chapter split at a scene's start pasting, the split's caret, the story
  times read on the main thread.
- **Design:** `Scrivi_Structured_CutCopyPaste_Design_v0_3.md` (new; v0.2 marked superseded) records Q1–Q9 (§1b), the sidecar
  fields, relabel (§4.4a), the one-Range history nodes; the active Sprint and Epic docs point at it.
- ⚠️ **Corrected:** Plan 4's "100 → 99" was the gap audit's 2026-08-24 snapshot; `scrivi.h` goes **136 → 135**.

### 🟡 2026-10-21 — The merge endpoints retired; every suite green; ⏳ AC11 live passes (user)
- ⛔ **Retired:** `scrivi_merge_scene` / `scrivi_merge_chapter` (`scrivi.h` 136 → 135), the facade's `mergeScene` /
  `mergeChapter`, `SceneMerger`, `MergeSceneRequest` / `Result`; Swift `ScriviEngine.mergeScene` / `mergeChapter` (+ stub,
  + result types, + 4 interop tests); Linux `ScriviBridge::mergeScene` / `mergeChapter`. `ChapterMerger` stays for
  `FragmentCutter` (neither app calls it). The 9 facade tests in `MergeSceneTests.cpp` went; its 2 tests of the I-0083
  composition stay. `scene_merge_smoke` now merges through `rangeCut` / `rangeRelabel` and undoes both. The gap audit
  records the retirement.
- **Linux mutations: 11/11 caught** (the delta ignoring `afterSceneID`, unsaved scenes not written first, no history node,
  no cue, no refusal in flight, a cut's redo pasting, an unbalanced copy, a chapter split at a scene's start pasting, the
  split's caret, a balanced paste not normalized, a written scene left dirty).
- **Fixed on the way:** `MergeSceneTests`' `TempDir` named directories by the clock alone; under `ctest -j6` on Linux two
  collided once (a kept test failed, then passed alone). Now the process ID and a counter too; 3 full Linux runs green.
- **Mac quit:** `saveAllDirtyBlocking` now also waits for a structural edit in flight (the close/quit wait panel already
  did, through `isWritingInBackground`).
- **Final:** ctest **748/748** (Mac, Debug) and **752/752** (Linux, Qt 6.4 container, non-root, Debug); interop **301/301**;
  Linux smokes **38/38** (`SCRIVI_VERIFY_SCENE_TEXT=1`); stub parity and the ABI project guards clean.
- ⏳ **AC11 — the live passes (user):** steps in the chat reply.

### 🟡 2026-10-21 — Live passes: all 12 actions PASS on the Mac and the rig; details to fix; Q10 + two scope rulings
- User: *"The actual actions for all 12 steps above on both the Mac and the Rig pass."* The details reported:
  - **Mac 6, 7, 9 (and the rig):** at a scene's start that is a HEADING (`## …`), ⇧⌘-Return leaves the `##` behind in the
    previous chapter, and a paste there turns the pasted text into the heading and the heading into body text. ✅ Cause: the
    heading's hidden `## ` is a caret stop whose home is AFTER it, so the anchor lands 3 bytes into the scene — not at its
    start (offset 0 would have taken the Q7 / Q8 paths). The same holds for an opening `**` and a list marker.
  - **Rig 2, 5–8:** undo, every merge, split chapter, merge chapter and the structured paste take too long. ✅ The rig's log
    (build 76): the workers take 0.4–2.3 s; after an undo of a split the Inspector's "set scene" took **45,309 ms** (its
    writes to the scene the undo had just removed — [I-0315]); after every chapter split and cross-chapter paste the window
    stalled **~12.3 s** in unmarked code (Linux's I-0063 rename of every later "Chapter N" title, on the GUI thread, over
    the share).
- ✅ **Q10 (user): chapter numbers are DERIVED, never stored, on both platforms.** An untitled chapter stores no title; both
  apps show "Chapter N" from its position; the core stops writing "Chapter N"; Linux's disk renumbering and the Mac's
  in-memory renumbering go; a stored title that is exactly "Chapter <number>" is treated as untitled (pre-release).
- ✅ **[I-0315] into SP-191** (user): the Inspector writes only what changed, never to a scene that is gone; the Mac checked.
- ✅ **The Linux worker reads only the story times after a structural edit** (as the Mac does), and logs each phase's time.

### 🟡 2026-10-21 — The live-pass follow-ups done (Plan 15–18); every suite green; ⏳ AC11's re-pass (user)
- ✅ **Q10:** the core stores no "Chapter N"; a stamped one reads as untitled. Linux's disk renumbering and the Mac's
  in-memory renumbering removed; dead Mac loader functions (the old split / merge / insert paths) removed with it.
  `scene_rename_smoke` and `scene_reorder_smoke` expectations updated (they assumed a stored or a bare "Chapter" title).
- ✅ **Heading anchor** on both (Plan 16). ⚠️ The line break a paste adds before a heading is my call — vetoable.
- ✅ **[I-0315]** fixed on Linux; the Mac's Inspector already wrote only what changed.
- ✅ **Linux worker:** saves · flush · range call · history node · story times, each timed in the log line
  `structural "…": worker finished in N ms — saves … ms · flush … ms · …`, then `structural "…": applied in N ms`.
- ⚠️ Not explained yet: the rig's *"apply history step took 3524 ms"* on an earlier undo. The new log lines will show
  whether it recurs.
- ✅ **Results:** Mac ctest 750/750, interop 302/302; Linux ctest 754/754, smokes 38/38; core Q10 mutation caught;
  Linux mutations 15/15 caught (4 new: the heading step-back, the heading line break, each I-0315 compare).
- ✅ **Mac re-pass (user): *"All Mac steps pass"*** — the heading cases (split, chapter split, paste, each undone), Q10's
  derived numbering (split, merge, undo/redo, an older project) and the regression steps. ⏳ The rig re-pass.
