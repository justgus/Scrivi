# Sprint SP-237 — `[Apple]` [EP-059] C5/C6: custom relationship types on the Mac and iOS, and the endpoint's C ABI test

**Status:** ✅ **CLOSED 2026-11-23** (user, 2026-11-23: *"Close SP-237."*). Was 🟡 **ACTIVE** 2026-11-22 (user, 2026-11-22: *"Q1-Q7 as recommended, implement SP-237"*); ✅ Q1–Q7 as recommended. Built 2026-11-22: AC1–AC4 met; ✅ **All ACs met** (AC6: user, 2026-11-23: *"all the tests pass on iOS as well (both iPad and iPhone)"*); CI green on `af6f71e`; closed on approval.
 Created 2026-11-22 (user, 2026-11-22: *"plan C5-C6"*).
**Carries:** [T-0700] (C5 on Apple, C6).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **C**: C5, C6. ⛔ [SP-238] (C5 on Linux) is blocked by this Sprint.
**Size:** M — one core fix with its ABI tests, one settings section and one form, shared by the Mac and iOS.

---

## What reading the code found (2026-11-22)

1. **No platform has a surface, the Mac included.** `scrivi_upsert_relation_type` (`scrivi_c_api.cpp:2703`) is bridged on Linux
   (`ScriviBridge::upsertRelationType`) and has no caller anywhere; Swift has no binding. A writer has the four seeded types only
   (appears-in, located-at, sibling-of, cites). ⚠️ So Linux has nothing to adopt yet: this is A8/A9's shape — the Mac first
   (user, 2026-11-21: *"The Linux sprint should block on the MacOS/iOS sprint's completion."*).
2. **The endpoint silently widens a misspelled kind to "any kind".** It maps `sourceKind` / `targetKind` through
   `objectKindFromName`, whose `nullopt` means *unrecognised* (its own comment: *"callers must never silently substitute a
   default"*), but `RelationType` reads `nullopt` as *any kind*. `"charcter"` saves an unconstrained type and reports
   `saved: true`. `"world"` is also accepted, though a world is not an endpoint.
3. **A seeded type can be "edited" and is then reverted without a word.** `RelationTypeStore::load` replaces every seeded code
   that differs (T-0441, by design). The endpoint accepts `appears-in` and reports `saved: true`; the next read undoes it.
4. **A new type can overwrite an existing one.** Upsert replaces by `code`. A surface that derives the code from the label would
   silently replace a type whose code matches — a seeded one (reverted, finding 3) or the writer's own.
5. **No C ABI test** (C6). `RelationshipTests.cpp:561` tests the store; nothing tests the boundary, which is how findings 2–3 went unseen.
6. **There is no delete.** No endpoint removes a type, and edges reference types by code.
7. **Changing a used type's kinds or symmetry would break its edges.** Edges already stored would violate new kind constraints,
   and flipping symmetric changes canonical direction (§5.3), so stored edges would no longer be canonical — the duplicates §5.2 exists to prevent.
   Labels are safe: an edge's label is projected at read time (§5.2), so a renamed type renames every edge at once.
8. ⚠️ **`objectKindFromName` restates the kind list** (`ObjectTypes.hpp:127`) — the standing rule's defect class, currently correct.

## Plan

1. **Core (C6 and the fixes):** `scrivi_upsert_relation_type` refuses an unknown or non-endpoint kind name (empty = any, `scene`,
   or a storable kind); refuses a seeded code; refuses kind or symmetry changes to a type that edges use (Q4). `objectKindFromName`
   derived from `kAllStorableKinds` plus `world`. C ABI tests in `ObjectCApiTests.cpp`, through `scrivi_*`: create, list shows it,
   edit labels, every refusal, an edge of the new type created and labelled from both ends.
2. **Swift:** `ScriviEngine.upsertRelationType` (and the test in `ScriviInteropTests.swift`).
3. **Surface (Q2, Q3):** Project Settings gains **Relationship Types**: the built-in four listed, locked; the writer's own, editable;
   **New Relationship Type…** opens a form. The Relate… picker (`ObjectRelationPicker.swift`) gets the same **New Type…**, and the
   new type is chosen when the form closes.
4. **Tests and a live pass** on the Mac and on iOS.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Who goes first.** The Mac has no surface to adopt. | **The Mac and iOS first (this Sprint), Linux after ([SP-238], blocked)** — as A8/A9. | ✅ As recommended (2026-11-22) |
| **Q2** | **Where it lives.** | **Project Settings ▸ Relationship Types** to manage them (types are per project, `objects/relation-types.json`), **and New Type… in Relate…**, where the writer finds the type missing. | ✅ As recommended (2026-11-22) |
| **Q3** | **The form's fields.** The record has seven. | **Four:** "Reads from this end" (forward label), "Reads from the other end" (inverse), **Same both ways** (symmetric: one label, lexical direction), and **From / To** kinds (Any, Scene, or a kind — the list from `scrivi_list_object_kinds`). The `code` is made from the label and never shown, made unique so it never replaces another type (finding 4); `canonicalDirection` follows from Same both ways. | ✅ As recommended (2026-11-22) |
| **Q4** | **Editing a type in use.** | **Labels always; kinds and Same both ways only while no edge uses it**, refused in the core and greyed with the reason in the form (finding 7). Built-in types are not editable (finding 3). | ✅ As recommended (2026-11-22) |
| **Q5** | **Delete.** There is no endpoint (finding 6). | **Not in this Sprint.** Filed as [T-0701] (`[Cross]`, Backlog): delete a type no edge uses — a new endpoint, every platform. Withdrawn if ruled otherwise. | ✅ As recommended (2026-11-22) |
| **Q6** | **The silent "any kind" and the seeded-code revert** (findings 2, 3). | **Fix both in the endpoint** — refuse, with a message — tested through `scrivi_*` (C6). No migration: no type was ever written through it (no caller) ([project_no_testers_yet]). | ✅ As recommended (2026-11-22) |
| **Q7** | **`objectKindFromName`'s restated list** (finding 8). | **Derive it in this Sprint** — this Sprint edits its only boundary caller here, and the standing rule calls a restatement a defect on sight. | ✅ As recommended (2026-11-22) |

## What was built (2026-11-22)

**Core** (C6 and Q6/Q7):
- `scrivi_upsert_relation_type` (`scrivi_c_api.cpp`) refuses an unknown kind name and `world` ("unknown kind: …"); an empty
  `code` is a new type. Its result's `code` is the one saved.
- `RelationTypeStore::upsert` (`RelationTypes.cpp`) returns the saved type: an empty code becomes a unique one made from the
  forward label (`mentor-of`, then `mentor-of-2`); a seeded code is refused; a type that edges use keeps its kinds, symmetry and
  direction. `isSeeded(code)` is new.
- `scrivi_list_relation_types` reports `builtIn` and `inUse` per type (Q4's greying).
- `objectKindFromName` (`ObjectTypes.hpp`) derived from `kAllStorableKinds` plus `world` (Q7).

**Swift:**
- `ScriviEngine.upsertRelationType` (`ScriviEngineGraph.swift`; the stub in `ScriviEngine.swift`); `RelationTypeEntry` gains `builtIn`, `inUse`.
- `RelationTypeForm.swift` (new; in the pbxproj for every app target): `RelationTypeForm` — Reads from this end, Same both ways,
  Reads from the other end, From / To (Between when Same both ways; the kinds from `listObjectKinds`); kinds and Same both ways
  greyed with the reason when the type is in use. `RelationTypesSection` — Project Settings ▸ Relationship Types: each type's labels
  and ends, built-in ones locked, Edit… on the writer's own, New Relationship Type….
- `ProjectSettingsSheet.swift`: the section, after Authors. `ObjectRelationPicker.swift`: **New Type…** beside the type menu; the
  popover shows the form, with From set to the object's kind, and returns with the new type chosen when it can start here.

## Tests

- C ABI (`ObjectCApiTests.cpp`, through `scrivi_*`): a custom type created, listed (`builtIn`, `inUse`), a second with the same
  label given its own code, an unused type's kinds changed, used by an edge and read from both ends, relabelled while in use; and
  refused: a misspelled kind, `world`, a seeded code, a kind change and a symmetry change to a used type. ✅ A mutant that accepts
  any kind name and seeded codes fails it (in a copy of the repo).
- `RelationshipTests.cpp`: the T-0441 "accepted consequence" test now edits the seeded type in the file (upsert refuses it) and
  still shows it reverted on the next read.
- Interop (`ScriviInteropTests.swift`): the engine round-trip and the built-in refusal.
- Results: macOS ctest **881/881**; interop **353/353**; iOS tests **38/38**; Linux in Docker ctest **886/886**, smokes **49/49**;
  the CI check scripts (package boundary, TextKit 2, layout convergence, stub parity, ABI guards, manuscript type source) clean.

## Live pass (AC6) — the Mac, then iOS

⚠️ The types made here stay in the project: there is no delete yet ([T-0701]). They are named "test …" to be easy to spot.

1. Run the Mac app and open `dumas-prose-timelines-history`. Click **Project Settings** in the toolbar (⌥⌘,).
2. Below Authors is **Relationship Types**, listing "appears in · features" (Any kind → Scene), "takes place at · hosts" (Scene →
   Location), "sibling of" (Between Character) and "cites · documented by" (Any kind → Any kind), each with a lock. Hover a lock: "Built in: …".
3. Click **New Relationship Type…**. A "New Relationship Type" sheet opens; Save is greyed. Type `test mentor of`, then `test
   mentored by` in Reads from the other end; set From and To to Character. Click Save. The list gains "test mentor of · test
   mentored by", Character → Character, with Edit….
4. Click its **Edit…**, set To to Location, Save: the row reads Character → Location. Edit… again, set To back to Character, Save.
5. Click Done. Open Abbé Faria's detail (Worldbuilding ▸ Characters, View Detail). Click **Relate…**: the popover has the type
   menu and **New Type…**. Choose "test mentor of" in the menu and click Edmond Dantès. Related shows "test mentor of" with Edmond Dantès.
6. Click Edmond Dantès there (Show): his Related shows "test mentored by" with Abbé Faria. Go back to Abbé Faria.
7. Click Relate… ▸ **New Type…**. The popover shows the form with From set to Character. Type `test rival of`, turn on **Same
   both ways**: the other-end field goes and From becomes Between. Save: the popover is back on the list with "test rival of"
   chosen. Press Esc.
8. Open Project Settings, **Edit…** "test mentor of": Same both ways, From and To are greyed, and the footer says relationships use
   this type. Change the first label to `test teacher of`, Save, Done. Faria's Related now reads "test teacher of".
9. Right-click Edmond Dantès under "test teacher of" ▸ Remove Relationship.
10. On iOS (the simulator or a device), open the same project, then Project Settings: Relationship Types lists both test types.
    Tap New Relationship Type…: the form opens. Tap Cancel.

## Acceptance criteria

- [x] AC1 — **C6:** C ABI tests for `scrivi_upsert_relation_type` through `scrivi_*`: a custom type created, listed, used by an
      edge labelled from both ends, its labels edited; an unknown kind, `world`, a seeded code, and a kind or symmetry change to a
      used type each refused with a message.
- [x] AC2 — Project Settings ▸ Relationship Types lists the built-in types (locked) and the writer's own; New Relationship Type…
      creates one; editing obeys Q4 with the reason shown.
- [x] AC3 — Relate… offers New Type…; the new type is selected and usable at once.
- [x] AC4 — The same on iOS.
- [x] AC5 — ctest, the interop tests and the iOS tests green; CI green. ✅ macOS 881/881, interop 353/353, iOS 38/38, Linux 886/886
      and 49/49; CI green on `af6f71e`.
- [x] AC6 — Live pass on the Mac and iOS.

## Progress log

- **2026-11-23** — ✅ **CLOSED** (user, 2026-11-23: *"Close SP-237."*). T-0700 archived. [SP-238] unblocked.
- **2026-11-23** — ✅ **AC5, AC6 met**: iOS step 10 passes on the iPad and Remus after the old Scrivi app was removed (user, 2026-11-23: *"all the tests pass on iOS as well (both iPad and iPhone)"*); CI green on `af6f71e` (ScriviCore, Apple, Linux). T-0700 ✅ Verified. Awaiting close approval.
- **2026-11-23** — Mac steps 1–9 pass (user, 2026-11-23: *"All 10 steps passed on Mac."*). ⚠️ iOS step 10 blocked: *"I was not able to open any inkwyn file on iOS."* No Swift or plist change touches the iOS open path since SP-229's iOS pass (2026-11-20); symptom requested.
  **Cause (user: *"the only files available … were .scrivi files"*):** not code. `devicectl` shows the pre-rename app `com.caposoft.scrivi` ("Scrivi" 1.0.410) still on the iPad beside Inkwyn 1.0.431, and on Remus ONLY Scrivi 1.0.412 (no Inkwyn). The old app exports the same type IDs with `.scrivi`, so iOS resolves `com.caposoft.scrivi.project` to `.scrivi` — the [I-0366] cause, on iOS. Remedy: delete the old Scrivi app; install Inkwyn on Remus.
- **2026-11-22** — Built: AC1–AC4 met; AC5 green locally; T-0700 → 🟠. ⏳ AC6 live pass, CI.
- **2026-11-22** — 🟡 **ACTIVE** (user, 2026-11-22: *"Q1-Q7 as recommended, implement SP-237"*); Q1–Q7 as recommended; T-0700 → active.
- **2026-11-22** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-22: *"plan C5-C6"*); carries [T-0700] (new); [T-0701] filed for Q5. Q1–Q7 to rule.

*Last Updated: 2026-11-23 (Closed.)*
