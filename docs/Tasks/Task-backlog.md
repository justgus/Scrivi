# Task Backlog

Tasks listed here are **documented, unstarted, and not assigned to an active Sprint.**

**What does _not_ belong in this file:**

| State | Belongs in |
| ----- | ---------- |
| 🟢 **Active** (assigned to a Sprint, being worked) | [`Task-active.md`](Task-active.md) |
| 🟠 **Implemented — Not Verified** | [`Task-unverified.md`](Task-unverified.md) |
| ✅ **Verified** | `Verified/Task-verified-XXXX.md` — removed from **both** list files |

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
| T-0369 | EP-030 verification + Epic close prep | EP-030 (SP-094) | 🔵 Backlog |
| T-0390 | External Change Repair Matrix — world-package conditions | EP-031 (SP-100) | 🔵 Backlog |
| T-0391 | EP-031 verification + Epic close prep | EP-031 (SP-100) | 🔵 Backlog |
| T-0400 | `[ScriviCore]` History log-segment rotation | EP-019 (deferred) | 🟢 **Nice to have** — no sprint assigned |
| T-0416 | ⚠️ Seeded relation-type vocabulary does not reach existing projects | EP-031 (unscheduled) | 🔵 Backlog |

### Not backlog — recorded here only as pointers

| Task | Disposition |
| ---- | ----------- |
| T-0175 | ⚪ **Superseded** by EP-017 (2026-06-23) — expanded into an Epic; no work tracked under this ID |
| T-0185–T-0188 | ⚪ **Descoped** — I-0057, `CSImportExtension` non-functional on macOS |
| T-0191 | ✅ **Done** 2026-06-24 (throwaway spike, no archive — findings in `Sprint-SP-048.md` + design §3.2) |
| T-0199 | ✅ **Done** 2026-07-06 (throwaway spike, no archive — findings in design §8/§12.6) |
| T-0215 | ⚪ **Superseded → T-0366** — ships as the `history` card (EP-030 SP-092), not a standalone panel |
| T-0216 | ⚪ **Closed — OBE** 2026-08-05 (user-directed) — see the warning below |
| T-0410 | ⛔️ **Removed as OBE** 2026-08-15 (user-ruled) — traced to no design section, AC or defect |

---

## Detail

### T-0118 — Scroll bar fidelity

**Epic:** EP-011 · **Status:** 🔵 Backlog
Per-scene character-ratio thumb position and size. Full detail: [`Task-0118.md`](Task-0118.md).

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

### T-0369 — EP-030 verification + Epic close prep

**Epic:** EP-030 (SP-094) · **Status:** 🔵 Backlog

EP-030 verification (AC1–AC7) + Epic close prep. **User approval required to close.**

> **Moved from SP-093 → SP-094** (user ruling, 2026-08-07). T-0369 and EP-019's SP-057 were both
> one-task, no-build-work verification passes gated on the same live session and app build, so they
> merged into a single **SP-094 — "EP-019 + EP-030 verification & Epic close."** Both Epics' ACs were
> verified in one pass, then **closed independently** — each on its own direct user approval, a
> failure in one not blocking the other.

⚠️ **Status note:** EP-030 is recorded ✅ **Closed** in
[`Epic-EP-030.md`](../Epics/Closed/Epic-EP-030.md), and SP-094 is closed. This row is retained
because no Task-level verification record was written for T-0369 itself. **Resolve at the next audit
rather than assuming either way.**

### T-0390 — External Change Repair Matrix: world-package conditions

**Epic:** EP-031 (SP-100) · **Status:** 🔵 Backlog · Scheduled — SP-100 runs last.

### T-0391 — EP-031 verification + Epic close prep

**Epic:** EP-031 (SP-100) · **Status:** 🔵 Backlog · Scheduled — SP-100 runs last, and ⚠️ **owns the
AC1 re-verification.**

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
