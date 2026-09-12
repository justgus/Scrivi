# SP-128 — `[Linux]` ⚠️ **Honest waiting: progress for slow reads, and a name for an absent world**

**Status:** ✅ **CLOSED 2026-09-11 — user-approved.** ✅ **All three Tasks VERIFIED (T-0499, T-0500, T-0501); all DoD items ticked with MEASURED evidence.** ⚠️ **The live pass found TWO defects in this sprint's own work — [I-0198] and [I-0199] — and 23/23 smokes were green through BOTH**, ✅ **which is the sprint's most useful finding.** ⚠️ **[I-0196]'s load cost is NOT this sprint's and is carried by [EP-039].**
**Epic:** [EP-038](../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig · ⚠️ **sprint 3 of 3**
**Codebase:** `[Linux]` — ⚠️ **Qt/C++ only.** ✅ **No ScriviCore change is expected** (see §4).
**Date Created:** 2026-09-10
**Closes:** ⚠️ **No new EP-038 AC** — ✅ **AC1–AC7 are ALL CLOSED.** ⚠️ **This Sprint exists by USER RULING
(2026-09-10) to finish the two Issues the rig work SURFACED but did not fix.**
**Tasks:** **T-0499 – T-0501** (three) · **Next available:** T-0502 · **Issues:** ✅ **[I-0195]** (Open),
✅ **[I-0182]** (Resolved - Not Verified)

⚠️ **WHY THIS SPRINT EXISTS, stated plainly:** ⚠️ **EP-038's goal — stand up the rig, establish drive-loss
ground truth — IS ACHIEVED**, ✅ **and all seven ACs are closed.** ⚠️ **But the rig did what a rig is for:
it found defects a suite structurally cannot.** ⚠️ **Two of them are still owed** — ✅ **the user ruled
2026-09-10 that they finish HERE rather than under a new Epic**, ⚠️ **because they are the rig's own
findings and splitting them from it would lose the reason they exist.**

---

### 1. ⚠️ The two Issues, and why they are ONE Sprint

| | Issue | ⚠️ State | ✅ What it needs |
| - | ----- | -------- | --------------- |
| **[I-0195]** | ⚠️ **Project open BLOCKS the UI for the whole read, with no progress** | 🔵 **Open — UNBUILT** | ⚠️ **A determinate PROGRESS UI** |
| **[I-0182]** | ⚠️ **An unreachable world was named by its raw UUID in the status line** | 🟢 **Resolved - Not Verified** | ✅ **A RIG LIVE PASS with the share down** |

✅ **They belong together because they are the SAME MOMENT in the writer's experience:** ⚠️ **opening a
project whose world is on slow or absent storage.** ⚠️ **[I-0195] is what the writer sees while WAITING;
[I-0182] is what the writer reads when the wait ENDS BADLY.** ⚠️ **Shipping one without the other leaves
that moment half-honest.**

---

### 2. ✅ [I-0195] — ⚠️ **the design is ALREADY RULED, and it is the user's**

⚠️ **DO NOT REDESIGN THIS.** ✅ **The user ruled it on 2026-09-08 and the ruling is load-bearing:**

> ✅ **Waiting is ACCEPTABLE. ⚠️ Waiting with a FROZEN, SILENT UI is NOT.**

✅ **AND THE USER SUPPLIED THE MECHANISM:** ⚠️ **the FILE COUNT is known EARLY — before the bulk of the
reading — for both the project and each bound world.** ✅ **So `files read / files to read` is a GENUINE
DETERMINATE PERCENTAGE**, ⚠️ **not a spinner pretending to be one.**

⚠️ **THE PREREQUISITE ALREADY EXISTS AND MUST BE REUSED, NOT RE-DERIVED:**
✅ **`platforms/linux/src/AsyncCall.hpp`** — ⚠️ **built for [I-0193], proven on the rig, and the reason
this Sprint is cheap.** ⚠️ **[I-0193] and [I-0195] were ALWAYS one root cause** (⚠️ **no async path for
world/project reads**) — ✅ **[I-0193] paid for the machinery; [I-0195] spends it.**
⚠️ **Do NOT write a second threading mechanism** (`feedback_look_for_existing_pattern_first`).

⚠️ **THE DISTINCTION THAT MUST NOT BLUR:**

| | ⚠️ Case | ✅ Needs | ⚠️ Why not the other |
| - | ------- | -------- | ------------------- |
| **[I-0193]** ✅ done | ⚠️ **Volume UNREACHABLE** | ⚠️ **A TIMEOUT** | ⚠️ **A progress bar would never finish** |
| **[I-0195]** | ⚠️ **Volume REACHABLE but SLOW** | ⚠️ **PROGRESS** | ⚠️ **A timeout would ABORT a legitimate slow load** |

⚠️ **MEASURED, so the target is not guesswork** (rig, share healthy, `cache=none`): ⚠️ **`0.47 s` to list
`Eskandar.scrivworld` recursively; `1.49 s` to read its `57` files.** ⚠️ **AND THE COST ONLY GROWS, along
three axes the user named:** ⚠️ **(1) a world accumulates objects over a project's life** — ✅ **Eskandar
is 57 files and is a TEST world**; ⚠️ **(2) a project may bind MORE THAN ONE world**, each paying it;
⚠️ **(3) project AND worlds may BOTH sit on slow network storage.**

⚠️ **`cache=strict` MASKS this and must not be mistaken for a fix.** ⚠️ **An earlier reading of mine
called this "mount configuration, not an app defect" — ✅ THAT WAS WRONG and is withdrawn**: the app
blocks the UI for the whole read *whatever* the reason for the slowness.

---

### 2a. ✅ **THE COUNT IS KNOWABLE — user ruling, 2026-09-10.** ⚠️ **This question is CLOSED.**

⚠️ **The planning draft carried a risk that the file count might not be knowable early, and said to raise
it rather than ship a spinner.** ✅ **THE USER RULED THE RISK AWAY, and the reasoning is load-bearing:**

✅ **FILESYSTEM CALLS ARE DETERMINISTIC BY NATURE.** ⚠️ **A directory listing returns what is there; it
does not estimate.** ✅ **So `files read / files to read` is a real fraction, available before the
expensive part begins.**

✅ **AND THE FILESYSTEM IS UNDER OUR CONTROL.** ⚠️ **We are NOT searching an unknown, user-facing tree** —
✅ **a `.scrivworld` package is a layout WE define, create and write** (`WorldStore::createWorld` builds
the skeleton; Doc: *Scrivi_Project_Package_Structure*). ⚠️ **Its shape is not a discovery problem.**

✅ **THE FALLBACK ORACLE, if the listing ever disappoints:** ⚠️ **read the MASTER INDEX** — `index.json`
carries the object set, ✅ **so the count can come from the index rather than from walking the tree.**
⚠️ **Either way the number is KNOWN, not estimated.**

#### ⚠️ **The ONE case that would change this — and how to recognise it**

⚠️ **A STATELESS NETWORK PROTOCOL (HTTPS or similar) is the only thing the user identified that would make
the count non-deterministic.** ✅ **AND IT WOULD ANNOUNCE ITSELF:** ⚠️ **we would be the ones adding it,
so its stateless nature would be KNOWN AT DESIGN TIME**, ✅ **and the choice between an indeterminate or
PARTLY determinate bar could be ruled CONCRETELY at that point.**

⚠️ **This is NOT a licence to ship indeterminate progress today.** ✅ **No such protocol is in play** —
⚠️ **`cifs`/local disk are stateful filesystems** — ✅ **so the determinate bar is the requirement, and a
spinner would be a REGRESSION against a settled ruling.**

---

### 2b. ⚠️ **THE TEST WORLD — a fixture T-0501 CANNOT RUN WITHOUT**

⚠️ **USER-OWNED, in progress 2026-09-10.** ✅ **The user is building a world that legitimately takes a
long time to load**, ⚠️ **because without one T-0501 cannot verify [I-0195] AT ALL.**

⚠️ **WHY THIS IS NOT OPTIONAL, and why it is not a shortcut either:** ✅ **the fix must be watched under a
load that is GENUINELY slow, not artificially delayed.** ⚠️ **A `sleep` in the read path would prove only
that a progress bar animates** — ✅ **it would NOT exercise the real cost structure** (⚠️ **one
`openScene` per scene, sequential, each a round trip over the mount**), ⚠️ **which is what determines
whether the fraction moves smoothly or sits still and then jumps.**

#### ✅ What makes a world slow — the three axes the user already named ([I-0195])

| Axis | ⚠️ Effect on load |
| ---- | ---------------- |
| ⚠️ **Object COUNT** | ✅ **The dominant one.** ⚠️ **Eskandar is `57` files and is a TEST world** — a real one grows over a project's life |
| ⚠️ **SCENE count** | ⚠️ **This is what the PROGRESS BAR counts** — ✅ **one `openScene` per scene, so `n` scenes is `n` round trips** |
| ⚠️ **Mount latency** | ✅ **`cache=none,actimeo=1,closetimeo=1`** — ⚠️ **the rig measured `0.47 s` to list Eskandar and `1.49 s` to read its 57 files under it** |

⚠️ **THE PROGRESS BAR COUNTS SCENES, NOT OBJECTS** — ✅ **so a world with thousands of objects but a
handful of scenes will still open with a bar that jumps 0→100%.** ⚠️ **To watch the FRACTION MOVE, the
PROJECT needs many SCENES**; ✅ **to make each step slow, the WORLD needs bulk and the mount needs
latency.** ⚠️ **Both halves are needed, and they are different knobs.**

#### ⚠️ Guard rails

⚠️ **NEVER build this from real writing work** (`feedback_never_drive_synthetic_input_at_real_work`) —
✅ **generate it, or copy a disposable one.** ⚠️ **The `tintagael`/`Eskandar` rig is REAL WORK: back up
first** (`project_test_rig_tintagael_eskandar`).

#### ✅ **THE GENERATOR EXISTS** — `platforms/linux/tests/dumas_world_fixture.{cpp,sh}` (2026-09-10)

✅ **Setting: ALEXANDRE DUMAS' FRANCE** — ⚠️ **public domain (Dumas d. 1870)** — drawn from *Les Trois
Mousquetaires*, *Le Vicomte de Bragelonne* (*The Man in the Iron Mask*) and *Le Comte de Monte-Cristo*.
✅ **Chosen because the three novels share ONE France**, ⚠️ **so the relationships are REAL rather than
random** — the same king, the same Paris, the same regiment. ⚠️ **Names come from the NOVELS only**: no
modern adaptation, no translation-specific coinage, no invented expansion material.

| Scale | Objects | Scenes | Files on disk |
| ----- | ------- | ------ | ------------- |
| **1** (default) | **224** | **384** | **~1,000** |
| **3** | **672** | **1,152** | **~3,000** |

⚠️ **For comparison, `Eskandar` — the world that exposed [I-0195] — is `57` files.**

✅ **MEASURED: 446 ms to build scale 1 on local disk** — ⚠️ **which is the point of the warning it prints:
BULK ALONE IS FAST.** ⚠️ **The world must sit on the HIGH-LATENCY MOUNT for the defect to appear**, so
pass the world path explicitly:

    dumas_world_fixture.sh "$BIN" ~/ScriviProjects/dumas.scrivi \
        /mnt/scrivi-net/worlds/Dumas-France.scrivworld 3

⚠️ **Kinds are DERIVED from the core via `ObjectKindScope`** — ✅ **the fixture names no kind list and a
kind the core does not know is SKIPPED, not invented** (CLAUDE.md's standing rule).
⚠️ **Relationships use ONLY the seed relation types and RESPECT their constraints** (`sibling-of` is
character→character), ✅ **since a violated constraint is rejected and would leave the graph thinner than
the printed counts claim.**
✅ **It REFUSES to run against an existing directory** (exit 3) — ⚠️ **verified**, and the guard is there
because this must never be pointed at real work (I-0150).

---

### 3. ✅ [I-0182] — ⚠️ **fixed, and NOT YET SEEN BY A WRITER**

✅ **The fix is in** (2026-09-10): ⚠️ **`worldNames_` is populated ONLY when `listWorlds` SUCCEEDS**, so a
dead volume left it empty and the status line named the world by its raw `worldID`. ✅ **`worldPaths_` now
caches `lastKnownPackagePath`** (⚠️ **emitted regardless of status — T-0419/[I-0137]**) ⚠️ **and is
deliberately NOT cleared on a failed reload.** ✅ **The line now reads `World "Eskandar" is offline`.**

⚠️ **BUT THE BRANCH HAS NEVER EXECUTED IN FRONT OF A WRITER.** ⚠️ **It requires `listWorlds` to FAIL,
which needs a REAL dead share** — ✅ **Docker cannot stage one** (⚠️ **the same structural blindness that
hid [I-0193]'s timeout branch until a rig pass**). ⚠️ **22/22 smokes green is NOT evidence here.**

⚠️ **THIS ISSUE'S ORIGINAL DIAGNOSIS WAS WRONG and the correction is worth carrying:** ⚠️ **it described
an OBJECT named by its objectID.** ✅ **The user corrected it 2026-09-10** — ⚠️ **object names are CORRECT
(the core fills them from the binding's `cachedIndex`), the double-click dialog is a PLACEHOLDER, and the
real surface is the STATUS LINE, which names a WORLD.**

---

### 4. ⚠️ Scope — what this Sprint is NOT

| Item | ⚠️ Where it goes |
| ---- | --------------- |
| ⚠️ **Any ScriviCore change** | ✅ **Not expected.** ⚠️ **The file count comes from the LISTING the app already performs; if the core must expose a count, that is a FINDING to raise, not a silent widening** |
| ⚠️ **Linux session persistence** ([I-0176]/[I-0177]/[I-0178]) | ⚠️ **NOT here.** ✅ **Apple solved all three in EP-018; the Linux equivalents want their OWN Epic** |
| ⚠️ **Apple parity for the progress UI** | ⚠️ **Out of scope** — ✅ **Apple does not exhibit [I-0195]** |
| ⚠️ **Retiring `AsyncCall`'s timeout** | ⚠️ **NOT retired** — ✅ **[I-0193]'s case is REAL and distinct** |

---

### 4a. Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0499** | ⚠️ **`[Linux]` Get project open OFF the UI thread** — [I-0195]. ✅ **Reuse `AsyncCall`; ⚠️ NO second threading mechanism.** ⚠️ **The writer must stay able to read what is already loaded** | **High** | 🔵 **Not started** |
| **T-0500** | ⚠️ **`[Linux]` DETERMINATE progress for project + world reads** — [I-0195]. ✅ **`files read / files to read`, from the count known EARLY** — ⚠️ **not a spinner.** ⚠️ **If the count proves NOT knowable early, RAISE IT rather than shipping a fiction** | **High** | 🔵 **Not started** — ⚠️ **depends on T-0499** |
| **T-0501** | ⚠️ **LIVE PASS on the real rig** — ⚠️ **[I-0182]'s renamed world READ with the share DOWN**, ⚠️ **and [I-0195]'s progress WATCHED on a `cache=none` mount.** ✅ **Plus `ctest` + smokes non-root on the rig** | **High** | 🔵 **Not started** — ⚠️ **the only thing that can VERIFY either Issue** |

⚠️ **T-0501 is NOT ceremony.** ⚠️ **[I-0182]'s branch requires `listWorlds` to FAIL and [I-0195]'s cost
requires a SLOW mount** — ✅ **neither is stageable in Docker or in a suite**, ⚠️ **which is exactly how
[I-0193]'s freeze reached a writer in the first place.**

---

### 5. Definition of Done

- [x] ✅ **[I-0195] — project open does NOT freeze the UI**, ✅ **and shows a DETERMINATE
      `files read / files to read` percentage**, ⚠️ **not a spinner.** ✅ **DETERMINATE IS THE RULING, NOT
      AN ASPIRATION** (§2a) — ⚠️ **an indeterminate bar here would be a REGRESSION AGAINST A SETTLED
      DECISION, not a pragmatic fallback.**
- [x] ✅ **Built on `AsyncCall`** — ✅ **no second threading mechanism.**
- [x] ✅ **A LIVE PASS ON THE REAL RIG — DONE 2026-09-11, build 42, and it FOUND TWO DEFECTS IN THIS SPRINT'S OWN WORK ([I-0198], [I-0199]) that 23/23 smokes were green through.** — ⚠️ **[I-0182]'s renamed world is READ BY A WRITER with the
      share DOWN**, ⚠️ **and [I-0195]'s progress is WATCHED on a slow (`cache=none`) mount.**
      ⚠️ **Neither is provable from Docker or from a suite.**
- [x] ✅ **`ctest` + Linux smokes GREEN on the rig, non-root — MEASURED 2026-09-11: `590/590` ctest, `23/23` smokes.** — ⚠️ **not merely in the container**
      (`project_linux_container_tests_off`).
- [x] ✅ **The build under test is CONFIRMED — `build 42 (2026-09-11 22:35:36 UTC), Qt 6.10.2`, read from the binary ON the rig.** (`scrivi_linux --version` / Help ▸ About) before the pass
      (`feedback_confirm_the_build_under_test`).

---

### 6. ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ✅ **~~The file count is NOT actually known early~~ — RETIRED 2026-09-10 by user ruling** | ✅ **FILESYSTEM CALLS ARE DETERMINISTIC BY NATURE**, ⚠️ **and the filesystem in question is UNDER OUR CONTROL** — ✅ **a `.scrivworld` package WE define and write, not an unknown user-facing tree we search.** ⚠️ **See §2a: the count is knowable, and `index.json` is the fallback oracle if the listing ever disappoints.** |
| ⚠️ **A progress dialog becomes a NEW blocking modal** | ⚠️ **The point is a RESPONSIVE UI.** ✅ **The writer must be able to keep reading what is already loaded** |
| ⚠️ **The rig sleeps / is powered off** | ✅ **EXPECTED — the operator powers it off at night** (WOL settled 2026-09-10, not pursued). ⚠️ **`ssh` timing out is not a fault to diagnose** |
| ⚠️ **The rig runs a stale binary** | ✅ **Bump on the Mac → commit → push → pull → rebuild** (`platforms/linux/bump-build-stamp.sh --commit`). ⚠️ **The rig is READ-ONLY for source** |
| ⚠️ **Progress percentages that jump or stall** | ⚠️ **A determinate bar that lies is worse than an honest indeterminate one** — ✅ **if the count cannot be trusted, say so rather than animating a fiction** |

---
---
