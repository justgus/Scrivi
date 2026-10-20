---
sprint: SP-186
epic: EP-053
status: Active
platform: ScriviCore
created: 2026-10-17
---

# SP-186 — `[ScriviCore]` [EP-053] **S1**: the Range and the range operations

**Status:** 🟡 **ACTIVE** — created and activated 2026-10-17 (user: *"draft EP-053, start the first sprint"*).
**Carries:** [T-0614] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md) · [I-0311] → [`../Issues/Issue-active.md`](../Issues/Issue-active.md).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) — R2, R4, R6,
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
- [ ] AC2 — `scrivi_range_read`: only the range's texts are read; a range that does not match the index is refused.
- [ ] AC3 — `scrivi_range_paste`: a two-scene paste ≤ 4 reads + 4 writes, no listing; [I-0311]'s order is right; the delta
  matches the disk; a stale caret text is refused.
- [ ] AC4 — `scrivi_range_cut`: within its budget; head + tail joined directly; emptied chapters gone; Q2's promotion; the delta
  matches the disk.
- [ ] AC5 — cut∘paste and paste∘cut restore the disk.
- [ ] AC6 — ABI documented, parity check passes, ctest green on Linux and macOS.

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
