---
sprint: SP-145
epic: EP-043
status: Active
platform: Linux
created: 2026-09-27
activated: 2026-09-27
---

# SP-145 — `[Linux]` **The Session Split** (S1 of [EP-043])

**Status:** 🟡 **ACTIVE — 2026-09-27.** ✅ **All three Tasks IMPLEMENTED; all eight ACs MET.**
⚠️ **AWAITING USER VERIFICATION** — ⛔ **no live pass on the rig yet.**
**Epic:** [EP-043](../Epics/Epic-EP-043.md) `[Linux]` **The Session** — ⚠️ **S1 of a SERIAL chain
[SP-145] → [SP-146] → [SP-147] → [SP-148]**; ⛔ **a stall here stalls the Epic.**
**Goal:** ✅ **Separate "the project's state" from "the widget showing it"** — ⚠️ **so that [SP-146] can
put one project in each window.** ⛔ **NO window changes in this Sprint.**
**ACs contributed:** — ✅ **foundation only** (⚠️ **R1–R8 are [SP-146]/[SP-147]'s**) · ⚠️ **[I-0251]**
**Rulings in force:** ✅ **[R-Q2]** (identity = `projectID`) · ✅ **[R-Q4]** ([I-0251]) · ✅ **[R-Q5]**
(the visibility carve-out) — → [`../Epics/Epic-EP-043.md`](../Epics/Epic-EP-043.md) §Rulings.

⚠️ **ID NOTE:** ✅ **This Sprint is SP-145 because [EP-043] and `Sprint-backlog.md` reserved that ID for
it in five places.** ⛔ **`next-id.py sprint` returned SP-151** — ⚠️ **the counter is a HIGH-WATER MARK
and had jumped past 145–148 when SP-149/SP-150 were created for EP-040/EP-041 work.** ✅ **User ruled
2026-09-27: honour the reservation; the registry was reset to 146 and now carries a `_reserved` note so
the next reader does not "correct" it upward again.**

---

## ⚠️ THE MANDATE — behaviour-preserving, with ONE named exception

✅ **[EP-018]'s equivalent Sprint (SP-048 / T-0192) carried the same word: *"behavior-preserving."***
⚠️ **That is the whole risk control here.** ⛔ **This Sprint must not change what the app DOES** —
✅ **only where its state LIVES** — ⚠️ **because a regression introduced here is indistinguishable from
one introduced by [SP-146]'s much larger window rework, and [SP-146] cannot be debugged on top of an
unproven extraction.**

⚠️ **THE ONE EXCEPTION IS [I-0251], AND IT IS DELIBERATE:** ✅ **it is a real behaviour CHANGE (the
writer's inspector choice will now survive a quit)**, ⛔ **so it is its OWN Task (T-0553) and NOT folded
into the extraction Task.** ✅ **[R-Q4]/[R-Q5] ruled it here because the extraction is already moving
that state, so the flag is touched ONCE.** ⚠️ **If T-0553 has to be dropped, T-0551 and T-0552 still
stand on their own.**

---

## ⚠️ WHAT I MEASURED BEFORE PLANNING — and where the Epic's own text was wrong

✅ **Measured 2026-09-26/27 in `platforms/linux/src/`, not recalled.**

| Claim | ✅/⛔ | What the code says |
| --- | --- | --- |
| `EditorShell.cpp` is 2,783 lines | ✅ **holds** | ✅ **2,783; `EditorShell.hpp` is another 545** |
| `EditorShell` owns the project singly | ✅ **holds** | ✅ **`bridge_`, `projectID_`, `projectPath_`, `appSupportRoot_`, `sceneDoc_`, `dirtyScenes_`, `activeSegment_` are all single-instance members** |
| ⛔ **"the extraction takes `inspectorVisible_` + `timelineVisible_`"** | ⛔ **WRONG — THOSE MEMBERS DO NOT EXIST** | ⚠️ **visibility is read STRAIGHT OFF THE WIDGETS: `inspector_->isVisible()` / `timeline_->isVisible()` (`EditorShell.cpp:1966`, `:1983`).** ⚠️ **`EditorShell.cpp:124` *mentions* `inspectorVisible_` in a COMMENT — ⛔ and the member was never added.** ✅ **So [R-Q5]'s carve-out must CREATE the state, not move it** |
| `InspectorLayoutStore` is `EditorShell`'s | ⛔ **WRONG** | ✅ **it is `SceneInspector`'s (`SceneInspector.hpp:257`), loaded in `SceneInspector::load` (`:316`)** — ⚠️ **so [I-0251] touches `SceneInspector`, NOT `EditorShell`** |

⚠️ **THE `inspectorVisible_` ERROR IS THE INSTRUCTIVE ONE.** ⛔ **[R-Q5] was written from a COMMENT that
names a member the code never had** — ✅ **exactly the "a list rots without being edited" failure
`CLAUDE.md`'s standing rule describes, and the reason that rule says a restatement is a defect on sight
even when it currently reads correctly.** ✅ **The ruling's INTENT is unaffected** (⚠️ *pane visibility
must become per-project state with a home*) — ⛔ **but its mechanism changes from "move two members" to
"introduce them," which is slightly MORE work, not less.**

---

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0551** | ✅ **Introduce `ProjectSession`** — per-project state out of `EditorShell`; ⚠️ **behaviour-preserving** | ✅ **Implemented — Not Verified** |
| **T-0552** | ✅ **Introduce `OpenProjectRegistry`** — `projectID` → live session ([R-Q2]); ⚠️ **still one session at a time** | ✅ **Implemented — Not Verified** |
| **T-0553** | ⚠️ **[I-0251]** — pane visibility becomes per-project STATE; inspector visibility persists THROUGH THE CORE ([R-Q4]) | ✅ **Implemented — Not Verified** |

---

## Acceptance Criteria

- [x] **AC1** — ✅ **A `ProjectSession` type owns the per-project state** currently held as
      `EditorShell` members: `projectID_`, `projectPath_`, `appSupportRoot_`, `bridge_`, `sceneDoc_`,
      `dirtyScenes_`, `activeSegment_`. ⚠️ **`EditorShell` keeps the WIDGETS** (`viewport_`,
      `navigator_`, `splitter_`, `inspector_`, `timeline_`, the progress row) — ✅ **the split line is
      "state the project has" vs "widget showing it."**
- [x] **AC2** — ⚠️ **BEHAVIOUR-PRESERVING.** ✅ **Every existing Linux smoke still passes unchanged**,
      ⛔ **and no smoke's ASSERTIONS are edited to accommodate the refactor.** ⚠️ **An assertion that
      must change means behaviour changed, which is this Sprint's one prohibition** — ✅ **surface it
      rather than editing the test.**
- [x] **AC3** — ✅ **An `OpenProjectRegistry` maps `projectID` → live session** and is the
      **authoritative** answer to *"is this project open?"* ⚠️ **Keyed by `projectID` per [R-Q2]**,
      ⛔ **not by path.** ⚠️ **Still at most ONE session in this Sprint** — ✅ **[EP-018]'s registry
      shipped the same way (T-0193 before T-0194) and that sequencing is deliberate.**
- [x] **AC4** — ✅ **The registry exposes what [SP-146] needs and nothing more:** `session(for:)`,
      `isOpen(projectID:)`, `register`, `deregister`, `openProjectIDs`. ⛔ **No window code, no
      persistence** — ⚠️ **those are [SP-146]'s and [SP-147]'s, and building them here would be the
      "capability shipped, surface never built" class this project has filed four times.**
- [x] **AC5** — ⚠️ **[I-0251] / [R-Q4].** ✅ **Pane visibility becomes per-project STATE on the session**
      (⛔ **not read off `isVisible()`**), ✅ **and the INSPECTOR's value is loaded from, and written
      back to, `inspector-layout.json` THROUGH THE CORE** (`scrivi_get/put_inspector_layout`).
      ⚠️ **TIMELINE stays session-scoped** — ✅ **Apple does not persist it either.**
- [x] **AC6** — ⛔ **`inspectorHidden` MUST round-trip.** ⚠️ **A smoke asserts an Apple-written
      `inspector-layout.json` keeps EVERY key after a visibility change** — ✅ **the same data-loss
      guard `inspector_layout_smoke` already applies to `selectedTab`**, ⚠️ **because [I-0215] is
      precisely what two owners of this file cost once.**
- [x] **AC7** — ✅ **`scripts/check-package-boundary.sh` stays GREEN.** ⛔ **No `QFile`/`QSaveFile`
      added anywhere in `platforms/linux/src/`** — ⚠️ **[I-0251] goes through the core, and that is the
      whole point of [R-Q4].**
- [x] **AC-build** — ✅ **Docker build clean; `ctest` green on Linux run as NON-ROOT in the second
      image** (⚠️ **`project_linux_container_tests_off` — the Dockerfile builds
      `SCRIVI_BUILD_TESTS=OFF`, so "the container is green" does NOT mean `ctest` ran**).
      ✅ **Apple must also still build** (⚠️ **`ScriviCore` is shared; ⛔ no core change is expected
      here, so an Apple break would mean I touched something I should not have**).

---

## ⛔ Explicitly OUT of scope

- ⛔ **ANY window change.** ✅ **One `QMainWindow`, one `QStackedWidget`, landing on page 0 — unchanged.**
  ⚠️ **[R-Q3] ruled the separate Landing window; ✅ that is [SP-146]'s to build.**
- ⛔ **ANY persistence of geometry, splitters or the open-set.** ✅ **[R-Q1] named `session.ini`;
  ⚠️ [SP-147] writes it.** ⛔ **Do not create the file here.**
- ⛔ **The R6 test guard.** ⚠️ **It belongs in the same commit as the first line of RESTORE code
  ([SP-147])** — ✅ **there is no restore code in this Sprint, so there is nothing yet to guard.**
- ⛔ **`scrivi_close_project` per window (R8)** — ✅ **[SP-146]'s.** ⚠️ **Linux already does it correctly
  for its single project; ⛔ do not disturb that while moving state.**
- ⛔ **ANY ScriviCore / C ABI change.** ✅ **[I-0251] uses the EXISTING endpoint pair.**
- ⛔ **[I-0244]'s shell work** — ✅ **ruled a sibling Epic AFTER this one ([R-Q5]).**

---

## ⚠️ Known traps for THIS Sprint

- ⚠️ **The mandate is the trap.** ✅ **A 2,783-line file invites "while I am in here" fixes.** ⛔ **Every
  one of them makes AC2 unprovable** — ⚠️ **and [SP-137]'s T-0547 needed FOUR implementations, each
  building clean and passing every suite, which is what an unfalsifiable green looks like.**
- ⚠️ **`SceneInspector` owns the layout store, not `EditorShell`.** ⛔ **Do not move it.** ✅ **[I-0251]
  reads and writes it where it lives.**
- ⚠️ **`EditorShell.cpp:124`'s comment names a member that does not exist.** ✅ **Fix the comment as
  part of T-0553** — ⛔ **it is what misled [R-Q5].**
- ⚠️ **The rig is READ-ONLY for source** (`feedback_rig_is_read_only`) — ✅ **edit on macOS, push, pull.**
- ⚠️ **Find how the app ALREADY does it first** (`feedback_look_for_existing_pattern_first`) — ✅ **SP-118's
  dominant defect.** ⚠️ **`InspectorLayoutStore`'s `setSelectedTab` is the EXACT pattern [I-0251] needs.**

---

## Progress log

- **2026-09-27** — ✅ **Sprint created and activated.** ✅ **Three Tasks issued (T-0551/0552/0553).**
  ⚠️ **Pre-planning measurement corrected [R-Q5]'s mechanism:** ⛔ **`inspectorVisible_` /
  `timelineVisible_` DO NOT EXIST** — ✅ **visibility is read off the widgets, so the carve-out CREATES
  the state rather than moving it.** ✅ **Recorded in [EP-043] §Rulings as an amendment, not silently.**
- **2026-09-27** — ✅ **ALL THREE TASKS IMPLEMENTED; ALL EIGHT ACs MET.** ⚠️ **Not user-Verified.**

---

## ✅ What was built

| File | Change |
| ---- | ------ |
| `src/ProjectSession.hpp` | ✅ **NEW** — per-project state; ⛔ **no widgets, no logic.** ✅ **Explicitly non-copyable/non-movable** |
| `src/OpenProjectRegistry.hpp/.cpp` | ✅ **NEW** — `projectID` → session ([R-Q2]); ⛔ **empty ID never registered** |
| `src/EditorShell.hpp` | ✅ **Seven state members became REFERENCES into `session_`** |
| `src/EditorShell.cpp` | ✅ **+22 lines, ⛔ 0 deletions** — register/deregister, visibility → session, restore at load |
| `src/InspectorLayoutStore.hpp/.cpp` | ✅ **`inspectorHidden()` / `setInspectorHidden()`** ([I-0251]) |
| `src/SceneInspector.hpp` | ✅ **`storedInspectorHidden()` / `setStoredInspectorHidden()`** — ⚠️ **the panel owns the document** |
| `tests/inspector_layout_smoke.cpp` | ✅ **+81 lines, ⛔ 0 deletions** — sections 1b + 1c |
| `CMakeLists.txt` | ✅ **Registry added to ALL THREE targets compiling `EditorShell.cpp`** |

### ⚠️ THE KEY IMPLEMENTATION DECISION — reference-bound members

⚠️ **The seven extracted members have ~300 call sites across 2,783 lines.** ⛔ **Rewriting every one to
`session_.projectPath()` would have produced a diff in which a genuine behaviour change was invisible**
— ✅ **and this Sprint's ONE mandate is that it is behaviour-preserving.**
✅ **So the state MOVED to `ProjectSession` and `EditorShell` binds its original member NAMES as
references into it.** ⚠️ **Every existing statement is untouched and provably does what it did.**
⚠️ **IT IS A SEAM, NOT THE END STATE** — ✅ **[SP-146] removes the bindings when each window gets its
own session; ⛔ the `*Ref()` accessors exist only for this and are marked "do not call from new code."**

---

## ✅ Verification — what was actually RUN

| Check | Result |
| ----- | ------ |
| Docker build (shipping image, `TESTS=OFF`) | ✅ **clean, no warnings** |
| `ctest` — ⚠️ **NON-ROOT (uid 1001), tests-on image** | ✅ **641/641 passed, 0 failed** |
| Linux smokes (all 24 binaries, via their `.sh` wrappers) | ✅ **24/24 PASS** |
| `inspector_layout_smoke` specifically | ✅ **30 checks, 0 failures** (⚠️ **was 13 assertion sites, now 21**) |
| `scripts/check-package-boundary.sh` | ✅ **GREEN — AC7** |
| Apple `xcodebuild -scheme ScriviApp build` | ✅ **BUILD SUCCEEDED** |
| ⚠️ **AC2 proven MECHANICALLY** | ✅ **`git diff --stat` on `tests/`: 81 insertions, ⛔ 0 deletions — not one existing assertion altered** |

### ⚠️ THE GUARD WAS PROVEN, NOT ASSUMED

⛔ **A test that passes proves nothing until it can fail.** ✅ **The [I-0251] fix was temporarily
REMOVED and the suite rebuilt:** ⚠️ **`inspector_layout_smoke` went RED with exactly the two right
failures** — *"inspectorHidden was PERSISTED"* and *"a re-opened project reports the inspector as
hidden"* — ✅ **then green again on restore.** ⚠️ **This is the discipline [SP-143]'s D1 used
(`78a739f~1`), and it is why that Sprint's guard was trustworthy.**

### ⚠️ TWO HARNESS ERRORS OF MINE, recorded so they are not mistaken for defects

1. ⛔ **I first ran the smokes with `sh` (dash) and all 24 "failed"** — ⚠️ ***`set: Illegal option -o
   pipefail`***. ✅ **They are bash scripts; rerun with `bash`, 23/23 passed.**
2. ⛔ **`dumas_world_fixture` then "failed"** — ⚠️ **it is a project GENERATOR that refuses a
   pre-existing directory, and I handed it `mktemp -d`.** ✅ **Given a non-existent path it exits 0.**

⚠️ **Neither was a code defect** — ✅ **but both would have read as one in a summary that only reported
counts, which is why they are here.**

### ⛔ WHAT WAS **NOT** VERIFIED

⛔ **NO LIVE PASS ON THE REAL RIG.** ⚠️ **[I-0251] is a *"does it survive a quit?"* defect, and
`feedback_live_pass_finds_what_suites_cannot` is explicit that a green suite cannot answer that.**
✅ **The smoke proves the SCHEMA round-trips; ⛔ it does NOT prove the running app restores the pane.**
⚠️ **That belongs to the user on the rig** — ✅ **and [SP-148] owns the Epic's live pass.**
⚠️ **Confirm the build first** (`scrivi_linux --version`, `feedback_confirm_the_build_under_test`).

### ⚠️ ONE FINDING FOR [SP-146] — ✅ **RULED AND SCOPED 2026-09-27**

⛔ **`registry_` lives on `EditorShell`, and that is wrong for more than one window.** ✅ **User agreed
2026-09-27; ✅ the fix is now scoped in the Epic** → §*The app-level owner*.

⚠️ **THE FINDING WAS BIGGER THAN [SP-145] STATED.** ⛔ **This record first called it [SP-146]'s "first
step," which reads like a preparatory move.** ✅ **MEASURED 2026-09-27: Linux has NO app-level state
owner at all** — ⛔ **no `AppEnvironment` equivalent, no singleton; `appSupportRoot` is a LOCAL IN
`main()` (`main.cpp:87`) hand-threaded into three consumers.** ✅ **So [SP-146] must CREATE Linux's first
app-global owner, which is a real Task, not a preamble.** ⚠️ **[SP-146] is therefore re-scoped as *"the
app object AND the windows."***
✅ **[SP-145]'s reference-binding seam is removed in that Sprint's step 2** — ⚠️ **it was built for
exactly that moment.**

### ⛔ A SECOND "FINDING" THIS SPRINT RAISED AND THEN RETRACTED — the bridge duplication

⚠️ **[SP-145] observed TWO `ScriviBridge` instances (`Landing.qml:32`, `EditorShell.cpp:69`), each
calling `bootstrap()`, and I proposed filing an Issue.** ⛔ **INVESTIGATED 2026-09-27: NOT A DEFECT, and
NO ISSUE WAS FILED.** ✅ **Benchmarked through the C ABI in the container — the second
`scrivi_ensure_local_identity` costs `0.024–0.037 ms` against the first's `0.78–5.0 ms`;
✅ `CoreSingleton` is one per process so both bridges share one `SecureStore` and one identity;
✅ and their two `errorOccurred` handlers are DELIBERATELY different surfaces (the editor's needs a
`QueuedConnection` because its calls run on a worker, T-0499).** ⛔ **Merging them would be a
regression.**
⚠️ **WORTH RECORDING AS A LESSON:** ✅ **"two instances of a boundary class" RESEMBLES the duplication
class this Epic's siblings keep finding ([I-0215], [I-0241], [I-0242])** — ⛔ **and a resemblance is not
a defect.** ✅ **A measurement is what told them apart; ⛔ filing on the resemblance would have spent a
Sprint's attention on nothing.** ⚠️ **Full detail in the Epic's §The app-level owner.**
