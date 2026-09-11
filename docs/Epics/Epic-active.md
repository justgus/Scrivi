# Active Epics

⚠️ **TWO Epics are active**, running in parallel by user ruling 2026-08-25:
**EP-035** `[Linux]` Object Foundations · **EP-038** `[Linux]` The Real Hardware Rig.
⚠️ **EP-038's first task is blocked on PHYSICAL ACCESS**; EP-035's early work is blocked on nothing.

> ⚠️ **PRIORITY RESET — 2026-09-10, user ruling.** ✅ **[EP-039] was promoted and every other Epic
> backlogged**: *"It is the most important thing right now."*
>
> ⚠️ **WHY, in one measurement:** ⚠️ **an ORDINARY 1,153-scene manuscript freezes the app for `321 s`
> with no progress and no cancel** — ⚠️ **the user ran it for over an HOUR before killing it.**
> ⚠️ **The cost is QUADRATIC, so it is invisible at 16 scenes and fatal at 1,153.**
>
> ⚠️ **[EP-035] `[Linux]` Object Foundations was DEFERRED mid-Epic** (2 of 4 ACs open) → `Epic-backlog.md`.
> ✅ **Its 2 verified ACs are NOT lost.** ⚠️ **Its open AC5 (card thumbnails) is DEPENDENT on EP-039's AC7**
> — ✅ **the rule that a blob index holds LOCATION AND SHAPE, never the bytes** — ⚠️ **so doing AC5 first
> would design the memory behaviour twice.**
>
> ⚠️ **[EP-038] is NOT backlogged — it is COMPLETE** (all 7 ACs closed). ⚠️ **[SP-128] remains ACTIVE under
> it only because T-0501's rig pass is BLOCKED on the rig being powered on.**

## EP-039: `[Cross]` Project Load Performance — ⚠️ **the in-memory index, and the async gap**

**Status:** 🟡 **ACTIVE — promoted 2026-09-10 by user ruling**, ⚠️ **preempting EP-035**: ✅ *"the most important thing right now."* ⚠️ **Created the same day from [I-0196]'s root-cause analysis.**
**Codebase:** `[ScriviCore]` (the indexes + bulk endpoints) **+ `[Apple]`** (the async gap). ⚠️ **Linux
already has its half** (SP-128 / T-0499/T-0500).
**Goal:** ⚠️ **A project of ORDINARY SIZE opens without freezing the app**, ✅ **and the core stops
answering per-item questions with full-tree traversals.**
**Design:** [`../Scrivi_Project_Index_Design_v0_1.md`](../Scrivi_Project_Index_Design_v0_1.md)
**Date Created:** 2026-09-10 · **Target Close:** — (⚠️ **estimated 3–4 sprints, before implementation**)
**Sprints:** 🔵 **SP-129** (`[Apple]` the four unbuilt surfaces) · 🔵 **SP-130** (`[Apple]`+`[ScriviCore]` close the ScriviCore bypasses — [I-0197]) — ⚠️ **both in `Sprint-backlog.md`, PLANNING, not activated**

### ⚠️ Why this Epic exists — MEASURED, not estimated

⚠️ **One project open, 1,153 scenes: `321.10 s` frozen, no progress, no cancel.** ⚠️ **The user ran it for
over an HOUR before killing it.**

| phase | measured |
| ----- | -------- |
| `TimelineViewModel.load` | ⚠️ **251.02 s** (78%) |
| `loader.loadAll` (1,153 × `openScene`) | ⚠️ **69.80 s** |
| everything else | ✅ **< 0.3 s** |

⚠️ **1,153 scenes is an ORDINARY MANUSCRIPT, not a stress test.** ⚠️ **The cost is QUADRATIC, so it is
invisible at 16 scenes and fatal at 1,153** — ✅ **which is why no earlier sprint caught it.**

### ✅ Root cause — an OMISSION in ScriviCore, not a coding slip

✅ **`ScriviCore` is STATELESS** (its only member is `CoreServices services_`). ⚠️ **So a per-item question
costs a FULL TRAVERSAL by construction** — `findSceneMetaPath` resolves the ENTIRE manuscript to turn one
`sceneID` into one path (**7 call sites**), and **28 more** sites build a `ManuscriptOrderResolver` and
resolve the whole tree. ⚠️ **The app then loops these per scene.**
✅ **Nobody wrote a nested loop; the quadratic is EMERGENT.** ⚠️ **The core exposes per-item endpoints and
almost no bulk ones, so the app has NO EFFICIENT WAY TO ASK.**

### Acceptance Criteria (draft — ⚠️ to be ruled at promotion)

- **AC1** — ✅ **`SceneLocationIndex`** (`sceneID` → paths, chapter, ordinal) replaces `findSceneMetaPath`'s
  full resolve. ⚠️ **Measured target: `8–116 ms` → O(log N).**
- **AC2** — ✅ **`SceneStoryTimeIndex`** replaces the timeline's per-scene read. ⚠️ **Measured target:
  `251 s` → a single build pass.**
- **AC3** — ✅ **`ManuscriptOrderIndex`** replaces the **28** resolver call sites; ✅ **one traversal at open
  feeds all three indexes.**
- **AC4** — ✅ **`scrivi_list_story_times` — SPARSE**: returns a record ONLY for a scene whose story time is
  EXPLICITLY SET. ⚠️ **MEASURED: on a 1,203-sidecar fixture, ZERO scenes have a `storyTime` block — the key
  is `null`** — ✅ **so the call returns an EMPTY ARRAY and the timeline draws its default chain with NO
  per-scene I/O.** ⚠️ **`251 s` was spent discovering that nothing is set.**
  ⚠️ **THE EMPTY-ARRAY TRAP APPLIES and empty is the COMMON case** (`project_envelope_empty_vs_failed`):
  ⚠️ **the caller MUST use the failure signal, never emptiness**, or a real timeline reads as empty.
- **AC5** — ⚠️ **INVALIDATION IS RULED, not assumed.** ⚠️ **A silently stale index is WORSE than no index** —
  ✅ **[I-0183] destroyed 10 of 12 relationships exactly that way** (a world resolved `available` while its
  index was unreadable). ⚠️ **An index MISS must fall back to a real traversal, NEVER to a negative claim**
  (*absence is never deletion*). ⚠️ **External change (git checkout, Finder rename, a sync client) does NOT
  go through the core** — ✅ **Doc 2's repair matrix owns that question and the index must not invent a
  second answer.**
- **AC6** — ⚠️ **`[Apple]` THE LOAD RUNS OFF THE MAIN THREAD, with a determinate progress bar.**
  ✅ **MEASURED: 93 engine call sites in `Scrivi/App` + `Scrivi/Views`; ZERO run off the main thread** — no
  `Task.detached`, no `DispatchQueue.global`, no `nonisolated` work anywhere. ⚠️ **`ProjectSession` is
  `@MainActor`, so every call it makes blocks the UI.** ✅ **Linux solved this in SP-128; ⚠️ Apple never
  did** — ⚠️ **this is [I-0195] on Apple.**
  ⚠️ **AC6 IS INDEPENDENT OF AC1–AC4 AND BOTH ARE REQUIRED:** ⚠️ **index alone ⇒ fast, but still freezes on
  slow storage; async alone ⇒ 321 s of honest, watchable, unusable waiting.**
- **AC7** — ✅ **`AssetLocationIndex` — the TEXT/BLOB prong.** ⚠️ **The index holds LOCATION AND SHAPE ONLY
  (path, byte size, hash, mtime) — NEVER THE BYTES.** ⚠️ **WHY THIS IS RULED NOW: Scene/Object/Item image
  THUMBNAILS need the same mechanism and are potentially memory-intensive.** ✅ **1,153 scene bodies is a
  few MB; ⚠️ 1,153 thumbnails is NOT** — ⚠️ **a design that "just caches the content" works for text and
  then falls over on images.** ✅ **Bytes fetched on demand, LRU bounded by MEMORY, not by count.**
- **AC8** — ⚠️ **A REGRESSION TEST PINS THE COMPLEXITY, not a duration.** ⚠️ **A timing assertion is flaky
  and explains nothing** — ✅ **assert that opening one scene does not cost work proportional to how many
  OTHER scenes exist** (the read-counting decorator pattern, as [I-0196]'s test already does).

### ⚠️ SCOPE EXPANDED 2026-09-10 (user ruling) — ⚠️ **twice, and both from AUDIT FINDINGS**

⚠️ **This Epic began as "make project open fast."** ⚠️ **Auditing for dead functions found no dead
functions — ✅ it found UNBUILT SURFACES ([SP-129]) and then CORE BYPASSES ([I-0197] → [SP-130]).**

✅ **Both belong here rather than in a separate Epic, for a MEASURED reason, not a scheduling one:**
⚠️ **`TimelineViewModel.loadImportedTimelines` bypasses the core ON THE TIMELINE LOAD PATH** — ⚠️ **the
same path measured at `251 s`.** ✅ **A bypass is a place the core's guarantees do not apply**, ⚠️ **so
indexing the core while the app reads around it would leave the acceleration unreachable.**

### ⚠️ Out of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **Retiring "dead" core functions** | ✅ **AUDITED 2026-09-10: there are NONE.** ⚠️ **All 101 `scrivi_*` endpoints are referenced; the four `ScriviEngine` methods with no Apple caller are ALL LIVE ON LINUX** — ⚠️ **they are APPLE SURFACES NEVER BUILT, a PARITY GAP deserving its own Issue, not a cleanup** |
| ⚠️ **Lazy-loading scene BODIES** | ⚠️ **NOT ruled.** ⚠️ **The editor loads all 1,153 eagerly and Spotlight re-reads all 1,154 on every open AND every resign** — ⚠️ **both real, NEITHER measured in isolation.** ✅ **Measure, then rule** |
| ⚠️ **A core-held open-project SESSION** (beyond the indexes) | ⚠️ **Deliberately not proposed** — ⚠️ **it reintroduces the cache-invalidation and external-change questions EP-027 settled by making the filesystem authoritative** |

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The index becomes a SECOND SOURCE OF TRUTH** | ⚠️ **EP-027's filesystem-authoritative ruling STANDS.** ✅ **The index is DERIVED and DISPOSABLE; any doubt ⇒ rebuild from disk** |
| ⚠️ **Silent staleness** | ⚠️ **AC5.** ✅ **[I-0183] is the precedent for how this destroys data** |
| ⚠️ **Fixing the index and declaring victory** | ⚠️ **AC6 is the other half.** ✅ **A fast synchronous load is still a freeze on slow storage** |
| ⚠️ **Thumbnails designed after the fact** | ✅ **AC7 rules the blob shape NOW, while the JSON prong is still soft** |

---

---

## EP-038: `[Linux]` ⚠️ **The Real Hardware Rig** — native Ubuntu + drive-loss ground truth

**Status:** ✅ **COMPLETE 2026-09-10** — ⚠️ **AWAITING USER APPROVAL TO CLOSE.** ✅ **ALL SEVEN ACs CLOSED (AC1–AC7).** ⚠️ **NOT backlogged — an Epic with nothing left to do is COMPLETE, not deferred.** ⚠️ **[SP-128] is still ACTIVE under it** — ✅ **T-0499/T-0500 Implemented; ⚠️ T-0501 (the rig live pass) is BLOCKED on the rig being powered on** — ⚠️ **so the Epic cannot be CLOSED until that verification lands, but no new work belongs here.** ⚠️ **Original status: promoted 2026-08-25 by user ruling.**
**Codebase:** `[Linux]` + ⚠️ **environment/infrastructure**, plus one Qt/C++ deliverable.
**Goal:** Stand up a **real Ubuntu machine** as a first-class test rig, then ⚠️ **establish what ACTUALLY
happens when a drive carrying a world is physically unmounted** — and implement the platform refinement
against that ground truth rather than against documentation.
**Date Created:** 2026-08-24 · **Promoted:** 2026-08-25 · **Sprints:** ✅ **SP-123 (CLOSED 2026-08-29)**, ✅ **SP-124 (COMPLETE 2026-09-10, awaiting close)**, 🟡 **SP-128 (ACTIVE 2026-09-10)**
**Tasks:** **T-0474 – T-0479** (six) + ⚠️ **T-0498** (`[ScriviCore]`, SP-124) + 🔵 **T-0499 – T-0501** (SP-128) — ⚠️ **nine plus one**
**Blocks:** ⚠️ **EP-036's AC4 is UNSPECIFIABLE until T-0477 reports.**
**Runs in PARALLEL with EP-035** — ⚠️ **user ruling 2026-08-25** (see §3).

---

## 1. ⚠️ Why Docker cannot answer this — the structural reason

⚠️ **This is NOT "Docker is inconvenient." The container is BLIND to the failure mode.**

| | Real USB unplug | Docker bind-mount stop |
| - | --------------- | ---------------------- |
| Open file descriptors | ⚠️ **`EIO` / `ESTALE`** | cleanly gone |
| `/proc/mounts` entry | ⚠️ **vanishes, possibly leaving a stale mount point** | never existed as a volume |
| Kernel events | ⚠️ **udev / uevent traffic** | none |
| Partial writes in flight | ⚠️ **may be torn** | not exercised |

### ✅ The precedent — Apple's own code, and it is unambiguous

`Scrivi/App/WorldVolumeStatus.swift:15-23`:

> ⚠️ **`volumeIsRemovable` : false** on a drive unplugged by hand.
> ⚠️ **`volumeIsEjectable` : false** — `diskutil` agrees: *"Removable Media: Fixed"*.

⚠️ **The documented API LIED.** The signal that worked (`volumeIsLocal`) was found ⚠️ **by pulling a real
drive.** ⚠️ **Drive-loss-while-editing cost Apple SIX Issues** (I-0162, I-0165, I-0165b, I-0166, I-0167,
I-0168) — ⚠️ **every one found by ejecting a drive, NONE by a suite.**

### ⚠️ What is missing, measured

| Check | Result |
| ----- | ------ |
| `WorldVolumeStatus` equivalent in `platforms/linux/` | ⚠️ **DOES NOT EXIST** — zero hits for `unmounted` |
| Apple's implementation | **121 lines** |
| Who decides `unmounted` vs `offline` | ⚠️ **NOT the core** — `WorldStore.hpp:28-35` calls it a **"platform-layer refinement"** |

---

## 2. ✅ USER RULING (2026-08-25) — display: **native desktop + remote convenience**

> ✅ **Ubuntu Desktop with a real display, ALSO reachable remotely** so live passes can be driven when the
> user is not at the machine.

**Three consequences, all binding:**

1. ✅ **The GUI runs in a REAL X session, never Xvfb.** ⚠️ **`xvfb`/`x11vnc` are the CONTAINER's answer
   and are not the rig's.**
2. ⚠️ **The drive pull is ALWAYS PHYSICAL.** ⚠️ **No remote path substitutes for it** — that is the entire
   point of the Epic.
3. ⚠️ **Remote input may STILL drop some modifiers.** ⚠️ **The no-gesture-only rule
   (`project_linux_vnc_input_constraints`) therefore STANDS** — ⚠️ **it is NOT retired by this rig**,
   because a live pass may still be driven remotely. ✅ **Every action still needs a button or menu path.**

---

## 3. ✅ USER RULING (2026-08-25) — EP-038 runs in PARALLEL with EP-035

⚠️ **T-0474 (account + SSH) is blocked on PHYSICAL ACCESS and gates everything else in this Epic.**
✅ **EP-035's early work is pure Qt/QML and is blocked on nothing.**

⚠️ **The dependency is WEAKER than it looks and this is recorded so it is not overstated:**
`ScriviBridge::getWorldStatus` **already crosses the bridge** (`ScriviBridge.cpp:889`), so ⚠️ **EP-035's
AC3 can ship a disabled-and-explained world on the CORE's status alone.** ✅ **EP-038's refinement is a
BETTER EXPLANATION, not a prerequisite for the surface.** ⚠️ **Only EP-036's AC4 is genuinely blocked.**

⚠️ **The real reason to run EP-038 early is to avoid verifying EP-035's surfaces TWICE** — once through
VNC and again on real hardware.

---

## 4. Sprints

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **SP-123** | ⚠️ **Rig reachable + building natively** — T-0474 – T-0476 | ✅ **CLOSED 2026-08-29** |
| **SP-124** | ⚠️ **Ground truth + refinement** — T-0477 – T-0479 | ✅ **COMPLETE 2026-09-10 — awaiting close approval** — ⚠️ **scope WIDENED to three scenarios; blocked on the rig being AWAKE** |
| 🟡 **SP-128** | ⚠️ **Honest waiting** — T-0499 – T-0501 — ⚠️ **[I-0195] progress UI + [I-0182] rig verification** | 🟡 **ACTIVE 2026-09-10** — ⚠️ **added by USER RULING: EP-038's ACs are all closed, but the rig's own findings are not** |

⚠️ **The sprint seam is deliberately AT the blocking point.** ✅ **T-0477 (instrument) cannot leak into
T-0478 (implement) because a sprint boundary separates them.**

## 5. Tasks

| ID | Title | Sprint | Owner |
| -- | ----- | ------ | ----- |
| **T-0474** | ⚠️ **Account + SSH key exchange + network reachability** — ⚠️ **PHYSICAL/CONSOLE ACCESS** | SP-123 | ⚠️ **USER** |
| **T-0475** | **Toolchain + dependencies** — Qt6 ≥ 6.4, CMake, ninja, `libssl-dev`, QML modules, ⚠️ **plus desktop + remote-desktop server**; ⚠️ **NOT xvfb/x11vnc** | SP-123 | Claude (SSH) |
| **T-0476** | **Clone + FIRST NATIVE BUILD** (⚠️ **no container**) + `ctest` ⚠️ **non-root, tests ON** + the Qt app **launches on the real display** | SP-123 | Claude (SSH) |
| **T-0477** | ⚠️ **DRIVE-LOSS INSTRUMENTATION** — ⚠️ **FINDINGS ONLY, NO CODE.** What `/proc/mounts`, `statfs`, open FDs and the kernel actually report on a physical pull | SP-124 | ⚠️ **USER pulls**; Claude instruments |
| **T-0478** | ⚠️ **`WorldVolumeStatus` for Linux**, written **against T-0477's findings** — ⚠️ **NEVER from documentation** | SP-124 | Claude |
| **T-0479** | ⚠️ **Correct the Porting Outline's §9** from what the rig actually taught | SP-124 | Claude |

⚠️ **T-0478 MUST NOT START BEFORE T-0477 REPORTS.** ✅ **Enforced by the sprint boundary, not by intent.**

---

## 6. Acceptance Criteria

- [x] ✅ **AC1 — CLOSED by SP-123 (2026-08-29).** ⚠️ **The Ubuntu box is reachable over SSH** with key
      auth and a documented path → `docs/Scrivi_Linux_Rig_Setup_v0_1.md` §2.
      ⚠️ **NO passwords, keys or hostnames in any tracked file.**
- [x] ✅ **AC2 — CLOSED by SP-123 (2026-08-29):** native build **outside** a container, `ctest`
      **571/571 NON-ROOT** plus 18/18 smokes, ⚠️ **on Qt 6.10.2 / GCC 15.2 / CMake 4.2.3 with ZERO code
      changes.** ⚠️ **The Qt app BUILDS AND RUNS NATIVELY on Ubuntu** — ⚠️ **not in Docker** — and `ctest`
      runs **non-root, tests ON**.
- [x] ✅ **AC3 — CLOSED by SP-123 (2026-08-29):** the app launched on a real session over RDP and
      opened a project; navigator, manuscript and timeline all nominal. ⚠️ **The app displays on a REAL X session** and is drivable both at the machine and
      remotely. ⚠️ **Xvfb is NOT acceptable for this AC.**
- [x] ✅ **AC4 — CLOSED by SP-124 (T-0477, 2026-09-07).** ✅ **ALL THREE scenarios OBSERVED on real
      hardware**: S1 clean `umount`, ⚠️ **S2 a `cifs` share killed at the source** (full streamed
      capture, two passes, 3,110 lines), ⚠️ **S3 the physical USB yank** (observed BY HAND — the probe
      was started on the wrong machine, ✅ **and the finding it produced was correct and load-bearing
      anyway**). ⚠️ **`before/during/after` for every scenario was RETIRED 2026-09-10 as a PHANTOM
      REQUIREMENT** — ✅ **it cannot be accurately measured for an instantaneous operator-driven event.**
      ✅ **THE OBVIOUS SIGNALS DID LIE, as Apple found:** ⚠️ **FIVE of them** — `mountpoint -q`, a
      directory listing with zeroed sizes, `statvfs`, a transiently successful `read`, and ⚠️ **a held FD
      that survived `umount -l` + `losetup -D`.** ✅ **`st_dev` works, with a stated limit.**
      ⚠️ **ORIGINAL TEXT: A real removable drive carrying a world copy is mounted, and its PHYSICAL loss is
      OBSERVED and RECORDED** — ⚠️ **whatever the findings turn out to be**, including *"the obvious
      signal lies,"* which is what Apple found.
      ⚠️ **WIDENED 2026-08-31 by user ruling — THREE scenarios, not one:** **S1** clean `umount`,
      ⚠️ **S2 a network share killed AT THE SOURCE**, and **S3** the physical USB yank.
      ⚠️ **S2 and S3 do NOT substitute for each other** — a clean unmount cannot strand an FD, and a
      stranded FD is the state that cost Apple six Issues. ✅ **The physical pull is JOINED, not replaced.**
- [x] ✅ **AC5 — CLOSED by SP-124 (T-0478, verified 2026-09-10).** ✅ **`offline`/`hostUnreachable`
      EMITTED FOR THE FIRST TIME IN THIS PROJECT'S HISTORY**, from a real killed `cifs` share,
      reproducibly, at BOTH endpoints, ⚠️ **on a positive `EHOSTDOWN` (112).** ⚠️ **Before this, `offline`
      was a documented lie — an enum value no code path could produce.** ⚠️ **The same live pass
      CONDEMNED the feature it proved** — ✅ **[I-0193] (102 s UI freeze → Force Quit) and [I-0194], both
      now fixed** — ⚠️ **and `missing` was ALSO wrong on an unmounted volume** ([I-0181]/T-0498).
      ⚠️ **ORIGINAL TEXT: `WorldVolumeStatus` for Linux distinguishes `unmounted` / `offline` / `missing`**,
      ⚠️ **verified against the REAL drive**, not a bind-mount.
      ⚠️ **`offline` is DEFINED by the NETWORK case** — ⚠️ **the pre-widening plan would have shipped it
      UNTESTED**, since a USB-only pass exercises `unmounted` alone.
- [x] ✅ **AC6 — CLOSED by SP-124 (T-0479, 2026-09-07).** ✅ **§9's RULE held; ⚠️ FOUR of its PREDICTIONS
      did NOT**, each recorded as prediction-vs-measurement. ⚠️ **The checklist grew from six items to
      eight.** ✅ **Had T-0478 been written from §9 as it stood, it would have shipped a stale-mount
      defence that is not needed, an `EIO`/`ESTALE` handler for errors that never arrive, no timeout at
      all, and an `offline` branch that never fires.**
- [x] ✅ **AC7 — CLOSED by SP-124 (2026-09-10).** ✅ **The rig doc's §7 is WRITTEN FROM THE RIG** — 183
      lines, eight subsections, ⚠️ **including §7.4's five lying signals and §7.7's runnable steps for all
      three scenarios, machine-tagged 🐧/🍎.** ⚠️ **THE WINDOWS RIG MUST EXECUTE IT, NOT RE-DERIVE IT.**
      ⚠️ **§7.8 records what is STILL OPEN as UNKNOWNS blocking nothing** (black-hole S2 variant, NFS,
      held FD across a yank, torn writes) — ✅ **and what is DELIBERATELY NOT OWED.**

---

## 7. ⚠️ Out of scope

| Item | Where it goes |
| ---- | ------------- |
| ⚠️ **Any Linux object/UI surface** | **EP-035 / EP-036 / EP-037** |
| ⚠️ **The Windows rig** | ⚠️ **A later Epic — executing §9.** ⚠️ **The Windows app does not exist yet** |
| ⚠️ **Retiring Docker** | ⚠️ **NOT proposed.** ✅ **The container stays valid for builds and `ctest`** — it is blind only to **hardware** failure modes |
| **CI automation of the rig** | ⚠️ **Out of scope** — reachable and documented, not automated |
| ⚠️ **Retiring the no-gesture-only rule** | ⚠️ **NOT retired** — remote input may still drop modifiers (§2) |

---

*Last Updated: 2026-08-25 (**EP-035 🟡 PROMOTED and SPLIT THREE WAYS by user ruling.** ⚠️ **The draft's
AC1 — "inherits AC1–AC9 verbatim" — was NINE ACs in one**, over a surface Apple took EP-030 + EP-031
(planned 6, delivered 11) + EP-034 (8 sprints) to build. ⚠️ **Linux lacks the PREREQUISITES**: zero
kind-card hits, no world UI, a 67-line stub inspector, two QML files. ✅ **Split: EP-035 foundations →
EP-036 Detail Sheet & media → EP-037 relationships & sources.** ⚠️ **User-ruled: a LIVE VNC pass is
required PER SURFACE SPRINT**, not deferred to Epic close. Next Sprint **SP-123**; Task **T-0474**;
Issue **I-0173**; Epic **EP-038**.)*
