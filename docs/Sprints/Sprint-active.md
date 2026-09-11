# Active Sprints

⏸️ **SP-124 is PAUSED** (EP-038, sprint 2 of 2) — ⚠️ **the DRIVE PULL.** ✅ **Paused 2026-08-31 by user
ruling, NOT blocked** — ⚠️ **its S1 baseline is captured and the rig's `cifs` mount is left INTACT**, so
it resumes without redoing setup.

✅ **SP-127 CLOSED 2026-09-02** (user-approved) → [`Closed/Sprint-SP-127.md`](Closed/Sprint-SP-127.md) — **EP-035 AC3** delivered. ⚠️ **Its live pass found FOUR defects, all Verified.**

⚠️ **NO SPRINT IS CURRENTLY ACTIVE.** ⏸️ **SP-124 is PAUSED and resumable** — ✅ **its gate is now satisfied: SP-127 delivered the Worlds surface it was waiting for.**

⚠️ **WHY THE SWAP** (user, 2026-08-31): ✅ ***"the App won't incorrectly represent the mount point until
it can correctly represent the mount point."*** ⚠️ **Linux has NO world surface at all** —
`addWorld`/`relinkWorld`/`getWorldStatus`/`getWorldBinding` are bridged with ⚠️ **ZERO callers**.
⚠️ **So a drive-loss sprint could measure OS signals but could never SHOW them to a writer**, and
⚠️ **I-0181 was re-scoped to a LATENT CORE defect once that was seen.** ✅ **Building the surface first
means SP-124's scenarios get verified against something a writer can actually read.**

✅ **THE SWAP WAS VINDICATED.** ⚠️ **SP-127's live pass found a 🔴 DATA-LOSS defect (I-0183) that only
a real degraded mount could trigger** — ⚠️ **10 of 12 relationships destroyed in the user's real
project** — ⚠️ **and it was found THROUGH the new surface, by a writer using it.** ⚠️ **A drive-loss
sprint run first would have measured the OS correctly and never seen it.**

⚠️ **I-0181 is now worth revisiting**: the Worlds dialog is exactly where a false `missing` would
become visible to a writer, which is the condition the Issue was parked on.

✅ **SP-126 CLOSED 2026-08-30** → [`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) — the three-tab
Scene Inspector shell, ⚠️ **closing EP-035 AC10** and retiring the "Scene Entities" placeholder.

✅ **SP-125 CLOSED 2026-08-29** → [`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) — the first
Linux object surface.

✅ **SP-123 CLOSED 2026-08-29** → [`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) — the rig is
BUILT, GREEN, and runs the app on real hardware.

### ⚠️ What is available to activate next

| Candidate | Epic | State |
| --------- | ---- | ----- |
| **AC3** — world binding + ⚠️ **the RELINK UI that does not exist** | EP-035 | ⚪ Not planned — ⚠️ **a moved world currently cannot be repaired from the app at all** |
| **AC4** — object CRUD | EP-035 | ⚪ Not planned |
| ⚠️ **Session persistence** (I-0176/0177/0178) | ⚠️ **NO EPIC** | ⚠️ **Apple's EP-018 equivalent — wants its own Epic** |
| **I-0180** — `[Apple]` object-card label | ⚠️ **NO HOME** | ⚠️ **EP-034 is closed** |

---
---

## SP-128 — `[Linux]` ⚠️ **Honest waiting: progress for slow reads, and a name for an absent world**

**Status:** 🟡 **ACTIVE — activated 2026-09-10 by user approval.**
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

- [ ] ⚠️ **[I-0195] — project open does NOT freeze the UI**, ✅ **and shows a DETERMINATE
      `files read / files to read` percentage**, ⚠️ **not a spinner.** ✅ **DETERMINATE IS THE RULING, NOT
      AN ASPIRATION** (§2a) — ⚠️ **an indeterminate bar here would be a REGRESSION AGAINST A SETTLED
      DECISION, not a pragmatic fallback.**
- [ ] ⚠️ **Built on `AsyncCall`** — ✅ **no second threading mechanism.**
- [ ] ⚠️ **A LIVE PASS ON THE REAL RIG** — ⚠️ **[I-0182]'s renamed world is READ BY A WRITER with the
      share DOWN**, ⚠️ **and [I-0195]'s progress is WATCHED on a slow (`cache=none`) mount.**
      ⚠️ **Neither is provable from Docker or from a suite.**
- [ ] ⚠️ **`ctest` + Linux smokes GREEN on the rig, non-root** — ⚠️ **not merely in the container**
      (`project_linux_container_tests_off`).
- [ ] ⚠️ **The build under test is CONFIRMED** (`scrivi_linux --version` / Help ▸ About) before the pass
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

## SP-124 — `[Linux]` ⚠️ **Drive-loss ground truth + the Linux refinement**

**Status:** ✅ **COMPLETE 2026-09-10** — ⚠️ **AWAITING USER APPROVAL TO CLOSE.** ✅ **All FOUR Tasks VERIFIED and ARCHIVED** (T-0477, T-0478, T-0479, T-0498). ✅ **All FOUR ACs CLOSED** (EP-038 AC4–AC7). ✅ **All DoD items settled** — ⚠️ **three of them by USER RULING rather than by more work** (see below). ✅ **Issues: [I-0193] VERIFIED; [I-0194] and [I-0181] RESOLVED.** ⚠️ **[I-0195] remains OPEN and is NOT this sprint's to close** — ✅ **it is the REACHABLE-BUT-SLOW case needing PROGRESS, distinct from [I-0193]'s UNREACHABLE case needing a TIMEOUT** — ⚠️ **and it now has [I-0193]'s `AsyncCall` machinery to build on rather than re-derive.** ⚠️ **[I-0182] also remains open.**

### ✅ What this sprint actually proved — ⚠️ **and what it cost to find out**

⚠️ **THE OBVIOUS SIGNALS LIE — FIVE OF THEM** (`mountpoint -q`, a directory listing with zeroed sizes, `statvfs`, a transiently successful `read`, ⚠️ **and a held FD that survived `umount -l` + `losetup -D`**). ✅ **That is the Linux counterpart to Apple's `volumeIsRemovable == false` on a hand-unplugged drive**, ⚠️ **and finding it is the POINT of a real rig.**

✅ **`offline` was EMITTED FOR THE FIRST TIME IN THIS PROJECT'S HISTORY** — ⚠️ **it had been a documented lie, an enum value no code path could produce.**

⚠️ **THE LIVE PASS CONDEMNED THE FEATURE IT PROVED, IN THE SAME SESSION** — ✅ **and that is the sprint's most valuable output.** ⚠️ **Two defects were filed against T-0478's OWN output by T-0478's OWN pass, NEITHER findable from any suite** ([I-0193] 102 s freeze → Force Quit; [I-0194] a six-`../` writer-facing path). ⚠️ **A THIRD ([I-0181]) was a wrong `missing` on an unmounted volume.** ✅ **All three are fixed.**

⚠️ **THREE RECURRING LESSONS, each earned the hard way here:**
1. ⚠️ **An Issue that names TWO call sites is not closed by fixing ONE** — ✅ **however well the first is proven** ([I-0193] took two rig passes).
2. ⚠️ **A finding must state HOW it was gathered** — ✅ **"the user watched the screen" is legitimate evidence, and here it BEAT instrumented output pointed at the wrong machine.**
3. ⚠️ **A caveat written in a plan is not a caveat honoured in code** — ✅ **T-0498's polarity was implemented BACKWARDS despite the sprint predicting exactly that trap; the CONTROL TEST caught it.**
**Epic:** [EP-038](../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig · **sprint 2 of 2**
**Codebase:** `[Linux]` — ⚠️ **one Qt/C++ deliverable (T-0478); T-0477 ships NO code**
**Date Activated:** 2026-08-31
**Closes:** **AC4, AC5, AC6, AC7**
**Tasks:** **T-0477 – T-0479** (three) + ⚠️ **T-0498** (`[ScriviCore]`, see §3a) · **Next available:** T-0499 · Issues **I-0181** (⚠️ **now ASSIGNED — T-0498**), ⚠️ **I-0182** (found by the SP-127 live pass; see §2c) · ⚠️ **I-0193**, ⚠️ **I-0194** and ⚠️ **I-0195** (✅ **filed 2026-09-08 from T-0478's LIVE PASS**; see §3b)

⚠️ **GATE SATISFIED:** SP-123 closed 2026-08-29 — the rig is reachable, builds natively, and runs the
app on a real display.

---

## 1. Sprint Goal

⚠️ **Find out what Linux ACTUALLY reports when a volume carrying a world goes away** — then implement
`WorldVolumeStatus` for Linux **against those findings**, and ⚠️ **correct the Porting Outline's §9
from what the rig taught**, so the Windows rig executes a recipe rather than re-deriving one.

⚠️ **The sprint's whole value is the ORDER.** ✅ **Instrument, THEN implement.** A refinement written
from documentation is exactly what this Epic exists to prevent.

---

## 2. ⚠️ SCOPE WIDENED AT ACTIVATION — user ruling 2026-08-31

⚠️ **The plan said "pull a USB drive." The user challenged that framing, correctly, and the scope is
now THREE scenarios rather than one.**

### ⚠️ The user's point, and where it holds

> *"A remote drive is indistinguishable in the Operating system from a USB removeable drive. Both
> require mount/umount protocols. Both will interface to a kernel based `/dev/*` device."*

✅ **TRUE at the mount layer** — and it exposes a real gap in the original plan: ⚠️ **`WorldStatus::offline`
is DEFINED by the network case**, and the sprint as written would have shipped it **untested**. On
Apple, `offline` is what `volumeIsLocal == false` detects. ⚠️ **A USB-only sprint proves `unmounted`
and leaves `offline` a guess.**

### ⚠️ Where it does NOT hold — and why the physical pull SURVIVES

⚠️ **A clean `umount` and a physical yank are different events, and they differ in the one place this
Epic cares about:**

| | Clean `umount` | ⚠️ **Device vanishes under a live mount** |
| - | -------------- | --------------------------------------- |
| Open FDs | closed in order | ⚠️ **`EIO` / `ESTALE`** |
| `/proc/mounts` | entry removed | ⚠️ **may persist as STALE** |
| Writes in flight | flushed | ⚠️ **may be TORN** |
| `umount` itself | succeeds | ⚠️ **may refuse — `EBUSY`** |

⚠️ **A clean unmount cannot produce a stranded FD**, and a stranded FD is the state that cost Apple six
Issues. ✅ **So the physical pull is NOT replaced — it is JOINED.**

### ✅ The three scenarios T-0477 must instrument

| # | Scenario | ⚠️ What it is the ONLY source of |
| - | -------- | -------------------------------- |
| **S1** | **Clean `umount`** of a network share | The orderly baseline — ⚠️ **what "gone" looks like when nothing is wrong** |
| **S2** | ⚠️ **Network share killed AT THE SOURCE** (serving host stops sharing / drops off) | ⚠️ **`offline`'s REAL evidence** — ✅ **and this scenario is the user's contribution; the original plan had no test for it at all** |
| **S3** | ⚠️ **PHYSICAL USB yank** on `oathkeeper` | ⚠️ **Stranded FDs, stale mount entries, torn writes.** ⚠️ **AC4 says "PHYSICAL loss is OBSERVED" and means it** |

⚠️ **S2 is NOT a substitute for S3 and S3 is NOT a substitute for S2.** ⚠️ **Recording one and
inferring the other is the exact failure mode this Epic was created to stop.**

### ⚠️ Two facts about the rig that shape the setup

- ⚠️ **`ScriviWorlds` is an AFP volume.** ⚠️ **`oathkeeper` likely cannot mount it at all** — AFP is
  effectively dead on modern Linux (`afpfs-ng` is unmaintained). ✅ **Do NOT plan around AFP.** Use
  **SMB** (`cifs-utils`) or **NFS** from the Mac or the Windows box; ⚠️ **the protocol is a variable to
  RECORD, not a detail to gloss** — `cifs` and `nfs` fail differently, and `nfs` hard-mounts hang where
  `cifs` returns errors.
- ✅ **The rig SLEEPS, exactly as §1 says.** ⚠️ **`uptime` reading 1d 13h on return is NOT evidence
  against sleep** — ⚠️ **suspend-to-RAM does not stop the kernel clock**, and `uptime` counts
  wall-clock since BOOT regardless of suspend. ⚠️ **An intermediate claim in this session that "it was
  never asleep" was WRONG and has been withdrawn** (user correction, 2026-08-31); ✅ **§1's heuristic
  stands and needs no correction.**
- ⚠️ **The WOL question is OPEN and is the USER's to settle** — `nmcli` reports `wake-on-lan: --`
  (unset) and `ethtool` needs `sudo`. ⚠️ **Magic packets to `255.255.255.255` and `192.168.1.255`
  (ports 9/7/0) drew no response**, ✅ **which is consistent with WOL simply being disarmed.**

---

## 2b. ✅ PRELIMINARY CONTAINER PASS (2026-08-31) — ⚠️ **and what it did NOT do**

⚠️ **While the rig was unreachable, a NARROW question was put to a real Linux kernel** in a privileged
container on a loopback ext4 mount: ✅ **how does Linux report MOUNT STATE through the path API?**

⚠️ **Admissible because that question is about KERNEL SEMANTICS, not hardware.** ⚠️ **It closes NO AC,
and T-0478 remains HARD-GATED on the real session.** → `platforms/linux/tools/T-0477-PRELIMINARY-container.md`

| ✅ Finding | ⚠️ Why it matters |
| --------- | ---------------- |
| ⚠️ **`statvfs` LIES** — ⚠️ **it SUCCEEDS on an unmounted path**, silently reporting the ROOT filesystem (14325 → 15300561 blocks) | ⚠️ **Linux's `volumeIsRemovable`, and WORSE**: Apple's lie was a wrong boolean; this is a CONFIDENT SUCCESS with a plausible number |
| `exists` / `isdir` / `listdir` also lie | ⚠️ **The mountpoint DIRECTORY survives** — empty ≠ absent |
| ✅ **`st_dev` vs the parent's WORKS** — and `/proc/mounts` works | ⚠️ **CAVEAT: a dir that NEVER held a mount also matches.** ⚠️ **`st_dev` proves "not a mount NOW", NOT "a volume went away"** |
| ⚠️ **A held FD survived `umount -l` + `losetup -D` ENTIRELY** — read AND write kept succeeding while the PATH broke instantly | ⚠️ **An open FD and a path can disagree COMPLETELY** — ✅ **so "the writer's editor still works" is NOT evidence the volume is present.** ⚠️ **Do NOT generalise to a yank: that is S3's to measure** |

### ⚠️ **I-0181 — the finding that justifies the whole pass**

⚠️ **An unmounted Linux volume is reported `missing`.** The core's rule is *package absent AND parent
exists* (`WorldStore.cpp:330-348`); ⚠️ **an unmounted Linux volume satisfies BOTH.**

⚠️ **MEASURED ON BOTH PLATFORMS:** macOS **removes** `/Volumes/<name>` (parent absent → honest
`unavailable`); ⚠️ **Linux KEEPS the mountpoint** (parent present → ⚠️ **false `missing`**).

✅ **The same core rule is correct on Apple and WRONG on Linux, and the core cannot see the difference.**
⚠️ **This is the strongest possible argument for the Epic's premise**: ⚠️ **porting Apple's file by
reading it would have shipped this defect.** ⚠️ **`missing` is the one status that invites DESTRUCTIVE
remedies against an intact world** (Doc 2 §7.2.1) — ⚠️ **[[I-0115]]'s class, re-earned via a new trigger.**

⚠️ **NOT FIXED — but now ASSIGNED: [T-0498](#3a-t-0498--the-core-resolution-fix-for-i-0181).**
⚠️ **S3 must first establish whether the mountpoint survives a PHYSICAL yank** — udisks2 may remove it,
which changes which branch fires. ⚠️ **That is why T-0498 is GATED on T-0477, not started alongside it.**

---

## 2c. ⚠️ SMB OBSERVATIONS FROM THE SP-127 LIVE PASS (2026-09-01) — ⚠️ **rig findings, NOT S3**

⚠️ **These came out of the SP-127 Worlds-surface live pass, not from SP-124's instrumentation.** ⚠️ **They
are recorded here because SP-124 owns drive-loss ground truth** — ✅ **but they are `cifs` findings and
⚠️ they DO NOT satisfy S3, which is a PHYSICAL yank of a LOCAL device.** ⚠️ **Do not let them stand in
for it.**

**Setup:** the Worlds volume was shared from the Mac over SMB (`scripts/mount-shares-on-rig.sh`) and
pulled ON THE MAC while the rig held it mounted — ⚠️ **a source-side disappearance, which is closest to
S2, not S3.**

### ✅ What the app did — ⚠️ **and a correction**

✅ **The app did NOT hang.** ⚠️ **An intermediate claim in this session that it did was MY MISREADING of
the user's words and is WITHDRAWN** (user correction, 2026-09-01): *"The app did not hang."* ✅ **It
stayed responsive and reported the world unavailable** — ⚠️ **but named the object by its raw
`objectID`, which is now [I-0182].**

### ⚠️ Finding 1 — a `cifs` client can serve a PHANTOM listing of a DIFFERENT share

⚠️ **After the volume was pulled and replugged, `/mnt/scrivi-worlds` presented the directory listing of
the OTHER share (`ScriviLinux`) — byte-identical entries, same sizes, same timestamps** (`.DS_Store`
6148, `.scrivi-fd-probe.tmp` 13 B, and `appsupport/ projects/ worlds/`). ⚠️ **`mountpoint -q` reported
it MOUNTED and `/proc/mounts` named the right share** — ✅ **while the volume's real content
(`Eskandar.scrivworld`) returned `ENOENT` and every phantom entry returned `EBADF`.**

⚠️ **This is WORSE than the `statvfs` lie 2b records.** ⚠️ **`statvfs` returns a plausible number; this
returns a plausible DIRECTORY TREE belonging to a different volume.** ⚠️ **Any check that trusts a
listing — "the world folder is still there" — is defeated outright.**

✅ **Cause: the mount carried `noserverino`**, so the client invents inode numbers instead of using the
server's, and `cache=strict` (the default) kept serving entries after the volume was gone. ✅ **FIXED IN
THE RIG SCRIPT**, not in product code: the Worlds mount now uses **`serverino,cache=none`**
(`scripts/mount-shares-on-rig.sh`, commented with this observation).

⚠️ **This is a RIG DEFECT, not a Scrivi defect** — ⚠️ **but it partly CONTAMINATES the rig as a proxy
for drive loss**: for that window the app was reacting to a FABRICATED listing, not to a missing
volume. ⚠️ **Any SMB-based drive-loss run must confirm the mount options first.**

### ⚠️ Finding 2 — a pulled SMB volume leaves a ZOMBIE mount that blocks remount

⚠️ **`mount` lists the entry; `mountpoint -q` denies it; `fuser` reports `Stale file handle`.** ⚠️ **A
fresh mount over it fails `mount error(16): Device or resource busy`** — ⚠️ **which reads as "something
is using it" and sends you hunting for a process that does not exist.** ✅ **`umount -l` clears it.**
✅ **The rig script now DETECTS this (`is_stale`) and clears it automatically before mounting**, and
`--status` reports **STALE** distinctly from **not mounted**.

⚠️ **`/proc/mounts` persisting is exactly what §2's table predicts for a vanished device** — ✅ **so this
is CONFIRMING evidence for the table**, ⚠️ **but over `cifs`, which is not what AC4 asks for.**

### ✅ What this means for S3

⚠️ **S3 is UNAFFECTED and still owed.** ⚠️ **A local yank has no SMB client cache in the path**, so it
⚠️ **cannot produce a phantom listing from an unrelated share** — ✅ **and the errno set it returns
(`EIO`/`ESTALE` on stranded FDs) is still unmeasured.** ⚠️ **§7 of the rig doc stays EMPTY.**

---

## 2d. ✅ THE S3 DRIVE IS PROVISIONED (2026-09-07) — ⚠️ **and the filesystem is a RECORDED VARIABLE**

⚠️ **`Scrivi-Worlds` is APFS. Ubuntu cannot mount it** — the rig reported *"apfs is not configured for
this kernel"*, which is correct and not a misconfiguration.

⚠️ **An APFS driver was CONSIDERED AND REJECTED.** `apfs-fuse` is read-only (⚠️ **so it cannot produce a
torn write at all**) and `linux-apfs-rw` is out-of-tree and explicitly experimental — ⚠️ **not to be
pointed at 931 GB of real manuscripts.** ✅ **The deciding argument is methodological, not practical:**
⚠️ **a FUSE driver fails through the FUSE layer, not the block layer**, so S3 would have measured the
DRIVER and inferred the kernel — ⚠️ **the exact substitution this Epic exists to stop.**

✅ **THE USER PARTITIONED THE DRIVE (2026-09-07):** the APFS partition ⚠️ **keeps the real files,
untouched**, and a second **`MS-DOS (FAT)`** partition — ⚠️ **FAT32 (`vfat`)**, which Disk Utility's
plain "MS-DOS (FAT)" produces at this size — carries ✅ **a COPY of the world** for S3.
⚠️ **Confirm with `lsblk -f` on the rig rather than trusting this note.**

| ✅ What this BUYS | ⚠️ What it COSTS |
| ----------------- | ---------------- |
| ✅ **`vfat` is IN-KERNEL** — no driver, no FUSE layer between the yank and the observation | ⚠️ **`vfat` has NO POSIX ownership or permissions** — uid/gid/mode come from mount options |
| ✅ **The real drive stays APFS and stays on the Mac** — no 931 GB destroy-and-restore | ⚠️ **Any permission-shaped observation is an ARTEFACT**, exactly like VirtioFS's `root:root` flattening in §2b |
| ✅ **Block-layer behaviour — stranded FDs, stale mounts, torn writes — is filesystem-agnostic and stays VALID** | ⚠️ **The world under test is a COPY on a different filesystem than Apple's** — ⚠️ **`WorldVolumeStatus` is read on both** |

⚠️ **§2 already rules that the protocol is "a variable to RECORD, not a detail to gloss."** ✅ **The same
applies to the local filesystem** — ⚠️ **T-0477's findings MUST state that S3 ran on `vfat`.**

### ✅ Two artifacts added (2026-09-07)

| Artifact | ⚠️ The gap it closes |
| -------- | -------------------- |
| ✅ **`platforms/linux/tools/s3-baseline-capture.sh`** | ⚠️ **§4 requires BEFORE/DURING/AFTER; `volume-loss-probe.sh` covers only during and after.** ⚠️ **NOTHING captured `scrivi_get_world_status`** — which §4 itself calls ⚠️ **THE INPUT TO T-0478.** ⚠️ **It exits non-zero and refuses to pass silently when that probe is unavailable.** ✅ **Also takes per-file checksums — the ORACLE without which the torn-write test is unfalsifiable.** |
| ✅ **`T-0477-RUNBOOK.md` §5.0–5.3** | ⚠️ **Records the APFS/FAT32 decision, and makes the MOUNTPOINT-OWNERSHIP question deliberate** — ⚠️ **`/media/*` (udisks2, likely REMOVED) vs `/mnt/*` (hand-mounted, likely SURVIVES).** ⚠️ **§2b says this decides WHICH BRANCH of T-0498 fires**, so ✅ **both must be measured, not whichever the drive happened to land on.** |

⚠️ **NEITHER closes an AC.** ⚠️ **T-0477 remains In Progress and T-0478 remains HARD-GATED** — ✅ **these
are instrumentation, and the sprint's rule is that the findings win.**

---

## 2e. ✅ **S3 IS DONE (2026-09-07)** — ⚠️ **and it overturned an assumption while finding a defect nobody was hunting**

✅ **THE USER PULLED THE DRIVE ON `oathkeeper`.** ⚠️ **Full findings:**
`platforms/linux/tools/T-0477-FINDINGS-S3.md`

| ✅ Observed | ⚠️ Consequence |
| ----------- | -------------- |
| ⚠️ **The yank was SILENT** — no error, no warning | ⚠️ **The predicted `EIO`/`ESTALE` storm did NOT occur** |
| ✅ **udisks2 REMOVED the mountpoint `/run/media/<user>/<label>`** | ✅ **I-0181's false `missing` is NOT REACHED on the automounted path** — ⚠️ **the parent is gone, so the core resolves the honest `unavailable`, exactly as macOS does** |
| ⚠️ **Scrivi still said AVAILABLE, and a double-click still SUCCEEDED** | ⚠️ **NEW DEFECT — [I-0192]**, ⚠️ **and NOT the one first filed** |
| ✅ **A scene change flipped it to `unavailable`** | ✅ **The core is honest WHEN ASKED** |

### ⚠️ The assumption that fell

⚠️ **§2b established that Linux KEEPS the mountpoint where macOS removes it, making the false `missing`
a Linux-specific trap.** ✅ **That is TRUE for a hand-mounted path and FALSE for the automounted one** —
⚠️ **and the automounted path is what a real writer's machine uses.**

✅ **The `/mnt` half was RULED NOT WORTH RUNNING (user, 2026-09-07)**, and ⚠️ **the ruling is not a gap**:
⚠️ **a hand-created `/mnt` directory is an ordinary directory the operator owns, and nothing has any
mandate to delete it.** ✅ **udisks2 removes `/run/media/...` precisely BECAUSE it created it.**
⚠️ **T-0498 keeps its justification** — ✅ **the `/mnt` path is real and reachable, and there the false
`missing` DOES fire** — ⚠️ **its trigger is simply narrower than this sprint assumed.**

### ⚠️ **[I-0192] — the finding the sprint was NOT looking for** — ⚠️ **RE-DIAGNOSED 2026-09-07**

⚠️ **A world on a physically-removed drive kept reporting AVAILABLE, and a double-click kept returning
SUCCESS, until a scene change forced a re-resolve.**

⚠️ **THE FIRST DIAGNOSIS — "a cached status with no invalidation" — IS WITHDRAWN.** ✅ **Reading the code
disproved it:** the double-click performs a ⚠️ **genuine `openObject` through the ABI** and parses the
name from the returned `objectJson`; ⚠️ **there is no app-side status cache**, and ✅ **`WorldStore::resolve`
caches no verdict either** — it returns `available` only after reading and parsing `world.json`
(`WorldStore.cpp:337-342`). ⚠️ **Re-scoped `[Linux]` → `[ScriviCore]`.**

⚠️ **So THE FILESYSTEM ANSWERED SUCCESSFULLY for a volume that was physically gone**, and every layer
above correctly trusted a correct answer. ⚠️ **Likely the PAGE CACHE** — ✅ **§2b already measured the
stronger form**, a held FD outliving `umount -l` + `losetup -D` entirely.

⚠️ **This is I-0181's SIBLING, not its opposite:** ⚠️ **I-0181 infers ABSENCE it cannot prove** (directory
exists → `missing`); ⚠️ **I-0192 infers PRESENCE it cannot prove** (read succeeded → `available`).
✅ **T-0498's `st_dev` primitive is plausibly the fix for BOTH directions.**

⚠️ **NOT SETTLED, and MUST NOT be folded into T-0498 on inference** — ⚠️ **that is exactly how I-0181's
block was narrowed three times without fixing it.** ✅ **The deciding measurement is a `scrivi_world_probe`
run SAMPLED every ~2 s across a yank:** ⚠️ **a verdict that DECAYS is the page cache and is not T-0498's;
one that PERSISTS is `resolve` asserting presence, and folds in.**

### ⚠️ **The instrumentation did NOT produce any of this** — ⚠️ **and that is a process finding**

⚠️ **`volume-loss-probe.sh` was run on `Flitwick-5` (the MacBook) against a `/run/media/...` path that
does not exist on macOS**, because ⚠️ **the runbook never said WHICH MACHINE each command belonged to.**
⚠️ **§5.1 also instructed the user to BUILD A BINARY on `oathkeeper`, which has no dev environment** —
⚠️ **and a binary that did not exist in the repo at the time.**

✅ **Fixed:** ⚠️ **runbook §0a now tags EVERY command 🐧 `oathkeeper` or 🍎 `Flitwick-5`**, and
✅ **`scrivi_world_probe` now exists** (`ScriviCore/tools/scrivi_world_probe.cpp`, ⚠️ **Qt-free**, so it
builds without the Linux app configured).

⚠️ **THE LESSON IS SHARPER THAN THE FIX:** ⚠️ **no probe in §4's table would have caught I-0192 even had
it run correctly** — ✅ **every one of them questions the OS, and the OS was truthful throughout.**
✅ **A human noticed that a double-click still said SUCCESS.** ⚠️ **`feedback_live_pass_finds_what_suites_cannot`,
one layer further out: a green suite never means usable, and ⚠️ neither does correct instrumentation.**

---

## 2f. ✅ **S2 IS DONE (2026-09-07)** — ⚠️ **two passes, and TWO ASSUMPTIONS FELL**

✅ **The share was killed at the SERVING Mac while the rig held it mounted**, twice:
`cache=strict`, then `cache=none`. ⚠️ **Full findings:** `platforms/linux/tools/T-0477-FINDINGS-S2.md`
(⚠️ **3,110 lines of capture**).

✅ **NO DRIVE WAS NEEDED** — the world was a copy on the Mac's local disk, served through `ScriviLinux`.
⚠️ **This is why S2 was runnable while the USB drive was out and the rig's workstation was in use.**

### ⚠️ The confound the USER caught before the run

⚠️ **The served share carries `projects/` and `appsupport/` as well as `worlds/`.** ⚠️ **Killing it would
have taken the project and the appSupportRoot too**, and the run would have measured *"everything
vanished"* rather than *"a world went offline."* ✅ **The user asked the right question — *"Will
unmounting it also cause the base app to miss the Scrivi data as well?"*** — ⚠️ **and it was not in the
plan.** ✅ **Isolation was verified first**: project and appSupportRoot both on LOCAL disk.

### ⚠️ **FINDING 1 — the ~10-second block. A DESIGN constraint for T-0478.**

⚠️ **Every call against the dead share BLOCKS ~10 s before returning `EHOSTDOWN` (112)** — ⚠️ **and it is
IDENTICAL under `cache=strict` and `cache=none`, with `soft` set in both.** ⚠️ **`soft` prevents an
unkillable hang; it does NOT prevent a ten-second stall.**

⚠️ **§6's risk table predicted hangs for NFS HARD mounts and treated `cifs` as the safe case.
⚠️ IT IS NOT.** ⚠️ **The cost COMPOUNDS**: `scrivi_world_probe` makes several core calls per run and
degraded to ⚠️ **43-second intervals.** ✅ **The healthy path is 0.090 s** — ⚠️ **~110× faster**, so a
timeout is cheap and discriminating.

✅ **CONSEQUENCE: the app MUST NOT call the core synchronously on the UI thread for world status.**

### ⚠️ **FINDING 2 — `offline` was NEVER produced by the case that DEFINES it**

⚠️ **S2 exists because `WorldStatus::offline` is defined by the network case.** ⚠️ **The core returned
`unavailable` — in BOTH passes, within ~1 s.** ⚠️ **`offline` has still never been observed in this
project.**

⚠️ **T-0478 must RULE on the enum** — ✅ **retire it, emit it (⚠️ `EHOSTDOWN` is a promising and
currently-unused signal), or document it as unreachable** — ⚠️ **but it must NOT quietly ship a Linux
mapping that never emits it.** ⚠️ **That is the documented-lie state EP-038 exists to prevent.**

### ⚠️ **FINDING 3 — `cache=none` made the CORE's answer WORSE**

⚠️ **Under `cache=strict` the core returned an honest `unavailable` within ~1 s. Under `cache=none` the
probe returned NO STATUS AT ALL** — the loop shell stayed alive while every invocation blocked.
⚠️ **The client cache was the only thing answering fast enough.** ✅ **`cache=none` remains CORRECT for
§2c's phantom listing** — ⚠️ **but it is not a free improvement, and neither setting is safe alone.**

### ✅ **FINDINGS 4 & 5 — the phantom listing reproduced; recovery is AUTOMATIC**

⚠️ **`mountpoint -q` said YES throughout; `ls` of `worlds/` SUCCEEDED with ZEROED sizes while one level
deeper failed.** ✅ **Confirms §2c on a DIFFERENT trigger** (§2c: volume pulled behind a live server;
S2: the server itself). ⚠️ **Any check trusting a listing is defeated.**

✅ **Recovery needed NO intervention — no zombie mount, no `umount -l`**, ⚠️ **unlike §2c.**
⚠️ **The distinction is SERVER vs VOLUME; do not generalise either to the other.**

### ⚠️ Not measured, not inferred

⚠️ **The "network off / black hole" variant** (only "File Sharing OFF" — a refused connection — was
run) · ⚠️ **NFS** · ⚠️ **a held-open FD across the kill** · ⚠️ **what the APP shows a writer** (this was
an ABI-level run; ⚠️ **the DoD's live pass is still owed**).

---

## 3. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0477** | ⚠️ **DRIVE-LOSS INSTRUMENTATION — FINDINGS ONLY, NO CODE.** ⚠️ **THREE scenarios (S1/S2/S3)**; ⚠️ **the USER pulls, Claude instruments** | **High** | ✅ **ALL THREE OBSERVED 2026-09-07** — S1 captured · **S3** (`T-0477-FINDINGS-S3.md`) · ✅ **S2** (`T-0477-FINDINGS-S2.md`, ⚠️ **two passes, 3,110 lines**). ✅ **IMPLEMENTED - Not Verified (2026-09-10).** ✅ **ALL DoD ITEMS NOW SETTLED.** ✅ **Rig doc §7 IS WRITTEN** — 183 lines, eight subsections, ⚠️ **from the rig, not from documentation** — ✅ **including §7.4's FIVE LYING SIGNALS and §7.7's runnable steps.** ✅ **§7.5 CORRECTED 2026-09-10: it read "`offline` has NEVER been observed" and that is NO LONGER TRUE** — ⚠️ **T-0478 made it real and measured it** (`hostUnreachable`, 1m42s, both endpoints). ✅ **The writer-facing live pass WAS DONE** (T-0478, 2026-09-08/09) — ⚠️ **it is what found [I-0193] and [I-0194].** ⚠️ **`before/during/after` RETIRED as a phantom requirement by user ruling; ⚠️ S3 will NOT be re-run.** ⚠️ **Open questions remain in §7.8 (black-hole variant, NFS, held FD, torn writes)** — ✅ **recorded as UNKNOWNS, blocking nothing.** |
| **T-0478** | ⚠️ **`WorldVolumeStatus` for Linux** — `unmounted` / `offline` / `missing`, ⚠️ **written AGAINST T-0477's findings, NEVER from documentation** | **High** | ✅ **VERIFIED 2026-09-10 (user-approved) — ARCHIVED** → [`../Tasks/Verified/Task-verified-0475-0479.md`](../Tasks/Verified/Task-verified-0475-0479.md). ⚠️ **Was `Implemented - Not Verified (2026-09-08)`.** ✅ **CORE VERIFIED ON REAL HARDWARE** — ⚠️ **`offline`/`hostUnreachable` emitted for the FIRST TIME EVER** (§3b). ✅ **[I-0193] is now VERIFIED and ARCHIVED (2026-09-10) and the writer-facing string HAS BEEN READ** — ⚠️ **both DoD blockers from the live pass are cleared.** ✅ **[I-0194] is now RESOLVED (`e978360`, `lexically_normal` on the fallback; ctest 583/583).** ✅ **WOL SETTLED 2026-09-10 by user ruling — NOT PURSUED** (the operator powers the rig off at night, so there is no NIC to answer; the question is CLOSED, not deferred). ✅ **The inconclusive-branch DoD item was SETTLED 2026-09-10 by user ruling — SATISFIED:** the writer-facing messages already report an unreachable world as unreachable, never as damaged. ⚠️ **A claim of mine is WITHDRAWN there: I-0183 and I-0192 were NOT instances of it** — ✅ **both were wrong-status defects in the core's RESOLUTION logic, both fixed.** ⚠️ **Residue of that class belongs to [I-0181]/T-0498.** ⚠️ **RESIDUAL, recorded not hidden: the OFFLINE branches of both fixes are unproven by any suite** — ✅ **[I-0194]'s fallback cannot be staged in-process, [I-0193]'s timeout needs a real blocking mount.** ⚠️ **Ruled on the enum rather than omitting it, which FINDING 2 demanded.** ⚠️ **Original gating note: UNGATED 2026-09-07 — T-0477 has reported.** ⚠️ **Two findings CONSTRAIN it before a line is written:** ⚠️ **(1) `offline` was NEVER produced by the network case that DEFINES it** — the enum needs a ruling, ⚠️ **not a quiet omission**; ⚠️ **(2) the core BLOCKS ~10 s per call against a dead share** — ⚠️ **so it MUST NOT be called synchronously on the UI thread** |
| **T-0479** | ⚠️ **Correct Porting Outline §9** from what the rig actually taught | **Medium** | ✅ **DONE 2026-09-07** — ⚠️ **§9's RULE held; FOUR of its PREDICTIONS did not** (§§9.1–9.5, each naming prediction vs measurement). ⚠️ **Checklist grew 6 → 8 items.** ✅ **A §9 that survived unchanged would have been evidence it was never tested** |
| **T-0498** | ⚠️ **`[ScriviCore]` Stop inferring `missing` from DIRECTORY EXISTENCE** — [I-0181]. ✅ **Add a device-identity primitive to `FileSystem`**; require *package absent* **AND** *same device as parent* before `missing`. ⚠️ **CORE fix, not a platform refinement** | **Medium** | 🟡 **Implemented - Not Verified (2026-09-10).** ✅ **GATE OPENED — T-0477 S3 reported.** ✅ **`deviceID` added to `FileSystem`; blast radius was EXACTLY the two implementations predicted** (`LocalFileSystem` + the `ScanCountingFileSystem` forwarder). ⚠️ **S3's finding NARROWED the Task rather than confirming it:** ✅ **udisks2 REMOVES the mountpoint, so the automounted path never reaches the defective branch** — ⚠️ **the fix guards the HAND-MOUNTED `/mnt` case.** ⚠️ **I IMPLEMENTED THE POLARITY BACKWARDS FIRST and the CONTROL TEST CAUGHT IT** — ✅ **recorded because the sprint's own caveat predicted exactly this:** *`st_dev` proves "not a mount NOW", NOT "a volume went away"*. ✅ **`ctest` 585/585 macOS, new test VERIFIED FAILING against the unfixed core.** ⚠️ **KNOWN RESIDUAL: a SURVIVING mountpoint after a pull is indistinguishable from an ordinary directory by device identity** — ⚠️ **closing it needs evidence from the BINDING, not the filesystem.** |

⚠️ **T-0478 MUST NOT START BEFORE T-0477 REPORTS.** ⚠️ **The gate is the POINT of the sprint, not
ceremony.** ✅ **If T-0477's findings contradict this plan, the findings win.**

---

## 3a. T-0498 — ⚠️ **the CORE resolution fix for [I-0181]**

**Codebase:** ⚠️ **`[ScriviCore]`** — ⚠️ **NOT `[Linux]`.** This is the sprint's only cross-platform
deliverable; it lands in the shared core and changes what BOTH platforms report.

### The defect, precisely

`WorldStore::resolve` (`ScriviCore/src/worlds/WorldStore.cpp:290-296`) establishes `missing` from two
facts:

```cpp
auto pkgE = fs_.exists(cand);
if (pkgE.ok() && !pkgE.value()) {           // package definitively absent
    auto parentDir = util::parent(cand);
    if (auto e = fs_.exists(parentDir); e.ok() && e.value()) {
        sawContainerButNoPackage = true;    // → missing
    }
}
```

⚠️ **An unmounted volume satisfies BOTH.** ✅ **A mountpoint is just a directory**; when the device goes
away the directory survives and its pre-mount contents reappear. So the container "exists", the package
does not, and ⚠️ **an intact world on a pulled drive gets the ONE status reserved for positive proof of
absence** — the status Doc 2 §7.2.1 says invites DESTRUCTIVE writer remedies.

### ⚠️ **This is the THIRD leak of the same inference**

✅ **The file's own comments record the prior two**, both narrowings of this exact block:

| Prior fix | What it excluded | Why it was still wrong afterwards |
| --------- | ---------------- | --------------------------------- |
| **T-0419** | a sandboxed host: package unreadable but PRESENT | narrowed the READ, not the CONTAINER question |
| **T-0420 / [I-0136]** | a package too NEW to parse | ditto — parse-level, not container-level |
| ⚠️ **T-0498** | ⚠️ **an unmounted volume** | ⚠️ **directory EXISTENCE was the wrong question all three times** |

⚠️ **Each fix narrowed a symptom and left the inference intact.** ✅ **T-0498 attacks the inference.**

### ✅ The fix direction — ⚠️ **the user's ruling, and 2b's measurement**

⚠️ **`exists()` cannot answer "is a device mounted here".** ✅ **`st_dev` vs the parent's can** — 2b
measured it working on a real Linux kernel, and I-0181 records the identical result on macOS with a
hand-specified mountpoint.

⚠️ **`FileSystem` has NO device-identity primitive** (`ScriviCore/include/scrivi/Services.hpp:38`). One
must be added — e.g. `Result<std::uint64_t> deviceID(const AbsolutePath&)`.

✅ **Blast radius is SMALL — exactly two implementations:**

| Implementation | Change |
| -------------- | ------ |
| `platform/LocalFileSystem` | real `stat`, populating `st_dev` |
| `ScanCountingFileSystem` (`tests/integration/ObjectIndexTests.cpp:36`) | ⚠️ **a pure forwarding decorator** — one added line |

⚠️ **`statvfs` MUST NOT be used.** ✅ **2b measured it SUCCEEDING on an unmounted path**, reporting the
root filesystem's block counts — ⚠️ **a confident success with a plausible number**, which is worse than
a failure.

### ⚠️ **The gate, and why it is real**

⚠️ **T-0498 is GATED on T-0477's S3** (physical yank), for the reason §2b already states: ⚠️ **udisks2
may REMOVE the mountpoint on a physical yank**, which changes ⚠️ **which branch fires** — and therefore
whether this fix is reached at all in the case that matters most.

⚠️ **2b's own caveat cuts the other way too:** ✅ **`st_dev` proves "not a mount NOW", NOT "a volume went
away".** ⚠️ **A directory that NEVER held a mount matches identically** — ✅ **which is the ordinary
world-deleted case and MUST still report `missing`.** ⚠️ **So the fix cannot key on `st_dev` alone as
proof of a departed volume; it uses the match to WITHHOLD `missing`, never to assert absence.**

### Success criteria

- [ ] `FileSystem` gains a device-identity primitive; ⚠️ **both implementations supply it**
- [ ] `resolve` sets `sawContainerButNoPackage` ⚠️ **only when package-absent AND same-device-as-parent**
- [ ] ⚠️ **Anything else resolves `unavailable`**, never `missing`
- [ ] ✅ **A mock `FileSystem` returning a MISMATCHED deviceID proves the new branch** (unit)
- [ ] ✅ **Every existing `missing` test in `WorldTests.cpp` still passes** — ⚠️ **same-device absence is STILL positively established absence**
- [ ] ⚠️ **VERIFIED BY A REAL DRIVE PULL on the rig, not by the mock alone**

⚠️ **The mock proves the LOGIC; only the rig proves the PREMISE.** ⚠️ **This Issue was found BY
INSTRUMENTATION** — ✅ **its fix earns the same standard.**

### ⚠️ Latency — stated honestly

⚠️ **This defect reaches NO writer on EITHER platform today**, and T-0498 does not claim otherwise:

- ⚠️ **Linux has no Worlds READ path that surfaces status to a writer.** ⚠️ **SP-127 built the Scene Inspector and a `Manage Worlds…` dialog** — ⚠️ **an earlier note in §2 of this sprint claimed SP-127 satisfied I-0181's gate; that is TRUE for the surface's existence, and the false `missing` is now REACHABLE there.**
- ⚠️ **On Apple the surface exists but `diskarbitrationd` TIDIES `/Volumes` mountpoints it created** — ✅ **so Apple is MASKED BY CONVENTION, not protected.** ⚠️ **A hand-specified mountpoint defeats the masking**, which is exactly what I-0181 measured.

✅ **Fixed on the merits: a latent trap in the shared core, removed before the surfaces grow into it.**

---

## 3b. ✅ **T-0478's LIVE PASS (2026-09-08)** — ⚠️ **what it PROVED and what it BROKE**

**Rig:** 🐧 `oathkeeper`, `the-stairs-of-tintagael.scrivi` (LOCAL disk) bound to `Eskandar.scrivworld`
on `/mnt/scrivi-net` — ⚠️ **a `cifs` share served from the workstation, killed by turning File Sharing
OFF.** ✅ **Isolation VERIFIED FIRST** (the confound the user caught before S2): ⚠️ **the share also
carries `projects/` and `appsupport/`, but the app used NEITHER** — project in `~/ScriviProjects`,
appSupportRoot at `~/.local/share/Scrivi`, ✅ **so the kill removed the WORLD ONLY.**

✅ **Build under test CONFIRMED before the pull** (`feedback_confirm_the_build_under_test`): the running
binary contains `hostUnreachable` and the writer-facing `"is offline"` strings, built 2026-09-07 17:12,
process started 2026-09-08 16:13 — ⚠️ **not the day-stale binary the rig ran once before.**

### ✅ **PROVED — `offline` EXISTS, for the first time in this project's history**

| Phase | `status` | `statusReason` | `packagePath` | ⏱ |
| ----- | -------- | -------------- | ------------- | -- |
| **BEFORE** | `available` | — | ✅ `/mnt/scrivi-net/worlds/Eskandar.scrivworld` | ✅ **0.087 s** |
| ⚠️ **SHARE KILLED** | ✅ **`offline`** | ✅ **`hostUnreachable`** | ✅ **empty** | ⚠️ **1m42.2 s** |
| ⚠️ **repeat** | ✅ **`offline`** | ✅ **`hostUnreachable`** | ✅ **empty** | ⚠️ **1m42.4 s** |

✅ **Both endpoints agree** (`scrivi_list_worlds` and `scrivi_get_world_status`), ⚠️ **reproducibly.**
✅ **This closes FINDING 2**: `offline` is no longer a documented lie — ⚠️ **it is emitted, on a positive
`EHOSTDOWN`, from real hardware rather than a decorator.**

### ⚠️ **BROKE — the feature is NOT USABLE, and the live pass is what proved it**

⚠️ **The user clicked a SCENE. The app froze, showed "not responding" after ~6 s, stayed frozen ~4–5
MINUTES, and was FORCE QUIT.** → ✅ **[I-0193], High.**

⚠️ **FINDING 1 WAS UNDERSTATED TWICE, and both corrections matter more than the confirmation:**

1. ⚠️ **The magnitude.** Finding 1 measured ~10 s. ⚠️ **This mount blocks 102 s** — ✅ **~1,175× the
   healthy path.** ⚠️ **The mount options differ** (`cache=none,actimeo=1,closetimeo=1`), ⚠️ **so the
   cost is MOUNT-TUNING DEPENDENT and ~10 s is NOT a ceiling.**
2. ⚠️ **The reach.** Finding 1 read as a Worlds-dialog concern. ⚠️ **The blocking call is on
   `setScene`** — `EditorShell.cpp:1029` → `SceneInspector.cpp:318` → `:330` → ⚠️ **`listWorlds` at
   `:395`.** ✅ **So ORDINARY NAVIGATION freezes the app**, not an occasional dialog.

⚠️ **`WorldsDialog::reload()` has the SAME defect** (`WorldsDialog.cpp:176-180`) and ⚠️ **was simply not
the path hit.** ✅ **Any fix must cover BOTH, and needs a TIMEOUT as well as a thread.**

### ⚠️ **ALSO FOUND — a path defect only the offline route produces** → ✅ **[I-0194], Medium**

⚠️ **`lastKnownPackagePath` came back UNNORMALIZED when offline:**
`…/the-stairs-of-tintagael.scrivi/../../../../../../mnt/scrivi-net/worlds/Eskandar.scrivworld`
— ⚠️ **six `../` segments**, against ✅ **a CLEAN path in the healthy baseline minutes earlier.**
⚠️ **It is writer-facing**: `displayPath()` passes it through with no normalization.

### ⚠️ **ALSO REPORTED — the SAME gap, in the HEALTHY case** → ✅ **[I-0195], Medium**

⚠️ **After the share was restored, the user reported project open ~10× slower.** ✅ **Diagnosed as the
`cache=none` remount, NOT a code regression** — ⚠️ **world resolve measured `0.05–0.08 s` throughout.**

⚠️ **But the user's ruling reframed it, correctly:** ⚠️ **an earlier reading of mine called this "mount
configuration, not an app defect." ⚠️ THAT WAS WRONG.** ✅ **The app blocks the UI for the whole read
whatever the reason for the slowness** — ⚠️ **so a slow mount EXPOSES the defect rather than causing
it**, ⚠️ **and `cache=strict` MASKS it rather than fixing it.**

⚠️ **The cost is UNBOUNDED**: worlds grow, ⚠️ **a project may bind SEVERAL**, and ⚠️ **project and
worlds may BOTH be on slow network storage.** ✅ **User ruling: waiting is fine; a FROZEN SILENT UI is
not.** ✅ **And the design is determinate — the FILE COUNT is known early**, so
`files read / files to read` is a real percentage, ⚠️ **not a spinner.**

⚠️ **[I-0193] and [I-0195] share ONE root cause and must NOT be fixed separately:**

| | ⚠️ Case | ✅ Needs |
| - | ------- | -------- |
| **[I-0193]** | ⚠️ **Volume UNREACHABLE** — blocks ~102 s | ⚠️ **A TIMEOUT** |
| **[I-0195]** | ⚠️ **Volume REACHABLE but SLOW** — completes correctly | ⚠️ **PROGRESS** |

⚠️ **A timeout alone would ABORT a legitimate slow load. A progress bar alone would show a bar that
never finishes.** ✅ **Both presuppose the read is OFF THE UI THREAD** — ⚠️ **the gap FINDING 1 named,
which T-0478 did not close.**

### ⚠️ **STILL OPEN — the DoD item this pass was meant to close**

⚠️ **The writer-facing string was NEVER READ.** ⚠️ **The freeze prevented reaching the Worlds dialog and
the object error**, so ⚠️ **"a LIVE PASS — the writer-facing string is READ" remains UNTICKED**, and
⚠️ **[I-0193] BLOCKS IT.** ✅ **Recorded honestly rather than ticked from the ABI evidence** — ⚠️ **the
ABI returning `offline` is NOT the same claim as a writer reading it.**

---

## 4. T-0477 — what gets captured, per scenario

⚠️ **Capture BEFORE, DURING and AFTER for every scenario.** ⚠️ **"After" alone cannot show a stale
entry, because a stale entry looks exactly like a live one.**

| Probe | Why |
| ----- | --- |
| `/proc/mounts` + `/proc/self/mountinfo` | ⚠️ **Does the entry vanish, or persist STALE?** |
| `statfs()` / `stat()` on the mount root **and** on the world package | ⚠️ **Which one fails first, and with WHAT errno** |
| ⚠️ **A held-open FD, read AND written across the event** | ⚠️ **`EIO`? `ESTALE`? `ENOTCONN`? Silence?** ✅ **This is the probe a container cannot run** |
| `lsblk` / `/dev/*` presence, `findmnt` | Device-node survival |
| `udevadm monitor` (S3), `dmesg -w` | ⚠️ **Kernel's own account** |
| ⚠️ **What `scrivi_get_world_status` returns** at each phase | ⚠️ **The core's verdict is the INPUT to T-0478** |
| ⚠️ **A write in flight** | ⚠️ **Torn? Silently lost? Error?** — Doc 2's repair path depends on the answer |

⚠️ **Record the SURPRISES loudly.** ⚠️ **Apple's headline finding was that the OBVIOUS API LIED**
(`volumeIsRemovable == false` on a hand-unplugged drive). ⚠️ **Whatever Linux's equivalent lie is,
T-0477's job is to catch it — and a finding of "nothing surprising" must be stated as a RESULT, not
assumed.**

### ⚠️ Ownership

| Phase | Who |
| ----- | --- |
| Wake the rig; provision the drive; ⚠️ **PULL IT** | ⚠️ **THE USER — physical** |
| Serve the network share; ⚠️ **kill it at the source** (S2) | ⚠️ **THE USER** (Mac or Windows box) |
| Everything else — scripts, capture, analysis, the write-up | **Claude, over SSH** |

⚠️ **NEVER drive synthetic input at real writing work** (`feedback_never_drive_synthetic_input_at_real_work`).
✅ **Use a COPY of a world, never the live `ScriviWorlds` content.**

---

## 5. Definition of Done

- [x] ✅ **S1, S2 and S3 each OBSERVED on the real rig** — ✅ **all three, 2026-09-07.**
      ⚠️ **S2 has full streamed capture (3,110 lines, two passes); ⚠️ S3 was observed BY HAND with NO
      probe running** (it was started on the wrong machine) — ✅ **and its finding was CORRECT and
      load-bearing regardless**: ⚠️ **udisks2 removes the mountpoint, so [I-0181] is NOT reached on the
      automounted path.** ✅ **See `T-0477-FINDINGS-S3.md` §0, which states plainly that the user
      produced the finding and the instrumentation did not.**
      ⚠️ **THE "BEFORE/DURING/AFTER for every scenario" SUB-REQUIREMENT IS RETIRED — 2026-09-10, USER
      RULING** — ✅ **as a PHANTOM REQUIREMENT that cannot be accurately measured.** ⚠️ **A physical yank
      is instantaneous and operator-driven, and the during-window is exactly when the machine is least
      able to report on itself.** ⚠️ **S3 WILL NOT BE RE-RUN.** ✅ **The lesson that survives is about
      EVIDENCE, not cadence: a finding must state HOW it was gathered, and "the user watched the screen"
      is legitimate — stronger, here, than instrumented output aimed at the wrong machine.**
      ⚠️ **Do NOT re-add this checkbox to a future platform's rig work** (rig doc §7.8).
- [x] ✅ **The rig doc's §7 is WRITTEN — from the rig, not from documentation** (2026-09-07, v0.2),
      ✅ **including the runnable steps for all three scenarios, machine-tagged 🐧/🍎.**
- [x] ⚠️ **Which Linux signal is AUTHORITATIVE — and which LIE — is RECORDED** (rig doc §7.4).
      ⚠️ **FIVE signals lie**: `mountpoint -q`, a directory listing (⚠️ **zeroed sizes**), `statvfs`,
      a successful `read` (transiently), and ⚠️ **a held FD (survived `umount -l` + `losetup -D`).**
      ✅ **`st_dev` works** (⚠️ with a stated limit); ✅ **`EHOSTDOWN` (112) is the strongest UNUSED signal.**
- [x] ✅ **`WorldVolumeStatus` exists for Linux and IS verified against the REAL volume** (2026-09-08, §3b) — ⚠️ **`offline`/`hostUnreachable` produced from a killed `cifs` share on the rig, reproducibly, at BOTH endpoints.** ✅ **First time `offline` has ever been emitted.**
- [x] ✅ **Every inconclusive branch returns the core's status** — ⚠️ **a wrong `missing` is worse than
      an honest `unavailable`** (Doc 2 §7.2.1; I-0115 was this defect shipped).
      ✅ **SETTLED 2026-09-10 BY USER RULING — SATISFIED, not deferred.** ✅ **The writer-facing messages
      already do this correctly**: an unreachable world is reported as unreachable — *"a world may be on a
      disconnected or unreachable volume. Nothing has been lost."* — ⚠️ **never as damaged.**
      ✅ **THE USER'S RULE, which is SHARPER than this DoD line and supersedes it:**
      ⚠️ **`corrupt`/`damaged` is a DETERMINISTIC claim, made ONLY when the world CAN BE REACHED and
      something EXPECTED is missing or something UNEXPECTED is present.** ⚠️ **Unreachability is a
      DIFFERENT STATEMENT and is already made correctly.**
      ⚠️ **A PRIOR CLAIM OF MINE IS WITHDRAWN HERE:** ⚠️ **I cited [I-0183] and [I-0192] as instances of
      this item. ⚠️ THEY ARE NOT.** ✅ **Both were WRONG-STATUS defects inside the core's RESOLUTION logic**
      (a world reading `available` when it was not) — ✅ **both fixed** — ⚠️ **and neither was the app
      telling a writer their world was corrupt.** ⚠️ **Stacking them behind this line made a SETTLED
      concern look like an OPEN risk.**
      ⚠️ **What remains of that class is OWNED BY [I-0181]/T-0498** (stop inferring `missing` from
      directory existence) — ✅ **not by this item, and not by T-0478.**
- [x] ✅ **Porting Outline §9 CORRECTED** (2026-09-07, T-0479) — ⚠️ **FOUR predictions overturned**,
      ⚠️ **each recorded as prediction-vs-measurement**; ⚠️ **the checklist grew from six items to eight.**
      ⚠️ **Had T-0478 been written from §9 as it stood, it would have shipped a stale-mount defence that
      is not needed, an `EIO`/`ESTALE` handler for errors that never arrive, no timeout at all, and an
      `offline` branch that never fires.**
- [x] ✅ **`ctest` + Linux smokes GREEN on the rig, non-root** (2026-09-07): ✅ **583/583 ctest** and
      ✅ **22/22 smokes**, at HEAD `065fd24`. ⚠️ **This is the first run of I-0191's 11 new tests on
      LINUX/x86-64** — they had only ever passed on macOS/arm64, ⚠️ **and I-0121/I-0122 are the
      precedent for that distinction mattering.**
- [x] ✅ **A LIVE PASS on the rig** — ⚠️ **the writer-facing string is READ, not just returned.**
      ⚠️ **ATTEMPTED 2026-09-08 and BLOCKED by [I-0193]**: ⚠️ **the app froze ~4–5 min on a scene
      click and was Force Quit, so the string was NEVER REACHED.** ⚠️ **The ABI returning `offline`
      is NOT this item** — ✅ **that is the item above.** ✅ **UNBLOCKED AND TICKED 2026-09-09/10:**
      build 34 put the scene-click string in front of a writer — *"This scene's objects are taking
      longer than expected to read — a world may be on a disconnected or unreachable volume. Nothing
      has been lost."* — ⚠️ **from a branch that had NEVER EXECUTED anywhere before** (Docker has no
      dead network share). ✅ **[I-0193] is now VERIFIED and ARCHIVED** (both call sites async, `571fac1`):
      ⚠️ **`Project > Manage Worlds` OPENS with the share down** and the timeouts surface correctly.
- [x] ✅ **Whether the rig answers WOL is settled and recorded** (NIC vs firmware) — ✅ **SETTLED 2026-09-10 BY USER RULING: NOT PURSUED, and the question is CLOSED rather than deferred.** ⚠️ **WOL is IMPOSSIBLE on this rig by operating practice, not by hardware:** ✅ **the operator powers the machine OFF at night**, ⚠️ **so there is no NIC left powered to answer a magic packet** — ⚠️ **the NIC-vs-firmware distinction is MOOT and must not be re-investigated.** ⚠️ **Consequence to PLAN AROUND: a rig pass requires the machine to be powered on by hand first** — ✅ **`ssh` timing out is the EXPECTED state, not a fault to diagnose** (`deploy-to-rig.sh` reads this as "asleep").

---

## 6. ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The rig sleeps and ignores WOL** | ⚠️ **Physical trip.** ✅ **Settle NIC-vs-firmware in the same trip so it is the LAST one** |
| ⚠️ **AFP is unmountable on Linux** | ✅ **Use SMB/NFS.** ⚠️ **Record the protocol — they fail DIFFERENTLY** |
| ⚠️ **NFS hard-mount HANGS instead of erroring** | ⚠️ **A hang is a FINDING, not a failed experiment.** ⚠️ **It would freeze the app's UI thread** — ✅ **capture it and consider `soft`/`intr` as a recorded trade** |
| ⚠️ **Findings contradict this plan** | ✅ **The findings win.** ⚠️ **This section is a plan, not a prediction** |
| ⚠️ **T-0478 drifts toward mirroring Apple** | ⚠️ **`/Volumes/<name>` has NO Linux equivalent.** ⚠️ **Mirroring the Apple file is the failure mode; the ENUM is shared, the DETECTION is not** |
| ⚠️ **Real writing work at risk** | ✅ **Work on a COPY.** ⚠️ **Back up first** (`project_test_rig_tintagael_eskandar`) |

---

## 7. ⚠️ Out of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **Any Linux object/UI surface** | **EP-035 / EP-036 / EP-037** |
| ⚠️ **Session persistence** (I-0176/0177/0178) | ⚠️ **Wants its OWN Epic** |
| **Repair-matrix behaviour changes** | ⚠️ **EP-036 AC4** — ✅ **unblocked BY this sprint, not done in it** |
| **The Windows rig** | ⚠️ **A later Epic — EXECUTING §9** |
| **CI automation of the rig** | ⚠️ **Out of scope** |

---
---


*Last Updated: 2026-09-04 (⚠️ **T-0498 WRITTEN into SP-124** — `[ScriviCore]`, the core resolution fix
for **[I-0181]**, ⚠️ **which is no longer Unassigned.** ✅ **The fix attacks the INFERENCE, not a third
symptom**: `resolve` establishes `missing` from DIRECTORY EXISTENCE, which an unmounted volume
satisfies — ⚠️ **and T-0419 and T-0420/[I-0136] each narrowed this same block without touching the
question it asks.** ✅ **`FileSystem` gains a device-identity primitive; blast radius is TWO
implementations.** ⚠️ **`statvfs` is RULED OUT — 2b measured it succeeding on an unmounted path.**
⚠️ **GATED on T-0477's S3**, because udisks2 may remove the mountpoint on a physical yank and change
which branch fires. ⚠️ **SP-124 remains PAUSED — writing a Task into it does NOT activate it.**
⚠️ **Next available Task is T-0499, NOT T-0492** — the older notes below say T-0492 and were correct
when written; T-0492–T-0497 have since been taken. Prior note follows.)*

*Last Updated: 2026-08-30, second pass (**SP-126 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) with its six Tasks verified and archived in the
SAME STEP → `Verified/Task-verified-0485-0490.md`. ✅ **The Audit Check was CLEAN**: 6/6 Tasks Verified,
0 unticked DoD items, nothing stale. ✅ **EP-035 AC10 is CLOSED.** ⚠️ **NO Sprint is now active** — see
the table above for what is available. ⚠️ **The largest unhomed item is session persistence**
(I-0176/0177/0178), Apple's EP-018 equivalent, ⚠️ **which wants its OWN Epic rather than being folded
into an existing one.** Next Task **T-0492**; ⚠️ **next Issue I-0181, opening a new decade.** Prior
note follows.)*

*Last Updated: 2026-08-30 (**SP-126's six Tasks ✅ ALL VERIFIED** — user-approved after a live pass on
the **REAL RIG** at build 8: *"all verified in app. no findings."* ✅ **The three-tab shell is
delivered**: Writing · Worldbuilding · Properties, ⚠️ **and "Scene Entities" — Apple's deleted SP-090
placeholder — is finally retired.** ✅ **571/571 ctest + 19/19 smokes on the rig**, Qt 6.10.2.
⚠️ **THREE defects were found by LOOKING, none by any suite**: the Properties tab unreachable at 200px
(scroll arrows, a gesture-only affordance); outline and todo NEVER SAVED (`QTextEdit` has no
`editingFinished`, and ⚠️ **my own comment claimed a focus-out hook I had not written**); and I-0179,
⚠️ **which took THREE rounds because the first two fixed the symptom** — the real defect was that the
relationship label was REDUNDANT on every row, ✅ **hoisted to the group header by user ruling.**
⚠️ **I-0180 filed against APPLE** — the same wrong label has shipped on macOS since EP-031 unnoticed;
✅ **building the surface a second time is what exposed it.** ⚠️ **SP-126 is NOT CLOSED** — that needs
explicit user approval. Next Task **T-0492**; ⚠️ **next Issue I-0181, opening a new decade.** Prior
note follows.)*

*Last Updated: 2026-08-29, third pass (**SP-123 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) with its three Tasks verified and archived in the
SAME STEP → `Verified/Task-verified-0474-0476.md`. ✅ **The Audit Check before the close was CLEAN**:
7/7 DoD, all three Tasks Verified, nothing stale in backlog or unverified. ✅ **EP-038's AC1, AC2 and
AC3 are CLOSED.** ⚠️ **The rig runs the Linux app on REAL HARDWARE** — ⚠️ **and surfaced three Issues
(I-0176 no reopen, I-0177 no geometry, I-0178 single-project) within minutes, none findable by any
suite**; ✅ **user-ruled non-blocking**, and ⚠️ **I-0178 wants its OWN Epic** (Apple's EP-018
equivalent). ✅ **`docs/Scrivi_Linux_Rig_Setup_v0_1.md` written AS RUN** — ⚠️ **§7 (drive dismount)
deliberately EMPTY, owed by SP-124/T-0477.** ⚠️ **ONLY SP-126 is now active.** Next Task **T-0492**;
next Issue **I-0179**. Prior note follows.)*

*Last Updated: 2026-08-29, second pass (**SP-126 🟡 ACTIVATED** — EP-035's second sprint, ⚠️ **running
PARALLEL to SP-123**, which waits on `oathkeeper` being awake. ✅ **SP-126 is blocked on NOTHING** — the
four scene-notes endpoints are already bridged and `inspector-layout.json` needs no endpoint at all.
⚠️ **TWO CORRECTIONS FOUND AT PLANNING:** (1) ⚠️ **the draft's tab order was WRONG** — Apple's display
order is `Writing | Worldbuilding | Properties` (`InspectorTab` declaration order), defaulting to
Writing, ⚠️ **which is DISTINCT from the proving order** Writing → Properties → Worldbuilding;
(2) ✅ **the layout schema ALREADY EXISTS and is already populated** in the user's real projects —
`scrivi.inspector-layout.v1`, ⚠️ **APP-SIDE with no `scrivi_*` endpoint.** ⚠️ **T-0486 carries the
sprint's real risk**: Linux does not model `stackSort` or per-scene stacks, so it must PATCH the
document rather than reconstruct it — ⚠️ **reconstructing would silently delete the writer's
Apple-side card layout, invisibly until she reopened the project on the Mac.** ⚠️ **Accepted risk:
two app-side implementations of one schema; if a THIRD platform needs it, move it into ScriviCore.**
⚠️ **T-0491 (per-stack sort + drag-reorder) filed UNSCHEDULED**, user-ruled out of scope. Next Task
**T-0492**; next Issue **I-0176**. Prior note follows.)*

*Last Updated: 2026-08-29 (**SP-125 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) with its five Tasks already verified and archived
2026-08-28. ✅ **The Audit Check before the close was CLEAN**: no Task in two layers, no stale status,
all three of its Issues settled. ⚠️ **AC10 (the three-tab shell) was NOT in SP-125's scope** — it did not
exist when the sprint was planned — ✅ **so the sprint closed on its stated scope rather than being
retroactively widened**; the shell is **SP-126**'s. ✅ **I-0171 VERIFIED and archived** in the same step.
⚠️ **Only SP-123 remains active, and it is BLOCKED ON THE USER.** Next Sprint **SP-126** (⚠️ **planning COMPLETE 2026-08-29**, six Tasks T-0485–T-0490, blocked on
nothing); ⚠️ **T-0491 filed UNSCHEDULED** (per-stack sort + drag-reorder, user-ruled out of SP-126);
next Task **T-0492**; next Issue **I-0176**.)*
