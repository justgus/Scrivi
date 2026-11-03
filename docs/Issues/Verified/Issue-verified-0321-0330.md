# Verified Issues: I-0321 – I-0330

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0311–I-0320 are in [`Issue-verified-0311-0320.md`](Issue-verified-0311-0320.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0321** | `[ScriviCore]` ⚠️ **The branch chooser offered a branch with no text: a change that only typed spaces (then undone) became a branch of its own.** Found by the user 2026-10-24 ([SP-188] Mac pass). → [details](#i-0321) | **Medium** | [SP-188] | ✅ **VERIFIED 2026-10-24** |
| **I-0322** | `[Linux]` ⚠️ **Linux has no "Project File Not Available" handling at all ([T-0570] was Apple-only): no banner when the project's drive goes away, and no "Some changes can't be saved" question on close or quit.** Found 2026-10-24 by reading, during [SP-193] ([I-0306]). → [details](#i-0322) | High | ✅ **Verified 2026-11-02** (folded into [SP-195] (Q3) and built there; [T-0648] passed on the Linux VM (the drive pulled: the red line, *"Some changes can't be saved"*, the held save landing)) |
| **I-0324** | `[ScriviCore]` ⛔ **With real scene links, dumas-prose-timelines does not open: every relationship endpoint walks the whole manuscript.** Found 2026-10-28 linking 7,029 scenes to the new Dumas world: `repairDangling` (every open) re-parsed all 1,175 scene sidecars for each of ~14,000 endpoints (5+ min at full CPU, local disk, not finished); a card's edge listing and each citation listing did the same. → [details](#i-0324) | High | [SP-197] | ✅ **Verified 2026-11-02** ([SP-197]'s Mac pass (2026-11-01) and [T-0650] on the VM (checklist: Pass)) |
| **I-0325** | `[Apple]` ⚠️ **iOS: the caret jumps to the start of a scene — the last character of the manuscript sends it to the previous scene's start, and typing or scrolling moves it elsewhere.** The iOS selection `onChange` had none of the Mac's [I-0132] loop break, so every viewport change echoed back as a navigation. ⚠️ Re-pass 2026-10-30: the end-of-manuscript case passes; a click then typing still lands elsewhere — diagnosing. → [details](#i-0325) | High | [SP-198] | ✅ Verified |
| **I-0326** | `[Apple]` `[Linux]` ⛔ **Replace All of a heading's first word deletes the heading's `## `** — dumas-prose-timelines lost the heading on all 542 scenes whose heading starts "The". The selection snap took the hidden prefix into the match. → [details](#i-0326) | High | [SP-198] | ✅ **Verified 2026-11-03** (user: Mac D1–D4; the Linux half → [T-0651]) |
| **I-0327** | `[Apple]` ⚠️ **The Scene Navigator showed an untitled scene's first line RAW — `## Youth against age`, escapes and emphasis markers included.** ✅ Ruled (user, 2026-10-30): *"The Navigator should show what the manuscript shows and hide what the manuscript hides."* → [details](#i-0327) | Medium | [SP-198] | ✅ Verified |
| **I-0328** | `[Apple]` ⛔ **Closing a project (⌘W / Close Project) saved nothing: typing inside the 1 s autosave was LOST, and the project reopened where it had been opened, not where the writer left it.** Both platforms: `ProjectSession.close()`, the one chokepoint every close reaches, released the session without the save and resume stamp that quit and app-resign do. → [details](#i-0328) | High | [SP-198] | ✅ **Verified 2026-11-03** (user: Mac D1–D4, iPad A13, iPhone B7) |
| **I-0329** | `[Apple]` ⛔ **iPad: dragging a Timeline scene dot a second time froze the whole UI for about 6 minutes** (no touch, keyboard or tap), then it recovered by itself. Found by the user on a real iPad with a hardware keyboard, 2026-10-31, in [SP-199]'s live pass, project `dumas-prose-timelines` (1,175 scenes). Steps: zoom the Timeline, drag a dot (it moves; on release the Time Delta Picker popover opens, as designed); dismiss it (the dot returns, as designed); drag a dot again → freeze. Evidence: the console logged `_UISystemGestureGateGestureRecognizer … has been in possible phase for 359.959951 seconds` and `Gesture: System gesture gate timed out`, which points to the main thread or the touch pipeline being stuck. ✅ **Cause found** (second `bt all`, taken WHILE frozen, 2026-10-31): the freeze starts when a Time Delta Picker choice is tapped ("that night"), not on the drag. `TimelineViewModel.setStoryTime` → `recomputeAndPersistFrom` (`TimelineStripView.swift:784`) calls `scrivi_set_scene_story_time` once for EVERY later scene, on the MAIN thread (~1,172 calls here). Each call's `StoryTimePatchGuard` (`scrivi_c_api.cpp:474`) drops the index BEFORE the write, so the core's `findSceneMetaPath` rebuilds the whole `ProjectIndex` (every sidecar, over iCloud) — ~1,172 full traversals ≈ 1.4M reads. The read-amplification class again; T-0549's patch saved only the AFTER rebuild. Linux `EditorShell.cpp:3763` runs the same chain loop. The dismiss "snap back" is by design (dismiss commits nothing); the moved position seen after reload is the "that night" write finishing. ✅ **Fix (user ruling: both halves, 2026-10-31):** (1) `StoryTimePatchGuard` no longer drops the index BEFORE the write (`scrivi_c_api.cpp:479`) — a warm index locates the scene, the after-write patch/drop is unchanged; (2) only the moved scene is saved — Apple `recomputeOffsetsFrom` (`TimelineStripView.swift:785`) recomputes later scenes in memory, Linux `showTimeDeltaPicker` (`EditorShell.cpp:3763`) dropped its re-save loop; both loads already derive offsets from the gap chain; (3) `exportProjectTimeline` derives scene starts from the gap chain instead of the stored `offsetMs` (`ScriviCore.cpp:1110`). Test: `ProjectIndexTests.cpp` "a story-time write keeps the WARM index (I-0329)". ctest 790/790, Mac interop 327, iOS 14/14, Mac build. ⚠️ Linux not built or smoked; no mutation check of the new test. iPad pass 1 (2026-10-31): steps 1, 2, 5 pass (no freeze on "that night"; the Mac step passes). Steps 3–4 were blocked: the picker showed only its first row of buttons. Cause: `.popover(arrowEdge: .top)` is honoured on iOS 18.1+, so the picker opened BELOW the dot, inside the bottom strip. Fix: `arrowEdge` is `nil` on iOS (`SceneDotView.pickerArrowEdge`); the Mac keeps `.top`. iPad pass 2: all steps pass, the picker fits. | High | — | ✅ **Verified** (user, 2026-10-31) |
| **I-0330** | `[ScriviCore]` `[Apple]` ⛔ **The pending-edge sweep walked the whole manuscript (2,538 file operations on dumas) on every switch back into the app, and its answer is discarded unless a world is away.** Found in the SP-197 Mac live pass log, 2026-11-01. → [details](#i-0330) | Medium | [SP-197] | ✅ **Verified** (user, 2026-11-01: Mac re-check) |

---

## I-0321

**Title:** The branch chooser offers a branch with no text
**Status:** ✅ **VERIFIED 2026-10-24** (user, Mac: *"I-0321 passes, mark it verified"*)
**Platform:** `[ScriviCore]` (the Mac shows what the core offers; Linux has no chooser)
**Component:** `HistoryService::forkAheadAt` (`ScriviCore/src/history/HistoryService.cpp`)
**Severity:** Medium. A choice with nothing in it gives the writer nothing to decide by.
**Sprint:** [SP-188]
**Date Identified:** 2026-10-24, user, Mac, on `dumas-prose-timelines`

**What happened (user):** *"I type "One", Undo, "Two", Undo, Redo "One", Then I type some more text after one. Undo, Undo,
Redo "One", Redo again. Choicepoint, somehow I have a choicepoint with no actual text in it. Or its all spaces. That is
unhelpful. Historical branches should be merged with the nearest sibling so that the writer is always presented with text
to choose from."*

**Cause (read from the project's own history log, today's session):** the fork after "Three\." had five branches, one of
them `inserted: "  "`: spaces typed after the sentence and then undone. The undo committed them as an event first, and the
next change from that point became its sibling. A branch can also START with whitespace (`"  One"`), and its preview showed
the leading spaces.

**✅ Ruling (user, 2026-10-24):** the writer is always presented with text to choose from.

**Resolution (2026-10-24):** the chooser's branches (`forkAheadAt`, used by `redoFork`) leave out a branch that has no
text anywhere in it. That branch is folded into its siblings: still in the history tree and the History card, just not
offered. Each branch that is offered shows the first text down its primary path, trimmed. If only one branch with text is
left, there is no chooser. Text = an insertion or removal with a non-whitespace character; a structural step always counts.
Core test `[I-0321]`.
- ⚠️ Not handled: if the blank branch is the PRIMARY one and only one other branch has text, ⇧⌘Z (no chooser) still
  follows the primary, and re-types the spaces.
- ⚠️ Not changed: an undo still commits typed spaces as their own event, which is what creates such a branch. Whether
  whitespace alone should ever be an undo step is a separate question.

---

*Last Updated: 2026-10-24 — **I-0321** archived (user-verified, Mac).*

---

## I-0325

**Title:** iOS: the caret jumps to the start of a scene
**Status:** ✅ Verified (2026-10-30)
**Platform:** `[Apple]` iOS / iPadOS
**Component:** `EditorView.container`, the iOS `NavigationSplitView`'s `.onChange(of: selectedSceneID)`
(`Scrivi/Views/EditorView.swift:222–230`)
**Severity:** High. The writer cannot put the caret where they click.
**Sprint:** [SP-198] (AC5 live pass)
**Date Identified:** 2026-10-30, the user's iPad simulator pass on dumas-prose-timelines: *"if I try and position the caret
as the last character in the document, it gets moved to the start of the previous scene. Also a couple of times the caret
moved to other places in the document seemingly at random, or when I start to type."*

**Root cause:** the navigator and the manuscript form a deliberate cycle: a caret move or scroll → `setViewportScene` → the
navigator mirrors it into `selectedSceneID` → `onChange` → `navigateToSceneID` → the manuscript puts the caret at that
scene's start. The Mac breaks the cycle with two guards ([I-0132], `EditorView.swift:297–298`); the iOS branch had neither,
so every viewport change came back as a navigation. The console showed it: `setCurrentIndex` and `setViewportScene`
flipping between scenes (1173 ↔ 1174 at the end of the manuscript).

**Resolution:** the iOS `onChange` takes the Mac's two guards (`isMirroringViewportToSelection`, then the selection equal to
the viewport). Mac code unchanged.
**Files Affected:** `Scrivi/Views/EditorView.swift`
**Verification:** iOS tests 10/10, Mac interop 325/325. ⚠️ No automated test: the cycle runs through SwiftUI's update of a
`NavigationSplitView`. ⏳ User: the iPad live pass — caret at the very end, typing in several scenes, scrolling.

**Re-pass (user, 2026-10-30):** B (caret after the last character) ✅. ⛔ C/D: *"I click and position the caret in one
scene and the highlighted scene in the Scene Navigator is something else entirely. Then I start typing an l and the Scene
Navigator scrolls up to the beginning … None of the l's I typed went where I intended them to go."* The history records
where they went: ONE edit `ll` in scene `…2ea5` at byte 793 (20:02:32) — not the clicked scene, not the manuscript's end.
- **The highlight (explained):** the Navigator highlight is the viewport scene, the scene at the CENTRE of the view, as on
  the Mac (`scrollDidChange`, `ManuscriptTextView.swift:~875`). On iOS the keyboard appearing after a click scrolls the text,
  so the centre moves to another scene; the Mac's click never scrolls. Not yet decided what iOS should do.
- **Where the typing goes (not explained):** reading found nothing that moves the caret per keystroke (`insertText` hands
  a plain letter to UIKit; nothing a keystroke writes triggers `rebuildStorage`). A type-to-select hypothesis was disproven
  (the scene jumped to does not start with "l"). ✅ A Debug-only `[SCRIVI-CARET]` diagnostic now logs every selection
  change (scene, first responder, rebuilding), every `insertText` location and every rebuild, and the call stack of any
  move over 200 characters no keystroke made (`ManuscriptUITextView.swift`). ⏳ The next pass's console decides the fix.

**Pass 3 console (user, 2026-10-30) — the cause, measured:** *"when I click a scene in the Scene Navigator, the scroll does
not put the caret anywhere, and it does not scroll to the center of the viewport, instead opting for either the top or
bottom."* The `[SCRIVI-CARET]` lines: the resume and the Navigator's `navigateToScene` set the caret with the view NOT first
responder; then the writer's tap moved it to **1,848,832 — the manuscript's end** (stack all UIKit); every `l` went there.
- ✅ Measured on the simulator: focus alone keeps the caret (`focusKeepsCaret`). ⛔ After a long programmatic jump with
  `scrollRangeToVisible`, the hit-test at the screen's centre landed **35,507 characters** from the visible text in a
  1.7 MB manuscript WITH scene dividers (none without them): TextKit 2 was left on estimated positions, so a tap resolved
  elsewhere.
- ✅ **Fix — the Mac's shape:** `ManuscriptUITextView.centre(_:)` is the Mac's `centerStorageOffset` — scroll, lay out the
  viewport, measure the target, centre it, lay out again, repeated until it stays put (the Mac re-centres a restore the
  same way). `place()` uses it, so a navigation and the resume CENTRE the scene and leave TextKit 2's layout true. The iOS
  Navigator `onChange` now calls `loader.takeFocus()` after navigating, as the Mac does, so the caret it placed is live.
  Test `jumpCentresAndHitTests`: within one screen (was 35,507 characters away).
- ⚠️ Still open: the highlight is the scene at the view's centre; the keyboard's own scroll can move it. Re-judge after
  this pass. The `[SCRIVI-CARET]` diagnostic stays until the pass confirms.

**Pass 4 (user, 2026-10-30): all six steps pass** — resume centred, Navigator click centres with a live caret, typing lands
where clicked (also after a long scroll), close keeps the place and the typing ([I-0328]). Caveat: *"when the project is
reloaded … the focus originates in the Scene Navigator instead of in the Manuscript. We would prefer it if the writer could
just begin typing."* ✅ `restoreWritingSurface` now calls `loader.takeFocus()` after placing the caret, as the Mac's window
opens with the manuscript first responder. iOS 12/12. ⏳ User: reopen and type at once.

✅ **Verified** (user, 2026-10-30): pass 4 — all six steps; *"Reopen passes, typing works immediately."*

---

## I-0327

**Title:** The Scene Navigator showed an untitled scene's first line raw
**Status:** ✅ Verified (2026-10-30)
**Platform:** `[Apple]` (Mac and iOS: shared code). Linux is not affected: an untitled scene reads "Untitled Scene"
(`platforms/linux/src/EditorShell.cpp`, `rebuildNavigator`) — ⚠️ a parity gap with Apple's first-line titles, not tracked here.
**Component:** the live title — five copies of "first non-blank line as stored" (`ViewportSceneLoader.swift:223`, `:646`,
`:968`; `ManuscriptEditing.swift:822`; `ManuscriptTextView.swift:2193`), shown by `SceneNavigatorView.swift:796` and the
timeline's dot titles (`TimelineStripView.swift:513`)
**Severity:** Medium.
**Sprint:** [SP-198]
**Date Identified:** 2026-10-30, the user on dumas-prose-timelines: the heading formatting *"appears in the Scene Navigator"*.
**Ruling (user, 2026-10-30):** *"The Navigator should show what the manuscript shows and hide what the manuscript hides."*

**Resolution:** `ManuscriptPresenter.presentedFirstLine(of:)` (`ManuscriptPresenter.swift:196–217`): the first line as the
manuscript draws it at rest — escapes, emphasis markers and a heading prefix hidden, a list prefix shown (SP-163 Q7). It reads
the whole first block, so an emphasis pair that closes on a later line is still a pair, and it uses the presenter's own
analysis (`ManuscriptPresenter.analyze`, now static; `info` caches it), so the two cannot drift. All five sites call it. ⛔ Not
`scrivi_markdown_balanced_copy`'s `presented`: that is what OTHER APPS get, and it drops a list's `- `.
**Files Affected:** `Scrivi/Views/ManuscriptPresenter.swift`, `ViewportSceneLoader.swift`, `ManuscriptEditing.swift`,
`ManuscriptTextView.swift`; test `presentedFirstLine` (`Scrivi/Tests/ScriviInteropTests.swift:3802`).
**Verification:** Mac interop 326/326, iOS 10/10. ⏳ User: the Navigator on dumas shows `Youth against age`, no `##`.

✅ **Verified** (user, 2026-10-30): pass 3 step A — the Navigator shows the heading text with no `##`.

## I-0324

**Title:** With real scene links, a project does not open: every relationship endpoint walks the whole manuscript
**Status:** ✅ **Resolved - Verified 2026-11-02** — [SP-197]'s Mac pass (2026-11-01) and [T-0650] on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-10-28)
**Platform:** `[ScriviCore]` (both apps)
**Component:** `EndpointResolver::resolve` (`ScriviCore/src/objects/EndpointResolver.cpp`), called per endpoint by
`RelationshipStore::repairDangling` (every open), `listFor` (a card's links, the Inspector's citations), `listPending`
**Severity:** High. A writer who links scenes to their characters makes the project unopenable at a few thousand links.
**Sprint:** [SP-197] (user, 2026-10-28: *"file the issue and fix it"*)
**Date Identified:** 2026-10-28, generating the Dumas world (87 characters, 21 locations, 21 buildings, 7,091 links) on a
local copy of dumas-prose-timelines (1,175 scenes)

**Measured (local disk, Debug):**
- `scrivi_open_project` on the linked copy: 5+ min at full CPU, stopped. A sample showed
  `repairDangling` → `EndpointResolver::resolve` → `ManuscriptOrderResolver::resolve` → `parseSceneMeta`.
- Linking: about 3 links a second by the end (each create resolves its scene endpoint with a walk).
- `scrivi_load_scene_inspector` with citations: 21,747 file calls, 2.8 s.

**Root cause:** each `resolve` (1) loaded the project index and, on a miss, REBUILT it (`ObjectIndex::find`), (2) resolved
every bound world and read its index, and (3) walked and parsed every scene sidecar, all for ONE endpoint. A scene endpoint
misses (1) and (2) and pays all three. The open's per-call read cache served the repeated reads from memory, so a file count
of the open never showed it; the cost was parsing.

**Resolution:** a resolver remembers the project index (rebuilt at most once), each world's resolution and index, and the
scene list, for its own lifetime: one operation, as every caller makes it locally (the `BindingCache` rule).
`RelationshipStore::listFor` takes an optional shared resolver, and the Inspector's citations share one.
**Files Affected:** `ScriviCore/src/objects/EndpointResolver.{hpp,cpp}`, `ScriviCore/src/objects/RelationshipStore.{hpp,cpp}`,
`ScriviCore/src/public_api/scrivi_c_api.cpp`; tests in `ScriviCore/tests/integration/RelationshipTests.cpp` and
`ObjectCApiTests.cpp`.

**Verification:**
- `[I-0324]` (`RelationshipTests.cpp`): 20 scene links; `repairDangling` and `listFor`, below any cache, read no file more
  than twice (unfixed: 80 and 40). `[I-0324]` (`ObjectCApiTests.cpp`): a card's listing costs the same file calls at 2 and 20
  scene links (unfixed: 112 → 1,084). Both fail against the unfixed core in a copy.
- The linked dumas copy: open + d'Artagnan's 584 links 1.5 s; one scene's Inspector load 49 file calls, or 4,009 with
  citations (one walk, for the far objects' scene titles), 1.3 s.
- ctest 789/789, interop 324/324, Linux smokes 38/38. ⏳ User verification (the rig pass on the linked project).

---

## I-0330

**Title:** The pending-edge sweep walked the whole manuscript on every app activation
**Status:** ✅ Verified 2026-11-01 (user, 2026-11-01: *"The re-check passed, no history calls on main."* The log: `scrivi_history_open` on `[worker]`, no `scrivi_history_get_tree` or `scrivi_list_pending_edges` lines)
**Platform:** `[ScriviCore]` and `[Apple]`
**Severity:** Medium. Background work (T-0523 moved it off the main thread), but 2,538 file operations per activation over a slow
volume, holding the project's lock: the Inspector's read waited up to 382 ms behind it.
**Sprint:** [SP-197]
**Date Identified:** 2026-11-01, from the console of [SP-197]'s Mac live pass: `[worker] core (background):
scrivi_list_pending_edges 371–409 ms, 2538 file ops` on every switch back into the app. User: *"Is it necessary that
scrivi_list_pending_edges does 2538 file operations? Isn't the edge data already loaded into memory by then?"*

**Root cause:** `EndpointResolver::resolve` learned which IDs are scenes by walking the manuscript (every scene sidecar), once
per call since [I-0324], though the open project's index already holds every scene in memory. A scene endpoint also missed the
object index first, which forced that index's rebuild from disk. And the Mac's world-warning strip ran the sweep on every
activation and editor appearance, though it uses the result only to count links into UNAVAILABLE worlds
(`WorldWarningView.swift:76`): with every world present the answer was discarded.

**Resolution:**
1. `SceneLocator::listSceneTitles` (`Services.hpp:218`, default "no hint"); the ABI's locator answers from the warm project
   index (`scrivi_c_api.cpp:699`). The resolver checks scenes from memory first and walks only when there is no index
   (`EndpointResolver.cpp:18`).
2. The strip skips the sweep when no bound world is unavailable (`WorldWarningView.swift:76`, `:116`).
**Verification:** ctest `[I-0330]` (`ObjectCApiTests.cpp:1304`, through `scrivi_*`): the sweep's file operations 12 → 12 from 1 to
21 scenes; a mutation in a copy (the index lookup off) reads 18 → 58 and fails. ctest 791/791, Mac interop 328/328, iOS 17/17,
Linux smokes 38/38 (no Linux caller of the sweep). ⏳ User: a live pass log with no `scrivi_list_pending_edges` on activation
while every world is present.

---

## I-0322

**Title:** Linux has no project-drive-lost handling ([T-0570]'s Linux half was never built)
**Status:** ✅ **Resolved - Verified 2026-11-02** — folded into [SP-195] (Q3) and built there; [T-0648] passed on the Linux VM (the drive pulled: the red line, *"Some changes can't be saved"*, the held save landing); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🔵 Open (never updated when [SP-195] built it)
**Platform:** `[Linux]`
**Component:** `ScriviWindow::closeEvent` (`platforms/linux/src/ScriviWindow.cpp:690`), `EditorShell` save path
**Severity:** High. The same loss [I-0306] closed on the Mac: with the project's drive pulled, typed edits can be lost on
close or quit with no question asked.
**Sprint:** Not Assigned
**Date Identified:** 2026-10-24, by reading, while fixing [I-0306] in [SP-193]. Filed by user request (2026-10-25:
*"yes, file both Issues"*).

**What is missing:** [T-0570] (verified 2026-09-30, Apple) gives the Mac three things: the red non-dismissable
*"Project File Not Available"* banner, dirty scenes kept and saved automatically on reconnect, and a Cancel / Quit Anyway
question on close and quit. ⚠️ Linux has none of them. `closeEvent` calls `flushEditor()` and closes; nothing checks
whether the writes landed. ⚠️ What a failed flush does on Linux (logged, silent, retried) is not yet measured.

**Fix direction (not ruled):** port [T-0570]'s shape from Apple (`feedback_linux_adopts_apple_shape`), including the
close guard through every close path (the X, Close Project, Quit; [I-0306]'s lesson). Measure on the rig with the
drive pulled first.

---

---

## I-0328

**Title:** Closing a project saved nothing: recent typing lost, resume place not stamped
**Status:** ✅ Verified 2026-11-03 (user)
**Platform:** `[Apple]` — Mac and iOS (shared `ProjectSession`)
**Component:** `ProjectSession.close()` (`Scrivi/App/ProjectSession.swift:394`), reached by the Mac's red button / ⌘W
(`windowWillClose → didCloseProjectWindow → closeProject`) and iOS's File ▸ Close Project (`AppEnvironment.closeProject`)
**Severity:** High. Silent loss of the writer's last second of typing on every close.
**Sprint:** [SP-198]
**Date Identified:** 2026-10-30, live pass step I on the iPad simulator: *"Position at the end of the manuscript … Cmd-W.
Open Project. I'm in Scene 17 of Chapter 49."* The console: the reopen resolved scene index 1014, where the session had
OPENED; no `quit:` / `stampWritingSurface` line at the close (they appear only at `onAppResign`).

**Root cause:** `close()` committed and closed the history and released the session, but never ran
`saveAllDirtyBlocking()` — the history flush, the dirty-scene writes, the resume stamp and the wait for the write queue that
`applicationWillTerminate` and `willResignActive` do. A scene typed into within the 1 s autosave was dropped with the loader,
and the resume place was whatever the last save had stamped. The test showed the typing loss on the Mac as well.

**Resolution:** `close()` runs `saveAllDirtyBlocking()` first (`ProjectSession.swift:409–412`), so every close path saves
and stamps exactly as a quit does. The Mac's [I-0305] wait has already let background writes land before this runs.
**Files Affected:** `Scrivi/App/ProjectSession.swift`; test `closeKeepsThePlace` (`ScriviInteropTests.swift:7869`).
**Verification:** `closeKeepsThePlace` (type into scene 2, move to scene 3, close, reopen in a new session) failed before
the fix — the reopened scene held `"Bravo"`, not `"BravoX"` — and passes after. Mac interop 327/327, iOS 10/10.
⏳ User: iPad — leave the caret somewhere, ⌘W, Open Project; type, ⌘W at once, reopen, the typing is there. Mac: the same.
✅ **iPad half passed** (user, 2026-10-30, pass 4 step 5). ⏳ **Mac half not yet run** — the fix is in shared code, so it needs its own check.

---

## I-0326

**Title:** Replace All of a heading's first word deletes the heading's `## `
**Status:** ✅ Verified 2026-11-03 (user)
**Platform:** `[Apple]` (both Mac and iOS: shared code) and `[Linux]` (the same rule, ported)
**Component:** `MarkdownEscapes.isUnreachable` (`Scrivi/Views/ManuscriptEscapes.swift:176–182`), used by `snapSelection`,
which `applyReplacements` (`ManuscriptTextView.swift:2480`) and the edit verdict (`ManuscriptEditing.swift:444`) call;
Linux `ManuscriptEditor::isUnreachable` (`platforms/linux/src/ManuscriptEditor.cpp:172`)
**Severity:** High. Silent loss of the writer's formatting across hundreds of scenes in one step.
**Sprint:** [SP-198]
**Date Identified:** 2026-10-30. The user: *"in the dumas-prose-timelines project many (and I mean a lot) of the Scene headers
have lost their formatting."* On disk, 542 of 1,175 scenes begin `the claim stated plainly` where the fixture generator
(`ScriviCore/tools/scrivi_make_test_world.cpp:231`) writes `## The claim stated plainly`. Exactly the topics that start
"The" are affected, 47–48 scenes each, and `They` → `they` in body text: a case-sensitive Replace All of `The` → `the`
from an earlier live pass. The iCloud copy and the rig's copy are identical.

**Root cause:** a SELECTION never starts just past a stop run (`snapSelection`, E2-S2), so ⌘B on a bold word takes the `**`
with it. A heading prefix `## ` is a stop run too, so a match at a heading's first presented character (storage 3..<6) was
widened to 0..<6 and replaced, `## ` included. The same rule applied to a writer's own selection typed over or cut.

**Resolution:** `StopRun` carries `blockPrefix` (a heading or list prefix); the boundary just past one is the paragraph's
first character, so it is reachable. An emphasis opener is still taken in (⌘B's toggle). Linux: the same exemption.
**Files Affected:** `Scrivi/Views/ManuscriptEscapes.swift`, `Scrivi/Views/ManuscriptEditing.swift` (`runLookup`),
`platforms/linux/src/ManuscriptEditor.cpp`; tests `Scrivi/Tests/ScriviInteropTests.swift` (`replaceAllKeepsHeading`, and a
`snapSelection` case in the stop-run test), `platforms/linux/tests/replace_smoke.cpp` (a heading seed scene).
**Verification:** `replaceAllKeepsHeading` failed before the fix with the dumas damage exactly (`"the claim\n\nthey said."`)
and passes after. Mac interop 325/325, iOS 10/10.
**Fixture repaired (user: *"yes, please repair the document"*, 2026-10-30):** both copies (iCloud and `~/ScriviLinux/projects`)
restored from their OWN history logs: the two 2026-10-26 Replace All groups (`The` → `teh`, then `teh` → `the`) undone per
scene, each undo checked byte-for-byte against the recorded inserted text; the writer's later typing replayed on top. 1,167
scenes restored per copy, 0 skipped; one scene's two keystrokes (typing `##` and deleting it — an attempt at this same
heading) were not replayed. Backups: `dumas-manuscript-before-repair-2026-10-30.tar.gz` beside each package. ⚠️ Sidecar
`stats` were not recomputed; the next open records an `externalChange` barrier per scene (design §6.b), so undo stops there.
⏳ User: Replace All of a heading's first word on the Mac; a selection from a heading's start typed over.
➡️ The Linux half → [T-0651] ([EP-057], the rig); this Issue may be Verified on the Mac steps alone.

*Last Updated: 2026-11-03 — **I-0326, I-0328** archived (user-verified: Mac D1–D4; I-0328 also iPad and iPhone).*
