# Verified Task — T-0606

**Sprint:** [SP-179] · **Epic:** [EP-026] (S5) · **Platform:** `[Linux]`
**Filed:** 2026-10-12 · **Implemented:** 2026-10-12 · ✅ **USER-VERIFIED 2026-10-13 by the rig live pass** (build 73, steps 13–23: every chord, the palette, the modifier-following button, the second project window and Edit ▸ Paste From Buffer passed)
**Archived:** 2026-10-15, with the Sprint close (*"close SP-179"*).

---

## T-0606 — `[Linux]` EP-026 S5: copy buffers 1–9, View ▸ Show Buffers (Ctrl+Alt+B) and the Buffers palette

**Epic:** [EP-026] (S5) · **Status:** ✅ **Verified 2026-10-13** (rig live pass, AC8, user) — archived with the Sprint close · **Sprint:** ✅ [SP-179] →
[`../../Sprints/Closed/Sprint-SP-179.md`](../../Sprints/Closed/Sprint-SP-179.md) ·
**Date Requested:** 2026-10-12 (user: *"start planning S5"*)

**What:** Apple's copy buffers on Linux, in Apple's shape (`BufferService.swift`, `BuffersPalette.swift`, the coordinator's
buffer actions): bridge `scrivi_buffers_*`; a per-project `BufferService` slot mirror; copy (Ctrl+digit, no undo step), paste
(Ctrl+Shift+digit, one step) and cut (Alt+digit, one step tagged with the slot) through S2's `HistoryCapture`; the app tracks
its front project; View ▸ Show Buffers (Ctrl+Alt+B) toggles one app-wide, non-activating palette with a modifier-following
button per slot. Same `history/buffers.json` as the Mac. Structured slots are S4's (Q1). Proven by Linux smokes through the
real key dispatch and a rig live pass.
