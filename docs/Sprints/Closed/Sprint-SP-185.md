---
sprint: SP-185
epic: EP-026
status: Closed
closed: 2026-10-22
activated: 2026-10-17
platform: Linux
created: 2026-10-17
---

# SP-185 — `[Linux]` [EP-026] Responsive after load: the rig pass's freezes and its crash

**Status:** ✅ **CLOSED 2026-10-22** (user: *"SP-185 can be closed (authorized)."*) · AC5 → [EP-052] (user) · 🟡 **ACTIVE** — created and activated 2026-10-17 (user: *"Yes lets do them all in a new sprint."*).
**Carries:** [I-0307] (select, copy and paste each freeze 10–15 s), [I-0297] (a structured
paste's undo takes 40 s over the share), [I-0309] (closing a window mid-load frees memory its worker uses; found by plan step 0), [I-0310] (a shown window torn down crashed)
→ [`../../Issues/Verified/`](../../Issues/Verified/) (all verified) · [T-0612] (a diagnostic
log) · [T-0613] (the find bar never freezes while the writer types) → [`../../Tasks/Verified/`](../../Tasks/Verified/) (both verified).
**Epic:** [EP-026] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). Two of the three are its structured cut, copy and
paste. [I-0308] (the find-bar crash) was carried until 2026-10-22; it is in the backlog, not reproducible (user).
**Size:** M–L. ⚠️ Every acceptance needs the **rig**: each finding showed only on the writer's 1,186-scene project over the
network share. The rig is away until 2026-10-18 evening; Docker work runs meanwhile.

---

## The standard (user, 2026-10-17)

The 2026-10-15 rule ("once loaded the app must do everything instantly") is walked back after the user spoke with a writer:
*"'instant' is not truly achievable … But we must meet the writer's expectations. She will know if her 'Replace All' operation
will be time consuming."*

- An operation the writer EXPECTS to be large (Replace All over the manuscript) may take time, if the app shows it working
  and how far it has got.
- An operation the writer expects to be small (select, copy, paste, undo of a paste, typing in the find bar) must not freeze.

## Goal

The writer never sees a frozen window after the project has loaded, and never loses the app. Where a large operation takes
time, the window shows the work being done and how much remains.

## Plan

0. **[I-0308] first: reproduce the crash in Docker.** The real dumas-prose manuscript (1,153 scenes, the Mac's probe copy),
   steps 7–11 of the rig pass replayed through a real `EditorShell`, built with AddressSanitizer so a memory error is caught
   where it happens, not when the app later falls over.
1. **[T-0612] the diagnostic log**, so the rig can tell us what the app was doing (the user: *"a simple code inspection may
   not be adequate"*). See Q1.
2. **[I-0307] measure on the rig, with the log**: select, Ctrl+C and Ctrl+V, each timed and attributed before any fix.
3. **[I-0297] cut the reload.** The 40 s is the core's index rebuild plus re-reading all 1,186 scenes after a paste touched
   two or three (the Issue's measurement). That is avoidable work, not inherent work; see Q2.
4. **Progress where the work is large** (the user's proposal, Q3).

## Questions

### Q1 — The diagnostic log
✅ **RULED 2026-10-17** (user: *"build the log as proposed."*):
- One file, `~/.local/state/scrivi/scrivi.log`, always on, kept to the last two sessions (`scrivi.log`, `scrivi.log.1`).
- Each line: time, thread, and what started or finished, with its duration (open, load, paste, undo, find, save, reload).
- A main-thread watchdog: if the window stops answering for more than 250 ms, it logs that and what was running. That is
  exactly [I-0307]'s symptom.
- Qt's own warnings and fatal messages go in it too, so an exit that is not a crash still leaves its last words.
- Help ▸ Show Log Folder opens it, so the writer can send it.

### Q2 — [I-0297]: optimise, or only show progress?
✅ **RULED 2026-10-17** (user): **cut the reads first, then progress** for whatever remains.
The user's understanding (2026-10-17): *"optimization may not gain us anything."* ⚠️ The measurement says otherwise for
this one. On local disk the same undo is 393 ms; over the share it is about 40 s. The difference is file calls on the
share, and most of them re-read scenes the paste did not touch. Recommended: cut those reads first, then add progress
for whatever remains.

### Q3 — Showing Replace All working (user's proposal)
✅ **RULED 2026-10-17** (user): **a real count, and the view follows** a few times a second, showing replacements land.
*"Scrolling to each location of the word and replacing it visually while also providing a real progress indicator (since we
know exactly how many items need to be changed)."*
- ✅ A real count works: the matches are known before the first replacement ("Replacing 312 of 2,048").
- ⚠️ Scrolling to EVERY match would be slower than the work it shows. A common word has thousands of matches, and each
  scroll and repaint costs more than the replacement itself. Recommended: the view follows along a few times a second,
  showing replacements landing as the count climbs.
- ⚠️ Needs care with EP-052: today Replace All computes and saves on a worker while the editor is locked. Showing each
  replacement in the editor as it lands changes that shape. To be designed before it is built.

## Acceptance Criteria

- [x] AC0 — [I-0308] reproduced in Docker, or the reproduction attempt recorded with what it ruled out — ✅ not reproduced;
  recorded in [I-0308] with what it rules out and what it cannot.
- ~~AC1 — [I-0308] fixed, with a test that fails before the fix; no crash in the rig pass's step 11.~~ ➡️ **Removed**
  (user, 2026-10-22): [I-0308] is in the backlog, not reproducible.
- [x] AC1b — [I-0309] fixed: a window closed, or the app quit, during a load does not crash; proven under ASan — ✅
  `background_wait_smoke` §5 fails the old code under ASan and passes the fix. ⏳ The rig pass (AC6) closes dumas mid-load.
- [x] AC2 — [T-0612] the log exists as ruled in Q1, and a deliberate stall shows up in it — ✅ `diagnostic_log_smoke` (CI),
  3 mutations caught (no watchdog · typed text logged · no crash handlers). ⏳ The rig pass (AC6) still reads a real one.
- [x] AC3 — [I-0307] measured on the rig; select, copy and paste across scenes do not freeze the window — ✅ the rig 2026-10-21 (build 75, run sheet step 9); ✅ measured and
  fixed in Docker (Ctrl+C 2,678 → 3 GUI-thread file calls; the primary selection); ⏳ the rig.
- [x] AC4 — [I-0297] a structured paste's undo over the share completes as Q2 rules — ✅ paste 5,642 → 1,890 and undo
  5,688 → 1,933 worker file calls. ➡️ **MOVED to [EP-053] S2** (user, 2026-10-20): the reload's full open (~1,700) is
  removed there (no reload), not given progress here. ✅ Met by [SP-191] (closed 2026-10-22); [I-0297] verified (user).
- ~~AC5 — Q3 ruled, and built as ruled.~~ ➡️ **MOVED to [EP-052]** (user, 2026-10-22: *"Move AC5 to EP-052"*) as its AC7. ✅ Q3 ruled 2026-10-17. ➡️ **The build WAITS FOR [EP-052]** (user, 2026-10-20):
  showing replacements land changes Replace All's locked-editor shape, which EP-052's queue replaces.
- [x] AC5b — [T-0613] typing in the find bar never waits: the search runs after a 500 ms pause, over text built ahead —
  ✅ `find_smoke` (CI), 3 mutations caught. ⏳ The rig pass (AC6) types a search in dumas.
- [x] AC6 — ✅ **Rig live pass (user)** — ✅ 2026-10-22, build 79: a window closed mid-load and a quit mid-load, both clean
  ([I-0309]); the real `scrivi.log` read ([T-0612]).

## Progress log

### 🟡 2026-10-17 — Sprint CREATED and ACTIVATED
- User: *"Yes lets do them all in a new sprint. The rig is no longer available … But you can by all means do that docker work
  now."* Issued via `next-id.py` (SP-185, T-0612).
- The "instant" rule walked back (see The standard); Q1–Q3 recorded.
- Plan step 0 started: the ASan reproduction of [I-0308] in Docker.

### 🟡 2026-10-17 — Plan step 0, first run: [I-0309] found
- The probe (scratch, not in the repo) ran under ASan on a copy of dumas-prose. Its own 120 s wait expired during the first
  load (ASan is slow), and destroying the shell then tripped a `heap-use-after-free` in the still-running load worker.
- That is a real defect, filed as [I-0309] and taken into this Sprint: a window closed during a load does the same.
- The probe's wait raised to 15 minutes; the reproduction of [I-0308] continues.

### 🟡 2026-10-17 — [T-0612] the diagnostic log built (Q1 ruled)
- User: *"build the log as proposed."*
- `DiagnosticLog` (`platforms/linux/src/DiagnosticLog.hpp/.cpp`), started in `main.cpp` and stopped after `app.exec()`:
  - `$XDG_STATE_HOME/scrivi/scrivi.log` (default `~/.local/state/scrivi/`); the previous session kept as `scrivi.log.1`;
  - one unbuffered `write()` per line: time, thread, text;
  - a watchdog thread: the GUI thread beats every 100 ms; after 250 ms without a beat it logs "not answering", what was
    running and the last input, then "still not answering" every 5 s, then "answering again, after N ms";
  - Qt's messages (chained to stderr as before);
  - fatal signals: "crashed", what was running and a backtrace (`ENABLE_EXPORTS` on `scrivi_linux`, so frames are named),
    then the default action; SIGTERM is noted; a clean quit writes "session ended (exit code N)";
  - ⛔ never the writer's text: keys only as command chords (Ctrl+C, logged as `key` lines) or "typing".
- Timed spans (a line only when ≥ 50 ms, unless marked always): the load (always) and its main-thread build; caret moved,
  the history's caret report, active scene changed, the timeline, Inspector and navigator updates, scrolling, edits, saves,
  undo, redo, a history step, a structured paste; copy, cut, paste and key presses in the manuscript; the find bar's query,
  refresh, highlights and count. The workers note start and finish with durations: save off main, Replace All, structural
  steps (and their time-outs).
- Help ▸ Show Log Folder.
- ✅ Debug build in Docker (Qt 6.4.2): smokes 36/36 (the new `diagnostic_log_smoke` included), ScriviCore ctest 728/728.
  The real app headless writes the header, Qt's message and the SIGTERM line.

### 🟡 2026-10-17 — [I-0309] fixed
- User: *"fix I-0309 while the probe runs"*. Every worker now builds its own bridge from a copy of the identity
  (`WorkerBridge`); errors and a load's progress cross a relay the worker co-owns. Six workers moved, including the Scene
  Inspector's and the Worlds dialog's (the same capture).
- ✅ `background_wait_smoke` §5: the old code fails it under ASan (the same `heap-use-after-free`), and the fix passes it.
  Debug Docker: smokes 36/36, ctest 728/728, CI script checks pass.

### 🟡 2026-10-17 — [T-0613] filed: the find bar's first-keystroke build
- The [I-0308] probe measured Linux building the whole manuscript's searchable text on the find bar's first keystroke, on the
  GUI thread: 17 s under ASan (about 1.1 s Debug, 0.3 s Release).
- User: *"I would prefer it if it worked like search on the mac … when you stop typing (say 500 ms), then it does all the
  initialization it needs to complete the search."* Planned as [T-0613].

### 🟡 2026-10-17 — [T-0613] built
- User: *"yes, build T-0613 now"*. The search runs 500 ms after the last keystroke (Return, Next, Previous and Replace run a
  pending one at once); the searchable text is built ahead after each load, in 8 ms idle slices; an action before it is
  ready waits behind "Searching…".
- Measured (Debug, Docker): 1,186 scenes built ahead over 1,132 ms, longest slice 8 ms. `find_smoke` 2 new sections, 3
  mutations caught. Smokes 36/36, ctest 728/728, CI script checks pass.

### 🟡 2026-10-17 — Q2 and Q3 ruled; the rest of the Sprint started
- User: *"build the rest of SP-185 while the probe runs"*. Q2: cut the reads, then progress. Q3: count, and the view follows.
- First step, as the plan says: attribute [I-0307] and [I-0297] by counting each phase's file calls per thread (`strace`, Docker).

### 🟡 2026-10-17 — [I-0307] fixed, [I-0297] mostly cut, [I-0310] found and fixed; [I-0308] not reproduced
- **Measured** (`strace`, Docker, 1,186 scenes, a real `EditorShell`, phases bracketed by marker probes): the cross-scene
  Ctrl+C made 2,678 GUI-thread file calls; the paste's worker 5,642 and its undo's 5,688 (see the Issues).
- **[I-0307]:** the extract takes its order from the project index (ScriviCore); the structured copy is built only for the
  clipboard, never for the primary selection (Linux). Ctrl+C: 2,678 → 3.
- **[I-0297] (Q2):** the paste and its undo find their scenes through the index, report the chapters they changed and rebuild
  only those, and Linux's reload reads only what the step wrote. Paste 5,642 → 1,890; undo 5,688 → 1,933. ⏳ Left: the reload's
  full open (~1,700).
- **[I-0310]:** the measurement harness crashed at exit (a shown shell torn down); fixed and tested.
- **[I-0308]:** the ASan replay of steps 7–11 on the real dumas-prose survived (Qt 6.4.2, offscreen). Recorded.
- ✅ ctest Linux 730/730, macOS 726/726; Linux smokes 36/36; interop 292/292 (1 on-demand skipped); 6 mutations caught across
  the new tests; the 2 that survived are recorded with why (a redundant chapter in the report; an unreachable dirty scene).

### 🟡 2026-10-17 — The structural half re-planned (user)
- After the measurements, user: *"do the design revision, answer my follow on questions above, provide a more detailed
  description … and then start planning how we're going to get out of it."*
- `docs/Scrivi_Structured_CutCopyPaste_Design_v0_2.md` (DRAFT, for ruling): range-based structural calls, the delta applied in
  memory with no reload, a background sync verification, cut structural again (reversing 2026-10-02), the same threads on
  both platforms; a proposed Epic (EP-053) absorbing [I-0297], [I-0311] and this Sprint's structural half.
- [I-0311] filed: a chapter-opening paste scrambles the reading order.

### 🟡 2026-10-20 — AC4 and AC5 re-homed (user)
- User: *"AC4 goes to EP-053 S2; AC5 waits for EP-052."*
- AC4: the paste and undo reads are cut here; the remaining reload goes away in [EP-053] S2 rather than getting progress.
- AC5: Q3 (a real count; the view follows) stands as ruled; its design and build wait for [EP-052]'s write queue.
- ⚠️ Neither AC closes in this Sprint. What remains here: AC1 ([I-0308]), AC3's rig half, AC6 (rig live pass).

### 🟡 2026-10-21 — The rig pass (build 75): AC3 met; [I-0308] did not recur

User, 2026-10-21: *"I performed the live pass listed above on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass laid out in the attachemnt. This is build 75. Step 11: Ctrl-Alt-F showed Find-and-Replace. Step 22: The Bold text is acceptable. No steps failed!"*
- the combined rig run sheet ("Scrivi Rig Live Pass", 29 steps, 2026-10-16), run on build 75 (`886b497`). It predates this Sprint, so it covers only part of AC6.
- AC3 ✅: step 9, a cross-scene copy and paste in dumas over the share, without the freeze. [I-0307] ✅ Verified →
  `Issues/Verified/Issue-verified-0301-0310.md`.
- [T-0613] ✅ Verified and archived: step 11 typed a search in dumas with no wait and no crash.
- AC1: step 11 did not crash, but [I-0308] was never reproduced or fixed, so there is no failing test. ✅ Ruled
  2026-10-21: **kept Open** (user: likely background read collisions, for an upcoming Epic). AC1 stays unmet.
- ⏳ Still to run on the rig for AC6: close dumas while it is loading ([I-0309], AC1b), and read a real
  `~/.local/state/scrivi/scrivi.log` ([T-0612], AC2).

### 🟡 2026-10-22 — [I-0308] removed from the Sprint (user)
- User: *"Remove I-0308 from SP-185 and put it in the backlog as 'not-reproducable'."*
- AC1 is removed with it. What remains here: AC6's rig steps (close dumas mid-load for [I-0309]; read a real log for
  [T-0612]).

### ✅ 2026-10-22 — The last rig steps pass; CLOSED (user)
- User: *"Move AC5 to EP-052; T-0612 and I-0310 are verified. the step in item 1 passed. SP-185 can be closed (authorized)."*
- ✅ **AC6 / [I-0309] on the rig (build 79, dumas over the share):** a window closed while the progress row showed, then a quit
  while it showed. The log has `load:` with no `load: finished`, then `session ended (exit code 0)`, for both sessions.
- ✅ **[T-0612]:** the real log, read over SSH the same day: the build line, timed load spans, each structural worker's
  phases, the watchdog's stall lines naming what was running, and a clean end to each session.
- ✅ **Verified and archived:** [I-0309], [I-0310] → `Issues/Verified/Issue-verified-0301-0310.md`; [T-0612] →
  `Tasks/Verified/Task-verified-0612.md`. Already verified: [I-0307], [T-0613] (2026-10-21), [I-0297] (with [SP-191]).
- ➡️ **AC5** (Replace All's visible progress, Q3) → [EP-052] AC7. [I-0308] left 2026-10-22 (backlog, not reproducible).

## Retrospective
- **Went well:** counting file calls per phase first (`strace`, Docker) put the freezes where they were: Ctrl+C 2,678 → 3
  GUI-thread calls, the paste and its undo ~5,600 → ~1,900, then nothing reloaded at all once [EP-053] S2b landed. The
  ASan replay found a real use-after-free ([I-0309]) while chasing a different crash, and the diagnostic log made the build 78
  fault and today's interrupted loads readable after the fact.
- **Went badly:** [I-0308] was never reproduced, so its AC could not be met and was removed; AC4 and AC5 each had to move
  to another Epic because their real fix lay in another design (no reload; the write queue).
- **Adjust:** when an AC's fix belongs to another Epic's design, move it at ruling time rather than carrying it open.
