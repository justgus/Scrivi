# EP-052: `[ScriviCore]` **Background Writes** — one ordered write queue per project, in the core

**Status:** ✅ **CLOSED 2026-11-02** (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*) — ✅ all ACs met; Sprints [SP-194]–[SP-197], [SP-200], [SP-201] closed; ✅ Audit Check run at the close (findings below). — activated 2026-10-26 (user, 2026-10-26: *"close SP-188 and activate EP-052."*) — created 2026-10-15 (user: *"A new Epic."*, ruling Q5 of the design). S1 [SP-194] ✅ closed 2026-10-26 (AC0–AC3). S2a [SP-195] 🟡 active 2026-10-26; S2b [SP-196] 🟡 active 2026-10-27 (beside S2a, whose rig pass is open). S3 [SP-197] 🟡 active 2026-10-27.
**Design:** [`../Scrivi_Background_Writes_Design_v0_1.md`](../../Scrivi_Background_Writes_Design_v0_1.md) — ✅ Q1–Q5 ruled
2026-10-15. **Builds on:** `Scrivi_Core_Concurrency_Design_v0_1.md` (SP-173's per-project lock).
✅ **v0.2 RULED 2026-10-26** (user: *"Q1–Q7 agreed as recommended"*): [`../Scrivi_Background_Writes_Design_v0_2.md`](../../Scrivi_Background_Writes_Design_v0_2.md) —
the main thread is for the screen: the main-thread inventory (Mac + Linux), two locks per project (memory and disk),
reads off the main thread, Spotlight in the background. Q4 (objects cached in memory) and Q6 (level 5) are not now.
**Codebase:** `[ScriviCore]` is home (the queue, the endpoints, the ordering rule). Apple and Linux adopt it in the SAME
Sprint (`feedback_linux_adopts_apple_shape`), each AC verifiable on its own platform.
⚠️ `Epic-GUIDELINES.md` prefers one Epic per platform; the user ruled ONE Epic (Q5), so the two adoptions are separate ACs.

**Goal:** no write to a project, including undo, redo, Replace All, structural operations and the typing autosave, runs on
the GUI thread. Every write to one project lands in the order it was made, by ONE rule in ScriviCore, and the editor never
locks to wait for a save.

**Why:** undoing a large Replace All writes every scene on the GUI thread (SP-181's finding); every typing autosave is a
network write on the GUI thread over the share; and the two platforms each keep their own rule to stop an older write landing
over a newer one, which have already drifted once ([I-0304] against [I-0259]).

**Acceptance Criteria:**
- [x] **AC0 — `[ScriviCore]` Two locks per project (v0.2 Q1):** a memory lock (M) and a disk lock (D) replace SP-173's
  single exclusive lock; D before M, never M before D; an endpoint that touches no file takes M only.
- [x] **AC1 — `[ScriviCore]` The queue (W1, W3):** one writer thread per open project, first-in-first-out, coalescing a scene
  queued again before its write starts; every synchronous endpoint that reads or writes scene files first waits for the
  jobs queued before it; `scrivi_close_project` drains it.
- [x] **AC2 — `[ScriviCore]` The endpoints (W2, W4, W7):** queue scene saves and structural operations with a ticket; the
  history log's appends (undo, redo, capture) join the same queue; the core tells history the persisted bytes itself;
  `scrivi_wait_for_writes` reports every outcome, a failure included.
- [x] **AC3 — `[ScriviCore]` Tests:** boundary tests through `scrivi_*` (order, coalescing, read-after-queue, failure, close
  drains) and a ThreadSanitizer stress run on macOS and Linux.
- [x] **AC4 — `[Apple]` Adoption:** autosave, Replace All, undo and redo writes, and structural operations go through the
  queue; no project write on the main thread; the manuscript does not lock for a save (Q3); a failed write leaves its scene
  dirty; close and quit wait ([I-0305]'s dialog). W6's platform-side ordering code is removed. — ✅ [SP-195] (saves), [SP-196] (structural edits), [SP-201] (Replace All); Mac passes and [T-0648]/[T-0649]/[T-0655].
- [x] **AC5 — `[Linux]` Adoption:** the same, in the same Sprint as AC4. — ✅ the same Sprints, Linux halves; [T-0648], [T-0649], [T-0655] passed on the VM 2026-11-02.
- [x] **AC6 — Rig:** undoing a Replace All on dumas over the share does not freeze the window, and typing never waits on a save. ➡️ Carried by
  [EP-057] ([T-0648]–[T-0650], [T-0653]–[T-0655]) under the rig rule (user, 2026-11-02); met for this Epic's close once its Sprints close. — ✅ [T-0648]–[T-0650], [T-0653]–[T-0655] passed on the Linux VM 2026-11-02.
- [x] **AC7 — `[Linux]` Replace All shows its progress** (moved from [SP-185] AC5, user 2026-10-22: *"Move AC5 to EP-052"*):
  [SP-185] Q3 as ruled 2026-10-17 — a real count (*"Replacing 312 of 2,048"*), and the view follows a few times a second,
  showing replacements land. To be designed against the queue (it changes Replace All's locked-editor shape). — ✅ [SP-195] AC4; [T-0648] step 2 on the VM.
- [x] **AC8 — `[Apple]` + `[Linux]` Reads off the main thread (v0.2 Q2, R3):** the Inspector and the timeline load as
  one read job each; a result whose revision moved is dropped; a click, select or scroll makes no file operation on the
  main thread (the inventory probes read 0). — ✅ [SP-197] ([T-0629], [T-0630]); [T-0650] on the VM.
- [x] **AC9 — `[Apple]` Spotlight in the background (v0.2 Q3):** one serial queue, per-scene re-index, behind the saves;
  no Spotlight work on the main thread at open or on app switch. — ✅ [SP-197] ([T-0631]), Mac pass 2026-11-01.
- [x] **AC10 — Gesture cues (v0.2 Q5):** every queued gesture shows a cue at once; a repeat is ignored while one is
  pending; ⌘Z waits its turn and never cancels a write. — ✅ [SP-196] ([T-0627]); [T-0649] on the VM.
- [x] **AC11 — `[ScriviCore]` The history opens at the cost of its live history** (user, 2026-11-01, from [SP-197]'s Mac pass
  log; [I-0332]): an open reads one compacted segment, and records store changes, not whole texts →
  [SP-200] (✅ closed 2026-11-02), design [`../Scrivi_History_Load_Design_v0_1.md`](../../Scrivi_History_Load_Design_v0_1.md). — ✅ [SP-200]; [T-0653] on the VM — and the load 82 → 6 s after [I-0341]–[I-0344].
**Rough span (v0.2 Q7):** 4 Sprints (S2 split in two, ruled 2026-10-26, [SP-195] Q2): S1 ScriviCore (AC0–AC3), S2a every save queued (AC4–AC7), S2b structural edits queued + cues (AC4, AC5, AC10),
S3 Apple + Linux reads off the main thread + Spotlight (AC8, AC9). Each ends with a live pass over the loop share.
**Also under this Epic:** [SP-201] (✅ closed 2026-11-02) — [I-0333] Replace All holds the window, [I-0334] main-thread lock waits
behind queued saves (AC4, AC6, AC8).
**Backlog, not yet an AC:** [T-0628] keep a held queue on local disk and push it when the project returns (user, 2026-10-26).
**Out of scope:** world writes (concurrency design D6), two processes on one project, the speed of the structural
operations themselves.

**Audit Check (2026-11-02, at the close — read-only greps):** (1) AC4–AC11 were unticked here though their Sprints met
them, and SP-200 / SP-201 still read 🟡 Active → ticked with their evidence above; (2) `Task-Documentation.md`'s Sprint
column still read 🟡 for SP-195–SP-201 → corrected; (3) `Epic-Documentation.md` still read 🟡 ACTIVE → closed; (4) [I-0259]'s
archive entry is a table row with no details section (the older format) — evidence present, no action. Every Task cited
here has its `Task-verified-XXXX.md`; every Issue its archive row.
