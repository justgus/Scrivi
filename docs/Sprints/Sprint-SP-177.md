---
sprint: SP-177
epic: EP-026
status: Planning
platform: Cross
created: 2026-10-11
---

# SP-177 — `[Cross]` [EP-026] **S1**: the undo capture rules move into ScriviCore; Apple re-pointed

**Status:** 🔵 **PLANNING — ready to activate** — created 2026-10-11 (user: *"plan EP-026 S1"*). ✅ Q1–Q4 ruled 2026-10-11 (all as recommended).
**Carries:** [T-0604] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-026] `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste →
[`../Epics/Epic-active.md`](../Epics/Epic-active.md). ✅ EP-026 **Q1** (ruled 2026-10-11): *the capture rules move into the
core first, Apple re-pointed with no visible change, then Linux wires to them* — [SP-174]'s pattern.
**Authority:** [`../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`](../Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md)
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

- [ ] Parity fixture generated from Apple's current Swift; replayed through `scrivi_*` with no unexplained difference
- [ ] The capture rules live in ScriviCore (`CaptureSession` + `scrivi_capture_*`); `HistoryService` unchanged
- [ ] Apple calls them; `isCommitBoundary`, `sceneLocalByteOffset`, the latch, idle and whitespace logic deleted from Swift
- [ ] EP-019's granularity interop tests pass unchanged; ctest green on macOS and Linux; mutation-checked
- [ ] ⏳ **Mac live pass (user)** — no visible change

---

## Progress log

### 🔵 2026-10-11 — Sprint CREATED in Planning

✅ User: *"plan EP-026 S1."* ✅ SP-177 and [T-0604] issued by `next-id.py`. ✅ Read first: `HistoryCapture.swift` (590 lines) and
its call sites in `ManuscriptTextView.swift`; design §4.a, §4.a.1, §8. The engine is already the core's; the rules for WHEN a
step ends are Swift's, in two files.

### 🔵 2026-10-11 — Q1–Q4 RULED (all as recommended) — ready to activate

✅ user: *"Q1: agreed.  Q2: agreed.  Q3: agreed.  Q4: agreed exactly as they are."* Q1 the platform passes `now`, the core returns the idle deadline · Q2 UTF-16 caret offsets in · Q3 Apple-generated
parity fixture + EP-019's tests unchanged + a Mac live pass · Q4 `.` `!` `?` newline, exactly as today.
