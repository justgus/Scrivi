---
sprint: SP-200
epic: EP-052
status: Closed
closed: 2026-11-02
platform: ScriviCore
created: 2026-11-01
---

# SP-200 — `[ScriviCore]` [EP-052] History load: compaction and compact records

**Status:** ✅ **CLOSED 2026-11-02** (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*) — activated 2026-11-01 (user, 2026-11-01: *"activate and implement SP-200 please."*) — created 2026-11-01 (user: *"I'd like to do all three (a), (b), and
(c)"*; *"I accept your recommendation for 1, 2, and 3"*). Q1–Q4 ruled.
**Carries:** [T-0400], [T-0643], [T-0644], [T-0645], [T-0646], [T-0647] → [`../Tasks/Task-active.md`](../../Tasks/Task-active.md).
**Issue:** [I-0332] (parts 1 and (a) done in [SP-197]).
**Design:** [`../Scrivi_History_Load_Design_v0_1.md`](../../Scrivi_History_Load_Design_v0_1.md).
**Epic:** [EP-052] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).

## Goal

✅ Opening a project's undo history costs what its LIVE history costs, not what its log has accumulated: an open reads
one compacted segment, and the records that stored whole texts store changes. No change to undo or redo.

## Where it starts (2026-11-01)

dumas-prose-timelines: 34 MB log, 12,245 lines, ≈4,600 events. `scrivi_history_open` 2.05 s → 1.35 s ([I-0332] part 1) →
0.25 s ((a), `util/Json.cpp` at `-O2`). The generated 25,000-event history (`scrivi_make_test_history`) is the second
measure; its baseline is taken before T-0400 starts.

## Plan (ruled order: (c) → b1, b2 → b3)

1. **[T-0400] (c) compaction** (design §2): the store opens the segment `state.json` names; a compacted `log-(N+1)` written
   through the write queue at open and close when worth it (Q1); old segments removed after `state.json` moves.
2. **[T-0643] b1**: a diff is a list of hunks (design §3 b1, Q3).
3. **[T-0644] b2**: barrier texts stored as changes; `sceneTextsBefore` only when it differs (Q2).
4. **[T-0645] b3**: the structural payload stored as JSON.
5. **[T-0646]** measurements (dumas and the generated history, before and after each step) and live passes: the Mac and
   the rig open dumas and the generated history, undo and redo across a structural edit and a Replace All.

## Acceptance Criteria

- [x] AC1 — Q1–Q4 ruled (user, 2026-11-01: *"Your recommendations for Q1-Q4 are approved."*)
- [x] AC2 — [T-0400] An open replays one segment; compaction is crash-safe at every step of design §2.2 (tests); an old
  log opens identically and is compacted without being rewritten in place
- [x] AC3 — [T-0643], [T-0644], [T-0645] New records in the new shapes; old shapes read forever; the round-trip test of
  design §4 passes with every record kind
- [x] AC4 — [T-0646] Open time and log size recorded before and after for dumas and the generated history
- [x] AC5 — ✅ ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38; ✅ Mac live pass (user, 2026-11-01); ➡️ rig live pass → [T-0653] ([EP-057]) (rule: `Sprint-GUIDELINES.md`, 2026-11-02)
- [x] AC6 — [T-0647] The Properties tab shows the scene's links, a Project section and one World section per bound world,
  read off the main thread with no scene, object or history file opened; ✅ Mac live pass (user, 2026-11-01); ➡️ rig live
  pass → [T-0654] ([EP-057]) (rule: `Sprint-GUIDELINES.md`, 2026-11-02)

## Progress log

### 🟠 2026-11-02 — The Linux VM's first ctest found a racy [T-0400] test (fixed)
- On the new Linux VM (EP-057, Ubuntu 26.04.1 arm64), `ctest -j6` failed 803/804: *"a crash at any step of a compaction"*,
  section 1, saw `{log-000001, log-000002}` where it expected `{log-000001}`. It passed alone 3/3. Cause: the open removes the
  stale `log-000002`, then compacts the long `log-000001` again through the write queue; the check raced that queued write.
- Fix (test only): `scrivi_wait_for_writes` after the open, then check the settled state — `log-000002` active, both
  segments present (`HistoryCApiTests.cpp:1531`). Mac: `[T-0400]` 5/5, ctest `-j12` 800/800 twice. ⚠️ After the re-compaction
  the stale segment's removal is no longer observable on its own; the tree and undo checks still prove the old one was read.

### ➡️ 2026-11-02 — Rig pass moved to [EP-057] (T-0653, T-0654)
- Under the new rule (user, 2026-11-02: *"1 and 2 approved"*; `Sprint-GUIDELINES.md`, *"The rig pass never holds a
  Sprint open"*): the rig AC is carried by T-0653, T-0654, steps in [`Rig-Live-Pass-Held.md`](../Rig-Live-Pass-Held.md); the AC counts as met
  for the close. Linux evidence: the Docker ctest and smokes recorded in this log.

### 🟠 2026-11-01 — AC6 Mac live pass PASSED (rig still held)
- User: *"All steps pass."* T-0647's six Mac steps on dumas-prose-timelines.

### 🟠 2026-11-01 — T-0647 filed and Implemented - Not Verified (AC6)
- User: *"Your recommendations for the metrics are approved; file both issues … For 5. a new task in SP-200 is
  sufficient."* Both Issues are in the backlog, not this Sprint: [I-0333] (Replace All holds the window), [I-0334] (the
  main thread waits for the lock behind queued saves).
- New endpoint `scrivi_get_project_metrics`; Mac/iOS and Linux Properties tabs gain scene Links, Project and World
  sections. ctest 800/800, Mac interop 329/329, iOS 17/17, Linux smokes 38/38.

### 🟠 2026-11-01 — AC5 Mac live pass PASSED (rig still held)
- User: *"All steps pass."* dumas-prose-timelines and dumas-prose-timelines-history on the removable drive, from Xcode:
  `scrivi_history_open` 155 ms (dumas) and 1,878 ms on the generated history's first open (it compacted), 368 ms after.
- Rulings: Q3's threshold stays 4 KB; the Q1 and Q3 implementation departures are accepted; the fork-child order is not
  filed (user, 2026-11-01: *"no 4KB is sufficient. These departures are accepted."*).
- ⚠️ Once, reopening dumas after ⌘W showed an error that did not happen again; the console shows no failed call. ⏳ The
  message text.
- Seen in the log, outside this Sprint: Replace All across 1,174 scenes held the window for 6 s (dumas) and 18.6 s (the
  generated history; `apply=13427 ms`), and main-thread calls waited up to 15 s for the lock behind its saves → filed as
  [I-0333], [I-0334].

### 🟠 2026-11-01 — T-0400, T-0643, T-0644, T-0645 Implemented - Not Verified (AC2–AC4)
| History | Log | `scrivi_history_open` |
|---|---|---|
| dumas-prose-timelines | 34 MB → **18.7 MB** (compaction alone: 22.6 MB) | 0.20 s → **0.16 s** (2.05 s this morning) |
| dumas-prose-timelines-history (25,000 events) | 49 MB → **11.7 MB** (the evicted history gone) | 0.72 s → **0.33 s** (16.7 s this morning) |
- **(c)** compaction at open and close (Q1), crash-safe in the order of design §2.2; a new `ctl:root` names a promoted root.
- **b1** a diff holds several changes — **by word** (Q3 said by character; a character diff of a long Replace All passes
  its own bound). ⚠️ **Q3's 4 KB threshold reaches almost none of dumas's Replace All records** (6.86 of 6.91 MB are under
  it); 1 KB would cut them to 4.6 MB. ⏳ The user's call.
- **b2** barrier texts as changes against the history's text at the parent; equal before-texts as a name only.
- **b3** the structural payload as JSON.
- **Found on the way:** the load's two walks copied every scene's text at each fork; one shared walk restores instead of
  copying (626 → 331 ms on the generated history). Every reload orders a fork's children by eventID, so two minted in the
  same millisecond can swap in the card (not a writer's case; noted, not changed). The root anchor's timestamp is
  restamped at every open (not data).
- **Tests:** `[T-0400]` ×3, `[T-0643]` ×2, `[T-0644]`, `[T-0645]`; four mutations in copies, each caught — two only after
  the tests were strengthened (fork primaries also live in `state.json`; equal-length replacements hid the reverse shift).
  Linux's `structured_paste_smoke` reads both payload shapes.

### 🟡 2026-11-01 — Sprint ACTIVATED (user, 2026-11-01: *"activate and implement SP-200 please."*)
- T-0400, T-0643–T-0646 → active.

### ✅ 2026-11-01 — Q1–Q4 RULED as recommended

### 🔵 2026-11-01 — Baselines (before T-0400), Debug build, local disk
| History | Log | `scrivi_history_open` | One recorded event |
|---|---|---|---|
| dumas-prose-timelines | 34 MB, 12,245 lines | 0.2 s (2.05 s this morning) | ≈0 ms |
| dumas-prose-timelines-history (generated, 25,000 events) | 49 MB, 60,151 lines | 0.72 s (16.7 s this morning) | 0.3 ms (16 ms) |
- The drops are [I-0332] parts 1–4, in [SP-197]: two whole-history walks per `setPrimary` and per eviction were the
  largest cost on a long history, not the bytes. What is left grows with the log: this Sprint.

### 🔵 2026-11-01 — Sprint CREATED in Planning
- From [SP-197]'s Mac pass log: [I-0331], [I-0332]. The generated history is being built for the measurements.

*Last Updated: 2026-11-01 (T-0647 added, AC6.)*

### ✅ 2026-11-02 — Sprint CLOSED (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*)

Every AC met; the rig AC passed on the Linux VM (`Rig-Live-Pass-Checklist-2026-11-02.md`: Pass; VM counts per
Sprint-GUIDELINES rule 6). **Final status:** [T-0400], [T-0643], [T-0644], [T-0645], [T-0646], [T-0647] ✅ Verified 2026-11-02 · [T-0653], [T-0654] ✅ Verified 2026-11-02 (the rig ACs, on the VM) · [I-0331], [I-0332] ✅ Verified.

**Retrospective.**
- ✅ On the VM, after [I-0341]–[I-0344]: dumas-prose-timelines-history opens in 6.0 s over the share (82 s before); its history in 1.5 s.
- ⚠️ The history read cost 26 s over SMB until [I-0341]: the core read every file 8 KB at a time. Measured only when the history grew to 36 MB (77% Replace All records).
- ⚠️ A VM SMB connection degraded to ~1.7 MB/s and made the fixed read look unfixed; check `time cat` of a large share file before any timing pass (rig setup §9.3).
- ⛔ Further replay speed ([I-0345]: `-O2` history code, a lower Q3 threshold) — not to be done (user, 2026-11-02).
