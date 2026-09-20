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

⛔ **AC5 IS NOT MET. IT IS DEFERRED — and "narrowed" was the WRONG WORD.**

⚠️ **Nothing about the landing's open was made smaller. Only its BLOCKING was
removed.** The AC asks for the double open to be RESOLVED — either the landing's
result is handed to `EditorShell`, or the landing's open is reduced to what the
landing actually needs. ⛔ **Neither was done.**

✅ **MEASURED 2026-09-20** (`[I-0232] AC5` test, 10 chapters x 6 scenes):

| | Filesystem calls |
| - | ---------------- |
| First `openProject` | 196 |
| ⚠️ **Second `openProject`, same root** | ⚠️ **155 (~79%)** |

⚠️ **THE SECOND OPEN IS NOT FREE, and no existing cache absorbs it:**
- ⚠️ **`ProjectIndex` does NOT help.** It is reached via
  `CoreServices::sceneLocator`, which `openScene` consults to turn a sceneID into
  a path — ⛔ **`ProjectOpener` never touches it.** An `openProject` pays its own
  way every time.
- ⚠️ **[I-0231]'s `ReadThroughCache` does not span calls**, by design: it dies
  with the call that built it, because the filesystem is authoritative (EP-027)
  and Scrivi does no filesystem watching.

✅ **WHAT IS GENUINELY FIXED:** the 155 s FREEZE. Both opens now run off the UI
thread, so the launch screen stays alive and shows a busy strip throughout.
⚠️ **WHAT IS NOT:** the project is still opened TWICE at near-full cost. ✅ The
per-open cost fell ~3x from [I-0231], so the duplicated work is much cheaper than
it was — ⛔ **but it is still duplicated, and that is outstanding AC5 work.**

✅ **The user ruled this scope deliberately (2026-09-20)** — handing the result to
`EditorShell` would rework the one half that already works. ⚠️ **Recorded here so
the remaining cost is visible rather than remembered**, and the `[I-0232] AC5`
test will FAIL the day it is actually removed, forcing a rewrite.

---

## ⛔ What is NOT done

⛔ **AC6 — NOT MET.** ⚠️ **Everything above is macOS LOCAL DISK.** ✅ The AC is
explicit that Docker has no slow mount and the page cache hides the defect —
⚠️ **this is the exact gap that let [I-0195] be marked resolved while still
broken**, and it must not be repeated here.

✅ **THE QT HALF IS NOW COMPILED AND SMOKE-TESTED** (2026-09-20, Docker /
Ubuntu 24.04 / Qt 6.4).

⚠️ **THE FIRST BUILD FAILED, and the failure is worth recording**: `ScriviBridge.cpp`
used `kProjectOpenTimeoutMs` unqualified. ⚠️ **The constant had just moved to
`AsyncCall.hpp`, and only `EditorShell.cpp` carried the `using` declaration** —
✅ **exactly the kind of error a hand review does not catch and a compiler does in
one second.** ⛔ **"Reviewed and brace-balanced" is not "builds".**

✅ **After qualifying it: 0 errors, QML cache generated** (which is what validates
`Landing.qml`), ✅ **23/23 Linux smokes pass via their `.sh` wrappers** — ⚠️
**including `open_progress_smoke` (I-0195's determinate-progress guard) and
`lifecycle_smoke` (the full open loop)** — ✅ **and Linux `ctest` 611/612, the one
failure being the PRE-EXISTING `AC-A4`, identical to macOS.**

⚠️ **The smokes needed their `.sh` wrappers**: run bare, every one exits with
`usage: <name> <projectDir>`, which reads as 18 failures and is not one.

⚠️ **[I-0195] STILL MUST NOT BE MARKED VERIFIED** — AC4's code exists but is
unbuilt and unproven on the rig.


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