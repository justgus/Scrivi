# SP-090: `[Apple]` EP-030 — Scene Inspector card framework ✅ CLOSED

**Status:** ✅ **Closed 2026-08-05 (Human-approved)** — activated and completed the same day. The first EP-030
sprint: replaced the 57-line placeholder inspector with the approved card framework (protocol + registry,
`inspector-layout.json`, bottom tabs, stack mechanics). All four tasks (T-0359–T-0362) Verified.
ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **BUILD SUCCEEDED** + launches clean.

### What shipped

- **`Scrivi/Views/Inspector/InspectorCard.swift`** (NEW) — `InspectorCard` protocol, `AnyInspectorCard`
  type-erasure, `InspectorCardRegistry`, and the `InspectorTab` / `InspectorStack` / `InspectorSort` /
  `CardConfig` types. Plus `PlaceholderCard`, explicit scaffolding for SP-091 to replace.
- **`Scrivi/App/InspectorLayoutStore.swift`** (NEW) — `scrivi.inspector-layout.v1` in the project package
  (project-level and Git-visible; **not** the scene sidecar, **not** UserDefaults). Atomic saves (temp +
  replace). A corrupt file falls back to defaults and is **left untouched**, never clobbered on open.
- **`Scrivi/Views/Inspector/InspectorCardStackView.swift`** (NEW) — the scrolling column: add/remove/collapse,
  **per-stack** sort (C6), apply-to-all-scenes, and the unknown-typeID notice (AC11).
- **`Scrivi/Views/SceneInspectorView.swift`** (REWRITTEN) — bottom tabs `Writing | Worldbuilding | Properties`,
  defaulting to Writing; drag-resizable pane (220–560pt) replacing the hard 280pt.
- **`ProjectSession`** — owns `inspectorLayout`; `inspectorVisible` promoted from in-memory to persisted.
- **`EditorView` / `ScriviApp`** — inspector call sites pass scene context; registry populated at app start.

> ### ⚠️ Post-close correction — I-0101 (2026-08-05)
>
> SP-090's AC11 was verified for **reporting** unknown typeIDs, not for **removing** them. The
> `unknownCardsNotice` shipped display-only, so when SP-091 retired `PlaceholderCard` a scene that still
> named it stranded an entry the writer could not clear. Logged as **I-0101** and fixed in SP-091 (one
> removable row per unavailable card). The store's `removeCard` was always correct — only the UI was missing.
>
> **Lesson for future card retirements:** confirm the unknown-type path is *actionable*, not merely visible.

### Notable implementation decisions

- **Restore-before-publish** in `ProjectSession.load`: `inspectorVisible` is set from the store *before*
  `inspectorLayout` is assigned, so the new `didSet` cannot write back the value just read.
- **Unknown typeIDs survive edits** — mutations read through `rawEntries` (which retains unresolved types)
  rather than the resolved list, so editing a stack cannot silently drop a card this build lacks.
- **`.manual` sort preserves stored order** and must never re-sort.

---

## SP-090: `[Apple]` EP-030 — Scene Inspector card framework (protocol, layout schema, tabs)

**Status:** ✅ **Closed 2026-08-05 (Human-approved).** All four tasks ✅ Verified. First sprint of EP-030.

> ### Verification (2026-08-05, user)
>
> Verified live: **tab selection and inspector visibility persist across launches** (the AC3/AC4 pair, and the
> in-memory `inspectorVisible` bug is closed); the **tabbed rewrite** of `SceneInspectorView` with its default
> messages and scaffolding; the **sort menu** carries the required items. T-0362 accepted on the clean suites.
> All four tasks marked ✅ Verified.
>
> **The Add ("+") menu — resolved 2026-08-05, no defect.** An initial observation that "cards could not be added"
> was **confirmed by the user to have been on the Worldbuilding tab**, where an empty Add menu is *correct*: the
> ruled default is an empty stack and the only registered card (`PlaceholderCard`) is `.writing`-scoped.
>
> | Tab | Add menu | Verified |
> | --- | --- | --- |
> | **Worldbuilding** | "No cards available yet" | ✅ correct — empty by ruling |
> | **Writing** | **Placeholder**, addable | ✅ selecting it adds the card to the stack, **above** the "3 cards aren't available yet" notice |
>
> That second row also confirms the framework end-to-end in one gesture: registry lookup → layout mutation →
> atomic save → re-render, with the unknown-typeID notice (AC11) coexisting correctly with a resolved card.
**Epic:** EP-030 `[Apple]` — Scene Inspector Card Framework (1st of 4 sprints).
**Goal:** Stand up the **card framework**: a card protocol + registry, the `inspector-layout.json` persistence
schema, and the bottom-tab shell — replacing the placeholder `SceneInspectorView`. **No writing-tool card
content ships this sprint** (that is SP-091); the sprint ends with an empty-but-real stack the writer can
configure.
**Design:** `docs/Scrivi_Scene_Inspector_Card_Framework_v0_1.md` ✅ **Approved 2026-08-05** — §3 (what a card is),
§4 C1/C2/C3/C6, §4.7 (tabs), §5 (card protocol), §9 AC1–AC12.
**Start Date:** 2026-08-05
**Target Close Date:** TBD — closes on live verification of the sprint exit criteria.
**Depends on:** nothing unbuilt.

### Sprint Goal (one sentence)

Replace the 57-line placeholder inspector with a real, persistent, writer-configurable card framework — tabs,
stacks, and per-scene layout — so SP-091's cards have somewhere to live.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0359 | `InspectorCard` protocol + registry + `CardContext` | High | ✅ **Verified (2026-08-05)** |
| T-0360 | `inspector-layout.json` schema + load/save + unknown-`typeID` skip | High | ✅ **Verified (2026-08-05)** |
| T-0361 | Bottom tabs + persisted `selectedTab` + menu hide/show; replace `SceneInspectorView` | High | ✅ **Verified (2026-08-05)** |
| T-0362 | Card stack: add/remove/reorder, collapse, per-stack sort, "apply to all scenes" | Medium | ✅ **Verified (2026-08-05)** |

### What the code looks like today (surveyed 2026-08-05)

- **`Scrivi/Views/SceneInspectorView.swift`** — 57 lines. An `InspectorTab` enum with a **single** `.entities`
  case, a segmented `Picker`, a stub `SceneEntitiesTabView` ("No entities yet." + a disabled "Add Entity"
  button), and a hard `.frame(minWidth: 240, idealWidth: 280, maxWidth: 280)`. **All of this is replaced.**
- **`Scrivi/Views/EditorView.swift:171-179`** — the only call sites, gated on `session.inspectorVisible`, with an
  iOS branch that additionally excludes iPhone. The new view drops in here unchanged in shape.
- **`session.inspectorVisible`** (`ProjectSession.swift:75`) — already exists and is already wired to three
  "Show Scene Inspector" menu toggles (`ScriviApp.swift:183,297`, `EditorView.swift:195`). ⚠️ **It is
  `var inspectorVisible: Bool = true` — in-memory only, reset every launch.** Doc 2 AC4 requires the hide/show
  state to persist, so **T-0361's real work is persistence**, not adding a menu item.

### Task detail

**T-0359 — `InspectorCard` protocol + registry + `CardContext`.**
Per design §5: `typeID` (stable, persisted), `title`, an associated `Body` view, and a `CardContext`
(`sceneID`, `engine`, `config`). A registry maps `typeID` → implementation so `inspector-layout.json` stays
decoupled from Swift types. Include **one throwaway placeholder card** so the stack can be exercised before
SP-091 — removed or replaced in SP-091, not shipped as a feature.

**T-0360 — `inspector-layout.json`.**
Schema `scrivi.inspector-layout.v1` per design §C2: `selectedTab`, `inspectorHidden`, `defaultStacks`
(worldbuilding **empty**; writing = `tags`/`outline`/`todo`), `stackSort`, and per-scene `worldbuilding`/`writing`
arrays. Project-level, Git-visible, **not** the scene sidecar and **not** workspace state. An unknown `typeID`
is **skipped with a notice, never a crash** (§5, AC11). Scenes absent from `scenes` fall back to `defaultStacks`;
deleting a scene drops its entry.

> **Note on `defaultStacks`:** SP-090 writes the *schema* including the writing defaults, but the three named
> cards do not exist until SP-091. Until then the writing stack resolves those `typeID`s through the
> unknown-type path — which makes AC11 exercisable this sprint rather than theoretical. Confirm the notice reads
> sensibly for that transient case.

**T-0361 — Tabs + persistence + replace the placeholder.**
Bottom tabs in order **`Writing | Worldbuilding | Properties`**, defaulting to **Writing** when no selection is
saved (§4.7). Tab selection **must not change when the scene changes** — switching scenes reloads that tab's
cards for the new scene. Persist `selectedTab` **and** `inspectorHidden` at project level; the latter means
promoting the existing in-memory `session.inspectorVisible` to persisted state. Replace the fixed 280pt frame
with a **resizable pane** with a sensible minimum. Properties tab renders as a placeholder this sprint (built in
SP-092).

**T-0362 — Card stack mechanics.**
Add / remove / reorder (drag within the column) / collapse. **Sort is per-stack, not per-card** (C6 ruling) —
one `stackSort` per stack, applying to every card in it. "Apply this card layout to all scenes" and "…to this
chapter" (C1), leaving each scene's entry independently editable afterward.

### Exit criteria — met (user-verified 2026-08-05 except where noted)

- [x] The inspector renders **bottom tabs** (`Writing | Worldbuilding | Properties`); each stack tab is a
      scrolling column of collapsible cards. The one-card-per-tab layout is gone. (AC1)
- [x] With no saved selection the inspector opens on **Writing**. (AC2)
- [x] Tab selection persists **at project level** across relaunch and **does not change** when the scene
      changes. (AC3) — **user-verified across app launches**
- [x] The whole tab view hides/shows from the menu, and **that state persists across relaunch**. (AC4) —
      **user-verified**; closes the in-memory `inspectorVisible` bug
- [x] The inspector pane is **resizable**; cards remain usable at the minimum width. (AC5)
- [x] A writer can add, remove, and reorder cards on a scene; stacks persist via `inspector-layout.json`. (AC6)
      — **user-verified 2026-08-05**: Placeholder is offered in the Writing tab's Add menu, and selecting it adds
      the card to the stack, rendering **above** the unknown-cards notice.
- [x] Worldbuilding's default stack is **empty**; nothing is inferred from scene content. (AC7)
- [x] "Apply layout to all scenes" works; each scene stays independently editable. (AC9) — accepted on the clean
      suites; re-exercised naturally in SP-091 once several real cards exist.
- [x] **Sort is set once per stack** and applies to every card in it; no per-card sort exists. (AC10) —
      **user-verified**: the sort menu carries the required items
- [x] An unknown `typeID` is **skipped with a notice**; the inspector still renders. (AC11) — **user-verified**
      via the default-message scaffolding
- [x] `ctest` **369/369** + interop **45/45** green; **BUILD SUCCEEDED**, no new warnings; app launches clean.

**Deferred to later sprints (explicitly out of scope here):** the `tags`/`todo`/`outline`/`sources` cards
(SP-091, AC8); the `history` card and the real Properties tab (SP-092, AC13–AC15); card-failure isolation
(SP-092, AC12).

### Non-negotiables for this sprint

- **`Scrivi.xcodeproj/project.pbxproj` MUST be updated in the same step** as every new `.swift` file under
  `Scrivi/` — before the build, not after (CLAUDE.md). SP-090 adds several new files, so this applies repeatedly.
- **Swift is UI only** (Architecture v0.3). Cards call ScriviCore through `ScriviEngine`; no backend logic in
  Swift.
- **§2 manual-surfacing principle:** nothing is presented, proposed, or auto-populated. A new scene shows
  `defaultStacks` and nothing inferred from its content.
- Claude may mark tasks **"Implemented — Not Verified"**; only the user marks them Verified.


---

*Archived 2026-08-05.*
