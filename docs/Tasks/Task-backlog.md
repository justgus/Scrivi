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
| **T-0459** | ⚠️ **`[Cross]` Per-citation reference markers on the `cites` EDGE** — surface the edge `note`, and ⚠️ **move `page` off the source object** | **EP-032** | 🔵 **Backlog** — ⚠️ **found by live use, SP-120** |
| **T-0472** | ⚠️ **`[Linux]` Surface for custom relationship types** — build the UI that calls `scrivi_upsert_relation_type`. ⚠️ **The endpoint is fully implemented and has NEVER had a caller on any platform** | **EP-035** | 🔵 **Backlog** — ⚠️ **user ruling 2026-08-25: a REAL FEATURE owed a surface, not dead code** |
| **T-0473** | ⚠️ **`[Apple]` Timeline endpoint parity** — reach `resolve_timeline_project_times`, `set_timeline_epoch_offset`, `set_world_epoch_offset`. ⚠️ **Unreached on Apple; bridged on Linux by SP-121** | ⚠️ **None — no Epic** | 🔵 **Backlog** — ⚠️ **user ruling 2026-08-25: tracked, NOT scheduled** |

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

### T-0459 — ⚠️ `[Cross]` Per-citation reference markers belong on the EDGE, not the source

**Epic:** EP-032 · **Status:** 🔵 Backlog · **Component:** `cites` edge `note`, `ObjectSourcesSection`,
`CitationPopover`, `scrivi_c_api` · **Priority:** Medium · **Date Requested:** 2026-08-24
**Sprint Assigned:** **Not Assigned** — ⚠️ **deliberately NOT folded into SP-120** (see below)

#### The finding (user, 2026-08-24, from real citation work)

> *"I currently have, in The Lone Golem, Myton sourced by 'The Stairs of Tintagael'. Myton appears on
> almost every page of that document and so the entire document should be the source for Myton at 23.
> However, Tintagael does not. We may only wish to highlight certain passages… to adequately reference
> the location."*

**One publication, two citations, different locators.** Myton's citation covers the whole work; Tintagael's
covers specific passages.

#### ✅ Ruling (user, 2026-08-24)

**ONE `source` object per publication. The reference markers go on the `cites` edge.**

⚠️ **A source object per reference was considered and rejected.** It would (1) assert five publications
exist where one does, (2) ⚠️ **defeat the aggregate card's de-duplication**, which is *by source ID* and
was an explicit T-0365 design point (*"two rows for one source reads as two sources"*), and (3) turn one
publisher correction into N edits.

✅ **Free text on the edge is sufficient** — *"I'll take what I have today for nothing."* ⚠️ **No structured
page/chapter/paragraph fields.** The `note` field already exists on every edge and already round-trips.

#### ⚠️ SP-120 put `page` in the WRONG PLACE, and said so at the time

`attributes["page"]` is on the **source object**, so it holds **one value for every citation of that work**
— Myton and Tintagael cannot differ. ⚠️ **SP-120's own S11 §5.2 flagged this and shipped it anyway:**

> *"Belongs on the **edge** in a stricter model (two objects citing different pages of one book) —
> ⚠️ **recorded as a known modelling limit**, not silently accepted."*

⚠️ **The limit was hit ONE SPRINT LATER, by the first writer to do real citation work.** ✅ **Recording it
is what made this a five-minute diagnosis instead of a rediscovery** — the S11 entry is the reason the
answer was already written down.

#### Scope — ⚠️ this is `[Cross]`, which is why it is not in SP-120

| Piece | State |
| ----- | ----- |
| `note` on `scrivi_create_edge` | ✅ **Ships** — `EdgeView.note` round-trips; ⚠️ **SP-120 passes `""` at every call site and surfaces it nowhere** |
| ⚠️ **`scrivi_update_edge`** | ❌ **DOES NOT EXIST.** Only create/delete/list (`scrivi.h:163-200`). ⚠️ **A locator cannot be corrected without deleting and recreating the edge**, which loses the edgeID |
| Surface the note when citing | ❌ Not built — `ObjectSourcesSection` create + attach both pass `""` |
| Show it in `CitationPopover` | ❌ Not built |
| ⚠️ **Migrate `attributes["page"]`** | ❌ **A data decision, not just code** |

⚠️ **Folding this into SP-120 was rejected by the user.** SP-120 is `[Apple]`-only and **verified by use**;
adding a ScriviCore change after verification would make its record dishonest — the same reason SP-116 was
re-ruled `[Cross]` **at planning** rather than quietly widened.

#### ⚠️ Why EP-032 and not EP-034

**EP-032's AC5 renders footnotes for `source` references, and a footnote needs THIS reference's locator**
— not the work's single page number. ⚠️ **It is also adjacent to EP-032's own open question Q4** —
*"Footnote vs. pull quote — where is the choice stored? Per-reference in the body, or per-source in the
object?"* ⚠️ **That is the SAME per-reference-vs-per-source axis this Task resolves for locators**, so
ruling them together is cheaper than ruling them apart, and ⚠️ **ruling them apart risks answering the
same question two different ways.**

Assigning it to EP-034 would widen that Epic a second time for work its own ACs do not name.

---

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

**Epic:** EP-031 → **EP-034** · **Status:** ✅ **CLOSED — shipped as T-0441 + I-0149 (SP-118), Verified
2026-08-23.** Deferred by user ruling 2026-08-17; scheduled 2026-08-21; shipped and verified 2026-08-23.

⚠️ **Kept here for its history, not as open work**, and removed from the backlog table above. The record is
[`../Sprints/Closed/Sprint-SP-118.md`](../Sprints/Closed/Sprint-SP-118.md) and
[`Verified/Task-verified-0441-0445.md`](Verified/Task-verified-0441-0445.md).

⚠️ **T-0441 alone did NOT fix it.** The reconciliation was placed in `RelationTypeStore::load()`, which a
project open never calls — the repair shipped correct and unreachable, and **I-0149** is what actually
made it run on open. ⚠️ **"On open" is an event, not a function.**

`RelationTypeStore::load` re-seeds `objects/relation-types.json` **only** when the file is missing or
unparseable (`RelationTypes.cpp:150-176`); a valid file is loaded verbatim, so **any change to the
seeded vocabulary is invisible to every project already on disk**.

Surfaced by **I-0125**: widening `appears-in` fixed new projects while the user's real project kept
the old constrained type and kept failing chronicle creation. The immediate blocker cleared — his
file was patched by hand — so the general mechanism is a separate decision.

**Options considered:** reconcile seeded types on open (add missing, update changed, never touch
writer-authored types); or carry a `seedVersion` in the file and re-seed on version bump.

> ### ✅ CONFIRMED ON THE REAL RIG — 2026-08-21, and RULED
>
> ⚠️ **`the-twisted-remains-of-myself.scrivi` still carries the pre-I-0125 `appears-in`** —
> `sourceKind: "character"` and `inverseLabel: "has characters"`. **Proven by running the same operation
> against both vocabularies through `scrivi_*`:** a fresh seed creates the edge; the drifted one fails with
> *"endpoints do not satisfy the kind constraints of relation type 'appears-in'"*.
>
> ⚠️ **`appears-in` is the type EIGHT of the ten object cards use**, and the object is written to disk
> **before** the edge fails — so the writer is told creation failed while the object exists.
>
> ✅ **`tintagael` is CLEAN** (hand-patched at I-0125), ⚠️ **so the rig will not reproduce this by
> default** — a drifted fixture is required or a test passes vacuously.
>
> ✅ **RULED (user, 2026-08-21): reconcile on open, seeded types only.** ⚠️ **Accepted consequence: it
> overwrites a seeded type a writer deliberately edited** — chosen over leaving a hand-edited `appears-in`
> broken forever with no explanation. **Scheduled as T-0441 in SP-118.**

⚠️ **The earlier note here read "whatever is chosen must not clobber a type a writer deliberately
customized."** ⚠️ **The 2026-08-21 ruling deliberately overrode that for SEEDED types** — it is the accepted
consequence, chosen over leaving a hand-edited `appears-in` broken forever. It still holds in full for
**writer-authored** codes, which reconciliation never touches and never deletes.

✅ **As implemented (T-0441):** seeded codes are added if missing and replaced if they differ; every other
code is left alone; nothing is deleted; and the file is written **only when something actually changed**
(S3 — an unconditional rewrite would churn mtimes and Git status on every open).

---

## ⚠️ T-0472 — `[Linux]` Surface for custom relationship types

**Epic:** EP-035 · **Ruled:** 2026-08-25 (user) · **Origin:** EP-034 close / SP-121's ABI gap audit

⚠️ **`scrivi_upsert_relation_type` is fully implemented and has zero callers on ANY platform.** The core
writes custom relationship types into the project; `ProjectCreator` seeds the defaults
(`RelationTypeStore::seedTypes()`); `scrivi_list_relation_types` READS them and the app uses it.
⚠️ **Nothing can ADD one** — the write half has never been surfaced.

✅ **User ruling: this is a REAL FEATURE owed a UI, not speculative code.** Custom relationship vocabulary
is a legitimate worldbuilding capability — a DM wanting `sworn-enemy-of` or `liege-of` beyond the seeded
set.

⚠️ **Assigned to EP-035 because that Epic builds the Linux object layer**, where relationship creation
already lands. ⚠️ **It is BACKLOG, not scheduled** — EP-035 is itself 🔵 Proposed.

⚠️ **Also owed: a C ABI test.** `scrivi_upsert_relation_type` has **no ScriviCore test at the boundary**
(`feedback_boundary_tests_not_facade` — a facade test cannot see a boundary gap; that is how I-0113
shipped green).

---

## ⚠️ T-0473 — `[Apple]` Timeline endpoint parity

**Epic:** ⚠️ **NONE — deliberately unassigned** · **Ruled:** 2026-08-25 (user)

⚠️ **Three endpoints are unreached on Apple**: `resolve_timeline_project_times`,
`set_timeline_epoch_offset`, `set_world_epoch_offset`. ✅ **All three are bridged on Linux** (SP-121), and
⚠️ **the two epoch-offset endpoints have real core implementations AND core tests** (`WorldStore`) — so
this is a missing Apple surface, not missing capability.

⚠️ **Linux's Timeline Panel is 1,422 lines and is the mature surface; Apple's is thinner.** ⚠️ **This is
a genuine platform asymmetry**, and the endpoints exist because Linux needed them first.

✅ **User ruling: Apple parity IS owed — file it so it is tracked rather than rediscovered at each audit.**
⚠️ **NOT scheduled, and carries NO Epic** — it is a tracked debt, and ⚠️ **must not be read as a
commitment in any current Epic.**

⚠️ **`resolve_timeline_project_times` has NO core test either** — unlike the two epoch setters.

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

*Last Updated: 2026-08-23, ninth pass (**T-0446–T-0450 MOVED to `Task-active.md`** — SP-119 activated
2026-08-23, so they leave the backlog per layer discipline. Next available Task: **T-0451**. Prior note
follows.)*

*Last Updated: 2026-08-23, eighth pass (**T-0446–T-0450 ADDED for SP-119** 🔵 Planning — images, tags and
AC9's second half; **closes AC2, AC3, AC4**. ⚠️ **T-0446 is `[ScriviCore]`** by user ruling: the object
index carries `image`, so a thumbnail is one read for the whole list rather than N reads in a 280pt pane.
⚠️ **The asset stack has ZERO view callers today.** Next available Task: **T-0451**. Prior note follows.)*

*Last Updated: 2026-08-23, seventh pass (**T-0416 REMOVED from the backlog table** — ✅ shipped as T-0441 +
I-0149 and Verified at SP-118 close; its detail block is kept for history only. Next available Task:
**T-0446**. Prior note follows.)*

*Last Updated: 2026-08-22, sixth pass (**T-0441–T-0445 MOVED to `Task-active.md`** — SP-118 activated
2026-08-22, so they leave the backlog per layer discipline. **T-0416 is superseded by T-0441**, which is
✅ Implemented — Not Verified. Next available Task: **T-0446**. Prior note follows.)*

*Last Updated: 2026-08-21, fifth pass (**T-0441–T-0445 added for SP-118** 🔵 Planning; **AC5 + AC6** close
there. ⚠️ **T-0416 CONFIRMED ON THE RIG and finally scheduled** as **T-0441** after being deferred since
2026-08-17 — the drifted `appears-in` was proven to fail the exact operation SP-118 is about. Next
available Task: **T-0446**. Prior note follows.)*

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
