---
sprint: SP-189
epic: EP-053
status: Planning
platform: Cross
created: 2026-10-20
---

# SP-189 — `[Cross]` [EP-053] **S4**: the background sync verification — disk wins, silently

**Status:** 🔵 **PLANNING** — created 2026-10-20 (user: *"complete planning for EP-053"*). ⏳ Q1–Q2 below.
**Carries:** [T-0617] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-187] and [SP-191] (S2a, S2b) and [SP-188]: every structural operation returns a delta that the editor applies in memory.
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_3.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_3.md) (v0.3, 2026-10-21, supersedes v0.2) — R3,
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

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Where the comparison runs.** Comparing is not backend logic, but which fields count as "differs" is a rule both platforms must share. | ⏳ **A core endpoint returns the disk's version of the delta's scenes and chapters; each platform compares field by field** against a list the core documents. Alternative: the platform sends its view and the core returns only the differences (one rule, but the whole touched text crosses the ABI twice). |
| **Q2** | **A scene the writer typed into before the verification ran.** | ⏳ **Skip it**, and log that it was skipped. The writer's text is newer than the disk's; overwriting it would lose typing. |

## Acceptance Criteria

- [ ] AC1 — After every structural operation, a background verification reads only the delta's files, on both platforms
- [ ] AC2 — A divergence is corrected from the disk silently (Q3), logged, with the caret and scroll unchanged
- [ ] AC3 — A scene typed into meanwhile is never overwritten (Q2)
- [ ] AC4 — Tests on both: forced divergence corrected; no divergence → zero corrections; count guard; mutation-checked
- [ ] AC5 — ✅ **Live passes (user):** Mac and rig, over the share — the diagnostic logs show each verification and its cost

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-187], [SP-188], [SP-190] via `next-id.py`; [T-0617] filed.
- Q1–Q2 await rulings.
