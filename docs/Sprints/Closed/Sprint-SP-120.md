# Active Sprint

## SP-120 — `[Apple]` Source creation + footnote text (R6) — ✅ **CLOSED; T-0365's write half PAID**

**Status:** ✅ **CLOSED 2026-08-24** (user-approved)
**Epic:** [EP-034](../Epics/Epic-active.md) — `[Cross]` Object Detail & Media · **sprint 6 of 8**
**Codebase:** `[Apple]` — ⚠️ **no ScriviCore change is expected** (see §2)
**Date Activated:** 2026-08-24
**Closes:** **AC8**
**Design:** [`../Scrivi_Object_Detail_Sheets_Design_v0_1.md`](../Scrivi_Object_Detail_Sheets_Design_v0_1.md) §4 R6
**Tasks:** **T-0453 – T-0458** (six) — ✅ **all Verified** · **Next available:** T-0459 · **I-0171**

---

## 1. Sprint Goal

**A writer can create a source from the object it documents, give it citation detail, and see it — from
both entry points §3.1.1 requires.** When this sprint closes, the aggregate `sources` card renders
**content for the first time since it shipped in SP-102**, and T-0365 stops being a partial delivery.

R6, in the user's own framing: a citation documents an **object**. Creation therefore lives on the
object's detail surface and **never** on the scene-scoped `sources` card — *"it would muddy the fact that
the source must be associated with an object in the world."*

---

## 2. ⚠️ What the code already provides — verified 2026-08-24, not assumed

**Every backend piece of AC8 already exists and is already wrapped in Swift.** This was checked against
the code before planning, because ⚠️ **EP-034's opening finding was exactly this mistake in reverse** —
a design draft claimed world-asset storage was "the single largest piece of backend work" and was wrong.

| Piece | Where | State |
| ----- | ----- | ----- |
| `source` object kind, project-scoped at `objects/sources/` | `ObjectTypes.hpp:45-46` | ✅ Ships (SP-098 / T-0406) |
| `cites` relation type, ⚠️ **unconstrained on BOTH ends** | seeded; `RelationshipTests.cpp:188` | ✅ Ships (SP-096 / T-0373) |
| `scrivi_create_object` | `scrivi_c_api.cpp` | ✅ Ships; wrapped `ScriviEngine.swift:237` |
| `scrivi_create_edge` | `scrivi_c_api.cpp` | ✅ Ships; wrapped `ScriviEngineGraph.swift` |
| `attributes` freeform map, round-tripped | `ObjectTypes.hpp:224`, `ObjectJson.cpp:46-50,104-107` | ✅ Ships — ⚠️ **read by NOTHING** |
| `CitationPopover` | `SourcesCard.swift:235` | ✅ Built — ⚠️ **ONE call site, not two** |

> ⚠️ **This sprint ships no new capability. It ships the surface for capability that has been sitting
> unreached — in one case since SP-095.** That is `project_capability_without_surface` for the third time
> inside EP-034 (after `subtitle`/`notes`/`image`, and `listPendingEdges`), and it is the whole reason
> this Epic exists.

**Consequence: SP-120 is `[Apple]`-only.** ⚠️ If any Task turns out to need a ScriviCore change, that is a
**finding to surface immediately**, not a scope decision to make quietly — SP-116 had to be re-ruled
`[Cross]` at planning and the record says so.

---

## 3. ⚠️ Three rulings taken at planning (user, 2026-08-24)

### Ruling 1 — citation fields live in `attributes`

AC8 names "footnote text", but ⚠️ **a `source` object has no citation fields at all**: `ObjectRecord`
carries `displayName`, `subtitle`, `notes`, `tags` and a freeform `attributes` map, and nothing else
(`ObjectTypes.hpp:208-224`). `SourceDetail` reads exactly three of those (`SourcesCard.swift`).

✅ **Ruled: use `attributes`.** It already crosses the ABI, already round-trips, and needs **no schema
change, no ABI change, and no version decision** — which is what keeps this sprint `[Apple]`.

⚠️ **The trap, and it has already bitten this Epic once.** `attributes` serialises as an **array of
`{"k":…,"v":…}` pairs**, *not* as a JSON object (`ObjectJson.cpp:46-50`):

```json
"attributes": [ {"k": "author", "v": "Ursula K. Le Guin"} ]
```

**This is the identical shape trap T-0449 hit with `tags`**, where writing a plain string array parsed
back as an empty list — every tag silently dropped, with the file looking perfectly reasonable to a human
(`ObjectDetail.swift:130-136` carries that warning). ⚠️ **Writing `{"author": …}` here would fail exactly
the same way, and no suite would catch it.** T-0453 owns this and **its round-trip test is written first.**

### Ruling 2 — the second entry point ships here

T-0365's archive names **two** undelivered items, and ⚠️ **only one of them is in any AC**:

1. the write half → **AC8**;
2. §3.1.1's *"same interaction on worldbuilding-object cards — one popup implementation, two entry
   points"* → ⚠️ **named in NO acceptance criterion and owned by NO sprint.**

`grep CitationPopover Scrivi/` returns **one file**: the card that declares it. Zero references from
`ObjectCard.swift` or the Detail Sheet.

✅ **Ruled: SP-120 owns it** (**T-0456**). It is untestable until sources can exist — which is precisely
what this sprint delivers — and leaving a built-but-unreferenced popup behind would be the same defect
class the Epic exists to cure, committed by the sprint curing it.

### Ruling 3 — S11 enumerates OPERATIONS, not just fields

Carried forward from SP-119 as an explicit instruction. ⚠️ **S11 listed FIELDS and not OPERATIONS, which
is how I-0164 slipped through** — "attach an image already in the world" was an operation no field table
could have surfaced.

✅ **Ruled: S11 is written FIRST, and it enumerates operations.** ⚠️ **The mirror of I-0164 is named in
advance here:** *attach an **existing** source to a second object.* A writer citing one book across three
characters must not be forced to create the book three times — and that is the operation a fields-only
review would miss again.

---

## 4. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0453** | ⚠️ **`attributes` read/write in `ObjectDetail`** — the `{"k","v"}` shape, ⚠️ **round-trip test first** | **High** | ✅ **Verified** |
| **T-0454** | **AC8 (a)** — create a `source` **from the documented object** + attach the `cites` edge, atomically-reported | **High** | ✅ **Verified** |
| **T-0455** | ⚠️ **Attach an EXISTING source** to a second object — the I-0164 mirror, named at planning | **High** | ✅ **Verified** |
| **T-0456** | ⚠️ **T-0365's second entry point** — object surfaces its own sources → **the same `CitationPopover`** | **High** | ✅ **Verified** |
| **T-0457** | **Citation fields + footnote text** in the sheet, and ⚠️ **`CitationPopover` learns to show them** | **High** | ✅ **Verified** |
| **T-0458** | ⚠️ **S11 written FIRST** — operations *and* fields; then the live pass | **High** | ✅ **Verified** — S11 §5 + live pass done |

### ⚠️ Execution status — 2026-08-24

**Files added:** `Scrivi/Views/Detail/ObjectSourcesSection.swift` (T-0454/T-0455/T-0456),
`Scrivi/Views/Detail/Citation.swift` (T-0457). **Modified:** `ObjectDetail.swift` (T-0453),
`SourcesCard.swift` (T-0457), `ObjectDetailSheet.swift` (wiring), `ScriviInteropTests.swift` (5 tests),
`project.pbxproj` (⚠️ **both files registered in all three app targets, verified by compile count**).

| Evidence | Result |
| -------- | ------ |
| `ctest` (macOS arm64) | ✅ **567/567** — ⚠️ **unchanged, which is the point**: SP-120 made no core change, confirming the `[Apple]` scoping held |
| App build | ✅ **BUILD SUCCEEDED**, no warnings |
| New files reached | ✅ **Proven, not assumed** — forced recompile shows each file compiled **3×**, once per app target (`feedback_prove_code_is_reached`) |
| T-0453 logic | ✅ **8/8 standalone**, including a ⚠️ **negative control**: the object-shaped write decodes **EMPTY**, proving the trap is real and not hypothetical |
| **Interop suite** | ✅ **127/127** (12 suites) — ⚠️ **run only after the user quit Scrivi**, on request. Up from **122** at SP-119's close: the five new T-0453 tests, ⚠️ **each confirmed BY NAME in the log**, not inferred from the total. ⚠️ **The I-0150 hazard did not recur** — no real-project path appears in the run log and `tintagael`'s mtime predates it, both checked. |
| **Live click-through** | ✅ **DONE 2026-08-24, on a real sequel project** — ⚠️ **and it found the only two defects of this sprint** (I-0169, I-0170), both now ✅ Verified |

⚠️ **A trap worth recording: `xcodebuild test` printed `Executed 0 tests` alongside `TEST SUCCEEDED`.**
That is the *XCTest* tally, and these are **Swift Testing** tests, which report separately. ⚠️ **Taken at
face value it is a green result that ran nothing** — the precise shape of a suite that silently stops
covering anything. The real line is `Test run with 127 tests in 12 suites passed`. **Read that one.**

### ⚠️ Two Issues from the live pass — I-0169, I-0170 (2026-08-24)

⚠️ **Both found by the user's click-through. Neither was found by any suite** — the suites were green
before and after, at **127/127**, and they were not wrong. **They assert what the code does; they cannot
assert that a writer can reach it, or that what she reads is true.** ⚠️ **That is now 22 Issues across
SP-118, SP-119 and SP-120, and not one came from a test.**

| Issue | What it was | Fixed by |
| ----- | ----------- | -------- |
| **I-0169** | ⚠️ **The `sources` card had NO route to the Detail Sheet** — and it was the writer's FIRST instinct. Every other object row opens on double-click + right-click → "View Detail"; this one did neither. ⚠️ **`CardContext.openObjectDetail` already existed and this card never called it** — `feedback_look_for_existing_pattern_first`, the same shape as SP-118's four. | `SourcesCard.swift` |
| **I-0170** | ⚠️ **The popup named only ONE citing object when the source had two.** Real data: one source cited **Myton at 23** *and* **Tintagael**. ⚠️ **The SP-120 decision to reword the card's *"via …"* was CORRECT; the implementation of it dropped information instead of re-centring it.** The aggregate card had it right all along. | `ObjectSourcesSection.swift` |

⚠️ **I-0170 is the more interesting failure.** It is not a missing feature — it is a surface **quietly
under-reporting the writer's own graph**, which reads as correct and is not. ⚠️ **A field-level review
would not have caught it**: every field was present and populated. It took real data with a source cited
**twice, across two different kinds**.

✅ **AC8 IS CLOSED** — verified by use 2026-08-24 on `the-lone-golem.scrivi`, the sequel to
`the-stairs-of-tintagael`, against the live **Eskandar** world on USB.

⚠️ **The `sources` card rendered CONTENT for the first time since it shipped in SP-102.** Until this
sprint it could only ever draw its empty state, which is indistinguishable from working correctly.

### T-0453 — `attributes` read/write in `ObjectDetail`

**Files:** `Scrivi/Engine/ObjectDetail.swift`
**Why first:** every other Task writes through it.

- Decode `attributes` from the `[{"k","v"}]` array into `[String: String]`.
- Extend `applyingEdits(...)` with an `attributes:` parameter following **exactly** the `tags` precedent
  already in that function: **`nil` means "not edited"** so a caller that ignores attributes cannot erase
  them, empty means remove the key entirely.
- ⚠️ **Preserve unknown keys.** The patch rule is *patch, never reconstruct* — an attribute a future build
  writes must survive a save by this build.
- ⚠️ **Write the round-trip test BEFORE the encoder** — save → reopen → assert the pairs survive. The tags
  bug proves an encoder can look right and lose everything.

### T-0454 — AC8 (a): create a source from the documented object

**Files:** `Scrivi/Views/Detail/` (new source section), `ObjectDetailSheet.swift`

- An affordance on the Detail Sheet: name the source, create it, attach `cites`.
- ⚠️ **`source` is the ONLY project-scoped kind** (`ObjectTypes.hpp:45-46`) — the create call passes **no
  `worldID`**, even when the documented object is world-scoped. ⚠️ **Do not derive scope by restating a
  kind list**; derive it from `scrivi_list_object_kinds` (D5), per the standing rule — ⚠️ **a Swift
  restatement of exactly this shape blocked object creation outright in SP-104.**
- ⚠️ **Two writes, one intent.** If the object is created and the edge fails, **keep the source and report
  honestly** — `project_unlinked_world_objects_are_normal`: an unlinked object is a library entry, never
  debris. **Never roll back a created source.**
- Follow SP-119's guarded-exit rule: an in-progress source draft is unsaved work and must not be silently
  discarded (I-0167 / I-0168 closed all six routes into this surface).

### T-0455 — ⚠️ Attach an EXISTING source to a second object

**The I-0164 mirror, named at planning rather than after a writer hits it.**

A writer citing one work across three characters must be able to reach the source that already exists.
`ObjectRelationPicker` already lists **every object, every kind, every world, unfiltered** and `cites`
accepts **any** pair of kinds — so the plumbing exists and this is a reachability Task, not a new
capability.

⚠️ **This is the operation, not the field, that S11 must catch** — and the reason Ruling 3 exists.

### T-0456 — ⚠️ T-0365's second entry point

**Files:** `Scrivi/Views/Detail/` (or `ObjectCard.swift`), reusing `SourcesCard.swift:235`

- The documented object surfaces **its own** sources; clicking one opens **the same `CitationPopover`**.
- ⚠️ **Reuse the popup, do not write a second one.** §3.1.1's wording is *"one popup implementation, two
  entry points"* — a second implementation would satisfy the screenshot and violate the design.
- ⚠️ **`attribution` needs a sensible value from this entry point.** The card's phrasing is *"via <object>"*,
  which answers *"why is this here?"* for a scene walk; opened **on the citing object itself** that
  sentence is circular. **Decide the wording explicitly and record it.**

### T-0457 — Citation fields + footnote text

- Citation fields in the sheet, stored via T-0453 into `attributes`.
- ⚠️ **`CitationPopover` currently reads `displayName`/`subtitle`/`notes` and nothing else.** If it is not
  extended, **every citation field a writer types is invisible in the very popup §3.1.1 calls "the
  record."** ⚠️ **This is the capability-without-surface trap reappearing INSIDE the sprint that is curing
  it** — the field would be written, stored, round-tripped, and never shown.
- `SourceDetail` gains the fields; both entry points render them.

### T-0458 — ⚠️ S11 written FIRST, then the live pass

- **Before implementation:** the S11 table — ⚠️ **operations AND fields** (Ruling 3).
- **After:** `ctest` (macOS arm64 + x86-64 + sanitizers), interop, app **BUILD SUCCEEDED**.
- ⚠️ **Then the live click-through, which is the only thing that has ever found these defects.**

---

## 5. ⚠️ S11 — the design-tension check, written BEFORE the UI (T-0458)

⚠️ **Ruling 3: this enumerates OPERATIONS as well as fields.** SP-119's S11 listed fields only, and
**that is precisely how I-0164 slipped through** — *"attach an image already in the world"* was an
operation no field table could have surfaced. ⚠️ **Not surfacing something is allowed; not having
considered it is not.**

### 5.1 ⚠️ OPERATIONS on a source — the half SP-119's S11 did not have

| Operation | Decision | Reasoning |
| --------- | -------- | --------- |
| **Create a NEW source from the documented object** | ✅ **SURFACED** — T-0454 | AC8 proper. The affordance lives on the object, never on the `sources` card (user ruling). |
| ⚠️ **Attach an EXISTING source to another object** | ✅ **SURFACED** — T-0455 | ⚠️ **The I-0164 mirror, and the reason this table exists.** A writer citing one work across three characters must not create it three times. `cites` accepts any pair of kinds and the picker already lists every object — this is reachability, not new capability. |
| **View a source's citation record** | ✅ **SURFACED** — T-0456/T-0457 | §3.1.1's *"one popup, two entry points"*, owed since SP-102. |
| **Detach a `cites` edge** (uncite) | ✅ **SURFACED — free** | `ObjectRelationsSection` already removes edges and a `cites` edge is an ordinary edge. ⚠️ **Verified as reachable, not assumed** — S11's job. |
| **Edit a source after creating it** | ✅ **SURFACED — free** | A `source` is an object; its own Detail Sheet already edits it. ⚠️ Reached by navigating to it from the citing object (T-0456), which is why that Task is not cosmetic. |
| **Delete a source object** | ⚠️ **NOT SURFACED** | ⚠️ **Consistent with SP-119's asset-deletion ruling** — an unlinked source is a bibliography entry, not debris (`project_unlinked_world_objects_are_normal`). Object deletion has no surface anywhere in Scrivi; inventing one here would be an unruled trade. |
| **Cite the same source from the same object twice** | ⚠️ **REFUSED by the core** | Duplicate edges are rejected (AC6 precedent). The picker pre-warns rather than erroring — `existingObjectIDs` already does this. |
| **Cite a source from a PENDING object** | ⚠️ **BLOCKED, and explained** | Doc 3 §4.6 rule 1: the graph is frozen toward an unavailable world in both directions. `isReadOnly` already gates the relate affordance; the sources affordance must honour the same gate. ⚠️ **Not a new rule — an existing one this surface must not forget.** |
| **Attach an image to a source** (attribution's own attribution) | ⚠️ **NOT SURFACED, deliberate** | Works at the core via `worldID: ""` and is noted in SP-119's S11. A source is a citation record, not a picture. ⚠️ **Recorded as an omission, not an oversight.** |
| **Create a source NOT attached to any object** | ⚠️ **NO PATH, by design** | ⚠️ **The user's founding ruling for this Epic**: creation belongs on the documented object, *"it would muddy the fact that the source must be associated with an object in the world."* ⚠️ **But see the create/attach split in §5.3** — a source whose edge fails is kept, not deleted. |

### 5.2 FIELDS — where citation data lives

⚠️ **A `source` object has NO citation fields.** `ObjectRecord` carries `displayName`, `subtitle`,
`notes`, `tags`, `attributes` and nothing else (`ObjectTypes.hpp:208-224`).

| Field | Decision | Reasoning |
| ----- | -------- | --------- |
| `displayName` | ✅ **SURFACED** — the work's title | What the writer recognises. Required; a source with no name is unciteable. |
| `subtitle` | ✅ **SURFACED** — the short-form citation | ⚠️ **Already rendered by `CitationPopover`.** Author + year reads well here and needs no new field. |
| `notes` | ✅ **SURFACED** — ⚠️ **this is "footnote text"** | AC8's phrase. Long-form, already a `TextEditor`, already in the popup. ⚠️ **No new field is needed for AC8's footnote half** — the requirement was already satisfiable, which is worth stating rather than discovering later. |
| `attributes["author"]` | ✅ **SURFACED** — T-0457 | Structured, so a later bibliography export need not parse prose. |
| `attributes["url"]` | ✅ **SURFACED** — T-0457 | ⚠️ **Displayed as text, NOT as a live link.** Opening arbitrary URLs from project data is an unruled trust decision. |
| `attributes["publisher"]` | ✅ **SURFACED** — T-0457 | Standard citation element. |
| `attributes["year"]` | ✅ **SURFACED** — T-0457 | ⚠️ **A free-text STRING, not a date.** "c. 1387", "n.d." and "forthcoming" are all real citation years; a date picker would refuse them all. |
| `attributes["page"]` | ✅ **SURFACED** — T-0457 | Where in the work. ⚠️ Belongs on the *edge* in a stricter model (two objects citing different pages of one book) — ⚠️ **recorded as a known modelling limit**, not silently accepted. |
| `attributes["accessed"]` | ✅ **SURFACED** — T-0457 | Required for web sources; free text for the same reason as `year`. |
| ⚠️ **Unknown `attributes` keys** | ✅ **PRESERVED, not shown** | T-0453 patches; it never reconstructs. A key a future build writes must survive a save by this build. |
| `tags` | ⚠️ **NOT surfaced on the source affordance** | Already editable on the source's own sheet. A second editor would be a second truth. |
| `status`, `createdBy`, `modifiedBy` | ⚠️ **NOT SURFACED** | Core-stamped; the existing metadata block already shows what matters. |
| `image` | ⚠️ **NOT SURFACED** — see §5.1 | Deliberate omission. |

### 5.3 ⚠️ Two writes, one intent — the create/attach split

Creating a cited source is **`create_object` then `create_edge`**, and ⚠️ **the core offers no
transaction across the two.**

✅ **Ruled here, from precedent rather than invention:** if the object is created and the edge fails,
**keep the source and report honestly.** `project_unlinked_world_objects_are_normal` — an unlinked object
is a library entry, never debris — and ⚠️ **rolling back would delete a writer's typing to tidy up a
failure she did not cause.** The message must name what exists and what did not happen.

### 5.4 Capabilities the core has that this sprint does NOT surface

⚠️ **Named rather than silently skipped:**

- **`cites` in the reverse direction** — the type is unconstrained on **both** ends, so an object may cite
  a source *or* a source may cite an object. ⚠️ **This sprint always creates `source ──cites──▶ object`.**
  `EdgeView.label` reads correctly from either end regardless, so nothing breaks; the single direction is
  for the writer's sake, not the model's.
- **A source citing another source** — legal (`cites` is unconstrained) and genuinely meaningful for
  secondary citation. **No surface.** Future scope.
- **Edge `note`** — every edge carries one; unused here. ⚠️ **This is where a per-citation page number
  belongs** if §5.2's `page` limit is ever revisited.

---

## 6. ⚠️ The evidence rule this sprint is planned against

**SP-118 raised thirteen Issues. SP-119 raised seven. ⚠️ Not one of the twenty came from a test.**

The suites were green throughout and were **not wrong** — they assert what the core does. ⚠️ **What no
suite covered was whether a writer could reach the capability, and what happened when the world moved
under her.**

So for SP-120:

- ⚠️ **Green suites are not evidence AC8 is met.** AC8 closes on a writer creating a source and seeing it
  appear on the `sources` card — **by use**.
- ⚠️ **A `sources` card that still renders empty is indistinguishable from one that works** (T-0365's own
  archive says exactly this). **The verification must show content**, not absence of error.
- **Find how the app already does it before writing new behaviour** — SP-118's dominant defect was four
  Issues each violating a rule already present in the repo. The precedents that bind here are named
  inline above: the `tags` shape, the patch-don't-reconstruct rule, the D5 derivation, the guarded exit,
  and the unlinked-object ruling.

---

## 7. Definition of Done

- [x] ✅ **T-0453 – T-0458** all ✅ **Verified** (user-approved 2026-08-24) and archived in the same step
- [x] ✅ **S11 written BEFORE implementation**, enumerating **operations and fields** — ⚠️ **and it changed
      the work three times** (§5)
- [x] ✅ `ctest` **567/567** · interop **127/127** · app **BUILD SUCCEEDED**
- [x] ✅ ⚠️ **Live pass: a source created from an object APPEARS on the `sources` card** — content, not
      empty, **for the first time since SP-102**
- [x] ✅ ⚠️ **Live pass: an existing source attached to a SECOND object** (T-0455) — ⚠️ **and to a second
      KIND**: *Myton at 23* (character) and *Tintagael* (location)
- [x] ✅ ⚠️ **Live pass: the same popup opens from BOTH entry points** (T-0456)
- [x] ✅ **AC8 judged by use**, on a real sequel project against the live Eskandar world — never on
      fixtures (the I-0137 rule)
- [x] ✅ **User verification** given for all six Tasks and both Issues
- [x] ✅ **Sprint closed 2026-08-24 on user approval.**

---

## 8. Explicitly OUT of scope

| Item | Where it goes |
| ---- | ------------- |
| **AC11** — `[Linux]` parity | **SP-121** |
| **AC12** — full suite verification + Epic close prep | **SP-122** |
| **Footnotes rendered IN manuscript text** | ⚠️ **EP-032** — needs object rendering inside scene text (Doc 1 §3.4.1). ⚠️ **This sprint is what unblocks EP-032's AC5** |
| **Typed citation fields in ScriviCore** | Not needed — Ruling 1 |
| **Thumbnail generation** (`thumbnailAssetID` read, never written) | Unscheduled |
| **Asset deletion** | ✅ Ruled acceptable to omit (SP-119) |
| **I-0147** — the 60 s post-crash lock window | ✅ Accepted → network-worlds design |
| iOS / iPadOS / visionOS | ✅ Deferred by Q-c |

---

*Last Updated: 2026-08-24 (**SP-120 🟡 ACTIVATED** — EP-034's sixth of eight sprints. Six Tasks
**T-0453–T-0458**. ⚠️ **Three rulings taken at planning:** citation data lives in `attributes` (no ABI
change, sprint stays `[Apple]`); ⚠️ **T-0365's second entry point — owned by no AC and no sprint — is
adopted here**; and **S11 enumerates OPERATIONS as well as fields**, with the I-0164 mirror (**attach an
existing source**) named in advance as T-0455. ⚠️ **Verified against code, not assumed: every backend
piece of AC8 already ships** — this sprint builds surface only. Next available Task **T-0459**; Issue
**I-0169**.)*

---

## 9. Retrospective

### ✅ What worked

- ⚠️ **S11, written FIRST and widened to OPERATIONS.** It earned its place three times: it found **T-0455**
  before a writer did; it caught **`CitationPopover` being unable to SHOW** the fields the sprint was
  adding — ⚠️ **`capability_without_surface` inside the sprint curing it**; and it **shortened the work**
  by establishing `notes` already *was* AC8's footnote text.
- ⚠️ **Reading the code before planning.** Every backend piece already shipped, so the sprint was scoped
  `[Apple]`-only and stayed there. ⚠️ **`ctest` holding at 567/567 is the EVIDENCE of that, not an absence
  of evidence.**
- ✅ **The round-trip test written before the encoder**, with a **negative control** proving the
  `{"k","v"}` trap is real. Without it the citation fields would have been written, stored — and lost.
- ✅ **Grepping for T-0365's owed second entry point.** It was owned by **no AC and no sprint**; the popup
  had exactly one call site. It would otherwise still be owed.

### ⚠️ What did not

- ⚠️ **I had the tap-gesture order backwards** — the intuitive order, which kills the double-click. The
  correct order was already in `ObjectRelationsSection`, in the same directory
  (`feedback_look_for_existing_pattern_first`).
- ⚠️ **I-0169: I built a surface that taught a different verb from every other object row**, using a hook
  (`CardContext.openObjectDetail`) that already existed and that I did not call. ⚠️ **The writer's FIRST
  instinct found it.**
- ⚠️ **I-0170: the popup quietly UNDER-REPORTED the graph.** Every field present and populated, and still
  not true. ⚠️ **No field-level review could have caught it** — it took real data, a source cited twice
  across two kinds.
- ⚠️ **I flagged `page` as being in the wrong place in S11 §5.2 and shipped it anyway.** ⚠️ **The limit was
  hit ONE SPRINT LATER** by the first real citation work → **T-0459**. ✅ **Recording it made the diagnosis
  five minutes instead of a rediscovery** — but recording a known-wrong model is not the same as not
  shipping one.

### ⚠️ The pattern that has now held for three sprints

**22 consecutive Issues across SP-118, SP-119 and SP-120 came from clicking. NONE came from a suite.**

⚠️ The suites are not wrong and were green throughout. **They assert what the code does. They cannot
assert that a writer can reach it, or that what she reads is true.** Both of this sprint's defects were of
the second kind.

### Carried out of SP-120

| Item | Owner |
| ---- | ----- |
| **AC11** — `[Linux]` parity | **SP-121** |
| **AC12** — full suite verification + Epic close prep | **SP-122** |
| ⚠️ **T-0459** — per-citation locators on the `cites` EDGE; ⚠️ **`scrivi_update_edge` does not exist** | **EP-032** (user-ruled) |
| ⚠️ **Thumbnail generation** — `thumbnailAssetID` read, never written | Unscheduled |
| **I-0147** — the 60 s post-crash lock window; ✅ Accepted | **Network-worlds design** |

---

*Last Updated: 2026-08-24 (**SP-120 ✅ CLOSED, user-approved.** Six Tasks + two Issues Verified and
archived in the same step. ✅ **AC8 closes**; ✅ **T-0365 is COMPLETE after four sprints** and the `sources`
card shows content for the first time since SP-102; ✅ **EP-032 is genuinely unblocked.** ⚠️ **Both Issues
came from the live pass, neither from a suite — 22 consecutive.** ⚠️ **T-0459 filed to EP-032**: SP-120's
own S11 predicted the `page`-on-source limit and it was hit one sprint later. Active Sprints 1 → 0;
EP-034 stays 🟡 Active, **6 of 8 sprints closed**.)*
