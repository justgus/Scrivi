# SP-144 — ⚠️ **Project open cost: the amplification and the blocked thread**

**Status:** 🟡 **IN PROGRESS — work began 2026-09-20.** ⚠️ **ACTIVATION NOT YET USER-APPROVED — ⛔ Claude may not activate a Sprint.**
**Epic:** 🟡 **[EP-042]** `[Cross]` **Project Open Cost** → [`../Epics/Epic-EP-042.md`](../Epics/Epic-EP-042.md)
**Codebase:** `[Cross]` — `[ScriviCore]` **+** `[Linux]`
**Issues:** ⚠️ **[I-0231]** (`[ScriviCore]`, High) · ⚠️ **[I-0232]** (`[Linux]`, High)
**Task:** **T-0538**
**Date Created:** 2026-09-18

---

## ⚠️ The measurement this Sprint starts from

⚠️ **Taken 2026-09-18 on 🐧 `oathkeeper`, build 43, against the user's REAL project
`the-stairs-of-tintagael.scrivi` (10 chapters, 60 scenes, 150 files) on a `cache=none` CIFS mount.**

✅ **THE PROJECT IS INTACT AND THE ENVELOPE IS CORRECT** — ⚠️ **`mode: ready`, 60 scenes, a
byte-identical 30,548-byte envelope from macOS and Linux.** ⛔ **There is no data defect here.**

| Measurement | ⚠️ Value |
| ----------- | -------- |
| Files in project | **150** |
| ⚠️ **`read` syscalls, ONE `scrivi_open_project`** | ⚠️ **14,362** |
| ⚠️ **`openat` syscalls, same call** | ⚠️ **8,044** |
| ⚠️ **Opens per SCENE sidecar** | ⚠️ **~96×** |
| ⚠️ **Opens per CHAPTER sidecar** | ⚠️ **~98×** |
| ⚠️ **Opens of an ABSENT world `binding.json`** | ⚠️ **188 — ALL `ENOENT`** |
| ✅ **Open, LOCAL disk** | ✅ **0.21 s** |
| ⚠️ **Open, `cache=none` CIFS** | ⚠️ **154 s (~730×)** |
| ⚠️ **Full app load (open + 60 × `openScene`)** | ⚠️ **371 s** |
| ⚠️ **Reads across the whole app load** | ⚠️ **55,574** |

✅ **THE UI-THREAD SPLIT, sampled from `/proc/<pid>` every 2 s through a real load:**

```
t=51–206s   state=D   187 reads/2s   ← landing openProject: ONE thread, UI FROZEN   → I-0232
t=206–371s  state=S   426 reads/2s   ← EditorShell::load:  worker + live UI thread  → I-0195 works
t=371s                 12 reads/2s   ← idle; project loaded
```

⚠️ **THE RATE DOUBLING AT `t=206` IS THE PROOF** that a second thread joins. ✅ **[I-0195]'s async path
works — it is entered 155 s too late.** ⚠️ **The 155 s `D` span matches a standalone core probe's
154 s**, ✅ **so the landing call and the core call are the same cost on different threads.**

---

## ⛔ What is NOT yet decided — ✅ **this Sprint's first job**

⛔ **THE FIX SHAPE FOR [I-0231] IS NOT CHOSEN.** ⚠️ **The measurement says WHAT is wrong (the same
sidecars are re-read ~96×); ⛔ it does not say WHERE the redundancy is introduced.** ⚠️ **Candidates
NOT yet distinguished by evidence:**

- ⚠️ **`ManuscriptOrderResolver` re-resolving per scene** rather than once per open
- ⚠️ **`SceneIndex` / `ChapterIndex` rebuilt per query** instead of once per open
- ⚠️ **`EndpointResolver` / `WorldStore`'s documented per-endpoint re-read** widening under open
- ⚠️ **An `objects/index.json` read per object** rather than per open

⚠️ **AC1 IS TO LOCATE IT BY MEASUREMENT, NOT BY READING.** ⛔ **[I-0181]'s history is the warning:
three narrowings of one block, each made from inference.** ✅ **`strace -c` + a per-call-site counter
settles it; a code read does not.**

---

## Acceptance Criteria

**AC1** ⚠️ **LOCATE the amplification by measurement.** ✅ **Name the call site(s) and show the
per-site open/read counts for one `scrivi_open_project`** — ⛔ **not a narrative from reading the code.**

**AC2** ✅ **Reduce reads-per-file for project open to a small constant.** ⚠️ **NO TARGET FIGURE IS
STATED HERE** — ✅ **AC1's measurement sets it**, ⛔ **and a number invented before the cause is located
is a guess. The DIRECTION is: each sidecar is read ONCE per open unless a reason is recorded.**

**AC3** ✅ **An ABSENT world `binding.json` costs at most ONE probe per open**, ⚠️ **not 188.**
⛔ **Do NOT "fix" this by deleting the empty world directory** — ✅ **that is data the user owns, and
[I-0223] holds what it means.**

**AC4** ✅ **`Landing.qml`'s `openProject` runs OFF the UI thread**, ⚠️ **reusing `AsyncCall`
(`platforms/linux/src/AsyncCall.hpp`)** — ⛔ **NOT a second mechanism.** ✅ **The launch screen stays
responsive and shows progress for the WHOLE open**, ⚠️ **including the landing phase.**

**AC5** ⚠️ **The double open is resolved.** ✅ **Either the landing's result is HANDED to
`EditorShell`, or the landing's open is narrowed to what the landing actually needs** —
⚠️ **today the project is opened TWICE at full cost.** ⛔ **Whichever is chosen, record WHY.**

**AC6** ✅ **PROVEN ON THE REAL RIG under `cache=none`**, ⚠️ **not in Docker and not on local disk.**
⛔ **Docker has no slow mount and the page cache hides the defect** — ✅ **this is exactly the gap that
let [I-0195] be marked resolved while still broken.** ⚠️ **Report before/after wall-clock AND read
counts for the same project.**

**AC7** ✅ **A regression guard that fails on read-count, not on wall-clock.** ⚠️ **A timing test is
useless here — local disk passes it today at `0.21 s` while performing 55,574 reads.**

---

## ⚠️ Sequencing, and why it is not negotiable

⚠️ **[I-0231] BEFORE [I-0232].** ✅ **[I-0231] is what makes the wait go away.** ⛔ **Landing AC4 first
would produce a RESPONSIVE 6-minute open and make [I-0195] look verified while the real cost is
untouched.** ⚠️ **AC1–AC3 then AC4–AC5.**

---

## ⚠️ Blocked on this Sprint

⚠️ **[I-0195] MUST NOT BE MARKED VERIFIED until AC4 lands.** ✅ **Its async machinery is correct;
⛔ its call site is not the only one.**

---

## ⚠️ Out of scope

⛔ **Mount tuning.** ⚠️ **`cache=none` is the rig's drive-loss test configuration** — ✅ **the condition
that makes the defect visible, not the defect.**
⛔ **Apple's load path.** ⚠️ **[I-0231]'s fix is core-side and Apple inherits it** — ✅ **but no Apple
UI work is in this Sprint.**
⛔ **What an unbound world directory MEANS** — ⚠️ **[I-0223]'s territory.**


---

## ✅ AC1 — the measurement (2026-09-20)

⚠️ **LOCATED BY MEASUREMENT, NOT BY READING** — the instrument is
`ScriviCore/tests/mocks/CountingFileSystem.hpp`, a decorator over the REAL
`FileSystem` that forwards every call unchanged and attributes it to the pass
that made it. ⛔ **`strace` sees syscalls, not call sites**, so it could prove the
amplification but never its origin.

**One `scrivi_open_project`, 10 chapters x 6 scenes, 136 files, macOS local disk:**

| Pass | Calls | Reads |
| ---- | ----- | ----- |
| `1-migrateChapterOrderKeys` | 12 | 11 |
| `2-rebuildIndexIfInconsistent` | 12 | 11 |
| ⚠️ **`3-migrateScenes`** | ⚠️ **241** | ⚠️ **210** |
| `4-relationTypes+repairDangling` | 3 | 1 |
| ⚠️ **`5-validate`** | ⚠️ **203** | 71 (+132 `exists`) |
| ⚠️ **`6-resolveOrder`** | ⚠️ **151** | ⚠️ **140** |

✅ **THE ANSWER: SIX INDEPENDENT WALKS OVER THE SAME MANUSCRIPT.** ⚠️ **Every
chapter sidecar was read 8x and every scene sidecar 6x per open**, for bytes that
do not change between the first read and the last. ⚠️ **Inside `migrateScenes`
alone a scene sidecar is read 3x** — `listScenesByOrder`, then
`normalizeSceneContentPathIfStale`, then `rebuildChapterScenesIfInconsistent`
calling `listScenesByOrder` AGAIN.

⛔ **None of the four candidates this Sprint listed was the whole answer.** ✅ The
redundancy is not inside any one pass — each genuinely needs what it reads — it is
in the REPETITION ACROSS passes, which is why the fix belongs where the
repetition is visible rather than threaded through six call sites (the coupling
[I-0196]'s quadratic came from).

---

## ✅ AC2 / AC3 — the fixes, and what they measured

✅ **FIX 1 — `ScriviCore/src/util/ReadThroughCache.hpp`**, wrapped around
`*services_.fileSystem` for the length of ONE `openProject` and destroyed with it.
⚠️ **Write- and rename-invalidating**: the passes REPAIR (they rewrite sidecars),
so a stale hit would silently undo a repair. ⛔ **Never persistent** — the
filesystem stays authoritative (EP-027) and Scrivi does no filesystem watching on
any platform, so a cache that outlived the call could not know when it went stale.

✅ **FIX 2 — `repairDangling` now shares ONE `BindingCache` across its sweep**, and
the cache is plumbed through `WorldStore::resolve`.
⚠️ **[I-0207] FIXED `listPending` AND MISSED THIS ONE** — the more expensive miss,
because `listPending` runs when a writer opens the pending-edge view while
`repairDangling` runs on EVERY PROJECT OPEN. ⚠️ **It also threaded its cache only
into the `loadBinding` call on the PENDING branch, not into `resolve`**, which is
the call made for every endpoint x every bound world.

| Measure | Before | After |
| ------- | ------ | ----- |
| Calls per open | 622 | ✅ **196** (3.2x) |
| `readTextFile` per open | 444 | ✅ **85** (5.2x) |
| Calls per file | 4.58 | ✅ **1.44** |
| Worst single file (reads) | 8 | ✅ **2** |
| ⚠️ **Absent `binding.json` per sweep** | ⚠️ **25** | ✅ **1** |

⚠️ **THE 25 WAS VERIFIED BY REVERTING THE FIX AND RE-RUNNING, not assumed** — and
the AC2 guard was likewise verified to FAIL with the cache removed. ⛔ A guard
never seen red is not a guard.

✅ **The remaining 2 reads of a chapter sidecar are the repair working**:
`migrateScenes` rewrites the scenes[] cache, the write correctly invalidates the
cached copy, and the next pass re-reads the NEW bytes.

---

## ✅ AC7 — the guard

✅ **Asserts on READ COUNT, never wall-clock**, per the AC: local disk passes a
timing test today at `0.21 s` while performing 55,574 reads. ✅ Bounds live in
`ScriviCore/tests/CMakeLists.txt` as compile definitions so tightening one is a
visible change, each carrying its measured justification.

✅ **Scaling: 1.71 calls/file at 42 files vs 1.56 at 216** — per-file cost FALLS as
the manuscript grows, which is the opposite of [I-0196]'s quadratic signature.

---

## ✅ AC4 / AC5 — the landing (⛔ NOT COMPILED)

✅ **AC4**: `ScriviBridge::openProjectAsync` reuses `AsyncCall`; `Landing.qml`'s
blocking call is gone; the launch screen shows a busy strip and disables
New/Open/recents while opening. ⚠️ **Indeterminate on purpose** — the landing has
no scene count to divide by, and [I-0195]'s ruling forbids a spinner pretending to
be a percentage.

✅ **AC5 IS MET — THE PROJECT IS OPENED ONCE.**

⚠️ **The earlier "deferred / narrowed" framing was WRONG and is withdrawn.**
Nothing had been made smaller; only the blocking had been removed. ⚠️ **The user
rejected that scope (2026-09-20): "Fix them both."**

✅ **`Landing.qml` hands its envelope to the editor:**
`shell.openEditor(path, title, result)` → `ShellController::openEditor` →
`ScriviWindow::showEditor` → `EditorShell::load(..., openedProject)`, and the
worker uses it instead of calling `openProject` again.

✅ **THIS IS APPLE'S SHAPE, NOT A NEW MECHANISM.** `ProjectSession.loadAsync`
already opened ONCE on a worker and passed `result.scenes` into the scene loop.
⚠️ **Linux was the platform that had drifted** — ✅ **and the user made this a
STANDING RULE: Linux must ALWAYS adopt Apple's shape, and a shape change on Apple
must be made the same way on Linux, in the same work.**

⚠️ **THE EMPTY-ENVELOPE PATH IS RETAINED DELIBERATELY**, for the two RELOAD sites
after a structural edit (`EditorShell.cpp:940,989`), which MUST re-read disk, and
for New Project, where the project was CREATED rather than opened. ⛔ Both must
stay behaviourally identical to the handed-over path; only the OPENER differs.

### ✅ End-to-end, the full landing→editor chain (61 scenes, shipped ABI, `strace -c -f`)

| Syscall | Before | After | |
| ------- | ------ | ----- | - |
| ⚠️ **`write`** | ⚠️ **62** | ✅ **1** | ⚠️ **−98%** |
| `openat` | 512 | ✅ **365** | −29% |
| `read` | 766 | ✅ **617** | −19% |
| `newfstatat` | 626 | ✅ **496** | −21% |
| **TOTAL** | **1,966** | ✅ **1,479** | ⚠️ **−25%** |

⚠️ **THE −25% UNDERSTATES THE REAL GAIN ON THE RIG.** On local disk a write is
cheap; on a `cache=none` share each of the 61 eliminated writes was a
temp-create + write + rename ROUND-TRIP. ✅ That is the cost the writer feels on a
SMALL project.

---

## ⚠️ A REGRESSION THIS SPRINT NEARLY SHIPPED — ✅ caught and fixed

⚠️ **[I-0234] removed `openScene`'s implicit per-read surface stamp. On APPLE
that was safe; on LINUX it was NOT, and the difference was nearly missed.**

✅ **Apple was already protected**: `saveAllDirtyBlocking` saves the current scene
UNCONDITIONALLY and then calls `stampWritingSurfaceBlocking`
(`ViewportSceneLoader.swift:411`), which exists precisely so *"a scene the writer
scrolled to but never edited still resumes correctly"* ([I-0058]/[I-0131]).

⛔ **Linux had NO equivalent** — it saved only DIRTY scenes. ⚠️ **So a writer who
NAVIGATED without typing would have had nothing recording their place, and
reopening would have landed them on the wrong scene — silently.**

✅ **FIX: `EditorShell::stampWritingSurface()`, called from
`ScriviWindow::flushEditor()`** — the single chokepoint every teardown reaches
(Close, New, Open, window close, `aboutToQuit`). ⚠️ **ONE write on teardown, not
one per scene on load.**

✅ **Guarded by a test** (`[SP-144][I-0234]`) asserting that `saveScene` records
`lastWritingSurface` and that a reopen resumes on the saved scene — ⛔ **because
if that ever stops being true, resume breaks with no error anywhere.**

---

---

## ⚠️ A finding this Sprint did NOT go looking for — ✅ [I-0233]

⚠️ **The user asked whether Linux uses the performance state Apple has.**
✅ **THE ANSWER INVERTED: Linux is the platform doing it correctly.**

✅ **Everything below the ABI is genuinely shared** — `CoreServices::sceneLocator`
is wired in `CoreSingleton`'s constructor (`scrivi_c_api.cpp:150`) and
`abiServices()` (`:176`), so the `ProjectIndex` seam that turned [I-0196]'s
234–251 s into a hash lookup serves BOTH platforms with no per-platform work.
✅ **[I-0231]'s fixes are core-side and Apple inherits them unchanged.**

⚠️ **But `scrivi_close_project` — which RELEASES that index — is called ONLY by
Linux.** ⛔ **The Apple tree has zero call sites.** ⚠️ **[T-0512] is recorded
✅ Verified and its text claims the Apple half shipped**; the Linux half is real,
the Apple half is not. ✅ **Filed as [I-0233] (Medium, unassigned)** — ⛔ **NOT
this Sprint's to fix.**

⚠️ **It is also a VERIFICATION defect, not only a code one**: a Task was marked
Verified for work that was half done.

---

## ✅ WHAT FIXES A LARGE PROJECT — ✅ measured, 2026-09-20

⚠️ **The user asked the question this Sprint had not answered: the small-project
fix was [I-0234], so what fixes a LARGE one?**

✅ **THE SAME FIX DOES, AND THE SCALING IS WHY.** ⚠️ **Before it, a bulk load's
DURABLE WRITES were O(scenes)** — every `openScene` re-stamped
`lastWritingSurface`. ✅ **They are now O(1).**

**Measured on the shipped ABI (🐧 Linux, `strace -c -f`, landing→editor chain):**

| Scenes | ⚠️ Syscalls BEFORE | ✅ AFTER | ⚠️ **Writes BEFORE** | ✅ **Writes AFTER** |
| ------ | ------------------ | -------- | -------------------- | ------------------- |
| **60** | 1,966 | ✅ **1,479** | ⚠️ **62** | ✅ **1** |
| **120** | 3,706 | ✅ **2,799** | ⚠️ **122** | ✅ **1** |
| **240** | 7,186 | ✅ **5,439** | ⚠️ **242** | ✅ **1** |

⚠️ **AT 240 SCENES THAT IS 242 DURABLE WRITES ELIMINATED**, ✅ **each one a
temp-create + write + rename ROUND-TRIP on a `cache=none` volume.** ⚠️ **The
larger the manuscript, the more this fix is worth — which is the opposite of how
it was first described.**

✅ **THE LOAD IS NOW LINEAR IN SCENE COUNT.** ⚠️ **60 → 240 scenes (4×) costs
1,479 → 5,439 syscalls (3.7×), with per-scene cost FALLING (24.6 → 22.7).**
⛔ **No quadratic term remains** — ✅ **which is what [I-0196] ("invisible at 16
scenes, fatal at 1,152") existed to prevent.**

✅ **GUARDED** by `[SP-144][I-0234][scaling]`, which asserts writes are INDEPENDENT
of scene count. ⛔ **A total-call budget would NOT catch a regression here** —
⚠️ **reads legitimately grow with the manuscript, so the defect would hide inside
a rising total.**

---

## ✅ `AC-A4` FIXED — ⛔ it was never "pre-existing", it was UNFINISHED

⚠️ **Reported as a pre-existing failure through several messages of this Sprint.**
⛔ **That was the wrong call** — ✅ **the user's instruction: "not be stashed but
rather fixed. now!"**

✅ **ROOT CAUSE: a STALE TEST, not a defect.** ⚠️ **The user's own [I-0222] ruling
(2026-09-17) unified the error `detail` to ONE spelling, `worldUnavailable:`.**
✅ **Apple's suite was updated** (`ScriviInteropTests.swift:2619` asserts the old
spelling is RETIRED) — ⛔ **`ObjectCApiTests.cpp` was MISSED** and kept asserting
`worldPending:`. ⚠️ **So `ctest` failed on BOTH platforms for three days while the
CODE WAS CORRECT.**

✅ **FIXED by DERIVING the expectation from `kWorldUnavailableDetailPrefix`** —
⛔ **not by writing the new literal**, ✅ **because restating it is exactly how the
two spellings drifted apart in the first place** (the same standing rule CLAUDE.md
states for `ObjectKind` lists).

⚠️ **THE PROCESS LESSON: a known-red test is indistinguishable from a tolerated
one.** ✅ **It trained this Sprint to read 611/612 as success, which is precisely
the slot a real regression would hide in.**

✅ **BOTH SUITES ARE NOW FULLY GREEN: macOS 613/613 · Linux 617/617 · 23/23 smokes.**


---

## ⚠️ WHAT REMAINS TO BE VERIFIED — the complete list (2026-09-20)

### ✅ What the user's stated Linux pass already covers

| Covers | AC | ⚠️ Note |
| ------ | -- | ------- |
| ✅ The load happens ONCE | **AC5** | ✅ **"appears to happen once" + "much less time to load" IS the AC5 evidence** |
| ✅ Both progress bars appear and update | **AC4** | ⚠️ **INDETERMINATE on the launch screen (landing has no scene count yet), then DETERMINATE `n of m scenes` in the editor. Seeing BOTH is the point — one alone means the handoff broke** |
| ✅ Launch screen stays responsive | **AC4** | ✅ **This is [I-0232]'s whole defect: 155 s frozen and silent** |
| ✅ Drive pull asserts correctly | — | ✅ **Guards [I-0193]/[I-0181]/[I-0221] against regression from this Sprint's changes** |

### ⛔ WHAT IS NOT COVERED BY WATCHING THE UI

**1. ⛔ AC6 — the NUMBERS. ⚠️ THE ONE THING STILL OUTSTANDING.**
⚠️ **AC6 does not ask "is it faster"; it asks for BEFORE/AFTER WALL-CLOCK *AND*
READ COUNTS for the SAME project under `cache=none`.** ⛔ **A stopwatch cannot
produce the read counts, and the read counts are the half that survives a
faster machine.** ✅ **Run:**

```bash
platforms/linux/tools/sp144-open-cost-probe.sh \
    /mnt/scrivi-worlds/the-stairs-of-tintagael.scrivi
```

✅ **It reports the MOUNT OPTIONS first and says plainly when the mount is not
`cache=none`** — ⚠️ **because `cache=strict` MASKS this defect and a run under it
is not AC6 evidence.** ✅ **It runs the OLD path first (the unflattering order, on
purpose) and prints AC3's `binding.json` count.**

**2. ⚠️ A NEW RISK THIS SPRINT INTRODUCED — ✅ cheap to check while you are there.**
⚠️ **[I-0234] removed `openScene`'s implicit surface stamp, and Linux had no
replacement until `EditorShell::stampWritingSurface()` was added in the same
work.** ⛔ **The failure mode is SILENT — no error, the writer simply reopens on
the wrong scene.** ✅ **Check: scroll to a scene WITHOUT TYPING, quit, reopen —
it must land where you left it.** ⚠️ **Do it without typing; typing marks the
scene dirty and would pass for the wrong reason.**

**3. ⚠️ [I-0233] — Apple, not Linux.** ✅ **`scrivi_close_project` is now called
from `ProjectSession.close()`.** ⚠️ **Needs a macOS pass: open project A, close
it (red button / tab ✕ / File ▸ Close Project), open project B — memory should
not retain A's index.** ⛔ **Not verifiable on Linux; Linux always did this.**

### ⚠️ What CANNOT be closed by this Sprint

⚠️ **[I-0195] is Verifiable ONLY once AC6 passes** — ✅ **its own status line says
so, and AC4's code is what unblocks it.**

### ✅ Everything else is already evidenced

✅ **AC1** (the per-pass attribution table), ✅ **AC2** (622 → 196 calls/open),
✅ **AC3** (25 → 1 binding reads, `strace`-confirmed), ✅ **AC7** (read-count
guards, verified failing with the fix reverted) — ⚠️ **all measured, and all
re-checked on Linux: `ctest` 617/617, 23/23 smokes.**
