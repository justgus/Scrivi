# Verified Task — T-0679

**Sprint:** [SP-218] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-15** (user, 2026-11-15: *"pushed and rebuilt on the VM. Ran Linux steps 1-5. all passed."*; Mac and iPad passes 2026-11-14/15)
**Archived:** 2026-11-15.

---

## ✅ T-0679 — `[Cross]` EP-054: import a manuscript from Markdown or plain text

**Epic:** [EP-054] (AC6, AC7, AC8) · **Status:** ✅ **Verified 2026-11-15** (user) · **Sprint:** [SP-218] (🟡 Active) →
[`../Sprints/Sprint-SP-218.md`](../../Sprints/Sprint-SP-218.md) · **Date Requested:** 2026-11-14 (user: *"We're ready to begin planning
S6, the Markdown/text and .docx impor."*)

**What:** `scrivi_import_manuscript` reads a Markdown or `.txt` file into a `scrivi.range.v1` and a report of what it could
not map; File ▸ Import ▸ Manuscript… on the Mac, iPad and Linux creates a new project from it or (Mac, Linux) pastes it into
the open project at the caret as one undo step. Rulings and ACs: the Sprint.
**Built (2026-11-14):** core `ScriviCore/src/manuscript/ManuscriptImport.cpp` (pure parser) and `scrivi_import_manuscript`
(`scrivi_c_api.cpp`, `scrivi.h`); Apple `ScriviEngine.importManuscript` (+ stub), `presentImportManuscript` /
`importManuscript` (`Scrivi/App/ProjectArchiveFlow.swift`), `ProjectSession.importRangeAction` → the editor's
`pasteStructuredRange`; Linux `ScriviBridge::importManuscript`, `ShellController::importManuscript` (`ScriviWindow.cpp`) →
`EditorShell::pasteRange`. Evidence: `ManuscriptImportCApiTests` (11 cases + a timed one); macOS ctest 853/853, Linux (Docker,
GCC) 857/857; interop 350/350; 43/43 Linux smokes. ⏳ The live passes (SP-218 AC7).
