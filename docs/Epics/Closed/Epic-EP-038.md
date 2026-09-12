# EP-038: `[Linux]` ⚠️ **The Real Hardware Rig** — native Ubuntu + drive-loss ground truth

**Status:** ✅ **CLOSED 2026-09-11 — user-approved.** ✅ **All SEVEN ACs closed; all THREE Sprints closed (SP-123, SP-124, SP-128).**
**Codebase:** `[Linux]` + ⚠️ **environment/infrastructure**, plus one Qt/C++ deliverable.
**Goal:** Stand up a **real Ubuntu machine** as a first-class test rig, then ⚠️ **establish what ACTUALLY
happens when a drive carrying a world is physically unmounted** — and implement the platform refinement
against that ground truth rather than against documentation.
**Date Created:** 2026-08-24 · **Promoted:** 2026-08-25 · **Sprints:** ✅ **SP-123 (CLOSED 2026-08-29)**, ✅ **SP-124 (COMPLETE 2026-09-10, awaiting close)**, ✅ **SP-128 (CLOSED 2026-09-11)**
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
| ✅ **SP-128** | ⚠️ **Honest waiting** — T-0499 – T-0501 — ⚠️ **[I-0195] progress UI + [I-0182] rig verification** | ✅ **CLOSED 2026-09-11** — ⚠️ **added by USER RULING: EP-038's ACs are all closed, but the rig's own findings are not** |

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


---

## ✅ Closing summary — EP-038 (written at close, 2026-09-11)

**Goal:** stand up a real Ubuntu machine as a first-class rig, ⚠️ **establish what ACTUALLY happens when a
drive carrying a world is lost**, and implement the platform refinement against that ground truth rather
than against documentation. ✅ **Achieved.**

### ⚠️ The headline finding: THE OBVIOUS SIGNALS LIE — five of them

⚠️ **`mountpoint -q`, a directory listing (zeroed sizes), `statvfs`, a transiently successful `read`, and
a held FD that survived `umount -l` + `losetup -D`.** ✅ **This is the Linux counterpart to Apple's
`volumeIsRemovable == false` on a hand-unplugged drive** — ⚠️ **and finding it is the entire reason a real
rig exists.** ✅ **`st_dev` works (with a stated limit); `EHOSTDOWN` (112) was the strongest UNUSED signal.**

### ✅ What shipped

✅ **`WorldVolumeStatus` for Linux**, written against measurement. ⚠️ **`offline`/`hostUnreachable` emitted
for the FIRST TIME IN THIS PROJECT'S HISTORY** — ⚠️ **it had been a documented lie, an enum value no code
path could produce.** ✅ **Project open moved OFF the UI thread with a DETERMINATE progress bar.**
✅ **The rig doc's §7 written FROM THE RIG** (183 lines) — ⚠️ **the Windows rig must EXECUTE it, not
re-derive it.**

### ⚠️ **THE LESSON THIS EPIC KEEPS TEACHING: live passes found what suites structurally could not**

⚠️ **EVERY defect of consequence here was found by a HUMAN USING THE APP, and the suite was GREEN through
all of them.**

| Defect | ⚠️ How it was found |
| ------ | ------------------ |
| **[I-0193]** ⚠️ 102 s freeze → Force Quit | ⚠️ **A scene click on a dead share.** ⚠️ **Docker has no dead network share** |
| **[I-0194]** six-`../` writer-facing path | ⚠️ **Only the offline resolution path produces it** |
| **[I-0181]** false `missing` on an unmounted volume | ⚠️ **Needed a real mount** |
| **[I-0198]** ⚠️ `UniqueConnection` silently rejects lambdas | ⚠️ **23/23 smokes GREEN; the load succeeded and nothing was listening** |
| **[I-0199]** ⚠️ progress bar rendered on a HIDDEN page | ⚠️ **23/23 smokes GREEN; it could never be seen at ANY size** |

⚠️ **The last two were defects in THIS EPIC'S OWN FINAL SPRINT**, ✅ **found by the very pass meant to
verify it.**

### ⚠️ Recorded honestly rather than ticked clean

⚠️ **S3 was observed BY HAND with no probe running** (the probe was started on the wrong machine) — ✅ **and
its finding was correct and load-bearing anyway.** ⚠️ **`before/during/after` capture was RETIRED as a
phantom requirement**: ✅ **it cannot be measured for an instantaneous operator-driven event.**
⚠️ **WOL was settled as NOT PURSUED** — ⚠️ **the operator powers the rig off at night, so `ssh` timing out
is the EXPECTED state, not a fault to diagnose.**

### ⚠️ What this Epic did NOT fix, and who owns it

⚠️ **[I-0196]: a 1,153-scene project freezes `321 s` on open.** ⚠️ **NOT this Epic's** — ✅ **it belongs to
[EP-039], which is ACTIVE.** ⚠️ **Scene count is not the only driver: a 71-scene project with 50 edges and
2 bound worlds on a share still takes ~5 s**, ⚠️ **because the core is STATELESS and every per-item
question costs a full traversal.**

⚠️ **[I-0195] is fixed on LINUX ONLY.** ⚠️ **Apple has NO asynchrony at all — 93 engine call sites, ZERO
off the main thread** — ✅ **carried by [EP-039] AC6.**
