# Active Epics

## EP-030: [Apple] Scene Inspector Card Framework (base cards)

**Codebase:** `[Apple]` — the framework + writing-tool cards are SwiftUI; ScriviCore work is minimal (Doc 2 adds
no new C ABI beyond Doc 1's).
**Status:** 🔵 **Planned 2026-08-05** — ready to activate at SP-090.
**Goal:** Replace the placeholder `SceneInspectorView.swift` (57 lines: one tab case, a stub body, fixed 280pt)
with the approved card framework — bottom tabs (`Writing | Worldbuilding | Properties`), per-scene card stacks
persisted in `inspector-layout.json`, and the **writing-tool cards**. Folds in EP-019's history panel (T-0215) as
the `history` card, which is what lets **EP-019 close** without waiting for the object model.
**Design:** `docs/Scrivi_Scene_Inspector_Card_Framework_v0_1.md` ✅ **Approved 2026-08-05** (C1=C, C2=B, C3=A
within tabs, C4=A, C5=C + in-stack creation, C6=B with sort per-stack).
**Depends on:** nothing unbuilt. Deliberately sequenced *before* EP-031 so EP-019 can close first.
**Date Created:** 2026-08-05
**Target Close Date:** TBD (4 sprints)

### Acceptance Criteria

- [ ] AC1 — Bottom tabs render in order `Writing | Worldbuilding | Properties`; each stack tab is a scrolling
      column of collapsible cards. The one-card-per-tab layout is gone. (Doc 2 AC1–AC2)
- [ ] AC2 — Tab selection persists **at project level** and does **not** change when the scene changes; the whole
      tab view hides/shows from a menu item. (Doc 2 AC3–AC4)
- [ ] AC3 — Card stacks are per-scene, persisted in `inspector-layout.json`, with "apply to all scenes";
      Worldbuilding ships **empty**, Writing ships with empty `tags`/`outline`/`todo`. (Doc 2 AC6–AC9)
- [ ] AC4 — Sort is per-**stack**, not per-card; unknown `typeID` is skipped with a notice; one card's failure
      never blocks the stack (failed card shows a warning in place of content). (Doc 2 AC10–AC12)
- [ ] AC5 — The `history` card delivers T-0215's behavior (windowed tree, branch selection, stale badges, purge).
      (Doc 2 AC13)
- [ ] AC6 — The Properties tab renders author/timestamps/metrics, is **not** a card stack, and is built
      **field-driven** so a future editable property is a field-level addition. (Doc 2 AC14–AC15)
- [ ] AC7 — No regression: editor, scene navigation, timeline, undo/redo, external-change scan unchanged;
      `ctest` + interop suites green.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-090 | Framework: card protocol, registry, `inspector-layout.json`, tabs | 🔵 Planning | — |
| SP-091 | Writing-tool cards: tags, todo, outline, sources | 🔵 Planning | — |
| SP-092 | `history` card (folds in T-0215) + Properties tab | 🔵 Planning | — |
| SP-093 | Verification & Epic close | 🔵 Planning | — |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0359 | `InspectorCard` protocol + registry + `CardContext` | SP-090 | 🔵 Backlog |
| T-0360 | `inspector-layout.json` schema + load/save + unknown-`typeID` skip | SP-090 | 🔵 Backlog |
| T-0361 | Bottom tabs + persisted `selectedTab` + menu hide/show; replace `SceneInspectorView` | SP-090 | 🔵 Backlog |
| T-0362 | Card stack: add/remove/reorder, collapse, per-stack sort, "apply to all scenes" | SP-090 | 🔵 Backlog |
| T-0363 | `tags` + `todo` cards (scene sidecar) | SP-091 | 🔵 Backlog |
| T-0364 | `outline` card (scene sidecar) | SP-091 | 🔵 Backlog |
| T-0365 | `sources` card + `source` object kind (`objects/sources/`) | SP-091 | 🔵 Backlog |
| T-0366 | `history` card — windowed tree, branch selection, stale badges, purge (**supersedes T-0215**) | SP-092 | 🔵 Backlog |
| T-0367 | Properties tab — field-driven view, author/timestamps/metrics | SP-092 | 🔵 Backlog |
| T-0368 | Card failure isolation + inline warning presentation | SP-092 | 🔵 Backlog |
| T-0369 | EP-030 verification (AC1–AC7) + Epic close prep | SP-093 | 🔵 Backlog |

> **T-0215 relationship:** EP-019's history-panel task is **delivered by T-0366** as a card rather than a separate
> panel (Doc 2 §8). EP-019's SP-057 therefore no longer needs to build a panel — it verifies AC2/AC7/AC8, and the
> panel requirement is met here. Sequencing: **EP-030 SP-092 → EP-019 SP-057 close.**

---

## EP-031: [ScriviCore] Worldbuilding Object Model & Relationship Graph

**Codebase:** `[ScriviCore]` primarily (C++ model, index, graph, C ABI) with `[Apple]` object cards on top.
**Status:** 🔵 **Planned 2026-08-05** — blocked on EP-030 (needs the card framework) but its ScriviCore half can
start independently.
**Goal:** Implement the approved object model — new kinds, the object index, the canonical relationship graph,
world packages — then the worldbuilding-object cards on top of EP-030's framework.
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` ✅ **Approved 2026-08-05** (T1–T6 ruled) +
`docs/Scrivi_World_Data_Separation_v0_1.md` ✅ **Approved 2026-08-05** (W1–W6 ruled).
**Date Created:** 2026-08-05
**Target Close Date:** TBD (6 sprints)

### Acceptance Criteria

- [ ] AC1 — New kinds (`building`, `vehicle`, `artifact`, `map`, `chronicle`, `faction`, `world`, `source`)
      round-trip; legacy 5-kind files load unchanged; `timeline` kind retired. (Doc 1 AC1)
- [ ] AC2 — `objects/index.json` is built on open, updated atomically, and **rebuilt from a scan** when
      missing/stale/corrupt; `findByID` resolves via the index. (Doc 1 AC2–AC3)
- [ ] AC3 — **One canonical edge** per relationship, created from either endpoint, with the inverse as a
      read-time label projection. Duplicate rejection tested for **asymmetric and symmetric** (faction↔faction)
      types. (Doc 1 AC4–AC5)
- [ ] AC4 — Cascade-prune on delete; **orphans survive** and are findable; `objectID` preserved across
      `item`→`artifact` promotion with **zero edges rewritten**. (Doc 1 AC6–AC8)
- [ ] AC5 — `relationships.jsonl` compacts at **30% or 1,000 tombstones**, whichever first; torn final line
      detected and truncated. (Doc 1 AC9)
- [ ] AC6 — The **epoch chain** resolves (event → timeline → world → project); rebinding a world changes exactly
      one number; two timelines in one world relate without any project. (Doc 1 AC13–AC15)
- [ ] AC7 — ⚠️ **Absence is never deletion:** an unavailable world holds edges **pending** — never pruned, never
      modified, surviving save, restored on reattach. Status reports offline/unmounted/missing where
      determinable, else generic unavailable. (Doc 1 AC16–AC17, Doc 3 AC-A1–A7)
- [ ] AC8 — World packages: `worldID`-verified resolution, no search/registry, platform-neutral bindings,
      lock→write→unlock with stale-lock recovery. (Doc 3 AC-P1–P4, AC-L1–L5)
- [ ] AC9 — Worldbuilding-object cards on EP-030's framework: unfiltered picker, in-stack creation with no modal,
      "Remove from scene" deletes the edge only. (Doc 2 AC16–AC24)
- [ ] AC10 — No regression: `ctest` + interop suites green; existing projects open unchanged.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-094 | Object kinds + fields (`subtitle`/`image`/`worldID`) + object index | 🔵 Planning | — |
| SP-095 | Relationship graph: canonical edges, relation types, append-log, compaction | 🔵 Planning | — |
| SP-096 | Integrity: cascade-prune, orphans, promotion, pending-vs-dangling | 🔵 Planning | — |
| SP-097 | World packages: `.scrivworld`, bindings, resolution, locking, epoch chain | 🔵 Planning | — |
| SP-098 | Worldbuilding-object cards (Apple, on EP-030's framework) | 🔵 Planning | — |
| SP-099 | Verification & Epic close | 🔵 Planning | — |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0370 | `ObjectKind` additions + `objectKindSubdir` + schema table; retire `timeline` | SP-094 | 🔵 Backlog |
| T-0371 | `WorldObjectFields` extensions: `subtitle`, `image`, `worldID` | SP-094 | 🔵 Backlog |
| T-0372 | `objects/index.json` — build, atomic update, scan-rebuild; `findByID` over index | SP-094 | 🔵 Backlog |
| T-0373 | `relation-types.json` + `canonicalDirection` + `symmetric` | SP-095 | 🔵 Backlog |
| T-0374 | `relationships.jsonl` append-log: create/delete/list, tombstones, torn-line recovery | SP-095 | 🔵 Backlog |
| T-0375 | Canonical normalization + duplicate rejection (asymmetric **and** symmetric) | SP-095 | 🔵 Backlog |
| T-0376 | Compaction at 30% / 1,000 tombstones | SP-095 | 🔵 Backlog |
| T-0377 | Cascade-prune on delete + load-time repair | SP-096 | 🔵 Backlog |
| T-0378 | `scrivi_list_objects` / `scrivi_list_orphaned_objects` | SP-096 | 🔵 Backlog |
| T-0379 | `scrivi_promote_object` (item↔artifact), `objectID`-preserving | SP-096 | 🔵 Backlog |
| T-0380 | ⚠️ Pending-vs-dangling loader distinction + frozen graph toward unavailable worlds | SP-096 | 🔵 Backlog |
| T-0381 | `.scrivworld` package + `world.json` + world index | SP-097 | 🔵 Backlog |
| T-0382 | `binding.json` + `worldID`-verified resolution + relink | SP-097 | 🔵 Backlog |
| T-0383 | Lock→write→unlock + heartbeat + stale-lock recovery | SP-097 | 🔵 Backlog |
| T-0384 | Epoch chain: world/timeline/binding offsets + resolve endpoint | SP-097 | 🔵 Backlog |
| T-0385 | Cached world index entries → named pending entries | SP-097 | 🔵 Backlog |
| T-0386 | Object cards (one implementation, per-kind config) on EP-030's framework | SP-098 | 🔵 Backlog |
| T-0387 | Object picker (unfiltered, all worlds) + inline type-ahead + "Create new…" | SP-098 | 🔵 Backlog |
| T-0388 | In-stack create/edit, edit-state visuals, scene-change complete-or-discard | SP-098 | 🔵 Backlog |
| T-0389 | Pending presentation + Worlds menu + warning view under the timeline | SP-098 | 🔵 Backlog |
| T-0390 | External Change Repair Matrix — world-package conditions | SP-099 | 🔵 Backlog |
| T-0391 | EP-031 verification (AC1–AC10) + Epic close prep | SP-099 | 🔵 Backlog |

> **Highest-risk task: T-0380.** The pending-vs-dangling distinction is the one failure in this Epic that is
> *silent and unrecoverable* — a loader that reads "world unavailable" as "endpoint deleted" destroys every
> relationship into that world with no error shown. Both branches need explicit test coverage before any
> cascade-prune code ships.

---

## EP-019: [Apple] Custom Undo/Redo History & Multiple Copy Buffers — RESUMED

> **Un-deferred 2026-07-24 (user request):** moved back from the backlog to Active to **lock the copy-buffer
> implementation on Apple first**, so it becomes the reference the Linux side (EP-026) mirrors rather than
> being designed twice. Resumes at **SP-056** (multiple copy buffers → AC6). The undo/redo *engine* (C++
> `HistoryService` + C ABI + Swift wrappers) is delivered & verified (AC1/AC3/AC4/AC5); remaining:
> **AC6 (copy buffers, SP-056)**, then AC2/AC7/AC8 (SP-057 verify + close).

---

> **Codebase:** `[Apple]` — the undo/redo engine (ScriviCore `HistoryService` + C ABI) is delivered &
> verified; all remaining work (AC2/AC6/AC7/AC8) is Apple-platform UI. The parallel Linux undo/redo UI
> is a separate `[Linux]` Epic.

**Status:** 🟡 Active — **HELD PENDING** its final sprint SP-057. SP-051–SP-056 ✅ all closed: AC1 (2026-07-07),
AC3+AC5 (2026-07-09), AC4+AC5-branch (2026-07-13), **AC6 (2026-07-27, SP-056 — copy buffers)**.
**Scope reduced 2026-08-05:** the history panel moved to EP-030 (T-0366), and an audit found **AC2/AC7 already
implemented** with both test suites green — so **SP-057 is now a verification sprint** (live verify AC2/AC7/AC8 +
perf fixtures T-0216 + close), not a build sprint. See the close-out assessment below. EP-029 `[Cross]` (the gap
surfaced while verifying SP-056) was tackled first and ✅ closed 2026-08-03.
**Goal:** Replace the broken native undo (I-0019) with a from-scratch, sentence-granular undo/redo system backed by a tree-structured, per-project, on-disk persistent history (cross-session undo with session-boundary warning; branching with primary-line selection; capacity eviction and stale-branch purge), plus vim/emacs-register-style multiple copy buffers whose pastes are history events.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (v0.1 ✅ Approved baseline 2026-07-06 — trades ruled: T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A)
**Supersedes:** I-0019 (Undo/Redo have no effect — ⚪ Closed 2026-07-06, OBE/superseded by this Epic, user-approved; see `docs/Issues/Closed/Issue-closed-0019.md`. AC1 carries the requirement; delivery target SP-053. Re-open I-0019 if EP-019 is cancelled or AC1 descoped.)
**Date Created:** 2026-07-06
**Target Close Date:** TBD (7 sprints)
**Actual Close Date:** —

> ### 📋 Close-out assessment (2026-08-05) — **no implementation work remains; SP-057 is a verification sprint**
>
> Prompted by the Doc 1–3 approvals, EP-019's remaining scope was audited against the code and test suites:
>
> | AC | Implementation | What actually remains |
> | --- | --- | --- |
> | **AC2** (event model) | ✅ **Built.** Commit triggers `sentence`/`paste`/`cut`/`sceneSwitch`/`cursorMove`/`flush` in `ManuscriptTextView.swift:638-802`; the "no event for cursor-move/whitespace-only" rule is implemented as the `soft`-trigger whitespace-delta path (`HistoryCapture.swift:185-194`). | **Live verification only** |
> | **AC7** (barriers) | ✅ **Built.** `recordBarrier` on structural ops (`ManuscriptTextView.swift:645,862-863`); undo stops with a notice (`:246-254`, `presentBarrierNotice` `:390`). | **Live verification only** |
> | **AC8** (no regression) | ✅ **Suites green 2026-08-05**: ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **TEST SUCCEEDED**. | **Live verification** of auto-save / navigation / structure ops / external-change scan / Git snapshots |
> | ~~history panel~~ | ⚪ **Superseded** → EP-030 **T-0366** (ships as the `history` card, approved Doc 2 §8) | — |
> | **T-0216** (perf fixtures) | 🔵 Not built — 100k-event history + 500 KB scene fixtures | Build + run |
> | **T-0216** (`.gitignore` migration) | ⚠️ **Likely OBE** — see note below | Confirm, then drop or keep |
>
> **Conclusion: EP-019 cannot be closed today, but not because code is missing.** AC2/AC7 are implemented and
> AC8's mechanical half passes; what is outstanding is **live user verification** (the three ACs were never
> exercised end-to-end in front of the user) plus the T-0216 perf fixtures. Per CLAUDE.md, Claude cannot mark
> these Verified — that requires direct user confirmation. SP-057 is therefore a **verification sprint**, and it
> is materially smaller than its original scope now that the history panel has moved to EP-030.
>
> **Two bookkeeping corrections made the same day:** `Closed/Sprint-SP-056.md` was written (the sprint closed
> 2026-07-27 but was never archived), and T-0213/T-0214 were corrected from 🔵 Backlog to ✅ Verified in
> `Task-backlog.md`.
>
> **⚠️ `.gitignore` migration (T-0216) — needs a ruling.** It was scoped to add `history/` to the `.gitignore`
> of **existing projects**. Scrivi has not shipped (the same fact that deleted Doc 3's migration work), so there
> are no existing projects in the field. This is probably obsolete; the only reason to keep it is if developer
> fixtures or the user's own test projects predate the `history/` entry and would otherwise commit history noise.
>
> **Recommended SP-057 scope:** AC2 live verify · AC7 live verify · AC8 live regression sweep · T-0216 perf
> fixtures · T-0217 doc updates + Epic close. Sequence it **after EP-030 SP-092** so T-0366 (the history card)
> exists when the Epic closes.

### Acceptance Criteria

- [x] AC1 — ⌘Z/⇧⌘Z work in the macOS manuscript editor: repeated ⌘Z walks back one history event at a time; ⇧⌘Z re-applies (**delivers the fix formerly tracked as I-0019**). ✅ **Verified live 2026-07-07** (SP-053; `Tasks/Verified/Task-verified-0204-0206.md`).
- [ ] AC2 — Events commit exactly per the design's event model (`.` `!` `?`, Return, cursor-move-with-pending-changes, paste/cut, scene switch, flush); cursor moves/newlines without text changes produce **no** event. 🔧 **Implemented (audited 2026-08-05)** — `ManuscriptTextView.swift:638-802` + `HistoryCapture.swift:185-194`; **awaiting live verification** (SP-057).
- [x] AC3 — History persists across quit/relaunch; undoing past the session boundary shows a warning (once per crossing) before proceeding. ✅ **Verified 2026-07-09** (SP-054; `Tasks/Verified/Task-verified-0207-0209.md`).
- [x] AC4 — Undo-then-type creates a branch; the new line becomes primary; the old branch is selectable at the fork and becomes primary when selected; abandoned text fully restorable. ✅ **Verified 2026-07-13** (SP-055; `Tasks/Verified/Task-verified-0210-0212.md`).
- [x] AC5 — History capacity configurable (per Trade T1); oldest events fall off at capacity; branches auto-purge when their branch point ages off; stale branches detectable and purgeable with user confirmation. ✅ **Verified 2026-07-09** (SP-054 — capacity config + linear eviction of the root→current path; `Tasks/Verified/Task-verified-0207-0209.md`). ✅ **Branch clauses verified 2026-07-13** (SP-055 — branch-aware auto-purge on eviction + stale-branch detection/user-confirmed purge; `Tasks/Verified/Task-verified-0210-0212.md`).
- [x] AC6 — Copy buffers: ≥ 2 buffers loadable and pasteable at multiple locations (design CONOPS §9.a); each paste is one undo step; system pasteboard unaffected; buffers persist across relaunch. ✅ **Verified live 2026-07-27** (SP-056; T-0213 + T-0214). Delivered as explicit chords **⌘1–9 copy / ⌃1–9 paste / ⌥1–9 cut** (design refinement, user-approved 2026-07-25/27 — the originally-designed single context-sensitive chord + ⌥⌘C/⌥⌘V HUD was replaced; buffer 0 = the system pasteboard). Palette (app-global, per-project reload), Edit-menu + Scene/Chapter menu items, cut-into-buffer `cut` event tagged with bufferID (backend schema extended).
- [ ] AC7 — Structural operations record barriers; undo stops at a barrier with a clear notice; no text corruption. 🔧 **Implemented (audited 2026-08-05)** — `ManuscriptTextView.swift:645,862-863` (record) + `:246-254,390` (notice); **awaiting live verification** (SP-057).
- [ ] AC8 — No regression: auto-save, scene navigation, structure ops, external-change scan, Git snapshots unchanged; backend `ctest` + interop suites green. 🔧 **Suites green 2026-08-05** — `ctest` **369/369**, interop **45/45**, app TEST SUCCEEDED; **live regression sweep awaiting verification** (SP-057).

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-051 | Design sign-off, ⌘Z-routing spike, schema spec | ✅ Closed (user-approved) | 2026-07-06 – 2026-07-06 |
| SP-052 | Linear history engine core (C++) + C ABI + Swift wrappers | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-053 | In-session undo/redo on macOS (capture, apply, barriers) — AC1 (ex-I-0019) delivered here | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-054 | Persistence, sessions, capacity, settings | ✅ Closed (user-approved) — AC3 + AC5 | 2026-07-07 – 2026-07-09 |
| SP-055 | Branching — tree ops, fork popover, purge | ✅ Closed (user-approved) — AC4 + AC5 branch clauses | 2026-07-10 – 2026-07-13 |
| SP-056 | Multiple copy buffers | ✅ Closed (user-approved) — **AC6** | 2026-07-24 – 2026-07-27 |
| SP-057 | **Verification sprint** — AC2/AC7/AC8 live verify + perf fixtures (T-0216) + Epic close (**history panel moved to EP-030 T-0366**) | 🔵 Planning — scope reduced 2026-08-05; run **after EP-030 SP-092** | — |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0198 | Design doc + trade studies (this Epic's design) | SP-051 | ✅ Verified (2026-07-06) → `Verified/Task-verified-0198.md` |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | SP-051 | ✅ Done (2026-07-06) — proxy rejected; action-method mechanism confirmed live; design §8/§12.6 updated |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | SP-051 | ✅ Verified (2026-07-06) → `Verified/Task-verified-0200.md` |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + unit tests | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0202 | C ABI: `scrivi_history_open/record_event/record_barrier/undo/redo/close` | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0206 | Barriers on structural operations | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | SP-054 | ✅ Verified (2026-07-09) |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | SP-054 | ✅ Verified (2026-07-09; linear eviction, full in SP-055) |
| T-0209 | Session-boundary warning popup | SP-054 | ✅ Verified (2026-07-09) |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (T2 core interaction) | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | SP-056 | ✅ **Verified (2026-07-27)** |
| T-0214 | Buffer UX: palette + Edit/Scene/Chapter menu items + explicit ⌘/⌃/⌥1–9 chords; paste/cut history integration (cut tagged w/ bufferID) | SP-056 | ✅ **Verified (2026-07-27)** |
| T-0215 | History panel (T2 management surface) | SP-057 | ⚪ **Superseded 2026-08-05 → EP-030 T-0366** (ships as the `history` **card** in the Writing stack, per approved Doc 2 §8) |
| T-0216 | Performance/integration fixtures (100k events, 500 KB scene); gitignore migration | SP-057 | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | SP-057 | 🔵 Backlog |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0019 | Undo and Redo have no effect in the manuscript editor | ⚪ Closed 2026-07-06 — OBE/superseded by this Epic (user-approved); requirement lives on as AC1 → `docs/Issues/Closed/Issue-closed-0019.md` |

### Scope Notes

- Structural undo (scene/chapter create/delete/merge/reorder) is explicitly **out of scope** — those operations record history *barriers* in v1 (design §4.5); full structural undo is a documented future extension.
- iOS/iPadOS capture layer deferred (editor is a stub); the C++ engine and ABI are platform-neutral by construction.
- All six trade-study decisions **approved by the user 2026-07-06**: T1=B (capacity in Project
  settings), T2=A with interaction refinements (popover on undo-landing-at-fork, suppressed when
  undoing past, immediate on redo, redo-past takes primary, branch relegation — design §10 T2, §5),
  T3=C (copy-into-buffer not an event; cut is), T4=B+D with A for discoverability, T5=C
  (project-open session + idle rollover), T6=A (history inside package, gitignored).

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_


---

*Last Updated: 2026-08-03 (**EP-029 ✅ CLOSED (Human-approved) — cross-boundary structured Cut/Copy/Paste
complete.** Final sprint **SP-089** delivered all four `[Apple]` tasks: T-0354 (Pass A clipboard, Verified
2026-07-29), T-0357 (title-capture + chapter promotion, Verified 2026-07-29), T-0355 (Pass B structured buffers,
Verified 2026-08-03), T-0356 (AC6 reversible structured undo, Verified 2026-08-03). AC1–AC7 all met; the AC6 undo
runs **app-side** (`HistoryService` stays in-memory; new core endpoint `scrivi_fragment_uncut_paste` is the exact
inverse of paste). ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **BUILD SUCCEEDED**. **EP-029
archived to `Closed/Epic-EP-029.md`; SP-089 archived to `Sprints/Closed/Sprint-SP-089.md`.** Active Epics: EP-019
(held pending SP-057). Prior note follows.)*

*2026-07-27 (**SP-085 ✅ closed (Human-approved) — EP-029 design doc APPROVED.** The EP-029
`[Cross]` design sprint delivered + got approval for `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`: the
`scrivi.fragment.v1` ordered-pieces schema, extract / paste-splice / cut-merge behaviour (composing EP-027
create/split + EP-028 `SceneMerger`/`ChapterMerger`), buffer-schema evolution (extend `scrivi.buffers.v1` in
place), and a one-reversible-event history shape (`structuredCut`/`structuredPaste`, undo = inverse op). Trades
ruled **T1=A · T2=A · T3=A · T4=A**; Open Questions #1–#3 resolved — no cross-window/cross-project structured
paste in v1; **caret-in-heading paste = refuse + flash the screen** (user override of the drafted silent-retarget);
divider-anchored selection normalised to the adjacent scene body. Task T-0350 Verified; docs-only. **EP-029 → SP-086**
(ScriviCore extract-fragment) next, awaiting go-ahead. Epics Active 1 (EP-029) + held/draft (EP-019 held, EP-026
Linux draft). Next available Sprint **SP-086**, Task **T-0351**. Prior note follows.)*

*2026-07-27 (**SP-056 ✅ closed (Human-approved) — AC6 Verified; EP-019 held pending SP-057; new
Epic EP-029 opened.** T-0213 + T-0214 both ✅ Verified live: multiple copy buffers delivered as explicit
⌘1–9/⌃1–9/⌥1–9 (copy/paste/cut) chords + app-global per-project palette + Edit/Scene/Chapter menu items +
bufferID-tagged cut event (backend schema extended); ctest 327 + interop 43 green. AC6 met. EP-019 → **held
pending** its final sprint SP-057 (AC2/AC7/AC8 verify + history panel + close). **EP-029** `[Cross]`
(cross-boundary structured Cut/Copy/Paste) opened to capture the gap surfaced during SP-056 verification —
manuscript-as-monolithic-document copy/cut/paste with structured buffers, cut-that-merges, paste-that-splits;
5 sprints planned (SP-085 design → SP-086 extract → SP-087 paste-splice → SP-088 cut-merge → SP-089 Apple wiring);
design doc to be written & approved first (T-0350). Epics Active 1 (EP-019, held) + Draft 2 (EP-026 Linux, EP-029);
next available Epic EP-030, Sprint SP-085, Task T-0351. Prior note follows.)*

---

*Last Updated: 2026-07-24 (**EP-025 ✅ CLOSED (Human-approved) — the final sprint SP-084 delivered AC6b and
closed the Epic.** SP-084 (co-located dot **clustering** — aggregate dot: larger core + count + segmented arc
ring + selection arc; hover fan-out; zoom-resolve) + a persistence sweep + full EP-025 verify; T-0346–T-0349 all
Verified live over VNC. Four clustering findings surfaced + fixed + re-verified same session (larger-diameter
aggregate-of-aggregates; fan overlay + grey backing; tighter dismiss; the **phantom double-draw** defect — a
fanned aggregate's members drew twice because the skip-set excluded the fanned aggregate; fixed so all members
are skipped from the baseline loops and the ring is the sole draw). Final container build **green (216/216)** +
new `timeline_cluster_smoke` + 14 regression smokes + app-launch PASS; `scrivi.h` untouched; no pbxproj
(Linux-only). **EP-025 archived to `Closed/Epic-EP-025.md`; Active Epics 1→0.** The full six-sprint completion
summary lives in the archive. Prior note follows.)*

*2026-07-24 (**EP-025 SP-082 ✅ closed (Human-approved) + SP-084 planned + activated** — the final sprint of
EP-025. SP-082 delivered **AC5** (historical events + imported timelines + export + File-menu items,
T-0340–T-0345 all Verified; I-0090/I-0091 fixed + re-verified; container green 211/211 + `timeline_events_smoke`
+ 11 smokes PASS; archived `Closed/Sprint-SP-082.md`). SP-084 then delivered **AC6b** clustering (above) and
closed the Epic. Prior EP-025 history — SP-079 [AC1/AC2], SP-080 [AC3], SP-081 [AC4], SP-083 [AC6a] — lives in
`Closed/Epic-EP-025.md`.)*
