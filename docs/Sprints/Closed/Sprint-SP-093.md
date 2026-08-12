# SP-093: `[Cross]` EP-019 — history capture granularity + presentation ✅ CLOSED

**Status:** ✅ **Closed 2026-08-11 (Human-approved)** — 11 items, all user-Verified.
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers.
**Goal:** Make the history a faithful record of how the writer actually works — coherent typing sessions
instead of arbitrary fragments, honest labels for whitespace and deletions, and a card that refreshes and
highlights correctly.
**Design:** `Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §4.a (commit triggers), §7 (ABI),
§10 Trade T2.
**Dates:** 2026-08-07 – 2026-08-11.
**Suites at close:** ctest **413/413** (from 388 at sprint start) · interop **53/53** (from 46) · macOS
BUILD SUCCEEDED. No new source files, so **no pbxproj change** was required at any point.

### Origin — the SP-092 live-verify (2026-08-07)

The user verified the "This scene only" filter and orphaned-entry deletion as working correctly, then
reported six further findings in the same session, on `the-stairs-of-tintagael.scrivi` Ch 1 Sc 4. Diagnosis
traced all six to code: three defects (I-0104/I-0105/I-0106) and three behaviour changes
(T-0396/T-0397/T-0398). **None was a regression from SP-092** — the capture-granularity behaviour dated to
SP-053; the history card merely made it visible for the first time.

**The reference case.** The writer typed one continuous sentence — *"Now is the winter of our discontent made
glorious summer by this son of york"* — and history recorded it as **three** entries, one break falling
**mid-word** (`"…made glo"` / `"rious…"`), the signature of a wall-clock timer rather than any intentional
boundary. Cause: the 1 s autosave debounce calling `flushThenSave()`.

### Items closed — 11, all Verified

| # | ID | Title | Verified |
| - | -- | ----- | -------- |
| 1 | I-0104 | `externalChange` fires every open — head hash over replayed text, not disk bytes | 2026-08-11 (EC1, two launches) |
| 2 | I-0105 | History card doesn't refresh on commit | 2026-08-10 (EC6) |
| 3 | T-0396 | Typing-session coalescing — deferred save-commit + 45 s idle timer | 2026-08-10 (EC2–EC5) |
| 4 | I-0106 | Wrong entry bolded — caret-at-boundary + deletions match two rows | 2026-08-10 (EC7/EC9) |
| 5 | T-0398 | Distinguish added vs. deleted text in history rows | 2026-08-10 (EC10) |
| 6 | T-0397 | Whitespace-kind labels instead of "(no text)" | 2026-08-10 (EC11) |
| 7 | I-0107 | Caret highlight uses stale offsets — hit zones drift; zero-span rows unreachable | 2026-08-10 |
| 8 | I-0108 | Stale branches listed project-wide under "This scene only" — badge only; purge → Project Settings | 2026-08-11 |
| 9 | I-0109 | Navigator: Return does not open the arrow-key-selected scene | 2026-08-11 |
| 10 | I-0110 | **History fails to open** — `unknown node` when a replayed purge contains the current node | 2026-08-11 |
| 11 | I-0111 | Prune-driven purges written with `seq 1` (diagnostic only) | 2026-08-11 (approved on test evidence) |

### ⚠️ §4.d was relaxed — a changed invariant, user-approved

The sprint plan said "record the pending text so disk is covered, and the entry **stays open** for
continuation". **That is not implementable:** `HistoryService::record` always appends a node and diffs against
the current head (`HistoryService.cpp:204-206`) — there is no amend/extend path — so a save that records
anything necessarily creates a node, and undo walks one node per step. The writer would have seen one row but
three ⌘Z stops.

**Ruled (user, 2026-08-10): defer the commit.** An autosave mid-session writes the file and records
**nothing**; the entry commits once at a real boundary (idle, any kept AC2 trigger, or close). One node, one
undo step, one row.

**Consequence:** the invariant "disk never contains text no history node describes" no longer holds strictly —
**disk may lead history by at most one save's worth of typing while a session is open.** Bounded: `close()`
commits pending text, so a clean quit loses nothing; the only exposed window is a hard crash mid-session,
where I-0104's head-hash check raises an `externalChange` barrier. Failure mode is *"undo stops early"*, never
*"undo corrupts the manuscript"*.

**Documented by T-0217 (2026-08-11)** in design §4.a/§4.a.1 + §4.d + §12.2/§15 AC2, `Epic-active.md` AC2,
package-structure §16a, and repair-matrix §6.21. **The design change was explicitly user-approved 2026-08-11**,
separately from EP-019's close.

### Implementation notes

- `flushThenSave()` no longer flushes mid-session (commits only if already idle); a **45 s** `idleTask` seals
  the session on its own, so a writer who stops mid-sentence and walks away still gets their entry;
  `noteEdit` commits first if the gap already exceeded the threshold.
- **Backspace does not commit** — an ordinary edit into the open latch, so intra-word correction stays
  in-session.
- **All kept triggers untouched** — `cursorMove`/`sentence`/`paste`/`cut`/`sceneSwitch` still call `flush` at
  their own call sites (`ManuscriptTextView.swift:638-806`).
- **I-0108 amended T-0366**, which specified stale badges + user-confirmed purge *in the card*. Purge is now
  Project-Settings-only — one irreversible operation, one entry point. Recorded rather than done quietly,
  since T-0366 was already user-Verified.

---

## The finding that matters for how later sprints are run

**Five of eleven items — I-0107 and I-0108 through I-0111 — were found by live verification, not by CI.** Two
were severe: I-0110 stopped history opening at all (the project ran with no undo/redo), and I-0104's real
cause survived four wrong hypotheses and two shipped-then-reverted fixes, one of which made the symptom
measurably worse for the writer.

**Why the tests kept agreeing.** Three separate times a test passed while testing nothing: the I-0107 rebase
tests only ever inserted at offset 0 (always "before", the one shape the naive rule gets right); the I-0104
tests all called `scrivi_history_close`, which *was* the missing fix, so they could not fail; the first
I-0111 test left its bogus node unparented, so the prune never reached it. **Each suite exercised the shape
its author already had in mind.**

**What actually resolved it:** an instrumented probe driving the real C ABI against a **copy** of the writer's
project. It answered in one run what four rounds of reasoning had not. **Reach for real-data instrumentation
earlier**, especially when a hypothesis needs a test that cannot fail.

**Lesson:** when a fix turns on a variable (here, *where* a later edit happened), the tests must **vary that
variable**, not hold it constant. I-0106 fixed the tie-break but not offset drift; the first I-0107 fix
addressed drift with a rule that was wrong for appending — the most common editing shape. Both passed their
tests. Green tests certified a broken rule twice.

**Coverage gap carried forward:** `HistoryCapture` is not compiled into the test target, so T-0396's 45 s idle
boundary, the deferred save, and backspace-does-not-split have **no automated coverage** — they rest entirely
on the live verification performed 2026-08-10/11 (EP-019 AC2 items 5–8, user-confirmed 2026-08-11).
