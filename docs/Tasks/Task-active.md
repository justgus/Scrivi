# Active Tasks

---

## T-0606 — `[Linux]` EP-026 S5: copy buffers 1–9, View ▸ Show Buffers (Ctrl+Alt+B) and the Buffers palette

**Epic:** [EP-026] (S5) · **Status:** 🟡 Active · **Sprint:** [SP-179] (🟡 Active 2026-10-12) → [`../Sprints/Sprint-SP-179.md`](../Sprints/Sprint-SP-179.md) ·
**Date Requested:** 2026-10-12 (user: *"start planning S5"*)

**What:** Apple's copy buffers on Linux, in Apple's shape (`BufferService.swift`, `BuffersPalette.swift`, the coordinator's
buffer actions): bridge `scrivi_buffers_*`; a per-project `BufferService` slot mirror; copy (Ctrl+digit, no undo step), paste
(Ctrl+Shift+digit, one step) and cut (Alt+digit, one step tagged with the slot) through S2's `HistoryCapture`; the app tracks
its front project; View ▸ Show Buffers (Ctrl+Alt+B) toggles one app-wide, non-activating palette with a modifier-following
button per slot. Same `history/buffers.json` as the Mac. Structured slots are S4's (Q1). Proven by Linux smokes through the
real key dispatch and a rig live pass.

**Also in this Task:** ✅ [I-0289] `[Apple]` — the Mac buffers made balanced FIRST (Q2 Option A), so Linux mirrors the corrected shape. ✅ VERIFIED 2026-10-12. · [I-0290] `[Apple]` — buffer paste moved ⌃1–9 → ⌃⌘1–9 (Mission Control took ⌃1). ✅ VERIFIED 2026-10-12. · [I-0291] `[Apple]` — the palette button follows ⌃⌘. ✅ VERIFIED 2026-10-12.

---

*Last Updated: 2026-10-12 (**T-0606 → active** with SP-179's activation.)*
