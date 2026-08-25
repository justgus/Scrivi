# Closed Sprint

## SP-122 — `[Cross]` AC verification + ⚠️ **the live-use pass** + EP-034 close prep

**Status:** ✅ **CLOSED 2026-08-25** (user-approved) — all six Tasks ✅ **Verified**
**Epic:** [EP-034](../../Epics/Epic-active.md) — `[Cross]` Object Detail & Media · **sprint 8 of 8 — THE LAST**
**Codebase:** `[Cross]` — ⚠️ **verification sprint. No new feature work is in scope** (see §5)
**Date Activated:** 2026-08-25
**Closes:** **AC12** — ⚠️ **the only AC still open.** AC1–AC10 are met; **AC11 left this Epic** to EP-035
**Tasks:** **T-0466 – T-0471** (six) — ✅ **all Verified**, archived to [`../../Tasks/Verified/Task-verified-0466-0471.md`](../../Tasks/Verified/Task-verified-0466-0471.md) · **Next available:** T-0472 · Issue **I-0173**

---

## 1. Sprint Goal

**Prove EP-034's eleven met ACs are actually true — by running the suites AC12 names and by USING the
app on the real rig — then prepare the Epic for close.**

⚠️ **This sprint's job is to try to FALSIFY the Epic, not to confirm it.** EP-034 has closed seven
sprints; ⚠️ **22 consecutive Issues across SP-118–SP-120 came from clicking, and NONE from any suite.**
✅ **The live-use pass is therefore the primary instrument here, and the suites are the floor, not the
ceiling.**

⚠️ **Finding Issues in this sprint is SUCCESS, not slippage.** EP-031 planned 6 sprints and delivered 11;
**four of the five additions came from USE.** If the live pass produces Issues, they are filed and fixed
before the Epic closes — that is what this sprint is for.

---

## 2. ⚠️ What AC12 actually demands — five legs, and two are rarely run

- [x] ✅ **L1** — `ctest` green on **macOS arm64**
- [x] ✅ **L2** — ⚠️ `ctest` green on **macOS x86-64** — ⚠️ **rarely run.** `CMakeLists.txt:17-19` records why
      it matters: **arm64 `sdiv` quietly yields 0 where x86-64 `idiv` raises `#DE`**, so "all tests pass"
      has meant "all tests pass on arm64."
- [x] ✅ **L3** — ⚠️ `ctest` green **with sanitizers** (`-DSCRIVI_ENABLE_SANITIZERS=ON`, T-0413/SP-106) —
      ⚠️ **also rarely run.** A sanitizer removes architecture from the question: the check is emitted in
      the code, so the same defect fails on **every** target.
- [x] ✅ **L4** — **Interop tests green** + app **BUILD SUCCEEDED** (macOS)
- [x] ✅ **L5** — ⚠️ **Linux container green, meaning `ctest` ACTUALLY RAN** — ⚠️ **the shipping Dockerfile
      builds tests OFF**, so a green image has never implied a green suite
      (`project_linux_container_tests_off`). ✅ **SP-121 ran this leg: 571 cases / 9,439 assertions,
      non-root. It must be re-run here, not cited.**

⚠️ **`xcodebuild test` LAUNCHES the app and has rewritten a real project (I-0150).** ✅ **Check Scrivi is
not running first — and ⚠️ "not running" is NOT sufficient**: use a throwaway project, never the real rig,
for any suite run (`feedback_check_scrivi_running_before_tests`).

---

## 3. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0466** | **L1 + L4** — `ctest` macOS arm64, interop, app build | **High** | 🟠 **Implemented — Not Verified** |
| **T-0467** | ⚠️ **L2 + L3** — **x86-64** and **sanitizer** runs — ⚠️ **the two legs that hide defects** | **High** | 🟠 **Implemented — Not Verified** |
| **T-0468** | **L5** — Linux container `ctest`, ⚠️ **tests ON, NON-ROOT**, re-run not cited | **High** | 🟠 **Implemented — Not Verified** — ⚠️ **filed I-0171** |
| **T-0469** | ⚠️ **THE LIVE-USE PASS on the real rig** — ⚠️ **the primary instrument** | **High** | 🟠 **Implemented — Not Verified** — ✅ **user ran it 2026-08-25; CLEAN** |
| **T-0470** | ⚠️ **Re-verify AC1–AC10 against their OWN triggers** | **High** | 🟠 **Implemented — Not Verified** — ✅ **no failures** |
| **T-0471** | **Epic close prep** — ✅ **Audit Check RUN (1 finding, corrected)** + completion summary | Medium | 🟠 **Implemented — Not Verified** |

### T-0469 — ⚠️ The live-use pass

**Rig:** `the-stairs-of-tintagael` / `the-lone-golem` + the **Eskandar** world on USB
(`project_test_rig_tintagael_eskandar`).

⚠️ **This is REAL WRITING WORK. Back it up before touching it**, and ⚠️ **never point a test runner at
it** (I-0150).

**Exercise the whole Epic end-to-end, not each AC in isolation** — open a Kind Card, detail sheet,
edit fields, import/replace/remove an image, create a relationship and traverse it from both endpoints,
create a source and read the footnote, ⚠️ **and pull the USB drive mid-edit** (AC9's second half).

⚠️ **`volumeIsRemovable`/`volumeIsEjectable` both read FALSE on that drive** — availability is decided by
**volume-root mount presence**, not the flags.

### T-0470 — ⚠️ Re-verify, do not re-cite

⚠️ **An AC marked met by the sprint that built it is a self-report.** Each of AC1–AC10 gets re-checked
against its own trigger — `feedback_verify_each_half_separately`, whose origin (I-0132) was exactly a
two-part fix where one half was credited without being tried.

### T-0471 — Close prep

✅ **The Audit Check is legitimate here** — it is run **before an Epic close** and is ⚠️ **NOT an Audit**;
its findings are ruled as part of the close. ⚠️ **It is READ-ONLY** (`Audit-Guidelines.md` §Rules).

⚠️ **Claude may mark EP-034 🟠 Complete. Only the user may CLOSE it.**

### ⚠️ Carried in from SP-121's retrospective — do not let these evaporate

1. ⚠️ **`upsert_relation_type` is bridged but unreached on BOTH platforms** — it needs an EP-035 consumer.
2. ⚠️ **Three timeline endpoints are unreached on Apple**: `resolve_timeline_project_times`,
   `set_timeline_epoch_offset`, `set_world_epoch_offset`.
3. ⚠️ **SP-121's 47 bridged endpoints have NO live click-through.** ⚠️ **They are NOT in scope here** —
   EP-035 owns them — but ⚠️ **EP-034 must not be read as having proven them.**

### ⚠️ A duplicate found at planning — file, do not fix silently

⚠️ **`platforms/linux/docker/Dockerfile.dockerignore` ALREADY EXISTED** and already solved the exact
problem SP-121's new root `.dockerignore` addresses — and ⚠️ **it is the more complete file** (it also
excludes `**/build/` and the ~0.5 GB Apple `.build/` tree that once exhausted the Docker copy).

⚠️ **Docker prefers `<dockerfile>.dockerignore` when present, so the root file may be dead weight** —
⚠️ **or may be the one actually in force, depending on how the build is invoked.** ✅ **T-0468 determines
which is live and files an Issue; it does NOT delete either file on sight.**

⚠️ **This is `feedback_look_for_existing_pattern_first` again** — SP-118's dominant defect, and SP-121
re-earned it by adding a file instead of grepping for one.

---

---

## ⚠️ Execution status — 2026-08-25

### ✅ AC12 — all five legs GREEN, each RUN not cited

| Leg | Command | Result |
| --- | ------- | ------ |
| **L1** arm64 | `cmake -B … -DCMAKE_OSX_ARCHITECTURES=arm64` + `ctest` | ✅ **567/567** (7.3 s) |
| **L2** ⚠️ **x86-64** | `-DCMAKE_OSX_ARCHITECTURES=x86_64` + `ctest` | ✅ **567/567** (18.6 s under Rosetta) — ⚠️ **binary confirmed `Mach-O 64-bit x86_64`, not a silently-native build** |
| **L3** ⚠️ **sanitizers** | `-DSCRIVI_ENABLE_SANITIZERS=ON` + `ctest` | ✅ **567/567**, zero sanitizer diagnostics — ⚠️ **verified LINKED: 55 `__asan`/`__ubsan` symbols vs **0** in the plain build** |
| **L4** interop + app | `xcodebuild … test` / `build` | ✅ **127 tests / 12 suites**, ✅ **BUILD SUCCEEDED** |
| **L5** ⚠️ **Linux** | second image, `SCRIVI_BUILD_TESTS=ON`, `docker run` | ✅ **571/571**, ⚠️ **NON-ROOT confirmed `uid=1001(scrivi)`** |

⚠️ **TOOLCHAIN CORRECTION — user-flagged mid-sprint.** The first L1 run used **Xcode 26.6**
(`xcode-select` default). ⚠️ **The project requires Xcode 27**, which lives at `/Applications/Xcode-beta.app`.
⚠️ **That run was DISCARDED, not adjusted** — a different compiler (**AppleClang 21.0.0** vs 26.6's) makes
the result void. ✅ **All five legs above were run under `DEVELOPER_DIR=/Applications/Xcode-beta.app/Contents/Developer`**,
session-scoped rather than a machine-wide `xcode-select --switch`.

⚠️ **macOS 567 vs Linux 571** — the four extras are the Linux-only `EncryptedFileSecureStore` tests
(SP-059/T-0229), as SP-121 recorded. **Checked, not assumed.**

### ✅ I-0150 did NOT recur

⚠️ **The real rig was MOUNTED during the `xcodebuild test` run** (`/Volumes/Scrivi Worlds`, carrying
`Eskandar.scrivworld` and `the-stairs-of-tintagael.scrivi`). ✅ **Fingerprinted before and after: 625 files,
identical `md5` of all mtimes, nothing modified in the preceding 10 minutes.** The interop suite uses
`temporaryDirectory` and deliberately non-existent `/Volumes/…` paths.

### ⚠️ I-0171 — filed, NOT fixed (T-0468)

⚠️ **REPRODUCED LIVE**, and ⚠️ **SP-121's `.dockerignore` fix is INCOMPLETE.** A `--no-cache` container
build failed on `/src/build-tests/CMakeCache.txt` — ⚠️ **`build-tests/` is dated Aug 21 and is
PRE-EXISTING**, not created by this sprint. ⚠️ **Five build directories exist** (`build/`, `build-tests/`,
`build-linux-tests/`, `build-iphoneos/`, `build-iphonesimulator/`) and ⚠️ **both ignore files exclude only
`build/`** — SP-121 matched the one directory it had seen rather than the pattern.

⚠️ **L5 was measured by working AROUND the defect** (a non-colliding build-dir name), ⚠️ **not by fixing
it** — the fix belongs to I-0171 under a ruling, and §5 puts it out of scope.

⚠️ **Two incidental blockers in my own throwaway Dockerfile, NOT repo defects:** missing `libssl-dev`
(needed by `EncryptedFileSecureStore`) and an OOM from unbounded `--parallel`. Both fixed in the scratch
file only.


### ✅ T-0469 / T-0470 — the live pass RAN, and it was CLEAN

**User ran the pass 2026-08-25 on the real rig.** ✅ **No failures across AC1–AC10.**

⚠️ **USER RULING — and a correction to this sprint's own framing:**

> *"We vetted all the features in the Apple App and you are having me do a regression test verbatim from
> previous tests. I would expect no failures at this point. And I didn't have any."*

⚠️ **This sprint was written as though a clean pass would be suspicious.** ✅ **That framing was WRONG and
is corrected here.** The *"22 consecutive Issues came from clicking, none from a suite"* statistic comes
from passes over **NEWLY BUILT** surfaces — SP-118's related-objects, SP-119's images, SP-120's sources.
⚠️ **A verbatim re-run of already-vetted features is a REGRESSION test, and clean is the EXPECTED
result.** The statistic does not transfer, and predicting Issues here was a misreading.

✅ **What the pass DOES establish:** every AC1–AC10 behaviour still works after seven sprints of change,
on real data, with the Eskandar world attached over USB.

### ⚠️ The console log was read, not skimmed — three findings, two dismissed

| Observation | Verdict |
| ----------- | ------- |
| **55 consecutive `saveSceneBlocking WROTE` for ONE scene** | ✅ **BY DESIGN.** `saveSceneBlocking` guards on `isDirty`, and the editor auto-saves on a **1-second keystroke debounce** (`ManuscriptTextView.swift:14`). ~55 writes ≈ 55 s of typing |
| **Repeated `extract OK: 96 records` + `donate`** | ✅ **BY DESIGN.** Re-donation is tied to `saveAllDirty` on app-resign (`ProjectSession.swift:269-275`) — window switching, not keystrokes. The 96 → 97 → 62 drift tracks real content change and project focus |
| ⚠️ **`-layoutSubtreeIfNeeded on a view which is already being laid out`** | ⚠️ **REAL — ours, not the system's.** → **I-0172**, ✅ **fixed and Verified** |

⚠️ **Everything else in the log was system-framework noise** (`ViewBridge Terminated`,
`Ignoring request to entangle context`, `DetachedSignatures`) — ⚠️ **checked, not assumed clean.**

### ⚠️ T-0471 — Audit Check RUN (read-only); close summary NOT yet writable

✅ **Seven mechanical checks run 2026-08-25.** Clean: sprint-status agreement across all files, ID
continuity (T-0472 / I-0171 reserved only), no orphan files, AC status agreement, table/entry parity.

⚠️ **One finding, and it was NOT small:** `Issue-Documentation.md` §Active Issues had gone **four sprints
stale** — it read *"2 Issues open"* and listed **I-0140/I-0141 as open in SP-116**, ⚠️ **both of which
were fixed, Verified and archived on 2026-08-21** when SP-116 closed. ⚠️ **It also had no row for I-0171.**
✅ **Corrected in place**, since the missing row was this sprint's own filing.

⚠️ **This is exactly what the Audit Check exists for.** `Audit-Guidelines.md` records that the 2026-08-19
audit found two findings that would have corrupted **EP-031's** close; ⚠️ **this one would have let EP-034
close against an index claiming open Issues that had been resolved four sprints earlier.**

⚠️ **The EP-034 completion summary is NOT written and cannot honestly be written yet** — ⚠️ **AC12 is not
met until T-0469's live pass runs**, and eleven of EP-034's twelve ACs rest on evidence T-0470 has not
re-verified. ✅ **Drafting a summary now would be the self-report T-0470 exists to prevent.**


### ⚠️ Issues filed by SP-122 — TWO, and neither blocks the close

| ID | Source | Status |
| -- | ------ | ------ |
| **I-0171** | ⚠️ **T-0468**, by RUNNING the Linux leg — SP-121's `.dockerignore` fix is incomplete (five build dirs, one pattern) | 🔴 **Open** — ⚠️ **filed, not fixed**; the fix needs a ruling |
| **I-0172** | ⚠️ **T-0469's console log** — `ForkPopover` forced a nested AppKit layout pass | 🟠 **Resolved - Not Verified** — fixed on user instruction; ⚠️ **unproven by use** |

⚠️ **NEITHER came from the regression pass itself, which was CLEAN** — one came from running a build leg,
the other from reading the log the pass produced.


## 4. Definition of Done

- [x] ✅ **All five AC12 legs GREEN**, each with its command and output recorded — ⚠️ **including x86-64 and
      sanitizers**, and ⚠️ **Linux `ctest` RE-RUN, not cited from SP-121**
- [x] ✅ **The live-use pass is DONE on the real rig** — ⚠️ **CLEAN, which is the EXPECTED result for a regression pass** (user ruling)
- [x] ✅ **Every Issue found is FILED** — **I-0171** (open) and **I-0172** (✅ Verified + archived). ⚠️ **Neither came from the pass itself**: one from running a build leg, one from reading the log
- [x] ✅ **AC1–AC10 re-verified against their own triggers**, not re-cited — **no failures**
- [x] ✅ **AC12 MET** — all five legs green + a clean live pass; EP-034 completion summary drafted
- [x] ✅ **Audit Check run** (read-only) — ⚠️ **one finding: a four-sprint-stale Issue index, corrected**
- [x] ✅ **User verification — GRANTED 2026-08-25.** All six Tasks ✅ Verified; Sprint closed the same step.
      ⚠️ **EP-034 remains 🟠 Complete and is NOT closed — that approval is separate and still owed.**

---

## 5. Explicitly OUT of scope

| Item | Where it goes |
| ---- | ------------- |
| ⚠️ **ANY new feature work** | ⚠️ **A verification sprint that builds features cannot verify itself** |
| ⚠️ **Linux UI / object layer** | **EP-035** |
| **history + buffers endpoints (19)** | **EP-019** |
| **Live click-through of SP-121's 47 endpoints** | **EP-035** |
| **Fixing the `.dockerignore` duplicate** | ⚠️ **FILE it (T-0468); the fix is a follow-on** |
| **T-0459** — per-citation locators | **EP-032** |

---

*Last Updated: 2026-08-25, second pass (**SP-122 EXECUTING** — ⚠️ **AC12's five legs are ALL GREEN**:
arm64 567/567, ⚠️ **x86-64 567/567**, ⚠️ **sanitizers 567/567**, interop 127 + BUILD SUCCEEDED, ⚠️ **Linux
571/571 NON-ROOT**. ⚠️ **The first run used Xcode 26.6 and was DISCARDED** — the project needs Xcode 27
(`/Applications/Xcode-beta.app`), a different compiler. ✅ **I-0150 did NOT recur** — the mounted rig was
fingerprinted before and after, unchanged. ⚠️ **I-0171 FILED: SP-121's `.dockerignore` fix is INCOMPLETE**
— five build dirs exist, both ignore files match only `build/`. ⚠️ **The Audit Check found the Issue index
FOUR SPRINTS STALE** (I-0140/I-0141 shown open in a sprint that closed 2026-08-21). ⚠️ **T-0469 (the live
pass) and T-0470 are BLOCKED ON THE USER** — they need a human at the app and a physical USB pull.
Prior note follows.)*

*Last Updated: 2026-08-25 (**SP-122 🟡 ACTIVATED** — EP-034's **eighth and LAST** sprint. ⚠️ **AC12 is the
only AC still open**; AC1–AC10 met, AC11 moved to EP-035. ⚠️ **Two of AC12's five legs — x86-64 and
sanitizers — are rarely run**, and `CMakeLists.txt:17-19` records why that matters. ⚠️ **The live-use pass
is the PRIMARY instrument**: 22 consecutive Issues came from clicking, none from a suite. ⚠️ **A
pre-existing `Dockerfile.dockerignore` duplicates SP-121's root `.dockerignore`** — filed, not fixed.
Tasks **T-0466–T-0471**; next available **T-0472**; Issue **I-0171**.)*


---

## ✅ Retrospective — SP-122

### What went well

✅ **The two rarely-run legs were actually run, and both were green.** x86-64 and sanitizers had been
skipped often enough that `CMakeLists.txt` carries a comment explaining why that is dangerous.
⚠️ **Neither was cited from a previous sprint** — both were executed, and the x86-64 binary and the
sanitizer symbols were **verified genuine** rather than assumed from a green exit code.

✅ **The Audit Check paid for itself in one finding** — a four-sprint-stale Issue index that would have
corrupted EP-034's close.

✅ **Two Issues were found by RUNNING things, not by reading them.** I-0171 came from executing the Linux
leg; I-0172 from reading the console log the live pass produced. ⚠️ **Neither would have surfaced from a
document review.**

### What didn't

⚠️ **The sprint was PLANNED on a misapplied statistic.** It expected the regression pass to find defects,
because *"22 consecutive Issues came from clicking."* ⚠️ **That statistic describes passes over NEWLY
BUILT surfaces, and SP-122's pass was a verbatim regression over vetted ones.** ✅ **User-corrected**, and
the correction is recorded in the Task archive rather than quietly dropped.

⚠️ **The first test run used the wrong Xcode** (26.6 rather than the required 27), and ⚠️ **it was the
USER who caught it, not the sprint.** ⚠️ **Four legs had already been planned against that toolchain.**
✅ **The result was discarded rather than adjusted**, but the check belonged at the start.

⚠️ **SP-121's `.dockerignore` fix was incomplete and this sprint had to find that by tripping over it.**
⚠️ **It matched the ONE build directory it had seen** rather than the pattern — and five exist.

### What to adjust

1. ⚠️ **Verify the toolchain BEFORE the first suite run**, not after four legs.
2. ⚠️ **Distinguish a REGRESSION pass from a DISCOVERY pass at planning time**, and state which one a
   sprint is running. They have different expected outcomes and conflating them makes a clean result
   look like weak evidence.
3. ⚠️ **When fixing a class of defect, match the PATTERN, not the instance** — I-0171 is the second time
   in two sprints that a fix addressed the single case in front of it.

### ⚠️ What this sprint did NOT establish

⚠️ **SP-121's 47 bridged Linux endpoints remain unexercised by any human.** ⚠️ **AC12 green does not
touch them** — they are suite-proven only, and EP-035 owns the click-through.

---

*Closed 2026-08-25 on user approval. Tasks T-0466–T-0471 ✅ Verified and archived in the same step.*
⚠️ **EP-034 is 🟠 Complete but NOT closed — that approval is separate.**
