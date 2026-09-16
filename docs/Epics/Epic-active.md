# Active Epics

🟡 **EP-040** — `[Apple]` **The Editor Shell** — ⚠️ **the sole ACTIVE Epic** (activated 2026-09-15).

✅ **EP-039** `[Cross]` Project Load Performance — **CLOSED 2026-09-15**
→ [`Closed/Epic-EP-039.md`](Closed/Epic-EP-039.md) (⚠️ **moved 2026-09-15, audit ruling [R-07]**).
⚠️ **[I-0206] and [I-0213] were CARRIED from it into EP-040 and remain OPEN.**

⚠️ **Every other Epic is in [`Epic-backlog.md`](Epic-backlog.md).**
⛔ **No count is stated here** — ✅ **audit ruling [R-15]; read the rows.**

---

## EP-040: `[Apple]` ⚠️ **The Editor Shell** — ✅ **build Scrivi as the kind of app it actually is**

**Status:** 🟡 **ACTIVE — activated 2026-09-15 (user-approved).**
⚠️ **[EP-039] closed the same day; ✅ [SP-133] remains open under it and is not blocking.**
**Codebase:** `[Apple]` — ⚠️ **`Scrivi/Views/EditorView.swift`, `SceneInspectorView.swift`,
`Detail/ObjectDetailSheet.swift`, `App/ProjectWindowManager.swift`, `App/ScriviApp.swift`.**
✅ **NO ScriviCore change. NO document-format change.**
**Goal:** ⚠️ **A writer can see and reach every part of the editor.** ✅ **The window has real chrome,
the panes are real panes, and no surface hand-builds what the platform provides.**
**Design:** ✅ **[`../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`](../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md)**
(⚠️ **the app-shape ruling, S1–S6**) **+ [`../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md`](../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md)**
(⚠️ **the sourced conformance findings F1–F7**). ✅ **BOTH USER-APPROVED 2026-09-14.**
**Date Created:** 2026-09-14 · **Target Close:** — (⚠️ **estimated 5 sprints; ✅ see the sizing note**)

### ✅ Why this Epic exists

⚠️ **Scrivi's Apple UI was built surface by surface, each solving its own problem.** ⚠️ **It was never
assembled AS A KIND OF APP.** ✅ **[I-0203] is what made that visible: a warning banner appearing made
FOUR pieces of window chrome disappear at once** — ⚠️ **which is not four bugs, it is one structural
assumption being wrong.**

✅ **THE RULING (app-shape study §2):** ⚠️ **Scrivi is a SINGLE-DOCUMENT EDITOR with a navigable
OUTLINE and a CONTEXTUAL INSPECTOR — the Xcode/Scrivener shape.** ⚠️ **It is NOT master/detail.**
✅ **Proven structurally: the Inspector is fed `viewportSceneID` (what you are SCROLLED TO), not
`selectedSceneID` (what you CLICKED)**, ⚠️ **and there is a deliberate loop-breaker between them.**
⚠️ **In master/detail those are the same variable by definition.**

✅ **THE REPEATED DEFECT, THREE INSTANCES DEEP:** ⚠️ **surfaces HAND-BUILD what the platform provides.**

| surface | ⚠️ hand-builds | ✅ the API that exists | step |
| ------- | -------------- | -------------------- | ---- |
| banner / timeline / inspector tab bar | ⚠️ plain `VStack` siblings | ✅ `safeAreaBar` (macOS 26.0+) | **S3** |
| Scene Inspector | ⚠️ `HStack` + `resizeHandle` + `.frame(width:)` | ✅ `.inspector` (macOS **14.0+**) | **S4** |
| Object Detail Sheet | ⚠️ **a toolbar `HStack`, incl. a close button** | ⚠️ **no single target — must be ruled** | **S6** |
| the WINDOW itself | ⚠️ **`.navigationTitle` with NO `NSToolbar` anywhere** | ✅ **`NSToolbar`** | **S1** |

⚠️ **`NSToolbar` appears ZERO times in `Scrivi/Views` + `Scrivi/App`** (✅ **grep-verified**).

### ⚠️ What this Epic is NOT

- ⛔ **NOT a rich-text conversion.** ⚠️ **`ManuscriptTextView.swift:32` sets `isRichText = false` and the
  canonical body is Markdown (`"format": "markdown"`).** ⚠️ **Font/typeface/style controls have nowhere
  to persist and are OUT OF SCOPE** — ✅ **a display-only text-size lens and Markdown semantic marks are
  the fitting forms (app-shape §2.3).** ⚠️ **True rich text would change the on-disk format, the C++
  schema AND Linux/Qt parity: ✅ it is a SEPARATE ruling, not a toolbar decision.**
- ⛔ **NOT a performance Epic.** ✅ **That is [EP-039].** ⚠️ **S1–S3 deliberately do NOT touch
  `ManuscriptTextView`'s internals.**
- ⛔ **NOT iOS/iPadOS/visionOS.** ⚠️ **The iOS branch ALREADY has `.toolbar` (`EditorView.swift:189`)
  and a different shape.** ✅ **It needs its own pass and is explicitly deferred.**
- ⛔ **NOT new features.** ⚠️ **Export and the text-size lens are what the toolbar makes ROOM for** —
  ✅ **named as slots (AC7), NOT built here.**

### Acceptance Criteria

- [ ] **AC1** — ✅ **The project window has a real `NSToolbar`**, ⚠️ **and the window title/subtitle
      render in it** rather than as a stray in-content band. ✅ **Closes conformance F1.**
- [ ] **AC2** — ✅ **The toolbar surfaces the EXISTING verbs** (Scene: New/Merge · Chapter: New/Merge ·
      pane toggles), ⚠️ **calling the SAME `focusedSession?.<verb>Action?()` closures the menu bar
      calls.** ⛔ **No duplicated logic.**
- [ ] **AC3** — ⚠️ **A writer who loses the Navigator, Inspector or Timeline can bring it back FROM THE
      WINDOW**, ✅ **not only from a menu.** ⚠️ **This is [I-0203]'s *"no affordance to bring it back"*.**
- [ ] **AC4** — ✅ **The banner, Timeline and Inspector tab bar are `safeAreaBar`s** (or split-item
      accessories), ⚠️ **NOT `VStack` siblings.** ✅ **Closes conformance F3.**
- [ ] **AC5** — ⚠️ **Showing or dismissing the world-warning banner does NOT disturb any other
      surface.** ✅ **This is [I-0203]'s acceptance test and it is the Epic's headline outcome.**
- [ ] **AC6** — ✅ **The Scene Inspector is a REAL trailing column**, ⚠️ **not an `HStack` member with a
      hand-rolled resize handle.** ⚠️ **User-resizable width MUST survive the conversion** (⚠️ **today
      `@AppStorage("inspectorPaneWidth")`, 220–560pt**).
- [ ] **AC7** — ✅ **The toolbar has DECLARED SLOTS for Export and a text-size lens**, ⚠️ **left
      unimplemented and documented as such.** ⛔ **Building them is out of scope.**
- [ ] **AC8** — ✅ **The Object Detail Sheet no longer hand-builds window chrome**, ⚠️ **per a RULED
      hosting decision (a/b/c in app-shape §4.4).**
- [ ] **AC9** — ⚠️ **[I-0205] is ANSWERED before the banner is restyled** — ✅ **is it correct-but-ugly,
      or FALSE?** ⚠️ **These lead to OPPOSITE fixes and the evidence does not yet distinguish them.**
- [ ] **AC11** — ⚠️ **CARRIED FROM [EP-039] (user ruling 2026-09-15): the manuscript surface's
      remaining O(DOCUMENT) costs are addressed or ACCEPTED AS LIMITATIONS with a measurement.**
      ✅ **[I-0206]** (offset-linear `setSel`) ⚠️ **MUST be tested at the END of the document — ⛔ it
      was wrongly closed once on a sample taken ~1% in.** ✅ **[I-0213]** (chapter create `~305 ms`)
      ⚠️ **needs `reloadSceneDots`'s remaining ~200 ms INSTRUMENTED, not inferred.**
      ⚠️ **These are EP-040's because they live on the EDITOR SHELL, not because EP-039 failed** —
      ✅ **that Epic's goal (a non-freezing open) is met and measured at `0.34 s`.**

- [ ] **AC10** — ✅ **A CI guard prevents regression**: ⚠️ **a new bar added as a bare `VStack` sibling
      should be caught, the way [T-0527] guards the TextKit downgrade.** ⚠️ **⛔ Mechanism NOT yet
      designed — ✅ the guard must be SPECIFIED before it is promised.**

### Sprints — ⚠️ **PROPOSED, none activated**

| Sprint | Step | Title | Status | ⚠️ Risk |
| ------ | ---- | ----- | ------ | ------ |
| **SP-134** | **S1+S2** | ✅ **The toolbar** — `NSToolbar` + title/subtitle + existing verbs | 🔵 **Proposed** | ✅ **LOW** |
| **SP-135** | **S3** | ✅ **The bars** — `safeAreaBar` conversion; ⚠️ **closes [I-0203]** | 🔵 **Proposed** | ✅ **MEDIUM** |
| **SP-136** | **S4** | ⚠️ **The Inspector as a real column** (`.inspector`) | 🔵 **Proposed** | ⚠️ **MED-HIGH** |
| **SP-137** | **S6** | ⚠️ **The Object Detail Sheet** — ⚠️ **hosting RULING first, then chrome** | 🔵 **Proposed** | ⚠️ **MED-HIGH** |
| **SP-138** | **S5** | ⛔ **`NSSplitViewController` rebuild** | ⛔ **NOT SCHEDULED** | ⛔ **HIGH** |
| **SP-129** | ✅ **T-0502, T-0503, T-0506 + [I-0214]** (⛔ **T-0504/T-0505 deferred**) | ⚠️ **The unbuilt surfaces + the `loadImportedTimelines` bypass** — ⚠️ **scope GREW to `[ScriviCore]`+`[Linux]`.** ✅ **Core projection extended; both bypasses deleted** | ✅ **CLOSED 2026-09-15** — [record](../Sprints/Closed/Sprint-SP-129.md) | ✅ **VERIFIED by user LIVE PASS** |
| **SP-130** | ✅ **T-0508** | ⚠️ **Rule the 5 `fileExists` asset sites** (Class C of [I-0197]) | 🟡 **ACTIVE** (2026-09-15) | ✅ **LOW** — ⛔ **blocks on nothing** |
| **SP-140** | ✅ **T-0536** | ⛔ **[I-0215] Apple DROPS unknown `inspector-layout.json` keys — LIVE data loss** | 🔵 **Planned** | ✅ **LOW** — ⛔ **blocks on nothing; ✅ parallel to SP-130** |
| **SP-141** | ✅ **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** | 🔵 **Planned** | ⚠️ **HIGH** — ⛔ **needs an ENDPOINT-SHAPE ruling** |
| **SP-142** | ✅ **T-0537** | ⚠️ **Retire Linux's duplicate `InspectorLayoutStore.cpp`** | 🔵 **Planned** | ⚠️ **MEDIUM** — ⛔ **blocks on SP-141** |
| **SP-143** | ✅ **T-0510** | ⚠️ **The regression guard — closes [I-0197]** | 🔵 **Planned** | ✅ **LOW** — ⛔ **blocks on SP-141 AND SP-142** |

⚠️ **SCOPE NOTE ON SP-129 / SP-130 — ✅ MOVED BY USER RULING 2026-09-15; ⚠️ THEY DO NOT FIT THIS
EPIC'S STATED SUBJECT, AND THAT IS SAID PLAINLY RATHER THAN BURIED.** ⚠️ **This Epic is the WINDOW
AND PANE STRUCTURE** — ✅ **toolbar, `safeAreaBar`, the Inspector as a real column, the Detail
Sheet's chrome.** ⚠️ **SP-129/SP-130 are TIMELINE AND BOUNDARY work:** ✅ **routing
`loadImportedTimelines` through `scrivi_list_imported_timelines` (deleting a
`FileManager`/`JSONDecoder` bypass), an epoch-offset surface, `setTimelineEpochLabel`, and
`promoteObject`.** ⛔ **None of it is chrome.** ---- ✅ **THEY WERE HOMELESS: [EP-039] listed the
bypasses as in-scope and CLOSED WITHOUT DOING THEM**, ⚠️ **so parking them here keeps them visible
rather than losing them — ✅ which is the real argument for the move.** ⚠️ **THE COST: this Epic's
goal line (*\"a writer can see and reach every part of the editor\"*) will NOT answer for these two
Sprints at close time.** ✅ **A cleaner home would be an Epic of their own, or [EP-032] (`[Cross]`
inline object references), which already owns timeline-adjacent surface work.**
⚠️ **RAISED, NOT RE-DECIDED — the user's ruling stands.**

⚠️ **ON THE SIZING — ✅ IT IS FIVE SPRINTS OF WORK, NOT SIX.**
✅ **S1 and S2 are ONE Sprint deliberately:** ⚠️ **S2 adds NO new logic — it populates a toolbar S1 just
created with closures that already exist.** ⚠️ **Splitting them ships an EMPTY TOOLBAR as a milestone.**
⚠️ **AND S5 IS RECORDED, NOT SCHEDULED** — ✅ **the app-shape study marks it "NOT NOW", and an Epic that
schedules it reverses its own design ruling.** ✅ **It is the DESTINATION; ⚠️ it earns a Sprint only if
S1–S4+S6 leave something unfixed, which is a decision for the Epic close, not now.**
✅ **So: FOUR Sprints of planned EDITOR-SHELL work (SP-134–SP-137) + ONE recorded-not-scheduled (SP-138).**

⚠️ **PLUS the [I-0197] BYPASS CHAIN: [SP-130] (active) + [SP-140]–[SP-143].** ⚠️ **SP-130 was ONE
sprint of four tasks until 2026-09-15, when the user's sizing questions forced a re-measurement:**
⛔ **T-0509's remainder was ZERO, ⛔ T-0510 could not ship before SP-141, and ⚠️ T-0507 proved to be a
two-codebase ownership change, not a routing fix.** ✅ **Rationale:
[`Sprint-SP-130-RESTRUCTURE.md`](../Sprints/Sprint-SP-130-RESTRUCTURE.md).**

### ⚠️ Sequencing constraints — ✅ these are NOT preferences

1. ⚠️ **S4 BEFORE S6.** ✅ **Detail-Sheet option (c) — a second trailing column — CONTENDS WITH the
   Inspector for the SAME EDGE.** ⚠️ **Designed in the wrong order they fight.**
2. ⚠️ **[SP-133] (TextKit 2) MUST LAND FIRST.** ✅ **S1–S3 do not touch the manuscript's internals**,
   ⚠️ **but S4/S6 touch the shell that [I-0132]/[I-0161] show is delicate** (⚠️ **the
   selection/viewport loop and focus transfer**).
3. ⚠️ **AC9 ([I-0205]) BEFORE the banner is restyled in S3.** ⚠️ **If the banner is FALSE, the right fix
   is to stop showing it — ✅ and restyling it first would entrench a defect as a design.**
4. ⚠️ **The address-level capture [I-0203] demands is STILL OUTSTANDING.** ✅ **The conformance study
   identified the two scroll views STRUCTURALLY (the navigator `List`, the manuscript `NSScrollView`)
   but NOT BY ADDRESS** — ⚠️ **and three code-read diagnoses were already wrong in this investigation.**
   ⚠️ **S3 must not be declared a fix on structural inference alone.**

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| **I-0203** | ⚠️ **Scene Navigator / window chrome vanishes while the banner is up** | 🔵 **Open** — ✅ **AC5 is its acceptance test** |
| **I-0205** | ⚠️ **The world-unavailable banner itself** | 🔵 **Open — NOT DIAGNOSED** — ⚠️ **AC9 gates S3** |
| **I-0206** | ⚠️ **`setSelectedRange` LINEAR IN DOCUMENT OFFSET** (`57–81 ms` near 1.85 M) | 🔵 **Open** — ✅ **CARRIED FROM [EP-039] 2026-09-15** |
| **I-0213** | ⚠️ **Chapter create costs `~305 ms` of work** (`reloadSceneDots` ~200 ms of it) | 🟡 **Improved 880→305 ms, NOT closed** — ✅ **CARRIED FROM [EP-039]** |

### Tasks

⚠️ **NONE CREATED.** ✅ **Tasks are minted when a Sprint is activated** (⚠️ **next available: T-0535**),
✅ **not up front — per `Task-Guidelines.md` and the layer discipline that backlog = unstarted only.**

### Scope Notes

- ✅ **S6 (the Object Detail Sheet) was ADDED 2026-09-14 at user review.** ⚠️ **The app-shape study v0.1
  enumerated it as surface #6 and then gave it no step** — ✅ **an omission in the study, corrected in
  §4.4.** ✅ **It proved to be the THIRD instance of the Epic's core defect, not a leftover.**
- ⚠️ **Its hosting was ALREADY RULED ONCE** (✅ **`EditorView.swift:56-64`**): ⚠️ **D1-C (push into the
  280pt inspector) REJECTED — wrong width for long-form notes; ⚠️ D1-B (separate window) DEFERRED —
  Scrivi has no auxiliary window type and [EP-018] documents the cost.** ✅ **D1-E (non-modal
  beside-pane) shipped.** ⚠️ **S6 must RE-RULE deliberately, not silently re-litigate.**
- ⛔ **Export and the text-size lens are SLOTS ONLY (AC7).**
- ⛔ **AC10's guard mechanism is UNDESIGNED** — ✅ **flagged so it is specified, not assumed.**

### Completion Summary

⚠️ **(Filled at 🟠 Complete.)**
