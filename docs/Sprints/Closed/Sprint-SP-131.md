# Closed Sprint — SP-131 (`[ScriviCore]` the indexes)

**Epic:** [EP-039](../../Epics/Epic-active.md) — `[Cross]` Project Load Performance
✅ **EP-039 CLOSED 2026-09-15 (user-approved).**
**Sprint closed:** 2026-09-15 (user-approved) · **Archived from** `Sprint-active.md`

✅ **OUTCOME: THE INDEXES. This is the sprint that made the project OPEN.**
✅ **`~300 s` → `1.06 s`, user-confirmed** — ⚠️ **and every later gain in EP-039 was built on it.**
✅ **ACs met: AC1, AC2, AC3, AC4, AC5, AC8** (⚠️ **AC1–AC3 were ONE delivery, not three**).
⚠️ **[AC4]'s Apple surface was NEVER BOUND and that went unnoticed until 2026-09-15** —
✅ **`scrivi_list_story_times` shipped here; the Apple timeline only adopted it at the Epic close.**

---

## SP-131 — `[ScriviCore]` ⚠️ **The in-memory indexes** — and the session that owns them

**Status:** ✅ **VERIFIED 2026-09-12 (user-approved).** ⚠️ **Awaiting close approval.**
**Epic:** [EP-039](../Epics/Epic-active.md) — `[Cross]` Project Load Performance · ✅ **the Epic's FIRST
sprint, and the one that closes the measured freeze**
**Codebase:** `[ScriviCore]` ⚠️ **+ a small `[Apple]`/`[Linux]` lifecycle change** (T-0512).
**ACs:** **AC1, AC2, AC3, AC4, AC5a–AC5e, AC8**
**Design:** [`../Scrivi_Project_Index_Design_v0_1.md`](../Scrivi_Project_Index_Design_v0_1.md) — ✅ **§5
is RULED; this sprint implements it**
**Date Created:** 2026-09-12
**Tasks:** **T-0511 – T-0517** (seven) · **Next available:** T-0518

### ✅ The target, in one line

⚠️ **`234–251 s` of a `~300 s` project open is `getSceneStoryTime` called once per scene**, ✅ **and
every one of those calls walks the entire manuscript to find one sidecar.** ⚠️ **AC2 + AC4 remove that
whole phase.** ✅ **AC1 + AC3 remove the `64–70 s` `openScene` loop's traversal half.**

### ⚠️ What reading the code changed about the plan

⚠️ **Three things are NOT as the design doc describes, and each changes a task:**

1. ⚠️ **`ScriviCore` IS CONSTRUCTED PER CALL AT THE C ABI** (`scrivi::CoreServices svc = abiServices();`
   in each endpoint). ✅ **So an index CANNOT be a `ScriviCore` member** — ⚠️ **it would be built and
   destroyed within a single call and make things strictly slower.** ✅ **It must live in a
   REGISTRY KEYED BY PROJECT ROOT**, ⚠️ **which is exactly AC5's "per open project" scope ruling.**
2. ⚠️ **THERE IS NO `scrivi_close_project`.** ✅ **`scrivi_history_close` is the ONLY close endpoint
   (1 of 100).** ⚠️ **A registry with no close LEAKS ONE INDEX PER PROJECT OPENED** — ✅ **and the
   user's own session opened two projects back to back.** ⚠️ **T-0512 exists because of this.**
3. ✅ **THE PRIOR ART IS BETTER THAN CITED.** ⚠️ **`HistoryRegistry` (`scrivi_c_api.cpp:267`) is ALREADY
   a mutex-guarded `byRoot` map with an explicit open/close pair**, ✅ **and its comment already states
   the reason for the mutex: *"ScriviEngine may call from arbitrary Swift threads."*** ⚠️ **Do NOT
   invent a second session mechanism** (`feedback_look_for_existing_pattern_first`).

✅ **ALSO ALREADY PRESENT, and it shortens T-0516:** ⚠️ **`ReadRecordingFileSystem`**
(`MultiSceneTests.cpp:381`) ✅ **is a `FileSystem` decorator that records every path read.** ✅ **AC8 and
AC5a–AC5e are all expressible with it.** ⚠️ **Do not write a second one.**

✅ **AND THE TRAVERSAL ALREADY EXISTS:** ⚠️ **`ManuscriptOrderResolver::resolve()` walks
chapters→scenes in order and returns `ResolvedScene{sceneID, chapterID, title, chapterTitle, slug,
status, metadataPath, contentPath, chapterMetadataPath}`.** ✅ **That is ALREADY nearly the whole
`SceneLocationIndex` value shape.** ⚠️ **The index is BUILT FROM this one call — not from new walking
code.**

### Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0511** | ✅ **`ProjectIndexRegistry` — the session that owns the indexes.** ⚠️ **Mutex-guarded `byRoot` map, MIRRORING `HistoryRegistry`** (`scrivi_c_api.cpp:267`), ⚠️ **NOT a new mechanism.** ✅ **Holds the three indexes for one open project.** ⚠️ **AC5's scope ruling (per open project) is THIS task** | **High** | ✅ **Verified** |
| **T-0512** | ⚠️ **`scrivi_close_project` — a NEW ENDPOINT (100 → 101).** ⚠️ **WITHOUT IT THE REGISTRY LEAKS.** ✅ **Wire the app's EXISTING `ProjectSession.close()`** (`ProjectSession.swift:267,304` — ⚠️ **it already calls `historyCapture?.close()`, so the hook exists**) ⚠️ **and Linux's equivalent.** ⚠️ **Closing an unopened project MUST be a no-op, not an error** | **High** | ✅ **Verified** |
| **T-0513** | ✅ **Build all three indexes from ONE `ManuscriptOrderResolver::resolve()` pass** — **AC1, AC2, AC3.** ⚠️ **`SceneStoryTimeIndex` needs the `storyTime` block, which `ResolvedScene` does NOT carry** — ✅ **extend `ResolvedScene` rather than adding a second walk.** ⚠️ **ONE DELIVERY: partial is not acceptable** | **High** | ✅ **Verified** |
| **T-0514** | ✅ **Route the 6 `findSceneMetaPath` call sites and the resolver sites through the index** — ⚠️ **index hit ⇒ a PATH, and the caller STILL OPENS THE FILE** (**AC5a**). ⚠️ **A miss or a failed open ⇒ REAL TRAVERSAL + rebuild, NEVER a negative result** | **High** | ✅ **Verified** |
| **T-0515** | ✅ **`scrivi_list_story_times` — the SPARSE bulk call** (**AC4**). ⚠️ **A record ONLY where `offsetSource != "default"` OR `durationSource != "default"` OR `bandID` is non-empty.** ⚠️ **EMPTY IS THE COMMON CASE AND IS NOT AN ERROR** — ⚠️ **the caller MUST use the failure signal** (`project_envelope_empty_vs_failed`) | **High** | ✅ **Verified** |
| **T-0516** | ✅ **The invalidation tests — AC5a–AC5e — using the EXISTING `ReadRecordingFileSystem`.** ⚠️ **Each must be VERIFIED FAILING against an unfixed core before it counts** (`feedback_boundary_tests_not_facade`) | **High** | ✅ **Verified** |
| **T-0517** | ✅ **The complexity regression test** (**AC8**). ⚠️ **PIN THE COMPLEXITY, NOT A DURATION** — ✅ **extend the I-0196 pattern: opening one scene must not cost work proportional to how many OTHER scenes exist**, ⚠️ **and the same must now hold for a STORY-TIME query** | **High** | ✅ **Verified** |

### ✅ MEASURED RESULT — the reason this sprint existed

⚠️ **Measured at the C ABI on a COPY of the real 1,153-scene Dumas fixture** (`dumas-prose.scrivi`,
1,203 sidecars), ✅ **before and after, same harness, same machine** — ⚠️ **the "before" column is the
shipped behaviour, produced by disabling the locator, NOT an estimate.**

| phase | ⚠️ before | ✅ after | change |
| ----- | -------- | ------- | ------ |
| `get_scene_story_time` × 1154 | ⚠️ **251,800 ms** | ✅ **328 ms** | ✅ **767×** |
| `open_scene` × 1154 | ⚠️ **70,552 ms** | ✅ **356 ms** | ✅ **198×** |
| ↳ first-100 average | `12.81 ms` | `0.33 ms` | |
| ↳ **last-100 average** | ⚠️ **`110.29 ms`** | ✅ **`0.30 ms`** | ⚠️ **quadratic → FLAT** |
| `list_story_times` (bulk, AC4) | `215.6 ms` | ✅ **`0.2 ms`** | |
| **the two hot phases** | ⚠️ **322.4 s** | ✅ **0.68 s** | ✅ **~474×** |

✅ **THE BASELINE INDEPENDENTLY REPRODUCES THE USER'S CONSOLE** (`234–251 s` for the timeline phase),
⚠️ **from a harness that shares no code with the app.**
✅ **AC8's real proof is the last-100 row:** ⚠️ **`110.29 ms` against a first-100 of `12.81 ms` is the
quadratic, visible within a single run** — ✅ **after, both are `~0.3 ms`, so per-scene cost no longer
depends on how many scenes precede it.**

⚠️ **`open_project` itself is UNCHANGED (~5.0 s) and was never in scope** — ✅ **it is one traversal,
which is correct.**

### ⚠️ TWO DEFECTS FOUND BY THE NEW TESTS — both mine, both in this sprint's own work

1. ⚠️ **INVALIDATING ONLY *BEFORE* THE WORK WAS WRONG, AND SHIPPED GREEN AT FIRST.**
   ⚠️ **Nearly every mutation calls `findSceneMetaPath` to locate its target, which goes THROUGH THE
   LOCATOR and REBUILDS THE INDEX FROM PRE-WRITE DISK STATE.** ⚠️ **So a before-only drop left a
   freshly-built STALE index behind the moment the write landed:** ✅ **`scrivi_set_scene_story_time`
   wrote correctly to disk and `scrivi_list_story_times` then reported `count:0`.**
   ✅ **FIX: `ProjectIndexInvalidation`, an RAII guard that drops on BOTH sides.**
2. ⚠️ **MY FIRST AC5b AND AC5e TESTS ASSERTED THROUGH `openScene` AND COULD NOT FAIL.**
   ⚠️ **AC5a's validate-on-use fallback SILENTLY REPAIRS staleness**, ✅ **so both passed against a
   deliberately broken core** (before-only invalidation; a registry keyed by a constant).
   ✅ **FIX: both now assert through `scrivi_list_story_times`, which is served STRAIGHT FROM THE
   INDEX with no per-scene disk read to accidentally correct the answer.**
   ⚠️ **This is `feedback_boundary_tests_not_facade` one layer in: the tests were at the right
   BOUNDARY and still could not see the defect, because the endpoint they chose self-heals.**

### ✅ Verification performed

| check | result |
| ----- | ------ |
| `ctest` macOS | ✅ **594/594** (⚠️ baseline 586 + 8 new) |
| `ctest` Linux (Docker, ⚠️ **NON-ROOT**) | ✅ **598/598** |
| ⚠️ Linux `ctest` as ROOT | ⚠️ **596/598** — ✅ **the 2 failures are the I-0183 unreadable-package tests, which root defeats by being able to read a `chmod 000` directory.** ⚠️ **Pre-existing and unrelated to SP-131** (`project_linux_container_tests_off`) |
| Linux Qt smokes | ✅ **23/23** (⚠️ the 24th script is a fixture GENERATOR, not a smoke) |
| `xcodebuild` macOS / iOS / visionOS | ✅ **all three BUILD SUCCEEDED** |
| Linux Qt app image | ✅ **builds clean** |
| ⚠️ **Every new test VERIFIED FAILING first** | ✅ **AC4, AC5a, AC5b, AC5d, AC5e each re-run against a deliberately regressed core and confirmed RED** |

⚠️ **NOT DONE — the `rebuildStorage` probe fix** (a DoD item). ⚠️ **It is `[Apple]` instrumentation, not
core, and belongs with [EP-039] AC6/AC9's app-side work.** ⚠️ **It still reports a false
`projected=3519s`.**

⚠️ **NOT DONE — a LIVE PASS.** ⚠️ **Everything above is measured at the ABI and in suites.** ✅ **The
app builds, ⚠️ but no one has yet opened the Dumas fixture in the real Scrivi UI**
(`feedback_live_pass_finds_what_suites_cannot`). ⚠️ **THE APP IS STILL FULLY SYNCHRONOUS ([EP-039]
AC6), so the open will still BLOCK — it should simply block for far less time.**

### ⚠️ Sequencing

⚠️ **T-0511 → T-0512 first.** ✅ **The registry and its close are the container; the indexes have
nowhere to live until they exist.** ⚠️ **T-0513 then fills it, T-0514/T-0515 consume it, T-0516/T-0517
pin it.**
⚠️ **T-0512 IS NOT OPTIONAL AND IS NOT LAST.** ⚠️ **Shipping T-0511+T-0513 without it means every
project open leaks an index for the life of the process.**

### Definition of Done

- [ ] ⚠️ **The 1,153-scene Dumas fixture opens WITHOUT the `234–251 s` timeline phase.** ✅ **MEASURED
      with `ScriviDiag`, before AND after** — ⚠️ **a "fix" must not be able to quietly make it slower.**
- [ ] ⚠️ **`openScene`'s per-scene cost NO LONGER RISES with position.** ⚠️ **The `12.4 → 53.3 ms`
      running-average climb is the live symptom** — ✅ **it must be FLAT.**
- [ ] ✅ **AC1–AC4 delivered TOGETHER; AC5a–AC5e and AC8 all have passing tests.**
- [ ] ⚠️ **Every new test VERIFIED FAILING against the unfixed core.** ⚠️ **A test written after the fix
      that has never been red proves nothing.**
- [ ] ⚠️ **`scrivi_close_project` is wired on BOTH Apple and Linux** — ⚠️ **not just added to the ABI.**
      ✅ **`project_capability_without_surface` cuts both ways.**
- [ ] ✅ **`ctest` green** (⚠️ **baseline `586/586`**) ⚠️ **AND run on LINUX, not only macOS**
      (`project_linux_container_tests_off`: ⚠️ **the Docker image builds `SCRIVI_BUILD_TESTS=OFF`**).
- [ ] ⚠️ **The `rebuildStorage` probe is FIXED** — ✅ **it reports wall-clock-since-open as `elapsed`,
      making `avg` and `projected` noise.** ⚠️ **It has already sent one investigation chasing a
      non-defect.**

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **THE REGISTRY LEAKS** — no `close_project` exists today | ✅ **T-0512, sequenced SECOND, not last.** ⚠️ **The user's own session opened two projects back to back** |
| ⚠️ **A second session mechanism gets invented** | ✅ **`HistoryRegistry` is the pattern and it already handles the Swift-threading question** (`feedback_look_for_existing_pattern_first`) |
| ⚠️ **An index MISS is reported as "not found"** | ⚠️ **This is [I-0183]'s exact shape — it destroyed 10 of 12 relationships.** ✅ **AC5a, and T-0516 tests it directly** |
| ⚠️ **`scrivi_list_story_times` returns empty and a real timeline reads as EMPTY** | ⚠️ **`appendToArray` OMITS the key for an empty list, so `{}` is ambiguous.** ✅ **The caller MUST use the failure signal** (`project_envelope_empty_vs_failed`) |
| ⚠️ **Tests written green** | ⚠️ **A test that has never failed proves nothing.** ✅ **Every one VERIFIED FAILING first** |
| ⚠️ **"Green suite" mistaken for "the freeze is gone"** | ⚠️ **`ctest` cannot see a 300 s app freeze.** ✅ **The DoD requires a MEASURED open of the real 1,153-scene fixture** (`feedback_live_pass_finds_what_suites_cannot`) |
| ⚠️ **Declaring victory at the core** | ⚠️ **The app is STILL SYNCHRONOUS — [EP-039] AC6.** ✅ **This sprint makes the work SMALL; it does NOT make it ASYNCHRONOUS** |

---

## ⚠️ Which EP-039 AC is NOT in this sprint

| AC | Codebase | Where |
| -- | -------- | ----- |
| **AC6** — `[Apple]` load off the main thread + progress | `[Apple]` | ✅ **[SP-132] — T-0523** |
| **AC9** — notification batching | `[Apple]` | ✅ **[SP-132] — T-0524** |
| **AC10** — Spotlight indexing is change-driven | `[Apple]` | ✅ **[SP-132] — T-0522** |
| **AC7** — the blob-index RULE | — | ✅ **Ruled in [EP-039]; ⚠️ the BUILD is [EP-035] AC5** |

## 🔵 Planned, not activated

| Sprint | Codebase | ⚠️ Scope |
| ------ | -------- | ------- |
| **SP-129** | `[Apple]` | ⚠️ **The four UNBUILT Apple surfaces** + the timeline's direct-filesystem bypass. **T-0502–T-0506** |
| **SP-130** | `[Apple]`+`[ScriviCore]` | ⚠️ **Close the ScriviCore bypasses** ([I-0197]). **T-0507–T-0510** |

⚠️ **BOTH ARE AUDIT-FINDINGS SPRINTS, NOT PERFORMANCE WORK**, ✅ **and both sequence behind SP-131 and
the `[Apple]` async sprint.** ✅ **In [`Sprint-backlog.md`](Sprint-backlog.md).**
