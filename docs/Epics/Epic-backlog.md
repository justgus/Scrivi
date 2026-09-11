# Epic Backlog

Epics listed here are fully defined but have no active sprint. Epics that were partially delivered and returned here retain their verified AC state. Work resumes when a sprint is assigned.

---

## Backlog Epics

_(EP-019 `[Apple]` Undo/Redo was un-deferred back to Active 2026-07-24 — now in `Epic-active.md`.)_

## [Linux] App — Epic Family (EP-024–EP-026)

The Ubuntu Qt/QML port of the macOS app, split one Epic per capability (mirroring how the Apple app was
built Epic-by-Epic). **EP-020 (Foundation), EP-021 (Project Lifecycle & Landing), EP-022 (Writing Surface &
Scene Navigator), EP-023 (Manuscript Structure Editing), EP-024 (Scene Inspector Panel) and EP-025 (Timeline
Panel) are all ✅ closed** (→ `Epics/Closed/`) — **EP-025 closed 2026-07-24**, the last of them.

**EP-026 below is therefore the only `[Linux]` Epic left, and it is still 🔵 Draft** — it was never promoted
when EP-025 landed, because `[Apple]`/`[Cross]` work (EP-027–EP-031) took priority. Promotion is a scheduling
decision that has not yet been made.

Each is verified in Docker+VNC (developer) then on real Ubuntu (alpha tester). Each targets near-parity with
the named Apple Epic. Any new `scrivi_*` endpoint a screen needs is a Task with a `[ScriviCore]` note, not a
separate Epic.

### EP-026: [Linux] Undo/Redo, Menus, Settings & Parity Verification

**Status:** 🔵 Draft
**Goal:** Wire the EP-019 undo/redo history C ABI into the Linux UI (capture + apply + fork popover +
stale-branch purge), the app menu bar, project settings, and run the near-parity verification pass;
prepare the Linux app for the alpha tester on real Ubuntu. Mirrors Apple EP-012 + EP-019.
**Rough scope:** Linux `HistoryCapture` equivalent over `scrivi_history_*`, undo/redo apply, fork
popover, settings sheet (capacity + stale-branch purge), menu bar, full parity checklist, hand-off to
tester. **Depends on:** EP-022 (and benefits from EP-023–EP-025).

---

## EP-035: `[Linux]` Object Foundations — ⚠️ **the first PORT, and the template for four more**

**Status:** 🔵 **DEFERRED TO BACKLOG 2026-09-10 by user ruling** — ⚠️ **preempted by [EP-039]**, ✅ **which the user ruled "the most important thing right now."** ⚠️ **2 of 4 ACs remain open (AC4 object CRUD, AC5 card thumbnails); ✅ 2 are verified and are NOT lost.** ⚠️ **AC5 (thumbnails) is DEPENDENT ON EP-039's AC7** — ✅ **the blob-index rule that the index holds LOCATION AND SHAPE, never the bytes** — ⚠️ **so building AC5 first would design the memory behaviour twice, and the second design would have to undo the first.** ⚠️ **Original status line follows.** — 🟡 **ACTIVE** — promoted 2026-08-25 by user ruling, ⚠️ **and SPLIT into three Epics in the
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
      ⚠️ **All four are Verified** → [`Issue-verified-0181-0190.md`](../Issues/Verified/Issue-verified-0181-0190.md).
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
| **history (15) + buffers (4) endpoints** | **EP-019** — ⚠️ its Linux story is still unruled |
| **`[Apple]` timeline parity (T-0473)** | ⚠️ **No Epic — a tracked debt** |
| **iPad / iPhone / Windows / visionOS** | ⚠️ **Future ports — but the OUTLINE is written for them now** |

---


## 8. Sprints

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **SP-125** | ⚠️ **Scene Inspector wired to real objects** — replaces the 67-line EP-024 stub. ⚠️ **Scene-scoped only** | ✅ **ALL FIVE TASKS VERIFIED 2026-08-28** — ⚠️ **awaiting user approval to CLOSE** |
| **SP-126** | ⚠️ **The THREE-TAB shell** (AC10) — ⚠️ **display order Writing \| Worldbuilding \| Properties**; Writing + Properties populated, ⚠️ **today's object list MOVED into Worldbuilding**; ⚠️ **`inspector-layout.json` read+write, PATCHED not reconstructed** | ✅ **CLOSED 2026-08-30** → [`Closed/Sprint-SP-126.md`](../Sprints/Closed/Sprint-SP-126.md) |
| — | ⚠️ **Project-wide object browser** (AC2's other half) | ⚪ **Not planned** |
| — | **Object CRUD** (AC4) + **world binding UI** (AC3) | ⚪ **Not planned** |
| — | **Card thumbnails** (AC5) — ⚠️ **needs EP-036's image work** | ⚪ **Not planned** |

⚠️ **3–4 sprints is an estimate made BEFORE implementation.** ⚠️ **EP-031 planned 6 and delivered 11.**

---
---

---

## EP-036: `[Linux]` Object Detail Sheet & Media — ⚠️ **the second of three port Epics**

**Status:** 🔵 **Proposed** — created 2026-08-25 by the EP-035 three-way split.
**Codebase:** `[Linux]` — Qt/QML over `ScriviBridge`. ⚠️ **No ScriviCore change expected.**
**Goal:** A writer can **open an object and see and edit what it actually is** on Linux — fields, notes,
tags, and imagery — and ⚠️ **never lose work when a world goes away.**
**Depends on:** ⚠️ **EP-035** — kind cards and the object list are the entry point this Epic opens FROM.
**Date Created:** 2026-08-25 · **Target Close:** — (⚠️ **3–4 sprints, estimated before implementation**)

⚠️ **Apple equivalent: ~1,350 lines** (`ObjectDetailSheet.swift` 816, `ObjectImageSection.swift` 368,
`ExistingAssetPicker.swift` 168) — ⚠️ **and that is the SwiftUI line count, not the Qt one.**

### Acceptance Criteria (draft — ⚠️ to be ruled at promotion)

- **AC1** — The sheet **views and edits** `displayName`, `subtitle`, `notes`, `tags`; ⚠️ **edits persist
  across app restart.** ⚠️ **PATCH the object, never reconstruct it** (T-0436/T-0437) — a surface built
  before its typed model drops fields.
- **AC2** — An image can be **imported, displayed, replaced, removed**; ⚠️ for a **world-scoped** object
  it is stored **in the world package** and is present when that world opens **from a different project.**
- **AC3** — ⚠️ **A pending object opens READ-ONLY, explained, never written.**
- **AC4** — ⚠️ **A world going unavailable WHILE A SHEET IS OPEN loses no data.**
  ⚠️ **BLOCKED ON EP-038** — this AC cannot be written honestly until the real-hardware rig reports what
  a Linux drive-loss actually does. ⚠️ **Do NOT specify it from the Apple implementation**; that code's
  own header records that the documented API lied. ⚠️ **On Apple this took
  SIX fixes** (I-0162, I-0165, I-0165b, I-0166, I-0167, I-0168): the sheet must survive the outage,
  explain it, warn that unsaved edits are memory-only, and ⚠️ **not discard them on reload, close, or
  navigation.** ⚠️ **Expect the same on Linux — this is the highest-risk AC in the port.**
- **AC5** *(standing)* — ⚠️ **Porting Outline corrected from experience**, not merely followed.
- **AC6** *(standing)* — ⚠️ **Platform-independent rules re-honoured, not re-decided.**
- **AC7** *(standing)* — `ctest` **actually runs** in the container, ⚠️ **NON-ROOT, tests ON.**
- **AC8** *(standing)* — ⚠️ **A LIVE VNC click-through per surface-shipping sprint** (user ruling
  2026-08-25).

### ⚠️ Known traps

- ⚠️ **AC4 is where Apple bled.** Six separate data-loss routes into one surface, ⚠️ **all found by
  ejecting a drive or navigating away — none by a suite.**
- ⚠️ **`volumeIsRemovable`/`volumeIsEjectable` are UNRELIABLE** on the real rig; availability is decided
  by **volume-root mount presence** (`project_test_rig_tintagael_eskandar`).

---

## EP-037: `[Linux]` Relationships & Sources — ⚠️ **the third of three port Epics**

**Status:** 🔵 **Proposed** — created 2026-08-25 by the EP-035 three-way split.
**Codebase:** `[Linux]` — Qt/QML over `ScriviBridge`. ⚠️ **One ScriviCore-adjacent item: a C ABI test.**
**Goal:** The **relationship graph and citation apparatus** on Linux — related objects, relationship
creation, push-navigation, and sources with footnote text.
**Depends on:** ⚠️ **EP-036** — relationships and sources are sections OF the Detail Sheet.
**Date Created:** 2026-08-25 · **Target Close:** — (⚠️ **3–4 sprints, estimated before implementation**)

⚠️ **Apple equivalent: ~1,500 lines** (`ObjectSourcesSection` 604, `ObjectRelationsSection` 438,
`ObjectRelationPicker` 259, `ObjectDetailHistory` 100, `Citation` 97).

### Acceptance Criteria (draft — ⚠️ to be ruled at promotion)

- **AC1** — The related-objects section lists this object's edges ⚠️ **with labels reading correctly FROM
  THIS ENDPOINT.** ⚠️ **`list_edges_for` returns the label ALREADY RESOLVED — pass it through UNTOUCHED.**
  ⚠️ **Recomputing direction in Qt is the same defect class as restating kind scope**, which this project
  has paid for eight times.
- **AC2** — A relationship can be **created from the sheet**; ⚠️ **it appears from BOTH endpoints** and is
  **rejected as a duplicate from the second.**
- **AC3** — **Push-navigation to a related object, with back and forward.** ⚠️ **On Apple this needed FOUR
  Issues** (I-0151, I-0153, I-0157, I-0161) before it worked. ⚠️ **Verify both halves SEPARATELY** —
  back/forward and push-from-the-related-list are different mechanisms
  (`feedback_verify_each_half_separately`).
- **AC4** — **Sources can be created from the object they document**, with citation detail and footnote
  text. ⚠️ **Creation lives on the OBJECT's surface, never on the scene-scoped `sources` card** — user
  ruling R6: *"it would muddy the fact that the source must be associated with an object in the world."*
- **AC5** — ⚠️ **T-0472: a surface for CUSTOM RELATION TYPES**, calling `scrivi_upsert_relation_type`.
  ⚠️ **That endpoint is fully implemented and has NEVER had a caller on any platform** — projects are
  seeded with defaults and `list_relation_types` reads them, but ⚠️ **nothing can ADD one.**
  ✅ **User-ruled a REAL FEATURE owed a UI** (2026-08-25).
- **AC6** — ⚠️ **A C ABI test for `scrivi_upsert_relation_type`**, which ⚠️ **has NONE.**
  ⚠️ **Through the boundary, not the facade** — `feedback_boundary_tests_not_facade`; a facade test
  cannot see a boundary gap, and that is how I-0113 shipped green.
- **AC7** *(standing)* — ⚠️ **Porting Outline corrected from experience.**
- **AC8** *(standing)* — ⚠️ **Platform-independent rules re-honoured, not re-decided.**
- **AC9** *(standing)* — `ctest` **NON-ROOT, tests ON**, actually run.
- **AC10** *(standing)* — ⚠️ **A LIVE VNC click-through per surface-shipping sprint.**

### ⚠️ At this Epic's close, the port is DONE — and so is the debt

✅ **When EP-037 closes, SP-121's 47 bridged endpoints finally have readers** and the
`capability_without_surface` risk EP-034 named at its own close is discharged. ⚠️ **Until then it stands.**

---

## EP-032: `[Cross]` Inline Object References in the Manuscript

**Status:** 🔵 **Draft** — ⚠️ **RETURNED to this backlog 2026-08-20 by user ruling**, the same day it was
promoted. It is **not active** and has **no active sprint**.

> ⚠️ **This entry is NOT the rough sketch a backlog entry usually is.** It retains the **full planning
> pass** done at promotion — **AC1–AC10**, the **Q1–Q6** design questions, the code-verified claim table,
> the export scope gap, and the closeout procedure. Per this file's own rule — *"Epics that were partially
> delivered and returned here retain their verified AC state"* — **none of that is discarded on return.**
> When EP-032 is promoted again, this is the starting point; the planning does not need redoing.
>
> ⚠️ **Its sprint IDs SP-107–SP-114 remain RESERVED** — see the reservation note in the Sprints section
> below. **They are not released by this return.**
**Codebase:** `[Cross]` — ScriviCore (scene body syntax, fragment model, one seeded relation type) +
every UI that renders or edits manuscript text (Apple, Linux) + export.
**Goal:** A writer can place a **reference to a worldbuilding object inside scene text**, see it rendered
there, and have it survive editing, cut/copy/paste, undo, the object being renamed or deleted, and the
object's world going unavailable — with **footnote** and **pull-quote** presentations for `source`.
**Date Created:** 2026-08-12 (opened 🔵 Proposed) · **Promoted to Draft:** 2026-08-20 · ⚠️ **Returned to backlog:** 2026-08-20
**Target Close Date:** — (none; not scheduled)
**Actual Close Date:** —

**Origin:** Doc 1 §3.4.1 (`Scrivi_Worldbuilding_Object_Model_v0_2.md`) + Doc 2 §3.1.1
(`Scrivi_Scene_Inspector_Card_Framework_v0_1.md`). Deferred out of EP-031 by user ruling: sources attach
to objects now; source-in-manuscript needs a capability Scrivi does not have.

**Depends on:** EP-031 ✅ closed 2026-08-19 (object model + graph + world partition) · EP-029 ✅ closed
2026-08-03 (the fragment model this must extend).

---

### ⚠️ What makes this an Epic — verified against the code, 2026-08-20

Each claim below was checked rather than inherited from the backlog entry:

| Claim | Evidence |
| ----- | -------- |
| Scene bodies are plain Markdown with no reference syntax | `SceneReader`/`SceneWriter`; no reference token exists anywhere |
| The fragment model carries **no** object references | `FragmentPiece` (`FragmentExtractor.hpp:60-66`) is `opensWith` · `chapterTitle` · `sceneTitle` · **`text` (flat `std::string`)** · `partial`. Cut/copy/paste across a reference is undefined because the model has nowhere to put one. |
| Nothing resolves, refreshes, or repairs an embedded reference | no renderer, editor, or export path reads one |
| Undo/redo must treat a reference as a unit | EP-019 history is sentence-granular over byte spans; a reference spelled as characters would be editable mid-token |

⚠️ **One backlog claim was CORRECTED during this planning pass.** The entry called a `source`→`scene`
relation type *"additive — a row in `relation-types.json`"*, and I initially flagged that as wrong on the
grounds that `RelationType` endpoints are `ObjectKind`-typed and **`scene` is not an `ObjectKind`**
(`ObjectTypes.hpp:34-58`). **The backlog is right and the objection was wrong:** `RelationType` already
carries `sourceIsScene` / `targetIsScene` bool flags precisely because *"ObjectKind cannot express
'scene'"* (`RelationTypes.hpp:40-43`), and `appears-in` already uses that mechanism. A `source`→`scene`
type is a genuine seed row. **No schema change, no edge migration.**

⚠️ **A second backlog claim did NOT survive the check — see the scope-notes gap below on export.**

---

### Acceptance Criteria

**AC1 — Reference syntax exists and round-trips.**
A scene body can carry a reference to an object by `objectID`, written by ScriviCore and read back
byte-identically. The syntax is chosen by an approved trade ruling (SP-107), survives a save/reopen
cycle, and a body containing one is **still valid Markdown** for every reader that does not understand it.

**AC2 — The fragment model carries references.**
`scrivi.fragment.v1` represents a reference as a **unit**, not as the characters that spell it. Structured
cut, copy and paste of a range containing a reference preserves it intact — including across a scene or
chapter boundary — and the plain-text flattening degrades to readable text.

**AC3 — References render, resolved and live, in the Apple editor.**
A reference displays the referenced object's current `displayName`. Renaming the object updates every
rendering **without editing scene text** — the reference stores identity, never a copy of the name.

**AC4 — A reference behaves as one unit under editing and undo.**
Caret movement, selection, and deletion treat a reference atomically; EP-019 undo/redo restores it as a
unit. It is not possible to leave a half-deleted reference in a scene body.

**AC5 — Footnote and pull-quote presentations for `source`.**
A `source` reference renders in at least these two presentations, selectable per reference, and clicking
one opens **the same citation popup** Doc 2 §3.1.1 specifies — a third entry point onto one popup
implementation, not a new one.

> ✅ **UNBLOCKED 2026-08-24.** EP-034 SP-120 shipped source creation, so AC5 is no longer verifiable only
> on hand-authored fixtures. ⚠️ **The "third entry point" wording is now literal**: the popup has **two**
> live call sites (the aggregate `sources` card and the object's own sources section), and this AC adds
> the third. ⚠️ **The second one was owed since SP-102 and was owned by no AC and no sprint** until SP-120
> planning found it by grepping — ⚠️ **check this AC's own entry point is actually wired**, rather than
> assuming a shipped popup is a reached one.
>
> ⚠️ **AC5 depends on [T-0459](../Tasks/Task-backlog.md)** — a footnote must render **THIS reference's**
> locator, and today `page` sits on the source object where it can hold only one value for every citation
> of that work. **Found by live use one sprint after SP-120's S11 predicted it.**

**AC6 — Deleted and pending targets are handled, and never silently.**
A reference whose object was **deleted** renders as dangling and is repairable; a reference whose object's
**world is unavailable** renders as **pending** and is ⚠️ **never pruned, never rewritten** — EP-031's
*absence is never deletion* rule, now reaching into manuscript text. Repair-matrix rows exist for both.

**AC7 — Export resolves references.** ⚠️ *Scoped by the ruling this Epic owes — see the export gap below.*
Whatever manuscript output path exists when this Epic delivers renders references in a defined way, and
that definition is written down rather than left to the renderer.

**AC8 — `[Linux]` parity.**
Rendering, editing, atomicity, and the pending/dangling presentations behave the same in the Qt/QML editor,
verified in Docker+VNC.

**AC9 — `source`→`scene` relation type seeded.**
Added to `seedTypes()` using the existing `targetIsScene` mechanism, with ⚠️ **T-0416 accounted for** —
seeded types do not reach existing projects, so this AC is not met by adding a row alone.

**AC10 — Suites green on both architectures.**
`ctest` green on macOS arm64 **and** x86-64 with sanitizers; macOS interop green; Linux container green;
app **BUILD SUCCEEDED**. (EP-031 AC10's amended form, carried forward deliberately — it is the stronger one.)

---

### ⚠️ Open questions this Epic must rule BEFORE implementation

These are the reason SP-107 is a **design sprint** and not a coding sprint.

| # | Question | Why it cannot be deferred |
| - | -------- | ------------------------- |
| **Q1** | **What is the reference syntax in a Markdown body?** | It is a permanent on-disk format decision. Everything else — parsing, fragments, export, repair — is downstream of it. |
| **Q2** | **Does a reference store only `objectID`, or a cached display name too?** | Decides whether AC3 is free or needs invalidation. A cache makes stale text possible; no cache makes plain-Markdown readability worse. |
| **Q3** | **What does a reference look like to a non-Scrivi reader of the `.md` file?** | Scene bodies are plain Markdown **by design**. A syntax that renders as noise breaks that property. |
| **Q4** | **Footnote vs. pull quote — where is the choice stored?** | Per-reference in the body, or per-source in the object? Changes AC5's data model. ⚠️ **See T-0459 (2026-08-24): the SAME per-reference-vs-per-source axis has already been ruled for citation LOCATORS — per-reference, on the edge. Q4 should be ruled consistently with it or explicitly diverge.** |
| **Q5** | ⚠️ **What manuscript export actually exists?** | **See the gap below. This one is a genuine hole in the Epic's stated scope.** |
| **Q6** | **Does a reference participate in Spotlight indexing?** | EP-017/I-0118 precedent: `extractSearchableText`'s caller list is a known drift site (`project_capability_without_surface`). |

---

### ⚠️ Scope gap found during planning — export has nothing to attach to

The backlog entry lists **"export behaviour"** as in-scope. **Checked 2026-08-20: there is no manuscript
export path to modify.**

`grep` over the public C ABI returns exactly one export endpoint —
`scrivi_export_project_timeline` (`scrivi.h:478`) — which exports the **timeline**, not the manuscript.
There is no `scrivi_export_manuscript`, and the Apple app's only `export` hits are the timeline strip and
its engine plumbing.

**Consequence, stated rather than glossed:** AC7 as written cannot be verified against a thing that does
not exist. ⚠️ **This is not a reason to drop AC7** — it is a ruling the user owes, and it is Q5:

- **(a)** Descope AC7 — define reference-export semantics in the design doc, implement when a manuscript
  export path is built, and record the deferral explicitly so it is not silently lost the way EP-031's
  source-creation gap was.
- **(b)** Widen EP-032 to build a minimal manuscript export. ⚠️ **Recommend against** — that is its own
  Epic, and widening is exactly how EP-031 went from 6 sprints to 11.
- **(c)** Rule that "export" here means only the fragment model's **plain-text flattening** (AC2), which
  *does* exist, and drop the separate AC7.

**Recommendation: (a) or (c).** Both keep the Epic's boundary where the backlog drew it.

---

### ⚠️ The lesson EP-031 paid for five times — applied here in advance

**`project_capability_without_surface`** — a capability ships, its tests pass, and no writer can reach it.
EP-031 produced this **five times**; its subtlest form (I-0137) had a correct capability, passing unit
tests, **and** a correct call site, with only the data path missing.

> ⚠️ **A passing test suite named after an acceptance criterion is not evidence the criterion is met in
> the product.**

**Binding on this Epic, not advisory:**

1. **Every AC above is written to be verifiable BY USE**, not by suite name. AC1–AC6 and AC8 each describe
   something a writer does in a running app.
2. ⚠️ **No AC is marked Verified on fixture evidence alone.** EP-031's AC24 passed a fixture that supplied
   an input the real product never supplies.
3. **The last sprint is a live-use pass on the real rig** (the `tintagael` project + `Eskandar` world),
   mirroring T-0418 — which is what actually caught I-0137.
4. **A capability landed without its surface is a PARTIAL delivery** and says so in the Task, at the time,
   not at Epic close.

---

### Sprints — ⚠️ RESERVED, none scheduled

⚠️ **EP-032 has no active or planned sprint while it sits in this backlog.** The eight IDs below stay
reserved so they are **not reissued** to work executed before EP-032 — which is exactly the collision the
user identified when ordering the reservation. **SP-107 was in 🔵 Planning at promotion; on this return it
is reserved like the rest**, and the sequencing below stays provisional until SP-107 actually rules Q1–Q6.

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| **SP-107** | ⚠️ **Design + trade rulings** — Q1–Q6, design doc, no implementation | ⚪ **Reserved** (was 🔵 Planning) | — |
| SP-108 | `[ScriviCore]` Reference syntax, parse/write, `source`→`scene` seed type (AC1, AC9) | ⚪ **Reserved, not planned** | — |
| SP-109 | `[ScriviCore]` Fragment-model support + cut/copy/paste (AC2) | ⚪ **Reserved, not planned** | — |
| SP-110 | `[Apple]` Rendering, resolution, atomic editing + undo (AC3, AC4) | ⚪ **Reserved, not planned** | — |
| SP-111 | `[Apple]` Footnote + pull-quote presentations, citation popup (AC5) | ⚪ **Reserved, not planned** | — |
| SP-112 | `[Cross]` Deleted/pending targets + repair-matrix rows (AC6) | ⚪ **Reserved, not planned** | — |
| SP-113 | `[Linux]` Parity (AC8) | ⚪ **Reserved, not planned** | — |
| SP-114 | ⚠️ **AC verification + live-use pass on the real rig + Epic close prep** (AC7, AC10) | ⚪ **Reserved, not planned** | — |

⚠️ **Eight sprints is an estimate made before the design ruling, and the design ruling can change it.**
EP-031 planned 6 and delivered 11 — **four of the five additions came from USE, not from planning.** This
table is not a commitment; it is a starting shape.

> ⚠️ **SP-107–SP-114 are RESERVED IDs, recorded OUTSIDE this file too, so they survive it.**
> All eight appear as ⚪ **Reserved, not planned** in
> [`../Sprints/Sprint-backlog.md`](../Sprints/Sprint-backlog.md) and in
> [`../Sprints/Sprint-Documentation.md`](../Sprints/Sprint-Documentation.md)'s All-Sprints table and
> Statistics block, where **next available reads SP-115, not SP-107**.
>
> ✅ **This has now been TESTED by the very move it anticipated** — EP-032 was returned to the backlog on
> 2026-08-20 and the reservation held in all three files.
>
> **Why, in the user's words (2026-08-20):** if EP-032 is ruled inactive and returned to the Epic backlog,
> IDs documented only here *"get overwritten when developing new Sprints that will be executed before
> these seven."* ⚠️ **The reservation is written to survive exactly that move.** Work sequenced ahead of
> EP-032 takes **SP-115+**; releasing a reserved ID is a **user ruling**, not a planning convenience.

### Tasks

⚠️ **None assigned yet — Tasks are created when SP-107 is planned.** Next available: **T-0419**.

### Issues

⚠️ **Five Issues are OPEN and carried out of EP-031** (`../Issues/Issue-active.md`), none assigned to this
Epic. **I-0137 (High)** is the one to watch: its `WorldStore::listWorlds` `packagePath` gap starves the
world-status refinement, and ⚠️ **AC6's pending presentation reads exactly that status** — so this Epic
may inherit I-0137 as a live blocker.

| ID | Relevance to EP-032 |
| -- | ------------------- |
| **I-0137** | **High** — ⚠️ AC6's pending rendering depends on the status path this Issue reports as broken on real hardware |
| I-0136 | `formatVersion` never compared — a reference format is a second on-disk format with the same exposure |
| I-0135 · I-0138 · I-0139 | Triage only; no known bearing on this Epic |

**Also carried:** **T-0416** (seeded relation types never reach existing projects) — ⚠️ **AC9 depends on
it.** Adding a seed row is not the same as delivering the type to a project that already exists on disk.

### Scope Notes

- ⚠️ **Export (AC7) has no existing path to attach to** — see the gap above. **Owes a user ruling (Q5).**
- **`source`→`scene` is additive** — confirmed against `RelationTypes.hpp:40-43`, not merely inherited
  from the backlog. Uses the existing `targetIsScene` flag; no schema change, no edge migration.
- **EP-034 overlaps but does not conflict.** EP-034 owes **source *creation***; EP-032 owes source
  *reference in text*. ⚠️ **There is a sequencing risk:** AC5 renders footnotes for sources, and if nothing
  in the app can create a source, AC5 is verifiable only on hand-authored fixtures — **which is precisely
  the failure mode this Epic promised to avoid.** Worth ruling whether EP-034 should precede EP-032.
- **Not in scope:** object detail/media surfaces (EP-034), world lifecycle (EP-033), a manuscript export
  subsystem, and any `item`/`artifact` re-ruling (Doc 1 §3.1's open question).

### Completion Summary

*(Filled in when the Epic reaches 🟠 Complete.)*
---

## ⚠️ EP-032 verification & closeout procedure (per `Epic-GUIDELINES.md`)

Recorded at promotion and **kept on the return**, so a future close is not improvised — EP-031's close was
nearly corrupted by two defects that simple greps would have caught.

**At each Sprint close:** archive Verified Tasks/Issues **in the same step** the Sprint closes; remove
closed Sprints from `../Sprints/Sprint-backlog.md`; update `Epic-Documentation.md`.

**At Epic completion, in order:**

1. ⚠️ **Run the AUDIT CHECK FIRST** — read-only, mechanical (greps + counts), per
   [`../Audits/Audit-Guidelines.md`](../Audits/Audit-Guidelines.md). It runs **before** the ACs are judged,
   because the close reads every tracking layer as evidence. ⚠️ **A Check is NOT an Audit; an Epic close
   does not trigger one.** Its findings are ruled as part of that close. Something large or systemic →
   **recommend** a full Audit, which still begins only on user request.
2. **Judge AC1–AC10 against evidence by USE**, not by suite names.
3. Mark 🟠 **Complete**; draft the completion summary.
4. ⚠️ **User reviews and explicitly approves the close. Claude cannot close an Epic.**
5. On approval: move to `Closed/Epic-EP-032.md`, mark ✅ Closed, update `Epic-Documentation.md`, and
   ⚠️ **strip the active-file entry down to a pointer** — do not leave a closed Epic's body behind.

⚠️ **Claude may NOT:** mark ✅ Closed, defer the Epic, or remove an acceptance criterion — all three
require direct user approval.

---

## EP-033: [Cross] World Lifecycle Management — in-app view vs. dedicated application

**Status:** 🔵 **Proposed** (opened 2026-08-14 from the I-0118 design ruling)
**Codebase:** `[Cross]` — undetermined by design; **the product-boundary decision is the Epic's first
deliverable**, and it decides whether this is a Scrivi view or a separate application.
**Goal:** Answer where a **world** is managed across its whole life — created, deleted, shared between
projects, repaired, and its search index maintained — and then build it.
**Origin:** I-0118 Q1 (2026-08-14) — ✅ Verified, archived at
[`Issue-verified-0111-0120.md`](../Issues/Verified/Issue-verified-0111-0120.md), which carries the full
Q1–Q4 ruling text. The ruling was *"entries persist; never reference-counted, never deleted unless
expressly instructed."* **No such instruction exists anywhere in the product**, and the user deferred
inventing one pending this larger decision.

**Why this is an Epic and not an affordance.** "Add a Remove-from-Spotlight button" was the small version
of the question. The real one is **who owns a world**, and the current answer is nobody:

- a world is **shared between projects** and outlives every one of them, so a per-project panel is
  structurally the wrong home for its lifecycle — every binding project would offer the same destructive
  action against shared state, with no coordination;
- **deleting a world** has no home at all today: `scrivi_remove_world_reference` unbinds *this project's*
  reference and deliberately never touches the package (`scrivi.h:264`), which is correct — but it means
  nothing in Scrivi can delete a world, by design;
- under the I-0118 ruling a world's **search entries are write-only** — donated, never removed, with no
  affordance to clear them even when the package is gone from disk;
- **a world with no project bound to it is unreachable.** Scrivi opens *projects*; a world is only ever
  seen through one. A writer with a world and no project has no way in.

**The fork to rule first:**

| Option | Consequence |
| --- | --- |
| **A — a view inside Scrivi** | Cheapest; reuses the Worlds panel and identity/bookmark plumbing. But the per-project framing stays wrong for shared state, and it cannot manage a world with no project. |
| **B — a dedicated world-management application** | Matches how worlds actually live: one owner, across projects, independent of any manuscript. Costs a second app (packaging, sandbox grants, identity, its own release), and Scrivi still needs *some* in-app surface for binding. |

**Rough scope (either way):** world delete (package + index entries, with the "positively established"
care of I-0115); explicit index-entry removal; a worlds registry or browse path that does not require a
project; sharing/binding across projects; repair for a world whose package moved or died.

**Depends on:** EP-031 (the world partition, bindings, and the world package format).
**Explicitly deferred from:** I-0118, which ships world **indexing** and leaves **removal** to this Epic.

---


*Last Updated: 2026-08-20 (**EP-034 `[Cross]` PROMOTED out of this backlog → 🟡 ACTIVE**, widened in place
per user ruling and now carrying the full **Detail Sheet + Project↔World** scope; detail in
[`Epic-active.md`](Epic-active.md). ✅ **Q-a ruled: EP-034 precedes EP-032**, so ⚠️ **EP-032's reserved
SP-107–SP-114 stay reserved while EP-034 executes SP-115+ — sprint IDs run OUT OF SEQUENCE.** Backlog
Epics 4 → 3 (EP-026 `[Linux]`, EP-032, EP-033); next available Epic **EP-035**. Prior note follows.)*

*2026-08-20 (**EP-032 `[Cross]` RETURNED to this backlog by user ruling** — promoted and
returned the same day; Active Epics 1 → 0, Backlog Epics 3 → 4. ⚠️ **It returns with its FULL planning pass
intact, not as a rough sketch** — AC1–AC10, the Q1–Q6 design questions, the code-verified claim table, the
export scope gap (Q5) and the closeout procedure, per this file's rule that returned Epics *"retain their
verified AC state."* **Re-promoting it does not require redoing the planning.** ⚠️ **Its sprint IDs
SP-107–SP-114 remain RESERVED and are NOT released by this return** — recorded in
`../Sprints/Sprint-backlog.md`, `../Sprints/Sprint-Documentation.md` (table + Statistics) and in EP-032's
own Sprints section above; **next available Sprint for other work is SP-115, not SP-107.** Next available
Epic **EP-035**. Prior note follows.)*

*2026-08-20 (**EP-032 `[Cross]` Inline Object References PROMOTED out of this backlog to
🔵 Draft** — full detail now in [`Epic-active.md`](Epic-active.md); it is the active Epic and its first
sprint **SP-107** (design + trade rulings Q1–Q6) is next to plan. Its block was removed from this file rather
than left behind as a duplicate. ⚠️ **Two of its scope claims were checked against code during promotion:**
the `source`→`scene` "additive relation type" claim is **CONFIRMED** (`RelationTypes.hpp:40-43` already carries
`targetIsScene`, as `appears-in` uses), but the **"export behaviour"** scope item **has no existing path to
attach to** — the only export endpoint in the C ABI is `scrivi_export_project_timeline` (`scrivi.h:478`),
which is the timeline, not the manuscript. That is now **Q5**, a ruling the Epic owes. Backlog Epics 4 → 3
(EP-026 `[Linux]`, EP-033, EP-034); next available Epic **EP-035**. Prior note follows.)*

*2026-08-18 (**EP-034 `[Cross]` opened 🔵 Proposed** — object detail & media, from a user
finding while testing T-0365's `sources` card: sources cannot be created anywhere in the app, and the
cause is that **object cards edit `displayName` only** while `subtitle`/`notes`/`image` have shipped in
ScriviCore since SP-095 and are read by nothing. ⚠️ **Source creation was deliberately NOT put on the
`sources` card** — user ruling: a citation documents an object, so creating it from a scene-scoped
aggregate card inverts the model. **EP-032 was checked and does not cover this** (it is manuscript
reference syntax). Backlog Epics 3 → 4 (EP-026 `[Linux]`, EP-032, EP-033, EP-034); next available Epic
**EP-035**. Prior note follows.)*

*2026-08-14 (**EP-033 `[Cross]` opened 🔵 Proposed** — world lifecycle management, deferred
out of I-0118 by user ruling. The I-0118 Q1 ruling ("entries persist; never deleted unless expressly
instructed") has **no instruction to give** — nothing in Scrivi deletes a world or its index entries, and
a world with no project bound is unreachable entirely. **The Epic's first deliverable is the product
fork:** a view inside Scrivi vs. a dedicated world-management application. Backlog Epics 2 → 3 (EP-026
`[Linux]`, EP-032 `[Cross]`, EP-033 `[Cross]`); next available Epic **EP-034**. Prior note follows.)*

*2026-08-12 (**EP-032 `[Cross]` opened 🔵 Proposed** — inline object references in the
manuscript, deferred out of EP-031 by user ruling: sources attach to objects now, source-in-manuscript
(footnotes / pull quotes) needs a capability Scrivi does not have and is version-crossing. Backlog Epics
1 → 2 (EP-026 `[Linux]`, EP-032 `[Cross]`); next available Epic **EP-033**.)*

*2026-07-22 (**EP-025 [Linux] Timeline Panel promoted from this backlog to 🟡 Active** (full
detail now in `Epic-active.md`); its first sprint SP-079 activated same day. The largest Linux Epic (full
Apple EP-016 parity, ~5 sprints). Remaining [Linux] family: **EP-026** stays 🔵 Draft (promoted when EP-025
lands). EP-019 [Apple] remains 🔴 Deferred. Prior: 2026-07-22 (EP-024 ✅ closed same-day); 2026-07-15 (EP-022
✅ closed; EP-023 promoted to Active).)*
