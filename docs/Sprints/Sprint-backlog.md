# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning (parked) |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning (parked) |

> **SP-073 activated 2026-07-19** — moved to `Sprint-active.md` (EP-023 `[Linux]` chapter drag-reorder +
> Epic verify/close; tasks T-0294–T-0297).
> **SP-074 activated 2026-07-20, ✅ closed 2026-07-21** — `Closed/Sprint-SP-074.md` (EP-028 `[ScriviCore]`
> merge endpoints + I-0083 core fix; ctest macOS 317/317 + Linux 324/324). **SP-075 `[Apple]` activated
> 2026-07-21** — adopt the endpoints on macOS (T-0302/T-0303); moved to `Sprint-active.md`. SP-074/SP-075
> were defined in `Epics/Epic-active.md`, not staged here, so no Planning entry existed to remove.

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

## SP-056: Copy buffers — store, ABI, HUD/palette UX, history integration

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Multiple copy buffers usable for the two-buffer replacement-scan CONOPS (design §9.a); pastes are history events.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | High | 🔵 Backlog |
| T-0214 | Buffer UX: keyboard HUD + palette + Edit-menu items (T4); paste/cut history integration | High | 🔵 Backlog |

**Exit criteria:** CONOPS walkthrough verifiable; each buffer paste is one undo step; system pasteboard untouched; buffers persist across relaunch.

---

## SP-057: Undo/Redo — history panel, perf fixtures, verification & Epic close

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Management surface (Trade T2 option B), performance validation, and EP-019 acceptance-criteria verification.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0215 | History panel (T2 management surface) | Medium | 🔵 Backlog |
| T-0216 | Perf/integration fixtures (100k events, 500 KB scene); gitignore migration | Medium | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | High | 🔵 Backlog |

**Exit criteria:** all EP-019 ACs pass user verification; docs updated (`Scrivi_Project_Package_Structure`, repair matrix); Epic ready for close approval.

---

*Last Updated: 2026-07-19 (**SP-073 planned and activated same day** — EP-023's final sprint, renumbered
from the skipped SP-068: chapter drag-reorder (AC5) + full EP-023 verify (AC6–AC8) + Epic close prep; tasks
T-0294–T-0297; entry moved to `Sprint-active.md`. Path-refresh decision made at activation: additive reorder
envelope paths, `scrivi.h` untouched. SP-056–SP-057 remain 🔵 Planning, parked behind deferred EP-019
`[Apple]`.)*
