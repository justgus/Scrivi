# Sprint SP-115 — CLOSED

## SP-115: EP-034 — the five open Issues

**Status:** ✅ **CLOSED 2026-08-20 (user-approved)**
**Epic:** [EP-034](../Epics/Epic-active.md) `[Cross]` Object Detail & Media — Detail Sheets &
Project↔World Interaction
**Goal:** Resolve the five Issues EP-031 filed and deliberately did not fix (I-0135–I-0139), and file the
two kind-list findings the design pass surfaced.
**Start Date:** 2026-08-20
**End Date:** 2026-08-20
**Capacity:** one day (planned, activated, implemented, verified and closed 2026-08-20)

## ✅ COMPLETE — all seven Tasks and six Issues ✅ Verified by the user 2026-08-20

**All six Tasks implemented.** Suites:

| Suite | Result |
| ----- | ------ |
| `ctest` (macOS arm64) | ✅ **525/525** — was 520; **+5 new** |
| macOS interop | ✅ **103/103 in 10 suites** — was 99; **+4 new** |
| `ScriviApp` build | ✅ **BUILD SUCCEEDED** |

### ⚠️ One defect found by the USER, not by a suite (I-0142)

During verification the user reported: *"When the Edit Object popup surfaces, the world that the Object
belongs in is not selected."* ⚠️ **Three layers all gated `worldID` on `pending`**, so a reachable world
object crossed the boundary with no world attributed.

> ⚠️ **The visible symptom was the lesser half.** `rename()` passes that same `worldID` to `openObject`,
> which needs it to locate a world-scoped object's file — **so renaming ANY world object was failing.**
> The empty picker was the tell; the broken rename was the cost.

✅ **User ruling, taken as scope-limiting:** an object **cannot be moved between worlds**. The control is a
**label**, and the picker survives only for a genuinely unassigned object. The questions a move would
raise — migrate related objects? allow cross-world edges? delete them? remap to equivalents? — are
⚠️ **deliberately not opened here.**

⚠️ **The regression test was confirmed to FAIL against the un-fixed core** before being kept, so it
genuinely pins the defect rather than merely passing.

✅ **ALL VERIFIED BY THE USER 2026-08-20**, including ⚠️ **I-0137 on the REAL RIG** — `tintagael` +
`Eskandar` on USB, **drive ejected** — the check a passing suite genuinely cannot substitute for.

⚠️ **Tasks and Issues were ARCHIVED IN THE SAME STEP** (`feedback_archive_on_close`):
[`../Tasks/Verified/Task-verified-0419-0425.md`](../Tasks/Verified/Task-verified-0419-0425.md) ·
[`../Issues/Verified/Issue-verified-0131-0140.md`](../Issues/Verified/Issue-verified-0131-0140.md) ·
[`../Issues/Verified/Issue-verified-0141-0150.md`](../Issues/Verified/Issue-verified-0141-0150.md)

### ⚠️ Two things carried OUT of this sprint — do not read as delivered

| Item | Owed to |
| ---- | ------- |
| **T-0420 has NO writer-facing surface.** The core refuses a too-new world correctly, but a writer sees *"unavailable"* with **no explanation** — `unsupportedWorldFormatVersion` reaches nothing. ⚠️ **`project_capability_without_surface`, shipped by the very sprint that fixed four instances of it** | unassigned |
| **I-0140 + I-0141** — filed by T-0424, **fixed by neither**, by design | **SP-116** (design-doc **D5**) |

✅ **Closed 2026-08-20 with user approval.**

### What changed

| Task | Change |
| ---- | ------ |
| **T-0419** | `WorldResolution`/`WorldSummary` gain **`lastKnownPackagePath`** — carried regardless of status; `packagePath` keeps meaning *verified*. Wired through both C ABI envelopes and both Swift structs. ⚠️ **Found + fixed a second gap:** `WorldStatusResult.worldStatus` returned the **raw** core status while its sibling refined |
| T-0420 | `parseWorld` refuses `formatVersion > kSupportedFormatVersion` with **`unsupportedVersion`** + detail `unsupportedWorldFormatVersion`; ⚠️ `resolve` reports such a world **`unavailable`, never `missing`** |
| T-0421 | Exit button named for what it **actually does** — `Done`/`Cancel` when nothing changed, `Revert`/`Discard` (destructive) only when it is. Added `ObjectDraft.hasUnsavedChanges` |
| T-0422 | Test: a corrupt `world.json` → **`unavailable`, not `missing`**, and the file is **neither regenerated nor deleted** |
| T-0423 | Disabled remove button now explains **why** via `pendingHelp`; wording *"Remove from scene"* kept (AC22) |
| T-0424 | ⚠️ **FILED I-0140 + I-0141** — not fixed, as scoped |
| **T-0425** | ⚠️ **Added after the user's verification pass.** `worldID` now reported whenever an endpoint has one — `EndpointResolver`, `RelationshipStore`, C ABI, and the app. World shown as a **label**; ✅ **moving between worlds disallowed** by user ruling |

---

> ⚠️ **THIS SPRINT FIXES THOSE FIVE AND NOTHING ELSE** — user ruling 2026-08-20. **No Detail Sheet work.**
> The Detail Sheet begins at **SP-116**. A fix sprint that grows a feature stops being verifiable as
> either.

---

### Assigned Tasks

| ID | Title | Issue | Priority | Status |
| -- | ----- | ----- | -------- | ------ |
| **T-0419** | ⚠️ **`lastKnownPackagePath`** — carry the last-known candidate so AC24's refinement can fire | **I-0137** | **High** | ✅ **Verified** |
| T-0420 | `world.json` `formatVersion` compared against a supported maximum | I-0136 | Medium | ✅ **Verified** |
| T-0421 | Inline object editor — a clear exit that does not read as data loss | I-0139 | Medium | ✅ **Verified** |
| T-0422 | Corrupt `world.json` test coverage | I-0135 | Low | ✅ **Verified** |
| T-0423 | "Remove from scene" — disabled **and explained** | I-0138 | Low | ✅ **Verified** |
| **T-0424** | ⚠️ **FILE (do not fix) the two kind-list findings** as Issues | — | Medium | ✅ **Verified** |
| **T-0425** | ⚠️ **Object editor shows its object's OWN world, as a LABEL** — and **rename of a world object was broken** | **I-0142** | **High** | ✅ **Verified** |

### Assigned Issues

| ID | Sev | What | Approach |
| -- | --- | ---- | -------- |
| **I-0137** | **High** | AC24's world-status refinement **cannot fire on real hardware** — `packagePath` is empty for exactly the worlds it must diagnose | ✅ **D9 = A** |
| I-0136 | Medium | `formatVersion` read but **never compared** | Compare against a supported max |
| I-0139 | Medium | Title click opens an editor whose only exit reads "Revert" | ✅ **Q-b = patch the control** |
| I-0135 | Low | No coverage for a **corrupt** `world.json` | Add the test |
| I-0138 | Low | "Remove from scene" **disabled but unexplained** | Apply `pendingHelp` to the right control |

---

### Task detail

#### T-0419 — ⚠️ `lastKnownPackagePath` (I-0137, **High**)

**Root cause, confirmed at source:** `WorldStore::resolve` sets `out.packagePath` at **exactly one line**
(`WorldStore.cpp:303`), on the success branch. `listWorlds` then copies it **only** when
`status == available` (`WorldStore.cpp:385-390`). So the field is **never produced** on failure — this is
not a value being dropped, it is a value never created. `WorldVolumeStatus.refine` opens
`guard !packagePath.isEmpty` (`ScriviEngineGraph.swift:445`) and therefore **cannot fire for the one case
it exists for.**

**Approach (D9 = A):** carry the last-known candidate in a **new, distinctly named** field
(`lastKnownPackagePath`). The candidate (`cand`) is already in scope at every failure point of the resolve
loop, so this is small.

⚠️ **`packagePath` KEEPS its meaning — "verified" — and is NOT widened.** The distinct name is the
safeguard, not a style choice. `resolve` deliberately refuses to report a path it could not verify, and
that discipline (*"NEVER GUESS"*, `WorldStore.cpp:270-310`) is the same one I-0115 established.

**Definition of done:**
- [ ] New field plumbed: `resolve` → `listWorlds` → C ABI envelope → Swift
- [ ] `refine` reads it; `packagePath` semantics unchanged
- [ ] ⚠️ Test written against **`scrivi_*`**, not the C++ facade
- [ ] ⚠️ **Verified on the REAL RIG** — `tintagael` + `Eskandar` on USB, **drive ejected**, showing
      `unmounted`/`offline` where it previously said only *"unavailable"*

⚠️ **A fixture that supplies `packagePath` PASSES while the real rig cannot. That is precisely how this
defect reached a Verified AC.** Fixture evidence alone does not close this Task.

#### T-0420 — `formatVersion` comparison (I-0136, Medium)

**Confirmed:** `WorldJson.cpp:41` reads `formatVersion` into the record and
`grep -rn "formatVersion >" ScriviCore/src/` returns **nothing** — no code anywhere validates it. A world
package written by a **future** Scrivi is parsed as if current, with unknown fields silently dropped.

⚠️ **This is the one class of defect that cannot be retrofitted** — once a newer world file exists in the
wild, the old readers that mis-parsed it have already shipped. And a world package is **shared between
projects and carried across machines**, which is exactly where version skew occurs.

⚠️ **EP-034 raises the stakes:** D6 makes this Epic **write** to world packages for the first time.

**Definition of done:**
- [ ] Supported maximum defined and compared; unsupported-newer refused honestly (§6.16 does this for
      *project* files — mirror its behaviour, do not invent a second idiom)
- [ ] ⚠️ **Refusal must not read as "missing" or invite a destructive remedy**
- [ ] Test at the boundary

#### T-0421 — Inline editor exit (I-0139, Medium)

⚠️ **Q-b ruled 2026-08-20: the Detail Sheet does NOT replace the inline editor.** The editor and the list
item both remain; the sheet is opened *from* them, on request. **So this is a real fix, not a disposition.**

**The defect:** a single click on a title enters an edit mode whose only exit is labelled **"Revert"**
(`ObjectCard.swift:674-679`). The exit exists — `onDiscard` sets `draft = nil` (`:415`) and Revert is never
disabled — but **a destructive-styled label does not read as "stop editing."**

**Definition of done:**
- [ ] A clear, non-destructive-reading way out of the editor
- [ ] ⚠️ **Do NOT make the panel modal** — §4.6 forbids it
- [ ] ⚠️ **Do NOT add a Cancel that bypasses the unfinished-work prompt** — that route is how **I-0119**
      filed an object into the wrong scene
- [ ] ⚠️ **Do NOT "solve" it by removing the inline editor** — Q-b keeps it
- [ ] A single click on a title still does what it does today

#### T-0422 — Corrupt `world.json` coverage (I-0135, Low)

The `unavailable` fallback is covered generically (`WorldTests.cpp:234`) but **never for this cause**.
Behaviour appears correct — `parseWorld` validates the schema tag and rejects an empty `worldID`
(`WorldJson.cpp:31,43-46`), resolution continues to the next candidate, then degrades.

⚠️ **What the test must prove:** a corrupt world file is **not** auto-regenerated, **not** deleted, and
**not** reported as `missing`. A corrupt file is **evidence the package EXISTS**, so reporting `missing`
would be exactly the guess `WorldTests.cpp:277` exists to prevent.

**Definition of done:**
- [ ] Test for a corrupt/unparseable `world.json`
- [ ] Asserts `unavailable` (not `missing`), and that the file is left untouched

#### T-0423 — Disabled **and explained** (I-0138, Low)

**Confirmed:** `ObjectCard.swift:829` sets a **static** `.help("Remove from scene")`, and `:833` disables
the button when `entry.pending`. The comment two lines above claims the affordance is *"disabled and
explained"* — ⚠️ **it is disabled and UNexplained.** `pendingHelp` (`:838`) already composes the right
sentence — *"Held pending — this object's world is …"* — and is applied to a **different** control (`:800`).

**Definition of done:**
- [ ] The disabled remove button explains **why** on hover (§7.2: disabled-and-explained)
- [ ] Accessibility label conveys the same
- [ ] ⚠️ Keep the wording *"Remove from scene"* — **AC22: the edge goes, the object stays.** "Delete" would
      misdescribe it and scare a writer off a non-destructive action

#### T-0424 — ⚠️ FILE the two kind-list findings (do NOT fix)

Surfaced by the design pass (design doc §10). **Occurrence EIGHT** of the restated-kind-list class.

| Finding | Where |
| ------- | ----- |
| Swift restates the world-scope rule — `var isWorldScoped: Bool { kind != "source" }` | `ObjectCard.swift:46` |
| Header comment **stale since SP-103** — names 4 of 10 world-scoped kinds | `scrivi.h:97-99` |

⚠️ **File them as Issues; do NOT fix them here.** Both are cured by **D5's kind-scope endpoint in SP-116**,
and fixing them inside a five-Issue sprint blurs its boundary. ⚠️ **The cause is structural, not careless:**
no ABI endpoint exposes kind scope, so Swift has nothing to derive from.

**Definition of done:**
- [ ] Two Issues filed (**I-0140**, **I-0141**) with file:line references and the D5 pointer
- [ ] `Issue-active.md`, `Issue-Documentation.md` and the next-available ID updated

---

### Sprint Notes

⚠️ **Verify each Issue against ITS OWN trigger** (`feedback_verify_each_half_separately`) — I-0132 was
wrongly credited once already. **Five Issues means five separate verifications**, not one pass that
"looks right."

⚠️ **I-0137 must be verified on the REAL RIG**, drive ejected. See T-0419.

⚠️ **Test boundary changes against `scrivi_*`, never the C++ facade**
(`feedback_boundary_tests_not_facade`) — a facade test cannot see a boundary gap. That is how I-0113
shipped green.

⚠️ **Check Scrivi is not running before `xcodebuild test`**
(`feedback_check_scrivi_running_before_tests`) — a live instance blocks the runner with a LaunchServices
error. `pgrep` first.

⚠️ **Real writing work lives on the test rig** (`project_test_rig_tintagael_eskandar`) — **back it up
before the ejection test.**

**Suites expected at close:** `ctest` green macOS arm64 **and** x86-64 + sanitizers · macOS interop green ·
Linux container green · app **BUILD SUCCEEDED**.

### Retrospective

*(Completed at close.)*

---

---

## Retrospective

### Completed

**All seven Tasks ✅ Verified, all six Issues ✅ Verified** — the five EP-031 carried Issues plus **I-0142**,
which did not exist when this sprint was planned.

| Task | Issue | Outcome |
| ---- | ----- | ------- |
| T-0419 | I-0137 | ✅ Verified ⚠️ **on the real rig, drive ejected** |
| T-0420 | I-0136 | ✅ Verified ⚠️ **at the CORE ONLY — surface owed** |
| T-0421 | I-0139 | ✅ Verified |
| T-0422 | I-0135 | ✅ Verified |
| T-0423 | I-0138 | ✅ Verified |
| T-0424 | — | ✅ Verified — **filed** I-0140 + I-0141, fixed neither |
| T-0425 | I-0142 | ✅ Verified — ⚠️ **added mid-sprint; user-found** |

**Suites at close:** `ctest` **525/525** (was 520 at open; **+5**) · macOS interop **103/103 in 10 suites**
(was 99; **+4**) · app **BUILD SUCCEEDED**.

### Returned to backlog

**Nothing.** No item was returned; scope grew by exactly one Task (T-0425), for a defect found during
verification.

### ⚠️ Carried out — do NOT read as delivered

| Item | Owner |
| ---- | ----- |
| **T-0420 has no writer-facing surface.** A writer opening a too-new world sees *"unavailable"* with **no explanation**; `unsupportedWorldFormatVersion` reaches nothing | ⚠️ **unassigned — needs an owner** |
| **I-0140 + I-0141** — filed by T-0424, fixed by neither, **by design** | **SP-116** (design-doc **D5**) |

> ⚠️ **A condition without an owner is not a plan** (the SP-074 lesson: *"do not mark Verified until app
> adoption lands"* went unowned for four weeks). **T-0420's missing surface has no owner yet, and that is
> stated here rather than implied.**

### What went well

- **A fix-only sprint stayed a fix-only sprint.** Scope was "the five Issues and nothing else"; the one
  addition (T-0425) was a defect found *while verifying*, not feature creep.
- **T-0424 filed rather than fixed**, as ruled. Both findings went to SP-116 where D5 cures them properly,
  instead of being patched inside a sprint with a different remit.
- **The I-0142 regression test was confirmed to FAIL against the un-fixed core** before being kept — so it
  pins the defect rather than merely passing alongside it.

### ⚠️ What to improve — the lesson this sprint proves twice

> **User, at close:** *"ctest, and unit tests, and integration tests are designed to test specific things.
> But the true user experience can only be tested live, in app."*

**This sprint is that argument's evidence, twice over:**

1. ⚠️ **I-0137 — a suite named after the acceptance criterion PASSED while the feature could not fire.**
   *"World volume status refinement (EP-031 AC24)"* was green. `refine` was correct, unit-tested and
   correctly wired. **The datum it needed never arrived**, because `packagePath` was populated only for
   available worlds — i.e. never for the case the refinement exists for. **Only ejecting a real drive
   proved it.**
2. ⚠️ **I-0142 — the user found in five minutes what 628 automated tests did not.** The visible symptom
   (an empty world picker) was the *lesser* half: **renaming any world-scoped object was failing
   silently**, because `rename()` passes that same `worldID` to `openObject`.

**Practices to carry forward:**

- ⚠️ **A live pass is required evidence, not a courtesy.** EP-031 made T-0418 mandatory for this reason;
  SP-115 confirms it was right.
- ⚠️ **Verify each half against its OWN trigger.** Six Issues meant six separate checks, not one pass that
  "looked right."
- ⚠️ **Test boundary changes against `scrivi_*`, never the facade.** A facade test cannot see a boundary
  gap — how I-0113 shipped green, and structurally how I-0142 hid.
- ⚠️ **A control that implies a capability the code lacks is a defect even when nothing breaks.** The world
  picker offered a move `rename` could never have performed.

### Carry-forward notes for SP-116

- **D5, D6, D7** are the sprint's substance; **D5 also retires I-0140 + I-0141.**
- ⚠️ **`WorldRecord::kSupportedFormatVersion` now exists** (`WorldTypes.hpp`) — **raise it only when the
  build can actually READ the newer shape.**
- ⚠️ **`lastKnownPackagePath` means "where we looked", never "verified".** Do not conflate it with
  `packagePath`; the distinct name is the safeguard.
- **D6 makes world writes real**, which raises I-0136's stakes — and its missing surface with them.


---

*Closed 2026-08-20 with user approval. Tasks → `../../Tasks/Verified/Task-verified-0419-0425.md`; Issues → `../../Issues/Verified/Issue-verified-0131-0140.md` and `../../Issues/Verified/Issue-verified-0141-0150.md`.*
