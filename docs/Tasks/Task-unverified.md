# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0607 — `[Linux]` EP-026 S4: structured cut / copy / paste across scenes

**Epic:** [EP-026] (S4) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-13) — ⏳ rig live pass (AC9) · **Sprint:** [SP-180] (🟡 Active 2026-10-13) → [`../Sprints/Sprint-SP-180.md`](../Sprints/Sprint-SP-180.md) ·
**Date Requested:** 2026-10-13 (user: *"start planning EP-026 S4"*)

**What:** Apple's structured cut/copy/paste on Linux, in its current shape (a cut keeps the scenes, [I-0270]).
- A cross-scene Ctrl+C / Ctrl+X holds a balanced fragment, in the editor and tied to its own clipboard write (Q1).
- Ctrl+V rebuilds the scenes and chapters at the caret as ONE reversible structural step. A selection is replaced (Q2), and the flat text is Apple's (Q4).
- The copy buffers hold fragments.
- Structural history nodes replay on either platform.
- It replaces [SP-178] Q2's and [SP-179] Q1's stopgaps.
- Proven by Linux smokes through the real key dispatch and a rig live pass.

**Also in this Task (all Resolved - Not Verified):** [I-0293] `[ScriviCore]` + Apple, the history texts on both sides of a structured paste · [I-0294] `[Apple]`, unsaved typing saved first · [I-0295] `[Apple]`, a selection is replaced (Q2) · [I-0296] `[ScriviCore]`, the core's own writes are recorded, so a reopen flags nothing.

---

## 🟠 T-0606 — `[Linux]` EP-026 S5: copy buffers 1–9, View ▸ Show Buffers (Ctrl+Alt+B) and the Buffers palette

**Epic:** [EP-026] (S5) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-12) — ⏳ rig live pass (AC8) · **Sprint:** [SP-179] (🟡 Active 2026-10-12) → [`../Sprints/Sprint-SP-179.md`](../Sprints/Sprint-SP-179.md) ·
**Date Requested:** 2026-10-12 (user: *"start planning S5"*)

**What:** Apple's copy buffers on Linux, in Apple's shape (`BufferService.swift`, `BuffersPalette.swift`, the coordinator's
buffer actions): bridge `scrivi_buffers_*`; a per-project `BufferService` slot mirror; copy (Ctrl+digit, no undo step), paste
(Ctrl+Shift+digit, one step) and cut (Alt+digit, one step tagged with the slot) through S2's `HistoryCapture`; the app tracks
its front project; View ▸ Show Buffers (Ctrl+Alt+B) toggles one app-wide, non-activating palette with a modifier-following
button per slot. Same `history/buffers.json` as the Mac. Structured slots are S4's (Q1). Proven by Linux smokes through the
real key dispatch and a rig live pass.

**Also in this Task:** ✅ [I-0289] `[Apple]` — the Mac buffers made balanced FIRST (Q2 Option A), so Linux mirrors the corrected shape. ✅ VERIFIED 2026-10-12. · [I-0290] `[Apple]` — buffer paste moved ⌃1–9 → ⌃⌘1–9 (Mission Control took ⌃1). ✅ VERIFIED 2026-10-12. · [I-0291] `[Apple]` — the palette button follows ⌃⌘. ✅ VERIFIED 2026-10-12.

---

## 🟠 T-0605 — `[Linux]` EP-026 S2: undo/redo on Linux — Ctrl+Z / Ctrl+Shift+Z, persistence, session warning, barriers, grouped edits

**Epic:** [EP-026] (S2) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-11) — ✅ incl. the fix for [I-0287], which its smoke found (ruled into [SP-178], AC9); ⏳ rig live pass (AC8) · **Sprint:** [SP-178] (🟡 Active) →
[`../Sprints/Sprint-SP-178.md`](../Sprints/Sprint-SP-178.md) · **Date Requested:** 2026-10-11 (user: *"We are ready to plan EP-026 S2."*)

**What:** Linux reports to the capture rules SP-177 moved into ScriviCore (`scrivi_capture_*`) and applies undo/redo, in
Apple's shape (`HistoryCapture.swift`, `ManuscriptTextView.swift` apply path): bridge the S2 history/capture endpoints; a
plumbing-only `HistoryCapture` (idle timer at the core's deadline, `isApplying`); open once per project open (never on a
reload) and close in `ScriviWindow::closeEvent`; report edits, caret moves, paste/cut, grouped cross-scene edits and saves;
barriers on split/merge/delete (✅ [I-0287]: split and merge record theirs AFTER the op, carrying the scenes' new texts); Edit ▸ Undo (Ctrl+Z) / Redo (Ctrl+Shift+Z) with Apple's barrier and session-boundary
notices. Proven by Linux smokes through the real key dispatch and a rig live pass.

