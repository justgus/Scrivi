# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0608 — `[Linux]` EP-048 S4b: Replace and Replace All over the presented text

**Epic:** [EP-048] (S4b, L8's Replace half) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-15) — ⏳ rig live pass (AC6) · **Sprint:** [SP-181] (🟡 Active) →
[`../Sprints/Sprint-SP-181.md`](../Sprints/Sprint-SP-181.md) · **Date Requested:** 2026-10-15 (user: *"plan EP-048 fully."*)

**What:** the find bar's Replace row (Ctrl+Alt+F). Replace writes the replacement escaped and balanced, in the match's
first character's style ([SP-164] Q3). Replace All replaces every match in one pass as ONE undo step across scenes
([SP-164] Q2), through [SP-178]'s grouped edit. Apple's shape (`replaceFound`, `applyReplacements`, `replaceAll`). Replace All
runs its work on a worker, never the GUI thread, and writes only the scenes holding a match (SP-181 Q3). Proven by `find_smoke` through the real key dispatch and a rig
live pass.

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

---

## 🟠 T-0605 — `[Linux]` EP-026 S2: undo/redo on Linux — Ctrl+Z / Ctrl+Shift+Z, persistence, session warning, barriers, grouped edits

**Epic:** [EP-026] (S2) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-11) — ⏳ rig live pass (AC8) · **Sprint:** [SP-178] (🟡 Active) →
[`../Sprints/Sprint-SP-178.md`](../Sprints/Sprint-SP-178.md) · **Date Requested:** 2026-10-11 (user: *"We are ready to plan EP-026 S2."*)

**What:** Linux reports to the capture rules SP-177 moved into ScriviCore (`scrivi_capture_*`) and applies undo/redo, in
Apple's shape (`HistoryCapture.swift`, `ManuscriptTextView.swift` apply path): bridge the S2 history/capture endpoints; a
plumbing-only `HistoryCapture` (idle timer at the core's deadline, `isApplying`); open once per project open (never on a
reload) and close in `ScriviWindow::closeEvent`; report edits, caret moves, paste/cut, grouped cross-scene edits and saves;
barriers on split/merge/delete ([I-0287]: split and merge record theirs AFTER the op, carrying the scenes' new texts); Edit ▸ Undo (Ctrl+Z) / Redo (Ctrl+Shift+Z) with Apple's barrier and session-boundary
notices. Proven by Linux smokes through the real key dispatch and a rig live pass.

