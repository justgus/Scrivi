# T-0213, T-0214 — Copy Buffers (EP-019, SP-056)

**Status:** ✅ **Verified 2026-07-27** (user-approved)
**Epic:** EP-019 — Undo/Redo History & Copy Buffers
**Sprint:** SP-056 — Copy buffers
**Record of truth:** [`Sprint-SP-056.md`](../../Sprints/Closed/Sprint-SP-056.md)
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §9 (Trade T3, T4)

---

## T-0213 — Copy-buffer store + C ABI + engine wrappers

**Priority:** High · ✅ **Verified 2026-07-27**

`history/buffers.json` (`scrivi.buffers.v1`), 9 numbered slots, `scrivi_buffers_list` /
`scrivi_buffers_set` / `scrivi_buffers_clear` plus the `ScriviEngine` wrappers (design §9).

## T-0214 — Buffer UX + history integration

**Priority:** High · ✅ **Verified 2026-07-27**

Keyboard HUD + toggleable palette + Edit/Scene/Chapter menu items with explicit ⌘/⌃/⌥1–9 chords
(Trade T4). Paste-from-buffer records a `paste` event; cut-into-buffer records a `cut` event;
**copy-into-buffer is deliberately not an event** (Trade T3, ✅ approved). The system pasteboard is
left untouched.

---

*Archived 2026-08-18 during the tracking-documentation audit. These rows had remained in
`Task-backlog.md` after SP-056 closed; the Sprint archive was and remains the authoritative record.*
