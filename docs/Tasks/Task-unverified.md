# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

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

