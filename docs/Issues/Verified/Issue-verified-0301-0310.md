# Verified Issues: I-0301 – I-0310

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0291–I-0300 are in [`Issue-verified-0291-0300.md`](Issue-verified-0291-0300.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0302** | `[Apple]` ⚠️ **When the newest history step is a barrier, ⌘Z only flashes the screen — the barrier's notice (e.g. "This scene was changed outside Scrivi; undo stops here") never appears.** Linux shows it. Reported by the user 2026-10-14. ✅ Fixed: Undo stays available at a barrier, on both platforms. → [details](#i-0302) | **Low** | [SP-180] | ✅ **VERIFIED 2026-10-15** |
| **I-0303** | `[Apple]` ⚠️ **Replace All writes every changed scene on the MAIN THREAD, one after another — `saveScene` is `async` in name only.** Found 2026-10-15 by reading, planning [SP-181] Q3. Fix before Linux copies the shape (user ruling). | **Medium** | [SP-181] | ✅ **VERIFIED 2026-10-21** |
| **I-0304** | `[Linux]` ⛔ **A scene whose save FAILS is dropped from the dirty set, so it is never retried and quitting loses its edits.** The Mac keeps it dirty ([I-0259]). Found 2026-10-15 by reading ([SP-181]). ✅ Fixed: a failed write stays dirty. → [details](#i-0304) | **High** | [SP-181] | ✅ **VERIFIED 2026-10-21** |
| **I-0305** | `[Apple]` `[Linux]` ⚠️ **Closing a project window, or quitting, while scenes are being written in the background does not wait for the writes.** Found 2026-10-15 by reading ([SP-181]). ✅ Fixed on both platforms: a close or quit waits behind *"Please wait — saving updates…"*. → [details](#i-0305) | **Medium** | [SP-181] | ✅ **VERIFIED 2026-10-21** |
| **I-0307** | `[Linux]` ⛔ **Selecting across a scene break, Ctrl+C and Ctrl+V each FREEZE the window for 10–15 s** (dumas-prose-timelines over the share). Found on the rig 2026-10-15 (live pass, step 9). ✅ Measured and fixed in Docker: Ctrl+C's GUI-thread file calls 2,678 → 3; a mouse selection no longer extracts. → [details](#i-0307) | **High** | [SP-185] | ✅ **VERIFIED 2026-10-21** |

---

## I-0302

**Title:** On the Mac, ⌘Z at a barrier flashes instead of showing the barrier's notice
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0302 passes, the dialog appeared"* — ⌘Z in dumas-prose-timelines at the 14:56 barrier). ⚠️ **The Linux half (the Edit ▸ Undo menu item) is covered by `undo_smoke` only, not checked live on the rig.** Fixed 2026-10-15 in [SP-180] (user: *"Ok fix it in SP-180."*)
**Platform:** `[Apple]` (and Linux's Edit ▸ Undo menu item, the same rule)
**Component:** `ManuscriptTextView.swift` `validateUserInterfaceItem` (`#selector(undo(_:))` → `coordinator.canUndo`)
**Severity:** Low — nothing is lost, but the writer is told nothing about why undo does nothing
**Date Identified:** 2026-10-14 (user: *"I load the project and the very first thing I do is type Cmd-Z. The screen flashes as if
there is nothing to undo … I get no dialogs. Just the flash."*)

**Cause (read; matches the project's log):**
- The newest step of dumas-prose-timelines' history is an `externalChange` barrier (14:56 local, 2026-10-14), from a session
  that ended without closing under a build without [I-0299]'s fix.
- The core reports `canUndo: false` at a hard barrier. The Mac DISABLES Edit ▸ Undo on that, so ⌘Z only beeps or flashes,
  and `apply(step:)`'s barrier notice is never reached.
- Linux calls undo regardless and shows the notice (`EditorShell::applyHistoryStep`).

**Fix direction:** the core's capture state says when the newest step is a barrier (`atBarrier`). The Mac enables Undo
then, and ⌘Z shows the barrier's own notice, as Linux does. A failing interop test first.

**Resolution (2026-10-15):**
- **ScriviCore:** every capture result carries `atBarrier`: the newest step is a hard barrier
  (`HistoryService::atBarrier`, added once in `withCapture`). A reversible structural step is not one.
- **Mac:** `CaptureState.atBarrier`, `HistoryCapture.atBarrierNow`, and `Coordinator.undoAvailable` (`canUndo ||
  atBarrier`) decide Edit ▸ Undo. ⌘Z at a barrier now shows its notice. `onBarrierNotice` replaces the modal in tests.
- **Linux:** Ctrl+Z already reached the notice, but the Edit ▸ Undo menu item was disabled at a barrier. It now follows
  `EditorShell::undoAvailable()`, the same rule.

**Tests:**
- `CaptureCApiTests` `[I-0302]`: the flag at the root, after an edit, at a barrier, after an undo back onto it, and not for a
  reversible structural step. Red before the fix.
- Mac interop `undoAtBarrierShowsTheNotice`: a split, then Undo validated through `validateUserInterfaceItem` (the
  validation AppKit calls), then ⌘Z shows "Can't undo past creating a scene". Red without the Mac change (Undo disabled).
- Linux `undo_smoke` §5: at the split's barrier, `!canUndo && undoAvailable`. Red without the Linux change.
- ctest macOS 724/724, Linux 727/727; 32/32 Linux smokes; interop 291/291.

**To verify (Mac):** in dumas-prose-timelines (its newest step is the 14:56 barrier), press ⌘Z. A dialog says *"This scene was
changed outside Scrivi; undo stops here."* instead of a flash.

---

## I-0303

**Title:** Replace All writes every changed scene on the main thread
**Status:** ✅ **VERIFIED 2026-10-21** (user, 2026-10-21: *"all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*), rig live pass, build 75: step 4 on the Mac (Replace All in two scenes: "Saving 2 scenes…", only those two files written).
both platforms. File it in [Apple] and put it in SP-181 before linux."*)
**Platform:** `[Apple]`
**Component:** `ManuscriptTextView.Coordinator.replaceAll`; `ViewportSceneLoader.saveScene` / `saveSceneBlocking`
**Severity:** Medium. Nothing is lost, but the window stops responding while a large Replace All writes, and longer on slow
storage (a network share, a USB drive).
**Sprint:** [SP-181]
**Date Identified:** 2026-10-15, by reading, while planning [SP-181] Q3

**The rule (user, 2026-10-15, SP-181 Q3):** *"Replace All must not run on the gui thread. always use the worker (it will be
needed). Also we need to not automatically rewrite every frigging scene if it isn't necessary."* Then: *"apply the rule to
both platforms."*

**Description (read, not yet measured):**
- `replaceAll` (`Scrivi/Views/ManuscriptTextView.swift:2484`) applies the replacements, records ONE grouped history step,
  then starts one save per touched scene: `Task { @MainActor in await loader.saveScene(...) }` (`:2517–2518`).
- `saveScene` is declared `async` but only calls `saveSceneBlocking` (`ViewportSceneLoader.swift:372–374`), whose engine
  calls are synchronous C ABI calls. Each Task runs on the main actor. So every write happens on the main thread, one after
  another: on dumas, about 1,171 of them for a word that occurs in every scene.
- ✅ By reading, only scenes holding a match are written (`touched`, `:2487`; `saveSceneBlocking` also returns early for a
  clean scene, `:381`). ⏳ Proven by [SP-181] plan step 0's test, not assumed.

**Expected:**
- A Replace All never does its file writes on the main thread. AppKit requires the text storage edit itself to stay on the
  main thread; what moves off it is everything that does not touch the view, above all the scene writes.
- While it runs the writer sees progress, and the manuscript cannot be edited until it lands.
- Exactly the scenes that hold a match are written, and nothing else.

**Fix direction:**
- Find how the Mac already shows a long operation and blocks editing before writing a new one.
- Write the touched scenes off the main actor. ⚠️ Keep [I-0259]'s rule: a failed write leaves its scene dirty.
- Prove it with an interop test that fails first: no scene write on the main thread during a Replace All, and only the
  matched scenes written.

**Resolution (2026-10-15):**
- `ViewportSceneLoader.saveScenesOffMain`: the matched dirty scenes' texts are taken on the main actor and written in a
  detached task. When they land, each is marked clean (or saved again if it changed meanwhile) and its bytes are told to
  history. A failed write stays dirty ([I-0259]).
- A main-actor save of a scene still being written waits until that write lands, so an older write never lands over a
  newer one. Quitting waits for the writes (up to 30 s), then writes the still-dirty scenes itself.
- `replaceAll` locks the manuscript and turns Undo / Redo off until the writes land; the editor shows *"Saving N
  scenes…"* as an overlay.
- **Test:** `replaceAllWritesOffTheMainThread` (interop), red first; two mutations caught. Interop 292/292.

**To verify (Mac):** in a project with many scenes, Replace All a word that occurs in only two of them. *"Saving 2
scenes…"* shows briefly, the text cannot be typed into until it goes, and only those two scene files change on disk.

---

---

## I-0304

**Title:** A failed Linux save drops the scene from the dirty set
**Status:** ✅ **VERIFIED 2026-10-21** (user, 2026-10-21: *"all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*), rig live pass, build 75: steps 28–29 on the rig (a failed save showed an error and stayed dirty; after write access returned, it was saved).
**Platform:** `[Linux]`
**Component:** `EditorShell::saveScene`, `EditorShell::flushActiveScene`'s save, `EditorShell::saveScenesOffMain`
**Severity:** High. The edit stays on screen but is never written: nothing retries it, and closing or quitting does not
write it either, so it is lost. The Mac's equivalent was [I-0259] (High).
**Sprint:** [SP-181]
**Date Identified:** 2026-10-15, by reading, during [SP-181]

**The rule (user, 2026-10-15):** *"We'll have to remark it as needing saving when the save fails."*

**Description (read):**
- `saveScene` (`platforms/linux/src/EditorShell.cpp:1595`) removes the scene from `dirtyScenes_` *"whatever the outcome"*:
  *"re-queuing it would just fail again every debounce. (A retry policy is out of scope here.)"*
- The teardown save of the caret scene does the same.
- `saveScenesOffMain` (SP-181) re-dirties a scene only when its text changed meanwhile, not when its write failed.

**Expected:** as on the Mac ([I-0259]): a failed write leaves the scene dirty and is not told to history; the next save
retries it.

**Resolution (2026-10-15):**
- `saveScene` (`platforms/linux/src/EditorShell.cpp:1597`) and the teardown save of the caret scene remove a scene from
  `dirtyScenes_` only after a write that succeeded.
- `saveScenesOffMain` (`:2729`) puts a scene whose write failed back in the dirty set, and on its 10-minute timeout puts back
  all of them (nothing reported back, so nothing is known to be written).
- Retries come on the existing cadence: the debounce after an edit, a scene change, a close. Nothing retries on a timer
  alone, so a dead volume does not cause a loop of errors.
- **Test:** a new `background_wait_smoke` §1–§2 (CI): a read-only scene folder fails the save, and once it is writable again
  the next save writes the text. 2 mutations caught (the ordinary save dropping it, the off-main save dropping it).

**To verify (rig):** with the project on the share, disconnect the share (or make a chapter folder read-only), type into a
scene, and move to another scene; the error shows. Reconnect, type one more character anywhere, and wait two seconds: the
first scene's text is on disk.

---

---

## I-0305

**Title:** Closing a window or quitting does not wait for background writes
**Status:** ✅ **VERIFIED 2026-10-21** (user, 2026-10-21: *"all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*), rig live pass, build 75: steps 11 and 13 on the rig (Replace All then Ctrl+W, then File ▸ Quit: "Please wait — saving updates…", every replacement there); the Mac half passed 2026-10-15.
**Platform:** `[Apple]` `[Linux]`
**Component:** Apple: `ProjectWindowController.windowShouldClose`, `AppDelegate.applicationShouldTerminate`,
`ViewportSceneLoader.saveAllDirtyBlocking`. Linux: `ScriviWindow::closeEvent`, `AppEnvironment::quitApplication`,
`EditorShell::saveScenesOffMain` / `replaceAll` / `runStructural`.
**Severity:** Medium. A write cut off by the close can leave a scene's new text unwritten, or written without history being
told its bytes ([I-0104]).
**Sprint:** [SP-181]
**Date Identified:** 2026-10-15, by reading, after [SP-181]'s background saves ([I-0303])

**The rule (user, 2026-10-15):** *"In both cases (Apple and Linux), if we attempt to close the application while a
background update is in progress the close must WAIT until the background process is completed. I recommend showing a
dialog with a spinner that says something like "Please Wait until all save operations are complete." or "Please Wait,
saving updates"."*

**Description (read):**
- **Apple, window close:** `windowShouldClose` checks only for unsavable scenes; the window then closes and `session.close()`
  releases history while an off-main write may still be in flight. Its landing then finds no history to tell.
- **Apple, quit:** `saveAllDirtyBlocking` waits up to 30 s for the in-flight writes, on the main thread, with nothing on
  screen.
- **Linux:** `closeEvent` flushes the dirty scenes and releases the project. Scenes being written by `saveScenesOffMain` are
  no longer in the dirty set, so the close does not wait for them, and the landing never runs (the window is gone). The same
  holds for a structural step's worker (`runStructural`, [I-0297]) and Replace All's compute.

**Expected:** a close or quit while background work runs waits until it finishes, behind a dialog with a spinner (*"Please
wait — saving updates…"*), then closes as usual. Nothing is cut off.

**Fix direction:** one "background work done" signal per project (Apple: the loader's off-main writes; Linux: the shell's
off-main saves, Replace All and structural workers). Window close and quit wait on it behind the dialog.

**Resolution (2026-10-15):**
- **Apple:** `ViewportSceneLoader.whenOffMainWritesLand` (`Scrivi/Views/ViewportSceneLoader.swift:539`) runs its waiters in
  the step where the last off-main write lands. `SavingUpdatesWait.waitFor` (`Scrivi/App/ProjectWindowManager.swift:62`)
  shows the spinner (a SwiftUI `ProgressView` with *"Please wait — saving updates…"*), as a sheet over the window or, for
  Quit, a panel of its own, and goes on once every busy project has landed.
  - Window close (`windowShouldClose`, `:259`) and File ▸ Close Project (`close()`, `:241`) wait on it, then close again.
  - Quit: `applicationShouldTerminate` (`Scrivi/App/ScriviApp.swift:132`) answers `.terminateLater` and replies once the
    writes land, then asks the [T-0570] question as before. The quit path's 30 s wait in `saveAllDirtyBlocking` stays as a
    backstop.
- **Linux:** `EditorShell` counts its background work (`beginBackgroundWork` / `endBackgroundWork`, `EditorShell.cpp:2598`):
  the off-main saves, Replace All (compute and save), and a structural step's worker (`runStructural`, [I-0297]).
  `whenBackgroundWorkDone` (`:2609`) runs once none is left.
  - `ScriviWindow::waitForBackgroundWork` (`ScriviWindow.cpp:720`) shows `showSavingUpdates` (`:739`, a `QProgressDialog`
    with no Cancel) and closes again when the work is done. `closeEvent` (`:668`) and Close Project (`showLanding`,
    `:610`) use it.
  - Quit (`AppEnvironment::finishQuit`, `AppEnvironment.cpp:244`) waits for every busy window behind ONE dialog before
    closing any, then flushes and closes as before.
  - ⚠️ A Linux worker that never reports back still gives up after 10 minutes (the existing `AsyncCall` backstop, with its
    "taking longer than expected" message), and the close then goes ahead. The Mac waits without a limit.
- **Tests:**
  - Apple: `closeWaitsForBackgroundWrites` (interop; 2 mutations caught: the waiters never run, never waiting).
  - Linux: a new `background_wait_smoke` §3–§4 (CI; 2 mutations caught: `closeEvent` not waiting, Quit not waiting).
  - Interop 293/293; Linux smokes 34/34.

**To verify:**
- **Mac:** in a large project, Replace All a common word and at once press ⌘W (then, separately, ⌘Q). The sheet (or panel)
  *"Please wait — saving updates…"* shows until the writes finish, then the window closes (the app quits). Reopen: every
  replacement is there.
- **Linux (rig):** the same with Ctrl+W, the window's X, and File ▸ Quit, on the network-share project.

**Mac live check (2026-10-15, user):**
- First run: the sheet was too small for its text. ✅ Fixed: the label now sits above the spinner, and the sheet sizes to
  fit it.
- Re-check: ✅ *"the sheet looks right now."* ⏳ Linux, on the rig.

---

---

## I-0307

**Title:** Cross-scene select, copy and paste each freeze the window 10–15 s
**Status:** ✅ **VERIFIED 2026-10-21** (user, 2026-10-21: *"all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*), rig live pass, build 75: step 9 on the rig (a cross-scene Ctrl+C / Ctrl+V in dumas over the share, the window kept redrawing).
**Platform:** `[Linux]`
**Severity:** High. The user, 2026-10-15: *"Cut Copy and Paste must be as fast as possible at all times. I'll accept long
load times, but once loaded the app must do everything instantly."*
**Sprint:** [SP-185]
**Date Identified:** 2026-10-15, user, rig live pass step 9 (dumas-prose-timelines, 1,186 scenes, over the share)

**What happened (user):** *"UI froze after select across scene. Again after copy and again after paste. about 10-15 seconds.
each time."*
- ⚠️ **The SELECTION froze too.** Selecting does no write and no core call by design; something runs per selection change.
  [I-0297] measured and moved the paste's worker off the GUI thread, so this is a different cost, or one it did not reach.
- Not yet measured. The first step is to attribute each freeze (select, Ctrl+C, Ctrl+V) on the rig, as [I-0297] did, before
  any fix. The user, 2026-10-17: *"This is surprising. We may need to create a log file so that we can record what the app is
  doing. A simple code inspection may not be adequate."* → [T-0612].

**Measured (2026-10-17, Docker, `strace -f`, a real `EditorShell` on 1,186 generated scenes; each phase bracketed by marker
probes, calls split by thread):**

| Phase | GUI-thread file calls |
| - | - |
| The cross-scene selection | 10 |
| Ctrl+C | **2,678** (2,492 scene sidecars, 122 directory listings) |
| The click into scene 5 | 10 |

- **Copy:** Linux's structured copy asks ScriviCore for the fragment (`scrivi_fragment_extract`) on the GUI thread, and the
  extractor RESOLVED the whole manuscript (every scene sidecar read and parsed, the [I-0196] class) to find two scenes.
- **Select:** Qt fills the X11 / Wayland PRIMARY selection on every mouse selection by calling the same
  `createMimeDataFromSelection` Ctrl+C uses: the selection ran the same extract. ⚠️ Inferred, not observed: the offscreen
  platform Docker runs has no primary selection. It also replaced the held fragment and token, so a mouse selection after a
  Ctrl+C turned the next Ctrl+V flat.
- **Paste:** 21 GUI-thread calls; its cost is the worker's, which is [I-0297]'s.

**Resolution:**
- **ScriviCore:** `scrivi_fragment_extract` takes the reading order from the project index
  (`FragmentExtractor::extract(root, spans, ordered)`); with no index it resolves as before. Ctrl+C: **2,678 → 3** GUI-thread
  file calls (measured again).
- **Linux:** the structured fragment is built ONLY for the clipboard (`ManuscriptEditor::copyToClipboard`: Ctrl+C, Edit ▸
  Copy, cut); the primary selection gets the flat text and leaves the held copy alone.
- **Tests:** `FragmentExtractTests` `[I-0307]` (no sidecar read given the order); `structured_paste_smoke` §1 (the primary
  selection carries no fragment token, and Ctrl+V after it is still structured); 1 mutation caught (the primary selection
  doing the structured copy). ctest Linux 730/730, macOS 726/726; smokes 36/36; interop 292/292.

**To verify (rig, over the share):** in dumas, drag a selection from scene 1 into scene 2, then Ctrl+C, then click and Ctrl+V.
No freeze on the selection or the copy. The diagnostic log ([T-0612]) times each.

---
