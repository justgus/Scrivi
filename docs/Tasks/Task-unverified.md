# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0612 — `[Linux]` A diagnostic log: what the app was doing, and when the window stopped answering

**Epic:** [EP-026] · **Status:** 🟠 **Implemented - Not Verified** (2026-10-17) — ⏳ rig live pass (SP-185 AC6) · **Sprint:** [SP-185] (🟡 Active) →
[`../Sprints/Sprint-SP-185.md`](../Sprints/Sprint-SP-185.md) · **Date Requested:** 2026-10-17 (user: *"We may need to create a
log file so that we can record what the app is doing. A simple code inspection may not be adequate."*)

**What:** a log file the rig keeps between runs, so a freeze or a disappearance can be attributed afterwards. Its shape is
[SP-185] Q1 (✅ ruled 2026-10-17: *"build the log as proposed."*): one file under `~/.local/state/scrivi/`, timed spans of the app's operations, a
main-thread stall watchdog, and Qt's own warnings and fatal messages.

**Done (2026-10-17):** `DiagnosticLog` (`platforms/linux/src/DiagnosticLog.hpp/.cpp`) with the watchdog, Qt's messages, a
crash report with a backtrace, timed spans across the editor, find bar and workers, and Help ▸ Show Log Folder. Proven by a
new `diagnostic_log_smoke` (CI); 3 mutations caught.

---

## 🟠 T-0609 — `[Linux]` EP-048 S5a: Project Settings dialog, the bundled typefaces, Markup Hints

**Epic:** [EP-048] (S5a, L10) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-15) — ⏳ rig live pass (AC5) · **Sprint:** [SP-182] (🟡 Active) →
[`../Sprints/Sprint-SP-182.md`](../Sprints/Sprint-SP-182.md) · **Date Requested:** 2026-10-15 (user: *"plan EP-048 fully."*)

**What:**
- Replace the T-0312 *"coming soon"* stub with Linux's Project Settings dialog (Project: Title, Subtitle · Typography:
  Typeface, Text size).
- Back it with a Linux `ProjectPreferences` in Apple's shape over the shared `project-settings.json`: the whole document
  kept, keys written only once chosen.
- Register the seven bundled faces from `Resources/Fonts/fonts.json` and ship their OFL texts. The manuscript, headings and
  chapter titles draw in the project's face and size.
- View ▸ Show Markup Hints (Ctrl+Shift+H).
- Variable-font weights are measured on Qt 6.4.2 and 6.10.2 first.

**Done (2026-10-15):** `ManuscriptTypography` (`BundledFonts` from `fonts.json`, compiled in), `ProjectPreferences`,
`ProjectSettingsDialog`, the presenter's hints switch and chapter titles, View ▸ Show Markup Hints. Proven by a new
`settings_smoke` (CI), 5 mutations caught.

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

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** ([SP-182]); ⏳ rig live pass.)*

