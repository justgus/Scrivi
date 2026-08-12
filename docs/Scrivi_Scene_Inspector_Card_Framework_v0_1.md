# Scrivi — Scene Inspector Card Framework Design (v0.1)

**Epic:** TBD (EP-A per Doc 1 §10 — base card structure)
**Sprint:** TBD (design)
**Status:** ✅ **APPROVED 2026-08-05 (Human).** This is **Doc 2 of 3**. All trades (C1–C6) ruled; no blocking
questions remain. Implementation follows this doc; per CLAUDE.md any deviation must be surfaced and reconciled
before it is built.

| Doc | File | Status |
| --- | --- | --- |
| **1** | `Scrivi_Worldbuilding_Object_Model_v0_2.md` | ✅ **Approved 2026-08-05** |
| **2** | `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` *(this doc)* | ✅ **Approved** |
| **3** | `Scrivi_World_Data_Separation_v0_1.md` | 🟡 Outline — W1–W6 ruled, body undrafted |
**Date:** 2026-08-04
**Author:** Claude (planning), for user review.
**Extends:** `Scrivi_Worldbuilding_Object_Model_v0_2.md` (the object/edge model these cards read),
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (T-0215 history panel, folded in as a card),
`Scrivi_Architecture_v0_3.md` (Swift is UI only).
**Existing code this replaces:** `Scrivi/Views/SceneInspectorView.swift` — currently a 57-line placeholder
(one `InspectorTab.entities` case, a stub "No entities yet." tab, fixed 280pt width).

---

## 1. Purpose & scope

The Scene Inspector is a **per-scene, writer-configurable stack of cards**. Two families:

- **Writing-tool cards** — tags, todo, outline, **undo history** (folds in EP-019's T-0215), sources, lookahead.  Mostly app-side; depend on little or nothing from Doc 1.  We may specify a number of these writing-tool cards to be active by default in a stock Scrivi configuration, but all must be dismissible and the writers choices must persist.  
- **Worldbuilding-object cards** — characters, locations, artifacts, maps, chronicles, … Read the Doc 1 relationship graph.

**This doc designs the framework** — what a card is, how the stack is configured and persisted, how cards are
laid out, and the interaction rules. Per-kind card *content* for worldbuilding objects is EP-B work built on this
framework and on Doc 1's model.

**In scope:** card protocol/registry; per-scene configuration + persistence; layout; the manual-surfacing
interaction model; the picker; pending/offline presentation; the sequencing that lets EP-A ship before EP-B.

**Out of scope:** the object model and graph (Doc 1); world packaging, epochs, locking (Doc 3); the Timeline
Panel itself (EP-016/EP-025 — though a warning surface lands beneath it, §7.3).

---

## 2. The governing interaction principle (inherited ruling)

Doc 3 §4.5 established *the writer never waits for infrastructure*. Doc 3 §4.1 established the inspector-specific
counterpart, and it is the single most important constraint on this document:

> \*\*Everything appearing in the Scene Inspector with regard to worlds is manually surfaced by the writer.
> Nothing is presented, proposed, auto-populated, or suggested.\*\*

Consequences that shape every trade below:

1. **No card appears on a scene unless the writer put it there.** There is no default card stack derived from scene content, no "we noticed this scene mentions Ada" affordance.
2. **No worldbilding-object card auto-populates.** A Characters card on a scene shows exactly the characters the writer related to that scene — never inferred from the prose.  
3. **Pickers are unfiltered by association** (Doc 3 §4.1): the object picker lists objects from *all* worlds in the project, deliberately, because the writer may not have decided the scene's world yet and \*adding objects is how that decision gets made\*.
4. **Absence is never presented as error.** A world offline (Doc 3 §4.6) yields pending cards and a passive warning, never a modal, never a block.

> This principle is why the framework is a **card stack** rather than a fixed inspector: a fixed inspector
> implies the app knows what a scene needs. It doesn't, and it shouldn't guess.

---

## 3. What a card is

```
┌─ Card ────────────────────────────────┐
│ ▸ Characters              ⋯  ✕        │  ← header: disclosure, title, menu, remove
├───────────────────────────────────────┤
│  Ada                                  │  ← body: card-defined content
│  Thomas                               │
│  + Add Character                      │  ← explicit writer action (never automatic)
└───────────────────────────────────────┘
```

A card is: a **stable type identifier**, a **title**, a **body view**, an optional **per-card configuration**,
and a declaration of **what it needs to load** (nothing / graph edges / history / …).

Cards are **independent**: one card failing to load (an offline world, §7.2) must not prevent others from
rendering. This is a hard requirement, not an aspiration — it is the inspector-side expression of \*the writer
never waits\*.  A Card load failure should display a warning message in place of the card content so that the writer is aware of the problem.  

### 3.1 Card catalog (v1)

**Writing-tool cards (EP-A — ship first, need little/no Doc 1 model):**

| Card        | Reads                     | Notes                                                                              |
| ----------- | ------------------------- | ---------------------------------------------------------------------------------- |
| `tags`      | scene sidecar             | scene-level tags · **ships in the default Writing stack (empty)**                  |
| `todo`      | scene sidecar             | per-scene todo items · **ships in the default Writing stack (empty)**              |
| `outline`   | scene sidecar             | scene summary/synopsis · **ships in the default Writing stack (empty)**            |
| `history`   | `scrivi_history_get_tree` | **folds in EP-019 T-0215** — windowed tree, branch selection, stale badges + purge |
| `sources`   | objects (`source`)        | citations — **reusable across scenes** (ruled), so a real `objects/` file          |
| ~~`lookahead`~~ | —                     | ⏸ **DEFERRED to v2** (ruled 2026-08-05) — see §3.2                                 |

**Worldbuilding-object cards (EP-B — need Doc 1's graph):**

| Card                                        | Reads         | Notes                                        |
| ------------------------------------------- | ------------- | -------------------------------------------- |
| `characters`                                | edges         | project-scoped objects                       |
| `locations`                                 | edges         | project-scoped                               |
| `buildings` · `vehicles` · `items` · `maps` | edges         | project-scoped                               |
| `artifacts` · `rules` · `chronicles`        | edges → world | **world-scoped** — subject to pending (§7.2) |

All object cards share one implementation parameterized by kind; they differ only in title, icon, and the kind
filter applied to `scrivi_list_edges_for`. **One card type, ten configurations** — not ten card types.

### 3.2 `lookahead` — deferred to v2 (ruled 2026-08-05)

The envisioned card: a single card showing **thumbnails of the next scene's card stack for the same tab** — not
readable, but conveying the *shape* of what's coming.

**Deferred to at least v2** because the concept is not yet fleshed out and nothing depends on it. Two things
worth recording now so a later design isn't boxed in:

- It renders **another scene's stack for the current tab**, so it is a *meta-card* — the only card whose content
  is other cards. That likely needs the card protocol (§5) to support rendering a stack at thumbnail fidelity,
  which is a framework capability, not a card feature.
- "Next scene" is a **manuscript-order** concept (EP-027 ordering), not story-time order. Worth stating before
  someone reaches for the Timeline.

No EP-A work depends on this; it is removed from the v1 catalog rather than stubbed.

---

## 4. Trade studies — **all ruled 2026-08-05**

| #      | Question                   | Ruling |
| ------ | -------------------------- | ------ |
| **C1** | Card stack scope           | ✅ **C** — per-scene, with an explicit "apply to all scenes" action |
| **C2** | Where card config persists | ✅ **B** — one project-level `inspector-layout.json` |
| **C3** | Layout                     | ✅ **A** — a scrolling column of collapsible cards, **inside bottom tabs** (§4.7) |
| **C4** | Object card loading        | ✅ **A** — eager on scene select |
| **C5** | Add-object flow            | ✅ **C** — both picker and inline type-ahead, plus **in-stack creation** (§4.6) |
| **C6** | Card config granularity    | ✅ **B** — per-card options, but **sort is per-stack**, not per-card (§4.5) |

### C1 — Card stack scope (**C**)

Cards are configured **per scene**, because scenes genuinely differ — a dialogue scene wants Characters, a battle
wants Maps + Artifacts. A writer-invoked **"Apply this card layout to all scenes"** (and "to this chapter")
covers the drudgery case without a hidden inheritance model.

Rejected **B** (project default + override) because *inheritance is invisible*: a writer who changes the project
default silently mutates every non-overridden scene. Explicit bulk-apply produces the same result while leaving
each scene's stack readable on its own — consistent with §2's manual-surfacing principle.

### C2 — Persistence (**B**, project-level file)

```jsonc
// inspector-layout.json  (project-level)
{
  "schema": "scrivi.inspector-layout.v1",
  "selectedTab": "writing",              // project-level; default "writing" when absent (§4.7)
  "inspectorHidden": false,              // whole tab view hidden/shown via menu (§4.7)
  "defaultStacks": {
    "worldbuilding": [],                 // ships EMPTY (ruled)
    "writing": ["tags", "outline", "todo"]  // ships with these three, empty (ruled)
  },
  "stackSort": { "worldbuilding": "manual", "writing": "manual" },  // per-STACK (§4.5)
  "scenes": {
    "scene_01J8Z…": {
      "worldbuilding": [ { "type": "characters", "collapsed": false } ],
      "writing":      [ { "type": "outline", "collapsed": false },
                        { "type": "history", "collapsed": true } ]
    }
  }
}
```

- **Not the scene sidecar (A):** EP-027 made scene files filesystem-authoritative and identity-bearing; adding UI
  layout to them mixes creative identity with view preference and enlarges the file EP-027 keeps canonical.
- **Not workspace state (C):** a card stack is a *creative* decision (this scene is about these characters), so it
  travels with the project and is Git-visible — it does not evaporate on another device.
- Scenes absent from `scenes` use `defaultStacks`. Deleting a scene drops its entry (cascade, like Doc 1 §5.5).

> **`selectedTab` is project-level, not per-device** (ruled 2026-08-05). The tabs belong to the **project
> window**, not to a device-native structure, so the selection travels with the project. When no selection has
> been saved, the default is **Writing** (§4.7). Narrow-display concerns are handled by hiding the whole tab view
> from a menu item (§4.7), not by making the selection device-local.

### C3 — Layout (**A**, column of cards — within tabs)

A vertical stack of collapsible cards, **not** the current segmented-tab-per-card layout
(`SceneInspectorView.swift:15-29`), which cannot express "several cards visible at once" — the premise of a card
stack. Tabs-per-card hide everything but one; a writer wants Characters *and* Outline in view.

Rejected **C** (draggable grid): a narrow inspector has one usable column; free-form 2-D layout buys nothing and
costs a persistence/hit-testing model. Card **order** stays writer-controlled (drag to reorder within a column).

Fixed `maxWidth: 280` (`SceneInspectorView.swift:31`) becomes a **resizable** pane with a sensible minimum; map
and history cards are cramped at 280.

**Note the distinction from §4.7:** C3=A governs what lives *inside* a tab. The bottom tabs group cards by
family; they do not return to one-card-per-tab.

### C4 — Loading (**A**, eager on scene select)

Doc 1 §5.4 loads the whole edge graph into memory at project open, so "which objects relate to this scene" is an
**in-memory multimap lookup** — no I/O per card. Lazy loading would add state for no gain.

The exception that *looks* like it needs **C**: world-scoped cards whose world is offline. That is handled by
pending state (§7.2), not by a loading strategy — a pending card renders immediately with its pending
presentation; it does not hang waiting.

### C5 — Add-object flow (**C**, both entrances + in-stack creation)

- **Picker sheet** for browsing: lists **all objects in the project across all worlds** (Doc 3 §4.1 —
  deliberately unfiltered), grouped by world with kind filters the *writer* applies.
- **Inline type-ahead** in the card for the common fast path ("+ Add Character" → type "Ad" → Ada).

Both create the same Doc 1 edge; they are two entrances to one operation. The picker also offers **"Create
new…"** — see §4.6, which rules how creation happens.

### C6 — Card config granularity (**B**, with sort lifted to the stack)

Per-**card** options: collapsed state, and for object cards an optional kind sub-filter.

**Sort is per-stack, not per-card** (ruled). Three cards sorted by name while two sort by creation date is
incoherent to read and fiddly to configure. One sort applies to every card in a stack, set once at the stack
level, stored as `stackSort` (§C2).

`sortIndex` (Doc 1 §5.2) still backs the **manual** sort option — it stays reachable, just chosen once per stack
rather than per card.

### 4.5 (note) Why sort belongs to the stack

The writer's mental model is "how is this inspector ordered," not "how is each card ordered." Per-card sort also
interacts badly with C1's bulk-apply: applying a layout to all scenes would carry five independent sort settings
that a writer never deliberately chose.

### 4.6 In-stack creation **and editing** — no modal, ever (ruled 2026-08-05)

Neither creating **nor editing** an object may open a modal dialog. A modal interrupts writing, which §2 forbids.
Both operations happen **in place, in the card, in the stack**.

**Editing follows the same rules as creation** (ruled): a card enters an **edit state** in place, stays editable
while the writer writes, may be left unfinished, and is resolved on scene change (below). The only difference is
what a discard means — see the table at the end of this section.

Instead, a new object is created **as an editable card in the stack**, with blank fields:

1. The writer picks "Create new…" (or types a name that matches nothing).
2. A card appears **in edit state**, fields blank and editable in place.
3. She may fill it in immediately, partially, or **leave it under construction** while she writes — filling
   details as the scene develops. This is an explicitly supported working mode, not a degenerate case.
4. The card persists in edit state for as long as she stays on the scene.

**On scene change, unfinished card edits must be resolved.** When the writer leaves a scene with cards still
under construction, a dialog surfaces reminding her of the unfinished edits, offering:

- **Complete them** — return to the scene and finish, or
- **Discard** — the card **disappears uncreated**; no object is written, and no edge is created.

> **Why this is the one permitted interruption.** §2 bars modals that interrupt writing; this dialog fires
> exactly when the writer is *leaving* a scene, i.e. at a natural break rather than mid-sentence. The
> alternative — silently discarding or silently persisting half-made objects — either loses her work or litters
> the project with blank objects she never chose to create. Surfacing at the boundary is the least-interrupting
> way to avoid both. **It must never fire while she is writing within a scene.**

**Model consequence:** a card under construction is **app-side only** until committed. No `objectID` is
allocated, no `objects/` file is written, and no edge exists until the writer completes the card. Discard is
therefore a pure app-side drop — nothing to clean up in ScriviCore, and no orphan created.

#### 4.6.1 Create vs. edit — discard means different things

Both use one edit state and one scene-change prompt, but the **destructive meaning of "discard" differs**, and
the UI must say which is which rather than showing one generic "Discard changes?":

| | Under construction (**new**) | Being edited (**existing**) |
| --- | --- | --- |
| Backing object | none — app-side only | exists on disk, unchanged |
| Discard means | the card **disappears uncreated**; nothing is written | **edits are reverted**; the object and card remain as they were |
| Risk if confused | — | a writer who reads "discard" as "delete the character" will avoid it and be stuck |

> **This is a real hazard, not a wording nitpick.** One prompt covering both cases must name the consequence
> concretely — "Discard the new character *Ada*?" vs. "Revert your changes to *Ada*?" A generic prompt makes the
> destructive case and the harmless case indistinguishable, and the writer will assume the worse one.

#### 4.6.2 Cards in edit state must be visually distinct (ruled)

A card being created or edited must be **immediately distinguishable** from a settled one — the writer needs to
see at a glance what is unfinished, especially when leaving the scene:

- A persistent edit affordance on the card (border/tint treatment plus a state indicator in the header).
- **New vs. edited should read differently** where practical, since §4.6.1's consequences differ.
- The treatment must not rely on color alone (accessibility), and must survive the card being collapsed — a
  collapsed card under construction still needs to show it.

Exact visual design is EP-A implementation, but the *requirement* is binding: an unfinished card that looks
settled will be abandoned by accident, which is precisely what the scene-change prompt exists to prevent.

### 4.7 Bottom tabs — card families (ruled 2026-08-05)

The inspector carries **bottom tabs** grouping cards by family. Tab selection **persists across launches**
(`selectedTab`, §C2) and **does not change when the scene changes** — switching scenes reloads that tab's cards
for the new scene (per C4=A) while the writer stays where she was.

**Tab order is `Writing | Worldbuilding | Properties`** (ruled), and **Writing is the default** when no tab
selection has been saved:

| # | Tab | Contents | Kind |
| --- | --- | --- | --- |
| 1 | **Writing** ← *default* | tags, todo, outline, history, sources | **card stack** |
| 2 | **Worldbuilding** | characters, locations, buildings, vehicles, items, maps, artifacts, rules, chronicles | **card stack** |
| 3 | **Properties** | scene metrics and metadata (§4.8) | **not a card stack** — a fixed view |

> **Why Writing is first and default.** A new project has an **empty** Worldbuilding stack (ruled) and a Writing
> stack pre-seeded with `tags`, `outline`, and `todo` — so opening on Writing shows something useful, while
> opening on Worldbuilding would show an empty pane on first launch. Properties is placed last because it is
> reference material: **shown on request, never the landing tab.**

> **Why tabs do not contradict C3=A.** The rejected tab design was *one card per tab*, which defeats a stack.
> These tabs group **families of cards**, and each of the two stack tabs is a full C3=A scrolling column. The
> writer still sees Characters and Locations together; she just isn't shown worldbuilding and writing tools
> simultaneously in one narrow pane. This is a good trade at inspector width.

**Stable tab selection is what makes this work.** Because the tab does not follow the scene, a writer working
through several scenes on outlines stays on Writing across all of them; the inspector does not reset her context
scene by scene.

**`selectedTab` is project-level, not per-device** (ruled). The tabs belong to the **project window**, not to
some device-native structure, so the selection travels with the project in `inspector-layout.json` (§C2) rather
than living in workspace state.

**The entire tab view can be hidden/shown from a menu item** (ruled). This is the answer to narrow displays:
rather than adapting the inspector per device class, the writer hides it when it costs too much room and shows it
when she wants it. Hidden/shown state is inspector-level UI, not a per-card or per-tab setting.

### 4.8 The Properties tab (proposed — ruling requested)

**Not a card stack.** A fixed view of scene metrics and metadata, which are properties *of* the scene rather than
things the writer surfaces:

- **Author** of the scene (`createdBy`), **created** and **last modified** timestamps (`createdAt`/`modifiedAt`,
  already in the scene sidecar).
- **Metrics:** word count, character count, estimated reading time.
- **Story-time position** (from the Timeline, EP-016) — read-only here.
- **Scene History** — see the question below.

> **This tab is the one exception to §2's manual-surfacing principle, and legitimately so:** these are *facts
> about* the scene, not world content the writer chose to associate. §2 governs what the app *proposes* about
> worlds and objects; it does not bar showing a file's own metadata. Worth stating explicitly so the exception
> doesn't later get read as a violation.

**Scene History stays a Writing card** (ruled 2026-08-05) — option (b). History is an active writing tool, not a
static property, and §1 requires writing-tool cards be dismissible and positionable, which a Properties-only
history could not be. Properties may show a compact **summary** (event count, last edit, current branch) that
opens the full card; the card in the Writing stack remains the real surface, and EP-019's T-0215 folds in there.

#### 4.8.1 Read-only now, editable-capable by construction (ruled 2026-08-05)

Everything defined for Properties **today** is read-only, and some of it is inherently so (`createdAt`, computed
metrics). But **the Properties view must not be architected as a read-only surface** — future properties may well
be modifiable, and the UI should be built with that case in mind.

Practically, for EP-A:

- Build Properties as a **field-driven view** where each field declares its own editability, rather than a static
  formatted block. Adding an editable field later becomes a field-level change, not a rewrite.
- Fields that are read-only **because the data is derived** (metrics, timestamps) should be marked as such
  distinctly from fields that merely have no editor yet — the two are different facts and will diverge.
- Editing, when it arrives, follows §4.6: **in place, no modal**, consistent with the rest of the inspector.

> **Why this is worth writing down now.** A read-only view is the easy build, and it is the one that has to be
> thrown away. Committing to the field-driven shape while the tab is still unbuilt costs almost nothing; adding
> editability to a hard-coded read-only layout later costs a rewrite of the whole tab.

---

## 5. Card protocol (Swift, app-side)

Swift is UI only (Architecture v0.3); cards call ScriviCore through `ScriviEngine` and never implement logic.

```swift
protocol InspectorCard: Identifiable {
    static var typeID: String { get }          // stable; persisted in inspector-layout.json
    static var title: String { get }
    associatedtype Body: View
    @MainActor func body(context: CardContext) -> Body
}

struct CardContext {
    let sceneID: String
    let engine: ScriviEngine
    let config: CardConfig                     // collapsed, sort, per-card options (C6)
}
```

A **registry** maps `typeID` → card implementation, so `inspector-layout.json` stays decoupled from Swift types
and an unknown `typeID` (a newer Scrivi wrote the file) is **skipped with a notice, never a crash** — forward
compatibility matching how ScriviCore treats unknown schema keys.

---

## 6. Object cards and the graph (EP-B)

An object card for kind *K* on scene *S*:

1. `scrivi_list_edges_for(project, sceneID: S)` → all edges touching *S* (in-memory, §C4).
2. Resolve each opposite endpoint's kind via the **object index** (Doc 1 §4.2) — endpoints are bare `{id}`.
3. Keep endpoints of kind *K*; order by the card's sort (`sortIndex` for manual).
4. Render each with `displayName`, `subtitle`, and thumbnail (`image.thumbnailAssetID`).

Adding an object creates one **canonical** edge (Doc 1 §5.2–5.3): the card supplies the relation type, ScriviCore
normalizes direction and rejects duplicates. **The card never writes a reverse edge** — the inverse label is a
read-time projection, which is exactly the Cumberland trap Doc 1 §5.2 exists to avoid.

Removing an object from a card **deletes the edge, not the object** — and per Doc 1 §5.5 the object survives as an
orphan. The UI wording must reflect this: **"Remove from scene"**, never "Delete". A separate, explicit
destructive action deletes an object outright.

---

## 7. Failure, pending, and warning surfaces

### 7.1 Independent card failure *(rescoped to soft failures 2026-08-11, user-approved — EP-030 AC12 / T-0399)*

A card that **fails to load or produce its content** never blocks the stack (§3). The failed card renders an
error inline, in place of its content, and the rest of the inspector works normally. This is guaranteed by the
**framework** — a card that does not report its own error still gets the fallback — not left to each card.

**Soft failures only.** This covers a card that throws while loading, returns no usable content, or depends on
something unavailable (an offline world, §7.2). It does **not** cover a card whose view body **traps**:

> ⚠️ **SwiftUI cannot catch a trapping view body.** There is no `try`/`catch` equivalent around a `View`, and a
> runtime trap terminates the process — no parent view, boundary, or wrapper can contain it. The original
> wording ("one card's failure never blocks the stack") was therefore not achievable as stated for hard
> failures, and is superseded by this section. A trapping card is a **defect in that card**, caught by tests
> and code review, not something the stack can absorb at runtime.

**Layering.** Cards *should* still report their own load errors where they can — that produces a specific,
useful message (`CardErrorView`, e.g. *"Couldn't load notes for this scene"*) instead of a generic one. The
framework fallback is the **backstop** for cards that don't, so isolation does not depend on every card
author remembering. Both layers are required; neither replaces the other.

**Verification.** Not reachable from the UI — there is no way to make a card fail by using the app. This
criterion is verified with a **test-only failing-card fixture**, never by live inspection.

### 7.2 Pending presentation (world unavailable — Doc 3 §4.6)

When a world-scoped card's world is unavailable, the card renders **pending**, not empty and not broken:

```
┌─ Artifacts ───────────────── ⚠ ─┐
│  ⟨Midgard: Sword of Dawn⟩       │  ← cached index entry: named, not a bare ID
│  ⟨Midgard: The Grey Crown⟩      │
│  ─────────────────────────────  │
│  World "Midgard" is offline.    │  ← specific status, not a generic failure
│  These links are held pending.  │
└─────────────────────────────────┘
```

#### 7.2.1 Three unavailability states (ruled 2026-08-05)

**The pending behavior is identical in all three cases** — hold, never prune, never modify (Doc 3 §4.6). Only the
*reported status* differs, and ScriviCore should distinguish them where the platform permits:

| Status | Condition | Writer's likely remedy |
| --- | --- | --- |
| **offline** | The world package lives on a **remote/network volume not reachable in the current network configuration**. | Reconnect to the network / VPN |
| **unmounted** | The world lives on a **volume that is not mounted** — e.g. removable media. | Insert or mount the drive |
| **missing** | The containing folder **is available and mounted**, the reference was **not** a link to elsewhere, and the package **is not there**. | Relink or restore from backup |

The distinction is **diagnostic, not behavioral** — it changes the sentence shown to the writer, nothing else.
Getting it right matters because the three remedies are completely different: telling a writer her world is
"missing" when the NAS is merely unreachable would invite her to restore from backup or clear references, when
all she needed was to reconnect.

> **Best-effort, and it must degrade safely.** Distinguishing these requires volume/mount inspection that varies
> across Scrivi's seven target platforms, and Doc 3 §4.4.1 forbids a platform-specific *model*. Therefore:
> status detection is a **best-effort diagnostic** in the platform layer feeding a neutral enum, and when it
> cannot be determined the status falls back to a **generic "unavailable"** — never to a *guess*. A wrong
> "missing" is worse than an honest "unavailable", because only one of them invites destructive action.

Hard rules, inherited from Doc 3 §4.6 and non-negotiable in the UI:

- Pending items are **shown, not hidden** — hiding them makes it look as though the writer's work vanished.
- The card is **read-only toward that world**: add/remove into an unavailable world is refused (the graph is
  frozen, Doc 3 §4.6), with the affordance disabled and explained rather than absent.
- **Nothing is offered that would destroy pending links.** Any "clean up" affordance belongs solely to the
  explicit Worlds-menu action (§7.3), never inline on the card where it reads as routine tidying.

> This is why Doc 3 §5 argues the object index should **cache** world entries: without a cached `displayName`,
> a pending card can only show opaque IDs, and a writer cannot judge what she'd lose. The cache is what makes
> this presentation possible at all.

### 7.3 Warning surface

A passive, non-blocking indicator when any referenced world is unavailable:

- A **⚠ badge on affected cards** (§7.2).
- A **warning/log view beneath the Timeline view** (Doc 3 §4.6) listing unavailable worlds and pending counts.
- The global **"Remove All World References"** lives in the **Worlds menu** — never on a card, never in the
  warning view's primary position. It is destructive and must be sought out deliberately.

**No modal, no blocking dialog, at any point in this flow.**

---

## 8. Reconciliation & sequencing

- **Replaces** `SceneInspectorView.swift` (tabs → card stack). The `InspectorTab` enum and `SceneEntitiesTabView`
  stub are removed; the fixed 280pt frame becomes a resizable pane.
- **EP-019 / T-0215:** the history panel ships as the `history` **card** rather than a separate panel. This is
  what lets EP-019 close (Doc 1 §10 step 4) without waiting for the object model.
- **Doc 1:** object cards consume the graph, index, and orphan/list endpoints as designed; this doc adds **no**
  ScriviCore requirements beyond them.
- **Doc 3:** pending presentation and warning surfaces implement §4.6's writer-facing half.
- **Pbxproj:** every new Swift file under `Scrivi/` must be added to `Scrivi.xcodeproj/project.pbxproj` \*\*in the
  same step\*\* (CLAUDE.md, non-negotiable).

**Sequencing** (unchanged from Doc 1 §10): Doc 2 approved → **EP-A** (framework + writing-tool cards, incl.
`history`) → **close EP-019** → **EP-B** (object cards on Doc 1's model). EP-A deliberately depends on almost
nothing from Doc 1, which is what allows EP-019 to close first.

---

## 9. Acceptance criteria (draft, EP-A)

**Tabs & layout**

1. The inspector renders **bottom tabs in the order `Writing | Worldbuilding | Properties`**; the two stack tabs
   each render a **vertical column of collapsible cards**. The old one-card-per-tab layout is gone.
2. With **no saved tab selection, the inspector opens on Writing** (§4.7).
3. **Tab selection persists at project level** (in `inspector-layout.json`, not per-device) and **does not change
   when the scene changes**; switching scenes reloads the current tab's cards for the new scene (§4.7).
4. **The whole tab view hides and shows from a menu item**, and that state persists (§4.7).
5. The inspector pane is resizable; cards remain usable at the minimum width.

**Stacks & cards**

6. A writer can add, remove, and reorder cards on a scene; stacks persist across relaunch via
   `inspector-layout.json`.
7. **No worldbuilding card ever appears without an explicit writer action** (§2) — the Worldbuilding tab's
   default stack **ships empty**, and nothing is inferred from scene content.
8. The **Writing** tab ships with an empty `tags`, `outline`, and `todo` card; all three are dismissible and the
   writer's choice persists (§1).
9. "Apply layout to all scenes" (C1) applies the current stack; each scene's entry remains independently
   editable afterward.
10. **Sort is set once per stack** and applies to every card in it; no per-card sort exists (C6).
11. An unknown `typeID` in `inspector-layout.json` is skipped with a notice; the inspector still renders.
12. One card failing to load does not prevent other cards from rendering; the failed card **shows a warning in
    place of its content** (§7.1).
13. The `history` card provides T-0215's behavior (windowed tree, branch selection, stale badges, purge), and
    lives in the **Writing** stack (§4.8).

**Properties tab**

14. The **Properties** tab renders scene author, created/modified timestamps, and metrics, and is **not** a card
    stack (§4.8).
15. Properties is built **field-driven**, each field declaring its own editability, so a future editable property
    is a field-level addition rather than a rewrite (§4.8.1). Derived-and-therefore-read-only fields are marked
    distinctly from fields that simply have no editor yet.

**EP-B additions:**

16. An object card lists exactly the objects the writer related to that scene — never inferred.
17. The object picker lists objects from **all worlds in the project**, unfiltered by the scene's world
    association (Doc 3 §4.1), and offers "Create new…".
18. **Neither creating nor editing an object opens a modal.** Both happen in place in the card, and a card may be
    **left under construction** while the writer writes (§4.6).
19. **Cards in edit state are visually distinct** from settled cards, without relying on color alone, and remain
    distinguishable when collapsed (§4.6.2).
20. **Leaving a scene with unfinished card edits surfaces a dialog** offering complete-or-discard. The prompt
    **names the consequence concretely** and distinguishes the two cases (§4.6.1): discarding a **new** card
    makes it disappear uncreated (no object, no edge); discarding an **edit** reverts changes and leaves the
    object intact. The dialog **never** fires while the writer is working within a scene.
21. Adding an object from either entrance (picker or type-ahead) creates **one canonical edge**; adding the same
    relationship from the other endpoint is rejected as a duplicate.
22. **"Remove from scene" deletes the edge only** — the object survives as an orphan and remains findable via
    `scrivi_list_orphaned_objects`.
23. **Unavailable world:** affected cards render **pending with named entries**, add/remove is refused, no card
    offers to clear pending links, and nothing is silently dropped (§7.2). Reattaching the world restores the
    card with no writer intervention.
24. **Unavailability status is reported specifically** where determinable — **offline** (remote volume
    unreachable), **unmounted** (volume not mounted), or **missing** (folder present and mounted, package absent)
    — falling back to a generic "unavailable" rather than guessing (§7.2.1). Pending behavior is identical in all
    cases.

---

## 10. Open questions

**Answered by the 2026-08-05 rulings:**

- ~~`source` (citations) — object or app-side only?~~ **Citations are reusable across scenes**, so `source` is a
  real `objects/` file, not a per-scene app-side artifact.
- ~~Chapter-level inspector?~~ **No — scene-only.** Chapters can be *selected*, but selecting a chapter does not
  change manuscript position; only scenes do. With sceneless chapters removed from the Scene Navigator, a
  chapter-level inspector has nothing coherent to inspect.
- ~~`lookahead` behavior?~~ **Deferred to at least v2** (§3.2).
- ~~Should the default stack ship empty?~~ **Split by tab:** Worldbuilding ships **empty**; Writing ships with
  empty `tags`, `outline`, and `todo` cards.

- ~~Where does Scene History live?~~ **A card in the Writing stack** (§4.8), with Properties optionally showing a
  compact summary that opens it. History is an active writing tool and must stay dismissible/positionable.
- ~~Should `selectedTab` be project-level or per-device?~~ **Project-level.** The tabs belong to the project
  window, not a device-native structure; narrow displays are served by hiding the tab view (§4.7).
- ~~Is the Properties tab read-only?~~ **Read-only content today, but not a read-only architecture** — build it
  field-driven so future editable properties don't force a rewrite (§4.8.1).

**Still open (design detail, not blocking EP-A):**

1. **Properties field list** — §4.8 names author, timestamps, metrics, and story-time position. Additional
   fields can be added as the tab is built; none are load-bearing for the framework.
2. **Visual treatment for edit state** (§4.6.2) — the *requirement* is ruled; the specific design is EP-A
   implementation work.
