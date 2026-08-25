# Closed Sprint

## SP-121 — `[Linux]` Complete `ScriviBridge` to full ABI parity + ⚠️ **the Porting Outline** — ✅ **CLOSED**

**Status:** ✅ **CLOSED 2026-08-25** (user-approved) — all six Tasks ✅ **Verified**
**Epic:** [EP-034](../../Epics/Epic-active.md) — `[Cross]` Object Detail & Media · **sprint 7 of 8**
**Codebase:** `[Linux]` — ⚠️ **`ScriviBridge` only. NO UI ships in this sprint** (see §2, and read §3 before objecting)
**Date Activated:** 2026-08-24
**Closes:** ⚠️ **NOT AC11.** AC11 is **split out to a new Epic** by user ruling — see §1.
**Tasks:** **T-0460 – T-0465** (six) — ✅ **all Verified**, archived to [`../../Tasks/Verified/Task-verified-0460-0465.md`](../../Tasks/Verified/Task-verified-0460-0465.md) · **Next available:** T-0466 · Issue **I-0171**

---

## 1. ⚠️ The finding that split AC11 in half

**AC11 read `[Linux]` parity for AC1–AC9.** ⚠️ **That is not a parity sprint. There is nothing to reach
parity with — the Linux app has NO object layer at all.** Verified at planning, against the code:

| Check | Result |
| ----- | ------ |
| `scrivi_create_object` / `list_objects` / `open_object` / `save_object` on Linux | ⚠️ **ZERO call sites** |
| `scrivi_create_edge` / `delete_edge` / `list_edges_for` | ⚠️ **ZERO** |
| `scrivi_list_worlds` / `add_world` / `get_world_status` | ⚠️ **ZERO** |
| `scrivi_import_asset` / `list_assets` | ⚠️ **ZERO** |
| `SceneInspector.cpp` | ⚠️ **67 lines, and a STUB** — its own header says *"wired to NO project data… a title, a muted 'No entities yet.' empty state, and a DISABLED 'Add Entity' button."* Unchanged since EP-024/SP-078 |
| Apple surface AC1–AC9 covers | **~4,800 lines across 11 files** |

⚠️ **AC1 presumes a Kind Card list item to double-click. Linux has no Kind Cards.** Every AC from AC1 to
AC9 rests on an object layer that was never built there.

⚠️ **This is NOT "Linux lagging."** Linux has a real Timeline Panel (1,422 lines), manuscript editor,
navigator tree, story structures. **The object model simply landed in EP-031/EP-034 as Apple-only, and no
Linux sprint ever picked it up.**

### ✅ User ruling (2026-08-24)

> *"This is its own epic. In SP-121 we should focus solely on making sure the Linux version of
> ScriviEngine is complete with all the functionality we've invented. Then we'll focus an entire Epic on
> porting the App functionality into the Linux version. We should also mark all the steps that that will
> take and create a **porting outline**, because we're going to have to do it all again for iPad, iPhone,
> Windows and eventually visionOS."*

**Three consequences, all binding:**

1. **SP-121 = the bridge, and nothing else.** `ScriviBridge` reaches full ABI parity with `ScriviEngine`.
2. ⚠️ **AC11 leaves EP-034** → **EP-035**, a new Epic for the Linux UI port. EP-034 closes on AC12.
3. ⚠️ **The Porting Outline is a FIRST-CLASS DELIVERABLE, not documentation of the work** — it is written
   to be run **four more times** (iPad, iPhone, Windows, visionOS).

---

## 2. Sprint Goal

**`ScriviBridge` calls every endpoint `ScriviEngine` calls, with the same envelope handling and the same
error semantics** — so that EP-035 is a UI port and never a bridge-archaeology exercise.

⚠️ **Measured, not estimated.** `scrivi.h` exposes **100** endpoints (⚠️ **corrected from 102 — the
planning figure counted prose and `scrivi_free`; re-verified 2026-08-25 against the header**). Apple's layer reaches **96** (⚠️ **measured 2026-08-25, not inferred**);
`ScriviBridge` reached **34** at sprint start. ⚠️ **66 endpoints were missing**, in twelve clusters
(⚠️ **the planning figures were 35 / 62; T-0460's audit corrected them to 34 / 66**):

| Cluster | Missing | Cluster | Missing |
| ------- | ------- | ------- | ------- |
| **history** | **15** | world | 7 |
| object | 8 | edge | 4 |
| scene (notes/tags/todo/outline/story-time) | 8 | buffers | 4 |
| fragment | 4 | asset | 3 |
| comment | 3 | inbox | 2 |
| snapshot | 2 | repair | 1 |

⚠️ **The cluster rows above sum to 62 — the PRE-AUDIT figures, left as the planning record.** The audited
totals are **66 missing = 47 in scope + 19 deferred**; the four-endpoint difference is what planning
missed by grepping prose rather than declarations. ✅ **Post-sprint state, verified by re-running the
audit: Linux 34 → 81 of 100, with exactly the 19 history/buffers endpoints outstanding.**

---

## 3. ✅ USER RULING (2026-08-24) — the disparity is PLANNED, and this is the record of it

> *"We are aware of the disparity. However, we are planning for that. We'll verify the backend with the
> ctests and the subsequent Epic will manage the surfacing of the capability in the Linux App."*

⚠️ **This ruling is what makes SP-121 legitimate, and it must travel with the sprint** — otherwise a later
reader finds 43 bridged endpoints with no caller and correctly files it as
`project_capability_without_surface`.

**Three things it settles:**

1. ✅ **The gap between bridge and UI is DELIBERATE and staged** — not an oversight, not drift.
2. ✅ **`ctest` is the verification standard for this sprint.** ⚠️ **There is no live click-through and
   none is owed**, because no writer-facing surface ships. **T-0464 is the acceptance evidence.**
3. ✅ **EP-035 owns the surfacing**, and is already open in the Epic backlog with AC11 moved into it — so
   the consumer is **scheduled, not merely intended.**

⚠️ **What this ruling does NOT do:** it does not make the endpoints proven-by-use. ⚠️ **Until EP-035 runs,
every one of the 43 is exercised only by tests written alongside it** — which is exactly the evidence
class that has never once caught this project's real defects (22 consecutive Issues from clicking, none
from a suite). **That is an accepted, named cost of staging — not a claim that tests are sufficient.**

---

## 3a. ⚠️ Why this is NOT the `capability_without_surface` defect

**It looks like the defect this project has paid for eight times. It is not, and the distinction matters.**

**It looks like the defect this project has paid for eight times. It is not, and the distinction matters.**

✅ **`feedback_design_to_capability_not_lcd` already ruled core-before-surface CORRECT.** ScriviCore is the
capability layer; each platform is one surface over it. ⚠️ **The defect is a DANGLING READ WITH NO
READER** — a field the core emits that nothing will ever consume.

⚠️ **These 62 endpoints have a named consumer, a named Epic, and a written plan: EP-035, whose outline
this sprint also delivers.** That is the difference between *staging work* and *stranding it*.

⚠️ **The risk is real and is named here so it can be checked at close:** if EP-035 never runs, this sprint
has produced 43 unreached functions. ✅ **Mitigations, binding:** **T-0464's tests exercise every endpoint
through the bridge**, so nothing merges unproven; ⚠️ **the Porting Outline names EP-035's first sprint
explicitly**; and ✅ **EP-035 is already OPEN in the Epic backlog carrying AC11** — the consumer exists as
a tracked commitment before the capability ships, which is the inversion of the defect.

---

## 4. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0460** | ⚠️ **The gap audit, MECHANICAL** — enumerate all **100** endpoints; each **present / missing / deliberately N-A**, with reasons | **High** | 🟠 **Implemented — Not Verified** |
| **T-0461** | **Objects + kinds + worlds** — 8 + 7 + 1 endpoints (the EP-034 core) | **High** | 🟠 **Implemented — Not Verified** |
| **T-0462** | **Edges + relation types + assets** — 4 + 1 + 3 | **High** | 🟠 **Implemented — Not Verified** |
| **T-0463** | **Scene metadata, comments, inbox, repair, snapshots** — the long tail | **Medium** | 🟠 **Implemented — Not Verified** |
| **T-0464** | ⚠️ **Bridge tests — EVERY endpoint exercised**, ⚠️ **through `ScriviBridge`, never the C ABI directly** | **High** | 🟠 **Implemented — Not Verified** |
| **T-0465** | ⚠️ **THE PORTING OUTLINE** — the reusable deliverable; ⚠️ **written for FIVE platforms, not one** | **High** | 🟠 **Implemented — Not Verified** |

### ⚠️ Deliberately NOT in this sprint

**`history` (15) and `buffers` (4) — 19 of the 62 — are EXCLUDED.**

⚠️ **They belong to EP-019's undo/redo engine, not to the object model**, and EP-019's Linux story has
never been ruled. Pulling 19 history endpoints into an EP-034 sprint would set that direction **by
accident**, which is precisely how I-0144's locking model nearly got set inside an asset sprint.
✅ **T-0460 records them as a known, named exclusion with an owner: EP-019.**

**So SP-121 delivers 47 of 66** (⚠️ **corrected from "43 of 62" by T-0460's audit**), and ⚠️ **says so plainly rather than reporting "full parity" against a
number quietly redefined.**

### T-0460 — The gap audit (lands FIRST)

⚠️ **Mechanical and read-only** — greps and counts, in the spirit of the Audit Check. Produces a table of
**all 100** endpoints: present in the bridge / missing / **deliberately N-A**.

⚠️ **"Deliberately N-A" must carry a reason.** Apple reaches **96 of 100** — ⚠️ **the FOUR it does not reach
(⚠️ **not two, as the plan assumed**: `resolve_timeline_project_times`, `set_timeline_epoch_offset`,
`set_world_epoch_offset`, `upsert_relation_type`)
are themselves a finding** and must be identified, not rounded away.

### T-0461 — Objects, kinds, worlds

`create_object`, `open_object`, `save_object`, `delete_object`, `list_objects`, `promote_object`,
`list_orphaned_objects`, `list_object_kinds` · `list_worlds`, `add_world`, `create_world`,
`get_world_binding`, `get_world_status`, `relink_world`, `remove_world_reference`.

⚠️ **`list_object_kinds` is D5's endpoint and is load-bearing**: it is how a platform DERIVES which kinds
are world-scoped. ⚠️ **A Qt restatement of that partition would be occurrence NINE** of this project's
most-repeated defect — and **occurrence five was in Swift**, which is the proof it is not a C++ concern.

⚠️ **`worldID` must be threaded through every object call**, exactly as the Swift engine does. Omitting it
is how SP-104 blocked object creation outright.

### T-0462 — Edges, relation types, assets

`create_edge`, `delete_edge`, `list_edges_for`, `list_pending_edges` · `list_relation_types` ·
`import_asset`, `list_assets`, `remove_asset`.

⚠️ **`list_edges_for` returns the label ALREADY RESOLVED for the queried endpoint.** The bridge must pass
it through untouched — ⚠️ **recomputing direction in Qt would be a local copy of the rule the core owns**,
which is the same class as the kind-list defect and has already cost this Epic nine occurrences.

⚠️ **`list_pending_edges` has ZERO call sites on Apple too** — a real dangling read. ✅ **Bridge it, and
record it in T-0460 as a known no-reader**, so EP-035 inherits the fact rather than rediscovering it.

### T-0463 — The long tail

`get_scene_notes`, `set_scene_notes`-adjacent setters (`set_scene_tags`, `set_scene_todo`,
`set_scene_outline`), `clear_scene_story_time`, `set_timeline_epoch_label` · `add_comment`,
`list_comments`, `resolve_comment` · `list_inbox`, `import_from_inbox` · `apply_repair`,
`scan_for_external_changes` · `create_snapshot`, `enable_git_snapshots` · `extract_searchable_text`.

⚠️ **`extract_searchable_text`'s caller list is a KNOWN DRIFT SITE** (I-0118, which silently cost Spotlight
reach). Bridging it here is fine; ⚠️ **assuming a Linux consumer exists is not** — note it.

### T-0464 — ⚠️ Bridge tests, every endpoint

⚠️ **Through `ScriviBridge`, never `scrivi_*` directly.** `feedback_boundary_tests_not_facade` — ⚠️ **a
facade test cannot see a boundary gap, and that is exactly how I-0113 shipped green.**

Each endpoint: one happy path + one error envelope, asserting the `{"ok"}/{"error"}` contract and
`scrivi_free` on every returned pointer.

⚠️ **Run in the container as NON-ROOT, in a second image built with `SCRIVI_BUILD_TESTS=ON`.** The
shipping Dockerfile builds tests **OFF**, so ⚠️ **"the Linux container is green" has never meant `ctest`
ran** (`project_linux_container_tests_off`).

### T-0465 — ⚠️ THE PORTING OUTLINE

**`docs/Scrivi_Platform_Porting_Outline_v0_1.md`** — ⚠️ **the deliverable with the longest life in this
sprint.**

> *"We're going to have to do it all again for iPad, iPhone, Windows and eventually visionOS."*

⚠️ **Written as a REUSABLE PROCEDURE, not a Linux retrospective.** It must state, for any new platform:

1. **The layer order** — ABI binding → data models → read-only surfaces → editing → navigation. ⚠️ **And
   WHY that order**: EP-034 proved a surface built before its typed model reconstructs objects and drops
   fields (the patch-don't-reconstruct rule, T-0436/T-0437).
2. **The 100-endpoint checklist**, with per-platform columns.
3. ⚠️ **The rules that are PLATFORM-INDEPENDENT and must be re-honoured every time** — derive kind scope,
   never restate it; read edge labels, never recompute them; patch objects, never reconstruct them;
   absence is never deletion; disabled AND explained; ⚠️ **no gesture-only affordance** (VNC carries no
   Shift-combos or trackpad gestures — `project_linux_vnc_input_constraints`).
4. ⚠️ **The defects each platform will re-earn if it does not.** ⚠️ **Occurrence five of the kind-list
   defect was in SWIFT, not C++** — a new platform layer is exactly where it recurs.
5. **What is genuinely per-platform** (idiom, layout, input) versus **what is a re-decision of a settled
   ruling** — ⚠️ **which a port must NOT reopen by accident.**

⚠️ **It also names EP-035's first sprint**, so the Linux UI port is scheduled work and not an intention —
the §3 mitigation.

### ⚠️ Execution status — 2026-08-24

| Evidence | Result |
| -------- | ------ |
| **T-0460 gap audit** | ✅ `docs/Scrivi_ABI_Binding_Gap_Audit_v0_1.md` — all **100** endpoints, each with a disposition |
| ⚠️ **Scope CORRECTED by the audit** | ⚠️ **The plan said 43 in scope; the real number is 47.** Planning grepped prose as well as declarations (`scrivi_c_api`, `scrivi_buffers_`). ✅ **This is why T-0460 lands first** — a 9% under-estimate would have surfaced as "scope creep" at close |
| **Endpoints bridged** | ✅ **Linux 34 → 81 of 100** — ⚠️ **verified by re-running the audit**, not asserted. The only 19 missing are the history/buffers set deferred to EP-019; **nothing else** |
| **Linux container build** | ✅ **BUILD SUCCEEDED** — ⚠️ **zero compiler diagnostics** (the 19 "warning" log hits are apt package noise) |
| **T-0464 bridge parity smoke** | ✅ **28 checks, 0 failures**, ⚠️ **run NON-ROOT** |
| **All Linux smoke tests** | ✅ **17/17 passed** — no regressions |
| ⚠️ **`ctest` ON LINUX** | ✅ **571 test cases, 9,439 assertions, ALL PASSED**, non-root — ⚠️ **the leg SP-116 could never run** |
| `ctest` macOS | ✅ **567/567** unchanged (⚠️ this sprint touches no ScriviCore code) |
| **T-0465 Porting Outline** | ✅ `docs/Scrivi_Platform_Porting_Outline_v0_1.md` — written for **five** platforms |

⚠️ **macOS 567 vs Linux 571 is EXPECTED and was checked, not assumed**: the four extras are the
**Linux-only `EncryptedFileSecureStore` tests** (SP-059/T-0229). Apple's `KeychainSecureStore` was deleted,
so it has no counterpart.

### ⚠️ A latent defect found and fixed on the way — the container build was FRAGILE

⚠️ **The repo had NO `.dockerignore`, and the Dockerfile does `COPY . /src`.** A local macOS `build/`
directory therefore landed in the image, carrying a `CMakeCache.txt` that records the **host** path:

> *"The current CMakeCache.txt directory /src/build/CMakeCache.txt is different than the directory
> /Users/.../build where CMakeCache.txt was created."*

⚠️ **It succeeded under `--no-cache` and FAILED on a cached rebuild** — so anyone who had run `ctest`
locally poisoned their next container build, and ⚠️ **the error named CMake rather than the real cause.**
✅ **Fixed by adding `.dockerignore`.** ⚠️ **Pre-existing, not introduced here** — but it is plausibly why
the Linux leg has been skipped before.

---

## 5. Definition of Done

- [x] ✅ **T-0460 – T-0465** all 🟠 **Implemented — Not Verified**
- [x] ✅ **T-0460's audit covers all 100 endpoints** (⚠️ **not 102 — the plan's figure was wrong**) with a
      disposition and a reason for each
- [x] ✅ **47 endpoints bridged** (⚠️ **not 43 — corrected by the audit**); ⚠️ **19 history/buffers
      EXCLUDED and named**, owner EP-019. ✅ **Linux 34 → 81 of 100, verified by re-running the audit**
- [x] ✅ ⚠️ **Every bridged endpoint exercised through `ScriviBridge`** — not the C ABI (I-0113's lesson)
- [x] ✅ ⚠️ **`ctest` ACTUALLY RAN in the container, NON-ROOT, tests ON** — **571 cases / 9,439 assertions,
      all passed.** ⚠️ **Not "the image built"**
- [x] ✅ `ctest` green on macOS — **567/567**, unchanged
- [x] ✅ **The Porting Outline exists**, written for **five** platforms
- [x] ✅ **EP-035 opened** in the Epic backlog with AC11 moved into it
- [x] ✅ **User verification — GRANTED 2026-08-25.** All six Tasks ✅ Verified; Sprint closed the same step

✅ **NO live click-through is owed this sprint — user-ruled (§3).** No writer-facing surface ships, so
`ctest` **is** the verification standard here and **T-0464 is the acceptance evidence.**

⚠️ **Recorded honestly all the same:** that evidence class has never caught this project's real defects —
**22 consecutive Issues across SP-118–SP-120 came from clicking, none from a suite.** ⚠️ **The click-through
is DEFERRED to EP-035, not waived**, and EP-035's own ACs carry it.

---

## 6. Explicitly OUT of scope

| Item | Where it goes |
| ---- | ------------- |
| ⚠️ **ANY Linux UI** — object cards, detail sheet, sources, pickers | **EP-035** |
| ⚠️ **AC11 itself** | **EP-035** (moved by user ruling) |
| **history (15) + buffers (4) endpoints** | ⚠️ **EP-019** — its Linux story is unruled |
| **AC12** — full verification + Epic close prep | **SP-122** |
| **T-0459** — per-citation locators on the edge | **EP-032** |
| iOS / iPadOS / visionOS / Windows bridges | ⚠️ **Future Epics — but the OUTLINE is written for them now** |

---

*Last Updated: 2026-08-25 (**Audit Check remediation, user-ruled** — seven findings F-1…F-7 applied.
⚠️ **Endpoint count corrected 102 → 100** (`scrivi.h` declares 100 + `scrivi_free`; the old figure counted
prose). ⚠️ **Apple coverage MEASURED at 96 of 100, not 98** — and ⚠️ **FOUR endpoints are unreached on
Apple, not the two the plan assumed**: `resolve_timeline_project_times`, `set_timeline_epoch_offset`,
`set_world_epoch_offset`, `upsert_relation_type`. ⚠️ **In-scope corrected 43 → 47.** Task statuses
reconciled across the Epic/Sprint/Task layers; SP-121's six Tasks moved to `Task-unverified.md`.
Prior note follows.)*

*Last Updated: 2026-08-24 (**SP-121 🟡 ACTIVATED** — EP-034's seventh of eight. ⚠️ **AC11 was found at
planning to be an ENTIRE EPIC, not a parity sprint**: the Linux app has **zero** object/edge/world/asset
call sites and a **67-line stub** inspector, against ~4,800 lines of Apple surface. ✅ **User-ruled: AC11
splits out to EP-035**; SP-121 completes `ScriviBridge` instead. ⚠️ **Measured: 100 endpoints exist (⚠️ **not 102**), Apple
reaches 100, Linux reaches 35 — 62 missing; 43 in scope, 19 history/buffers excluded to EP-019.**
⚠️ **The Porting Outline (T-0465) is written for FIVE platforms** because this port recurs for iPad,
iPhone, Windows and visionOS. Next available Task **T-0466**; Issue **I-0171**.)*


---

## ✅ Retrospective — SP-121

### What went well

✅ **T-0460 landing FIRST paid for itself immediately.** It caught two wrong planning figures before a
line of bridge code was written: the endpoint count (**102 → 100**) and the in-scope total (**43 → 47**).
⚠️ **Had either surfaced at close, it would have read as scope creep rather than as a corrected estimate.**

✅ **`ctest` ACTUALLY RAN on Linux** — **571 cases / 9,439 assertions**, non-root, in a second image built
with `SCRIVI_BUILD_TESTS=ON`. ⚠️ **This is the leg SP-116 could never run**, and the reason it could not
(`project_linux_container_tests_off`) was addressed rather than worked around.

✅ **The scope split was made by ruling, not by drift.** AC11 was found at planning to be an entire Epic;
it left EP-034 for **EP-035** with the consumer opened *before* the capability shipped.

### What didn't

⚠️ **Planning measured by grepping prose.** Both wrong figures came from counting matches in text as if
they were declarations — including `scrivi_free` and `scrivi_c_api`. ✅ **Fix: count declarations, and
state the command used.**

⚠️ **The plan asserted "two endpoints Apple does not reach." The real number is FOUR** — and this was not
caught by the sprint at all. ⚠️ **It surfaced in the 2026-08-25 Audit Check, after the work was done.**
`resolve_timeline_project_times`, `set_timeline_epoch_offset`, `set_world_epoch_offset` and
`upsert_relation_type` are unreached on Apple. ⚠️ **`upsert_relation_type` needs an EP-035 consumer.**

⚠️ **Four tracking layers disagreed with each other at review time** — `Epic-active.md` showed all six
Tasks 🔵 Planned while they were 🟠 Implemented; the statistics block said **0** unverified against a file
holding **6**; ⚠️ **the six Tasks were never moved to `Task-unverified.md` as `Task-Guidelines.md` §"When
Marking Task as Implemented" requires.** ✅ **All repaired 2026-08-25 under user ruling.**

### ⚠️ The cost this sprint knowingly accepted

⚠️ **Not one of the 47 bridged endpoints has been exercised by a human using the app.** The suite is the
only evidence, and **22 consecutive Issues across SP-118–SP-120 came from clicking, none from a suite.**
⚠️ **The click-through is DEFERRED to EP-035, not waived.** ⚠️ **If EP-035 never runs, this sprint has
produced 47 unreached functions** — the risk is named here so it can be checked, not discounted.

### What to adjust

1. **Count declarations, never prose.** Publish the command alongside any endpoint figure.
2. ⚠️ **Reconcile the four tracking layers BEFORE marking a Sprint 🟠 Review**, not after — an Audit Check
   at review time would have caught all four of the above mechanically.
3. ⚠️ **Carry the four Apple-unreached endpoints into EP-035's planning** as named work, so
   `upsert_relation_type` gets a consumer rather than a second sprint of silence.

---

*Closed 2026-08-25 on user approval. Tasks T-0460–T-0465 ✅ Verified and archived in the same step.*
