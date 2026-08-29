# Active Epics

⚠️ **TWO Epics are active**, running in parallel by user ruling 2026-08-25:
**EP-035** `[Linux]` Object Foundations · **EP-038** `[Linux]` The Real Hardware Rig.
⚠️ **EP-038's first task is blocked on PHYSICAL ACCESS**; EP-035's early work is blocked on nothing.

## EP-035: `[Linux]` Object Foundations — ⚠️ **the first PORT, and the template for four more**

**Status:** 🟡 **ACTIVE** — promoted 2026-08-25 by user ruling, ⚠️ **and SPLIT into three Epics in the
same step** (see §1).
**Codebase:** `[Linux]` — ✅ **Qt WIDGETS over `ScriviBridge`** (✅ **CORRECTION CONFIRMED BY
IMPLEMENTATION 2026-08-28**: SP-125 built the first object surface as QWidgets — a `QTreeWidget` inside
the existing `QTabWidget` — and needed no QML at all. ⚠️ **Originally corrected 2026-08-25 at SP-125
planning — this Epic originally said "Qt/QML," which is MISLEADING**: `platforms/linux/qml/` holds
**exactly two files**, and the editor, navigator, timeline and `SceneInspector` are all QWidgets.
⚠️ **Building surfaces in QML would re-hit `project_ep022_editor_shell`**, where the app had to flip to
Widgets-hosts-QML on Qt 6.4.) ⚠️ **No ScriviCore change expected**; the capability already exists.
**Goal:** Give the Linux app the **object layer it has never had, from the ground up** — kind cards, an
object list, world binding, and object CRUD. ⚠️ **The floor that EP-034's AC1 wrongly assumed existed.**
**Date Created:** 2026-08-24 · **Promoted:** 2026-08-25 · **Target Close:** — (⚠️ **3–4 sprints, an
estimate made BEFORE implementation**)
**Successors:** **EP-036** (Detail Sheet & media) → **EP-037** (Relationships & sources)
**Depends on:** ✅ **SP-121** (EP-034, closed) — `ScriviBridge` at 81/100 + the Porting Outline.

---

## 1. ⚠️ Why this is THREE Epics, not one — ruled 2026-08-25

⚠️ **The draft EP-035 carried one AC reading "inherits EP-034's AC1–AC9 verbatim."** ⚠️ **That is NINE
acceptance criteria collapsed into one**, over a surface Apple took **multiple Epics** to build.

### ✅ The Apple precedent — measured, not recalled

| Apple Epic | Scope | Cost |
| ---------- | ----- | ---- |
| **EP-030** | Card / inspector framework | — |
| **EP-031** | Object model, index, relationship graph, world packages | ⚠️ **planned 6, delivered 11** |
| **EP-034** | Detail Sheets, images, relationships, sources | **8 sprints** |

⚠️ **A single Linux Epic would replicate the output of two-to-three Apple Epics and ~19 sprints.**

### ⚠️ The decisive finding — Linux lacks the PREREQUISITES, not just the surface

Checked against the code 2026-08-25:

| Check | Result |
| ----- | ------ |
| Kind-card / object-card list anywhere in `platforms/linux/` | ⚠️ **ZERO hits** |
| World-binding UI | ⚠️ **NONE** — `world` appears only in bridge/shell plumbing |
| `SceneInspector.cpp` | ⚠️ **67 lines, still the EP-024 stub** — *"No entities yet."* |
| QML files in the whole Linux app | ⚠️ **TWO** (`Landing.qml`, `NewProjectDialog.qml`) |
| Apple `Views/Detail/` alone | **2,850 lines / 8 files** — plus **~3,300** more of Inspector/card surface |

⚠️ **AC1 presumes a Kind Card list item to double-click, and Linux has no Kind Cards.** ✅ **So AC1–AC9
do not sit on a thin layer — they sit on NOTHING.** ⚠️ **A single Epic would spend its first sprints
building foundations its own ACs assume already exist** — which is *precisely* the error that split AC11
out of EP-034.

### ✅ The split

| Epic | Scope | ACs |
| ---- | ----- | --- |
| **EP-035** *(this one)* | Kind cards, object list, world binding, object CRUD | **AC1, AC4** |
| **EP-036** | Detail Sheet shell, field edit + persist, images, pending/unavailable | AC2, AC3, AC9 |
| **EP-037** | Related objects, relationship creation, navigation, sources | AC5, AC6, AC7, AC8 + **T-0472** |

⚠️ **The Porting-Outline correction and the `ctest` requirement are STANDING OBLIGATIONS carried by ALL
THREE Epics**, not deliverables of one.

---

## 2. Acceptance Criteria

- [~] **AC1** — ⚠️ **From a Kind Card list item, double-click AND a context-menu item both open the object.**
      ✅ **SCENE-SCOPED HALF DONE AND VERIFIED (SP-125, 2026-08-28)**: both affordances work in the Scene Inspector
      and were verified SEPARATELY — double-click on a WORLD-scoped kind, context menu on a
      PROJECT-scoped one, so `worldID` is proven threaded both ways. ✅ **Return/Enter opens too**, so the
      path does not depend on a pointer at all. ⚠️ **The project-wide list is still owed.**
      ⚠️ **This requires BUILDING the kind-card list first** — it does not exist on Linux.
      ⚠️ **No gesture-only affordance**: VNC carries no Shift-combos or trackpad gestures
      (`project_linux_vnc_input_constraints`), so ⚠️ **every action needs a button or menu path.**
- [~] **AC2** — ✅ **SCENE-SCOPED HALF DONE AND VERIFIED (SP-125, 2026-08-28)** — the Scene Inspector lists the
      SCENE's objects grouped by kind, in the core's own kind order. ⚠️ **The PROJECT-wide browser is
      still owed.** **An object list shows the project's objects**, grouped by kind, ⚠️ **with kind scope
      DERIVED from `scrivi_list_object_kinds`** — ⚠️ **never restated in C++ or QML.**
- [ ] **AC3** — **World binding works from the Linux app**: a world can be added, its status read, and
      ⚠️ **an unavailable world is DISABLED AND EXPLAINED**, never silently empty.
      ⚠️ **MEASURED 2026-08-29 — there is NO world UI on Linux at all.** `addWorld`, `listWorlds`,
      `getWorldStatus` and `relinkWorld` are all **bridged and unreached**; the only non-bridge caller
      is SP-125's read-only status line in `SceneInspector`. ⚠️ **Apple has a "Locate…" button
      (`WorldsView.swift:176`) and Linux has no Worlds view whatsoever.**
      ⚠️ **CONSEQUENCE, confirmed by the rig copy:** a project whose world moved opens with the pending
      state working **correctly** — world named, links held, nothing lost — ⚠️ **and NO WAY TO FIX IT
      from the app.** ✅ **That is `capability_without_surface` in its exact form**, and it is what AC3
      exists to close. ⚠️ **RELINK is the first thing AC3's sprint should build**, not an afterthought:
      it is the only affordance that turns an honest error message into a recoverable one.
- [ ] **AC4** — **Object CRUD round-trips**: create, open, save, delete, promote — ⚠️ **with `worldID`
      threaded through every call.** ⚠️ **Omitting it is how SP-104 blocked object creation outright.**
- [ ] **AC5** — ⚠️ **Card list items show a thumbnail when an image exists**, unchanged when none does.
- [ ] **AC10** — ⚠️ **ADDED 2026-08-29 (user-found gap, see §3a).** **The Scene Inspector is a THREE-TAB
      panel — Writing · Properties · Worldbuilding — matching Apple's `InspectorTab`.**
      ⚠️ **"Scene Entities" is NOT a tab**; it is the placeholder Apple's SP-090 deleted, and Linux
      inherited it by mistake. ⚠️ **DISPLAY order is `Writing | Worldbuilding | Properties`**
      (`InspectorTab` declaration order), defaulting to **Writing** — ⚠️ **distinct from the PROVING
      order (Writing → Properties → Worldbuilding)**, which is about what gets built first so the shell
      is proven on the cheap surfaces before the expensive ones sit on it.
      ⚠️ **`inspector-layout.json` (`scrivi.inspector-layout.v1`) is APP-SIDE with NO endpoint, and
      Apple already populates it.** ⚠️ **Linux must PATCH it, never reconstruct it** — it does not model
      `stackSort` or per-scene stacks, and dropping them would silently delete the writer's Apple-side
      layout. ⚠️ **If a THIRD platform needs this schema, move it into ScriviCore.**
- [~] **AC6** *(standing)* — ✅ **HONOURED for SP-125 (verified 2026-08-28)**: three rules ADDED to the Outline from what
      implementing it actually cost — **§4.4a** (the binding usually cannot tell absence from failure, so
      §4.4 is unimplementable until it can), **§4.4b** (there is a FOURTH state: primary read succeeded,
      confirming read failed), and **§4.6a** (a bad path does NOT prove failure detection — several
      stores treat "missing" as "empty" and succeed). ⚠️ **The Porting Outline is CORRECTED from this
      Epic's experience**, not merely followed. ✅ **It is the deliverable that survives; this Epic is its
      first proving run.**
- [~] **AC7** *(standing)* — ✅ **HONOURED for SP-125 (verified 2026-08-28)**, three of the five so far: ✅ **kind scope
      DERIVED** (grep-proven: no `ObjectKind` name in `platforms/linux/`); ✅ **edge labels READ, never
      recomputed** — the live pass rendered the `cites` type's INVERSE label because the scene is the
      `to` endpoint, which is the core's projection arriving intact; ✅ **absence is never deletion** —
      and ⚠️ **honouring it required finding §4.4a first.** ⚠️ **"Patch, never reconstruct" is not yet
      exercised** (nothing writes an object on Linux until AC4). ⚠️ **The platform-independent rules are
      RE-HONOURED, not re-decided**: derive kind scope from `list_object_kinds`; read edge labels, never
      recompute; patch objects, never reconstruct; absence is never deletion; disabled **and** explained.
- [~] **AC8** *(standing)* — ✅ **HONOURED for SP-125 (verified 2026-08-28)**: **571/571 non-root (`builder`), tests ON** —
      ⚠️ **which required fixing I-0171 first; it recurred and blocked the run outright.** `ctest`
      **actually runs** in the container, ⚠️ **NON-ROOT, tests ON**
      (`project_linux_container_tests_off`). ⚠️ **"The image built" has never meant the suite ran.**
- [~] **AC9** — ✅ **HONOURED for SP-125 (verified 2026-08-28)**, and it PAID: the live pass found **I-0173**, a defect all
      571 ctests and 23 smoke checks were green against, because it is a defect in what a writer can
      READ rather than in what the code computes. ⚠️ **EVERY surface-shipping sprint ends with a LIVE VNC
      CLICK-THROUGH** (user ruling 2026-08-25). See §3.

---

## 3. ⚠️ USER RULING (2026-08-25) — the live pass is PER SPRINT, not per Epic

> **Every surface-shipping sprint ends with a human click-through over VNC — the same standard EP-034
> held.**

⚠️ **This is the ruling that costs the most and matters the most.** **22 consecutive Issues across
SP-118–SP-120 came from a writer clicking, and NONE from any suite.**

⚠️ **A Linux pass is materially harder than the macOS one** — it needs Docker + VNC + a real project, and
⚠️ **the VNC input path cannot carry Shift-combos or trackpad gestures.** ✅ **The ruling holds anyway**,
because deferring the pass to Epic close would surface defects late, in bulk, against cold code.

---

## 3a. ⚠️ GAP FOUND BY THE USER (2026-08-29) — the TABBED PANEL was never planned

⚠️ **This Epic has no AC, and no sprint, for the Scene Inspector's TAB SHELL.** ✅ **Raised by the user
after SP-125's click-through**, and it is a real omission rather than a deferral.

### What Apple actually did

| Apple sprint | What it built |
| ------------ | ------------- |
| **SP-090** (T-0361) | ⚠️ **The SHELL FIRST** — bottom tabs **Writing \| Worldbuilding \| Properties**, ⚠️ **explicitly replacing "the SP-090 placeholder (a single segmented tab over a stub *Scene Entities* body)"** |
| later sprints | The cards inside those tabs — ⚠️ **Writing and Properties before Worldbuilding**, per the user |

⚠️ **`InspectorTab` (`InspectorCard.swift:19`) declares all three.** ⚠️ **"Scene Entities" is not a tab in
the finished Apple app at all** — it is the name of the PLACEHOLDER that SP-090 deleted.

### ⚠️ What Linux did instead, and why

⚠️ **SP-125 wired the PLACEHOLDER, not the shell.** The Linux stub inherited exactly one tab — *"Scene
Entities"* — from EP-024/SP-078, which had copied ⚠️ **Apple's SP-090 placeholder rather than its
successor.** SP-125's user ruling was *"wire the existing `SceneInspector` tab to real data"*, a
deliberately narrow slice — ✅ **and that ruling was honoured** — but ⚠️ **nobody noticed the tab being
wired was a stub Apple had already thrown away.**

⚠️ **The consequence is an INVERTED PROVING ORDER.** Apple proved the shell on **Writing** and
**Properties** — the cheap surfaces — before the expensive **Worldbuilding** cards landed on it.
⚠️ **Linux built Worldbuilding content FIRST**, because that is what this Epic's ACs named.

### ✅ Why the correction is cheap

⚠️ **Nothing needs rewriting** — SP-125's object list is Worldbuilding-tab content and simply moves into
the tab it belongs to. ✅ **Every endpoint the other two tabs need is ALREADY BRIDGED**:
`getSceneNotes` (`ScriviBridge.hpp:429`), `setSceneOutline`/`setSceneTags`/`setSceneTodo` (453–455).
✅ **Properties is read-only DERIVED data from the same `getSceneNotes` call** — the cheapest of the three.

⚠️ **Recorded so the next port does not repeat it:** ⚠️ **when mirroring an Apple surface, check whether
the thing being mirrored is the FINISHED surface or a placeholder that Apple later deleted.**

---

## 4. ⚠️ Known traps — each has already been paid for once

- ⚠️ **A Qt or QML restatement of the kind-scope partition would be OCCURRENCE NINE.** ⚠️ **Occurrence
  five was in SWIFT** — a new platform layer is exactly where it recurs. **Qt is no more immune than
  SwiftUI was.**
- ⚠️ **`list_edges_for` returns the label ALREADY RESOLVED** for the queried endpoint. ⚠️ **Recomputing
  direction in Qt is the same defect class**, and it lands in **EP-037**.
- ⚠️ **`capability_without_surface` in reverse:** ⚠️ **SP-121's 47 bridged endpoints have NO reader.**
  ⚠️ **If these three Epics do not run, that is what they stay.**
- ⚠️ **Patch objects, never reconstruct** (T-0436/T-0437) — a surface built before its typed model
  reconstructs objects and drops fields.

---

## 5. ⚠️ Carried IN from EP-034's close

| Item | Lands in |
| ---- | -------- |
| ⚠️ **SP-121's 47 bridged endpoints — NO live click-through** | ⚠️ **All three Epics** — this is where a human first uses them |
| **T-0472** — surface for custom relationship types | ⚠️ **EP-037**, with the relationship work |
| ⚠️ **A C ABI test for `upsert_relation_type`** | **EP-037** — ⚠️ **it has none** (`feedback_boundary_tests_not_facade`) |

⚠️ **T-0473 (`[Apple]` timeline parity) is NOT part of any of these Epics** — it carries no Epic by ruling.

---

## 6. Explicitly OUT of scope

| Item | Where it goes |
| ---- | ------------- |
| **Detail Sheet, field editing, images** | **EP-036** |
| **Relationships, navigation, sources** | **EP-037** |
| **history (15) + buffers (4) endpoints** | **EP-019** — ⚠️ its Linux story is still unruled |
| **`[Apple]` timeline parity (T-0473)** | ⚠️ **No Epic — a tracked debt** |
| **iPad / iPhone / Windows / visionOS** | ⚠️ **Future ports — but the OUTLINE is written for them now** |

---


## 8. Sprints

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **SP-125** | ⚠️ **Scene Inspector wired to real objects** — replaces the 67-line EP-024 stub. ⚠️ **Scene-scoped only** | ✅ **ALL FIVE TASKS VERIFIED 2026-08-28** — ⚠️ **awaiting user approval to CLOSE** |
| **SP-126** | ⚠️ **The THREE-TAB shell** (AC10) — ⚠️ **display order Writing \| Worldbuilding \| Properties**; Writing + Properties populated, ⚠️ **today's object list MOVED into Worldbuilding**; ⚠️ **`inspector-layout.json` read+write, PATCHED not reconstructed** | 🟡 **ACTIVE** — activated 2026-08-29 (T-0485–T-0490) |
| — | ⚠️ **Project-wide object browser** (AC2's other half) | ⚪ **Not planned** |
| — | **Object CRUD** (AC4) + **world binding UI** (AC3) | ⚪ **Not planned** |
| — | **Card thumbnails** (AC5) — ⚠️ **needs EP-036's image work** | ⚪ **Not planned** |

⚠️ **3–4 sprints is an estimate made BEFORE implementation.** ⚠️ **EP-031 planned 6 and delivered 11.**

---
---

## EP-038: `[Linux]` ⚠️ **The Real Hardware Rig** — native Ubuntu + drive-loss ground truth

**Status:** 🟡 **ACTIVE** — promoted 2026-08-25 by user ruling.
**Codebase:** `[Linux]` + ⚠️ **environment/infrastructure**, plus one Qt/C++ deliverable.
**Goal:** Stand up a **real Ubuntu machine** as a first-class test rig, then ⚠️ **establish what ACTUALLY
happens when a drive carrying a world is physically unmounted** — and implement the platform refinement
against that ground truth rather than against documentation.
**Date Created:** 2026-08-24 · **Promoted:** 2026-08-25 · **Sprints:** **SP-123**, **SP-124**
**Tasks:** **T-0474 – T-0479** (six)
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
| **SP-123** | ⚠️ **Rig reachable + building natively** — T-0474 – T-0476 | 🔵 **Planned** |
| **SP-124** | ⚠️ **Ground truth + refinement** — T-0477 – T-0479 | ⚪ **Not planned** — ⚠️ **gated on SP-123** |

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

- [ ] **AC1** — ⚠️ **The Ubuntu box is reachable over SSH** with key auth and a documented path.
      ⚠️ **NO passwords, keys or hostnames in any tracked file.**
- [ ] **AC2** — ⚠️ **The Qt app BUILDS AND RUNS NATIVELY on Ubuntu** — ⚠️ **not in Docker** — and `ctest`
      runs **non-root, tests ON**.
- [ ] **AC3** — ⚠️ **The app displays on a REAL X session** and is drivable both at the machine and
      remotely. ⚠️ **Xvfb is NOT acceptable for this AC.**
- [ ] **AC4** — ⚠️ **A real removable drive carrying a world copy is mounted, and its PHYSICAL loss is
      OBSERVED and RECORDED** — ⚠️ **whatever the findings turn out to be**, including *"the obvious
      signal lies,"* which is what Apple found.
- [ ] **AC5** — ⚠️ **`WorldVolumeStatus` for Linux distinguishes `unmounted` / `offline` / `missing`**,
      ⚠️ **verified against the REAL drive**, not a bind-mount.
- [ ] **AC6** — ⚠️ **The Porting Outline's §9 is CORRECTED from experience**, not merely confirmed.
      ⚠️ **A §9 that survives contact unchanged is evidence it was not tested.**
- [ ] **AC7** — ⚠️ **The Windows rig procedure is DERIVABLE from §9** — the next rig Epic ⚠️ **EXECUTES a
      recipe, it does not re-derive one.**

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
