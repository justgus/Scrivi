# Sprint SP-118 (CLOSED)

## SP-118: `[Cross]` Related objects + relationship creation

**Status:** ✅ **CLOSED 2026-08-23 (user-approved)**
**Epic:** [EP-034: `[Cross]` Object Detail & Media](../Epics/Epic-active.md) — its **fourth** sprint
**Goal:** Make the Detail Sheet show **what an object is connected to**, let the writer **create those
connections from it**, and ⚠️ **repair the vocabulary that stops eight of ten kinds from connecting at all
in existing projects.**
**Start Date:** 2026-08-22
**End Date:** 2026-08-23
**Trades implemented:** ✅ **D4-A** (inline creation) · **R3**, **R4**, **R5**'s second half, **R7**'s
second half — ⚠️ **the plan said "reusing `ObjectPickerView`"; see the T-0443 deviation below**
**Closes:** **AC5**, **AC6**

---

### ⚠️ T-0416 is no longer theoretical — it was CONFIRMED on the rig 2026-08-21

The design doc asked for this check *"before this sprint plans."* It was run, and it reproduced.

`the-twisted-remains-of-myself.scrivi` still carries the **pre-I-0125** vocabulary:

```diff
- "inverseLabel": "features"          ← today's seed
+ "inverseLabel": "has characters"    ← on disk
+ "sourceKind": "character"           ← the constraint I-0125 REMOVED
```

⚠️ **Proven by running the same operation against both vocabularies**, through `scrivi_*`:

| Project | Result |
| ------- | ------ |
| Fresh seed | Edge created ✅ |
| Drifted vocabulary | ❌ `"endpoints do not satisfy the kind constraints of relation type 'appears-in'"` |

⚠️ **`appears-in` is the type EIGHT OF THE TEN object cards use.** In an affected project a writer cannot
relate a chronicle, building, vehicle, item, map, artifact, faction or rule to a scene — ⚠️ **and the
object is already written to disk when the edge fails**, so she is told creation failed while the object
exists. That is I-0125's exact reported symptom, still live for old projects.

✅ **`tintagael` is CLEAN** — hand-patched when I-0125 was fixed. ⚠️ **So the rig will NOT reproduce this
by default**; SP-118 must test against a deliberately-drifted fixture, or the sprint will "pass" while the
defect stands.

> ### ✅ RULED 2026-08-21 — reconcile on open, seeded types only
>
> On load, ensure every **seeded** code exists and matches the current seed definition. ⚠️ **Writer-authored
> types are never touched, and nothing is ever deleted** — the file is writer-editable by design.
>
> ⚠️ **Accepted consequence, stated plainly:** this **overwrites a seeded type a writer deliberately
> edited.** The alternative (never touch a modified seeded type) leaves a hand-edited `appears-in` broken
> forever with no explanation. **The user chose self-healing over preserving edits to seeded types.**

---

### Assigned Tasks

| ID | Title | Codebase | Priority | Status |
| -- | ----- | -------- | -------- | ------ |
| **T-0441** | ⚠️ **T-0416** — reconcile seeded relation types on open | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0442** | **R3** — related-objects section in the Detail Sheet | `[Apple]` | **High** | ✅ **Verified** |
| **T-0443** | **D4-A** — create a relationship inline; ⚠️ **widen the picker's scene-target filter** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0444** | **R5/R7** — double-click **and** right-click → **"Show"** push-navigates to a related object | `[Apple]` | **High** | ✅ **Verified** |
| **T-0445** | ⚠️ **Pending far-endpoints** in the related list — named, explained, never silently dropped | `[Apple]` | Medium | ✅ **Verified** |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| **I-0149** | ⚠️ **T-0441's reconciliation never ran on project OPEN** — it reconciled on READ. ⚠️ **Found by the user on the real rig** after this sprint reported green | **High** | ✅ **Verified** |
| **I-0150** | ⚠️ **`xcodebuild test` LAUNCHES the app and reopened the user's real projects**, writing to them. ⚠️ **Found by the user refusing a wrong explanation Claude had given** | **High** | ✅ **Verified** |

⚠️ **I-0147 remains an Accepted limitation** and is not SP-118's.

---

### Task detail

#### T-0441 — ⚠️ T-0416, the only `[ScriviCore]` work here

`RelationTypeStore::load` re-seeds **only** when the file is missing or unparseable
(`RelationTypes.cpp:150-176`); a valid file is loaded **verbatim**, so a seed change never reaches an
existing project. ⚠️ **Confirmed to survive a reopen** — this is not a cache, it is permanent.

**Reconcile on load:** for each type in the current seed, if the file lacks it → add it; if it differs →
replace it with the seed definition. ⚠️ **Leave every non-seeded code alone. Delete nothing.**

⚠️ **Write only when something actually changed** — an unconditional rewrite would touch
`relation-types.json` on every open, churning mtimes and Git status for no reason.

⚠️ **Test against a DRIFTED fixture**, not a fresh project. A fresh project is already correct, so a test
that seeds normally passes vacuously — ⚠️ **precisely how this defect survived since 2026-08-17.**

#### T-0442 — the related-objects section (R3)

✅ **The data already exists.** `EdgeView` carries `label` (reads correctly from the queried endpoint),
`isForward`, `otherID`, `otherDisplayName`, `otherKind`, and the pending fields
(`ScriviEngineGraph.swift:288-330`). **AC5 is a rendering task, not a data task.**

⚠️ **Group or label by direction.** `label` is already endpoint-correct — do **not** recompute it in Swift,
which would be the restated-rule defect this Epic has paid for nine times.

#### T-0443 — inline creation (D4-A)

Reuse **`ObjectPickerView`**, do not write a second picker.

⚠️ **The one required change**, and the interesting part of the trade:

```swift
// ObjectPickerView.swift:209 — hardcodes a SCENE target
$0.acceptsSource(kind: cardKind.kind) && ($0.targetKind == nil || $0.targetKind == "scene")
```

For object→object it must accept types whose target is **not** a scene. ✅ **No ABI change** — the C ABI
already encodes a scene endpoint as the sentinel `"scene"` and round-trips it.

⚠️ **AC17 stands:** the picker lists objects **from all worlds, deliberately unfiltered** — a writer
relating an object must be able to reach one that lives in another world.

⚠️ **Never gesture-only** (D4-C rejected): VNC cannot carry drag-and-drop, and AC11 requires Linux parity.

#### T-0444 — push-navigation from the related list (R5, R7)

**Double-click** *and* **right-click → "Show"**. ✅ `ObjectDetailHistory` already handles this: `visit()`
truncates forward history, and re-visiting the current object is a no-op.

⚠️ **The word is "Show", not "View Detail"** — R7 names them differently for the two entry points, and the
sheet is already open in this case.

#### T-0445 — pending far-endpoints

A related object whose world is away must be **listed, named and explained** — never hidden. Doc 3:
*absence is never deletion*. ✅ `EdgeView` already carries `otherPending`/`otherWorldStatus`/`otherWorldID`.

⚠️ **I-0124's lesson:** `otherKind` travels on the edge precisely because a pending object is absent from
the index — **do not attribute a pending object by index lookup.**

---

### Success criteria

| # | Criterion |
| - | --------- |
| **S1** | ⚠️ **Against a DRIFTED fixture** (pre-I-0125 `appears-in`), opening the project **repairs** the seeded type, and a `chronicle` can then be related to a scene. ⚠️ **The same test must FAIL against the un-fixed core.** |
| **S2** | ⚠️ A **writer-authored** relation type in that same file **survives** reconciliation untouched, and **nothing is deleted**. |
| **S3** | ⚠️ Opening a project whose vocabulary is **already current** does **not rewrite the file** (mtime unchanged). |
| **S4** | The Detail Sheet lists this object's edges with **labels reading correctly from this endpoint** (AC5). |
| **S5** | A relationship can be **created from the sheet**; it appears from **both** endpoints and is **rejected as a duplicate** from the second (AC6). |
| **S6** | The picker offers **object→object** types, and ⚠️ still lists objects **from other worlds** (AC17). |
| **S7** | **Double-click** and **right-click → "Show"** push-navigate; back/forward move through that history. |
| **S8** | ⚠️ A **pending** far-endpoint is **listed, named, and explained**, and cannot be edited. |
| **S9** | ⚠️ **LIVE CLICK-THROUGH on the real rig** — ⚠️ **including a deliberately drifted project**, since `tintagael` is clean and will not reproduce T-0416. |
| **S10** | `ctest` green (arm64 · x86-64 · sanitizers) · Linux green · interop green · app **BUILD SUCCEEDED**. |

⚠️ **S1 and S3 are the pair that matters for T-0441.** S1 alone would pass with an unconditional rewrite;
S3 is what keeps the fix from churning every project's file on every open.

---

### Sprint Notes

- ✅ **AC5 and AC6 close here.** ⚠️ **AC2 still cannot** — `tags` is SP-119.
- ⚠️ **AC9's second half is NOT this sprint** — "a world goes away *while a sheet is open*" is a different
  trigger from the one SP-117 verified (`feedback_verify_each_half_separately`). It belongs with SP-119.
- ⚠️ **T-0416 has been open since 2026-08-17 and deferred once.** It is scheduled here because SP-118's
  entire subject is relationship creation — the one operation it breaks.
- ⚠️ **Every new `.swift` file MUST be added to `project.pbxproj` in the same step** (CLAUDE.md). SP-117
  added three across 4 build-file entries each + file refs + group + 3 Sources phases.
- ⚠️ **`pgrep Scrivi` before `xcodebuild test`** — a running instance blocks the runner. ⚠️ **AND NOTE
I-0150: that check does NOT make testing safe.** `xcodebuild test` **launches** the app, which restores the
writer's real projects from bookmarks and writes to them with just-compiled code. The check guards the
*converse* hazard. Fixed in `restoreOpenProjects()`, but the reasoning error is the thing to remember.

### Retrospective

*(Filled in at close.)*

---

---

## ⚠️ Execution status — 2026-08-22

**All five Tasks ✅ Implemented — Not Verified.** ⚠️ **S9 (the live click-through on the real rig,
including a deliberately drifted project) has NOT been run** — it needs the user, and it is the criterion
five EP-034 defects have been found by.

| Criterion | State |
| --------- | ----- |
| **S1** — drifted fixture repaired; chronicle→scene then relates | ✅ **and PROVEN to fail against the un-fixed core**; ⚠️ **but it did NOT prove the repair runs on OPEN — see I-0149** |
| **S2** — writer-authored type survives; nothing deleted | ✅ |
| **S3** — a current project is NOT rewritten (mtime unchanged) | ✅ |
| **S4** — edges listed with endpoint-correct labels | ✅ built; ⚠️ **not yet seen by a writer** |
| **S5** — relationship created from the sheet; both endpoints; duplicate rejected | ✅ at the boundary; ⚠️ **click-through owed** |
| **S6** — picker offers object→object types, still lists other worlds | ✅ built; ⚠️ **click-through owed** |
| **S7** — double-click and right-click → "Show" push-navigate | ✅ built; ⚠️ **click-through owed** |
| **S8** — pending far-endpoint listed, named, explained, not editable | ✅ built; ⚠️ **click-through owed** |
| **S9** — ⚠️ **LIVE CLICK-THROUGH incl. a drifted project** | ❌ **NOT RUN — owed by the user** |
| **S10** — all suites green | ✅ |

### ⚠️⚠️ I-0149 — S1 was green and the migration still did not happen

⚠️ **The user opened a drifted project with this sprint's fix in the running binary and NOTHING WAS
REPAIRED.** `reconcileSeeded` was confirmed present in the shipped binary; the file was byte-identical.

⚠️ **The ruling said "reconcile ON OPEN". T-0441 reconciled ON READ.** `RelationTypeStore::load()` runs
only when something asks for the vocabulary — listing types, creating an edge, opening the picker — and
**opening a project asks for none of them**. `the-twisted-remains-of-myself.scrivi` has an empty object
index and no `relationships.jsonl`, so nothing ever read the file.

⚠️ **Every T-0441 test called `store.load()` directly.** They proved the repair worked and proved nothing
about what invokes it — ⚠️ **a test that calls `load()` to check that `load()` repairs is a tautology
wearing a fixture.**

✅ **Fixed (I-0149):** `ProjectOpener::open` reconciles as repair pass **(e)**, ⚠️ **before** the
dangling-edge repair (d), so edges are judged against the current seed rather than a drifted one. ✅ The
new test **fails against T-0441-as-shipped**, and the fix was ✅ **verified against a copy of the user's
real project**, not only a fixture.

> ⚠️ **The sharp lesson: "on open" is an EVENT, not a function.** The ruling named a moment; the
> implementation picked a function that seemed adjacent to it. **When a ruling names a moment, the test
> must reproduce that moment** — here, *"open a project and touch nothing else."*

### ⚠️ S1's negative control was actually run

The plan required *"the same test must FAIL against the un-fixed core."* Reconciliation was temporarily
disabled and the suite re-run:

```
S1  REQUIRE( found.value().inverseLabel == "features" )
    with expansion: "has characters" == "features"          ← FAILED, as required
S1  REQUIRE( edge.ok() )
    with expansion: false                                    ← the WRITER-FACING symptom
```

⚠️ **The label assertion aborts the test before the `create()` assertion is reached**, so the edge failure
was confirmed by a second run with the label checks softened to `CHECK`. **Both halves fail without the
fix** — the test is not vacuous. Both temporary edits were reverted and verified reverted (grep = 0).

### Suites at implementation

| Suite | Result |
| ----- | ------ |
| `ctest` arm64 | ✅ **561/561** (was 554 — **+5 T-0441, +2 I-0149**) |
| `ctest` x86-64 | ✅ **561/561** |
| `ctest` ASan + UBSan | ✅ **561/561** |
| **Linux** (GCC 13, Ubuntu 24.04, ⚠️ **non-root**) | ✅ **565/565** |
| Interop | ✅ **120/120** (was 115 — **+4 SP-118, +1 I-0150**) |
| App | ✅ **BUILD SUCCEEDED** |

⚠️ **The Linux run used a purpose-built second image.** `platforms/linux/docker/Dockerfile` configures
`SCRIVI_BUILD_TESTS=OFF`, so "the Linux container builds" is **not** evidence that `ctest` ran there
(`project_linux_container_tests_off`). The image was run as a **non-root** user, because several tests
assert that a read-only path refuses a write and root ignores permission bits.

---

## ⚠️ What was built, and the two judgement calls inside it

### T-0441 — reconciliation (`RelationTypes.cpp`)

`load()` now reconciles the **seeded** codes: missing → added, differing → replaced with the seed, and
⚠️ **writes only when something actually changed**. Nothing is deleted, and a code this build does not seed
is left alone — it is either the writer's or a **later build's**.

⚠️ **The accepted consequence is asserted in the suite, not left implicit.** A test named
*"⚠️ ACCEPTED CONSEQUENCE — an edited SEEDED type is overwritten on open"* pins the cost the user chose, so
it is visible to whoever reads the suite next rather than discovered by a writer whose relabelled `cites`
quietly reverted.

⚠️ **One pre-existing test had to change.** `upsert replaces by code` edited the seeded `cites` and asserted
the edit survived `load()` — which now **contradicts the ruling**. It was repointed at a writer-authored
code, where "replaces by code" is still exactly what it tests.

### ⚠️ T-0443 — the reuse instruction was followed in substance, not literally

The plan said *"Reuse `ObjectPickerView`, do not write a second picker"* and named the one required change.
⚠️ **`ObjectPickerView` could not carry the object→object case without becoming mode-conditional
throughout**: it is bound to one `ObjectCardKind` and one **scene** target — it titles itself "Add
Characters", filters to a single kind, hands typed text to a *card's* draft state, and picks its relation
type by asking which types accept `kind → scene`. Object→object inverts every one of those.

**What was done instead:** `ObjectRelationPicker` reuses the *reusable* part verbatim in shape — AC17's
unfiltered all-worlds listing, world grouping, and ⚠️ **the `minHeight` I-0127 paid for** — while
`ObjectPickerView` keeps its scene filter, now named `targetAcceptsScene` rather than an inline
`== "scene"`.

⚠️ **This is a deviation from the plan's letter and the user should rule on it.** The alternative was one
picker whose every property was conditional on a mode flag.

✅ **No ABI change**, exactly as the plan predicted: scenes are not an `ObjectKind`, so a constrained scene
endpoint already crosses as the sentinel `"scene"`. It is now spelled **once**, as
`RelationTypeEntry.sceneToken`, with `targetAcceptsScene` / `targetAcceptsObject` / `sourceIsScene` derived
from it — ⚠️ **the two target properties OVERLAP rather than negating**, because an unconstrained type
accepts both.

---

## ⚠️ Deliberate scope refusals

- ⚠️ **AC5 and AC6 are NOT marked closed.** They close on **S9**, which is the user's to run. Marking them
  closed on a green suite is precisely the `capability_without_surface` move this Epic exists to cure.
- ⚠️ **`tags` untouched** — SP-119, so **AC2 still cannot close**.
- ⚠️ **AC9's second half untouched** — "a world goes away *while a sheet is open*" is SP-119's.

## ⚠️ Owed before this Sprint can close

0. ⚠️ **RE-RUN THE OPEN on the real project** — ⚠️ **the previous attempt proved nothing**, and this is
   what I-0149 exists for. ✅ A backup was taken:
   `~/Desktop/the-twisted-remains-of-myself-BACKUP-sp118-111809.scrivi`.
1. ⚠️ **S9 — the live click-through, including a DELIBERATELY DRIFTED project.** `tintagael` is clean and
   **will not reproduce T-0416**; `the-twisted-remains-of-myself.scrivi` on the rig **still carries the
   pre-I-0125 vocabulary** and is the natural subject. ⚠️ **Back it up first — it is real writing work.**
   ⚠️ **Opening it with this build REPAIRS it**, which is the point but is also not undoable.
2. **A ruling on the T-0443 deviation** above.
3. User verification of all five Tasks.

---

---

## Retrospective

### What SP-118 delivered

✅ **All five Tasks and thirteen Issues Verified 2026-08-23.** The Detail Sheet now shows what an object is
connected to, lets the writer create those connections, and ⚠️ **the vocabulary that stopped eight of ten
kinds from relating in existing projects is repaired on open.**

**Closes AC5, AC6 and AC7.**

### ⚠️ The number that matters: 13 Issues, 0 found by tests

Every one of I-0149 – I-0161 was found by **use**, after the suites went green. ⚠️ **The suites were not
wrong** — they assert edge creation, duplicate rejection, both-endpoint visibility and pending
presentation, and every one of those held up under live use. **What no test covered was whether a writer
could reach any of it.**

⚠️ **This is `capability_without_surface` for the third time in this Epic**, and the first time it appeared
at the **core** (I-0149) rather than in the UI.

### ⚠️ The dominant defect: an existing correct pattern the new code did not follow

**Four of the thirteen were one failure**, each violating a rule that already existed in this repo:

| Issue | The rule, already written |
| ----- | ------------------------- |
| I-0152 | the Navigator names an untitled scene "Scene N", never an ID |
| I-0155 | `ObjectCardModel.rename` re-reads before patching |
| I-0157 | I-0132 ruled `selectedSceneID` the source of truth |
| I-0158 | `SceneNavigatorView` uses `List(selection:)` |

⚠️ **The user's question is the cleanest statement of it:** *"A Swift standard List View handles all this
automatically, which makes me wonder why it is so hard for you."* It was not hard. New machinery was built
beside machinery that already worked. → `feedback_look_for_existing_pattern_first`.

### ⚠️ SwiftUI events are asynchronous — target the right callback

**User guidance at close, and the general principle behind I-0161, I-0156 and I-0157:**

> *"Many event detection operations do not occur in the 'intuitive' place in the codebase… they are run
> asynchronously between the time they are triggered and the time they are actually executed. We need to
> make sure we target the right callback function."*

⚠️ **The diagnostic tell is "it works, but the other half of the UI doesn't agree."** That is almost never
focus, gesture arbitration or rendering — the first guess every time — it is two views driven from
different points in one asynchronous sequence. → `feedback_swiftui_async_callback_targeting`.

### ⚠️ Three defects Claude caused by asserting behaviour without checking it

- **I-0151** — a **comment** claiming `openObject` resolves an empty worldID. Never checked against
  `ObjectStore.cpp`. False. Broke navigation to every world-scoped object.
- **I-0161 (attempt 2)** — a **diagnosis** of a timing race in code that ⚠️ **was never compiled into the
  macOS build.** The user's *"It didn't change anything"* was the correct verdict; Claude read it as "it
  ran and was wrong."
- **I-0150** — an **attribution**: Claude told the user *they* had reopened Scrivi and rewritten their
  project. ⚠️ **It was Claude's own `xcodebuild test`**, which launches the hosted app.

⚠️ **One root cause: asserting behaviour without verifying it.** Whether the assertion lands in a comment,
a diagnosis, or an accusation is incidental. → `feedback_prove_code_is_reached`,
`feedback_evidence_before_attribution`.

### ⚠️ Severity is not carried by the reporter's confidence

**I-0155 — silent data loss, a saved note reverting a saved rename — was reported as:** *"It isn't
necessarily a defect. More like an unintended consequence… Maybe there is a defect here after all."*

⚠️ **Third time in this Epic** (I-0148, I-0154, I-0155). **Weight the observation, not the hedge.**

### ⚠️ Two data-safety findings worth carrying forward

1. **I-0150 changed what "safe to test" means.** `xcodebuild test` is **not** read-only on this project —
   it is an app launch with full access to the writer's real projects through saved bookmarks. Fixed
   structurally; ⚠️ **`pgrep Scrivi` never protected against this and reads as though it does.**
2. **I-0159: a scroll region with no affordance is a data-loss report waiting to happen.** Myton appeared
   to show 3 of 8 relationships. ⚠️ **Whether rows are absent or merely unreachable-looking, the writer
   concludes her work is gone.**

### What went right

- ✅ **T-0441's negative control was actually run**, and against a deliberately drifted fixture — a fresh
  project is already correct, so a normally-seeded test would have passed vacuously.
- ✅ **The accepted consequence of the reconciliation ruling is asserted in the suite**, not left implicit.
- ✅ **Temporary `SCRIVI-DIAG` logging settled I-0161** after two failed theories, and was removed once it
  had. ⚠️ **Instrumenting a path is cheaper than a third hypothesis.**
- ✅ **The user's live pass caught everything the suites could not**, and is now unambiguously the highest-
  value verification step in this Epic.

### Carried out of SP-118 — do not read as delivered

| Item | Owner |
| ---- | ----- |
| ⚠️ **`tags` deferred** (user ruling) — **R2 not fully met, AC2 CANNOT CLOSE** | **SP-119** |
| ⚠️ **AC9's second half** — a world going away *while a sheet is open* is a DIFFERENT trigger from the one verified | **SP-119** |
| **AC3, AC4, AC8** — images and sources | **SP-119 – SP-120** |
| **I-0147** — the 60 s post-crash lock window; ✅ Accepted, regression-tested | **Network-worlds design** |
| ⚠️ **`ObjectIndex::loadWorldIndex`'s rebuild is still UNLOCKED** — `WorldLock` is not reentrant | **Network-worlds design** |

---

*Last Updated: 2026-08-23 (**SP-118 ✅ CLOSED, user-approved.** Five Tasks + thirteen Issues Verified and
archived in the same step. ⚠️ **Thirteen Issues, none found by any suite** — the strongest evidence yet
that the live click-through is not optional. ⚠️ **T-0443's deviation RULED IN FAVOUR by the user.**
Suites at close: `ctest` **561/561** arm64 · x86-64 · ASan+UBSan; Linux **565/565** non-root; interop
**120/120**; app **BUILD SUCCEEDED**. Active Sprints 1 → 0; EP-034 stays 🟡 Active, **4 of 8 sprints
closed**.)*
