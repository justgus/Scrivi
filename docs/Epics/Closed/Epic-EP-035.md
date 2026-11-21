# Epic EP-035 — ✅ CLOSED (superseded) 2026-11-20

**Closed:** ✅ **2026-11-20, SUPERSEDED by [EP-059]** (user, 2026-11-20: *"yes, file G1–G9 as an Epic (include everything here, so it is all tracked at once."*; asked how it relates to EP-035/036/037: *"One Epic for all"*).
Never activated. Every AC — verified and open — is carried into [EP-059] by reference; this file is the record of
what each AC meant and what it cost. ⚠️ The 2026-08-25 ruling that this work be THREE Epics (EP-035 §1) is superseded by
the same decision; EP-059 keeps the three as its parts A–C, in the same order.

---

## EP-035: `[Linux]` Object Foundations — ⚠️ **the first PORT, and the template for four more**

**Status:** 🔵 **BACKLOG.** ⚠️ **DEFERRED 2026-09-10 by user ruling, preempted by [EP-039]** (✅ *"the
most important thing right now"*) — ⛔ **BUT THAT DEFERRAL REASON HAS EXPIRED: [EP-039] CLOSED
2026-09-15.** ✅ **AC5's stated blocker (EP-039's AC7 blob-index rule) IS GONE.**
⚠️ **NOT ACTIVATED 2026-09-25 for a DIFFERENT reason — a file collision with [EP-043]:** ⛔ **its open
**AC4** (object CRUD) attaches to `EditorShell::onOpenObjectRequested` (`EditorShell.cpp:2214`), and
[EP-043]'s [SP-145] is extracting per-project state out of that same 2,783-line file.** ✅ **Running
AC4 first would have [SP-145] relocate it immediately.** ✅ **AC5 (thumbnails, `SceneInspector.cpp`)
does NOT collide and could run alongside [EP-043].**
⚠️ **ALSO OUTSTANDING: T-0497 (world CREATION)** — ✅ **the SP-127 pass RAISED its priority: a
Linux-only writer whose project has no world has nothing to add or relink.**
⚠️ **Original status line follows.** ⚠️ **2 of 4 ACs remain open (AC4 object CRUD, AC5 card thumbnails); ✅ 2 are verified and are NOT lost.** ⚠️ **AC5 (thumbnails) is DEPENDENT ON EP-039's AC7** — ✅ **the blob-index rule that the index holds LOCATION AND SHAPE, never the bytes** — ⚠️ **so building AC5 first would design the memory behaviour twice, and the second design would have to undo the first.** ⚠️ **Original status line follows.** — 🟡 **ACTIVE** — promoted 2026-08-25 by user ruling, ⚠️ **and SPLIT into three Epics in the
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
- [x] ✅ **AC3 — CLOSED by SP-127 (2026-09-02).** **World binding works from the Linux app**: a world
      can be added, its status read, and ⚠️ **an unavailable world is DISABLED AND EXPLAINED**, never
      silently empty. ✅ **`Project ▸ Manage Worlds…` ships**, with **Locate…** offered ⚠️ **only for a
      non-available world**, add-existing, and remove-reference.
      ✅ **`capability_without_surface` is CLOSED for worlds** — `addWorld`, `relinkWorld`,
      `getWorldStatus` and `getWorldBinding` had been bridged with **ZERO callers**; ⚠️ **a project
      whose world moved could not be repaired from the app at all.** ✅ **It can now**, verified by the
      user on the real rig: wrong world rejected, correct world accepted, pending notices cleared.
      ⚠️ **THE PASS THAT CLOSED THIS AC FOUND FOUR DEFECTS, ONE OF THEM 🔴 DATA LOSS** (I-0183 —
      10 of 12 relationships destroyed in a real project by an available-but-UNREADABLE world).
      ⚠️ **All four are Verified** → [`Issue-verified-0181-0190.md`](../../Issues/Verified/Issue-verified-0181-0190.md).
      ⚠️ **World CREATION is NOT part of this AC and remains open as T-0497** — ⚠️ **and the pass
      RAISED its priority: a Linux-only writer whose project has no world has nothing to add or
      relink, and the AC's own test could not be completed without the Apple app.**
- [ ] **AC4** — **Object CRUD round-trips**: create, open, save, delete, promote — ⚠️ **with `worldID`
      threaded through every call.** ⚠️ **Omitting it is how SP-104 blocked object creation outright.**
- [ ] **AC5** — ⚠️ **Card list items show a thumbnail when an image exists**, unchanged when none does.
- [x] ✅ **AC10 — CLOSED by SP-126 (2026-08-30).** The three-tab shell ships, ⚠️ **"Scene Entities" is
      retired**, and ⚠️ **`inspector-layout.json` round-trips Apple's keys intact (17/17).**
      ⚠️ **ADDED 2026-08-29 (user-found gap, see §3a).** **The Scene Inspector is a THREE-TAB
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
| **history (15) + buffers (4) endpoints** | **[EP-026]** `[Linux]` (rescoped 2026-10-11) |
| **`[Apple]` timeline parity (T-0473)** | ⚠️ **No Epic — a tracked debt** |
| **iPad / iPhone / Windows / visionOS** | ⚠️ **Future ports — but the OUTLINE is written for them now** |

---


## 8. Sprints

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **SP-125** | ⚠️ **Scene Inspector wired to real objects** — replaces the 67-line EP-024 stub. ⚠️ **Scene-scoped only** | ✅ **ALL FIVE TASKS VERIFIED 2026-08-28** — ⚠️ **awaiting user approval to CLOSE** |
| **SP-126** | ⚠️ **The THREE-TAB shell** (AC10) — ⚠️ **display order Writing \| Worldbuilding \| Properties**; Writing + Properties populated, ⚠️ **today's object list MOVED into Worldbuilding**; ⚠️ **`inspector-layout.json` read+write, PATCHED not reconstructed** | ✅ **CLOSED 2026-08-30** → [`Closed/Sprint-SP-126.md`](../../Sprints/Closed/Sprint-SP-126.md) |
| — | ⚠️ **Project-wide object browser** (AC2's other half) | ⚪ **Not planned** |
| — | **Object CRUD** (AC4) + **world binding UI** (AC3) | ⚪ **Not planned** |
| — | **Card thumbnails** (AC5) — ⚠️ **needs EP-036's image work** | ⚪ **Not planned** |

⚠️ **3–4 sprints is an estimate made BEFORE implementation.** ⚠️ **EP-031 planned 6 and delivered 11.**

---
---

---
