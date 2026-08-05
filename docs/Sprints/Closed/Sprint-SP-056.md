# SP-056: Undo/Redo — multiple copy buffers

**Status:** ✅ Closed (2026-07-27, user-approved)
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Goal:** Deliver vim/emacs-register-style **multiple copy buffers** whose pastes are history events —
**EP-019 AC6**.
**Start Date:** 2026-07-24
**End Date:** 2026-07-27
**Depends on:** SP-052–SP-055 (✅ closed) — `HistoryService`, C ABI, Swift capture/apply, persistence, branching.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §9 (copy buffers, CONOPS §9.a),
§10 T3/T4 (buffer trades).

> **⚠️ Archive reconstructed 2026-08-05.** This sprint closed 2026-07-27 (user-approved) but its archive file was
> never written — `Closed/` jumped SP-055 → SP-058, and the sprint's only surviving trace was a stale 🔵 Planning
> row in `Sprint-backlog.md`. This record is **reconstructed from the EP-019 entry in `Epic-active.md`** (AC6 and
> the T-0213/T-0214 rows), not from a contemporaneous sprint document. Outcomes and dates are taken from that
> record; the "What shipped" detail is necessarily less granular than sprints archived at close time. **No work
> was lost — only its sprint-level documentation.**

### Assigned Tasks

| ID | Title | Priority | Outcome |
| -- | ----- | -------- | ------- |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | High | ✅ Verified (2026-07-27) |
| T-0214 | Buffer UX: palette + Edit/Scene/Chapter menu items + explicit ⌘/⌃/⌥1–9 chords; paste/cut history integration | High | ✅ Verified (2026-07-27) |

### What shipped

- **Buffer store** — `history/buffers.json` (`scrivi.buffers.v1`), 9 numbered slots, C ABI + `ScriviEngine`
  wrappers; buffers persist across relaunch.
- **Explicit chords** — **⌘1–9 copy · ⌃1–9 paste · ⌥1–9 cut**, with **buffer 0 = the system pasteboard**.
- **Palette** (`Scrivi/Views/BuffersPalette.swift`) — app-global, reloaded per project.
- **Menu items** — Edit menu plus Scene/Chapter menus.
- **History integration** — paste-from-buffer records a `paste` event; cut-into-buffer records a `cut` event
  **tagged with its `bufferID`** (backend schema extended); copy-into-buffer is **not** an event (Trade T3).
- **System pasteboard unaffected** by buffer operations.

### Design refinement (user-approved 2026-07-25/27)

The originally-designed **single context-sensitive chord + ⌥⌘C/⌥⌘V HUD** was **replaced** by the explicit
⌘/⌃/⌥1–9 chord scheme above. This was a deliberate, user-approved deviation from
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §9, adopted during the sprint because the explicit
chords proved clearer in use than a modal HUD.

### Exit criteria — all met

- ✅ ≥ 2 buffers loadable and pasteable at multiple locations (design CONOPS §9.a).
- ✅ Each paste is exactly one undo step.
- ✅ System pasteboard unaffected.
- ✅ Buffers persist across relaunch.
- ✅ ScriviCore `ctest` 327 + interop 43 green.

### Outcome

**EP-019 AC6 ✅ Verified live 2026-07-27.** With AC6 closed, EP-019 retained only AC2/AC7/AC8 (verification) and
its final sprint SP-057. The Epic was then **held pending** at the user's direction so EP-029 `[Cross]`
(cross-boundary structured Cut/Copy/Paste — a gap surfaced while verifying this sprint) could be planned and
tackled first.

---

*Archived 2026-08-05 (reconstructed — see the notice above).*
