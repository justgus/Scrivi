# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-063 | `[Linux]` Scroll-driven scene switching + active-scene tracking | EP-022 | 🔵 Planning (next up) |
| SP-064 | `[Linux]` Cursor/focus + quit-reopen restore + EP-022 verify & close | EP-022 | 🔵 Planning |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning (parked) |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning (parked) |

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
> **SP-063/SP-064 drafted 2026-07-14** (🔵 Planning) — the rest of EP-022 after SP-062. **SP-063:** scroll-driven active-scene switching + navigator↔scroll sync (Apple SP-025 parity) → AC1/AC4 scroll portion. **SP-064:** cursor/focus + non-deletable separator + quit→reopen surface restore (Apple SP-033/EP-011 parity) + full EP-022 verification → **AC4/AC5/AC6/AC7**, **closes EP-022** (Claude drafts the completion summary; Epic close awaits user approval). Tasks T-0243–T-0248.
> **SP-062 activated 2026-07-14**, ✅ **closed 2026-07-14** — `Closed/Sprint-SP-062.md` (EP-022 `[Linux]` editable viewport + per-scene auto-save (debounce+switch+close/quit) + in-editor `Ctrl+Return`/`Ctrl+Shift+Return` scene/chapter creation; **AC3** delivered & user-verified over VNC; T-0238–T-0242 Verified. I-0061 Quit-button regression fixed+verified; I-0062 live chapter-label deferred to EP-023). Next EP-022 sprint is **SP-063** (scroll-driven switching).

---

## SP-063: [Linux] Scroll-driven scene switching + active-scene tracking

**Status:** 🔵 Planning
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (third of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — no ScriviCore change.
**Goal:** Make the **visible/caret scene** the "active" scene and keep **navigator selection ↔ viewport
scroll** in sync — the behavior Apple gave its own sprint (SP-025), kept separate here so it doesn't
destabilize the SP-062 write loop. Scrolling the continuous `QTextDocument` past a scene boundary promotes
the neighbor to active (saving the departing scene, reusing SP-062's save path); the navigator highlight
follows the scroll; a navigator click still scrolls the viewport (from SP-061) **without** moving the caret.
Delivers the scroll-driven portion of **AC1/AC4**.

**Parity target (Apple SP-025 + SP-033):** scroll-past-boundary promotes next/previous scene + saves the
departing scene; navigator highlight updates on scroll-driven switch; navigator tap scrolls to a scene
without moving the cursor; scroll-driven tracking is distinct from cursor position.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0243 | **Scroll → active-scene promotion** — map the viewport's visible region (via the offset map) to the active scene; crossing a boundary promotes next/previous and saves the departing scene (reuse SP-062 save) | High | 🔵 Backlog |
| T-0244 | **Navigator ↔ scroll sync** — navigator highlight follows scroll-driven active scene; navigator click scrolls the viewport without moving the caret; no feedback loop between the two | High | 🔵 Backlog |
| T-0245 | **Verify** — VNC: scroll through a multi-scene project → active scene + navigator highlight track the scroll; departing scenes saved; click-to-scroll leaves the caret put; CI green | High | 🔵 Backlog |

**Exit criteria:** scrolling past a boundary changes the active scene and saves the departing one; the
navigator highlight tracks scroll position; navigator click scrolls without moving the caret; no
scroll↔selection feedback loop. CI green. **AC1/AC4 (scroll portion) user-verified over VNC.**

---

## SP-064: [Linux] Cursor/focus behavior + quit-reopen restore + EP-022 verify & close

**Status:** 🔵 Planning
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (fourth of 4 sprints — **closes EP-022**)
**Codebase:** `[Linux]` (`platforms/linux/`) only — no ScriviCore change.
**Goal:** The EP-011-equivalent **cursor/focus polish** and **quit→reopen restore fidelity**, then the
full EP-022 verification pass and Epic close. On open, the editor takes focus and the caret lands sensibly;
crossing a scene boundary places the caret correctly (no jump-to-start); the scene-boundary separator is
non-editable/non-deletable. Closing and reopening the project (via recents) restores the **last active
scene + cursor + scroll** — proving the write→save→reopen loop end-to-end, using the `restored{anchor,
focus,scroll}` payload `scrivi_open_project` already returns and the `selectionAnchor/focus/scroll` args
`scrivi_save_scene` already persists. Delivers **AC4 + AC5 + AC6 + AC7** and **closes the Epic**.

**Parity target (Apple SP-033 / EP-011):** all-in-memory viewport cursor map, correct cursor placement
after navigate/switch (I-0010 fix), reliable first-responder transfer (Apple `takeFocus()` → Qt
`setFocus`/`activateWindow`), non-editable/non-deletable virtual separator, restored active scene + surface
state on relaunch.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0246 | **Cursor placement + focus** — editor takes focus on open; sensible initial caret; correct caret across boundary crossings (no jump-to-start); enforce non-editable/non-deletable boundary markers | High | 🔵 Backlog |
| T-0247 | **Quit→reopen surface restore** — persist active scene + selection anchor/focus + scroll on close (via `save_scene`); on reopen (recents), apply `openProject`'s `restored{anchor,focus,scroll}` to select/scroll/caret the last active scene | High | 🔵 Backlog |
| T-0248 | **EP-022 verification + close prep** — full write→save→switch→scroll→quit→reopen loop over VNC; real `.md` confirmed on disk / from macOS via the shared mount; AC1–AC7 checklist; extend the lifecycle/headless smoke for surface-state restore; CI green; draft EP-022 completion summary for user close approval | High | 🔵 Backlog |

**Exit criteria:** editor takes focus on open with a sensible caret; caret is correct across boundary
crossings; boundary markers are non-editable/non-deletable; closing + reopening (recents) restores the last
active scene with its cursor + scroll; the full write→save→switch→scroll→quit→reopen loop is VNC-verified
with real `.md` on disk; **all EP-022 ACs (AC1–AC7) pass user verification**; CI green (restore smoke +
`ctest`; macOS/EP-020/021 untouched). EP-022 completion summary drafted — **Epic close awaits user
approval** (Claude does not self-close Epics).

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

*Last Updated: 2026-07-14 (SP-060 ✅ closed; EP-021 ✅ closed. **All of EP-022 drafted** (🔵 Planning) —
SP-061 (shell flip + navigator + read-only viewport, T-0234–T-0237, AC1/AC2), SP-062 (editable +
per-scene auto-save + ⌘↩/⌘⇧↩ create, T-0238–T-0242, AC3), SP-063 (scroll-driven switching, T-0243–T-0245,
AC1/AC4 scroll), SP-064 (cursor/focus + restore + verify/close, T-0246–T-0248, AC4/AC5/AC6/AC7 — closes
EP-022). No ScriviCore work across the Epic. **None activated** — awaiting go-ahead. SP-056–SP-057 remain
🔵 Planning, parked behind deferred EP-019.)*
