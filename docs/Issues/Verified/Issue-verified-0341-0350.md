# Verified Issues: I-0341 – I-0350

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0331–I-0340 are in [`Issue-verified-0331-0340.md`](Issue-verified-0331-0340.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0341** | `[ScriviCore]` ⚠️ **Opening dumas-prose-timelines-history over the share stalled ~26 s on "Opening undo history…": every file was read 8 KB at a time, one network round trip each.** Found 2026-11-02 on the Linux VM, [T-0653] (EP-057). → [details](#i-0341) | High | — | ✅ **Verified 2026-11-02** (the load pass on the VM (build 1.0.321: history 26.2 → 1.5 s); user: *"I am pleased with the result."*) |
| **I-0342** | `[ScriviCore]` ⚠️ **An open read every scene sidecar THREE times: the relationship repair built the project index from disk, bypassing the open's cache — ~31 s of a 47 s open over a share.** Found 2026-11-02 on the Linux VM. → [details](#i-0342) | High | — | ✅ **Verified 2026-11-02** (the load pass on the VM (build 1.0.321: open 47.5 → 3.3 s); user: *"I am pleased with the result."*) |
| **I-0343** | `[ScriviCore]` ⚠️ **The scene texts read 8 at a time: 5.3 s for dumas's 1,176 over a share; 64 readers read them in 0.27 s.** Found 2026-11-02 on the Linux VM. → [details](#i-0343) | Medium | — | ✅ **Verified 2026-11-02** (the load pass on the VM (build 1.0.321: scene texts 5.3 → ~0.9 s); user: *"I am pleased with the result."*) |
| **I-0344** | `[Linux]` ⚠️ **The load's build on the UI thread read the Inspector layout and the copy buffers from disk — 0.6 s, plus 1.8 s waiting for the project lock behind an Inspector read.** Found 2026-11-02 on the Linux VM. → [details](#i-0344) | Medium | — | ✅ **Verified 2026-11-02** (the load pass on the VM (build 1.0.321: UI-thread build 2.55 s → 57 ms); user: *"I am pleased with the result."*) |
| **I-0346** | `[Linux]` ⛔ **The window stops answering for 350–630 ms after EVERY structural edit over the share** — each stall begins ~150 ms after `applied` and ends ~15–40 ms after the [SP-189] background verification's `verify:` line (its read: ~300 ms over the share); no main-thread core call is logged during it. Found in [T-0661]'s pass, scrivi-vm build 1.0.342. → [details](#i-0346) | High | [SP-189] | ✅ **Verified 2026-11-04** ([T-0661] re-run: Pass) |
| **I-0347** | `[Linux]` **Split Scene moves the caret's line to the TOP of the viewport** (the edit is right; the view jumps). Found in [T-0661] step 1. → [details](#i-0347) | Medium | [SP-190] | ✅ **Verified 2026-11-04** ([T-0661] re-run: Pass) |
| **I-0348** | `[Linux]` ⛔ **After structural edits and a quit, the project reopens in the wrong place** — left at *"Thanksgiving"* in chapter 6's last scene; reopened near chapter 14. Found in [T-0661] step 8. (Apple's [I-0328] was the Mac's close path; this is Linux's quit.) → [details](#i-0348) | High | [SP-190] | ✅ **Verified 2026-11-04** ([T-0661] re-run: Pass) |

---

## I-0341

**Title:** Every file read goes 8 KB at a time — the undo history takes 26 s to read over a share
**Status:** ✅ **Resolved - Verified 2026-11-02** — the load pass on the VM (build 1.0.321: history 26.2 → 1.5 s); user: *"I am pleased with the result."*; archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0653] step 6 (open dumas-prose-timelines-history)
**Platform:** `[ScriviCore]` — every platform reads through it; measured on Linux over SMB
**Severity:** High. Every open of a project with a large history waits for it.
**Date Identified:** 2026-11-02, build 1.0.320 (`3cb4846`), on the VM: *"The initial load of the dumas-prose-timelines-history
seems to stall for about 5-10 seconds 'Opening undo history...'"*. The log: `scrivi_history_open 26167 ms, 6 file ops` for a
36 MB `log-000007.jsonl` (`scrivi-vm:~/evidence-2026-11-02/scrivi-history-open.log`).
**Root cause (measured):** the same open on the VM's local disk: 940 ms — not the replay. The log read over the share in large
sequential blocks (`cat`): 0.33 s; in 4 KB reads: 49 s — on this mount each read is a network round trip.
`LocalFileSystem::readTextFile` copied the file with `ss << in.rdbuf()`, which libstdc++ does through the stream's 8 KB buffer:
~4,400 round trips.
**Resolution:** `readTextFile` reads in 1 MB blocks, which go straight to the kernel
(`ScriviCore/src/platform/LocalFileSystem.cpp:178`).
**Measured (read-only, the share's real log, page cache dropped each time):** `readTextFile` **24,674 ms → 342 ms**.
**Files Affected:** `ScriviCore/src/platform/LocalFileSystem.cpp`; test `ScriviCore/tests/unit/LocalFileSystemTests.cpp`
(every size around the block — 0, 1, 8,191, 1 MB − 1, 1 MB, 1 MB + 1, 3 MB + — reads back byte for byte).
**Verification:** VM scratch copy ctest `-j6` 805/805, smokes 38/38; Mac `build-tests` 801/801.
**Not this defect:** `scrivi_open_project` takes 47 s on the same project (2,881 file operations, ~16 ms each over the share) —
many small files, each its own round trip; reading them in larger blocks does not help. Not filed.

**Measured on the VM (user's load, build 1.0.321 `b5331a8`, fresh SMB connection, 2026-11-02 21:19; `scrivi-vm:~/evidence-2026-11-02/scrivi-after-load-fixes-2.log`):** the whole load **82.0 s → 6.0 s** — open 47.5 → **3.3 s** (2,881 → 220 file ops); scene texts 5.3 → **~0.9 s** (no batch over 100 ms); undo history 26.2 → **1.5 s**; UI-thread build 2,553 → **57 ms**. ⚠️ Still: the window does not answer for 3.2 s after the load ends (Qt's own layout and painting of the 2.9 M-character document). User: *"I am pleased with the result."*

---

## I-0342

**Title:** An open reads every scene sidecar three times when the project has relationship edges
**Status:** ✅ **Resolved - Verified 2026-11-02** — the load pass on the VM (build 1.0.321: open 47.5 → 3.3 s); user: *"I am pleased with the result."*; archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the combined load pass on the VM
**Platform:** `[ScriviCore]` (C ABI) — every platform
**Severity:** High. 47.5 s of the 82 s load over a share.
**Date Identified:** 2026-11-02, build 1.0.320 (`3cb4846`), the user's load of dumas-prose-timelines-history on the VM over SMB (82 s in all; `scrivi-vm:~/evidence-2026-11-02/scrivi-history-open.log`): `scrivi_open_project 47478 ms, 2881 file ops`.
**Measured:** the same open on the VM's local disk: 485 ms — the share's round trips are the cost. `strace`: 3,899 opens, 3 of
every scene and chapter sidecar. `gdb` on one sidecar: (1) the parallel prefetch into the open's cache, as intended; (2)
`listScenesByOrder` and (3) `ManuscriptOrderResolver::resolve`, both under `ProjectIndex::build` ← `EndpointResolver::resolve`
← `RelationshipStore::repairDangling` ← `ProjectOpener::open` — reading `LocalFileSystem` directly.
**Root cause:** the relationship repair resolves a scene endpoint through the project index ([I-0330]), and the ABI's
`RegistrySceneLocator` built it through `abiServices()` — the raw filesystem — not the open's `ReadThroughCache`. The ABI then
discards that index after the open and rebuilds it from the cache.
**Resolution:** `RegistrySceneLocator` takes the filesystem it builds through (`scrivi_c_api.cpp:149`); `scrivi_open_project`
gives its open a locator over the open's cache (`:1001`). The global locator is unchanged.
**Files Affected:** `ScriviCore/src/public_api/scrivi_c_api.cpp`; test `ScriviCore/tests/integration/ObjectIndexRebuildTests.cpp`
(through the C ABI: 40 scenes and a scene edge — an open's `fileOps` stays under the scene count; **112 without the fix, 23
with it**).
**Verification:** the new test FAILED with the open's locator on the raw filesystem; VM ctest 806/806, smokes 38/38; Mac core
802/802, interop 333/333.

**Measured on the VM (user's load, build 1.0.321 `b5331a8`, fresh SMB connection, 2026-11-02 21:19; `scrivi-vm:~/evidence-2026-11-02/scrivi-after-load-fixes-2.log`):** the whole load **82.0 s → 6.0 s** — open 47.5 → **3.3 s** (2,881 → 220 file ops); scene texts 5.3 → **~0.9 s** (no batch over 100 ms); undo history 26.2 → **1.5 s**; UI-thread build 2,553 → **57 ms**. ⚠️ Still: the window does not answer for 3.2 s after the load ends (Qt's own layout and painting of the 2.9 M-character document). User: *"I am pleased with the result."*

---

## I-0343

**Title:** The core reads many files only 8 at a time
**Status:** ✅ **Resolved - Verified 2026-11-02** — the load pass on the VM (build 1.0.321: scene texts 5.3 → ~0.9 s); user: *"I am pleased with the result."*; archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the combined load pass on the VM
**Platform:** `[ScriviCore]` — every platform's bulk scene-text read and the open's sidecar prefetch
**Date Identified:** 2026-11-02, build 1.0.320 (`3cb4846`), the user's load of dumas-prose-timelines-history on the VM over SMB (82 s in all; `scrivi-vm:~/evidence-2026-11-02/scrivi-history-open.log`): 19 × `scrivi_read_scene_texts` of ~280 ms (64 texts each) = 5.3 s.
**Measured (the VM over SMB, dumas's 1,176 scene texts, page cache dropped, read-only):** 8 readers 5,485 ms · 16 2,346 ·
32 927 · **64 266**.
**Resolution:** `kParallelReaders` 8 → 64 (`ScriviCore/src/public_api/scrivi_c_api.cpp:345`); both platforms ask for texts 64
at a time, so each batch is about one round trip.
**Verification:** existing tests (the reads' results are unchanged); VM ctest 806/806, Mac core 802/802, interop 333/333.

**Measured on the VM (user's load, build 1.0.321 `b5331a8`, fresh SMB connection, 2026-11-02 21:19; `scrivi-vm:~/evidence-2026-11-02/scrivi-after-load-fixes-2.log`):** the whole load **82.0 s → 6.0 s** — open 47.5 → **3.3 s** (2,881 → 220 file ops); scene texts 5.3 → **~0.9 s** (no batch over 100 ms); undo history 26.2 → **1.5 s**; UI-thread build 2,553 → **57 ms**. ⚠️ Still: the window does not answer for 3.2 s after the load ends (Qt's own layout and painting of the 2.9 M-character document). User: *"I am pleased with the result."*

---

## I-0344

**Title:** The load's UI-thread build reads the Inspector layout and the copy buffers from disk
**Status:** ✅ **Resolved - Verified 2026-11-02** — the load pass on the VM (build 1.0.321: UI-thread build 2.55 s → 57 ms); user: *"I am pleased with the result."*; archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the combined load pass on the VM
**Platform:** `[Linux]` (the Mac's load not checked here)
**Severity:** Medium. The window was frozen 5.6 s at the end of the load; this was 2.4 s of it.
**Date Identified:** 2026-11-02, build 1.0.320 (`3cb4846`), the user's load of dumas-prose-timelines-history on the VM over SMB (82 s in all; `scrivi-vm:~/evidence-2026-11-02/scrivi-history-open.log`): `[main] core: scrivi_get_inspector_layout 598 ms` and `[main] core: scrivi_buffers_list
33 ms, lock wait 1846 ms`, both inside `load: build the manuscript (main thread)`.
**Root cause:** `SceneInspector::setContext` loaded the layout through the bridge and `BufferService`'s constructor listed the
buffers, both on the UI thread, from `applyLoadedProject`.
**Resolution:** both are read on the load worker, before the timeline read (`scrivi_buffers_list` takes the project write
guard and moves the revision, as the history open does) — `platforms/linux/src/EditorShell.cpp:634` — and handed in:
`SceneInspector::setContext(…, layoutRead, failed)`, `InspectorLayoutStore::load(…, readOnWorker, failed)`, `BufferService(root,
listed)`. ⚠️ A reload of the SAME project reads the layout again on the UI thread after waiting for its own queued save, which a
worker read could predate.
**Not changed:** the build itself (~2.6 s on dumas) — Qt's text document is UI-thread-only; slicing it is a larger change.
**Files Affected:** `EditorShell.cpp/.hpp`, `SceneInspector.cpp/.hpp`, `InspectorLayoutStore.cpp/.hpp`, `BufferService.cpp/.hpp`;
test `platforms/linux/tests/structural_baseline_smoke.cpp` (the load makes no UI-thread `scrivi_get_inspector_layout` or
`scrivi_buffers_list`). Also fixed: an ignored `QFile::open` result in `inspector_layout_smoke.cpp` (a compiler warning).
**Verification (VM scratch copy):** the new check FAILED with the UI-thread reads restored; smokes 38/38, ctest 806/806.

**Measured on the VM (user's load, build 1.0.321 `b5331a8`, fresh SMB connection, 2026-11-02 21:19; `scrivi-vm:~/evidence-2026-11-02/scrivi-after-load-fixes-2.log`):** the whole load **82.0 s → 6.0 s** — open 47.5 → **3.3 s** (2,881 → 220 file ops); scene texts 5.3 → **~0.9 s** (no batch over 100 ms); undo history 26.2 → **1.5 s**; UI-thread build 2,553 → **57 ms**. ⚠️ Still: the window does not answer for 3.2 s after the load ends (Qt's own layout and painting of the 2.9 M-character document). User: *"I am pleased with the result."*

---

---

## I-0346: `[Linux]` The window stops answering after every structural edit over the share

**Found:** 2026-11-04, [T-0661] (the user's pass on scrivi-vm over the 5 ms share, dumas, build 1.0.342 `5087912`).
**Evidence:** `~/evidence-2026-11-04/` and `~/.local/state/scrivi/scrivi.log.1` on scrivi-vm. Every split, merge, chapter split /
merge, paste, cut, Backspace and their undo / redo: `structural "…": applied in ~33 ms, ~330 ms in all`, then the watchdog's
*"the window is not answering"* ~150 ms later, *"answering again"* after **345–632 ms** — each time **15–40 ms after** the
`verify: … read in ~305 ms — 0 corrected` line. No `[main] core:` call is logged during a stall ("running: nothing Scrivi has
marked"). ⚠️ The user saw each step pass; the stall is under a second, but the watchdog counts it, and AC "no freeze" fails.
**Cause (found 2026-11-04):** ⛔ NOT the verification — its `read in ~305 ms` is timed to its landing ON the GUI thread, so it
landed when the GUI thread came free (its real read is a listing + 1–2 files). The GUI thread was in [SP-203]'s accessibility
layer: the VM's accessibility bus was enabled (`org.a11y.Status IsEnabled: true`, even with the Screen Reader switch off), so
Qt sent every edit's events; re-sending one refreshes the page map, which matched its scenes to the list BY POSITION — after a
split or merge every later scene no longer lined up, and the rest of the manuscript was rebuilt (~0.4 s on the VM).
**Fix:** `PresentedText::align` (`ManuscriptFind.cpp`) matches by SCENE ID, and edits are marked by scene ID (positions shift
under a structural edit); Find shares the map and the fix. The re-send is a named span now (`DiagnosticLog`), so a stall there
names itself. **Test:** `accessibility_smoke` — a split mid-book on 1.8 MB: the page equals a fresh build, in **6 ms (a full
build: 506 ms)**; mutations (position matching back; edits marked by position) both caught. ✅ **Verified 2026-11-04** (user, 2026-11-04: *"all the checkes passed"* — [T-0661] re-run on scrivi-vm, build 1.0.348).

## I-0347: `[Linux]` Split Scene moves the caret's line to the top of the viewport

**Found:** 2026-11-04, [T-0661] step 1 (user: *"Split scene moved the caret to the top line of the viewport. Otherwise it
passed."*). The split itself is right; the view scrolls so the caret's line is the first visible line. ⏳ Compare Apple's
caret placement after a split (it keeps the caret's height — T-0573).
**Cause:** Linux placed the caret after a structural edit with `ensureCursorVisible()` only — Apple's T-0573 rule (the caret
keeps its height in the window) was never ported. With dumas's long WRAPPED paragraphs, replacing the run resets where the view
stands inside the top paragraph, and the caret's line ended at the top.
**Fix:** `EditorShell::runRangeEdit` records the caret's height when it is on screen; `EditorShell::placeCaret` scrolls it back
there (to the nearest line — the view scrolls by lines). Every structural edit and its undo / redo goes through them.
**Test:** `structural_baseline_smoke` — five scenes of long wrapped paragraphs, the caret a third of the way down, a split: the
caret within two lines of its height. ✅ Passes on Qt 6.4 and 6.10; with the fix removed it FAILS on both — ⚠️ an earlier
one-scene, short-line version did not reproduce it on either (wrapped paragraphs are what moves the view). ✅ **Verified 2026-11-04** (user, 2026-11-04: *"all the checkes passed"* — [T-0661] re-run on scrivi-vm, build 1.0.348).

## I-0348: `[Linux]` After structural edits and a quit, the project reopens in the wrong place

**Found:** 2026-11-04, [T-0661] step 8 (user: *"The manuscript existed at the work 'Thanksgiving' in the last scene of chapter
6. The project loaded somewhere near the beginning or end of chapter 14."*). Steps 1–7 had split, merged, cut and pasted
scenes in chapters 2–7 before File ▸ Quit. ⏳ Read what the quit stamped (`workspace-state.json`, `lastWritingSurface`) against
the scene the caret was in.
**Cause:** on open, Linux put the caret back in the stamped scene and centred it — then OVERRODE that with the saved
whole-document scroll FRACTION. The fraction is of the scroll range at quit, and `QPlainTextEdit` lays wrapped paragraphs out
lazily, so a fresh open's range is not that one: the view landed away from the caret (dumas: chapters away). Apple never used
the fraction ([I-0133], ruled 2026-08-18: it centres the restored scene). (The 15:56 stamp was overwritten by the 16:00 quit,
so it was read from the code and reproduced, not from the file.)
**Fix:** `EditorShell` restores the caret and CENTRES it; the fraction is no longer applied (Apple's shape). The stamp still
carries it, as Apple's does.
**Test:** `structural_baseline_smoke` — 40 scenes of wrapped paragraphs; a session reads through the book; the caret in scene
30, stamped as quitting does; reopened in a fresh shell: the caret in that scene, ON SCREEN, and the window showing it. ⛔ Before
the fix it FAILED on Qt 6.10 (the caret 287 px above the window). ✅ Passes on Qt 6.4 and 6.10. ✅ **Verified 2026-11-04** (user, 2026-11-04: *"all the checkes passed"* — [T-0661] re-run on scrivi-vm, build 1.0.348).
