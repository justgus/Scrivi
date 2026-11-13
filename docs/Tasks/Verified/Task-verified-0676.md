# Verified Task — T-0676

**Sprint:** [SP-215] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-13** (user, 2026-11-13: *"Mac steps 1-8 pass. iPad steps 1-5 pass. Linux steps 1-6 pass."*)
**Archived:** 2026-11-13.

---

## ✅ T-0676 — `[Cross]` EP-054: File ▸ Import / Export

**Epic:** [EP-054] · **Status:** ✅ **Verified 2026-11-13** (user) · **Sprint:** [SP-215] (✅ Closed 2026-11-13) →
[`../../Sprints/Closed/Sprint-SP-215.md`](../../Sprints/Closed/Sprint-SP-215.md) · **Date Requested:** 2026-11-13 (user: *"…'Export Manuscript'
would be misleading if the writer only wants to export a single chapter…"*)

**What:** on the Mac, iPad and Linux, File holds Copy as HTML, **Import ▸** Timeline…, Archive… and **Export ▸** Manuscript…,
Current Chapter…, Timeline…, Archive…; Export ▸ Current Chapter… replaces the scope question; the toolbar's Export button matches (tooltip
"Export"); the iPad can import and export a timeline. Rulings and ACs: the Sprint.

**Built (2026-11-13):** Apple — `ImportExportCommands` / `ExportMenuItems` (`Scrivi/App/ProjectArchiveFlow.swift`), used by
both menu bars (`ScriviApp.swift`) and the toolbar (`ProjectToolbar.swift`, tooltip "Export"); `exportManuscript(_:currentChapter:)`;
the timeline flow moved out of the panel (`exportTimeline`, `importTimeline`, `readTimeline`, `finishTimelineImport`; the
epoch sheet in `EditorView.swift`, `ProjectSession.pendingTimelineImport`; the iPad's picker via `ArchivePicker.timeline`).
Linux — the File menu's submenus (`ScriviWindow.cpp`), `EditorShell::exportManuscript(bool)`, `caretChapter`; `settings_smoke` §6.
Evidence: macOS and iOS builds, interop 348; Linux ctest 847/847, `settings_smoke` §6 (mutation-checked). **Verified (2026-11-13):** the user's live passes on the Mac, iPad and Linux (SP-215 AC6).
