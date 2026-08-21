# Task Backlog

Tasks listed here are **documented, unstarted, and not assigned to an active Sprint.**

**What does _not_ belong in this file:**

| State | Belongs in |
| ----- | ---------- |
| 🟢 **Active** (assigned to a Sprint, being worked) | [`Task-active.md`](Task-active.md) |
| 🟠 **Implemented — Not Verified** | [`Task-unverified.md`](Task-unverified.md) |
| ✅ **Verified** | `Verified/Task-verified-XXXX.md` — removed from **both** list files |

> ✅ **T-0390, T-0418 and T-0391 left this file 2026-08-19** when **SP-100 was activated** — they are
> 🟡 Active / assigned and live in [`Task-active.md`](Task-active.md). Their detail sections below are
> retained as the plan of record until the sprint closes.

⚠️ **A Task's row leaves this file when its Sprint closes.** The Sprint archive is the record of
truth; a backlog row that outlives its Sprint can only ever contradict it. **A Sprint may be
deactivated without deactivating its Tasks** — the Tasks stay in `Task-active.md` and the Sprint's
status changes on its own layer.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0197 | Enable Core Spotlight donation on iOS/iPadOS (+ iOS deep-link/bookmark consumer); visionOS when backend links | EP-017 (deferred from T-0190) | 🔵 Backlog |
| T-0249 | `[Linux]` Manuscript navigation gestures — Page Forward/Backward + jump to absolute manuscript start/end | EP-022 (unscheduled) | 🔵 Backlog |
| T-0400 | `[ScriviCore]` History log-segment rotation | EP-019 (deferred) | 🟢 **Nice to have** — no sprint assigned |
| T-0416 | ⚠️ Seeded relation-type vocabulary does not reach existing projects | EP-031 (unscheduled) | 🔵 Backlog |

> ✅ **T-0426–T-0430 LEFT this file 2026-08-21 when SP-116 was ACTIVATED** — they are 🟡 Active and live
> in [`Task-active.md`](Task-active.md). ⚠️ **Exactly as the note that stood here said they would**, per
> the standing rule that active Tasks never sit in the backlog.

> ✅ **T-0434–T-0440 LEFT this file 2026-08-21 when SP-117 was ACTIVATED** — they are 🟡 Active in
> [`Task-active.md`](Task-active.md), exactly as the note that stood here said they would be.

### Not backlog — recorded here only as pointers

| Task | Disposition |
| ---- | ----------- |
| T-0175 | ⚪ **Superseded** by EP-017 (2026-06-23) — expanded into an Epic; no work tracked under this ID |
| T-0185–T-0188 | ⚪ **Descoped** — I-0057, `CSImportExtension` non-functional on macOS → [`Closed/Task-closed-0185-0188.md`](Closed/Task-closed-0185-0188.md) |
| T-0191 | ✅ **Done** 2026-06-24 (throwaway spike, no archive — findings in `Sprint-SP-048.md` + design §3.2) |
| T-0199 | ✅ **Done** 2026-07-06 (throwaway spike, no archive — findings in design §8/§12.6) |
| T-0215 | ⚪ **Superseded → T-0366** — ships as the `history` card (EP-030 SP-092), not a standalone panel |
| T-0216 | ⚪ **Closed — OBE** 2026-08-05 (user-directed) — see the warning below |
| T-0410 | ⛔️ **Removed as OBE** 2026-08-15 (user-ruled) — traced to no design section, AC or defect |

---

## Detail

### T-0118 — Scroll bar fidelity — per-scene character-ratio thumb position and size

**Epic:** EP-011 · **Status:** 🔵 Backlog · **Component:** `ViewportSceneLoader`, scroll bar overlay (new)
**Priority:** Medium · **Date Requested:** 2026-06-08 · **Sprint Assigned:** **Not Assigned**

> ⚠️ **Folded in from `Task-0118.md` on 2026-08-19** (audit ruling **R-27**) — it was the only per-Task
> detail file outside `Verified/` and matched no documented pattern. **Content preserved verbatim.**
> ⚠️ **One correction:** the file recorded *"Sprint Assigned: SP-034"* — **SP-034 was cancelled**, so the
> Task is **Not Assigned**.

**Rationale:**
With the all-in-memory model (T-0114), the `NSScrollView` scroll bar will naturally reflect the full
manuscript extent — the primary fidelity problem from the old load/release model is resolved. This task
addresses residual fidelity issues and implements any remaining polish. Absorbs T-0096.

**Current Behavior:**
`NSScrollView` scroll bar thumb reflects only the loaded text buffer. With T-0114 complete this is largely
fixed, but character-count-based proportional sizing and positioning may still need tuning.

**Desired Behavior:**
- Scroll bar thumb position and size reflect the author's true proportional position in the full manuscript.
- Size ratio: `scene.charCount / manuscript.totalCharCount`
- Position offset: sum of size ratios of all preceding scenes

**Requirements:**
1. Scene character counts are available from `sceneStartMap` (T-0115) or `scene.meta.json`
2. Scroll bar thumb accurately reflects manuscript position for manuscripts with 2–20 scenes
3. Approach (custom `NSScroller` subclass vs. overlay) chosen and documented

**Design Approach:**
With T-0114 (all-in-memory), `NSScrollView` may handle this automatically. Evaluate first; only implement
custom logic if the native scroll bar is still inaccurate.

**Dependencies:** T-0114 and T-0115 must be complete.

**Test Steps:**
1. Open a 5-scene project; scroll to the midpoint of Scene 3
2. Confirm scroll bar thumb is near 50% (assuming equal-length scenes)
3. Open a project where Scene 1 is very long; confirm thumb starts large

**Notes:**
If T-0114 (all-in-memory) makes the native `NSScrollView` scroll bar accurate by default, this task may be
trivial to close. Evaluate before implementing custom logic.

### T-0197 — Core Spotlight donation on iOS/iPadOS

**Epic:** EP-017 (deferred from T-0190) · **Status:** 🔵 Backlog

Broaden the `SpotlightDonor` / `AppEnvironment` guards from `os(macOS)` to `os(macOS) || os(iOS)`,
verify Spotlight entitlements on the iOS target, and wire the iOS deep-link/bookmark **consumer** side
so a tapped Spotlight result can resolve a persisted project across launches (`ProjectBookmarkStore`
is currently macOS-only). visionOS is out of scope until its `ScriviEngine` backend links (I-0053
covered iOS/iPadOS only). Gated on (a) T-0189 ✅ done, and (b) iOS bookmark/restore parity.
Rationale + assessment: [`Verified/Task-verified-0190.md`](Verified/Task-verified-0190.md).

### T-0249 — `[Linux]` Manuscript navigation gestures

**Epic:** EP-022 (unscheduled) · **Status:** 🔵 Backlog

Page Forward / Page Backward + jump to absolute manuscript start / end. **Gestures and keystrokes are
undecided** — laptops and tablets lack extended keyboards, so no PageUp/PageDown/Home/End may be
assumed; pick touch-friendly bindings. Raised during SP-063 VNC verification.

⚠️ **Constrained by the VNC input path** (see `project_linux_vnc_input_constraints`): the
macOS→VNC→x11vnc→Xvfb→Qt chain drops Ctrl+Shift+key combos and pinch/pixelDelta gestures, so **a
gesture must never be the only path** to any of these actions.

*EP-022 itself is ✅ Closed (2026-07-15); this is an unscheduled follow-up, likely landing with EP-026
menus/parity.*

### T-0390 — External Change Repair Matrix: world-package conditions

**Epic:** EP-031 (**SP-100 🟡 Active**) · **Status:** 🟡 **Active (2026-08-19)** — runs **first** in the build order.

⚠️ **Greenfield, confirmed at SP-100 planning (2026-08-19):** the matrix
(`Scrivi_External_Change_Repair_Matrix_v0_2.md`, 578 lines, conditions §6.1–§6.21) contains **zero
occurrences of "world" or "scrivworld"**. Worlds landed in SP-097, three sprints after it was last
revised. Scope: §6.22–§6.28 (package missing · `worldID` mismatch · corrupt `world.json` · permanently
unresolvable binding · stale lock · object absent from the world index · worldless project) plus the §5
state vocabulary. **Ruled (R3): document AND test against shipped behaviour; file disagreements as
Issues rather than fixing them in a verification sprint.**

⚠️ **§6.22 and §6.25 must state *absence is never deletion* in the document's own voice** — the matrix is
where a maintainer looks to decide what a repair pass may delete, and **a pass written against the current
document could destroy a writer's whole relationship graph and satisfy every rule in it.**

### T-0391 — EP-031 verification + Epic close prep

**Epic:** EP-031 (**SP-100 🟡 Active**) · **Status:** 🔵 Backlog — runs **last** in the build order; ⚠️ **owns the AC1 re-verification.**

Per-AC pass over AC1–AC10 with named evidence. ⚠️ **AC1 and AC10 are AMENDED first, then verified**
(SP-100 rulings R1/R2) — both carry clauses superseded by the §3.0 no-migration ruling. Also resolves the
**T-0369 open question** below, and amends Doc 1 §3.0 consequence 4. **Claude cannot close the Epic.**

### T-0418 — ⚠️ Live-use pass on the real rig

**Epic:** EP-031 (**SP-100 🟡 Active**) · **Status:** 🔵 Backlog — runs **second** in the build order. ✅ **Rig backed up 2026-08-19 (user-confirmed).**

Create an object in **each of the ten world kinds** plus a project-scoped `source`; relate them from both
entrances including a symmetric and a cross-partition `cites` edge; **eject**; confirm pending
presentation and that reattach restores **with no writer intervention** (I-0129's exact defect); quit and
reopen with the drive present and absent.

**Why it exists:** ⚠️ **four of EP-031's eleven sprints were unplanned and every one came from USE**, and
SP-102's live runs produced eight Issues. Step 1 doubles as **AC1's re-verification performed by use**.

⚠️ **Back up the rig first** — `~/Desktop/the-stairs-of-tintagael.scrivi` +
`/Volumes/Scrivi Worlds/Eskandar.scrivworld` hold **real writing work**. Findings are **filed, not fixed**.
⚠️ **May surface T-0416** (seeded relation types never reach existing projects) as a live blocker.

### T-0400 — `[ScriviCore]` History log-segment rotation

**Epic:** EP-019 (deferred) · **Status:** 🟢 **Nice to have** (user ruling 2026-08-11) — not a
blocker; no sprint assigned.

`activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); nothing rotates it, so
a project accumulates one unbounded segment (a real project is already ~3.4 MB). Capacity/eviction
bounds the **tree**, not the **log**. The reader already honours `activeLogSegment` from `state.json`,
so rotation is **additive, with no format change**. Documented as not-implemented in package-structure
§16a. Raised by T-0217; deliberately **not** folded into a verification sprint.

### T-0416 — ⚠️ Seeded relation-type vocabulary does not reach existing projects

**Epic:** EP-031 (unscheduled) · **Status:** 🔵 Backlog · **Deferred by user ruling 2026-08-17.**

`RelationTypeStore::load` re-seeds `objects/relation-types.json` **only** when the file is missing or
unparseable (`RelationTypes.cpp:150-176`); a valid file is loaded verbatim, so **any change to the
seeded vocabulary is invisible to every project already on disk**.

Surfaced by **I-0125**: widening `appears-in` fixed new projects while the user's real project kept
the old constrained type and kept failing chronicle creation. The immediate blocker cleared — his
file was patched by hand — so the general mechanism is a separate decision.

**Options considered:** reconcile seeded types on open (add missing, update changed, never touch
writer-authored types); or carry a `seedVersion` in the file and re-seed on version bump.

⚠️ **Whatever is chosen must not clobber a type a writer deliberately customized** — the file is
writer-editable by design.

---

## ⚠️ T-0216 — closed OBE, with a coverage gap that outlived it

**⚪ CLOSED — OBE 2026-08-05 (user-directed).** Original scope: integration fixtures (100k-event
history, 500 KB single scene) + a `.gitignore` migration adding `history/` for existing projects
(design §6.c, §11).

**Why closed:** the `.gitignore` migration is obsolete — **Scrivi has not shipped**, so no projects
exist in the field whose `.gitignore` predates the `history/` entry; new projects get it at creation.
The user closed the task whole rather than splitting it.

⚠️ **What this gives up:** the **perf fixtures were never built**, so EP-019 shipped with **no**
large-scale performance coverage — the 100k-event history and 500 KB single-scene cases (design §6.c
worst case) are untested. `HistoryService` has unit + integration coverage at ordinary sizes only.
**If history performance is ever suspect at scale, this is the missing evidence** — re-open as a new
task rather than reviving T-0216.

---

*Last Updated: 2026-08-21, fourth pass (**T-0434–T-0440 REMOVED — SP-117 activated**; they moved to
`Task-active.md` in the same step. Next available Task: **T-0441**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**T-0434–T-0440 added for SP-117** 🔵 Planning under EP-034 — the
Detail Sheet pane, back/forward history, ⚠️ **a typed Swift object model** (there is none today), the three
plain fields, both entry points, pending read-only, and ⚠️ **T-0420's long-owed surface**. ⚠️ **Detail lives
in the Sprint entry** (P7). ⚠️ **`tags` deliberately NOT here** — deferred to SP-119, so **R2/AC2 cannot
close in SP-117**. Next available Task: **T-0441**. Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**T-0426–T-0430 REMOVED — SP-116 activated** and they moved to
[`Task-active.md`](Task-active.md) in the same step. ⚠️ **The backlog is for unstarted, unassigned work
only** (`feedback_task_layer_discipline`). Next available Task: **T-0431**. Prior note follows.)*

*Last Updated: 2026-08-21 (**T-0426–T-0430 added for SP-116** 🔵 Planning under EP-034 — D6 world assets,
D7 `assetPath`, ⚠️ **I-0143's escaping fix**, D5's kind-scope endpoint **including its Swift adoption**, and
I-0141's rotted comment. ⚠️ **Detail lives in the Sprint entry, not here** (P7). ⚠️ **T-0429 reaches into
Swift deliberately** — shipping the endpoint without adopting it would leave I-0140 open with its cure
unused, which is `capability_without_surface` committed by the sprint curing occurrence *eight* of its
sibling. Next available Task: **T-0431**. Prior note follows.)*

*Last Updated: 2026-08-18 (**Documentation audit — this file was rebuilt.** It had grown to 744 lines
carrying **~60 ✅ Verified Tasks, 3 🟢 Active Tasks and 7 🟠 Implemented — Not Verified Tasks**, none
of which belong here. All 54 Verified Tasks lacking an archive were written to `Verified/`
(`0213-0214`, `0217`, `0234-0248`, `0359-0364`, `0370-0406`, `0394-0395-0366-0367`, `0396-0398`,
`0399`) and removed from both list files; active Tasks moved to `Task-active.md`; unverified Tasks
moved to `Task-unverified.md`. ⚠️ **The worst staleness: T-0246/T-0247/T-0248 were shown 🔵 Backlog
and 🟡 Implemented — i.e. unstarted work — on EP-022, an Epic closed 2026-07-15, a month earlier.**
The "Full Detail" section, which duplicated Sprint-archive content for closed Sprints, was removed
wholesale. Prior note follows.)*

*2026-08-16 (Consistency audit — eight stale rows removed or corrected: T-0386/T-0387/T-0388/T-0407/
T-0408 (SP-099) and T-0409/T-0411 (SP-103), all ✅ Verified 2026-08-15; and T-0217, carried 🔵 Backlog
under the superseded SP-057 despite completing in SP-094.)*
