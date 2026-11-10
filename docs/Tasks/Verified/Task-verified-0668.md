# Verified Task — T-0668

**Sprint:** [SP-208] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-10** (the user's live passes on the Mac and Linux, then *"They opened cleanly, close SP-208"*)
**Archived:** 2026-11-10.

---

## ✅ T-0668 — `[Cross]` EP-054 S4: Word `.docx` export

**Epic:** [EP-054] (S4, AC5) · **Status:** ✅ **Verified 2026-11-10** (user) · **Sprint:** [SP-208] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-208.md`](../../Sprints/Closed/Sprint-SP-208.md) · **Date Requested:** 2026-11-10 (user: *"plan SP-208 for EP-054 S4"*)

**What:** `scrivi_export_manuscript` writes the manuscript (or one chapter) as a Word `.docx` — every paragraph in a named
style, chapters on new pages, scene breaks, emphasis, headings and real lists as in Scrivi — zipped with miniz to the same
bytes on every platform; and Export Manuscript… offers it on the Mac, iPad and Linux. Typography, endpoint shape, breaks,
lists and scope are SP-208's Q1–Q7 (✅ ruled as recommended).

**Built (2026-11-10):** `exportDocx` (`ScriviCore/src/manuscript/ManuscriptExport.cpp`, beside HTML, sharing `sceneBlocks`);
`scrivi_export_manuscript` `format: "docx"` + `destPath` (`scrivi_c_api.cpp`, `scrivi.h`); `ScriviEngine.exportManuscript`
(+ stub); Export Manuscript… ▸ Word Document (.docx) in `ProjectArchiveFlow.swift` and Linux `EditorShell::exportManuscript`.
Evidence: `ManuscriptExportCApiTests` 11 cases (macOS ctest 826/826), interop `manuscriptExportThroughEngine`,
`bridge_parity_smoke` export checks. Then the `.docx` matched to the screen (1.45 line, third-line gap, hanging lists, the scene
break as a 1 pt rule). Final evidence: macOS ctest 826/826; Linux (scrivi-vm, `3221180`) 830/830, bridge parity 73/0,
accessibility smoke 6/6; the same 60-scene `.docx` SHA-256 on the Mac and Linux arm64.
**Live passes (user, 2026-11-10):** a chapter and the whole manuscript exported on the Mac and on Linux open cleanly. The iPad
pass → [T-0669].
