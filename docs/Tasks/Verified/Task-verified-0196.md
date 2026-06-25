# T-0196: Deep-link on per-window model + scene-`ID` fix + EP-018 verification

**Status:** ✅ Implemented - Verified
**Component:** `AppEnvironment.swift`, `ScriviURL.swift`, `Scrivi_Project_Creation_and_Open_Flow_v0_2.md`, `Scrivi_Project_Package_Structure_v0_1.md`
**Epic:** EP-018 (final task — closes R5; unblocks EP-017 AC5)
**Sprint:** SP-050
**Date Requested:** 2026-06-24
**Date Implemented:** 2026-06-25
**Date Verified:** 2026-06-25
**Design Reference:** `Scrivi_PerWindow_Project_Model_Design_v0_1.md` §6 (deep-link rewrite + scene-`ID` fix)

**Goal:**
Make a deep link open/focus the target project's window on the new per-window model and select the
target scene by its `scene_…` ID. Cross-reference the per-window model from the open-flow doc. Run
the EP-018 R1–R5 acceptance pass; unblock EP-017 AC5.

**Resolution:**
1. **Scene-`ID` fix — confirmed already correct; no code change.** The `item=scene:<title>` failure
   mode the design doc §6 anticipated cannot occur: the ScriviCore facade emits
   `item=scene:<real scene_… id>` (`ScriviCore.cpp:1011,1021`), `SpotlightDonor` carries it as both
   `uniqueIdentifier` and the `relatedUniqueIdentifier` deep-link URL, `ScriviDeepLink.targetSceneID`
   extracts the real ID (`ScriviURL.swift:11-14`), and the whole navigation chain matches by scene ID
   (`ViewportSceneLoader.storageOffset(forSceneID:)`, `ManuscriptTextView.navigateToScene`). No
   title-based matching exists. Covered by the existing test at `ScriviInteropTests.swift:768`.
2. **Handler on per-window model — confirmed already landed in T-0194.** `handleDeepLink` /
   `handleSpotlightItem` route via `OpenProjectRegistry` + `requestOpenWindow` / `ensureOpenAndShow`
   (`AppEnvironment.swift:310-372`); no single-project switch remains.
3. **Open-flow cross-reference — added.** `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` §9.6 (window
   vs. project restoration scope); open-issue #10 (multi-window) resolved → points to the per-window
   design doc.
4. **Package-structure doc corrected (side-fix).** `Scrivi_Project_Package_Structure_v0_1.md` had wrong
   scene filenames (`scene-NNN.*`) and identity keys (`id`). Corrected to the real on-disk format:
   scenes are `NNN-<slug>.*`; keys are `projectID` / `sceneID` / `objectID` / `timelineID`. Added a
   convention note in §4. (Surfaced because the wrong doc names broke the verification greps.)

**Verification (user, 2026-06-25, against the running app):**
- **R1/R2** — multiple distinct projects, each in its own window. ✅
- **R3** — re-opening an open project focuses its existing window (no duplicate). ✅
- **R4** — relaunch restores all previously-open windows. ✅
- **R5** — assistant launched `scrivi://open?project=…&item=scene:scene_019ec7d3-8202-…` for
  `the-twisted-remains-of-myself.scrivi` chapter-005 / scene 2; app opened/focused the project and
  landed exactly on the target scene ("Liszt" — "I found my apartment wasn't empty…"). User confirmed
  "right where it should be." ✅
- **AC-build** — macOS build/codesign clean; `ctest` 224/224 green; no regression. ✅

**Acceptance Criteria:**
- [x] Deep link opens/focuses the target project's window and selects the item by its `scene_…` ID (R5).
- [x] Reuses `ScriviDeepLink` / `ProjectBookmarkStore` / `SpotlightDonor`; no single-project switch remains.
- [x] `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` cross-references the per-window model.
- [x] EP-018 R1–R5 verified; EP-017 AC5 unblocked.

*Verified by the user 2026-06-25 via manual testing + an assistant-launched deep link.*
