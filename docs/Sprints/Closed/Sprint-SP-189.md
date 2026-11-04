---
sprint: SP-189
epic: EP-053
status: Closed
closed: 2026-11-04
platform: Cross
created: 2026-10-20
---

# SP-189 — `[Cross]` [EP-053] **S4**: the background sync verification — disk wins, silently

**Status:** ✅ **CLOSED 2026-11-04** (user, 2026-11-04: *"yes, backlog that don't attach it to SP-190. Then close SP-189 and SP-190."*) — activated 2026-11-04 (user: *"continue on and do the remaining implementation for EP-053"*) — created 2026-10-20 (user: *"complete planning for EP-053"*). ✅ Q1–Q2 ruled 2026-11-04 (as recommended). ✅ Built and tested on both (below); ✅ Q3 ruled 2026-11-04 (as recommended: log it, change nothing); the live passes → [`Rig-Live-Pass-Held.md`](../Rig-Live-Pass-Held.md) ([T-0661], [EP-057]; the Mac's beside it).
**Carries:** [T-0617] → [`../../Tasks/Verified/Task-verified-0617.md`](../../Tasks/Verified/Task-verified-0617.md) ✅ verified.
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Builds on:** [SP-187] and [SP-191] (S2a, S2b) and [SP-188]: every structural operation returns a delta that the editor applies in memory.
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_3.md`](../../Scrivi_Structured_CutCopyPaste_Design_v0_3.md) (v0.3, 2026-10-21, supersedes v0.2) — R3,
P4, Q3 (*"silently reconciled"*: the disk wins, no notice).
**Size:** M.

---

## What reading the code found (2026-10-20)

- Nothing does this today. After a structural step both platforms reload, so memory is the disk by construction. Once
  [SP-191] removes the reload, nothing checks that the delta the editor applied matches what the core wrote.
- `scrivi_range_read` ([SP-186]) already reads exactly a range's texts. The delta names every scene and chapter it touched.
- Both platforms already run background work with a guard: Apple `Task.detached` with `whenOffMainWritesLand`
  ([I-0305]); Linux `AsyncCall` with `beginBackgroundWork` / `endBackgroundWork`.

## Goal

✅ After every structural operation, a background task on both platforms re-reads only the files its delta names and
compares them with the in-memory list: text, order and chapter membership. Any difference is corrected from the disk
without a notice, without touching the writer's caret or typing elsewhere.

## Plan

1. **The core reads; the platform compares** (Q1). A worker calls `scrivi_range_read` (or a sibling that takes the delta's
   scene IDs) and gets the disk's version of those scenes.
2. **Compare and correct** on the main thread: a scene whose text, position or chapter differs is replaced in the list and
   re-rendered. A scene the writer has typed into since the operation is skipped (its next save wins, as today).
3. **Same threads on both** (R5): read on a worker, apply on the main thread.
4. **Diagnostics:** every correction is a line in each platform's diagnostic log, even though the writer sees none.
5. **Tests:** deliberately diverge memory from disk after a paste (test hook) and prove the correction lands; prove a scene
   typed into meanwhile is not overwritten; prove an operation with no divergence makes zero corrections and reads only the
   delta's files (count guard). Mutation-checked.

## Questions — ✅ Q1–Q2 RULED 2026-11-04 (as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Where the comparison runs.** Comparing is not backend logic, but which fields count as "differs" is a rule both platforms must share. | ✅ **RULED (user, 2026-11-04: *"Q1 and Q2 approved as recommended"*): a core endpoint returns the disk's version of the delta's scenes and chapters; each platform compares field by field** against a list the core documents. Alternative: the platform sends its view and the core returns only the differences (one rule, but the whole touched text crosses the ABI twice). |
| **Q3** | ➕ **2026-11-04 (found building it): a difference a delta cannot put right** — a scene on disk the list does not know, a chapter folder gone from under its scenes, another chapter order. Apple has no project reload; Linux's is a one-off fallback. | ✅ **RULED (user, 2026-11-04: *"Q1 and Q3 ruled as recommended"*): log it, change nothing** (as built, both platforms): these mean an outside change or a defect, and guessing would be worse. Alternative: reload the project (Linux's `load`; Apple would need one built), which re-reads everything and moves the caret. |
| **Q2** | **A scene the writer typed into before the verification ran.** | ✅ **RULED (user, 2026-11-04): skip it**, and log that it was skipped. The writer's text is newer than the disk's; overwriting it would lose typing. |

## Acceptance Criteria

- [x] AC1 — After every structural operation, a background verification reads only the delta's files, on both platforms
- [x] AC2 — A divergence is corrected from the disk silently (design Q3), logged, with the caret and scroll unchanged — a text,
      or an order within a chapter; what a delta cannot fix is logged (✅ Q3: logged, never guessed at)
- [x] AC3 — A scene typed into meanwhile is never overwritten (Q2)
- [x] AC4 — Tests on both: forced divergence corrected; no divergence → zero corrections; count guard; mutation-checked
- [x] AC5 — ✅ **Live passes (user):** ✅ the rig over the share ([T-0661] run 2, 2026-11-04); ✅ the Mac on a local disk (2026-11-04, AC re-ruled) — the logs show each verification and its cost
      `Rig-Live-Pass-Held.md` ([T-0661] for the rig; the Mac's section beside it)

## Progress log

### 🟡 2026-11-04 — Activated and built
- **Core:** `scrivi_range_verify_read` (`RangeOperations::verifyRead`, `scrivi.h`) — what the DISK holds for the files a delta
  named: each chapter's scenes in order as the ORDER-KEY FILE NAMES (one listing; no sidecar read — membership and order are
  the names, as the core resolves the manuscript), each written scene's text, the chapter order when asked. The comparison
  rule is documented at the endpoint (Q1). Paths are checked at the boundary (`manuscript/…`, no `..`); a missing file is
  `present: false`, a failed read an ERROR, never "absent".
- **Linux:** `RangeVerify.hpp` (the pure comparison) and `EditorShell::verifyAgainstDisk` — the request built BEFORE the delta
  is applied (it still knows the removed scenes' chapters), the read on a worker, the comparison on the GUI thread, the
  correction applied as a delta with the caret and scroll kept; discarded when another structural edit has started since.
- **Apple:** `RangeVerify` + `ViewportSceneLoader.verifyAgainstDisk` (`ViewportSceneLoader.swift`), the Coordinator's
  `applyCorrection` (`ManuscriptTextView.swift`) — the same rule, the same threads (R5).
- **Tests:** core — 3 ABI tests + the budget (a two-scene paste's verification = **1 listing, 2 reads, 0 writes**); Linux —
  `structural_baseline_smoke` (every operation verifies with 0 corrections; the disk changed behind the list → put right, the
  caret held; typed into meanwhile → skipped; the comparison's reorder and its reports); Apple — 4 interop tests (the same,
  through the real coordinator). Mutations: **Linux 6/6, Apple 3/3 caught** (one Apple survivor at first — NSTextView already
  keeps a caret AFTER the corrected scene — killed by testing the caret INSIDE it). Mac ctest 806/806, interop 337/337;
  scrivi-vm build 1.0.330: ctest 810/810, 39 smokes.

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-187], [SP-188], [SP-190] via `next-id.py`; [T-0617] filed.
- Q1–Q2 await rulings.

### ✅ 2026-11-04 — CLOSED
- (user, 2026-11-04: *"yes, backlog that don't attach it to SP-190. Then close SP-189 and SP-190."*). Every AC met; [T-00617] verified and archived.
