# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-057 | Undo/Redo — perf fixtures, verification & Epic close (**panel now EP-030 T-0366**) | EP-019 | 🔵 Planning (parked) |
| SP-092 | `[Cross]` `history` card + Properties tab + `scrivi_history_get_tree` | EP-030 | 🔵 **Planning (drafted)** → `Sprint-active.md` |
| SP-093 | EP-030 verification & Epic close | EP-030 | 🔵 Planning |
| SP-094 | Object kinds + fields + object index | EP-031 | 🔵 Planning |
| SP-095 | Relationship graph: canonical edges, append-log, compaction | EP-031 | 🔵 Planning |
| SP-096 | Integrity: prune, orphans, promotion, pending-vs-dangling | EP-031 | 🔵 Planning |
| SP-097 | World packages: bindings, resolution, locking, epoch chain | EP-031 | 🔵 Planning |
| SP-098 | Worldbuilding-object cards | EP-031 | 🔵 Planning |
| SP-099 | EP-031 verification & Epic close | EP-031 | 🔵 Planning |

> ✅ **SP-056 archive written 2026-08-05** — `Closed/Sprint-SP-056.md`, reconstructed from the EP-019 record
> (the sprint closed 2026-07-27 user-approved but was never archived; `Closed/` had jumped SP-055 → SP-058). The
> archive is marked as reconstructed. Its tasks T-0213/T-0214 were also still showing 🔵 Backlog in
> `Task-backlog.md` despite being Verified — corrected the same day.

> ✅ **SP-091 closed 2026-08-05 (Human-approved)** — `Closed/Sprint-SP-091.md`. Writing-tool cards + the
> `scrivi.scene.v1` extension; T-0392/T-0393/T-0363/T-0364 Verified, I-0101 fixed in-sprint. **SP-092 is next.**

> ✅ **SP-090 closed 2026-08-05 (Human-approved)** — `Closed/Sprint-SP-090.md`. EP-030's card framework
> shipped; T-0359–T-0362 all Verified. Planning row removed. **SP-091 is next.**

> **Sequencing (2026-08-05):** EP-030 SP-090→SP-093, then **EP-019 SP-057** (its history-panel requirement is
> met by EP-030 T-0366), then EP-031 SP-094→SP-099. EP-031's ScriviCore sprints (SP-094–SP-097) have no
> dependency on EP-030 and may run in parallel if capacity allows; only SP-098 needs the card framework.

> **SP-078 activated & ✅ closed 2026-07-22** — `Closed/Sprint-SP-078.md` (EP-024 `[Linux]` Scene Inspector
> Panel, first & only sprint; tasks T-0318–T-0320 Verified live over VNC). **This closed EP-024** in one sprint,
> mirroring Apple EP-014/SP-037. SP-078 was defined in the Epic (not staged here), so no Planning entry existed
> to remove.

> **SP-073 activated 2026-07-19** — moved to `Sprint-active.md` (EP-023 `[Linux]` chapter drag-reorder +
> Epic verify/close; tasks T-0294–T-0297).
> **SP-074 activated 2026-07-20, ✅ closed 2026-07-21** — `Closed/Sprint-SP-074.md` (EP-028 `[ScriviCore]`
> merge endpoints + I-0083 core fix; ctest macOS 317/317 + Linux 324/324). **SP-075 `[Apple]` activated
> 2026-07-21, ✅ closed 2026-07-21** — `Closed/Sprint-SP-075.md` (adopt the endpoints on macOS; T-0302/T-0303).
> **SP-076 `[Linux]` ✅ closed 2026-07-22** — `Closed/Sprint-SP-076.md` (scene & chapter merge parity;
> T-0304–T-0309). **SP-077 `[Linux]` ✅ closed 2026-07-22** — `Closed/Sprint-SP-077.md` (native menu bar;
> T-0310–T-0317). **This closed EP-028.** SP-074–SP-077 were all defined in the Epic, not staged here, so no
> Planning entry existed to remove.

Design reference for all: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06).
Epic-level ACs: `docs/Epics/Epic-active.md` (EP-019). Task detail: `docs/Tasks/Task-backlog.md` (T-0198–T-0217).

> **SP-051 activated 2026-07-06** (design signed off at activation) — moved to `Sprint-active.md`.
> **SP-052 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-052.md`.
> **SP-053 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-053.md` (EP-019 AC1 delivered & verified).
> **SP-054 activated 2026-07-07**, ✅ **closed 2026-07-09** — `Closed/Sprint-SP-054.md` (EP-019 AC3 + AC5 delivered & verified).
> **SP-055 activated 2026-07-10**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-055.md` (EP-019 AC4 + AC5's deferred branch clauses delivered & verified).
> **SP-056/SP-057 parked 2026-07-13** — EP-019 `[Apple]` was deferred to the Epic backlog; these sprints remain 🔵 Planning but are **not** next-up. The active line of work is the `[Linux]` app family (EP-020+).
> **SP-058 activated 2026-07-13**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-058.md` (EP-020 `[Linux]` App Foundation delivered & verified; EP-020 → Complete). Next `[Linux]` sprint is defined when EP-021 is drafted.
> **SP-059 activated 2026-07-13**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-059.md` (EP-021 `[Linux]` Project Lifecycle create-half: appSupportRoot + recents + landing + create-project; AC1/AC2/AC4/AC5 delivered & user-verified over VNC; T-0229 `EncryptedFileSecureStore` added mid-sprint).
> **SP-060 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-060.md` (EP-021 `[Linux]` open/close half: Open Project + 3 open modes + close→landing + full-loop smoke; AC3/AC6/AC7/AC8 delivered & VNC-verified). **This closed EP-021.** Next `[Linux]` Epic **EP-022** (Writing Surface) is Active; its first sprint (SP-061) is drafted at planning.
> **SP-061 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-061.md` (EP-022 `[Linux]` shell flip to Widgets host + scene navigator + read-only continuous viewport; AC1 + AC2 delivered & user-verified over VNC; `QPlainTextDocumentLayout` render bug found in click-through and fixed).
> **SP-062 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-062.md` (EP-022 `[Linux]` editable viewport + per-scene auto-save (debounce+switch+close/quit) + in-editor `Ctrl+Return`/`Ctrl+Shift+Return` scene/chapter creation; **AC3** delivered & user-verified over VNC; T-0238–T-0242 Verified. I-0061 Quit-button regression fixed+verified; I-0062 live chapter-label deferred to EP-023). Next EP-022 sprint is **SP-063** (scroll-driven switching).
> **SP-063 activated 2026-07-15**, ✅ **closed 2026-07-15** — `Closed/Sprint-SP-063.md` (EP-022 `[Linux]` scroll-driven scene switching + navigator↔scroll sync; **AC1/AC4 scroll portion** delivered & user-verified over VNC; T-0243–T-0245 Verified. Navigator-click requirement reversed mid-sprint: click takes the **caret to the clicked scene's start** (not caret-free) per user. T-0249 logged unscheduled — Page Fwd/Back + jump to manuscript start/end, bindings TBD). Next EP-022 sprint **SP-064** (cursor/focus + restore + verify, closes the Epic).
> **SP-064 activated 2026-07-15**, ✅ **closed 2026-07-15** — `Closed/Sprint-SP-064.md` (EP-022 `[Linux]` cursor/focus polish (focus-on-open, caret normalization out of boundary gaps, non-deletable separator) + quit→reopen **surface restore** (last active scene + cursor + scroll via the `restored{}` payload + `save_scene` selection/scroll args — filled the 0/0/0.0 stub) + full EP-022 verify; **AC4-cursor/AC5/AC6/AC7** delivered & user-verified over VNC; T-0246–T-0248 Verified; no ScriviCore work, `scrivi.h` untouched). **This closed EP-022.** Next `[Linux]` Epic is **EP-023** (structure editing), drafted when the user activates it.

---

## SP-056 — ✅ CLOSED 2026-07-27 (stale Planning block removed 2026-08-05)

Its full record lives in `Closed/Sprint-SP-056.md` (archived 2026-08-05, reconstructed). T-0213/T-0214 are
✅ Verified. The Planning block that stood here still listed both tasks as 🔵 Backlog four months after they
shipped — removed rather than left to mislead.

---

## SP-057: Undo/Redo — verification & Epic close (**rescoped 2026-08-05 — no build work**)

**Status:** 🔵 Planning (parked — run **after EP-030 SP-092**)
**Epic:** EP-019
**Goal:** EP-019 acceptance-criteria verification and Epic close. **Rescoped 2026-08-05:** the history panel
moved to EP-030 T-0366 (ships as the `history` card) and T-0216 was closed OBE, leaving **no implementation
work** — AC2/AC7 were audited as already implemented and both suites are green; what remains is live user
verification.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0215 | History panel (T2 management surface) | Medium | ⚪ **Superseded → EP-030 T-0366** |
| T-0216 | Perf/integration fixtures (100k events, 500 KB scene); gitignore migration | Medium | ⚪ **Closed — OBE (2026-08-05)** |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | High | 🔵 Backlog |

**Exit criteria:** AC2/AC7/AC8 pass **live** user verification; docs updated (`Scrivi_Project_Package_Structure`,
repair matrix); Epic ready for close approval. **Sequence after EP-030 SP-092** so T-0366 exists at close.

---

## SP-091: `[Apple]` EP-030 — writing-tool cards

**Status:** 🔵 Planning
**Epic:** EP-030
**Goal:** The first real cards on SP-090's framework — `tags`, `todo`, `outline`, `sources`.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0363 | `tags` + `todo` cards (scene sidecar) | High | 🔵 Backlog |
| T-0364 | `outline` card (scene sidecar) | High | 🔵 Backlog |
| T-0365 | `sources` card + `source` object kind (`objects/sources/`) | Medium | 🔵 Backlog |

**Exit criteria:** the Writing tab's default stack (`tags`/`outline`/`todo`) renders real cards, empty and
dismissible, with the writer's choices persisting (Doc 2 AC8). `source` is a real `objects/` file — citations
are reusable across scenes (Doc 1 §3).

> **Note:** T-0365 adds the **first new object kind**, which overlaps EP-031 SP-094's `ObjectKind` work
> (T-0370). Coordinate: either land `source` here and let SP-094 extend, or defer T-0365 into SP-094. Decide at
> SP-091 planning.

---

## SP-092: `[Apple]` EP-030 — history card + Properties tab

**Status:** 🔵 Planning
**Epic:** EP-030
**Goal:** The `history` card (absorbing EP-019's T-0215) and the Properties tab.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0366 | `history` card — windowed tree, branch selection, stale badges, purge (**supersedes T-0215**) | High | 🔵 Backlog |
| T-0367 | Properties tab — field-driven view, author/timestamps/metrics | Medium | 🔵 Backlog |
| T-0368 | Card failure isolation + inline warning presentation | Medium | 🔵 Backlog |

**Exit criteria:** Doc 2 AC12–AC15. **Properties must be built field-driven** — each field declares its own
editability — so a future editable property is a field-level addition, not a rewrite (Doc 2 §4.8.1).

> **Unblocks EP-019:** once T-0366 lands, EP-019's history-panel requirement is met and SP-057 can run.

---

## ~~SP-093: `[Apple]` EP-030 — verification & Epic close~~ ⚪ SUPERSEDED (2026-08-07)

**Status:** ⚪ **Superseded — do not activate.** Two changes, both user-ruled 2026-08-07:

1. **SP-093 is now the EP-019 history-capture sprint** (granularity + presentation), opened from the SP-092
   live-verify — see `Sprint-active.md`.
2. **T-0369 merges into SP-094**, a combined **"EP-019 + EP-030 verification & Epic close"** sprint. This
   entry and EP-019's SP-057 were both one-task, no-build-work verification passes gated on the same live
   session and app build; running them as two consecutive sprints was ceremony. Both Epics' ACs are verified
   in one pass, then **closed independently** — each on its own direct user approval.

**Epic:** EP-030 · **Task:** T-0369 (now SP-094) · **Exit criteria** carried forward to SP-094: all EP-030
ACs pass user verification; `ctest` + interop green; Epic ready for close approval (user-only).

---

*Last Updated: 2026-07-22 (**SP-078 planned, activated, and implemented same day** — opened EP-024 `[Linux]`
Scene Inspector Panel (mirrors Apple EP-014); moved to `Sprint-active.md`. Default visibility = **shown**
(Apple parity, user decision). Container build green (184/184, 0 warnings) + all Linux smokes PASS; live VNC
verify + AC sign-off pending. SP-056–SP-057 remain 🔵 Planning, parked behind deferred EP-019 `[Apple]`.)*
