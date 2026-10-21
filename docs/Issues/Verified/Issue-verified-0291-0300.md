# Verified Issues: I-0291 – I-0300

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0281–I-0290 are in [`Issue-verified-0281-0290.md`](Issue-verified-0281-0290.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0291** | `[Apple]` ⚠️ **The Buffers palette's button switched to PASTE on ⌃ alone after paste moved to ⌃⌘ ([I-0290]).** Reported by the user 2026-10-12. ✅ Fixed: it follows ⌃⌘, as the chord does. → [details](#i-0291) | **Low** | [SP-179] | ✅ **VERIFIED 2026-10-12** |
| **I-0292** | `[Build]` ⛔ **Scrivi Linux App CI red on every push since SP-176: `find_smoke`'s timing check (< 5000 ms) ran against an UNOPTIMIZED build (6.7–6.8 s).** The workflow configured no build type (GCC -O0); the rig and Docker build Release. ✅ Fixed: CI builds Release. → [details](#i-0292) | **Medium** | — (unassigned) | ✅ **VERIFIED 2026-10-13** |
| **I-0293** | `[ScriviCore]` ⛔ **After a STRUCTURED (cross-scene) paste, undoing the next edit in the target scene restores its PRE-paste text — the text after the caret then appears twice, and the paste itself can no longer be undone.** Proven 2026-10-13 through `scrivi_*` ([SP-180] Q3). ✅ Fixed: a structural node carries the texts on both sides of its op; Apple sends them. → [details](#i-0293) | **High** | [SP-180] | ✅ **VERIFIED 2026-10-15** |
| **I-0294** | `[Apple]` ⚠️ **A structured (cross-scene) ⌘V builds on the target scene AS SAVED: typing not yet autosaved is dropped by the reload.** Found 2026-10-13 by [I-0293]'s interop test. ✅ Fixed: every scene with unsaved typing is saved before the paste. → [details](#i-0294) | **Medium** | [SP-180] | ✅ **VERIFIED 2026-10-15** |
| **I-0295** | `[Apple]` ⚠️ **A structured paste landed BEFORE a selection and left the selection in place; a flat paste replaces it.** Proven 2026-10-13 ([SP-180] Q2). ✅ Fixed: the selection is removed first, as its own step. → [details](#i-0295) | **Low** | [SP-180] | ✅ **VERIFIED 2026-10-15** |
| **I-0296** | `[Apple]` ⛔ **After a structured paste, the next open takes the target scene for an OUTSIDE edit — undo stops there.** The core wrote the scene, so the save path never reported its bytes. Proven 2026-10-13 (interop + Linux smoke). ✅ Fixed in the core: the fragment endpoints report what they write. → [details](#i-0296) | **Medium** | [SP-180] | ✅ **VERIFIED 2026-10-15** |
| **I-0299** | `[ScriviCore]` ⛔ **A reopen on the Mac took scenes written by Linux over the network share — and two scenes the Mac itself had pasted into and deleted from — for OUTSIDE edits.** Found 2026-10-13 ([SP-180] live pass, step 39). ✅ PROVEN 2026-10-14: a session that ends without closing its history (a force-quit, Xcode's Stop) lost the bytes it recorded. ✅ Fixed: each save's bytes are recorded in the log as they happen. → [details](#i-0299) | **Medium** | [SP-180] | ✅ **VERIFIED 2026-10-15** |
| **I-0298** | `[Apple]` `[Linux]` ⚠️ **An undo that crosses a structural step (a cross-scene paste) into an earlier session shows no "Undoing changes from a previous session" warning.** Found 2026-10-13 ([SP-180] live pass, step 36). ✅ Fixed on both platforms. → [details](#i-0298) | **Low** | [SP-180] | ✅ **VERIFIED 2026-10-21** |
| **I-0300** | `[Linux]` ⚠️ **After Ctrl+Shift+Return (new chapter) — and a mid-scene Ctrl+Return — the caret stays where it was instead of moving into the new chapter or scene.** Found on the rig 2026-10-13 ([SP-180] live pass, step 24). ✅ Fixed: the caret is placed when the reload has finished (also after a merge). → [details](#i-0300) | **Low** | [SP-180] | ✅ **VERIFIED 2026-10-21** |

---

## I-0291

**Title:** The Buffers palette's button switches to paste on ⌃ alone, not ⌃⌘
**Status:** ✅ **VERIFIED 2026-10-12** (user, Mac: *"I-0291 passes"* — the four steps below, including the ⌃⌘-click paste).
**Platform:** `[Apple]`
**Component:** `BuffersPalette.swift` `BufferAction.current`
**Severity:** Low — the palette's paste was reachable, just on a modifier that no longer matches the chord
**Sprint:** [SP-179]
**Date Identified:** 2026-10-12 — user, on the Mac: *"I-0290 passes. But the view buffers icon changes on holding Ctrl down, not
Ctrl-Cmd."*

**Root cause:** my choice in [I-0290] — I kept the palette on ⌃ alone (*"a click, not a chord"*) and said so; it does not match
the chord the hint names.

**Fix:** `BufferAction.current` returns paste only for ⌃ AND ⌘; ⌃ alone → copy. Hint: *"hold ⌃⌘ or ⌥ to change the button"*.
`BufferAction` made internal (was private) so a test can reach it.

**Tests:** `BufferPasteChordTests.paletteButtonFollowsTheChord` — FAILED on the old code (⌃ alone → paste), passes now. Interop
284/284. ✅ A ⌃⌘-CLICK on the button fires it — checked by the user on the Mac (verify step 4); no test covers the click.

**To verify (Mac):** open View ▸ Show Buffers. Hold ⌃ → the icons stay Copy. Hold ⌃⌘ → Paste icon; click a filled slot's button
with ⌃⌘ held → it pastes at the caret.

---

## I-0292

**Title:** Scrivi Linux App CI fails on every push since SP-176 — a timing check measured against an -O0 build
**Status:** ✅ **VERIFIED 2026-10-13** (user: *"ci-linux passes now. mark I-0292 verified"* — run 38315453907, commit `23237d2`, green)
**Platform:** `[Build]` — `.github/workflows/scrivi-linux-ci.yml`
**Severity:** Medium — the Linux CI signal has been red, so a real Linux failure would not stand out
**Sprint:** — unassigned (user: *"Scrivi's ci-linux is failing again."*)
**Date Identified:** 2026-10-12 (failing since 2026-10-11, run 38167539536, the SP-176 live-pass push)

**Evidence:** four consecutive runs (38167539536, 38174077102, 38267461495, 38310102628) fail ONLY on `find_smoke`:
*"FAIL: the build stays in seconds, not minutes (6703 / 6737 / 6826 / 6774 ms)"* — `tests/find_smoke.cpp:315`, `buildMs < 5000`
on a 1,186-scene, 1.86 MB manuscript. ⚠️ [SP-176] ticked *"`find_smoke` in CI"*; the push that closed it was the first red run.

**Root cause:** the workflow's Configure step set no `CMAKE_BUILD_TYPE`, so GCC built at -O0. Every other Linux build is Release
(the rig, `Scrivi_Linux_Rig_Setup_v0_1.md:231`; the Docker image, `Dockerfile:51`). Measured 2026-10-12 in the Docker image:
the same smoke builds the presented text in **284 ms (Release)** vs **1179 ms (CI's configuration)** — 4.2×. The rest of CI's
6.7 s is the hosted runner. ⚠️ Inferred, not measured: Release on the runner ≈ 6.7 / 4.2 ≈ 1.6 s.

**Fix:** `-DCMAKE_BUILD_TYPE=Release` in the workflow's Configure step, with the reason. ⛔ The threshold is NOT loosened — it
exists to catch a quadratic build, and it must measure the build that ships.

---

## I-0293

**Title:** After a structured paste, undoing the next edit in the target scene restores the scene's text from BEFORE the paste
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0293 through I-0299 passes"*) — its Mac verify step. ⚠️ **Its Linux path is covered by `structured_paste_smoke` §2; the live Linux check is [SP-180] AC9.** Was: ✅ **Resolved - Not Verified** (2026-10-13) — proven at the user's request (Q3: *"not sure this is an issue. Prove it."*); fixed in [SP-180] step 0 (user: *"yes, as step 0 for both platforms."*)
**Platform:** `[ScriviCore]` — every platform that records a `structuredPaste` node reaches it (Apple today; Linux from [SP-180])
**Component:** `history/HistoryService::recordBarrier` — the per-scene cached head; the platforms' structured-paste path
**Severity:** High — the writer's text is duplicated, and the structural undo that would repair it is refused, leaving the
created scenes behind
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 ([SP-180] planning, finding 1: read first, then proven)

**Description:** the core diffs each edit against its own cached copy of the scene (the head). [I-0287] made a HARD barrier
reset that copy for the scenes its op rewrote (`sceneTexts`). A STRUCTURAL node does not: `HistoryService.cpp:303` applies
`sceneTexts` only when `kind == Barrier`, and neither platform sends them with a `structuredPaste`. `fragment_paste` rewrites
the target scene (head + first piece), so its cached head stays at the pre-paste text.

**Proof (through `scrivi_*` alone, a real project; scratch program, 2026-10-13).** The sequence is Apple's
`pasteStructuredFragment` (`ManuscriptTextView.swift:2593`): `capture_flush` → `fragment_paste` → `capture_record_barrier`
with a `structuredPaste` payload. Target `"Alpha. Bravo."`, fragment `[none:"one", scene:"two", tail:"three"]` at byte 7.
After the paste: S = `"Alpha. one"`, plus new scenes `"two"` and `"threeBravo."`. The writer types `.` in S.

| | The target scene was edited earlier in the session | Control: not edited before the paste |
| - | - | - |
| Ctrl+Z #1 | ⛔ S = `"Alpha. Bravo."` (expected `"Alpha. one"`) — "Bravo." now ALSO in `"threeBravo."` | ✅ `"Alpha. one"` |
| Ctrl+Z #2 (the structural node) | ⛔ `fragment_uncut_paste` REFUSED: *"target scene does not end with the pasted head text"* — the two created scenes stay, and history has already moved past the node | ✅ uncut → `"Alpha. Bravo."`, scenes removed |

The control passes because a scene's first edit of a session seeds the head from "text before". Only a scene that is already
seeded keeps the stale head. That is the normal case: the writer edits a scene and then pastes into it.

**Expected:** after a structured paste, undoing the next edit in the target scene returns it to what the paste left; the
next undo removes the paste.

**Fix direction:** to be designed in [SP-180] step 0. A structural node carries the texts its op left (as [I-0287]'s
barrier does). The core keeps the heads right as undo and redo cross it, and on every head rebuild (reopen, eviction,
load validation). Both platforms send them. ⚠️ An optional field in `scrivi.history.v1`, backward-compatible.

**Related:** [I-0287] (the same gap for hard barriers; fixed), [EP-029] AC6 (reversible structural undo).

**Resolution (2026-10-13, [SP-180] step 0):**
- **Core:** a structural node now carries `sceneTexts` (what the op left, as [I-0287]'s barrier) and `sceneTextsBefore`
  (what each scene that existed held before it). Recording and redo reset the heads to `sceneTexts`; undo resets them to
  `sceneTextsBefore`; every head rebuild (reopen, load validation, eviction) reads `sceneTexts` as it does a barrier's
  (`applyBarrierTexts`, `HistoryService.cpp`). A scene the node brings into history takes its BEFORE text as its floor.
  Persisted in the node's `barrier` record, each written only when present; older logs read as before. ABI: optional
  `sceneTextsBefore` on `scrivi_history_record_barrier` / `scrivi_capture_record_barrier`.
- **Apple:** `pasteStructuredFragment` (`ManuscriptTextView.swift:2593`) reads back the target and created scenes after the
  paste and sends them with the target's text before it (`HistoryCapture.recordBarrier` / `captureRecordBarrier` gained
  `sceneTextsBefore`; the non-macOS engine stub follows).
- **Linux:** has no structured paste yet. It sends the same texts when [SP-180] step 4 builds it, which [SP-180] AC0 checks.

**Tests:**
- `CaptureCApiTests.cpp` (`[I-0293]`): the node's texts on both sides of it, seeded and unseeded, in session and after a reopen
  on each side; and the real-project sequence (`fragment_paste`, the next edit, its undo, then `fragment_uncut_paste`
  accepted).
  - Red before the fix.
  - Four mutations each caught: no undo-side reset, no redo-side reset, the floor taking the after-text, and the log not
    reading the texts back.
  - ctest macOS 720/720; Linux (Docker, uid 1000) 723/723 and 31/31 smokes.
- Interop `UndoAfterStructuralOpTests.structuredPasteThenUndo`, through the real coordinator: red with Apple's texts
  removed, green with them. Interop 285/285.

**To verify (Mac):**
1. Type a sentence into a scene and wait a second.
2. Copy text across a scene break from elsewhere.
3. Click mid-sentence in the scene you typed in and paste with ⌘V.
4. Type a word, then ⌘Z: the scene reads as the paste left it, and no text appears twice.
5. ⌘Z again: the pasted scenes go and the scene is whole.

---

## I-0294

**Title:** A structured paste builds on the target scene as saved on disk — unsaved typing in it is lost
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0293 through I-0299 passes"*) — its Mac verify step. ⚠️ **Linux, built to the fixed shape, is covered by `structured_paste_smoke`; live check is [SP-180] AC9.** Was: ✅ **Resolved - Not Verified** (2026-10-13) — ruled into [SP-180] (user: *"yes I-0294 in SP-180."*)
**Platform:** `[Apple]` (Linux must not repeat it: [SP-180] step 4)
**Component:** `ManuscriptTextView.swift` `pasteStructuredFragment` (`:2593`)
**Severity:** Medium — it loses only what was typed since the last autosave (about a second), but silently
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 ([SP-180] step 0, writing [I-0293]'s interop test)

**Measured (interop, real coordinator):** scene `"Alpha Bravo"`. Type `" Pre"` at its end (not yet saved), caret after
`"Alpha "`, then paste the fragment `[none:"one", scene:"two", tail:"three"]`. Result:
`["Alpha one", "two", "threeBravo"]` — ⛔ `" Pre"` is gone. Expected: `"threeBravo Pre"`.

**Cause (read):** `scrivi_fragment_paste` splits the target scene as it is ON DISK, and the reload that follows replaces the
editor's text with disk. `pasteStructuredFragment` commits pending history (`flush`) but never saves the scene first, and it
computes the caret byte from the in-memory text, so the two can disagree. A split saves first
(`handleCreateScene`: `loader.saveCurrentIfDirty`, `:1116`); the structured paste does not.

**Fix direction:** save the current scene if dirty before `fragmentPaste`, as a split does. A failing interop test first:
[I-0293]'s test without its explicit save.

**Resolution (2026-10-13):** `pasteStructuredFragment` writes every scene with unsaved typing before `fragmentPaste`
(`loader.saveSceneBlocking`), right after committing pending history. Every scene, not only the target, because the reload
replaces them all: the failing test showed the OTHER scene's unsaved typing lost too.

**Tests:** interop `UndoAfterStructuralOpTests.structuredPasteKeepsUnsavedTyping`, with two scenes typed in and neither
saved. Red before the fix: `["Alpha one", "two", "threeBravo", "Second"]`. Green after:
`["Alpha one", "two", "threeBravo Pre", "Second Pre"]`, and on disk. Interop 286/286.

**To verify (Mac):** type a few words into a scene and, at once (within a second), paste a cross-scene copy into it with ⌘V.
The words you typed are still there.

---

## I-0295

**Title:** A structured paste over a selection lands before it and leaves the selection in place
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0293 through I-0299 passes"*) — its Mac verify step. ⚠️ **Linux, built to the fixed shape, is covered by `structured_paste_smoke`; live check is [SP-180] AC9.** Was: ✅ **Resolved - Not Verified** (2026-10-13) — [SP-180] Q2 (user: *"Q2: recommended."* — prove on the Mac first, then fix both)
**Platform:** `[Apple]` (Linux built to the fixed shape in [SP-180])
**Component:** `ManuscriptTextView.swift` `pasteStructuredFragment`
**Severity:** Low
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 ([SP-180] finding 3, read; then proven)

**Measured (interop, red before the fix):**
- Inside one scene: `"Alpha Bravo Charlie"` with "Bravo" selected, pasting `[one | two | three]` gave
  `["Alpha one", "two", "threeBravo Charlie"]`.
- Across a break: the selection stayed in both scenes.

**Fix:** the selection is removed first, as its own undo step: across scenes with [I-0270]'s grouped delete, inside one scene
with an ordinary delete. The fragment then lands where the selection began. Ctrl+Z takes back the paste, and a second Ctrl+Z
the removal.

**Tests:** interop `structuredPasteReplacesTheSelection` (in one scene, with both undos; and across a break): red before,
green after. Interop 287/287. Linux: `structured_paste_smoke` §3.

**To verify (Mac):** copy text across a scene break, select a word elsewhere, ⌘V. The word is replaced. ⌘Z removes the paste;
⌘Z again brings the word back.

---

## I-0296

**Title:** After a structured paste, the next open takes the pasted-into scene for an edit made outside Scrivi
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0293 through I-0299 passes"*) — its Mac verify step. Was: ✅ **Resolved - Not Verified** (2026-10-13) — ruled into [SP-180] (user: *"yes fix it in SP-180."*)
**Platform:** `[ScriviCore]` (fixed there); reached from `[Apple]` today; `[Linux]` as soon as it pastes ([SP-180]). Both share the history file.
**Component:** `scrivi_fragment_paste` / `scrivi_fragment_uncut_paste` and the history's saved-bytes baseline (`HistoryStore`
`persistedDiskHashes_`, [I-0104])
**Severity:** Medium — nothing is lost, but undo stops at a false *"This scene was changed outside Scrivi"* after every
structured paste and every structural undo/redo, once the project is reopened
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 ([SP-180] step 8: `structured_paste_smoke` §7 reopened after a paste and was stopped by it)

**Cause:** at the next open, history compares each scene's file with the hash of the last bytes SCRIVI wrote, which the
save path reports (`noteScenePersisted`). A structured paste, and its undo and redo, are written by the CORE
(`fragment_paste`, `fragment_uncut_paste`), and nothing reports those bytes. So the baseline is the text from before the
paste, and the reopen calls the file an outside edit. A scene the paste CREATED is not flagged: its baseline comes from
[I-0293]'s `sceneTexts`.

**Proof:**
- **Mac:** interop `UndoAfterStructuralOpTests.structuredPasteThenReopen`. Paste, close the history as a window does,
  reopen, and validate each scene against its file, as `loadAsync` does. The target `"Alpha one"` is flagged.
- **Linux:** `structured_paste_smoke` §7 simulates a history written on the Mac. The reopen raised *"This scene was changed
  outside Scrivi"* three times, and every undo stopped there.

**Fix direction (recommended):** in the core. `scrivi_fragment_paste` and `scrivi_fragment_uncut_paste` write the scenes
themselves, so when the project's history is open they record those bytes as written (what `noteScenePersisted` does for
the save path). That is one place for both platforms, and it covers undo and redo too.
Alternative: each platform reports the scenes after each paste, re-paste and uncut (six call sites). A failing test first
in `CaptureCApiTests` (paste, close, reopen, validate); the interop test above and smoke §7 then pass.

**Resolution (2026-10-13), the recommended core fix:** the three ScriviCore calls that write scene files themselves now
record those bytes when the project's history is open, exactly as a platform's save does (`noteScenePersisted`). A shared
helper, `noteScenesWritten` (`scrivi_c_api.cpp`), covers:
- `scrivi_fragment_paste`: the target and the scenes it created;
- `scrivi_fragment_uncut_paste`: the target;
- `scrivi_fragment_cut`: the survivor. Neither platform calls it now, but old Mac histories replay its inverse.

With no history open it does nothing. Neither platform changed.

**Tests:**
- New `CaptureCApiTests` `[I-0296]` case on a real project. Paste → reopen → validate; undo the paste (uncut) → reopen →
  validate; a collapsing cut → reopen → validate. Red before the fix (the target flagged, and the undo stopped at the false
  mark). Each of the three calls removed in turn is caught.
- Mac interop `structuredPasteThenReopen` was red; it passes with no Mac change (288/288).
- Linux `structured_paste_smoke` §7 (a history written on the Mac, replayed on Linux) passes.
- ctest macOS 721/721, Linux (Docker, uid 1000) 724/724; 32/32 Linux smokes.

**To verify (Mac):** paste a cross-scene copy into a scene, quit Scrivi, reopen the project, then ⌘Z. It undoes the paste,
with no *"This scene was changed outside Scrivi"* notice.

---

## I-0299

**Title:** The Mac's reopen takes scenes written by Linux on the share, and scenes the Mac pasted into, for outside edits
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0293 through I-0299 passes"*) — its Mac verify step. Was: ✅ **Resolved - Not Verified** (2026-10-14) — ruled into [SP-180] (user, 2026-10-13); proven, then fixed (user, 2026-10-14: *"yes, lets fix I-0299 in SP-180."*)
**Platform:** `[ScriviCore]` (the saved-bytes baseline, [I-0104] / [I-0296]); reached from both
**Severity:** Medium — nothing is lost, but undo stops at a false *"This scene was changed outside Scrivi"*
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 ([SP-180] live pass, step 39)

**Measured (the project's own history log, `dumas-prose-timelines`, UTC):**
- Linux undid a structured paste at 21:08:02 (the core's uncut wrote scene `…d97a6d134813`) and pasted at 21:17:29 (the core
  wrote `…bb0f621e6f95`). The Mac's reopen at 21:21:12 raised `externalChange` for exactly those two scenes.
- The Mac pasted at 21:29:17 (its own session) and did a cross-scene delete at 21:33:12. Its reopen at 21:36:42 raised
  `externalChange` for the two scenes it had pasted into and deleted from (`…0d7e9c547b11`, `…1dfc91282744`). That one is
  Mac-only.
- [I-0296]'s fix is in build 73 (commit `139e532`); the same sequence on local disk passed (step 34, and the tests).

**Unknown:** why the recorded bytes did not hold here — the share, how those sessions ended (several quits), or a path the
tests do not cover. ⛔ No fix before a reproduction.

**Probes (2026-10-13):**
- ✅ A Mac cross-scene delete followed by a CLEAN close and reopen flags nothing (interop `crossSceneDeleteThenReopen`, kept
  as a guard).
- ✅ The Mac's save path records its bytes (`saveSceneBlocking` → `noteScenePersisted`).
- ✅ The [I-0296] path passes on local disk (tests, and rig step 34).
- ⚠️ **A clean close leaves no mark in the log;** only `state.json` changes. So the log cannot show which of those sessions
  ended cleanly.

**Leading hypothesis (not proven):** the recorded bytes reach `state.json` ONLY when the history closes. A session that ends
without closing loses every byte it recorded, and the next open flags each scene it wrote. That covers a force-quit, Xcode
stopping or relaunching the app, and a crash. ⏳ Asked the user how those sessions ended. If it is this, the remedy is to
record the bytes as they are written (an append-only log record, replayed at open) rather than only at close: a
`scrivi.history.v1` change, to be ruled.

**Rig re-run (2026-10-13 evening, build 74, every quit clean):** a watcher on the Mac logged each `state.json` checkpoint.
- **Sessions:** six or more across the Mac and Linux, each closed with a checkpoint.
- **What they did:** a Mac paste, undone and redone on Linux; a Linux paste of the Mac's buffer; a Linux paste then undone on
  the Mac.
- ✅ **No new `externalChange` barrier was recorded.**
- **What the user hit first:** "changed outside Scrivi; undo stops here". Those were **yesterday's** barriers (5:36 pm,
  session `c9201f`). The Mac's paste sat directly on top of them, so once that paste had been undone, the next undo stopped
  there, as a barrier should.
- **Conclusion:** consistent with the unclean-ending hypothesis; not proof of it.

**PROVEN 2026-10-14 (the user: *"Ok, you may conduct your test."*).** Three processes, `scrivi_*` only:
- **Session 1:** an edit, saved and reported (`note_scene_persisted`), then a clean close.
- **Session 2:** an edit, saved and reported, then the process ends WITHOUT closing (`_Exit`, as a force-quit or Xcode's Stop
  does).
- **Session 3:** reopens and validates the scene against the bytes session 2 wrote → ⛔ `externalChange: true`.
- **Control** (session 2 closes normally) → ✅ `externalChange: false`.

**Cause:** the recorded bytes live in memory (`persistedDiskHashes_`) and reach disk only at a checkpoint, which happens at
close (or every 200 records). A session that ends any other way loses them, and the next open compares each file with an
older hash.

**Fix direction (⏳ needs the user's ruling, a `scrivi.history.v1` change):** append a small "persisted" record to the log as
each save is reported, and replay it at open, so the baseline survives an unclean end. Older logs, without such records, read
as before.

**Resolution (2026-10-14):** `HistoryStore` appends a `rec:"persisted"` record (sceneID and sha256) to the log whenever the
save path reports a write and the bytes changed, and whenever an `externalChange` repair adopts the disk text. At open the
newest record per scene outranks `state.json`, for scenes with a floor, as `state.json` covers. Documented as A.1.d in
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`. Additive: older logs read as before.

**Tests:**
- New `CaptureCApiTests` `[I-0299]` case. A child process (`fork`) saves and exits without closing; the reopen flags nothing.
  A real outside edit is still flagged. A repair made in a session that then dies also holds.
  - Red before the fix at both points.
  - Each of the three parts removed in turn is caught.
  - POSIX only: `fork`, skipped on Windows.
- `HistoryCApiTests` "a stale state.json hash does not re-warn forever (I-0104 loop)" now strips the new records, so it still
  models a log from before the fix. With them present, a stale `state.json` alone no longer warns at all, which is the point.
- ctest macOS 722/722, Linux 725/725 (Docker, uid 1000); 32/32 Linux smokes; interop 290/290.

**To verify:** with the Mac's Scrivi run from Xcode, edit a scene, wait a second for the save, press Xcode's **Stop**, run
again, open the project, ⌘Z. There is no "This scene was changed outside Scrivi".

**The second half (2026-10-14).** The user's Mac check, and a probe through `scrivi_*`, showed the first fix opened a
data-loss path:
- Typing still PENDING (latched, not yet a step) when a session dies was saved by the autosave but never recorded.
- Once the unclean end no longer raised a barrier, the next edit diffed against history's OLDER copy, and undoing it also
  removed the pre-crash typing: `"Alpha. Beta gamma"` → `"Alpha."`.
- The barrier the first half removed had, wrongly worded, been re-syncing history to the file.

✅ **Ruled** (user: *"You need to record the session close as a historical event."*). The resolution:
- A clean `scrivi_history_close` appends `ctl:close` before its checkpoint, and every session record carries
  `closesRecorded: true`.
- At open, a last session that says it records its close but has none ended uncleanly. For each scene whose file is what
  Scrivi last wrote but whose history copy differs, the difference is recorded as an ordinary undo step.
- Nothing is lost, there is no false message, and Ctrl+Z / ⌘Z undoes that typing.
- After a clean close, or on a log from an older build (no `closesRecorded`), nothing is added. A file that differs from
  what Scrivi wrote is still an `externalChange` barrier.

**Tests (second half):** `CaptureCApiTests` "typing pending when a session dies is recovered as an undo step".
- The killed session: red before the fix, `"Alpha."` where `"Alpha. Beta"` belongs.
- A clean close adds nothing.
- A clean close with bytes history never saw adds nothing; this one depends on the close record.
- Mutations caught: no close record; no recovery.
- ctest macOS 723/723, Linux 726/726; 32/32 Linux smokes; interop 290/290.

---

*Last Updated: 2026-10-15 (**I-0293, I-0294, I-0295, I-0296, I-0299** VERIFIED (user, Mac).)*

*Last Updated: 2026-10-13 (**I-0292** VERIFIED (user).)*

*Last Updated: 2026-10-12 (batch created: **I-0291** VERIFIED (user).)*

---

## I-0298

**Title:** A structural undo into an earlier session skips the "Undoing changes from a previous session" warning
**Status:** ✅ **VERIFIED 2026-10-21** (user, 2026-10-21: *"all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*), rig live pass, build 75: step 10 on the rig (Ctrl+Z after a quit and reopen showed "Undoing changes from a previous session"); the Mac half passed 2026-10-15.
**Platform:** `[Apple]` and `[Linux]` — the same shape on both
**Component:** Apple `ManuscriptTextView.swift` `apply(step:)`; Linux `EditorShell::applyHistoryStep`
**Severity:** Low — the undo works; the writer is not told it reached into an earlier session
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 (user, [SP-180] live pass step 36: *"no dialog was shown warning us that we are undoing past
the session boundary"*)

**Cause (read):** the core reports `crossedSessionBoundary` on a structural step as on any other (`HistoryService.cpp`, the
structural branch of `undo`). Both platforms return early for `structuralInverse` (Apple `apply(step:)` `:408`; Linux
`applyHistoryStep`) — BEFORE the code that shows the warning (Apple `:458`; Linux at the end of `applyHistoryStep`).

**Fix direction:** show the warning for a structural step too, on both platforms. A failing test first.

**Resolution (2026-10-13):** both platforms show the warning after a structural step as after any other. Apple:
`sessionBoundaryReached`, with an `onSessionBoundary` hook replacing the modal in tests. Linux: `warnIfSessionBoundary`.

**Tests:**
- **Mac** (interop `structuralUndoAcrossSessionsWarns`): paste, reopen in a new session, undo. Red without the fix (0
  warnings); 290/290 with it.
  - The first version passed the wrong way: with nothing before the paste, the undo lands on the history's root, which belongs
    to no session. The test now edits first, as the rig's case had real work underneath.
- **Linux** (`structured_paste_smoke` §7, the Mac's paste undone on Linux): red without the fix. The new mutation
  `no-structural-session-warning` is caught.
  - §7 now keeps one Linux edit before the Mac's paste, for the same reason.

**To verify:** quit and reopen after a cross-scene paste, then Ctrl+Z / ⌘Z. "Undoing changes from a previous session" appears.

---

---

## I-0300

**Title:** After a new chapter (Ctrl+Shift+Return) or a mid-scene split (Ctrl+Return), the caret does not move into what was created
**Status:** ✅ **VERIFIED 2026-10-21** (user, 2026-10-21: *"all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*), rig live pass, build 75: steps 19–20 on the rig (the caret moved into the new scene and the new chapter).
**Platform:** `[Linux]`
**Component:** `EditorShell::onCreateChapterRequested`, `onCreateSceneRequested` (mid-scene path)
**Severity:** Low — the writer clicks into the new chapter; nothing is lost
**Sprint:** [SP-180]
**Date Identified:** 2026-10-13 (user, [SP-180] live pass step 24: *"When I hit Ctrl-Shift-Return in Step 24, the caret remained in Chapter 1 instead of moving on to Chapter 2."*)

**Cause (read):** both paths call `load(…)`, which reloads ASYNCHRONOUSLY (a worker, [T-0499]), then at once call
`moveCaretToSegment` / `selectNavigatorScene` — on the document the reload is about to replace. When the reload lands, it
places the caret from the restored writing surface instead. [SP-180]'s structured paste hit the same trap and waits for
`loadFinished` (`reloadThenPlaceCaret`).

**Fix direction:** place the caret when the reload has finished, through the same `reloadThenPlaceCaret`, with a smoke check
that fails first.

**Resolution (2026-10-13):**
- **`EditorShell::reloadThen(after)`:** runs `after` once the asynchronous reload has landed, and `placeCaret(sceneID, byte)`
  is shared with the structured paste.
- **Moved into `after`:**
  - the new chapter's caret and its I-0063 renumber, which also read the pre-split document;
  - the mid-scene split's caret;
  - the scene merge's seam caret;
  - the chapter merge's caret.
- **Tests:** `undo_smoke` §5 (the split's caret), §4b (the merge's seam), §7 (a new chapter mid-scene, and at the END of the
  last scene, the rig's step 24).
  - The step-24 case is red before the fix (caret at 103, want 150).
  - The other two pass either way: the reload's restore happens to land there. They are guards, not proofs.
  - Docker: `undo_smoke`, `structured_paste_smoke`, `scene_merge_smoke`, `scene_create_smoke` and `chapter_reorder_smoke` pass.

**To verify (rig):** at the end of the last scene press Ctrl+Shift+Return. The caret is in the new chapter's blank scene.

---
