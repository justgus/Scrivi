# Verified Task — T-0678

**Sprint:** [SP-215] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-13** (user, 2026-11-13: *"mac ipad linux all pass."*)
**Archived:** 2026-11-13.

---

## ✅ T-0678 — `[Cross]` EP-054: the export formats as submenus

**Epic:** [EP-054] · **Status:** ✅ **Verified 2026-11-13** (user) · **Sprint:** [SP-215] (✅ Closed 2026-11-13, AC7) →
[`../../Sprints/Closed/Sprint-SP-215.md`](../../Sprints/Closed/Sprint-SP-215.md) · **Date Requested:** 2026-11-13 (user: *"…make it a vertical list
of buttons like the iPad displays … That would unify the interface and make selection less error prone."*; option B ruled)

**What:** File ▸ Export ▸ **Manuscript ▸** Markdown…, HTML…, Word Document…, ePub…, PDF… and **Current Chapter ▸** the same
without ePub, on the Mac, the iPad (menu bar and toolbar's Export button) and Linux, in place of the format dialog.
**Built (2026-11-13):** Apple `ExportMenuItems` / `ExportFormat` and `exportManuscript(_:format:currentChapter:)`
(`Scrivi/App/ProjectArchiveFlow.swift`); Linux `ScriviWindow.cpp` submenus, `EditorShell::exportManuscript(format,
currentChapter)`; `settings_smoke` §6 (mutation-checked). Evidence: macOS and iOS builds, interop 348; Linux ctest 847/847 and
the menu smokes. **Verified (2026-11-13):** the user's live passes on the Mac, iPad and Linux (SP-215 AC7).
