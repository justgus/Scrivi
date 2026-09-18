---
sprint: SP-130
epic: EP-040
status: Active
platform: Apple
---

# Sprint SP-130 — `[Apple]` Rule the asset presence-checks (Class C of [I-0197])

**Epic:** [EP-040](../Epics/Epic-active.md) · **Activated:** 2026-09-15 · **Status:** 🟡 **ACTIVE**
**Task:** **T-0508** · **Issue:** [I-0197] (Class C only)

⚠️ **RESTRUCTURED 2026-09-15 (user-ruled).** ✅ **SP-130 was ONE sprint of four tasks; ⚠️ it is now
FIVE sprints** — ✅ **see [`Sprint-SP-130-RESTRUCTURE.md`](Sprint-SP-130-RESTRUCTURE.md) for why.**
✅ **This sprint keeps ONLY T-0508**, ⚠️ **the one piece that blocks on nothing and decides nothing.**

## ⛔ T-0509 IS STRUCK — its remainder is provably ZERO

⚠️ **T-0509 was "sweep the remaining hits in the 11 files", written 2026-09-10 when the count was 35.**
✅ **RE-MEASURED 2026-09-15: 10 hits, 6 files, and they partition EXACTLY:**

| bucket | n |
| ------ | - |
| T-0507 — `InspectorLayoutStore` (Class B) | 3 |
| T-0508 — `fileExists` (Class C) | **5** |
| Recorded non-violations | 2 |
| ⛔ **T-0509 remainder** | **0** |

⚠️ **SP-129 and prior work consumed the sweep.** ⛔ **T-0509 is STRUCK, not deferred** — ✅ **there is
no work behind it.**

## T-0508 — the five Class C sites

| file:line | ⚠️ context |
| --------- | ---------- |
| `ObjectCard.swift:980,984` | ✅ **ALREADY inside `Task.detached(.utility)`** — ⛔ **NOT on the UI thread** |
| `ObjectImageSection.swift:106,137` | ⚠️ **In a view builder** (`preview(_:)`) — ✅ **already comments that a missing file means an UNAVAILABLE WORLD, not damage** |
| `ExistingAssetPicker.swift:130` | ⚠️ **In a `@ViewBuilder` thumbnail** — ✅ **already comments that a failed load is EXPECTED, not corruption** |

⚠️ **THE PLANNING TEXT MISJUDGED THIS.** ⚠️ **It warned these are "the BLOCKING-STAT pattern that cost
[I-0193] 102 s".** ✅ **Two are already off-thread and three already degrade gracefully with the
reasoning written in-line.** ⚠️ **So this is mostly a RULING TO RECORD, not a defect to fix** — ⛔ **but
the ruling must be WRITTEN, because "whatever the code happens to do" is how it drifts.**

---

## ⚠️ OUTCOME (2026-09-16) — ⛔ **THE ABOVE PARAGRAPH IS ITSELF WRONG, AND THAT IS THE SPRINT'S RESULT**

✅ **The ruling is written: [`../Scrivi_Asset_Presence_Check_Ruling_v0_1.md`](../Scrivi_Asset_Presence_Check_Ruling_v0_1.md).**
⚠️ **Writing it forced the question to SPLIT IN TWO, and the split is what found the defect:**

| question | asks | ✅ answered by |
| -------- | ---- | ------------- |
| ⚠️ **ARCHITECTURE** | is this backend logic in Swift? | ⚠️ what the check is USED FOR |
| ⚠️ **COST** | does it block the main actor? | ⚠️ WHERE it runs |

⛔ **"Already degrades gracefully" answers ONLY the first.** ⚠️ **A site can say exactly the right
thing to the writer and still freeze the window for 102 s while deciding to say it** — ✅ **and that
conflation is precisely how `ExistingAssetPicker.swift:130` passed the 2026-09-15 re-measurement.**

### ✅ R1 — architecture: all five are LEGITIMATE

✅ **A `fileExists` on a path the CORE ALREADY RESOLVED is not a ScriviCore bypass.** ⚠️ **Narrowly:
the path came out of `scrivi_list_assets` or a resolved `imagePath`; nothing composes a package path,
parses a schema, or writes.** ⛔ **R1 does NOT license "Swift may stat inside the package"** —
⚠️ **a stat on a path SWIFT composed is Class B and is [SP-141]'s.**

### ⚠️ R2 — cost: an asset presence-check MUST NOT run on the main actor

✅ **Not a new rule** — ⚠️ **`ObjectCard.ObjectRowThumbnail` already states and obeys it** (EP-034
trade **D8-A**: *"a 4 MB PNG on a sleeping USB drive can take seconds; the inspector must stay live"*).
✅ **R2 just writes it where the other four sites can be measured against it**
(`feedback_look_for_existing_pattern_first`).

### ✅ Disposition

| # | site | R1 | R2 | result |
| - | ---- | -- | -- | ------ |
| 1–2 | `ObjectCard.swift:980,984` | ✅ | ✅ **in `Task.detached(.utility)`** | ⛔ **NO CHANGE** |
| 3–4 | `ObjectImageSection.swift:106,137` | ✅ | ✅ **bounded** | ⛔ **NO CHANGE** |
| 5 | `ExistingAssetPicker.swift:130` | ✅ | ⛔ **VIOLATES R2** | ⚠️ **FIXED** |

⚠️ **Sites 3–4 are safe for a reason the planning text did NOT give.** ⛔ **Not "they degrade
gracefully"** — ✅ **they are UNREACHABLE when the volume is away.** ⚠️ **`preview(path)` runs only
under `if let path = resolvedPath`, which is non-nil only when the CORE resolved the path;
world-away takes the `hasUnresolvableImage` branch instead.** ✅ **So the unbounded case cannot arise.**

⛔ **Site 5 had NO such gate.** ⚠️ **`asset.assetPath` comes straight from `listAssets` — a listing,
not a proof of readability** — ⚠️ **and the code ran `fileExists` **AND a full synchronous
`NSImage(contentsOfFile:)` decode**, on the main actor, in a `LazyVStack` row builder, ⚠️ **once per
visible row**, on bytes that live in a WORLD PACKAGE by construction. ⚠️ **Worse than sites 3–4 by a
factor of the row count.** ✅ **Fixed by adopting `ObjectRowThumbnail`'s shape — ⛔ not a second
mechanism.**

## Definition of Done

- [x] ✅ **The Class C ruling is WRITTEN DOWN, per site** — ✅ **R1 + R2 + the five rows.**
- [x] ✅ **Whatever is ruled does NOT block the UI thread** — ✅ **site 5 moved off the main actor;
      1–2 already were; 3–4 proven unreachable in the unbounded case.**
- [x] ✅ **`xcodebuild` green: `ScriviApp` (macOS), `ScriviApp-iOS`, `ScriviApp-visionOS`** — ✅ **all
      three BUILD SUCCEEDED, no new warnings in the changed file.** ✅ **Interop suite: 127 tests,
      12 suites, passed** (run with `SCRIVI_NO_PROJECT_LOAD=1`, per [I-0150]).

⚠️ **NOT PROVEN BY ANY OF THE ABOVE:** ⛔ **no test opens the picker against an unreachable volume.**
✅ **The verification pass is a LIVE one on the real rig** (`feedback_live_pass_finds_what_suites_cannot`):
⚠️ **pull the drive, THEN press *Choose Existing…*.** ✅ **The popover must open immediately.**

## ✅ LIVE PASS 2026-09-16 — ⚠️ **PASSED AT SCALE, with an honest limit**

✅ **Run by the USER in Xcode against a purpose-built 206-image world**
(`~/Desktop/Scrivi-AssetScale-Fixture/`). ⚠️ **NEITHER EXISTING RIG COULD TEST THIS:** ✅ **the picker's
cost is per ROW, and a row is an IMAGE IN THE WORLD** — ⚠️ **Eskandar has SIX; the 1,179-scene Dumas
fixture has none, because scenes are not assets.** ✅ **So a scale fixture had to be built: 200
generated 1024×1024 PNGs at ~1.35 MB, matched to the real assets' size so decode cost is
representative** (⚠️ **flat-colour images compress to a few KB and would have UNDERSTATED it**).

✅ **RESULT: the user scrolled the 206-row picker to the end and back to the middle** — ⚠️ ***"it all
displayed perfectly reasonably."*** ✅ **Thumbnails, the detail-sheet image and every button behaved.**

⚠️ **WHAT THIS DOES *NOT* ESTABLISH — ⛔ there is NO before/after number.** ✅ **Only the FIXED build was
ever run at 206 rows**, ⚠️ **so this is *"the fix performs well at scale"*, NOT a measured comparison
against the blocking version.** ⛔ **Do not report a speedup figure; none was taken.**

## ⚠️ THE PASS COST TWO FALSE STARTS, BOTH MINE — ✅ recorded so they are not re-earned

1. ⛔ **The first fixture was BROKEN and it looked like an app defect.** ⚠️ **I gave the copied world a
   fresh `worldID` to avoid collision, changing `world.json` and `binding.json` — ⛔ but ALL 44 OBJECT
   FILES INSIDE THE WORLD kept the OLD id.** ✅ **`ObjectDetailSheet.isReadOnly` then did exactly its
   job**: Myton's `worldID` was absent from `worlds[]`, so the object was *"bound to a world this
   project cannot see"* → ⚠️ **read-only, every button disabled, and NO banner.** ⚠️ **The user reported
   it as an app fault and was right to** — ✅ **the app was correct; the fixture was not.**
   ✅ **FIXED by reverting the world to the id its own objects carry.**
2. ⚠️ **I chased three wrong theories first** (stale image, sandbox grant, stale scene links),
   ⛔ **each ruled out only AFTER a user run.** ⚠️ **Root cause of the churn: `ObjectDetailSheet.swift`
   and `ObjectImageSection.swift` contain ZERO logging**, ✅ **so I misread the log's silence as
   evidence the sheet never opened.** ⚠️ **A temporary `[SCRIVI-RO]` probe named the cause on the
   FIRST run after it was added** — ✅ **and has since been removed.**

## ✅ T1 — THE ADVERSARIAL PASS, RUN 2026-09-16 · ⚠️ **THE FIX IS PROVEN**

✅ **Setup: the 206-image world COPIED TO THE USB DRIVE** (`/Volumes/Scrivi-Worlds/`), ⚠️ **the project
left on the Desktop** — ✅ **so only the ASSET path dies when the drive is pulled, isolating exactly
what this Sprint changed.**

⛔ **THE PROCEDURE I WROTE WAS WRONG, AND THE USER CAUGHT IT.** ⚠️ **It said "unplug the drive, THEN
press *Choose Existing…*"** — ⛔ **untestable: the Detail Sheet goes read-only the moment the world
vanishes and DISABLES the button under test**, ⚠️ **so the picker never opens and the changed code is
never reached.** ✅ **A pass that way would have been a FALSE GREEN.** ✅ **Correct order, and the one
actually run: OPEN THE PICKER FIRST, THEN PULL THE DRIVE.** ✅ **[`Sprint-SP-130-VERIFICATION.md`](Sprint-SP-130-VERIFICATION.md) corrected.**

✅ **OBSERVED, with the drive pulled out from under an open 206-row picker:**

| | ✅ result |
| - | -------- |
| ⚠️ **the picker** | ✅ **REMAINED ACTIVE — did not freeze, did not close** |
| ⚠️ **scrolling it** | ✅ ***"quietly showed blank thumbnails"*** — ⛔ **no spinner, no broken-image glyph** |
| ⚠️ **already-loaded thumbnails** | ✅ **unloaded quietly as rows recycled** |
| ⚠️ **responsiveness** | ✅ ***"Performance was perfect."*** |
| ⚠️ **the rest of the app** | ✅ **Scene Inspector invalidated its objects, surfaced *"World is unavailable"*, and updated the Detail Sheet** |

✅ **THIS IS THE R2 CRITERION MET: the pass test is RESPONSIVENESS, NOT PICTURES.** ⚠️ **No thumbnail
CAN load — the bytes are gone** — ✅ **and the surface degraded to the `photo` placeholder exactly as
trade D8-A requires.** ⚠️ **Under the OLD code this is where each visible row would have blocked the
main actor on a `fileExists` + 1.35 MB decode against a dead mount** ([I-0193]'s 102 s, ⚠️ **once per
row**).

⚠️ **STILL NO BEFORE/AFTER NUMBER — ⛔ the blocking build was never run at 206 rows on a dead mount.**
✅ **What IS established: the fixed build stays live in the exact condition that defines the defect.**

✅ **BONUS — the world-unavailability path behaved correctly throughout**, ⚠️ **which was not what this
Sprint changed but is the surrounding behaviour it depends on** ([EP-031] AC23/AC24).

## ✅ TWO DEFECTS FOUND BY THE PASS ITSELF

- ⚠️ **[I-0218]** (filed) — ✅ **the picker is UNORDERED**: ⚠️ **neither `AssetStore::list` nor
  `ExistingAssetPicker` sorts, so it renders raw filesystem order.** ⚠️ **Invisible at 6 images;
  a hunt at 206** — ✅ **the user found it immediately.**
- ⚠️ **A `try?` MASKING BUG in `EditorView.detailWorlds`** — ⛔ **a thrown `listWorlds` returned `[]`
  SILENTLY, which `isReadOnly` reads as "world not found", making EVERY world-scoped object read-only
  with no banner and no way to tell a failed call from an unavailable world.** ✅ **Fixed in place: the
  throw is now named in the log rather than swallowed.**

## ✅ Verification procedure

✅ **Setup / Test / Teardown: [`Sprint-SP-130-VERIFICATION.md`](Sprint-SP-130-VERIFICATION.md).**

⚠️ **TWO THINGS IT FOUND BEFORE IT COULD BE WRITTEN, both blocking:**
1. ⛔ **Eskandar does NOT currently resolve.** ⚠️ **`binding.json` records
   `/Volumes/Scrivi Worlds/` (SPACE); the drive is mounted `/Volumes/Scrivi-Worlds` (HYPHEN) and the
   space-named path does not exist** — ✅ **the drive was renamed at some point.** ⚠️ **An unavailable
   world means `listAssets` returns nothing and the picker opens EMPTY, testing NOTHING.**
   ✅ **Setup S3 fixes this by renaming the VOLUME (or via `Manage Worlds…`)** — ⛔ **never by hand-editing
   `binding.json`, which is the very defect class [I-0197] exists to close.**
2. ⚠️ **The change is INVISIBLE on a healthy volume.** ✅ **Old and new code are indistinguishable when
   the stat costs a few ms** — ⛔ **so "the picker still works" verifies nothing, and the adversarial
   case (drive pulled while the sheet is open) is the only discriminating test.**

⛔ **NO GUARD IN THIS SPRINT.** ✅ **T-0510 is [SP-143]** — ⚠️ **a guard written now would have to
allow-list `InspectorLayoutStore`, then be EDITED when [SP-141] removes it.** ⚠️ **It would pass green
across the very change it exists to police.**
