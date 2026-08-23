# Verified Tasks: T-0441 – T-0445

**Sprint SP-118** · EP-034 `[Cross]` Object Detail & Media · ✅ **All five Verified 2026-08-23
(user-approved).**

SP-118's subject: make the Detail Sheet show **what an object is connected to**, let the writer **create
those connections from it**, and **repair the vocabulary that stopped eight of ten kinds from connecting at
all in existing projects.**

---

## T-0441 — ⚠️ T-0416: reconcile seeded relation types on open `[ScriviCore]`

**Verified 2026-08-23.** ⚠️ **Complete only together with I-0149** — see below.

`RelationTypeStore::load` re-seeded **only** when the file was missing or unparseable, so a valid file was
taken verbatim and a seed change never reached an existing project. I-0125 removed `appears-in`'s
`sourceKind: character` on 2026-08-17 and fixed **new projects only**; every project created before that
kept the old vocabulary permanently — and `appears-in` is the type **eight of the ten object cards** use.

**Implemented:** for each code in `seedTypes()` — missing → added, differing → replaced with the seed.
⚠️ **Writer-authored codes are never touched and nothing is ever deleted.** ⚠️ **Written only when
reconciliation actually changed something** (S3), so a current project is not rewritten on every open.

⚠️ **Accepted consequence, user-ruled 2026-08-21:** a seeded type the writer deliberately edited **is
overwritten**. Chosen over leaving a hand-edited `appears-in` broken forever with no explanation. ✅ **A
test asserts this cost explicitly**, so it is visible in the suite rather than discovered by a writer whose
relabelled `cites` quietly reverted.

**Tested against a DELIBERATELY DRIFTED fixture**, because a fresh project is already correct and a
normally-seeded test passes vacuously — ⚠️ **exactly how this defect survived from 2026-08-17 with a green
suite.** ✅ **Negative control run:** with reconciliation disabled, both the label assertion **and**
`REQUIRE(edge.ok())` — the writer-facing symptom — fail.

> ⚠️ **I-0149 is inseparable from this Task.** The ruling said *reconcile ON OPEN*; the code reconciled
> **on read**, so a drifted project opened and closed untouched. **"On open" is an event, not a function.**

---

## T-0442 — R3: the related-objects section `[Apple]`

**Verified 2026-08-23.**

Edges grouped by the label as it reads **from this endpoint**. ⚠️ **`EdgeView.label` is READ, never
recomputed** — one stored edge renders as `forwardLabel` from one end and `inverseLabel` from the other
(Doc 1 §5.2), and recomputing it in Swift would be the restated-rule defect this Epic has paid for nine
times. `isForward` groups; it never picks a label.

✅ **Confirmed by use:** from Myton every relation read *"appears in"*, never *"features"*.

⚠️ **"The Lantern Foxes" was checked and closed as CORRECT** — the stored edge is
`chronicle --appears-in--> scene`, so *"appears in"* from the chronicle's end and *"features"* from the
scene's end are the same edge read from opposite endpoints.

---

## T-0443 — D4-A: inline relationship creation `[Apple]`

**Verified 2026-08-23.** ⚠️ **The plan's letter was deviated from, and the user RULED IN FAVOUR of the
deviation: *"Keep the second picker, that was the right call."***

The plan said *"reuse `ObjectPickerView`, do not write a second picker."* ⚠️ **It could not carry the
object→object case without becoming mode-conditional throughout**: it is bound to one `ObjectCardKind` and
one **scene** target — it titles itself "Add Characters", filters to a single kind, feeds a *card's* draft
state, and picks its relation type by asking which types accept `kind → scene`. Object→object inverts all
four.

**`ObjectRelationPicker`** reuses the reusable parts in shape — AC17's unfiltered all-worlds listing, world
grouping, and ⚠️ **the `minHeight` I-0127 paid for** — while `ObjectPickerView` keeps its scene filter, now
named `targetAcceptsScene` rather than an inline `== "scene"`.

✅ **No ABI change**, as the plan predicted: scenes are not an `ObjectKind`, so a constrained scene endpoint
already crosses as the sentinel `"scene"`. Spelled **once**, as `RelationTypeEntry.sceneToken`, with
`targetAcceptsScene` / `targetAcceptsObject` / `sourceIsScene` derived from it — ⚠️ **the two target
properties OVERLAP rather than negating**, because an unconstrained type accepts both.

✅ **Confirmed by use:** the picker offered `sibling-of` and `cites`, correctly excluded `located-at`
(scene→location, cannot start from an object), listed Eskandar objects, disabled non-accepted kinds per
type, and reported "Already Related" for an existing pair.

---

## T-0444 — R5/R7: push-navigation from the related list `[Apple]`

**Verified 2026-08-23.**

Double-click **and** right-click → **"Show"** (⚠️ **not "View Detail"** — R7 words the two entry points
differently on purpose, and the sheet is already open here). `ObjectDetailHistory.visit()` truncates
forward history and no-ops on a re-visit.

⚠️ **Four Issues were needed to make this actually work:** I-0151 (world-scoped navigation failed
outright), I-0153 (scene rows did nothing), I-0157 (navigator selection left behind), I-0161 (navigator did
not reveal the target).

---

## T-0445 — Pending far-endpoints `[Apple]`

**Verified 2026-08-23.**

A related object whose world is away is **listed, named, and explained**, and cannot be edited or opened.
⚠️ **`otherKind` travels on the edge** (I-0124) precisely because a pending object is absent from the
index — never attributed by index lookup.

✅ **Confirmed by use with the drive physically ejected:** rows stayed, each read *"Held pending"*,
"Relate…" was disabled, "Show" did nothing, **nothing disappeared** — and everything returned to normal on
remount.

---

## Suites at verification

`ctest` **561/561** (arm64 · x86-64 · ASan+UBSan) · Linux **565/565** (GCC 13, non-root) · interop
**120/120** · app **BUILD SUCCEEDED**.

---

## ⚠️ What this Task set actually cost, recorded honestly

**Thirteen Issues** (I-0149 – I-0161) were raised against these five Tasks **after** the suites went green.
⚠️ **Every one was found by the user's live click-through; none by any suite.**

⚠️ **Four were the same failure — an existing correct pattern the new code did not follow:**

| Issue | The rule already in the repo |
| ----- | --------------------------- |
| I-0152 | the Navigator names an untitled scene "Scene N", never an ID |
| I-0155 | `ObjectCardModel.rename` re-reads before patching |
| I-0157 | I-0132 ruled `selectedSceneID` the source of truth |
| I-0158 | `SceneNavigatorView` uses `List(selection:)` |

⚠️ **I-0155 was silent data loss** — a saved note reverting a saved rename — **and was reported as "maybe
not a defect."**

⚠️ **I-0150 is not about these Tasks at all but was found while verifying them:** `xcodebuild test`
launches the hosted app, which reopened the user's real projects and wrote to them.

---

*Last Updated: 2026-08-23 (**T-0441 – T-0445 ✅ VERIFIED, user-approved, and ARCHIVED in the same step** at
SP-118 close. ⚠️ **T-0443's deviation was ruled IN FAVOUR by the user.** ⚠️ **T-0441 is complete only with
I-0149.**)*
