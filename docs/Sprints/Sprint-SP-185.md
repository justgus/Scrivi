---
sprint: SP-185
epic: EP-026
status: Active
platform: Linux
created: 2026-10-17
---

# SP-185 — `[Linux]` [EP-026] Responsive after load: the rig pass's freezes and its crash

**Status:** 🟡 **ACTIVE** — created and activated 2026-10-17 (user: *"Yes lets do them all in a new sprint."*).
**Carries:** [I-0308] (crash in the find bar), [I-0307] (select, copy and paste each freeze 10–15 s), [I-0297] (a structured
paste's undo takes 40 s over the share), [I-0309] (closing a window mid-load frees memory its worker uses; found by plan step 0)
→ [`../Issues/Issue-active.md`](../Issues/Issue-active.md) · [T-0612] (a diagnostic
log) → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-026] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). Two of the three are its structured cut, copy and
paste. [I-0308] came from [EP-048]'s find bar; it is here because the same rig pass found it, in the same session.
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
The user's understanding (2026-10-17): *"optimization may not gain us anything."* ⚠️ The measurement says otherwise for
this one. On local disk the same undo is 393 ms; over the share it is about 40 s. The difference is file calls on the
share, and most of them re-read scenes the paste did not touch. Recommended: cut those reads first, then add progress
for whatever remains.

### Q3 — Showing Replace All working (user's proposal)
*"Scrolling to each location of the word and replacing it visually while also providing a real progress indicator (since we
know exactly how many items need to be changed)."*
- ✅ A real count works: the matches are known before the first replacement ("Replacing 312 of 2,048").
- ⚠️ Scrolling to EVERY match would be slower than the work it shows. A common word has thousands of matches, and each
  scroll and repaint costs more than the replacement itself. Recommended: the view follows along a few times a second,
  showing replacements landing as the count climbs.
- ⚠️ Needs care with EP-052: today Replace All computes and saves on a worker while the editor is locked. Showing each
  replacement in the editor as it lands changes that shape. To be designed before it is built.

## Acceptance Criteria

- [ ] AC0 — [I-0308] reproduced in Docker, or the reproduction attempt recorded with what it ruled out.
- [ ] AC1 — [I-0308] fixed, with a test that fails before the fix; no crash in the rig pass's step 11.
- [ ] AC1b — [I-0309] fixed: a window closed, or the app quit, during a load does not crash; proven under ASan.
- [x] AC2 — [T-0612] the log exists as ruled in Q1, and a deliberate stall shows up in it — ✅ `diagnostic_log_smoke` (CI),
  3 mutations caught (no watchdog · typed text logged · no crash handlers). ⏳ The rig pass (AC6) still reads a real one.
- [ ] AC3 — [I-0307] measured on the rig; select, copy and paste across scenes do not freeze the window.
- [ ] AC4 — [I-0297] a structured paste's undo over the share completes as Q2 rules.
- [ ] AC5 — Q3 ruled, and built as ruled.
- [ ] AC6 — ✅ **Rig live pass (user)**

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
