# Sprint SP-050: Per-Window Model — Deep-Link Rewrite & EP-018 Verification

**Status:** ✅ Closed
**Epic:** EP-018
**Start Date:** 2026-06-24
**End Date:** 2026-06-25
**Goal:** Finish EP-018 — confirm the deep-link handler is correct on the per-window model (much of
it already landed in T-0194), fix scene-ID matching, cross-reference the open-flow doc, and run
EP-018 R1–R5 verification. Unblocks EP-017 AC5.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0196 | Deep-link handler on the per-window model + scene-`ID` fix (R5); open-flow cross-ref; EP-018 verification | ✅ Verified |

### Acceptance Criteria

- [x] A deep link opens/focuses the target project's window and selects the item by its `scene_…` ID (R5). *(User-verified 2026-06-25.)*
- [x] Reuses `ScriviDeepLink` / `ProjectBookmarkStore` / `SpotlightDonor`; no single-project switch behavior remains.
- [x] `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` cross-references the per-window model. *(§9.6 + open-issue #10 resolved.)*
- [x] EP-018 R1–R5 verified; EP-017 AC5 unblocked.

### Outcome

The substantive deep-link work had largely already landed in **T-0194** (handler routes via the
`OpenProjectRegistry` + `requestOpenWindow`/`ensureOpenAndShow`; no single-project switch remains).
SP-050's real findings:

- **Scene-`ID` fix was already satisfied — no code change.** The `item=scene:<title>` failure mode
  the design doc §6 anticipated cannot occur: the ScriviCore facade emits
  `item=scene:<real scene_… id>` (`ScriviCore.cpp:1011,1021`), `SpotlightDonor` carries it as both
  `uniqueIdentifier` and the `relatedUniqueIdentifier` deep-link URL, `ScriviDeepLink.targetSceneID`
  extracts the real ID (`ScriviURL.swift:11-14`, tested at `ScriviInteropTests.swift:768`), and the
  whole navigation chain matches by scene ID (`ViewportSceneLoader.storageOffset(forSceneID:)`,
  `ManuscriptTextView.navigateToScene`). No title-based matching exists anywhere. The fix landed
  implicitly via the EP-017 facade/donor work; T-0196 confirms it rather than re-doing it.
- **Open-flow cross-ref done** — `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` §9.6 added;
  open-issue #10 (multi-window) resolved → points to `Scrivi_PerWindow_Project_Model_Design_v0_1.md`.
- **Doc side-fix** — `Scrivi_Project_Package_Structure_v0_1.md` corrected to the real on-disk format
  (scenes `NNN-<slug>.*`; identity keys `projectID`/`sceneID`/`objectID`/`timelineID`, not `id`).
  Surfaced because the wrong doc names broke the verification greps.

### Verification (user, 2026-06-25, against the running app)

- **R1/R2** — multiple distinct projects, each in its own window. ✅
- **R3** — re-opening an open project focuses its existing window (no duplicate). ✅
- **R4** — relaunch restores all previously-open windows. ✅
- **R5** — assistant launched `scrivi://open?project=…&item=scene:scene_019ec7d3-8202-…` for
  `the-twisted-remains-of-myself.scrivi` chapter-005 / scene 2; app opened/focused the project and
  landed exactly on the target scene ("Liszt"). User confirmed "right where it should be." ✅
- **AC-build** — macOS build/codesign clean; `ctest` 224/224 green; no regression. ✅

### Retrospective

**What went well:**
- Tracing the deep-link path end-to-end (facade → donor → URL parse → navigation) showed the
  "scene-ID fix" was already satisfied, avoiding a needless rewrite. The sprint became a
  verification + documentation pass rather than new code.
- The live R5 check (assistant launching the `scrivi://` URL while the user observed) was a clean way
  to verify the one criterion the user couldn't trigger manually.

**What was harder than expected:**
- The package-structure doc had drifted from the real on-disk format (`scene-NNN.*` vs the real
  `NNN-<slug>.*`; `id` vs `projectID`/`sceneID`). This broke the verification greps and cost a
  detour to inspect a real `.scrivi` package and correct the doc + record a reference memo so it
  won't recur.

**Carried forward:**
- EP-017 AC5 is now unblocked; the Spotlight epic resumes with SP-046 (importer extension) and
  SP-047 (verification).

---

*Closed 2026-06-25 with user approval. EP-018 closed alongside it (all criteria verified).*
