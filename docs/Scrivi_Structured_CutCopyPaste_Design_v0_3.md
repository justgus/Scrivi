# Scrivi — Structured Cut / Copy / Paste Design (v0.3)

**Status:** ✅ **RULED** — v0.2's R1–R7 and Q1–Q5 (user, 2026-10-17), and v0.3's additions (user, 2026-10-21, [SP-191]
Q1–Q9, §1b). Implementation is EP-053 (§6). Supersedes v0.2 (`Scrivi_Structured_CutCopyPaste_Design_v0_2.md`) wherever they
differ — v0.2 stays the record of the 2026-10-17 rulings — and v0.1 (EP-029, 2026-07-27), the record of how the code before
EP-053 came to be.
**Date:** 2026-10-21 (v0.2: 2026-10-17) · **Author:** Claude, from the user's rulings · **Platforms:** ScriviCore, Apple, Linux
(the same shape on every platform, by rule).
**Occasioned by:** [I-0297] (a paste's undo froze for 40 s – 2 min over the network share), [I-0307] (select, copy and paste
froze 10–15 s), [I-0311] (a chapter-opening paste scrambles the reading order), and the measurements of 2026-10-17 (§2).

---

## 1. The rulings this revision implements (user, 2026-10-17)

- **R1 — The editor already knows the structure.** *"The ManuscriptView is also keeping track of chapter and scene
  boundaries. I know that because I can see them. There is no need to determine if a boundary has been crossed. You already
  know that it does."* A selection is contiguous: *"any scenes and/or chapters is adjacent and consecutive. Multiple searches
  are not necessary."*
- **R2 — An operation is a range, and its cost is the range's.** *"We can easily create a core function that just gets a
  range from 'beginning' to 'end' that will include the remaining text in the scene, any scenes and or chapters in between,
  and the beginning text in the last scene up to the end marker. The chapters and scenes are arranged on disk in the order
  they occur in the manuscript."* Paste is the same range, inserted.
- **R3 — Never throw the manuscript away.** *"Instead of throwing the manuscript away, we start a background 'sync
  verification' that checks the manuscript against the files affected and quietly makes any corrections needed while the
  writer is typing elsewhere."*
- **R4 — Cut removes structure; cut and paste are inverses.** *"If I cut across a scene or chapter boundary and paste
  somewhere the scene and chapter boundary are inserted into the new part of the manuscript. That is paste from cut. So … the
  cut operation must therefore pull the scene and chapters out of the manuscript and merge the beginning of the scene where
  the cut starts with the end of the scene where the cut ends."* *"An undo of a paste is a cut and an undo of a cut is a
  paste."* ⛔ This **reverses** the 2026-10-02 ruling ([I-0270]: a cross-scene cut deletes the text and keeps the scenes).
- **R6 — The Range: one structure, built anywhere, used everywhere** (user, 2026-10-17; §4.2). *"Define a 'range' in
  ScriviCore as a linked list of scenes. Each scene object will contain the starting position in the file of the first
  character of the range and the length … a forward link to the next scene in the chain and the previous link … a chapter
  identifier … to determine … the chapter to restore to when an undo occurs … the text of the scene from the file. This
  structure is compact, portable, transportable, and copyable. It can be constructed on the app side when a selection is made
  … used as the contents of a copy buffer … used to insert text into a manuscript independent of the format it is kept in
  memory … used to send to ScriviCore the data that will need to be written to disk. ScriviCore can use this structure to
  read data from the disk as a range and pass that data to the app for rendering. It can be specified as simply a starting
  point and an ending point and any part of the system can construct it from its own data representation or from disk."*
- **R7 — The in-memory manuscript is a doubly linked list of scenes, and disk work is minimal by construction.** *"If we
  structure the 'in memory' scene data as, say, a linked list, then getting the next scene becomes a trivial operation … store
  it as a doubly linked list of scenes, eliminating the need for a special 'scene break' or 'chapter break' character."*
  *"There should be an aggregate ScriviCore function that streamlines the process."* *"A two scene paste should take 4 file
  reads and 4 file writes. We must structure our in memory structures in such a way as to minimise the number of disk
  operations performed."*
- **R5 — Same work, same threads, both platforms.** *"If we need to run something in the background on Linux we need to do it
  also in Apple."* The reload's text reads: *"We must go into both."*

## 1b. The rulings v0.3 adds (user, 2026-10-21, [SP-191])

- **The manuscript is itself a Range** (user: *"the full manuscript is also a range, and each operation operates on subranges
  of the full manuscript range"*). Every structural edit is ONE range call on a sub-range of it; the scene list each editor
  holds IS that Range (§4.2a), and the disk is too — so after every edit, undo and redo, the list must equal the disk.
- **Split and chapter split are a PASTE of a break** (Q1): an empty two-entry Range at the caret; an entry with an empty
  `sceneID` / `chapterID` mints a fresh one. A chapter split is the same paste with its two entries in different chapters.
- **No "Can't undo past…" stops** for split, merge, chapter split, chapter merge or paste (Q3): each undoes and redoes in ONE
  step, like any cut or paste across a break. (A scene or chapter DELETE from the navigator keeps its barrier: not EP-053's.)
- **A scene merge is a CUT of the zero-length sub-range across the break** (Q3e), joined **directly** — no blank line (Q3a).
- **A chapter merge is a RELABEL** (Q3e, §4.4a): it removes nothing from the Range — a chapter break is only neighbouring
  entries in different chapters — so it moves a run of whole scenes into the chapter before. On disk their files MOVE (and a
  chapter folder is deleted or created); no text is rewritten (user: *"that will require in ScriviCore that the scene files
  are moved and possibly new chapter folders be created when the in memory model is written to disk"*). Every scene of the
  later chapter joins the earlier one (Q3d). No general "replace" call: cut, paste and relabel cover every operation.
- **What an undo restores travels in the Range** (Q3b): each removed scene's whole sidecar and each removed chapter's sidecar.
  A paste that keeps an ID (the manuscript no longer has it: an undo) writes them back; a fresh ID (a copy's paste) starts
  clean.
- **At a scene's very start, the scene whose text survives keeps its identity** (Q7, user: *"A range paste at scene offset 0
  should paste everything before the scene, creating any new scenes before it as are necessary. The last part … is merged with
  the beginning of the current scene and the caret is placed at the end of the paste block."* — and ⌘/Ctrl-Return there puts
  the new scene before it, the caret in the new scene). For every undo to be exact, a cut that begins at a scene's first
  character keeps the END scene.
- **A chapter split at a scene's start is a relabel** (Q8): the scene and the rest of its chapter start a new chapter; no
  empty scene, no text moves. At a chapter's first scene it does nothing. **No confirmation** before a chapter split (Q9).
- **The clipboard and the copy buffers hold Ranges, as history does** (Q4a). An old-style object found there (a
  `scrivi.fragment.v1`) is treated as EMPTY and silently dropped — no compatibility layer for them (Scrivi is unreleased).
  ⚠️ History is NOT affected: an old `structuredPaste` / `structuredCut` node converts to a Range and undoes ([SP-187] Q2).
- **Chapter numbers are DERIVED, never stored** (Q10, user, after the live passes): an untitled chapter stores no title, and
  every app shows "Chapter N" from its position, so a chapter created or removed renumbers every untitled chapter with nothing
  to rewrite. The core never writes "Chapter N"; a stored title that is exactly `Chapter <number>` (stamped before this
  ruling) reads as untitled. ⛔ Linux's rename of every later chapter after a structural edit (I-0063) and the Mac's
  in-memory renumbering are gone — over the share Linux's took ~12 s per chapter split.
- **A caret that rests after a hidden line-start marker anchors BEFORE it** (live pass): a heading's `## ` (and a list marker,
  an opening `**`) is a caret stop whose home is after it, so a caret at a heading scene's start sits 3 bytes in. Every
  structural edit steps back over such stops first (Apple `structuralAnchorAndLine`, Linux `EditorShell::structuralAnchor`),
  so a split there takes the heading with its scene and a chapter split there is Q8's relabel. A multi-entry paste anchored
  before a heading's or list item's marker ends its last part with a line break, so the heading stays a heading; the break
  is part of what the paste inserts, and its undo removes it.
- **While a structural edit is in flight** (Q5): an immediate cue in the key press's own event-loop turn, then read-only;
  repeats, typing and undo are ignored, not queued; the progress row after 400 ms — *"as long as the initial operation response
  is nearly instantaneous"*.

## 2. What the code does today (measured 2026-10-17)

Measured on Linux in Docker with `strace -f` (a real `EditorShell`, 1,186 generated scenes, each step bracketed by marker
probes); the Mac read from its code (`ManuscriptTextView.swift`, `ViewportSceneLoader.swift`). Both call the same ScriviCore
endpoints.

| Step | ScriviCore calls | File calls (1,186 scenes) | Mac | Linux |
| - | - | - | - | - |
| Copy across scenes | `scrivi_fragment_extract` (the scenes as a span list) | **2,678** (every scene sidecar, to learn chapter membership) → **3** after SP-185's fix (the index) | main thread | main thread |
| Paste | `scrivi_fragment_paste` | **184** (the writes ~20; the rest re-reading the chapter and the chapter list, below) | main thread | worker |
| …then the reload | `scrivi_open_project` + every scene's text | **~1,700** for the open (every sidecar) + every text | main thread; **1,186 separate `openSceneForBulkLoad` calls** | worker; texts in parallel batches (SP-185: only what changed — ⚠️ Linux only) |
| Undo of the paste | `scrivi_fragment_uncut_paste` + the same reload | the same | main thread | worker |
| Cut across scenes | none structural: per-scene text deletes ([I-0270], 2026-10-02) | the scenes' saves | — | — |

**Inside the 184 of one paste** (two pieces, into a 20-scene chapter): write the target (text + sidecar); create the new
scene — list the chapter, read all 20 sidecars; save it — list the chapter again, read 23 sidecars; refresh the index — list
the manuscript, read all 59 other chapters' sidecars, then the target chapter's sidecars twice (42); `workspace-state.json`
rewritten after each save (6); the history log (2).

## 3. Why it is like this (the causes, not the symptoms)

1. **Operations are compositions of per-item primitives, each re-deriving the manuscript from disk.** EP-027 made the order
   filesystem-authoritative, and EP-029 built extract, paste and cut out of the EP-027 primitives (create scene, save scene,
   create chapter, delete…). Each primitive re-lists its chapter and re-reads its siblings to find order keys; the index is
   then re-observed. Nothing passes what one step learned to the next.
2. **The platforms treat the manuscript as disposable after a structural change.** v0.1 §SP-089 / the Mac's
   `reloadManuscriptFromDisk`: *"a full re-open is more robust than in-memory patching"*. Single-scene split and merge already
   patch memory; the structured paste never did. Linux copied the shape in SP-180.
3. **The core rediscovers what the editor knows.** The editor holds every scene's ID, chapter, title and position; the core
   was handed only scene IDs and rebuilt chapter membership by resolving the whole manuscript (fixed for copy in SP-185 by
   asking the index — still a lookup the editor did not need).
4. **Cut and paste stopped being inverses.** After 2026-10-02 a cut removes text only, while a paste adds structure — so a
   paste's undo needed its own operation (`fragment_uncut_paste`), and a cut's undo is a text restore.
5. **It was measured on local disk.** Every cost above is invisible on an SSD's page cache and dominant over a network share.
6. **The platforms diverged instead of being fixed together.** Linux moved the work off its GUI thread (SP-180) and cut its
   reload's reads (SP-185); the Mac still does both on its main thread.
7. **[I-0311]** — a chapter-opening paste leaves the target chapter's later scenes before the new chapter.

## 4. The model (v0.3)

### 4.1 Principles
- **P1 — The platform states the range; the core trusts and checks it.** A selection is `(startScene, startByte) →
  (endScene, endByte)`. The core walks that range in its index's order — no search — and refuses if the range is not what its
  index holds (a fast check, no I/O), so a stale editor can never write the wrong files.
- **P2 — One ScriviCore call per writer action, and its file work is the range's.** It reads and writes only the scenes and
  chapters in the range, plus the order keys of the chapters it changes — computed once, from the index — and updates the
  index in place. Target (1,186 scenes, a two-scene paste): **tens of file calls, not hundreds.**
- **P3 — The result is the change.** Every structural call returns the full delta: each scene created, removed, moved or
  rewritten (ID, chapter, position, paths, text) and each chapter created or removed (ID, title, position). The platform
  applies it to its in-memory manuscript — as split and merge already do. ⛔ No re-open, no re-read.
- **P4 — Then a background sync verification** (R3): a worker re-reads only the files the delta names and compares them
  with memory (text, order, chapter membership). A difference is corrected quietly (Q3 rules which side wins).
- **P5 — The same threads on both platforms** (R5): the structural call runs off the main thread on the Mac and on Linux,
  behind the same short progress indication; verification is a background task on both.

### 4.2 The Range (R6) — `scrivi.range.v1`
A **Range** is an ordered, doubly linked list of **scene entries**. Each entry:

| Field | Meaning |
| - | - |
| `sceneID` | the scene |
| `chapterID` | the chapter the scene belongs to — on an undo, the chapter it is restored to |
| `chapterTitle`, `sceneTitle` | carried so a paste re-creates the names (v0.1 §4.4, T-0357) |
| `start`, `length` | the part of the scene's text the range covers: `0` and the whole length when it covers the scene |
| `text` | that part of the scene's text, as stored in the file |
| `sceneLength` | the scene's whole text (bytes): the entry covers the whole scene when `start` is 0 and `length` is this |
| `sceneMeta` | ✅ v0.3 (Q3b), optional: a REMOVED scene's whole sidecar, serialized — filled by the core when it removes the scene |
| `chapterMeta` | ✅ v0.3 (Q3b), optional: on the entry that opens a REMOVED chapter, that chapter's sidecar, serialized |
| `prev`, `next` | the neighbouring entries; `null` at the first and the last |

- **No break characters, anywhere in the data.** A scene break is simply the link from one entry to the next; a chapter
  break is a link between entries whose `chapterID` differs. Every entry after the first begins a scene.
- **Contiguous by construction:** a selection, a cut or a paste is one unbroken run through the manuscript, so the list never
  has gaps and "the next scene" is a pointer, never a search.
- **A single-scene range is one entry.**
- **Built by anyone, from its own data:** the editor from its in-memory list at a selection; ScriviCore from disk, given a
  start `(sceneID, offset)` and an end `(sceneID, offset)`, walking its index in manuscript order.
- **Used everywhere:** the clipboard's structured content, a copy buffer's content, the input to a paste, the output of a
  read and of a cut, and what the core writes.
- **Across the C ABI** (JSON over string, permanent): an ordered array of entries; the links are the array's order, and both
  sides rebuild the doubly linked list on arrival.
- ⚠️ **Offsets are UTF-8 bytes**, not characters. The files are UTF-8, the core already works in scene-local byte offsets, and
  "a character" means a different unit to Swift (grapheme clusters), Qt (UTF-16 code units) and C++. Bytes are the one unit
  every side measures the same way; each platform converts at its edge, as it does today.
- ✅ v0.3: `sceneMeta` / `chapterMeta` are STRINGS (serialized JSON), so each platform's codec carries them through untouched.
  An empty `sceneID` / `chapterID` mints a fresh one (Q1).
- ✅ v0.3 (Q4a): the clipboard and the copy buffers hold Ranges; an old fragment there reads as EMPTY. `scrivi.fragment.v1`
  (v0.1 §3) still converts to a Range for the old HISTORY nodes, which keep undoing ([SP-187] Q2).

### 4.2a The in-memory manuscript (R7)
- On both platforms the model of record becomes the doubly linked list of scenes (each node: identity, chapter, title, text).
- ⚠️ **One clarification, so nothing is assumed.** Today both editors keep the whole manuscript as ONE contiguous text inside
  the text view (`NSTextStorage` on the Mac, `QTextDocument` on Linux), with a divider drawn between scenes; the per-scene list
  (`ViewportSceneLoader.segments`, `SceneDocument::segments`) is kept beside it and re-derived from it. Under v0.2 the list is
  the data and the text view is only a rendering of it: what the writer sees still runs continuously with drawn dividers,
  but those dividers exist only on screen, never in the data, and an edit in the view is applied to its scene's node through
  the boundary table both platforms already keep. A structural change (§4.3–4.5) re-links nodes and re-renders only the
  scenes it touched.

### 4.2b Copy
- **Within one scene:** no ScriviCore call. The balanced text goes on the clipboard, as today.
- **Across scenes:** the editor builds the Range from its own list at the selection (R6) — no ScriviCore call and no disk
  read. Its scenes are saved already, or saved first (an ordinary save, nothing structural).

### 4.3 Cut (R4 — structural again)
`scrivi_range_cut(root, range)` — the editor sends the Range it built — **removes the range from the manuscript**:
- the start scene keeps the text before the range, joined **directly** to the text after the range in the end scene (no
  blank-line seam — the 2026-07-27 ruling for cuts, and ✅ v0.3 Q3a for a scene merge);
- every scene inside the range, and the end scene, is deleted;
- ✅ v0.3 (Q7): when the range begins at the start scene's very FIRST character, nothing of it survives, so the END scene
  survives instead (with its tail) and the start scene is deleted — the scene whose text survives keeps its identity;
- ✅ v0.3 (Q3b): each scene and chapter it deletes travels in the returned Range with its sidecar;
- every chapter left empty is deleted;
- the end chapter's scenes after the range join the start scene's chapter (v0.1 §4.4's promotion — so no duplicate chapter
  titles).
The result is the delta (P3); the Range itself becomes the clipboard's. ✅ v0.3: the result also names where its undo pastes it
back — `undoCaretSceneID` / `undoCaretByte`, the survivor's seam.
- ✅ v0.3 (Q3e): **a scene merge is a cut of the zero-length sub-range across the break** — `[(previous, its length, 0),
  (scene, 0, 0)]` — so the two texts join directly, and its undo pastes the break back with the removed scene's sidecar.

### 4.4 Paste
`scrivi_range_paste(root, range, caretSceneID, caretByte, caretSceneText)` — ONE aggregate call (R7) — and the exact inverse
of the cut:
- the caret scene is split at the caret: its head takes the first piece;
- each later piece creates a scene (scene break) or a chapter and its first scene (chapter break, with its title);
- the caret scene's tail goes onto the last created scene;
- ✅ **[I-0311]:** when the fragment opens a chapter, the caret chapter's scenes after the caret move into the last created
  chapter — the mirror of the cut's promotion — so the manuscript reads exactly as a flat insert would.
- ✅ v0.3 (Q7): **at a scene's very start** (caret 0, more than one entry) the new scenes go BEFORE the caret scene, which keeps
  its text, identity and sidecar and takes the LAST entry's text in front of its own — the mirror of the cut's Q7 rule.
- ✅ v0.3 (Q3b): a scene or chapter re-created under the Range's own ID gets its `sceneMeta` / `chapterMeta` back.
- ✅ v0.3 (Q1): **a split is a paste of an empty two-entry Range**; a chapter split, the same with the entries in different
  chapters (so the chapter is created IN PLACE — ⛔ [I-0314]: the Mac's `createChapter` appended it at the manuscript's end).
The result is the delta.
**Disk budget (R7):** a two-scene paste is **4 reads and 4 writes** (re-ruled 2026-10-20: ≤ 3 reads and 5 writes, the chapter
sidecar's list kept current; ✅ v0.3: a two-scene CUT is ≤ 5 reads — the removed scene's sidecar, Q3b — and a chapter merge of N
scenes ≤ N + 3 reads and 3N + 3 writes, no listing, no text read): the caret scene's text and sidecar, the new scene's
text and sidecar. Order keys come from the index (no listing, no sibling reads), the index is patched in place (no
re-observation), and nothing else is written (no `workspace-state.json`). The undo history's one append is the history
subsystem's, counted separately. ✅ S1's count guard holds every structural call to its budget.

### 4.4a Relabel (✅ v0.3, Q3e)
`scrivi_range_relabel(root, range, targetChapterID)` moves a run of WHOLE scenes (one chapter's, consecutive; their text is not
sent) into another chapter, keeping the reading order:
- into the chapter **before** theirs — the run must open its chapter; it is appended there (**a chapter merge**);
- into the chapter **after** — the run must close its chapter; it goes before that chapter's first scene;
- into a chapter the manuscript does not have ("" = a fresh ID) — the run must close its chapter; the chapter is created right
  after it, from the first entry's `chapterMeta` when it has one (an undo) — **a chapter split at a scene's start** (Q8).
On disk the scene files MOVE to the target chapter's folder; their sidecars are rewritten (slug, contentPath) only when their
order keys would collide there. A chapter left empty is deleted, folder and all, AFTER its scenes are out ([I-0083]), and its
sidecar travels on the result's first entry. The result is `{delta, range, fromChapterID}`: the inverse relabels `range` back
into `fromChapterID`.

### 4.5 Undo and redo
- **Undo of a paste = a cut** of exactly the range the paste inserted (its delta knows it). **Undo of a cut = a paste** of the
  cut's fragment at the cut's start. Redo is the original operation again. One history node each, as now (T3 = A).
- ✅ v0.3: **each node holds the ONE Range both its undo and its redo use** — a `rangePaste` node the Range it inserted (undo:
  cut it; redo: paste it again at its caret); a `rangeCut` node the Range it removed, sidecars included (undo: paste it at the
  survivor's seam; redo: cut it again — the undo restored it exactly); a `rangeRelabel` node the run as it is after (undo: into
  `fromChapterID`; redo: into `targetChapterID`). Split, merge, chapter split, chapter merge and paste all record one (Q3: no
  barriers). The node also carries the texts on both sides of the edit ([I-0293]).
- `scrivi_fragment_uncut_paste` retires once both platforms use the range cut; it stays readable so old `structuredPaste` /
  `structuredCut` nodes in existing history logs still undo (both platforms' logs). ✅ v0.3 ([SP-187] Q2): the apps CONVERT
  those nodes to a Range and take the range path.

### 4.6 What goes away
- The Mac's `reloadManuscriptFromDisk` and Linux's structural `reloadThen` (and SP-185's Linux-only "read only what the step
  wrote", which P3 makes unnecessary). ✅ Gone in [SP-191].
- ✅ v0.3: `scrivi_merge_scene` and `scrivi_merge_chapter` (a merge is a cut; a chapter merge a relabel), the copy's
  `fragmentExtract` (the Range is built from the list), the chapter-split confirmation (Q9).
- The per-primitive re-reads inside the core's fragment operations.
- The cross-scene "delete text, keep scenes" cut (2026-10-02) — replaced by §4.3. ⚠️ Backspace / Delete over a cross-scene
  selection: Q4.

## 5. Rulings on the questions (user, 2026-10-17)

- **Q1 — The range API → the Range (R6, §4.2).** *"It can be specified as simply a starting point and an ending point and any
  part of the system can construct it from its own data representation or from disk."*
- **Q2 — The cut's surviving end-chapter scenes → into the start scene's chapter.**
- **Q3 — A difference found by verification → the disk wins,** *"silently reconciled"* — no notice.
- **Q4 — Backspace / Delete over a cross-scene selection → the same as Cut, minus the clipboard,** *"as long as the history
  gets properly updated, then the text can be recovered with a simple undo."*
- **Q5 — The editor during a structural call → read-only until it completes, the progress row after 400 ms.** *"I am
  uncomfortable doing anything that will stop the writer from writing. However a structural operation must complete before
  the writer can resume typing."* ⚠️ So the budget in §4.4 is what keeps that pause imperceptible; S1's guards hold it.

## 6. Plan

A new Epic (proposed **EP-053 `[Cross]` Range-based structural editing**), absorbing [I-0297], [I-0311] and the remaining
structural half of SP-185. Sprints, in order — each Sprint changes ScriviCore and BOTH platforms where it touches them:

1. **S1 `[ScriviCore]` — the Range and the range operations.** `scrivi.range.v1` (C++ + JSON, fragment conversion);
   `scrivi_range_read`, `scrivi_range_paste` (with the [I-0311] fix) and `scrivi_range_cut` (structural), all returning the
   delta; order keys from the index; the index patched in place. ✅ Count guards (`CountingFileSystem`): a two-scene paste
   into a 1,186-scene manuscript makes **4 reads and 4 writes**, and every call fails its guard if a walk returns. ABI tests
   check each delta against the disk, and cut∘paste and paste∘cut restore the disk exactly.
2. **S2 `[Apple]` + `[Linux]` — the scene list as the model of record (§4.2a), the delta applied to it, no reload.** Both platforms apply results in memory; the structural call moves
   off the Mac's main thread; Linux drops the reload. Copy and cut route to the range calls on both.
3. **S3 `[Apple]` + `[Linux]` — background sync verification** (P4, Q3).
4. **S4 `[Cross]` — history:** undo of paste = cut, undo of cut = paste, on both; old nodes still undo; the Mac and Linux undo
   each other's steps (the shared log).
5. **S5 — rig and Mac live passes** on dumas-prose-timelines over the share: copy, cut, paste, undo, redo across scenes and
   chapters; each step timed by the diagnostic log ([T-0612]) and the Mac's equivalent.

✅ **Re-ordered 2026-10-20 (user), planning EP-053:** each operation ships with its undo. S2 = the scene list, copy, paste
and paste's undo (a range cut), plus split, merge and chapter create/merge (now in scope) — split the same day into S2a,
the list as the model of record with no visible change ([SP-187]), and S2b, the operations as deltas ([SP-191]); the list
stays an ordered array (a neighbour is index ±1); S3 = structural cut,
Backspace / Delete over scenes and cut's undo (a range paste) — [SP-188]; S4 = the background sync verification —
[SP-189]; S5 = the live passes — [SP-190]. Step 4 above (history) is folded into S2 and S3. New history op names
`rangePaste` / `rangeCut` are approved, backward-compatible.

Until S2 lands, SP-185's Linux fixes stay in place (they are correct, only not shared); S2 removes the Linux-only reload
reuse when both platforms stop reloading.
