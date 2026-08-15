## SP-099: [Apple] Worldbuilding Object Cards — Engine Wrappers, Object Cards, Picker

**Status:** ✅ **CLOSED 2026-08-15 (user-approved).** All tasks and acceptance criteria met. 5/5 tasks verified; AC16, AC17,
AC21, AC22, one-card-implementation, AC6, AC18, AC19, AC20 all met; **AC10 struck as OBE (user-ruled)**.
`ctest` **516/516**, macOS interop **86/86**. **Ready to close — awaiting direct user approval.**

> ⚠️ **AC20 failed on first exercise and cost three defects** ([[I-0119]], now ✅ Verified) — every one
> found by the user *using the feature*, none by the test suite. The sprint is closable because they were
> fixed and re-verified, not because they were never there.

> ⚠️ **The AC20 check is exactly why "tasks verified" ≠ "sprint verifiable."** T-0388 was marked verified
> because in-place create/edit works — and it does. The bug lives in the *boundary* case AC20 was written
> to cover, which no task-level check touched.

> **AC audit (2026-08-14).** Verified tasks are not the same as verified acceptance criteria, so each was
> checked rather than assumed:
>
> | AC | State | Evidence |
> | -- | ----- | -------- |
> | AC16 | ✅ | card lists related objects; **survives quit/reload** (live) |
> | AC17 | ✅ | picker lists and relates (live); unfiltered `listObjects` (interop) |
> | AC21 | ✅ | *"a duplicate edge is rejected from EITHER creation order (AC21)"* (interop) |
> | AC22 | ✅ | *"deleting an edge leaves BOTH objects alive and findable (AC22)"* + orphan listing (interop) |
> | One card impl. | ✅ | `ObjectCardKind.all` is 10 configurations of one `ObjectCardBody`; `isWorldScoped` is derived |
> | AC6 no-regression | ✅ | ctest 516/516 + interop 85/85; the user's existing project opens and creates |
> | **AC18** | ✅ | in-place create/edit exercised live (no modal); the draft was **left under construction across a scene change** and survived — which is the "may be left under construction" clause, demonstrated by the I-0119 flow itself |
> | **AC19** | ✅ | user-confirmed 2026-08-15: *"the edit state reads as distinct."* Four redundant signals, none of them colour: labelled header "New"/"Editing" (`ObjectCard.swift:600`), icon (`:598`), dashed border (`:655`), and the I-0119 "for: Scene N" badge |
> | **AC20** | ✅ **(after [[I-0119]])** | ⚠️ **Failed on first exercise 2026-08-14** — three defects, all found by use: the commit went to the wrong scene, the alert did not own the commit, and the reopened draft did not name its destination. **All fixed and user-verified 2026-08-15**, with all three prompt options exercised. The "never fires while working WITHIN a scene" half is **structural**, not merely untested: `showUnfinishedPrompt` has exactly one trigger, `.onChange(of: context.sceneID)` (`ObjectCard.swift:461`), so it cannot fire without a scene change. |
> | **AC10** | ⛔️ **STRUCK 2026-08-15 (user: "clearly OBE")** | overtaken by T-0409 — see below |
>
> ⛔️ **AC10 STRUCK 2026-08-15 (user-ruled: "clearly OBE").** It required a writer to "tell at a glance
> that project-scoped kinds belong to no world." **T-0409 removed the distinction it was testing for** —
> `source` is the only project-scoped kind and it has no per-kind card, so no card in the product is
> project-scoped. The criterion could not be satisfied or failed; it had no referent left.
>
> ⚠️ **Struck, not silently ticked.** Its *intent* — world context is legible — is independently met and
> recorded elsewhere: the Worlds menu lists, creates and binds (T-0408, verified), and I-0117 added
> remove and relink. **AC10 was written 2026-08-13 under R4 and invalidated by a ruling made the next
> day**; that is worth noting for sprints that span a design change, since an AC can go stale exactly
> like a restated kind list.

> ✅ **T-0387 VERIFIED 2026-08-14 by live check** — the picker lists existing objects, selection relates
> the chosen one, **and the relationship SURVIVES A QUIT/RELOAD**. That last part exceeds AC17, which asks
> only that the picker list and offer: it demonstrates the edge reached
> `objects/relationships.jsonl` and was read back on open, so **AC16's "lists exactly what the writer
> related" is proven across a process boundary**, not just within one session.
>
> Its data half was independently covered through the C ABI (interop, *"Unfiltered lists every kind —
> what the picker shows (AC17)"*). "Create new…" remains **disabled** by design (R4) — in-place creation
> is T-0388's entry point, which is verified.
>
> ✅ **AC6 MET (2026-08-14).** `ctest` **516/516** and the macOS interop suite **85/85**.
>
> ⚠️ **The interop suite was not merely blocked — it was 15 tests STALE, and running it found them.**
> The earlier `Could not launch "ScriviInteropTests"` (LaunchServices error 20) was caused by an instance
> of Scrivi running from Xcode holding the app bundle; the user closed it and the suite ran. It then
> failed **15 tests**, every one a `worldRequired` refusal or an assertion encoding the **pre-T-0409
> partition** — the Swift twin of the C++ realignment SP-103/SP-104 had already done. Notably:
>
> - `the world-scoped kinds are exactly the four that live in a .scrivworld` — asserted the OLD
>   four-kind set. **Rewritten** to assert the derived whole set (all ten cards; `source` absent).
> - `project-scoped kinds never ask for a world` — asserted `character.isWorldScoped == false` and called
>   it *"correct, not an omission."* **That is precisely what the shipped code believed**, and it is why
>   creating a character was refused (I-0114). Inverted.
> - `listObjects … reports project scope` — asserted a character carries NO worldID. Inverted.
> - Both interop fixtures now seed a world (T-0409); `a project with no worlds lists none` was rebuilt
>   **without** the shared fixture, since it needs a project that genuinely has none.
>
> **This is the AC6 gap justifying itself.** The suite is the only thing that exercises the Swift side of
> the boundary, and while it could not run, five sprint tasks were marked Implemented against assertions
> that contradicted the shipped ruling.
>
> ⚠️ **Two of the verified tasks changed AFTER they were marked Implemented**, both in SP-104 and both
> covered by the user's live testing: `ObjectCardKind.isWorldScoped` was rewritten as derived (I-0114 —
> it was the blocker that made T-0386/T-0388 undemonstrable), and `WorldsView` gained remove/relink
> (I-0117).
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph (fifth of 7 sprints — see R2)
**Codebase:** `[Apple]` — Swift/SwiftUI + `ScriviEngine`. **No ScriviCore work; `scrivi.h` is not touched.**
**Goal:** Put the relationship graph in front of the writer. Wrap the 12 unwrapped graph/world endpoints in
`ScriviEngine`, then build the **one object card implementation** (per-kind configured) and the **unfiltered
object picker** on EP-030's card framework.
**Design:** `docs/Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §3.1, §5, §6, §9 AC16/17/21/22 +
`docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` §5.2–5.5 (both ✅ Approved).
**Start Date:** TBD | **End Date:** TBD | **Capacity:** ~10–12 hours

> **EP-031's first `[Apple]` sprint.** The entire `[ScriviCore]` half closed 2026-08-12 (SP-095–SP-098) and
> EP-030's card framework closed 2026-08-11. Nothing blocks this sprint.

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0407 | ⚠️ **`ScriviEngine` graph + world wrappers** — the 12 unwrapped endpoints (**do first**) | High | ✅ **Implemented - VERIFIED (2026-08-14, user-approved)** |
| T-0386 | Object cards — **one** implementation, per-kind config, on EP-030's framework | High | ✅ **Implemented - VERIFIED (2026-08-14, user-approved)** |
| T-0387 | Object picker (unfiltered, all worlds) + inline type-ahead + "Create new…" entry point | High | ✅ **Implemented - VERIFIED (2026-08-14, user-approved)** |
| T-0388 | **In-stack create/edit, edit-state visuals, scene-change complete-or-discard** (⬅ pulled from SP-102) | High | ✅ **Implemented - VERIFIED (2026-08-14, user-approved)** |
| T-0408 | **Worlds menu — list / create / bind + world context** (new, R4) | High | ✅ **Implemented - VERIFIED (2026-08-14, user-approved)** |

> ⚠️ **SCOPE CORRECTED 2026-08-13 (R4, user-ruled) after live verification.** The user exercised the shipped
> card and found two things the split got wrong. **Neither is a code defect — both are scope errors in my
> SP-099/SP-102 split**, which left SP-099 as a read-only surface over data that could not yet exist.
>
> 1. ⚠️ **The picker can only ever be empty.** "Create new…" shipped **disabled** (correctly — §4.6 forbids a
>    modal, and in-place creation was T-0388 in SP-102), but **`createObject` has zero UI call sites anywhere
>    in the app**. There is no other way to create a character, so the card lists nothing *permanently*, and
>    **AC16/AC17/AC21 are not demonstrable**. **T-0388 is pulled into SP-099.**
> 2. ⚠️ **"I have no idea what world I'm working with."** `listWorlds` likewise has **zero UI call sites**. A
>    writer cannot see, create, or bind a world, and cannot tell that `character` is project-scoped and
>    belongs to no world at all. Doc 2 §7.3 scopes the Worlds menu as the *warning surface* for unavailable
>    worlds — **nobody scoped how a writer gets a world in the first place.** New **T-0408** adds the Worlds
>    menu (list / create / bind) and makes scope legible in the picker.
>
> **SP-102 keeps:** pending presentation (AC23/AC24), the warning view under the Timeline, "Remove All World
> References", and T-0365's aggregate `sources` card.

### Assigned Issues

None at planning.

> ⚠️ **T-0407 IMPLEMENTED 2026-08-13 — 14 wrappers, not 12.** `createWorld` and `addWorld` were missing from
> the planned list (the plan counted the 12 endpoints the *cards* consume; a test cannot construct a world
> without these two). **16 new interop tests, all through `scrivi_*`** — including a world round-trip that
> creates an `artifact` through the C ABI, which is I-0113's exact shape and the assertion that would catch a
> wrapper dropping `worldID`. macOS interop **72 passed / 0 failed** (from 56). App **BUILD SUCCEEDED**.
> No ScriviCore change; `scrivi.h` untouched.
>
> ⚠️ **F4 — a NULL `result` is how the C ABI reports an EMPTY collection, and `decodeC` treated it as a
> failure.** Every list endpoint returns `{"ok":true,"result":null}` when it has nothing to list — an empty
> `JsonDoc` dumps as `null`, not `{}`. **Verified by probe** against `libScriviCore.a`, not inferred:
>
> ```
> list_worlds:  {"ok": true, "result": null}
> list_objects: {"ok": true, "result": null}
> ```
>
> `decodeC` threw `"ok=true but result missing"` on all of it, so **a project with no worlds, or a fresh scene
> with no relationships, read as a backend error** — the single most common state in a new project. Three
> tests caught it. Fixed in `decodeC`: a null result decodes as `{}` first, so a result type whose fields are
> all defaulted materializes empty, while a type with required fields still throws and a genuinely missing
> result is still caught. **This was latent for every existing list wrapper too, not just the new ones.**
>
> ⚠️ **Found at implementation (2026-08-13), inside T-0407: `ScriviError` discards `detail`.** The C ABI
> emits `detail` and `path` on every error envelope (`scrivi_c_api.cpp:164–165`), but `ErrorPayload`
> (`ScriviError.swift`) decodes **only `code` and `message`** — so `detail == "worldPending:<status>"`, set at
> `RelationshipStore.cpp:191,322` when the graph refuses a write toward an unavailable world, **never reaches
> Swift.** The sprint plan requires this to survive; SP-102's frozen-graph refusal (AC23) is unbuildable
> without it. Fixed as a prerequisite within T-0407 — `detail` and `path` added to `ErrorPayload` and
> `ScriviError` as optionals, so every existing call site and construction is source-compatible.

---

### Acceptance Criteria (sprint-level)

Sprint AC numbering follows Doc 2 §9, so it maps to the Epic's AC9 directly.

1. **AC16** — An object card lists exactly the objects the writer related to that scene, **never inferred**.
   Nothing is derived from scene text.
2. **AC17** — The picker lists objects from **all worlds in the project**, unfiltered by the scene's world
   association (Doc 3 §4.1), and offers **"Create new…"**.
3. **AC21** — Adding an object from **either** entrance (picker or type-ahead) creates **one canonical edge**;
   adding the same relationship from the other endpoint is rejected as a duplicate.
4. **AC22** — **"Remove from scene" deletes the edge only.** The object survives as an orphan and remains
   findable via `scrivi_list_orphaned_objects`. The word "Delete" appears nowhere in this affordance.
5. **One card implementation.** Adding an eleventh kind is a configuration entry, not a new card type —
   verified by the diff, not by assertion.
6. **No regression:** `ctest` and the macOS interop suite stay green; existing projects open unchanged.

**Added by ruling R4 (2026-08-13):**

7. **AC18** — Neither creating nor editing an object opens a modal; both happen in place in the card, and a
   card may be left under construction while the writer writes (§4.6).
8. **AC19** — Cards in edit state are visually distinct **without relying on color alone** (§4.6.2).
9. **AC20** — Leaving a scene with unfinished edits surfaces complete-or-discard, **naming the consequence
   concretely** and distinguishing new-vs-edit (§4.6.1); it never fires while working within a scene.
10. ~~**World context is legible** — a writer can see which worlds a project uses, create one, bind an
    existing one, and tell at a glance that project-scoped kinds belong to no world (R4).~~
    ⛔️ **STRUCK 2026-08-15 (user-ruled, OBE).** T-0409 made every carded kind world-scoped, so the
    "project-scoped kinds belong to no world" distinction has no referent. The rest of the clause is met
    by T-0408 (verified) + I-0117.

⏭ **Still deferred to SP-102:** AC23/24 (pending presentation + status refinement), the warning view under the
Timeline, "Remove All World References", and T-0365's aggregate `sources` card.

---

### Planning findings (2026-08-13)

The staged plan was audited against the shipped code before scoping. **Three findings; two change the sprint.**

#### F1 — ⚠️ **None of the 12 graph/world endpoints is wrapped in Swift** (→ T-0407, real work)

Every endpoint SP-099 consumes exists and is exported in `scrivi.h`, and **not one has a `ScriviEngine`
wrapper.** All twelve grep to zero call sites in `Scrivi/Engine/ScriviEngine.swift`:

```
create_edge · delete_edge · list_edges_for · list_pending_edges
list_objects · list_orphaned_objects · promote_object
list_worlds · get_world_status · relink_world · remove_world_reference · list_relation_types
```

`ScriviEngine` today has **object CRUD only** — `createObject` / `openObject` / `saveObject` / `deleteObject`
(`ScriviEngine.swift:237–318`), which gained `worldID` for free in SP-098 T-0405. The graph itself has never
been reachable from Swift.

> ⚠️ **This is the same lesson SP-098 wrote down, pointing the other way.** SP-098 found `ScriviEngine.swift`
> wrapped *more* than R1 assumed; here it wraps *less* than the staged plan assumed. In both cases the sweep —
> not the hypothesis — was what produced the truth. **T-0386–T-0389 as staged silently contained an entire
> engine layer.** It is now its own task, done first.

**T-0407 tests through `scrivi_*` in the interop target**, per the standing habit from the I-0113 audit: a
test that exercises only the Swift side cannot see a boundary gap, which is exactly how I-0113 shipped green.

#### F2 — ⚠️ **`offline` and `unmounted` are declared but never produced** (→ SP-102, AC24)

`WorldStatus` declares five states (`WorldTypes.hpp:68`), but `WorldStatus::unmounted` and
`WorldStatus::offline` **appear at no construction site anywhere in `ScriviCore/src`** — the core only ever
emits `missing` or `unavailable`. The header comments them as "platform-layer refinement," and Doc 3 §4.4.1
forbids a platform-specific *model*, so the refinement is Apple-layer work feeding the neutral enum.

**AC24 therefore has unbuilt work under it that no staged task named.** It travels with the pending
presentation into **SP-102**, where it is explicitly scoped rather than discovered mid-sprint. This is not a
defect — the core's fallback to `unavailable` is the honest behavior the design mandates — but AC24 cannot be
ticked on the core's output alone.

#### F3 — ✅ **Pending data is already fed to the card; no core work is needed for it**

`scrivi_list_edges_for` already returns `otherPending`, `otherDisplayName`, and `otherWorldStatus` per row
(`scrivi_c_api.cpp:891`). Doc 2 §7.2's named-not-bare-ID requirement is **satisfiable from the existing
payload**, and SP-102's pending card work is pure presentation. Worth recording so SP-102 is not planned as
though it needed core changes.

---

### Rulings taken at planning (2026-08-13, user-approved)

**R1 — The engine layer is its own task, first.** T-0407 wraps all 12 endpoints before any SwiftUI depends on
them, with interop tests through `scrivi_*`. Folding the wrappers into T-0386 would have hidden a real layer
of work in tracking and tested it only through the UI. Same reasoning that put T-0403 first in SP-097.

**R2 — SP-099 is split; SP-102 is new.** The staged sprint carried 5 tasks plus an unplanned engine layer,
mixing plumbing, CRUD UI, and failure-surface work in one verification pass. Split:

- **SP-099** (this sprint) — engine wrappers + object cards + picker → **AC16 / AC17 / AC21 / AC22**
- **SP-102** (new) — in-stack create/edit + edit-state visuals + complete-or-discard + pending presentation +
  Worlds menu + warning view + the aggregate `sources` card → **AC18 / AC19 / AC20 / AC23 / AC24**

**Sprint IDs stay in sequence and SP-100 keeps its number** as the Epic verification/close sprint; it simply
runs after SP-102. **EP-031 becomes a 7-sprint Epic.** (SP-101 is already taken — it was the unplanned EP-030
AC12 sprint, closed 2026-08-11 — so the new sprint takes **SP-102**.)

**R3 — Pending verification is ruled now, verified live in SP-102.** AC23 requires that reattaching a world
restores the card **with no writer intervention**, which a fixture cannot demonstrate. The mechanism is ruled
at this planning so SP-102 does not stall on it:

> Create a real `.scrivworld` package on a mounted **disk image**, bind it, relate objects from it to a scene,
> then **eject the image** to produce a genuinely unavailable world. Verify pending presentation live, remount,
> and verify restoration with no writer action. Moving the package aside in the filesystem is the fallback for
> the `missing` branch specifically — the two branches report different statuses (F2) and both need coverage.

---

### Task detail

**T-0407 — ⚠️ `ScriviEngine` graph + world wrappers (do first).** Wrap all 12 endpoints listed in F1,
following the established `withCString` + `decodeC` pattern at `ScriviEngine.swift:237–318`, with `Decodable`
result types mirroring each envelope. `list_edges_for` rows must carry `otherPending` / `otherDisplayName` /
`otherWorldStatus` through to Swift **even though nothing consumes them until SP-102** — dropping them here
would force a second pass over the same decode. Errors decode to `ScriviError` as elsewhere; in particular
`detail == "worldPending:<status>"` must survive as a distinguishable case, since SP-102's frozen-graph
refusal depends on reading it. Interop tests go through `scrivi_*`.
**Files:** `Scrivi/Engine/ScriviEngine.swift`, `Scrivi/Tests/ScriviInteropTests.swift`.

**T-0386 — Object cards, one implementation.** A single card type parameterized by kind — Doc 2 §3.1's *"one
card type, ten configurations."* Reads via `list_edges_for(sceneID)`, keeps endpoints of the configured kind,
renders `displayName` / `subtitle` / thumbnail, ordered by the **stack's** sort (C6 — sort is per-stack, never
per-card). Registers in `InspectorCardRegistry` under a stable `typeID` per kind. "Remove from scene" calls
`delete_edge` and never `deleteObject` (**AC22**). The Worldbuilding stack **still ships empty** — Doc 2 AC7
forbids any card appearing without explicit writer action, and that must not regress.
**Files:** new `Scrivi/Views/Inspector/ObjectCard.swift` (+ pbxproj in the same step), `InspectorCard.swift`.

**T-0387 — Object picker + type-ahead.** Picker sheet listing **all objects across all worlds** via
`list_objects`, grouped by world, with writer-applied kind filters (**AC17** — deliberately unfiltered by the
scene's world). Inline type-ahead in the card for the fast path. Both entrances call `create_edge`, which
normalizes direction and rejects duplicates — **the card never writes a reverse edge** (Doc 2 §6; the
Cumberland trap Doc 1 §5.2 exists to prevent). "Create new…" is present as an entry point but its in-stack
creation flow is **SP-102 (T-0388)**; in this sprint it may be surfaced disabled or deferred rather than
opening a modal — **§4.6's no-modal-ever rule is absolute and applies now.**
**Files:** new `Scrivi/Views/Inspector/ObjectPickerView.swift` (+ pbxproj in the same step).

---

### Implementation record (2026-08-13)

**As of 2026-08-13** — all three tasks then in scope 🟠 Implemented, Not Verified. ctest **510/510**
(baseline, unchanged — no ScriviCore work, `scrivi.h` untouched); macOS interop **78 passed / 0 failed**, up
from 56; app **BUILD SUCCEEDED** and launches clean with the ten cards registered.

> ⚠️ **Superseded — this is a historical snapshot, not current state.** The sprint grew to **five** tasks
> (T-0388 and T-0408 pulled in by R4 the same day), and all five are now ✅ Verified. Current figures:
> ctest **516/516**, macOS interop **86/86**. See the status block at the top of this sprint.

**New files** (all three registered in `project.pbxproj` in the same step): `Scrivi/Engine/ScriviEngineGraph.swift`,
`Scrivi/Views/Inspector/ObjectCard.swift`, `Scrivi/Views/Inspector/ObjectPickerView.swift`.

**T-0386 — one implementation, ten configurations.** `ObjectCardKind.all` is the configuration table; the body
is a single `ObjectCardBody`. Because `InspectorCard` needs *static* identity per card, each kind gets a
phantom-typed shell (`ObjectCardType<Provider>`) carrying no behavior — adding an eleventh kind is one row in
`all` plus one `register` line, which is what "not ten card types" has to mean in practice. `CardContext`
gained the stack's `sort` (C6 — cards must not grow their own sort control).

**T-0387 — picker.** Lists **all objects across all worlds** unfiltered (AC17), grouped by world for display
only, with the relation type resolved against the project's **live** vocabulary rather than the seed.
"Create new…" is present as an entry point but **disabled**: its in-place edit state is T-0388 (SP-102), and
§4.6 forbids satisfying it with a modal in the meantime.

**Three findings beyond F1/F2, all fixed in-sprint:**

1. ⚠️ **F4 — a NULL `result` is how the C ABI reports an EMPTY collection** (see above). Latent for every
   pre-existing list wrapper, not just the new ones.
2. ⚠️ **F5 — `ScriviError` discarded `detail`** (see above). Prerequisite for SP-102's AC23.
3. ⚠️ **F6 — a `project.pbxproj` ID collision silently dropped a file from the build.** `C069`/`C070` were
   already taken by `ScenePropertiesView.swift`; reusing them produced **duplicate `PBXBuildFile`
   identifiers**, and Xcode resolved each to the pre-existing entry. `ObjectCard.swift` was therefore **never
   passed to the compiler** — no error, no warning, just an "unknown member" failure at the *call site* in a
   different file. Diagnosed by grepping the compile line for the filename (**zero occurrences**) rather than
   trusting the entry's presence. Re-registered at `C071`/`C072`; a duplicate-identifier check now passes.
   > **The lesson generalizes past this sprint.** The standing pbxproj rule says *add the file*. It does not
   > say **verify the ID is unused** — and a collision fails in the one way that looks like a source error in
   > an unrelated file. `grep -oE '^\t\t[A-Z][0-9]{3}_BF' project.pbxproj | sort | uniq -d` is the check.

**Not done here, deliberately:** the ⚠ pending badge and disabled-affordance code paths in `ObjectCard.swift`
are written and carry `otherPending` end to end, but **AC23/AC24 are SP-102's** and are not claimed as met —
they need the live ejected-volume verification ruled in R3.

#### R4 addendum — T-0388 + T-0408 (2026-08-13)

Added after the user's live check found the sprint shipped a surface with **no way to put data into it**.
Suites after: ctest **510/510** (unchanged), interop **83 passed / 0 failed** (from 78), **BUILD SUCCEEDED**.

**T-0388 — in-place create/edit, no modal.** "Create new…" and a "New <Kind>" button open an **edit state
inside the card** (`ObjectDraftEditor`), pre-filled with whatever was typed in the picker. §4.6.2's
visual-distinctness requirement is met with **three redundant signals** — a "New"/"Editing" header, an icon,
and a dashed border — so it never rests on color alone. Committing creates the object **and** the edge in one
step; leaving a scene with an unfinished draft raises the complete-or-discard prompt, and per **§4.6.1** the
buttons name the consequence concretely (*"Discard New Character"* vs *"Revert Changes"*) rather than sharing
one generic "Discard". Editing an existing object patches `displayName` **into its own JSON** rather than
reconstructing the object, so fields this build does not know about survive the round trip.

**T-0408 — Worlds menu.** A `Worlds ▸ Manage Worlds…` menu with list / create / bind, plus a scope line in
the picker that states plainly where a kind lives (*"Characters belong to this project, not to a world"*).
This closes the second half of the user's report: world context was invisible because **`listWorlds` had no
UI call site at all**. `createWorld` uses a save panel so the writer chooses where the package lives — a world
is a real, movable, shareable package (Doc 3 §4.1), not hidden project state. "Remove All World References"
stays out (§7.3, SP-102): it is destructive to pending links and must be sought out.

⚠️ **A test-environment finding, not a code defect.** The suite began failing with **LaunchServices -1712**
(launch timeout) on the test runner. Cause: a Scrivi instance from a *different DerivedData tree* — the user's
Xcode session — held the app registration. **Confirmed environmental by stashing every change and reproducing
the identical failure on baseline code**, then cleared when the user quit that instance. Worth remembering:
`-1712` on this project means *an app instance is already running*, not that the tests are broken.

---

### Definition of Done

- [x] T-0407, T-0386, T-0387 implemented; all new Swift files added to `Scrivi.xcodeproj/project.pbxproj`
      **in the same step** (CLAUDE.md, non-negotiable). ⚠️ See **F6** — presence in the file is not
      sufficient; the identifier must also be unique, which is how `ObjectCard.swift` initially failed.
- [x] Sprint AC1–AC6 above demonstrated; AC21's duplicate rejection tested from **both** creation orders.
- [x] `ctest` **510/510** — exactly the baseline, no ScriviCore change.
- [x] macOS interop green and grown: **83 passed / 0 failed** (from 56 — +16 T-0407 wrappers, +6 card config,
      +5 creation/worlds after R4).
- [x] App **BUILD SUCCEEDED** and launches clean with ten cards registered.
- [x] **R4:** T-0388 + T-0408 implemented — a writer can now create a character in place and see/create worlds.
- [x] Tasks marked 🟠 **Implemented - Not Verified**; **Verified only on direct user approval.**
- [ ] ⏳ **User verification of the sprint ACs in the running app** — the object card and picker have not been
      exercised live by a writer, only by tests and a launch check.

---

### Risks

- **T-0407 is wider than it looks** — 12 endpoints with distinct envelope shapes. It is the whole reason this
  sprint was split; if it overruns, T-0387 returns to the backlog rather than compressing T-0386's verification.
- **The Worldbuilding stack shipping empty (Doc 2 AC7)** is easy to regress while building cards that want to
  be seen during development. Any dev-time default must not reach the committed default layout.
- **Sort belongs to the stack** (C6). An object card that grows its own sort control contradicts a ruled trade
  study and would have to be unbuilt.

---

*Last Updated: 2026-08-13 (SP-099 planned. ⚠️ **Two findings changed the scope:** none of the 12 graph/world
endpoints is wrapped in Swift — an entire engine layer was hidden inside the staged tasks, now **T-0407**,
done first; and **`offline`/`unmounted` are declared but never produced** by ScriviCore, so **AC24 has unbuilt
Apple-layer work** under it, scoped into SP-102 rather than discovered mid-sprint. Three rulings: **R1**
engine wrappers as their own first task; **R2** the sprint is **split — SP-102 is new**, EP-031 becomes a
7-sprint Epic with SP-100 keeping its number as the close sprint; **R3** pending verification is by real
`.scrivworld` on an **ejectable disk image**, ruled now so SP-102 does not stall. Also recorded: pending
presentation needs **no** core work — `list_edges_for` already returns `otherPending`/`otherDisplayName`/
`otherWorldStatus`.)*

---

## Retrospective (closed 2026-08-15, user-approved)

**Delivered:** the relationship graph in front of the writer — 14 `ScriviEngine` wrappers, one object-card
implementation in ten configurations, the unfiltered picker, in-place create/edit, and the Worlds menu.

### What this sprint actually cost, and why

SP-099 was planned as a read-only surface and **could not be verified as planned**. Live use found that
`createObject` and `listWorlds` had **zero UI call sites** (R4), so the card could only ever be empty; then
T-0409 changed object scope underneath the sprint mid-flight. Closing it required **two unplanned sprints**
(SP-104, SP-105) and **six Issues** (I-0114–I-0119).

### Three lessons worth carrying

1. ⚠️ **Every defect that mattered was found by USE, not by tests.** I-0114 (creation blocked outright),
   I-0119's three defects (wrong-scene commit, prompt not owning the commit, unnamed destination) — all
   surfaced by the user exercising the feature. The suites were green throughout. The I-0119 regression
   test pins the data invariant and was verified to catch the original bug by deliberate inversion, but it
   **could never have caught the third defect**, where the write was always correct and only the
   explanation was missing.
2. ⚠️ **A restated list rots without being edited.** I-0114 was the 4th and 5th occurrence in this Epic;
   two more (I-0118's caller list, test fixtures) followed. The standing rule is now *derive, never
   restate* — in any language — recorded in `CLAUDE.md` and Doc 1.
3. ⚠️ **An acceptance criterion can go stale exactly like a kind list.** AC10 was written 2026-08-13 under
   R4 and invalidated by T-0409 the next day; it was **struck, not ticked**. A sprint spanning a design
   change must re-read its ACs against the new ruling, not just its code.

### Verification at close

- 5/5 tasks ✅ Verified · AC16/17/21/22 + one-card-impl + AC6 + AC18/19/20 ✅ · AC10 ⛔️ struck (OBE)
- `ctest` **516/516** · macOS interop **86/86** · app **BUILD SUCCEEDED**
- Issues closed here: [[I-0119]] ✅ Verified. Related: I-0114–I-0117 (SP-104), I-0118 (SP-105).

⚠️ **The macOS interop suite was unrunnable for part of this sprint** (a running Scrivi instance holds the
bundle — LaunchServices error 20). While it could not run, **15 of its tests were silently stale** against
the T-0409 ruling, including two asserting the *old* scope partition. AC6 exists precisely to catch that.
