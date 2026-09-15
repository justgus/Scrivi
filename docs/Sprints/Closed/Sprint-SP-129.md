---
sprint: SP-129
epic: EP-040
status: CLOSED 2026-09-15 (user-approved)
platform: Apple + ScriviCore + Linux
---

# Closed Sprint — SP-129 — `[Apple]`+`[ScriviCore]` The unbuilt surfaces, and the bypass they exposed

**Epic:** [EP-040](../../Epics/Epic-active.md) — `[Apple]` The Editor Shell
**Activated:** 2026-09-15 · **Closed:** 2026-09-15 · **Status:** ✅ **CLOSED — user-approved**
✅ **T-0502 / T-0503 / T-0506 + [I-0214] VERIFIED by the user's LIVE PASS.**
⛔ **T-0504 / T-0505 DEFERRED with their design questions written** — ⚠️ **they do NOT carry forward as
unfinished work; ✅ they are design questions awaiting a ruling, recorded in this file.**

⚠️ **SCOPE CHANGED AT ACTIVATION, BY USER RULING.** ✅ **Planned as Swift-only; ⚠️ it is
`[Apple]`+`[ScriviCore]`+`[Linux]`** — ✅ **because the premise of T-0502 turned out to be wrong.**

## ⚠️ The finding that changed the sprint

⚠️ **T-0502 said "delete the `FileManager`/`JSONDecoder` bypass — the endpoint is already bound."**
⛔ **THE ENDPOINT COULD NOT FEED THE SURFACE.** ✅ **`listImportedTimelines` projected `eventCount`
but NOT the events**, ⚠️ **so a straight swap would have drawn EVERY imported timeline EMPTY** —
✅ **a silent data-loss-looking defect, not a build error.**

⚠️ **AND LINUX HAD THE SAME BYPASS**, for the same reason. ✅ **Its helper's own comment admitted it:**
*"the full record … which the list endpoint does not — so reads go straight to disk (Apple's pattern)."*
⚠️ **"Apple's pattern" was the standing-rule violation this sprint exists to remove.**

✅ **USER RULING: extend the CORE projection** rather than record an exception. ✅ **Both platforms
then drop their file reads** — ⚠️ **which is why this sprint touches Linux at all.**

## Tasks

| ID | Task | Status |
| -- | ---- | ------ |
| **T-0502** | ✅ **Route imported timelines through `scrivi_list_imported_timelines`** — ⚠️ **required EXTENDING the core projection to carry `events` with `projectOffsetMs` pre-resolved** | ✅ **VERIFIED** |
| **T-0503** | ✅ **Imported-timeline OFFSET editing on Apple** — mirrors Linux's `EpochOffsetDialog` | ✅ **VERIFIED** |
| **T-0504** | ⚠️ **Epoch-label editing** — ⛔ **DEFERRED, design question recorded below** | ⛔ **Deferred** |
| **T-0505** | ⚠️ **Object promotion** — ⛔ **DEFERRED, design question recorded below** | ⛔ **Deferred** |
| **T-0506** | ✅ **Audit the remaining direct-filesystem call sites** | ✅ **VERIFIED** |
| **[I-0214]** | ⚠️ **A malformed imported timeline is discarded in SILENCE** — ✅ **FOUND BY THE USER during this sprint's live pass** | ✅ **VERIFIED** |

## What changed

### T-0502 — the bypass, on BOTH platforms

✅ **`ScriviCore.cpp` `listImportedTimelines`** now emits `events[]` per timeline, each carrying
`eventID`, `title`, `offsetMs`, **`projectOffsetMs`** (= `offsetMs + epochOffsetMs`, ✅ **resolved in
the core**), `kind`, `notes`. ✅ **`eventCount` is RETAINED** — existing callers read it.

⚠️ **`projectOffsetMs` is pre-resolved deliberately:** ✅ **both platforms were computing that sum by
hand, identically.** ⚠️ **Two copies of one rule is how they drift.**

- ✅ **Apple** (`TimelineStripView.swift`): `loadImportedTimelines` no longer walks
  `objects/imported-timelines/`. ⛔ **`FileManager.contentsOfDirectory`, `Data(contentsOf:)` and the
  `ImportedTimelineFile` decoder are GONE**; it decodes one envelope.
  ⚠️ **On failure it leaves existing rows ALONE** — ⛔ **a failed read must not look to a writer like
  "your imported timelines are gone."**
- ✅ **Linux** (`EditorShell.cpp`): `reloadImportedTimelines` and `onEditImportedOffsetRequested` both
  read the projection. ✅ **`readImportedTimelineFile` DELETED**, ⚠️ **with a tombstone comment so it
  is not reintroduced.**

✅ **BOUNDARY TEST, not a facade test** (`feedback_boundary_tests_not_facade`):
`TimelineTests.cpp` — *"listImportedTimelines projects events with resolved projectOffsetMs"*.
⚠️ **It asserts the epoch offset is APPLIED (`kEpochOffset + 2000`), not merely carried** — ✅ **if
the projection ever stops carrying events BOTH platforms silently draw empty rows rather than failing.**

### T-0503 — offset editing on Apple

⚠️ **The epoch offset could only be set AT IMPORT.** ⚠️ **A writer who misjudged it had to REMOVE and
RE-IMPORT the timeline.** ✅ **The imported row label now has a context menu** — *Adjust Epoch Offset…*,
*Hide This Timeline*, *Remove Imported Timeline* — ✅ **reusing the existing `EpochOffsetDialog` in an
edit mode**, calling `updateImportedTimelineOffset`.

⚠️ **In edit mode the grey shade is NOT reassigned.** ✅ **It is how the writer recognises the row;
changing it on an offset edit would be a second change she did not ask for.**

### T-0506 — the remaining call sites

✅ **ONE remains: `InspectorLayoutStore.swift:173` reads `inspector-layout.json`.**
✅ **RECORDED EXCEPTION, and it is correct:** ⚠️ **no `scrivi_*` endpoint covers inspector layout
because it is UI STATE, not project data** — ✅ **the core has no opinion about which inspector cards
a writer has open.** ✅ **It already fails safe: a corrupt file falls back to defaults, reports the
error, and ⛔ does NOT overwrite the file.**

⛔ **ZERO direct-filesystem reads of PROJECT DATA remain in `Scrivi/Views` + `Scrivi/App`.**

## ⛔ T-0504 / T-0505 — deferred, with the design questions

✅ **SP-129's own Definition of Done permits this:** *"BUILT from a ruled design, or DEFERRED with the
design question written down — NOT built as placeholders to close a checkbox."*

### T-0504 — epoch-label editing (`setTimelineEpochLabel`)

⚠️ **NEITHER platform has ever surfaced this.** ⚠️ **Open questions:**
1. ⚠️ **WHOSE label?** ✅ **The endpoint sets the PROJECT's epoch label;** ⚠️ **each IMPORTED timeline
   also carries its own `epochLabel`.** ⚠️ **A single "Epoch Label…" command is ambiguous on a panel
   showing both.**
2. ⚠️ **What does the label DO?** ⚠️ **Today it is displayed text.** ⚠️ **If it is to anchor a real
   calendar, that is a Timeline Panel design change, not a text field.**

### T-0505 — object promotion (`promoteObject`)

⚠️ **It MOVES a writer's object from the project package into a world package.** ⚠️ **Open questions
that must be ruled BEFORE a button exists:**
1. ⚠️ **World unavailable mid-promote** — ⚠️ **the write is cross-package;** ✅ **`project_unlinked_world_objects_are_normal`
   says keep the object and report honestly, ⛔ never roll back into a sweep.**
2. ⚠️ **Duplicate identity** — ⚠️ **what if the world already holds that objectID?**
3. ⚠️ **Is it REVERSIBLE?** ⚠️ **There is no `demoteObject`.** ⚠️ **A one-way move needs to SAY so.**
4. ⚠️ **What happens to existing edges** pointing at the project-scoped object?

## ⚠️ [I-0214] — found by the USER, mid-sprint, and it is T-0502's regression

⚠️ **THE USER COULD NOT RUN THE LIVE PASS: the imported timelines were GONE.**
⚠️ **MY FIRST DIAGNOSIS WAS WRONG** — ✅ **I said the files "were always malformed".**
⛔ **THE USER REJECTED IT ON EVIDENCE: "the last time I ran Scrivi, the timelines were there."**
✅ **THEY WERE RIGHT.** ✅ **PROVEN by running BOTH decoders over the SAME four files:**

| reader | result |
| ------ | ------ |
| ⚠️ **old Swift bypass** (what shipped before T-0502) | ✅ **4 rows, 157 events** |
| ⚠️ **core `parseExternalTimeline`** (what T-0502 routed to) | ⛔ **0 rows — all four rejected** |

✅ **THE FILES NEVER CHANGED; THE READER DID.** ⚠️ **The old Swift decoder's `CodingKeys` never
included `schema`, so it never validated one.** ✅ **The core has required
`scrivi.externalTimeline.v1` since SP-040** — ⚠️ **nothing had ever reached that guard.**

⚠️ **TWO FAULTS, BOTH MINE:**
1. ⛔ **The fixtures were invalid.** ⚠️ **I hand-wrote them into `objects/imported-timelines/` on
   2026-09-14 instead of importing them** — ⚠️ **so they never got a `schema` key, and their
   filenames don't match the importer's `<timelineID>-<slug>` pattern either.**
2. ⛔ **THE REAL DEFECT: the core said NOTHING.** ⚠️ **Two bare `continue`s swallowed every failure.**
   ⚠️ **Four files failed and the panel drew an empty strip in silence.**

⚠️ **AND MY OWN TEST COULD NOT HAVE CAUGHT IT:** ✅ **it fed the parser fixtures the CORE had
written, which of course carry the schema key.** ⚠️ **It proved the projection carries events; it
never proved the parser accepts a file the core did not author.**

### The fix

✅ **`ListImportedTimelinesResult` gains `rejected` (path + reason);** ✅ **the C ABI emits
`rejectedCount` ALWAYS — including `0`** — ⚠️ **so a caller can branch without the empty-array trap.**
⛔ **A bad file still does NOT fail the call:** ⚠️ **one corrupt import must not hide every good one.**

- ✅ **Apple: a passive warning strip** in the timeline panel, ⚠️ **naming the FILE and the reason**
  ("an import failed" is unactionable). ✅ **Mirrors `WorldWarningView`;** ⛔ **nothing destructive —
  the files may be hand-authored or from another tool.**
- ⚠️ **Linux: LOGGED ONLY, and that is a RECORDED GAP.** ⛔ **Linux has no passive warning strip, and
  a `QMessageBox` would be MODAL on every load — worse than the silence it replaces.**

✅ **The four fixture files were REPAIRED** (schema key added; ✅ **backup in `/tmp/i0214-backup`**).
✅ **VERIFIED through the real endpoint: `count: 4`, `rejectedCount: 0`, `157` events** — ✅ **exactly
what the old decoder read.**

## Verification

✅ **`ctest` 598/598** (⚠️ **including the new boundary test; ⚠️ re-configure was required — CTest
discovers at CONFIGURE time, so a new `TEST_CASE` is invisible to `-R` until `cmake -S . -B build` reruns**).
✅ **`xcodebuild` BUILD SUCCEEDED — macOS, iOS (`ScriviApp-iOS`), visionOS (`ScriviApp-visionOS`).**
⚠️ **The DoD's "iOS/visionOS green" needs the SEPARATE schemes** — ⚠️ **`ScriviApp` is macOS-only, so a
simulator destination against it fails with a platform-mismatch error that READS like a missing simulator.**

✅ **LINUX BUILT AND SMOKE-TESTED** (Docker, 2026-09-15): ✅ **316/316 targets, ⛔ zero warnings,
✅ 23/23 smoke tests pass.** ✅ **`bridge_parity_smoke`: 28 checks, 0 failures.**
⚠️ **`dumas_world_fixture` is a FIXTURE GENERATOR, not a test** — ✅ **run separately with a project
dir; it succeeds.**

✅ **THE LINUX SMOKE WAS EXTENDED, because it did not cover what changed.**
⚠️ **`timeline_events_smoke` asserted `eventCount` but NEVER that the projection carries the EVENTS** —
⚠️ **exactly the contract T-0502 introduced.** ✅ **It now asserts `events.size()` AND that
`projectOffsetMs == offsetMs + epochOffsetMs`.**

✅ **THE NEW ASSERTION WAS PROVEN TO RUN, NOT MERELY TO PASS.** ⚠️ **A passing check can be vacuous —
an empty array satisfies a guarded block.** ✅ **Deliberately broken to `== 99`, it FAILED
(`FAIL: projection carries the 2 events`); ✅ restored, it passes.**

✅ **LIVE PASS COMPLETE — USER, 2026-09-15. ALL FIVE CHECKS PASSED:**
1. ✅ **The dots came back** — ⚠️ **the T-0502 regression risk, where a row renders EMPTY.**
2. ✅ **Offsets still align** — ⚠️ **the arithmetic moved from Swift into C++.**
3. ✅ **T-0503's context menu is REACHABLE** — ⚠️ **the hit target was the open question; a
   `.contextMenu` on a `.position()`-ed `Text` is only as big as the text.**
4. ✅ **Adjusting the offset works AND the grey shade is preserved.**
5. ✅ **[I-0214]'s banner names the bad file, and the other three timelines still draw.**

✅ **THE USER ALSO EXERCISED HIDE *AND* UNHIDE** (via the pull-down) — ⚠️ **not in the written script;
✅ all four rows came back `visible=true`.**

⚠️ **ONE SIDE-EFFECT, REPAIRED:** ⚠️ **the live pass left Iron Mask's epoch at `315360000000`.**
✅ **Restored to `1898677152000` THROUGH `scrivi_update_imported_timeline_offset`** — ⚠️ **not by
editing the file** — ✅ **so the restore was itself a second exercise of T-0503's endpoint.**
✅ **All four epochs now match the originals; `count: 4`, `rejectedCount: 0`, `157` events.**

⛔ **STILL NOT VERIFIED:**
- ⚠️ **The Linux GUI was not run** — ✅ **smokes are offscreen/headless.** ⚠️ **[I-0214]'s Linux
  surface remains a RECORDED GAP (logged only).**
