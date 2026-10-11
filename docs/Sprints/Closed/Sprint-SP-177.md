---
sprint: SP-177
epic: EP-026
status: Closed
closed: 2026-10-11
activated: 2026-10-11
platform: Cross
created: 2026-10-11
---

# SP-177 — `[Cross]` [EP-026] **S1**: the undo capture rules move into ScriviCore; Apple re-pointed

**Status:** ✅ **CLOSED 2026-10-11** (user: *"no, I do not want the rule changed.  Everything behaved as it should.  That verifies the Sprint.  Close SP-177."*) — complete 2026-10-11, Mac live pass passed · ACTIVATED 2026-10-11 (user: *"close SP-176 and activate SP-177"*) — created 2026-10-11 (user: *"plan EP-026 S1"*). ✅ Q1–Q4 ruled 2026-10-11.
**Carries:** [T-0604] → [`../../Tasks/Verified/Task-verified-0604.md`](../../Tasks/Verified/Task-verified-0604.md).
**Epic:** [EP-026] `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste →
[`../../Epics/Epic-active.md`](../../Epics/Epic-active.md). ✅ EP-026 **Q1** (ruled 2026-10-11): *the capture rules move into the
core first, Apple re-pointed with no visible change, then Linux wires to them* — [SP-174]'s pattern.
**Authority:** [`../../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`](../../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md)
§4.a (event model, triggers), §4.a.1 (T-0396: autosave does not seal; 45 s idle), §4.d (disk may lead by one session), §8.
CLAUDE.md: *no backend logic in Swift*.
**Size:** M. ⚠️ Needs a Mac live pass (typing granularity is felt, not only tested).

---

## What reading the code found (2026-10-11)

The engine — tree, diff, sessions, persistence, branches — is already ScriviCore's (`HistoryService`, `scrivi_history_*` ×15).
⚠️ **What decides WHEN an undo step ends is Swift**, in two places:

| Rule | Where on Apple today |
| ---- | -------------------- |
| The pending latch: scene, full text, caret before/after | `HistoryCapture.noteEdit` (`Scrivi/App/HistoryCapture.swift:186`) |
| Seed a scene's floor on its first edit this session | `seedBaselineIfNeeded` (`:219`) |
| **45 s idle** ends a session — before folding a keystroke in, and by its own timer while the writer is away | `idleSessionThreshold`, `isSessionIdle`, `scheduleIdleCommit` (`:66–101`) |
| **Sentence boundary:** commit when the character just typed is `.` `!` `?` or a newline | `isCommitBoundary` (`Scrivi/Views/ManuscriptTextView.swift:379`) |
| **Soft trigger:** a caret move defers a pending change that is only whitespace | `flush(soft:)`, `isWhitespaceOnlyDelta` (`HistoryCapture.swift:276, 358`) |
| **Scene switch** = hard commit; caret move in the same scene = soft | `textViewDidChangeSelection` (`ManuscriptTextView.swift:1044–1060`) |
| Paste / cut: commit what was pending, then label the next event `paste` / `cut` (+ the buffer slot) | `beginPasteOrCut`, `beginCutIntoBuffer` (`:238–253`) |
| Autosave never seals — unless already idle (T-0396) | `flushThenSave` (`:378`) |
| A multi-scene edit = one event per scene sharing a fresh group id | `recordGroupedEdit` (`:332`) |
| Undo is possible while an edit is pending | `canUndoNow` (`:119`) |
| Caret offsets are UTF-8 BYTES, computed in Swift | `sceneLocalByteOffset` (`ManuscriptTextView.swift:368`) |

⚠️ Design §4.a.1 says *"Coalescing stays app-side; `HistoryService` remains pure."* — ✅ **kept:** the rules go into a NEW core
component beside `HistoryService`, which is not changed. "App-side" meant *not inside the tree engine*; it now means *not
inside any one platform*.
✅ What stays platform-side, because it is event-loop plumbing, not a rule: the "edit in flight" flag that tells an
edit-driven selection change from a real caret move; the IME marked-text guard; `isApplying` around an undo apply; the timer
that fires at the deadline the core hands back; the history card's revision counter.

## Goal

✅ **One set of capture rules, in ScriviCore**, that both platforms call — so Linux (S2) gets Apple's undo granularity by
calling it, not by porting 590 lines of Swift. ✅ **No visible change on the Mac:** the same keystrokes make the same undo steps.

## Plan

1. **Parity fixture FIRST, from Apple's Swift, before it is deleted** ([SP-174]'s method): scripted sequences of
   edit / caret move / scene switch / paste / cut / cut-into-buffer / save / idle / grouped edit / undo, each with a fake clock,
   run through today's `HistoryCapture` + `ManuscriptTextView` rules in an interop test; record the history nodes each produces
   (kind, scene, text, cursors, group, buffer) → `ScriviCore/tests/fixtures/history_capture_parity.jsonl`. Needs one seam: an
   injectable clock in `HistoryCapture` (Q1).
2. **`CaptureSession` in ScriviCore** (`ScriviCore/src/history/`), one per open project, opened and closed with its history:
   the latch, floor seeding, the idle rule, the sentence boundary, the soft whitespace deferral, scene switch vs caret move,
   paste/cut kinds and buffer tags, autosave-never-seals, grouped edits, `canUndo` including pending. It calls
   `HistoryService` as Swift does today — `HistoryService` untouched.
3. **C ABI** `scrivi_capture_*` (names provisional): note an edit (scene, full text, caret, text before, now) → committed? +
   can-undo/redo + the idle **deadline**; caret moved (scene, now); begin paste/cut (kind, buffer); before save (now); idle
   (now); flush (trigger); grouped edit (JSON); state. Undo/redo flush pending first, in the core. JSON envelopes as every
   endpoint. Caret offsets per Q2.
4. **Tests through `scrivi_*`** (`feedback_boundary_tests_not_facade`): the fixture replayed (every line equal, or an accepted
   difference named — none expected); the reference case (*"Now is the winter of our discontent…"* typed continuously = ONE
   event; a 46 s pause = two); whitespace riding into the next event; mutation-checked.
5. **Apple re-pointed:** `HistoryCapture.swift` keeps only the wrappers, the deadline timer, `isApplying`, the revision counter
   and the timestamp formatter; `isCommitBoundary` and `sceneLocalByteOffset` deleted from `ManuscriptTextView.swift`.
   EP-019's granularity interop tests (SP-093/T-0396) pass **unchanged**.
6. **Mac live pass** (user) — the reference sentence, a pause, a paste, a cut into a buffer, a cross-scene delete, undo/redo
   through them; steps in the chat reply.

## ✅ Questions — Q1–Q4 RULED 2026-10-11 (all as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | The 45 s idle clock | ✅ **RULED: the platform passes `now` with every call; the core never reads a clock or owns a timer** — it returns the idle DEADLINE, and the platform's timer calls back at it. Pure and testable with a fake clock; no thread in the core. Alternative: the core reads a monotonic clock itself (harder to test; the timer still has to be the platform's) |
| **Q2** | Caret offsets across the ABI | ✅ **RULED: UTF-16 units in** (as `scrivi_markdown_*`, [SP-174]); the core converts to the UTF-8 bytes `HistoryService` stores. Apple drops `sceneLocalByteOffset`; Linux passes `QTextCursor` offsets as they are. Alternative: keep UTF-8 bytes (each platform keeps its own conversion) |
| **Q3** | How parity is proven | ✅ **RULED: Apple-generated fixture before the Swift logic is deleted** (plan 1) **+ EP-019's existing interop tests unchanged + a Mac live pass** — the SP-174 method, which caught the emoji edge. Alternative: interop tests and live pass only |
| **Q4** | The sentence-boundary characters | ✅ **RULED (*"agreed exactly as they are"*): move them exactly as they are** (`.` `!` `?` newline — approved 2026-07-06). ⚠️ They are ASCII only: `…` `。` `？` `！` do not end an undo step today. Changing that is a behaviour change — a separate ruling, not this Sprint |

⛔ **NOT in this Sprint:** any Linux wiring (S2) · any change to `HistoryService`, `scrivi.history.v1` or the buffers ·
the fork popover, History card, stale-branch UI (S3) · structured cut/copy/paste (S4) · copy buffers (S5).

## Acceptance Criteria

- [x] Parity fixture generated from Apple's current Swift; replayed through `scrivi_*` with no unexplained difference ✅ (203 sessions, all agree)
- [x] The capture rules live in ScriviCore (`CaptureSession` + `scrivi_capture_*`); `HistoryService` unchanged ✅
- [x] Apple calls them; `isCommitBoundary`, the latch, idle and whitespace logic deleted from Swift ✅ (⚠️ `sceneLocalByteOffset` KEPT — see the log)
- [x] EP-019's granularity interop tests pass unchanged; ctest green on macOS and Linux; mutation-checked ✅
- [x] ✅ **Mac live pass (user)** — passed 2026-10-11 (steps 1–10)

---

## Progress log

### 🔵 2026-10-11 — Sprint CREATED in Planning

✅ User: *"plan EP-026 S1."* ✅ SP-177 and [T-0604] issued by `next-id.py`. ✅ Read first: `HistoryCapture.swift` (590 lines) and
its call sites in `ManuscriptTextView.swift`; design §4.a, §4.a.1, §8. The engine is already the core's; the rules for WHEN a
step ends are Swift's, in two files.

### 🔵 2026-10-11 — Q1–Q4 RULED (all as recommended) — ready to activate

✅ user: *"Q1: agreed.  Q2: agreed.  Q3: agreed.  Q4: agreed exactly as they are."* Q1 the platform passes `now`, the core returns the idle deadline · Q2 UTF-16 caret offsets in · Q3 Apple-generated
parity fixture + EP-019's tests unchanged + a Mac live pass · Q4 `.` `!` `?` newline, exactly as today.

### 🟡 2026-10-11 — Sprint ACTIVATED (user-approved)

✅ User: *"close SP-176 and activate SP-177"* ✅ Left `Sprint-backlog.md`; [T-0604] → active.

### 🟠 2026-10-11 — IMPLEMENTED ([T-0604] Implemented - Not Verified); ⏳ Mac live pass owed

✅ User: *"implement SP-177."*

**The core (plan 2–4):**
- `ScriviCore/src/history/CaptureSession.hpp/.cpp` (new) — the rules, as a pure state machine over a `CaptureSink` (the
  engine calls it makes). `HistoryService` untouched. UTF-16 → UTF-8 caret conversion and the ASCII sentence boundary (Q4) live
  here; the whitespace set is Foundation's `whitespacesAndNewlines` (Z* + U+0009–U+000D + U+0085).
- `scrivi_c_api.cpp` — a `CaptureSession` per open history (created by `scrivi_history_open`; `scrivi_history_close` commits the
  pending typing first). 12 endpoints `scrivi_capture_*`: `note_edit`, `caret_moved`, `begin`, `flush`, `before_save`, `idle`,
  `grouped_edit`, `record_barrier`, `undo`, `redo`, `note_applied`, `state` — each reports `{committed, canUndo, canRedo,
  hasPending, idleDeadlineMs}`. The record / seed / barrier / undo / redo bodies became shared helpers (no endpoint calls
  another — the project lock is not re-entrant). Direct `scrivi_history_*` pointer moves keep the session's can-undo/redo current.
  Undo/redo changes now also carry `cursorAfterUtf16` (additive).
- `scrivi.h` — the endpoints, documented; `CMakeLists.txt`.

**Parity (plan 1, Q3):** an interop generator ran **203 sessions** (3 hand-written — the T-0396 reference sentence, the same with
a 46 s pause, whitespace riding along — and 200 seeded: 14,595 steps of typing at the end and inside, emoji and accents,
backspace, caret moves, scene switches, paste, cut, cut into a buffer, saves, pauses past 45 s, multi-scene edits, barriers,
undo, redo) through Apple's Swift (`HistoryCapture` + `isCommitBoundary`, with a fake clock seam) and through
`scrivi_capture_*`. ✅ **Every session left the same undo history** — the primary spine (4,253 nodes) and the undo walk back
(3,611 steps, 29 of them multi-scene). ✅ **Mutation bites:** dropping the whitespace rule made 60 of 403 sessions differ.
→ `ScriviCore/tests/fixtures/history_capture_parity.jsonl` (**3.1 MB**, larger than SP-174's 1.1 MB: each step carries its
scene's text). Replayed by the core test "every session of Apple's parity fixture…" — ✅ red under the same mutation.
⚠️ Modelled identically on both sides, because neither is a rule: the idle TIMER, and where the caret lands after an undo (the
earliest scene). The generator was removed with the Swift it drove.

**Apple re-pointed (plan 5):**
- `HistoryCapture.swift` — reports to `scrivi_capture_*` with a monotonic `nowMs`; runs the idle timer at the returned deadline;
  keeps `isApplying`, `revision`, the read-only history calls. The latch, seeding, idle, whitespace and kind logic are gone
  (590 → 316 lines, most of what remains is the history-card and stale-branch plumbing).
- `ManuscriptTextView.swift` — `isCommitBoundary` deleted (the core decides at `note_edit`); the selection-change branch calls
  `caretMoved(editInFlight:applying:)`; grouped edits pass UTF-16 carets; the undo apply uses `cursorAfterUtf16` and reports
  `noteApplied`. ⚠️ **`sceneLocalByteOffset` KEPT:** it still feeds the History card's caret highlight (UTF-8 bytes, EP-030) —
  not a capture rule; the card is S3's.
- `ScriviEngine.swift` — `capture*` wrappers + visionOS stubs; `CaptureState`, `CaptureGroupEdit`.
- Design doc §4.a.1 and §8 amended.

**Found and fixed on the way:** ⚠️ **the visionOS build was broken since [SP-173]** — `coreLockWaits` was defined inside the
macOS/iOS half of `ScriviEngine.swift` and read by `ProjectSession` on every platform. Moved outside the split.

**Tests:** ✅ ctest **716/716** macOS, **720/720** Linux (Docker, non-root; no warnings) — 8 new capture tests incl. the
reference sentence, idle, soft whitespace, paste/cut/buffer labels, a grouped edit + UTF-16 carets, barrier attribution, close.
✅ Interop **274/274** (36 suites; EP-019's T-0396 tests unchanged). ✅ iOS + visionOS BUILD SUCCEEDED; `check-textkit2.sh`,
`check-engine-stub-parity.sh`, `check-abi-project-guards.sh` (121 endpoints) clean.
⚠️ **Not covered (live only):** the real idle timer on a real clock; how undo FEELS while typing.

### ✅ 2026-10-11 — MAC LIVE PASS: STEPS 1–10 PASS; [T-0604] VERIFIED — three observations recorded

✅ User: *"Live Pass (Mac): 1-10 passes."* ✅ A pass the user reports is the instruction to verify: [T-0604] → ✅ Verified. ⏳ Sprint close awaits approval.

The user reported three further things. None is a change this Sprint made: each is an EP-019 rule that moved into the core
unchanged — the 203-session parity run compared exactly these rules against Apple's Swift before it was deleted.
1. **Step 10 showed a confirmation** to undo past the previous session. ✅ Expected: EP-019 **AC3**, the session-boundary
   warning (once per crossing). ⚠️ **The live-pass steps omitted it** — the writer had to discover it.
2. **Some undos created branches** that had to be selected before the right change could be made. ✅ Expected: EP-019 **AC4** —
   undo then type starts a new branch; a later step that lands on the fork offers the choice. ⚠️ **The step sequence caused
   it:** steps 2 and 7 end with ⌘Z and the next step types, so each left a fork in the same scene's history.
3. **Sometimes ⌘Z had to be pressed twice** — the first moved the caret to the end of the scene, the second undid. ⏳ **Not
   reproduced yet.** It matches two existing rules, both of which make an undo step that changes nothing VISIBLE:
   - **Return is its own step** (trigger #1 commits on the newline typed, and Apple stores a paragraph break as `\n\n`).
     Undoing it removes the break and puts the caret at the end of the text before it.
   - **⌘Z commits a pending whitespace-only change as its own step.** Spaces typed after a full stop wait for real text on a
     caret move (the soft rule), but ⌘Z commits them hard ("the newest keystrokes are not discarded"). The first ⌘Z then
     removes only the trailing spaces, with the caret at the end.
   ✅ **RULED 2026-10-11 (user): no change** — *"I do not want the rule changed. Everything behaved as it should."*

### ✅ 2026-10-11 — Sprint CLOSED (user-approved); observation 3 ruled: no rule change

✅ User: *"no, I do not want the rule changed.  Everything behaved as it should.  That verifies the Sprint.  Close SP-177."* ✅ [T-0604] → `Verified/Task-verified-0604.md` (same step). Linux (S2) inherits the rules as they are.
