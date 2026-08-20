# Scrivi — Object Detail Sheets & Project↔World Interaction

**Document:** `Scrivi_Object_Detail_Sheets_Design_v0_1.md`
**Status:** 🟢 **APPROVED FOR PLANNING — ✅ all nine trades (D1–D9) and all three decisions (Q-a/Q-b/Q-c)
ruled 2026-08-20.** ⚠️ **EP-034 precedes EP-032 and starts at SP-115.** iOS/visionOS explicitly deferred.
**Date:** 2026-08-20
**Author:** Claude (planning), for user review.
**Epic:** **EP-034** `[Cross]` Object Detail & Media — ⚠️ **widened in place** by this document (user ruling 2026-08-20), from "object fields beyond `displayName`" to the full Detail Sheet + relationship surface +
Project↔World interaction.

**Extends:**
- `Scrivi_Worldbuilding_Object_Model_v0_2.md` (Doc 1) — the object/edge model this surface reads and writes
- `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` (Doc 2) — the card stack this surface is entered from
- `Scrivi_World_Data_Separation_v0_1.md` (Doc 3) — world packages, pending semantics, `.scrivworld`
- `Scrivi_Architecture_v0_3.md` — Swift is UI only; the boundary is JSON-over-C-ABI

**Existing code this builds on:** `Scrivi/Views/Inspector/ObjectCard.swift` (919 lines) — today the object
surface in its entirety, and it edits **one field**.

---

## 1. The writer's case (verbatim, 2026-08-20)

This document exists to serve one described workflow, quoted so it is not paraphrased away:

> "The current Object Kind creation UI only allows me to enter a name for the object being created. That is
> perfectly fine for the writer when she is writing. She will want to make a note that \*'an object of this
> kind goes here'\* and then continue writing. But at some time, she will want to stop and work on fleshing
> out the objects she has created."

> "For example the character **Mara** in *The Stairs of Tintagael* requires a significant
> description/backstory so that the writer knows why she is the way she is. Initially she is writing a scene
> that she is in and so creates the character and moves on. Later she will want to, while inside Scrivi, take
> a look at the character to expand on that backstory."

**Two modes, one object.** This is the governing insight of the whole design:

| Mode          | Interaction                      | Cost to the writer          | Exists today?           |
| ------------- | -------------------------------- | --------------------------- | ----------------------- |
| **Capture**   | Name it, keep writing            | Must be \\\\\\\~zero        | ✅ Yes — and it works    |
| **Cultivate** | Open it, flesh it out, relate it | May be arbitrarily involved | ❌ **No surface at all** |

⚠️ **Capture is not broken and must not be "improved."** The one-field creation flow is correctly designed
for the writing loop, and Doc 2 §2's inherited ruling — \*nothing is presented, proposed, auto-populated, or
suggested\* — protects it. **This document adds the second mode; it does not modify the first.**

---

## 2. ⚠️ What already exists — verified against the code, 2026-08-20

\*\*This section is the most important finding in the document, and it inverts the expected shape of the work.\*\* Every row was checked, not assumed:

| Capability                                                         | Status      | Evidence                                                                                 |
| ------------------------------------------------------------------ | ----------- | ---------------------------------------------------------------------------------------- |
| `displayName`, `subtitle`, `notes`, `tags`, `attributes`, `status` | ✅ **Ship**  | `ObjectTypes.hpp:205-228` (`WorldObjectFields`)                                          |
| `image.assetID` / `thumbnailAssetID`                               | ✅ **Ship**  | `ObjectTypes.hpp:192-199` (`ObjectImageRef`)                                             |
| Read an object with all fields                                     | ✅ **Ships** | `scrivi_open_object` (`scrivi.h:121`)                                                    |
| **Write** an object with all fields                                | ✅ **Ships** | `scrivi_save_object` takes **full object JSON** (`scrivi.h:127`)                         |
| Create / delete / list edges                                       | ✅ **Ship**  | `scrivi_create_edge` `:153`, `scrivi_delete_edge` `:160`, `scrivi_list_edges_for` `:168` |
| Relation-type vocabulary (read + upsert)                           | ✅ **Ship**  | `scrivi_list_relation_types` `:284`, `scrivi_upsert_relation_type` `:286`                |
| Import / list / remove assets                                      | ✅ **Ship**  | `scrivi_import_asset` `:290`, `scrivi_list_assets` `:299`, `scrivi_remove_asset` `:303`  |
| Swift engine wrappers for **all of the above**                     | ✅ **Ship**  | `ScriviEngine.swift:265,283,325`; `ScriviEngineGraph.swift:35,59,75,257`                 |

> ⚠️ **The backend for the Detail Sheet is ALREADY BUILT, and so is its Swift binding.** With two exceptions in §3, this Epic is **a UI Epic wearing a `[Cross]` tag**. That is a materially different — and smaller — risk profile than EP-034's backlog entry assumed, and it should be reflected in how the sprints are sized.

⚠️ **This is `project_capability_without_surface` at its largest scale yet.** EP-031 produced that defect
five times; here the *entire object-editing capability* has shipped and verified while the app exposes a
single `TextField("Name")` (`ObjectCard.swift:647`).

> ⚠️ **The lesson, stated correctly (user, 2026-08-20):** \*"Ensure all core features have a UI the writer can reach!"\*
> 
> **This is not the same as "stop shipping core," and the difference matters.** The core work here is
> sound, tested and correct — ⚠️ **the defect is the missing surface, not the shipped capability.** Read the
> other way, the lesson would argue for building *less* core, which would be exactly the wrong correction:
> the remedy is to **finish the path to the writer**, not to slow down the engine.

---

## 3. ⚠️ The two REAL gaps — both at the Project↔World boundary

Two things are missing at the app's reach. ⚠️ **Neither is a missing ScriviCore CAPABILITY** — §3.1 was rewritten after the user correctly challenged that claim. \*\*Both are exactly the Project↔World interaction this Epic is named for\*\*, which is strong evidence the user's framing is the right one.

### 3.1 ⚠️ GAP-1 — Assets cannot *yet* be written to a world — ⚠️ **but ScriviCore is already capable**

> ⚠️ **THIS SECTION WAS REWRITTEN 2026-08-20 after the user challenged its first draft.** The draft asserted this was *"the single largest piece of backend work in the Epic."* **That was wrong.** The user's objection — \*"Just because Scrivi does not write assets to the world yet does not mean ScriviCore is not capable of doing so"\* — was correct, and a proper reading of `AssetStore` proves it.

#### What the code actually says

**`AssetStore` is entirely ROOT-AGNOSTIC.** Every operation derives its location from one parameter:

```cpp
// AssetStore.cpp:12-17 — the ONLY place a path is decided
AbsolutePath AssetStore::categoryDir(const AbsolutePath& projectRoot, AssetCategory cat) {
    return util::join(util::join(projectRoot, "assets"), assetCategorySubdir(cat));
}
```

⚠️ **The parameter is *named* `projectRoot`, but nothing in the class requires it to be one.** Verified by reading all three operations end to end (`AssetStore.cpp`, 180 lines):

| Operation | Project-specific logic? | What it actually does                                                            |
| --------- | ----------------------- | -------------------------------------------------------------------------------- |
| `import`  | ❌ **None**              | `createDirectories(categoryDir(root))` → copy bytes → write `.meta.json` sidecar |
| `list`    | ❌ **None**              | Scans `categoryDir(root)` per category for `*.meta.json`                         |
| `remove`  | ❌ **None**              | Scans the same dirs for a matching `assetID`, deletes both files                 |

\*\*No project manifest is read. No `project.json` is touched. No index is updated. No validation asserts the root is a project.\*\* The class is a self-contained directory store that happens to be *called* with a project root today.

#### The chain from the C ABI is a pure pass-through

- `scrivi_import_asset` copies its `projectRootPath` argument into the request **with no validation**  (`scrivi_c_api.cpp:1286-1288`)
- `ScriviCore::importAsset` constructs an `AssetStore` and forwards **verbatim**  (`ScriviCore.cpp:208-212`)
- `ImportAssetRequest::projectRootPath` is a plain `AbsolutePath` (`Requests.hpp:131-137`)

> ⚠️ \*\*Consequence: passing a `.scrivworld` package path to today's `scrivi_import_asset` would write to `<world>/assets/<category>/` and it would work.\*\* The world's `assets/` directory that `WorldStore.cpp:126` creates is not unreachable — it is simply **unaddressed**, because no caller has ever passed a world root.

#### Binary safety — checked, because an image is not text

The names `readTextFile` / `atomicWriteTextFile` invite the assumption that assets would be corrupted.
**They would not:**

- `LocalFileSystem::readTextFile` opens `std::ifstream(path, std::ios::binary)` and streams via `rdbuf()`
  (`LocalFileSystem.cpp:50-60`) — no newline translation, no truncation at NUL
- `util::atomicWriteTextFile` opens `std::ofstream(tmpPath, std::ios::binary | std::ios::trunc)` and calls
  `out.write(data, size)` with an explicit length (`AtomicWrite.cpp:19-24`)

⚠️ **Both paths are byte-exact.** The "Text" in the names is a misnomer this Epic need not fix.

#### So what IS actually missing?

Not capability — **addressing, safety and the C ABI surface**:

| \\\\\\# | Missing piece                                                                                                                                                      | Size                            |
| ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------- |
| **1**   | A way for a caller to **say which world** — `worldID` on the asset requests + the C ABI, resolving through `WorldStore::resolve` to a `packagePath`                | Small                           |
| **2**   | ⚠️ **World-availability refusal.** An unavailable world must fail honestly, never write                                                                            | Small — ⚠️ **but load-bearing** |
| **3**   | ⚠️ **Write-lock acquisition.** World packages are write-locked (`WorldStore::lockPath`, `WorldLock`); an asset write must take the lock like any other world write | Small                           |
| **4**   | Objects storing `image.assetID` must resolve it against **the right root** when reading back                                                                       | Small                           |

⚠️ **The pattern for #1 and #2 ALREADY EXISTS and should be copied, not invented** — `ObjectStore::kindDir` does exactly this for world-scoped objects (`ObjectStore.cpp:47-69`): require a `worldID`, resolve it, refuse when `status != available` with `detail = "worldUnavailable:<status>"`, then join the subdir. \*\*The asset path should mirror it line for line\*\*, including the error detail, so both surfaces fail identically.

> ⚠️ **Revised assessment.** GAP-1 is **not** a large architectural change. It is a \*\*parameter-threading and safety change\*\* over a store that is already location-neutral, following a pattern already proven in the same codebase. \*\*The trade in D6 is about WHERE assets should live — a data-ownership question — not about whether ScriviCore can put them there.\*\* It can.

⚠️ **The correction generalizes, and that is why it is recorded here rather than quietly fixed:** the first draft inferred an *incapability* from an *unused* code path. \*\*"No caller does X" is not evidence that "the core cannot do X."\*\* The same reasoning error would have inflated this Epic's backend estimate and
mis-sized its sprints.

### 3.2 ⚠️ GAP-2 — Nothing resolves an asset to something displayable

`AssetMeta` carries `filename` — *"basename only, e.g. `map.png`"* (`AssetTypes.hpp:46`) — and `scrivi_list_assets` emits only `assetID`, `filename`, `category`, `title` (`scrivi_c_api.cpp:1324-1330`).  There is **no** `scrivi_read_asset`, no `scrivi_asset_path`, no thumbnail endpoint (grep over `scrivi.h`).

So an app holding `image.assetID` **cannot turn it into pixels** — from a *list*. → **Trade D7.**

> ⚠️ **But the core already knows the path, and already returns it once.** `ImportAssetResult` carries `assetPath`, and `scrivi_import_asset` emits it in its envelope (`scrivi_c_api.cpp:1300-1305`). \*\*The path is disclosed at import time and then never again.\*\*

> **Same lesson as §3.1, in a smaller costume:** the capability is present; the *addressing* is missing.  The likely fix is not a new subsystem but **adding the path to the `list_assets` envelope** (and/or a  small resolve endpoint) — ⚠️ which makes D7 considerably cheaper than its options imply.

⚠️ **Sandbox consequence:** the app is sandboxed and reaches projects/worlds through security-scoped bookmarks (`ProjectBookmarkStore.swift`, `WorldBookmarkStore.swift`). A world on a **USB drive** — the real test rig — means an asset path may be **unreachable exactly when the world is unavailable.** Whatever D7 rules must degrade the way §6 requires, not crash or hang.

---

## 4. The Detail Sheet — required behavior

Stated as requirements, all traceable to the user's brief. Trades follow in §5.

**R1 — It is a first-class surface, not a popup.** *"This is not a popup anymore."*

⚠️ **R0 (added 2026-08-20, Q-b ruling) — the Detail Sheet is ADDITIVE. It replaces nothing.**
The list item and the inline editor **both remain exactly as they are**; the sheet is opened **on request**
using one of them as the object reference. **Three paths coexist by design** — list item (identify),
inline editor (§1 *Capture*), Detail Sheet (§1 *Cultivate*). ⚠️ **A single click on a title must keep doing
what it does today**; only an explicit affordance opens the sheet.

**R2 — It shows and edits every attribute** — name, subtitle, description/notes, image, tags — \*\*view and modify from one place.\*\*

**R3 — It has a related-objects section** surfacing this object's relationship **edges**.

**R4 — Relationships can be CREATED from the sheet** — the writer picks a target and a relation type.

**R5 — It is navigable.** \*"Double click of an object in the Sheet's related object list will push navigate to another object sheet."\* **Back/forward move through that history**, standard navigation controls.

**R6 — It is the mechanism for source entries and footnote text** — closing T-0365's write half. ⚠️ Per the user's earlier ruling, creation belongs **on the documented object**, never on the scene-scoped `sources` card, *"it would muddy the fact that the source must be associated with an object in the world."*

**R7 — Entry points:** from a Kind Card list item in the Worldbuilding tab, via **double-click** or **right-click → "View Detail"**. From the related-objects list, via **double-click** or \*\*right-click →"Show"\*\*.

**R8 — Image thumbnails on card list items.** \*"A tiny thumbnail of that image should be included in the List item of the Kind Card."\*

**R9 — Pending objects are honored.** A world-unavailable object opens **read-only**, explains itself, and ⚠️ **is never rewritten** — Doc 3's *absence is never deletion*, and I-0138's disabled-and-explained rule.

---

## 5. ⚠️ Trade studies — D1–D9, all requiring rulings

### D1 — Display idiom for the Detail Sheet ⚠️ **the trade the user explicitly asked for**

⚠️ **This recommendation CHANGED after reading the app's window history.** The first draft recommended a
non-modal window (option B) largely on principle — *"not a popup"* implies a window. \*\*The code says that
carries a cost the draft did not know about\*\*, and the trade is genuinely closer than it looked.

#### ⚠️ What EP-018 already paid for, and what it proves

The app's multi-window model was hard-won, and every line of it is a warning label:

| Evidence                         | What it cost                                                                                                                                                                            |
| -------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `OpenProjectRegistry.swift:9-12` | ⚠️ *"macOS 26 `WindowGroup(for:)` de-dup is **NOT race-safe** (two rapid same-value opens both create windows)"* — an entire registry exists because native de-dup could not be trusted |
| `ScriviApp.swift:71`             | `WindowGroup(for:)` **was abandoned** — *"it cached dead windows"* (T-0194)                                                                                                             |
| `ScriviApp.swift:6-10`           | `.onOpenURL` proved unreliable *"with a Window + WindowGroup split across multiple windows"*; an **`AppDelegate` was added** to receive URLs process-wide                               |
| `ScriviApp.swift:35-38`          | `applicationShouldTerminateAfterLastWindowClosed` had to be overridden — closing the last window killed the app mid-flow                                                                |

> ⚠️ **Adding a second window TYPE means re-entering every one of these problems** — identity/de-dup (one
> sheet per object, or many?), lifecycle against project close, focus routing, and the menu-command
> targeting that `focusedSession` currently assumes.

**And there is no precedent to lean on:** grep shows the app uses **`.sheet` and `.popover` only**
(`LandingView`, `EditorView`, `SceneNavigatorView`, `TimelineStripView`). ⚠️ \*\*A detail window would be the
first auxiliary window in Scrivi\*\*, and it would arrive in the same Epic as nine other trades.

#### The options, re-scored against that evidence

| Option                                                                                                              | For                                                                                                                                                                       | Against                                                                                                                                      |
| ------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------- |
| **A — Modal sheet** (`.sheet`)                                                                                      | Simple; matches every existing dialog                                                                                                                                     | ⚠️ **Blocks the manuscript**; Doc 2 §4.6 forbids a modal for in-place editing; R5 back/forward feels wrong in a modal                        |
| **B — Non-modal utility window**                                                                                    | Truest to *"not a popup"*; several objects open at once; back/forward natural                                                                                             | ⚠️ **Re-opens all four EP-018 hazards above**; no precedent in the app; per-window state; iOS/visionOS need a different answer anyway        |
| **C — Inspector-pane push-navigation**                                                                              | Reuses the card stack                                                                                                                                                     | ⚠️ **Fights the writer's stated mode switch**; the pane is **fixed at 280pt** (`SceneInspectorView.swift:7,46`) — wrong for Mara's backstory |
| **D — Full-screen / tab takeover**                                                                                  | Maximum room                                                                                                                                                              | Loses manuscript context entirely                                                                                                            |
| **E — ⚠️ NEW: non-modal *inspector-style* pane at editor level**, sized like a document sub-view, not a 280pt strip | Non-modal ✅; *"not a popup"* ✅; **no new window type** — no EP-018 hazards; back/forward trivially scoped to the pane; ports to iPad/visionOS as a navigation destination | Shares the window with the manuscript, so it does not feel like "leaving to go work on the object"; needs its own width/collapse behaviour   |

> ### ✅ **RULED 2026-08-20 — E** (non-modal editor-level pane now; **B recorded as the successor**)
> 
> **User ruling.** The Detail Sheet is a **non-modal pane at editor level**, sized as a document sub-view —
> **not** the 280pt inspector strip, and **not** a new window. The manuscript is hidden or narrowed while
> the writer is cultivating, which is the visible expression of §1's mode switch.
> 
> ⚠️ **Binding consequence for implementation:** it must be built as a \*\*self-contained, navigable
> component that does not depend on its host\*\*. That is what keeps B (a window) available later without a
> rewrite — and it is the reason this ruling costs nothing in the long run.
> 
> *Original recommendation text follows.*
> 
> **Recommendation: ⚠️ CHANGED — E for the first delivery, with B recorded as the successor.**
> 
> **Why the change:** E satisfies every *stated* requirement — non-modal, not a popup, navigable, roomy —
> \*\*without buying a second window type in an Epic that already carries nine trades and a ScriviCore
> change.\*\* B remains the better long-term answer for a writer who wants Mara open beside chapter 12, and
> nothing in E blocks it: a detail view that is already a self-contained, navigable component \*\*can be
> hosted in a window later\*\*, which is exactly the kind of decision that should be deferred until the
> content is proven.
> 
> ⚠️ **If you want B now, that is a legitimate call** — the writer's brief does lean toward it, and doing
> it later means revisiting focus and menu routing a second time. \*\*This is the trade's real fork, and it
> is yours:\*\* ship the surface sooner in-window (E), or pay the window cost once, up front (B).

⚠️ **D1 gates D2, D3, D5 and D8** — they all sit inside whatever D1 chooses.

### D2 — Navigation model (R5)

| Option                                                                | Notes                                                                                                                 |
| --------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| **A — `NavigationStack` push/pop**                                    | Familiar; ⚠️ SwiftUI gives **back** natively; **forward needs custom history regardless**                             |
| **B — Explicit browser-style history** (stack + back **and** forward) | Matches *"standard NavigatorView buttons"* exactly; well-understood; the machinery A would need anyway, made explicit |
| **C — A new window per double-click**                                 | No history needed; ⚠️ window explosion after five hops — **and under D1-B, five EP-018 identity problems**            |

> ### ✅ **RULED 2026-08-20 — B** (explicit back/forward history)
>
> ⚠️ **D1-agnostic by construction** — the same history object serves a pane (D1-E) or a window (B later),
> which is why it was safe to rule alongside D1 rather than after it.
>
> **Recommendation: B — unchanged, and now better supported.** The user asked to \*"move backwards to the
> previously viewed Detail Sheet"*; A cannot do **forward** without exactly B's machinery. ⚠️ *\*B is also
> D1-agnostic\*\* — an explicit history object works identically in a pane (E) or a window (B), which is
> precisely why it is safe to rule now even if D1 is deferred.

### D3 — Long-form text for `notes` (Mara's backstory)

⚠️ **Checked 2026-08-20:** the app has **one** `TextEditor` today (`WritingToolCards.swift:209`), and
⚠️ **`notes` is rendered nowhere at all** — grep over `ObjectCard.swift` returns no `notes` reference. So
this is a genuinely new surface, not a modification.

| Option                                    | Notes                                                                                                    |
| ----------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| **A — Plain `TextEditor`**                | Ships today; precedent exists in the app; no new format; consistent with scene bodies being plain text   |
| **B — Markdown with preview**             | Writers expect emphasis in a backstory; scene bodies are already Markdown, so the vocabulary is familiar |
| **C — Reuse the manuscript editor stack** | Maximum power; ⚠️ drags EP-019's sentence-granular history/undo into object editing                      |

> ### ✅ **RULED 2026-08-20 — A now, B as successor; ⚠️ C RULED OUT explicitly**
>
> Plain `TextEditor` ships first; Markdown-with-preview is the recorded successor. ⚠️ **C is rejected as a
> decision, not deferred as a backlog item** — reusing the manuscript stack would pull EP-019's
> sentence-granular history engine into object editing. ✅ **A→B costs no migration**: `notes` is a plain
> string, so only the renderer changes.
>
> **Recommendation: A for the first delivery, B recorded as the successor.**
> 
> ⚠️ **C is a trap and should be ruled out explicitly, not merely deprioritized.** The manuscript editor is
> bound to EP-019's history engine, which commits sentence-granular events against **scene** byte spans.
> Pointing it at an object's `notes` means either inventing a second history domain or corrupting the
> first. **That is an Epic of its own, and it would arrive uninvited inside this one.**
> 
> ⚠️ **A→B is cheap precisely because `notes` is a plain string in the model** (`ObjectTypes.hpp:222`):
> upgrading the *renderer* later changes no schema and migrates no data.

### D4 — Where relationship creation lives (R4)

⚠️ **Checked 2026-08-20 — most of this already exists.** `ObjectPickerView` (221 lines) already selects an
object **and** a relation type, then calls `createEdge`; ScriviCore normalizes direction and rejects
duplicates. Its header records the AC17 ruling that matters here: it lists objects \*\*from all worlds,
deliberately unfiltered\**, because *"a writer relating a character to a scene must be able to reach a
character that lives in another world."\*

| Option                                                                            | Notes                                                                                                                           |
| --------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| **A — Inline in the related section** ("+ Add relationship" → picker + type menu) | Fewest clicks; visible where the result lands; **reuses `ObjectPickerView`**                                                    |
| **B — Separate modal picker**                                                     | Clean; ⚠️ a modal launched from a non-modal pane (D1-E) is an idiom clash                                                       |
| **C — Drag-and-drop between sheets**                                              | Delightful; ⚠️ undiscoverable alone, and **VNC cannot carry it** for Linux verification (`project_linux_vnc_input_constraints`) |

> ### ✅ **RULED 2026-08-20 — A** (inline, reusing `ObjectPickerView`)
>
> ⚠️ **Required change:** widen the picker's hardcoded scene-target filter
> (`ObjectPickerView.swift:209`) to object→object. ✅ **No ABI change** — the `"scene"` sentinel already
> round-trips. ⚠️ **Never gesture-only** (C fails Linux/VNC parity), and ⚠️ **check T-0416 against the real
> rig before this sprint plans.**
>
> **Recommendation: A**, reusing `ObjectPickerView` rather than writing a second picker.
> 
> ⚠️ **One real change is required, and it is the interesting part of this trade.** The picker currently
> hardcodes a **scene** target:
> 
> ```swift
> // ObjectPickerView.swift:209
> $0.acceptsSource(kind: cardKind.kind) && ($0.targetKind == nil || $0.targetKind == "scene")
> ```
> 
> For object→object it must filter to types whose target is **not** a scene. ✅ \*\*The data needed is already
> at the boundary:\*\* the C ABI encodes a scene endpoint as the sentinel string `"scene"` in
> `sourceKind`/`targetKind` (`scrivi_c_api.cpp:1227-1229`), round-trips it (`:1254-1259`), and Swift models
> `nil`-as-unconstrained correctly (`ScriviEngineGraph.swift:522-536`). **No ABI change is needed for D4.**
> 
> ⚠️ **Never make C the only path** — a gesture-only affordance is unverifiable on Linux over VNC, and
> AC11 requires parity.
> 
> ⚠️ **T-0416 may surface here as a live blocker** (seeded relation types never reach existing projects): a
> writer relating objects in an **existing** project sees only whatever vocabulary that project already has
> on disk. **Worth confirming against the real `tintagael` rig before this sprint plans.**

### D5 — Sheet layout with a variable field set ⚠️ **now carries a required ABI addition**

All eleven kinds share **one** field block (`WorldObjectFields`, `ObjectTypes.hpp:205-228`) — Doc 2's
*"one card type, ten configurations"* precedent applies directly.

| Option                                                                 | Notes                                                                                                                                     |
| ---------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| **A — One layout for every kind**, kind-specific behaviour **derived** | ⚠️ Obeys CLAUDE.md's standing rule: kind lists must be **derived from `kAllStorableKinds` / `objectKindIsWorldScoped()`**, never restated |
| **B — Per-kind custom layouts**                                        | ⚠️ **Eleven places to drift.** This is precisely the restatement defect that recurred **seven times** in EP-031                           |

> **Recommendation: A, emphatically.**

#### ⚠️ Finding: the standing kind-list rule is ALREADY VIOLATED in the code this Epic touches

```swift
// ObjectCard.swift:46 — a restatement of the scope rule, in Swift
var isWorldScoped: Bool { kind != "source" }
```

⚠️ **CLAUDE.md calls this "a defect on sight, even when it is currently correct"** — and it is the \*same
site class\* as SP-104's occurrence, which *"blocked object creation in the app entirely."* It is currently
correct only because `source` happens to be the sole project-scoped kind today.

**And it is not the app's fault.** Grep of `scrivi.h` shows ⚠️ \*\*no endpoint exposes a kind's scope at
all\*\* — there is no `scrivi_list_object_kinds`, no scope field anywhere in the ABI. \*\*Swift cannot derive
what the boundary never tells it**, so it restates. **The defect is structural, not careless.\*\*

> ⚠️ **A second instance, found while checking the first — the header comment is itself stale.**
> `scrivi.h:97-99` still says world-scoped means \*"artifact / chronicle / faction / rule … pass "" for the
> project-scoped kinds, which is every other kind."\* \*\*SP-103 moved `character`, `location`, `item`,
> `building`, `vehicle` and `map` to world scope\*\*, so that sentence has been wrong since 2026-08-14.
> **This is the documented failure mode exactly:** *"a list rots without being edited"* — nobody touched
> this comment; a kind's scope changed underneath it.

**Consequence for this trade:** D5-A is **not achievable in Swift today**. Delivering it requires a small
ABI addition — an endpoint (or a field on an existing envelope) that reports \*\*each kind and whether it is
world-scoped\*\*, derived core-side from `kAllStorableKinds` + `objectKindIsWorldScoped()`.

⚠️ **This is small, and it retires a defect class that has now appeared eight times.** It should be ruled
here rather than discovered mid-sprint — and per `feedback_boundary_tests_not_facade`, \*\*its test belongs
against `scrivi_*`, not the C++ facade\*\*, which is how I-0113 shipped green.

> ### ✅ **RULED 2026-08-20 — A, and the ABI addition is IN SCOPE for this Epic**
> 
> **User ruling.** The boundary will expose **each kind and whether it is world-scoped**, derived core-side
> from `kAllStorableKinds` + `objectKindIsWorldScoped()`. The Detail Sheet then **derives** scope instead
> of restating it, and `ObjectCard.swift:46` is retired rather than inherited by a new surface.
> 
> **Both findings are to be FILED AS ISSUES**, not fixed silently inside a UI sprint:
> 
> (tbl)
> 
> ⚠️ **Test against `scrivi_*`, never the facade** (`feedback_boundary_tests_not_facade`) — a facade test
> cannot see a boundary gap, which is exactly how **I-0113** shipped green.

### D6 — ⚠️ Where world-object assets are stored (GAP-1)

| Option                                                                                         | Consequence                                                                                                                                                                                                                    |
| ---------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **A — World assets in the world package** (`<world>/assets/`, keyed by the object's `worldID`) | ✅ Correct: the world stays self-contained and **images travel with it between projects**. The directory **already exists** (`WorldStore.cpp:126`). Costs a `worldID` parameter through `AssetStore`, the C ABI, and the engine |
| **B — Keep everything in the project**                                                         | ⚠️ **Breaks world sharing** — Mara arrives at a second project with no portrait; a shared world's maps cannot travel. Rejected on Doc 3 grounds                                                                                |
| **C — Project assets + copy-on-bind**                                                          | Duplicates bytes; needs a sync story; two sources of truth                                                                                                                                                                     |

> ### ✅ **RULED 2026-08-20 — A + heartbeat during copy**
>
> World-scoped objects' assets live in `<world>/assets/<category>/`, so **images travel with a shared
> world**. ⚠️ **The copy MUST call `WorldLock::heartbeat()` during the transfer**, not only at its start —
> otherwise a large image on a slow volume can have its lock broken mid-copy at `kStaleSeconds = 60`.
> ⚠️ **`kStaleSeconds` is NOT to be raised** — that would weaken crash recovery for every world write to
> solve a problem local to one.
>
> **Recommendation: A.** ⚠️ **REVISED 2026-08-20 (see the §3.1 rewrite):** this originally read \*"the Epic's
> one significant ScriviCore change."\* **It is not significant.** `AssetStore` is already root-agnostic, so
> option A is **parameter threading + world-availability refusal + lock acquisition**, following
> `ObjectStore::kindDir`'s existing pattern (`ObjectStore.cpp:47-69`). ⚠️ \*\*This trade is a DATA-OWNERSHIP
> question — where should a world object's image live? — NOT a capability question.\*\* The Epic stays
> `[Cross]`, but on a much smaller backend footprint than first estimated.
> 
> ⚠️ **Write-lock interaction — checked, and it has a sharp edge.** `WorldLock` is RAII, acquires via
> `createFileExclusive` (*"the ONLY primitive that makes 'exactly one winner' true across processes"*),
> and treats a lock whose heartbeat is older than **`kStaleSeconds = 60`** as dead
> (`WorldStore.hpp:141-161`).
> 
> ⚠️ **An asset import is the first world write whose duration depends on FILE SIZE.** Every existing world
> write is a small JSON document; copying a large map image from a slow USB volume could plausibly approach
> or exceed 60 s — at which point **another process is entitled to break the lock mid-copy.** `heartbeat()`
> exists for exactly this and **must be called during the copy**, not merely at its start.
> 
> ⚠️ **Degrade correctly when the world is unavailable:** a writer with an ejected drive must be **told**,
> never left with a half-written asset or a `.meta.json` sidecar orphaned from its bytes. \*\*Refusal must
> mirror `ObjectStore::kindDir`'s `worldUnavailable:<status>` contract\*\* so both surfaces fail identically.

### D7 — ⚠️ How the app turns an `assetID` into pixels (GAP-2)

| Option                                                     | Consequence                                                                                                                                                   |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **A — `scrivi_asset_path` returns an absolute path**       | Cheap; SwiftUI loads the file directly. ⚠️ **Hands a raw path across a boundary whose whole design is JSON-over-string**, and the sandbox may not have access |
| **B — `scrivi_read_asset` returns bytes** (base64 in JSON) | Boundary-pure; ⚠️ base64 of a full-size image through a JSON envelope is **wasteful for large maps**                                                          |
| **C — Path for display + core-generated thumbnails**       | Best performance; ⚠️ adds a thumbnail pipeline (image decoding **in C++**) — a large new dependency                                                           |
| **D — Path + app-side thumbnail generation and cache**     | ⚠️ **No image decoding in ScriviCore**; Apple/Qt each use native imaging, which they already have                                                             |

> ### ✅ **RULED 2026-08-20 — D** (path + app-side thumbnails)
> 
> `assetPath` is added to the `list_assets` envelope; Apple and Qt each generate and cache thumbnails with
> native imaging. ⚠️ **No image codecs in ScriviCore** — a real portability and security surface across
> four platforms. ✅ **No schema change** — `thumbnailAssetID` already exists, so a generated thumbnail is
> storable as a real asset.
> 
> **Recommendation: D, with A's path endpoint as its input.** It keeps ScriviCore free of image codecs
> (a genuine portability liability in C++), and both platforms already have first-class imaging.
> ⚠️ **`thumbnailAssetID` already exists in the model** (`ObjectTypes.hpp:194`) — so a generated thumbnail
> can be stored as a real asset, which means **D does not require a schema change.**
> 
> ⚠️ **REVISED 2026-08-20:** option A is cheaper than the table implies — \*\*the core already computes
> `assetPath` and already returns it from `scrivi_import_asset`\*\* (`scrivi_c_api.cpp:1300-1305`); it is
> simply absent from the `list_assets` envelope (`:1324-1330`). A is therefore mostly \*"emit the path from
> `list_assets` too"*, **not a new subsystem.** ⚠️ *\*The sandbox/reachability concern below is the real
> substance of this trade\*\* — that, not the endpoint, is what needs your ruling.

### D8 — Card list-item thumbnails (R8)

⚠️ **Context checked:** the inspector pane is **fixed at 280pt** (`SceneInspectorView.swift:7,46`), and
`ObjectCardRow` (`ObjectCard.swift:787-835`) is a tight `HStack` — a pending badge, a name/label `VStack`,
a spacer, and a remove button.

| Option                                     | Notes                                                                                            |
| ------------------------------------------ | ------------------------------------------------------------------------------------------------ |
| **A — Thumbnail only when `image` is set** | ⚠️ **No layout shift for the common case**; the card stack stays dense, per Doc 2                |
| **B — Always reserve the slot**            | Uniform rows; ⚠️ wastes scarce width in a 280pt pane where most objects will never have an image |

> ### ✅ **RULED 2026-08-20 — A** (conditional, async, non-blocking)
> 
> Thumbnail appears **only when an image exists**; no reserved slot in the 280pt pane. ⚠️ \*\*Must never
> block the card\*\* (Doc 2 card-independence), and ⚠️ **a pending object's row is unchanged** — it has no
> reachable image by definition.
> 
> **Recommendation: A.**
> 
> ⚠️ **Thumbnails must load asynchronously and must never block the card.** Doc 2's card-independence rule
> is explicit — *"one card failing to load must not prevent others"* — and an object's image may live on
> **an unavailable world's volume**, i.e. a slow or failing read on the USB rig. \*\*A thumbnail that hangs
> the inspector would be a worse defect than no thumbnail at all.\*\*
> 
> ⚠️ **A pending object must render its row exactly as it does today** (name + ⚠ badge). It has no reachable
> image by definition, and attempting one would violate §6.

### D9 — ⚠️ I-0137: what `packagePath` means for an unavailable world

**Confirmed at the source, 2026-08-20.** `listWorlds` assigns the path on the available branch only:

```cpp
// WorldStore.cpp:385-390
auto res = resolve(projectRoot, id);
s.status = res.status;
if (res.status == WorldStatus::available) {
    s.packagePath = res.packagePath;      // ⚠️ ONLY here
    …
}
```

⚠️ **And `resolve` itself sets `out.packagePath` at exactly ONE line** — `WorldStore.cpp:303`, on the
success branch. **So the field is genuinely empty on failure**; this is not a value being dropped in
`listWorlds`, it is a value never produced. **A fix must plumb the candidate path deliberately.**

✅ **The good news:** the candidate (`cand`) is **in scope at every failure point** of the resolve loop, so
carrying it costs little.

| Option                                                                                      | Consequence                                                                                                         |
| ------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| **A — Carry the last-known candidate in a DISTINCTLY NAMED field** (`lastKnownPackagePath`) | ✅ The refinement fires; ⚠️ **the distinct name is the entire point** — it can never be mistaken for a verified path |
| **B — Reuse `packagePath` for unverified paths**                                            | ⚠️ **Rejected on the code's own stated grounds** — see below                                                        |
| **C — Refine from the app's bookmark instead**                                              | Uses what the app already holds; ⚠️ leaves the core's status coarse and duplicates world knowledge app-side         |

> ### ✅ **RULED 2026-08-20 — A** (`lastKnownPackagePath`, distinctly named)
>
> `resolve` carries the last-known candidate in a **new, distinctly named field**; `listWorlds` passes it
> through regardless of status; `WorldVolumeStatus.refine` reads it. ⚠️ **`packagePath` keeps its meaning —
> verified — and is NOT widened.** The distinct name is the safeguard, not a stylistic choice.
>
> **Recommendation: A.**
> 
> ⚠️ **B is not merely inferior — it contradicts a discipline this file defends at length.** `resolve`
> carries an explicit warning that a failed read *"is NOT evidence of absence"*, and refuses to report
> `missing` without positive proof, because \*"a wrong 'missing' invites destructive writer remedies when
> the world may be perfectly intact on an unreachable volume. NEVER GUESS."\* (`WorldStore.cpp:270-310`).
> \*\*`packagePath` is trusted by callers as verified. Quietly widening it to mean "verified, or maybe not"
> is exactly the class of change that breaks a distant caller\*\* — and I-0115 was that lesson already.
> 
> ⚠️ **What this does to AC24, stated plainly:** EP-031 marked AC24 Verified on fixture evidence that could not have distinguished this. Fixing it **completes a criterion**; it does **not** re-open a closed Epic. The user already ruled the scope of blame: \*"The Task was verified. What was not is due to unimplemented software features."\*
> 
> ⚠️ **Test it against `scrivi_*`, not the facade** — a fixture that supplies a `packagePath` passes while
> the real rig cannot. **That is precisely how this defect reached a Verified AC.**

---

## 6. Pending & unavailable worlds — the rule this surface must not break

⚠️ **Absence is never deletion.** EP-031's highest-risk property, confirmed live on real hardware, and the
Detail Sheet is the most dangerous new place to break it, because \*\*it is the first surface that WRITES
objects\*\*.

Binding requirements:

1. A pending object's sheet opens **read-only**, names its world, and explains why (§7.2, I-0138's
   disabled-**and-explained** rule — never disabled-and-silent).
2. ⚠️ **No save path may write an object whose world is unavailable.** Not a partial write, not a
   project-side shadow copy.
3. Edges to pending objects **list** with cached `otherDisplayName` — already supported by
   `scrivi_list_edges_for` — and **never prune**.
4. Relationship creation toward a pending object is **disabled and explained**.
5. ⚠️ **A world going unavailable while a sheet is open must be handled**, not just at open time. The USB
   rig can eject mid-edit. **This is a genuinely new hazard the Epic introduces.**

---

## 7. The five open Issues — SP-115's whole content

Per user ruling, the first sprint is **dedicated to these five and nothing else**.

| Issue      | Sev      | What it is                                            | Bearing on this Epic                                                                                                                                                                                                                                                                    |
| ---------- | -------- | ----------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **I-0137** | **High** | AC24 refinement cannot fire — `packagePath` withheld  | ⚠️ **Ruled by D9.** Project↔World status is this Epic's core theme                                                                                                                                                                                                                      |
| **I-0139** | Medium   | Title click opens an editor whose exit reads "Revert" | ⚠️ **A REAL FIX — ✅ Q-b ruled 2026-08-20: the Detail Sheet does NOT replace the inline editor.** The editor stays, so its defect stays. Remedy: `pendingHelp`-style explanation + a clearer exit. ⚠️ **Do not "fix" it by removing the inline editor**, by making the panel modal (§4.6), or with a Cancel that bypasses the unfinished-work prompt (**I-0119** filed an object into the wrong scene that way) |
| **I-0136** | Medium   | `world.json` `formatVersion` read, never compared     | A world written by a future Scrivi is parsed as current. ⚠️ **Cannot be retrofitted** once old readers ship — and this Epic **adds world writes** (D6), raising the stakes                                                                                                              |
| **I-0138** | Low      | "Remove from scene" disabled but unexplained          | `pendingHelp` already composes the right sentence at `ObjectCard.swift:838`; it is applied to a different control. **Small, and directly on this Epic's surface**                                                                                                                       |
| **I-0135** | Low      | No coverage for a corrupt `world.json`                | Test gap. ⚠️ **Related to I-0136** — both are world-file robustness, and D6 makes world files more load-bearing                                                                                                                                                                         |

> ✅ **Q-b RULED 2026-08-20 — patch the control.** ⚠️ **The first draft was wrong to suggest this might be
> *"resolved by design change."* **The inline editor and the list item both remain in place**; the Detail
> Sheet is opened *from* them, on request. **I-0139 is therefore an ordinary fix owed by SP-115.**

---

## 8. Proposed Epic shape

⚠️ **Per user ruling 2026-08-20, this widens EP-034 in place** — no new Epic ID. EP-034's backlog entry
already carries the finding, the user quotes, and the ruling that source creation belongs on the object.

**EP-034 `[Cross]` Object Detail & Media** *(widened)* — one Epic, because the Detail Sheet is a single
surface and splitting it would leave neither half owning it.

| Sprint     | Scope                                                                                                                                                                                                              | Codebase       |
| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------- |
| **SP-115** | ⚠️ **The five Issues, and nothing else** (I-0135–I-0139) — D9's ruling implemented for I-0137                                                                                                                      | `[Cross]`      |
| SP-116     | GAP-1 + GAP-2: world-scoped assets + asset resolution (D6, D7). ⚠️ **Smaller than first estimated** — see §3.1: parameter threading + availability refusal + lock acquisition, over an already root-agnostic store | `[ScriviCore]` |
| SP-117     | Detail Sheet shell: idiom, navigation, fields, save (D1, D2, D3, D5)                                                                                                                                               | `[Apple]`      |
| SP-118     | Related-objects section + relationship creation (D4)                                                                                                                                                               | `[Apple]`      |
| SP-119     | Images: import, display, card thumbnails (D8)                                                                                                                                                                      | `[Apple]`      |
| SP-120     | Source creation + footnote text (R6) — ⚠️ **closes T-0365's write half**                                                                                                                                           | `[Apple]`      |
| SP-121     | `[Linux]` parity                                                                                                                                                                                                   | `[Linux]`      |
| SP-122     | AC verification + ⚠️ **live-use pass on the real rig** + Epic close prep                                                                                                                                           | `[Cross]`      |

⚠️ **Eight sprints is an estimate made BEFORE these trades are ruled.** EP-031 planned 6 and delivered 11 —
**four of the five additions came from USE, not planning.** Expect this to move.

⚠️ **SP-107–SP-114 are RESERVED to EP-032** and are **not** available here. This Epic starts at **SP-115**.

### Relationship to other Epics

- **EP-032** (Inline Object References) — ⚠️ **This Epic unblocks it.** EP-032's AC5 renders footnotes for
  sources, which cannot be created until R6 ships. **Doing EP-034 first resolves that sequencing risk.**
- **EP-033** (World Lifecycle) — adjacent but distinct: EP-033 asks *who owns a world*; this Epic asks
  *how a writer edits objects inside one*. ⚠️ D6 gives EP-033 a new fact to reckon with: \*\*worlds will
  contain assets\*\*, so deleting a world starts destroying images.
- **T-0416** — seeded relation types don't reach existing projects. ⚠️ \*\*D4 may surface it as a live
  blocker\*\*: a writer relating objects in an existing project sees whatever vocabulary that project has.

---

## 9. Success criteria (draft ACs for the widened EP-034)

Written to be verifiable **by use**, per the lesson EP-031 paid for five times.

- **AC1** — From a Kind Card list item, **double-click** and **right-click → "View Detail"** both open the
  Detail Sheet for that object.
- **AC2** — The sheet **views and edits** `displayName`, `subtitle`, `notes` and `tags`; edits persist across
  app restart.
- **AC3** — An image can be **imported, displayed, replaced and removed**; ⚠️ for a **world-scoped** object it
  is stored **in the world package** and is present when that world is opened **from a different project**.
- **AC4** — Card list items show a **thumbnail when an image exists**, and are unchanged when none does.
- **AC5** — The related-objects section lists this object's edges with relation labels reading correctly
  from this endpoint.
- **AC6** — A relationship can be **created from the sheet**, choosing target and type; it appears from both
  endpoints and is rejected as a duplicate from the second.
- **AC7** — **Double-click** and **right-click → "Show"** on a related object **push-navigate** to its sheet;
  **back and forward** move through that history.
- **AC8** — A **source can be created from the documented object** and a `cites` edge attached; ⚠️ the
  scene-scoped `sources` card **then shows content for the first time**.
- **AC9** — ⚠️ A **pending** object opens **read-only, explained, and is never written**; a world going
  unavailable **while a sheet is open** is handled without data loss.
- **AC10** — All five Issues I-0135–I-0139 are **Resolved**, each verified against **its own trigger**.
- **AC11** — `[Linux]` parity for AC1–AC9, verified in Docker+VNC. ⚠️ **No gesture-only affordances**
  (VNC input constraints).
- **AC12** — `ctest` green on macOS arm64 **and** x86-64 with sanitizers; interop green; Linux container
  green; app **BUILD SUCCEEDED**.

---

## 10. ✅ Rulings — ALL NINE TRADES RULED

✅ **D1–D9 were all ruled 2026-08-20.** This table is the record; each trade section above carries the
full ruling and its reasoning. ⚠️ **Q-a, Q-b and Q-c below remain open** — **Q-a decides sprint numbering,**
**so it is the one that blocks planning.**

| Trade | ✅ Ruling (2026-08-20) | Binding consequence |
| ----- | --------------------- | ------------------- |
| **D1** | **E** — non-modal editor-level pane; **B (window) = successor** | ⚠️ **Build host-independent** so B stays available without a rewrite |
| **D2** | **B** — explicit back **and** forward history | D1-agnostic; the same object serves a pane or a window |
| **D3** | **A** now, **B** successor; ⚠️ **C ruled OUT** | C would pull EP-019's undo engine into object editing |
| **D4** | **A** — inline, reusing `ObjectPickerView` | Widen the scene-target filter (`:209`); ✅ no ABI change; ⚠️ never gesture-only |
| **D5** | **A** — one layout; ⚠️ **ABI kind-scope endpoint IN SCOPE** | Retires `ObjectCard.swift:46`; both findings **filed as Issues**; test vs `scrivi_*` |
| **D6** | **A** — assets in the world package **+ heartbeat during copy** | ⚠️ `kStaleSeconds` **not** to be raised; refusal mirrors `worldUnavailable:<status>` |
| **D7** | **D** — `assetPath` in `list_assets` + app-side thumbnails | ⚠️ **No image codecs in C++**; ✅ no schema change (`thumbnailAssetID` exists) |
| **D8** | **A** — thumbnail only when an image exists | ⚠️ Async, never blocks the card; pending rows unchanged |
| **D9** | **A** — `lastKnownPackagePath`, distinctly named | ⚠️ **`packagePath` keeps meaning "verified"** and is not widened |

⚠️ **Two findings surfaced while ruling these that are defects in their own right, not trade options:**

1. **`ObjectCard.swift:46` restates the world-scope rule in Swift** — CLAUDE.md's *"defect on sight."*
   ⚠️ **Structural, not careless:** no ABI endpoint exposes kind scope, so Swift has nothing to derive from.
2. **`scrivi.h:97-99` is stale** — it still lists world-scoped kinds as \*"artifact / chronicle / faction /
   rule."\* **SP-103 added six more on 2026-08-14.** ⚠️ **Occurrence eight** of the restated-kind-list class.

**Both should be filed as Issues** rather than fixed silently inside a UI sprint. ⚠️ \*\*They are the same
root cause D5 must fix\*\*, which is the argument for ruling D5 early.

## ✅ Q-a, Q-b, Q-c — RULED 2026-08-20

### ✅ Q-a — **YES. EP-034 precedes EP-032.**

EP-032's AC5 renders footnotes for `source` objects, and **nothing in the app can create a source until
this Epic's R6 ships.** Running EP-032 first would make its AC5 verifiable only on hand-authored
fixtures — ⚠️ **the exact `capability_without_surface` failure both Epics are written to avoid.**

**Consequence for numbering:** EP-032 keeps its reserved **SP-107–SP-114** (⚠️ **not released**), and
**EP-034 runs first, starting at SP-115.** ⚠️ **Sprint IDs will therefore run out of order** — SP-115+
executes before SP-107. That is deliberate and precedented (SP-083 ran before SP-082; SP-102 before
SP-100), and it must be recorded in `Sprint-Documentation.md`'s out-of-sequence note.

### ✅ Q-b — **PATCH THE CONTROL. ⚠️ The Detail Sheet does NOT replace the inline editor.**

> **User ruling, verbatim:** *"The Detail Sheet does not replace the inline editor or the list item. They
> remain in place. The Detail Sheet is displayed upon request from the writer using the list item or
> inline editor as an object reference."*

⚠️ **This corrects an assumption in §7 of this document's first draft**, which suggested I-0139 might be
*"largely dissolved"* by the Detail Sheet. **It is not.** The inline editor stays, so its defect stays, and
**I-0139 is a real fix in SP-115 — not a disposition to be argued away.**

**Three consequences that shape the whole surface:**

1. **Three editing paths coexist**, and each must remain coherent on its own:
   | Path | Purpose | Status |
   | ---- | ------- | ------ |
   | **List item** (card row) | Identify + surface the object | Stays |
   | **Inline editor** (`ObjectDraftEditor`) | Fast capture/rename in the writing loop (§1 *Capture*) | Stays — ⚠️ **and gets I-0139's fix** |
   | **Detail Sheet** | Cultivate (§1) | **New**, opened **on request** |
2. ⚠️ **The Detail Sheet is REQUESTED, never automatic.** It opens from an explicit affordance —
   double-click or right-click → "View Detail" — and **a single click on a title must keep doing what it
   does today.** This is Doc 2 §2's inherited rule: *nothing is presented, proposed, or auto-populated.*
3. **The list item and inline editor act as the object REFERENCE** the sheet is opened *from*, which is
   why R7's entry points live on them.

⚠️ **The I-0139 fix must not "solve" the problem by removing the inline editor.** Its remedy stays what
the Issue says: `pendingHelp`-style explanation and a clearer exit, ⚠️ **without a modal (§4.6) and without
a Cancel that bypasses the unfinished-work prompt** — the route by which **I-0119** filed an object into
the wrong scene.

### ✅ Q-c — **EXPLICITLY DEFERRED. iOS / visionOS are out of scope.**

> **User ruling, verbatim:** *"Both platforms will require significant work once we get done with the
> primary GUI elements. That is future work, not even defined yet."*

⚠️ **Deferred explicitly, not left unstated** — the distinction that keeps this from becoming a silent gap.
**D1's E ruling is macOS-shaped and that is accepted knowingly.**

**Recorded for whoever picks that work up:** D1-E was chosen partly *because* a self-contained navigable
component ports to iPad/visionOS as a navigation destination more naturally than an auxiliary window would.
⚠️ **That is a convenience, not a commitment** — no AC in this Epic asserts anything about either platform.

**Scope note:** ⚠️ **`[Linux]` parity remains IN scope** (AC11). Only iOS/iPadOS/visionOS are deferred.

---

*Document ends. ⚠️ **No implementation begins until D1–D9 and Q-a–Q-c are ruled.***
