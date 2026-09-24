# Closed Sprint — SP-150

## SP-150: ⚠️ **[I-0213] — stop a structural op re-walking the WHOLE manuscript**

| Field | Value |
| ----- | ----- |
| **Sprint** | **SP-150** |
| **Epic** | **[EP-040]** `[Apple]` **The Editor Shell** |
| **Serves** | ✅ **[EP-040] AC12 and AC13** (⚠️ **both NEW, added with this Sprint**) |
| **Planned / Activated** | **2026-09-24** |
| **Size** | ⚠️ **MEDIUM** — ✅ **T-0549 small; ⚠️ T-0550 is the risky half** |

**Status:** ✅ **CLOSED 2026-09-24 (user-approved).**
✅ **T-0549 and T-0550 both VERIFIED by the user's live pass and archived** →
[`../../Tasks/Verified/Task-verified-0549-0550.md`](../../Tasks/Verified/Task-verified-0549-0550.md).
✅ **ALL SIX ACs MET.** ✅ **[EP-040] AC12 and AC13 MET; AC11 MET for [I-0213].**
⚠️ **RUN ONE AFTER THE OTHER by user ruling, NOT in parallel** — ✅ **T-0549 first as the proving
ground for the patch-or-drop fallback, then T-0550.**

⚠️ **THIS IS `[ScriviCore]` WORK UNDER AN `[Apple]` EPIC, AND THAT IS DELIBERATE.** ✅ **The cost is paid
by the EDITOR SHELL** — ⚠️ **the timeline's `reloadSceneDots` is what triggers the rebuild** — ✅ **so it
belongs to [EP-040] AC11's family even though the fix lands in the core.**

---

### ⚠️ Why this Sprint exists

✅ **[I-0213]** `[Apple]` — ⚠️ *creating a chapter froze the app ~2.7 s on a 1,174-scene manuscript.*
✅ **VERIFIED 2026-09-24 at `396.7 ms` of work and ACCEPTED as a limitation**, ⚠️ **measured deliberately
from a USB mount by the user so the figure is a WORST-CASE FLOOR, not a typical case.**
⚠️ **The user's framing:** *"On Linux it will be even worse because it has to also come through this
computer's operating system."*

✅ **[EP-040] AC11** — ⚠️ *the carried performance clause: the manuscript surface's remaining
O(DOCUMENT) costs are addressed OR accepted as limitations WITH a measurement* — ✅ **asked for
`reloadSceneDots`'s remainder to be INSTRUMENTED rather than inferred.** ✅ **THAT IS NOW DONE**, and
it is what this Sprint acts on.

⚠️ **THE MEASURED CAUSE** (`ScriviCore/tests/integration/StructuralOpIndexRebuildTests.cpp`, 2026-09-24).
✅ **COUNTED IN FILESYSTEM CALLS, NOT WALL CLOCK** — ⚠️ **a warm page cache hides exactly what a USB
mount exposes** (`project_read_amplification_class`; ✅ **the same reasoning [I-0231] forced onto
`OpenProjectReadAmplificationTests`**):

| Measured on 400 scenes / 20 chapters | Filesystem calls |
| ------------------------------------ | ---------------- |
| ✅ `createChapter` — **the op itself** | **50** (45 reads) |
| ⛔ **the `ProjectIndex` rebuild it FORCES** | **866** (866 reads) — ⚠️ **~17x the op** |
| ✅ warm `explicitStoryTimes()` | **0** |

⚠️ **THE REBUILD SCALES WITH MANUSCRIPT SIZE WHILE THE OP DOES NOT** — ✅ **which is why it surfaces on
the USB rig and is invisible on internal storage.**
✅ **THE PASS IS ALREADY NEAR-MINIMAL PER PASS: `ManuscriptOrderResolver` reads chapter and scene
SIDECARS, never scene BODIES** (⚠️ **~2.15 reads/scene**). ⛔ **THE WASTE IS THE SCOPE OF THE PASS, NOT
ITS EFFICIENCY.**

⚠️ **SIXTEEN endpoints hold `ProjectIndexInvalidation` (`scrivi_c_api.cpp:366`) and they are NOT
equivalent.** ✅ **That inequivalence IS the fix, and it splits this Sprint in two.**

---

### ⛔ THE CONSTRAINT BOTH TASKS MUST RESPECT

⛔ **NEITHER TASK MAY REMOVE THE INVALIDATION.** ✅ **It fixed a REAL defect — [EP-039] AC5b**
(⚠️ *[EP-039] `[Cross]` **Project Load Performance** replaced a `~300 s` frozen open; its **AC5b** is
the index-correctness clause: a failed index update DROPS THE WHOLE INDEX, and the next query rebuilds
and returns the CORRECT answer*).

⚠️ **THE CONCRETE DEFECT AC5b CAUGHT:** ⚠️ **with before-only invalidation,
`scrivi_set_scene_story_time` wrote correctly to disk and `scrivi_list_story_times` then reported
`count:0`** — ✅ **because the mutation's own locator lookup rebuilt the index FROM PRE-WRITE STATE.**

✅ **BOTH TASKS KEEP PATCH-OR-DROP:** ⚠️ **any partial path that fails, or is uncertain, FALLS BACK to
dropping the whole index.** ✅ **The fast path is an optimisation LAYERED ON the existing correct
behaviour — ⛔ never a replacement for it.**

---

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| ✅ **T-0549** | ⚠️ **Class A — PATCH the index in place for single-scene ops that do NOT change manuscript order** (`set_scene_story_time`, `clear_scene_story_time`, `rename_scene`) | **Medium** | ✅ **VERIFIED 2026-09-24** |
| ✅ **T-0550** | ⚠️ **Class B — rebuild ONLY the affected chapter for ops that DO change order or membership** (13 endpoints) — ⛔ **BLOCKED ON T-0549** | **Medium** | ✅ **VERIFIED 2026-09-24** |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| **I-0213** | ⚠️ **Chapter create costs `~400 ms` of work on a USB mount** (⚠️ *originally a 2.7 s freeze*) | ⚠️ **HIGH** | ✅ **VERIFIED 2026-09-24 and ARCHIVED** — ⚠️ **this Sprint attacks its ACCEPTED REMAINDER, it does not reopen the record** |

---

### Acceptance Criteria

- [x] **AC1** — ✅ **T-0549: a Class A op leaves the index WARM.** ⚠️ **MEASURED, not asserted: a
      `set_scene_story_time` followed by a story-time read costs **ZERO** manuscript reads**
      (⛔ **today: 866 on 400 scenes**).
- [x] **AC2** — ✅ **T-0550: a Class B op re-reads ONLY the affected chapter.** ⚠️ **On 400 scenes /
      20 chapters the forced rebuild falls from **866** to **roughly one chapter's share**.**
      ⛔ **THE EXACT BOUND IS SET FROM THE T-0550 MEASUREMENT, NOT INVENTED NOW** — ✅ **a number chosen
      before the work is a guess** (⚠️ **the rule `OpenProjectReadAmplificationTests` already states**).
- [x] **AC3** — ⛔ **[EP-039] AC5b STILL HOLDS, PROVEN BY ITS OWN TEST.** ⚠️ **A write followed
      immediately by an index-served read returns the WRITTEN value, never `count:0`.**
      ⚠️ **THE TEST MUST ASSERT THROUGH AN INDEX-SERVED ENDPOINT** — ⛔ **NOT through `openScene`,
      ✅ whose validate-on-use fallback silently REPAIRS staleness and would hide exactly this.**
- [x] **AC4** — ✅ **The patch-or-drop fallback is EXERCISED, not merely written**: ⚠️ **a test forces
      the partial path to fail and proves the whole index is dropped and the next query is CORRECT.**
- [x] **AC5** — ✅ **Assertions are on READ COUNT, not elapsed time** (`project_read_amplification_class`).
      ⚠️ **A wall-clock test passes on a warm local cache while doing 866 reads** — ⛔ **that is how this
      class hides.**
- [x] **AC6** — ✅ **`ctest` stays green in full** (⚠️ **628/628 at Sprint start, including the two new
      I-0213 measurements**), ✅ **and `StructuralOpIndexRebuildTests.cpp` reports the BEFORE/AFTER figures.**

### Sprint Notes

⚠️ **ORDER IS RULED BY THE USER: T-0549 THEN T-0550, one after the other.**
✅ **T-0549 is the smaller change and provably zero-I/O; it exercises the patch-or-drop fallback on the
simplest possible case BEFORE T-0550 takes the risk.**

⚠️ **T-0549 WILL NOT MOVE THE NUMBER THE USER MEASURED, AND THAT IS STATED UP FRONT.** ⛔ **A chapter
create is Class B.** ✅ **T-0549 helps TIMELINE EDITING (setting story times, renaming); ✅ T-0550 is the
one that moves chapter create.** ⚠️ **Do not report T-0549 as a fix for [I-0213]'s headline cost.**

⛔ **T-0550 IS THE RISKY HALF:** ⚠️ **partial rebuilds interact with ORDER KEYS, and [EP-027] made
manuscript order FILESYSTEM-AUTHORITATIVE for reasons that bit before** (⚠️ *the I-0072 phantom /
duplicate chapter entries in the rebuildable cache*).

⚠️ **THIS SPRINT DOES NOT REOPEN [I-0213].** ✅ **That record is VERIFIED and ARCHIVED; this Sprint
attacks the remainder it ACCEPTED AS A LIMITATION.** ⚠️ **If the work lands, the Issue is NOT
re-verified — ✅ the improvement is recorded against these ACs.**

✅ **LINUX INHERITS THE RESULT.** ⚠️ **[I-0244]** (*the Linux app has none of EP-040's editor shell*)
**already records the expectation that Linux will be WORSE than `396.7 ms` on the same fixture** —
✅ **this Sprint's gain is in the CORE, so Linux gets it for free once its shell calls the same path.**

### Retrospective

✅ **THE MEASURED OUTCOME, ON THE USER'S USB RIG (1,178 scenes):** ✅ **`reloadSceneDots` ~235 ms →
`1.4 ms`, about 170x.** ✅ **At the core: a full index rebuild costs `861` filesystem calls on 400
scenes; one chapter costs `63`.** ✅ **`ctest` 637/637.**

⛔ **THE LESSON WORTH KEEPING: T-0550's FIRST IMPLEMENTATION WAS INERT AND EVERY SIGNAL SAID
OTHERWISE.** ⚠️ **It built clean, passed every test, and the perf test reported 13.7x — because that
test measured `rebuildChapters` DIRECTLY while the ABI path never reached it.** ⚠️ **The guard's
before-drop, load-bearing for [EP-039] AC5b, had destroyed the very index the partial rebuild needed.**
✅ **A PROBE found it; a code read had not.** ✅ **The NEGATIVE CONTROL is what proved the fix: the same
deliberate break passed 30/30 before and failed 7 assertions after.**
⚠️ **This is `feedback_prove_code_is_reached`'s FOURTH occurrence** — ✅ **and the first time the check
was run BEFORE success was reported rather than after a user found the gap.**

✅ **THE USER'S OWN DIAGNOSIS CLOSED THE REMAINING QUESTION, AND THE CODE CONFIRMED IT.**
⚠️ **`createChapter WORK` fell only 396.7 → 316.3 ms inserting at the START of the manuscript**, ⛔ **far
less than removing ~235 ms of dot reload predicted.** ✅ **The user read it correctly: inserting at the
start renumbers EVERY following chapter; inserting at the end renumbers one.**
✅ **CONFIRMED at `ViewportSceneLoader.renumberChapterTitlesFrom:795`** — ⚠️ **it renumbers from the
insertion point onward and rebuilds `allScenes`; ✅ the skip-if-unchanged guard makes an end insert
nearly free.** ⚠️ **POSITION-DEPENDENT BY DESIGN, SWIFT-SIDE** (✅ **`ChapterCreator.cpp` writes only
the new chapter's sidecar**), ⛔ **and ALWAYS PRESENT — hidden behind the dot reload, not caused by it.**

⚠️ **WHAT THIS SPRINT DID NOT DO:** ⛔ **six Class B endpoints still drop the whole index because
their result structs do not name the affected chapter** — ✅ **a ruling, recorded at the call site.**
⛔ **[I-0206] is untouched and REMAINS OPEN.**
