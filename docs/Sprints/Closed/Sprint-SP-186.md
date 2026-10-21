---
sprint: SP-186
epic: EP-053
status: Closed
closed: 2026-10-20
activated: 2026-10-17
platform: ScriviCore
created: 2026-10-17
---

# SP-186 — `[ScriviCore]` [EP-053] **S1**: the Range and the range operations

**Status:** ✅ **CLOSED 2026-10-20** (user: *"mark T-0614 as verified and close SP-186"*) · created and ACTIVATED 2026-10-17 (user: *"draft EP-053, start the first sprint"*).
**Carries:** [T-0614] → [`../../Tasks/Verified/Task-verified-0614.md`](../../Tasks/Verified/Task-verified-0614.md) · [I-0311] → [`../../Issues/Issue-active.md`](../../Issues/Issue-active.md).
**Epic:** [EP-053] → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Authority:** [`../../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) — R2, R4, R6,
R7, §4.2–4.5, Q2, Q4 (all ruled 2026-10-17).
**Size:** L. ScriviCore only: the platforms keep today's endpoints until S2 switches both to these together. No rig needed
(file-call counts are exact in tests; the live passes are S5).

---

## What reading the core found (2026-10-17)

- **`SceneWriter::save`** writes the text, READS the sidecar, writes the sidecar, then writes `workspace-state.json` — 1 read,
  3 writes per scene saved.
- **`SceneCreator::create`** reads `manuscript.meta.json` and the chapter's sidecar, LISTS the chapter and reads every scene
  sidecar in it (to find the neighbours' order keys on disk), writes an empty text and a sidecar.
- So a pasted scene is created empty, then saved: about 25 reads and 5 writes, before the index refresh re-reads the chapter
  and the chapter list (measured: 184 file calls for a two-scene paste into a 20-scene chapter).
- The project index (`ProjectIndex`) already holds every scene's ID, chapter, title, paths and position in order — everything
  the order keys and the links need — in memory.

## Goal

ScriviCore has the Range (`scrivi.range.v1`) and three aggregate operations — read, paste, cut — that each make one pass over
exactly the files in their range, return the full change (the delta), and keep the index current without re-reading it. A
two-scene paste into a 1,186-scene manuscript is **4 reads and 4 writes** or fewer (R7).

## Plan

0. **The guards first, red.** `CountingFileSystem` tests at the operation level on a 1,186-scene project: a two-scene paste
   must make ≤ 4 reads and ≤ 4 writes and no listing; a two-scene cut likewise; a range read reads only its scenes' texts. They
   fail against today's `FragmentPaster` (measured 184) — the proof they can see the cost.
1. **The Range** — `scrivi::manuscript::Range`: a doubly linked list of scene entries (`sceneID`, `chapterID`,
   `chapterTitle`, `sceneTitle`, `start`, `length`, `text`, `prev`, `next`; R6); `scrivi.range.v1` JSON (an ordered array —
   the links are its order); one-to-one conversion with `scrivi.fragment.v1`, so buffers and history nodes keep working.
   Offsets are UTF-8 bytes (design §4.2).
2. **`scrivi_range_read(root, startScene, startByte, endScene, endByte)`** — walks the index from start to end (no search),
   reads only those scenes' texts.
3. **`scrivi_range_paste(root, range, caretSceneID, caretByte, caretSceneText)`** — ONE aggregate pass (design §4.4):
   - the caret scene's text is read once and must equal what the platform sent (else refused — a stale editor never writes);
   - order keys for new scenes and chapters come from the index's neighbours (no listing, no sibling reads);
   - each file is written once, with its final content (no empty create, no second save, no `workspace-state.json`);
   - ✅ [I-0311]: when the range opens a chapter, the caret chapter's scenes after the caret move into the last created
     chapter;
   - the index is patched in place from what was written; the result is the delta (design P3).
4. **`scrivi_range_cut(root, range)`** — structural (R4, design §4.3): the start scene's head joined directly to the end
   scene's tail; the scenes inside and the end scene deleted; emptied chapters deleted; the end chapter's later scenes into
   the start scene's chapter (Q2); the index patched; the delta. Backspace / Delete over scenes use the same call (Q4).
5. **Inverses:** cut then paste of its Range restores the disk byte for byte (files, order keys aside, names); paste then cut
   of the same Range likewise.
6. **ABI exposure:** `scrivi.h`, the ABI audit doc's endpoint count, and the Apple engine stub parity
   (`check-engine-stub-parity.sh`); ctest on Linux and macOS.

## Acceptance Criteria

- [x] AC0 — the count guards exist and FAIL against today's operations (plan step 0) — ✅ the paste guard: **47 reads, 2
  listings, 11 writes** against a budget of 4 / 0 / 4 (today's `FragmentPaster` with SP-185's index lookup, 120 scenes). The
  cut guard follows `scrivi_range_cut` (today's cross-scene cut is the platforms' per-scene deletes).
- [x] AC1 — the Range: C++ type, `scrivi.range.v1` round trip, fragment conversion both ways — ✅ `Range.hpp/.cpp`
  (`std::list` of `RangeEntry`), 2 tests / 37 assertions: the round trip, contiguity refused when broken, a length that is not
  its text refused, the fragment conversion both ways.
- [x] AC2 — `scrivi_range_read`: only the range's texts are read; a range that does not match the index is refused — ✅ a
  five-scene read across a chapter boundary makes exactly 5 reads, no listing; an end before its start, an offset past a
  scene's end and an unknown scene are refused.
- [x] AC3 — `scrivi_range_paste`: a two-scene paste ✅ **≤ 3 reads + 5 writes** (re-ruled 2026-10-20, see below), no listing
  — measured **3 / 0 / 5** (1 probe); [I-0311]'s order is right (the whole manuscript reads as the old text with the range
  inserted at the caret); the delta matches the disk; a stale caret text is refused and writes nothing.
- [x] AC4 — `scrivi_range_cut`: within its budget — measured **4 reads / 0 listings / 5 writes** for a two-scene cut
  (⚠️ the cut's budget was not ruled; it follows the paste ruling's rule: the chapter sidecar's list kept current); head +
  tail joined directly; emptied chapters gone (folder and `manuscript.meta.json` entry); Q2's promotion; the delta matches
  the disk; a Range that skips a scene, or whose text is not on disk, is refused.
- [x] AC5 — cut∘paste and paste∘cut restore the disk — ✅ chapter IDs and titles, scene IDs and titles, every text (order
  keys and file names aside), each followed by a clean `scrivi_scan_for_external_changes`.
- [x] AC6 — ABI documented, parity check passes, ctest green on Linux and macOS — ✅ `scrivi.h` documents the three endpoints
  and the delta; `check-engine-stub-parity.sh` passes (no engine method added: S2 adds them); ctest **macOS 741/741**,
  **Linux 745/745** (Docker, Debug, non-root); the Mac app builds. ⚠️ `Scrivi_ABI_Binding_Gap_Audit_v0_1.md` was NOT
  updated: it is a dated audit of 100 endpoints, and `scrivi.h` now declares 135 — a one-line bump would contradict its own
  table. Recorded for an audit refresh.

## Progress log

### 🟡 2026-10-17 — Sprint CREATED and ACTIVATED
- User: *"This should give you all the rulings you need. draft EP-053, start the first sprint and publish the design doc for my
  perusal while you are doing your thing."* Issued via `next-id.py` (EP-053, SP-186, T-0614).
- Design v0.2 updated with R6 (the Range), R7 (the linked list, 4 reads + 4 writes) and Q1–Q5; published for reading.

### 🟡 2026-10-17 — Steps 0 and 1
- **Step 0 (AC0):** `RangeOperationBudgetTests.cpp` (6 chapters × 20 scenes, every file call counted). A two-scene paste through
  today's best path (`FragmentPaster` + the index lookup) made **47 reads, 2 listings, 11 writes**: red against R7's 4 / 0 / 4.
  The hottest files: the target's and the new scene's sidecars (4 each), the chapter sidecar (3), the chapter folder listed
  twice, every sibling sidecar read twice.
- **Step 1 (AC1):** the Range — `scrivi::manuscript::Range`, a `std::list` (the doubly linked list) of `RangeEntry`; `scrivi.range.v1`;
  conversion with `scrivi.fragment.v1`. ⚠️ Two decisions recorded in the header: offsets in UTF-8 bytes, and one field beyond
  R6's list, `sceneLength`, without which an entry cannot say it covers its whole scene.
- Next: step 2 (`scrivi_range_read`), then the aggregate paste.

### 🟡 2026-10-20 — Steps 2–6: the range operations built (two rulings on the way)
- User: *"complete the implementation of SP-186"*.
- ✅ **Two rulings (user, 2026-10-20), asked before the operations were written:**
  1. **The budget: 3 reads + 5 writes** for a two-scene paste. R7's 4 + 4 left out the chapter sidecar's `scenes[]` list; the
     validator and `scanForExternalChanges` (called by both apps) still read it, and a stale one reports a cut scene as a
     BLOCKING *"Missing scene.meta.json"*. It is now written once per changed chapter, from the index.
  2. **Scene identity: reuse an absent ID; no prune on cut.** A paste re-creates a scene (and a chapter) under the Range's own
     ID when the manuscript does not have it — a cut — and mints a fresh one when it does — a copy. A cut deletes the files but
     not the scene's relationship edges, so its undo, or a paste elsewhere (a move), brings them back. ⚠️ Only identity and
     titles travel: a re-created scene's sidecar is new (status, story time, tags, outline and todo are not in a Range).
- **Built:** `RangeOperations` (`ScriviCore/src/manuscript/RangeOperations.hpp/.cpp`) — read, paste, cut; every location,
  order key and title from the index, every file written once, no `workspace-state.json`; a new chapter's folder (and a new
  scene's sidecar in an existing chapter) PROBED first, because a chapter with no scenes is invisible to the index.
  `ProjectIndex::insertScenesAfter`, `removeScenes`, `relocateScene` patch the index in place. C ABI: `scrivi_range_read`
  (read guard, served from the cached index), `scrivi_range_paste`, `scrivi_range_cut` (write guard; the index is TAKEN,
  patched and put back only on success — any other ending drops it).
- **The delta** (design P3): each scene `created` / `rewritten` / `moved`, with its chapter, paths and the scene it now
  follows; removed scenes; created chapters with the chapter they follow; removed chapters. The paste also returns the Range
  it INSERTED under the real IDs (its undo cuts exactly that); the cut returns the Range it removed.
- ⚠️ **Found while testing — a cut's own ordering:** promoting the end chapter's scenes BEFORE deleting the cut scenes
  collided on a file name (keys are unique only within a chapter). The deletes now run first.
- ⚠️ **Found while testing — [I-0312] filed (pre-existing, not this Sprint's):** `util::keyBetween` renders an undotted
  legacy key as a dotted one that sorts BEFORE it (`keyBetween("004", "005")` → `"0.0.4.H"`, which byte-sorts before
  `"004"`). `SceneCreator` has the same exposure. The test fixtures now use generated-shape keys, as the app writes them.
- **Tests:** `RangeOperationBudgetTests.cpp` (the paste guard now drives the range paste; a cut guard and a read guard
  added) and `RangeOperationCApiTests.cpp` (11 cases through `scrivi_range_*`: read; in-chapter paste; chapter-opening paste
  as a flat insert [I-0311]; stale refusal; in-chapter cut; cross-chapter cut with Q2; both inverses; refusals; the patched
  index answering as a fresh one). ✅ **10 mutations, 10 caught** (chapter sidecar not written ×2, [I-0311] move off, ID
  reuse off, stale caret accepted, P1 order check off, the cut's text check off, the index patched at the wrong place, moved
  scenes not relocated in the index, a removed chapter left in `manuscript.meta.json`). The text-check mutation first
  SURVIVED — the stale edit changed the scene's length, so the length check caught it — and the test now edits in place.
- ✅ ctest macOS 741/741, Linux 745/745 (Docker, Debug, non-root); repo checks pass; the Mac app builds. ⏳ Neither app calls
  the endpoints yet — that is [SP-187] — so there is no live pass for this Sprint; verification is the user's review.

### ✅ 2026-10-20 — Sprint CLOSED (user)
- User: *"mark T-0614 as verified and close SP-186."* T-0614 → ✅ Verified, archived.
- [I-0311] stays open, moved to [SP-187]: fixed in `scrivi_range_paste`, but both apps still call the old paste until S2.
- [I-0312] (found here) stays in the Issue backlog, unassigned.

