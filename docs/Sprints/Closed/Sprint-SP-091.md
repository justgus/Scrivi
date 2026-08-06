# SP-091: `[Cross]` EP-030 — writing-tool cards (tags · todo · outline) ✅ CLOSED

**Status:** ✅ **Closed 2026-08-05 (Human-approved)** — planned, implemented, and verified the same day.
The second EP-030 sprint: the first *real* cards on SP-090's framework, backed by three new additive fields on
`scrivi.scene.v1`. ScriviCore `ctest` **381/381** (12 new), macOS interop **45/45**, app **BUILD SUCCEEDED**.

### What shipped

- **`scrivi.scene.v1` + `tags` / `outline` / `todo`** (`SceneMetaJson.hpp/.cpp`) — strictly additive, and
  **omitted when empty** so a scene that never uses the cards keeps its pre-SP-091 bytes exactly (no Git churn).
- **Four C ABI endpoints** (`scrivi.h`, `scrivi_c_api.cpp`, `ScriviCore.cpp`) — `scrivi_set_scene_tags` /
  `_outline` / `_todo` + `scrivi_get_scene_notes`, on the `scrivi_set_scene_story_time` pattern rather than
  growing the already-13-parameter `scrivi_save_scene` (a manuscript path, not a metadata path). One reader
  returns all three fields so a card stack costs one crossing.
- **`Scrivi/Views/Inspector/WritingToolCards.swift`** (NEW) — `TagsCard`, `OutlineCard`, `TodoCard` over a
  shared per-scene `SceneNotesModel`, plus a small `FlowLayout` for tag chips.
- **`ScriviEngine`** — `getSceneNotes` / `setSceneTags` / `setSceneOutline` / `setSceneTodo` + visionOS stubs.
- **`PlaceholderCard` retired**; the registry now registers the three real cards.

### Notable implementation decisions

- **Tag de-duplication happens in the core**, not the UI — the card can legitimately send duplicates (paste,
  quick re-entry) and the sidecar should never accumulate them.
- **Blank todo rows are never persisted**, at both the schema and the endpoint layer.
- **Outline commits on focus loss**, not per keystroke — one sidecar write per editing session.
- **`SceneNotesResult` decodes absent arrays as empty.** The C ABI omits empty arrays; a non-optional Swift
  field would throw `keyNotFound` on a scene with no tags — the exact shape of **I-0094** from EP-029.

### Issue found and fixed in-sprint

**I-0101** `[Apple]` — an unavailable ("unknown-typeID") card could not be removed: `unknownCardsNotice` shipped
display-only, so retiring `PlaceholderCard` stranded an entry in a real project (Chapter 7 Scene 6). The store's
`removeCard` was always correct; only the UI was missing. **SP-090's close-out claim that such cards were
"removable by the writer" was wrong** — AC11 had been verified for *reporting*, never for *removal*. Fixed as one
removable row per unavailable card; SP-090's archive annotated. ✅ Verified, including persistence across relaunch.

> **Carried forward:** any future card retirement must confirm the unknown-type path is **actionable**, not
> merely visible. This recurs whenever SP-092 or EP-031 renames or retires a `typeID`. Recorded in
> `InspectorCard.swift`.

### Process note — a stale build directory

`build/` had been configured without `SCRIVI_BUILD_TESTS=ON`, so `ctest` was replaying a cached binary and new
test cases appeared to pass without ever being compiled. Caught when the count did not move after adding four
tests. Reconfigured; the count then tracked correctly (369 → 373 → 381). **Earlier green runs this session were
real but could not have caught new cases.**

---

## SP-091: `[Cross]` EP-030 — writing-tool cards (tags · todo · outline)

**Status:** ✅ **Closed 2026-08-05 (Human-approved).** All four tasks ✅ Verified; I-0101 found, fixed, and
Verified during the same session. Second sprint of EP-030.

> ### Verification (2026-08-05, user)
>
> **Cards:** all three exist, surface their data, and **persist the collapsed state across quit/restart**.
> **ScriviCore:** T-0392/T-0393 verified as correctly implemented, producing correct output data.
> **I-0101** (below) was found during this verify, fixed, and re-verified — the stranded `placeholder` card
> removes via the new per-card ✕ and **stays removed after relaunch**.
>
> **⚠️ I-0101 — an unremovable unknown card (Medium).** Chapter 7 Scene 6 of `the-stairs-of-tintagael.scrivi`
> still carried the SP-090 `placeholder` card that this sprint retired, and the writer had **no UI to clear
> it**. `unknownCardsNotice` had shipped display-only; the store's `removeCard` was always correct.
> **SP-090's close-out claim that such cards were "removable by the writer" was wrong** — AC11 had been
> verified for *reporting*, never for *removal*. Fixed here (one removable row per unavailable card) and the
> SP-090 archive annotated. **Lesson recorded in `InspectorCard.swift`: any future card retirement must
> confirm the unknown-type path is _actionable_, not merely visible** — this recurs whenever SP-092 or EP-031
> renames a typeID.
**Epic:** EP-030 `[Apple]` — Scene Inspector Card Framework (2nd of 4 sprints).
**Goal:** The first *real* cards on SP-090's framework — `tags`, `todo`, and `outline` — backed by new
per-scene fields in `scrivi.scene.v1`. Retires `PlaceholderCard`.
**Design:** `docs/Scrivi_Scene_Inspector_Card_Framework_v0_1.md` ✅ Approved — §3.1 (card catalog), §9 AC8.
**Start Date:** 2026-08-05
**Target Close Date:** TBD — closes on live verification of the exit criteria.
**Depends on:** SP-090 ✅ (framework, registry, layout store).

> ### ⚠️ Scope correction found at planning (2026-08-05)
>
> The sprint was planned as `[Apple]`-only. **It is not.** `scrivi.scene.v1` has **no `tags`, `outline`, or
> `todo` fields** (`ScriviCore/src/schemas/SceneMetaJson.hpp:26-43` — the struct carries identity, authorship,
> content path, stats, and storyTime, and nothing else). The cards have nowhere to persist to.
>
> **Ruled 2026-08-05: extend the scene sidecar.** This is per-scene creative content — it belongs *with the
> scene*, travels with it, and is Git-visible. Storing it in `inspector-layout.json` was rejected: that file is
> view configuration, and putting a writer's prose and tags inside it would mean deleting a UI layout deletes
> her writing.
>
> **Consequence:** SP-091 is a **`[Cross]` sprint** — ScriviCore schema + C ABI + tests first, then the Apple
> cards on top. Two extra tasks (T-0392, T-0393) were added at planning; **next free task is now T-0394**.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0392 | `[ScriviCore]` Extend `scrivi.scene.v1`: `tags`, `outline`, `todo` (additive) + `SceneMetaData` + round-trip tests | High | ✅ **Verified (2026-08-05)** |
| T-0393 | `[ScriviCore]` C ABI: `scrivi_set_scene_tags` / `_outline` / `_todo` + `scrivi_get_scene_notes` + ctest | High | ✅ **Verified (2026-08-05)** |
| T-0363 | `[Apple]` `tags` + `todo` cards | High | ✅ **Verified (2026-08-05)** |
| T-0364 | `[Apple]` `outline` card | High | ✅ **Verified (2026-08-05)** |
| T-0365 | ~~`sources` card + `source` object kind~~ | — | ⚪ **Deferred → EP-031 SP-094** (ruled 2026-08-05) |

> **T-0365 deferred (ruled).** `source` is one of eight new `ObjectKind`s that SP-094 adds anyway (Doc 1 §3).
> Landing it here would mean two epics editing `ObjectKind` in two sprints; deferring keeps it a single coherent
> change. The `sources` **card** then ships with the other object cards in SP-098.

### Task detail

**T-0392 — sidecar schema.** Extend `SceneMetaData` (`SceneMetaJson.hpp`) and `serializeSceneMeta` /
`parseSceneMeta`:

```jsonc
{
  "schema": "scrivi.scene.v1",
  "sceneID": "...", "title": "...", "slug": "...", "status": "...",
  "tags": ["battle", "ada-pov"],                    // NEW — ordered, de-duplicated
  "outline": "Ada confronts the Warden…",           // NEW — freeform prose
  "todo": [ { "text": "check the timeline", "done": false } ]  // NEW — ordered
}
```

**Strictly additive.** Absent keys parse to empty — every existing `scene.meta.json` must load unchanged, which
is the round-trip test that matters most here.

**T-0393 — C ABI.** Follow the `scrivi_set_scene_story_time` precedent (`scrivi.h:258-263`): targeted per-scene
setters keyed on `(projectRootPath, sceneID)`, **not** more parameters on `scrivi_save_scene` — that endpoint
already takes 13 and is the manuscript-save path, not a metadata path.

```c
const char* scrivi_set_scene_tags(const char* projectRootPath, const char* sceneID,
                                  const char* tagsJson);      /* ["a","b"] */
const char* scrivi_set_scene_outline(const char* projectRootPath, const char* sceneID,
                                     const char* outline);
const char* scrivi_set_scene_todo(const char* projectRootPath, const char* sceneID,
                                  const char* todoJson);      /* [{"text":…,"done":…}] */
const char* scrivi_get_scene_notes(const char* projectRootPath, const char* sceneID);
```

One reader (`_get_scene_notes`) returns all three so a card stack costs one call, not three.

**T-0363 — `tags` + `todo` cards.** Token-style tag entry (add/remove, no duplicates); todo as a checkable list
with add/delete. Both write through `ScriviEngine` wrappers over T-0393.

**T-0364 — `outline` card.** Multi-line text for the scene's summary/synopsis, debounced save.

**Also in scope:** **retire `PlaceholderCard`** (`Scrivi/Views/Inspector/InspectorCard.swift`) — it is
scaffolding, not a feature. Remove it **after** the three real cards register, or both Add menus go empty.

### Exit criteria — all met (user-verified 2026-08-05)

- [x] `scrivi.scene.v1` carries `tags`, `outline`, `todo`; **an existing `scene.meta.json` without them loads
      unchanged** (additive-compatibility test).
- [x] The four new C ABI endpoints round-trip through ctest (8 new integration cases) **and** were exercised
      end-to-end against a copy of a real pre-SP-091 project.
- [x] The Writing tab's default stack renders **three real cards**; the "cards aren't available yet" notice is
      gone for them (Doc 2 AC8).
- [x] Each card is **dismissible**, and the writer's choice — including **collapsed state** — persists across
      quit/restart (§1).
- [x] Card edits persist to the scene sidecar and survive close/reopen.
- [x] `PlaceholderCard` is gone; the Writing Add menu offers the three real cards. **A layout still naming it
      is now removable** (I-0101).
- [x] **AC6 re-exercised with real cards** — add/remove/collapse across a multi-card stack, collapsed state
      persisting across relaunch.
- [x] `ctest` **381/381** + interop **45/45** green; **BUILD SUCCEEDED**; app launches clean.

### Non-negotiables

- **`Scrivi.xcodeproj/project.pbxproj` MUST be updated in the same step** as every new `.swift` file under
  `Scrivi/` (CLAUDE.md). **ScriviCore `.cpp`/`.hpp` go in CMake, NOT pbxproj** — the app links prebuilt
  `libScriviCore.a`.
- **Swift is UI only** (Architecture v0.3) — the cards call ScriviCore through `ScriviEngine`.
- **§2 manual-surfacing:** nothing auto-populates. An outline card is empty until the writer types in it.
- Claude may mark tasks **"Implemented — Not Verified"**; only the user marks them Verified.


---

*Archived 2026-08-05.*
