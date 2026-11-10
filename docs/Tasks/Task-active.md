# Active Tasks

*Last Updated: 2026-11-10 (**T-0668 → active** with [SP-208]'s activation (EP-054 S4).)*

*Last Updated: 2026-11-09 (**T-0666 → ✅ Verified** and archived → `Verified/Task-verified-0666.md` ([SP-207] closed) (user, 2026-11-09: *"I-0357 verified, close SP-207"*))*

*Last Updated: 2026-11-09 (**T-0666 → active** with [SP-207]'s activation (EP-054 S3).)*

*Last Updated: 2026-11-06 (**T-0665 → 🟠** → `Task-unverified.md` ([SP-206] built; the live passes are the user's).)*

*Last Updated: 2026-11-06 (**T-0665 → active** with SP-206's activation (EP-054 S2).)*

*Last Updated: 2026-11-06 (**T-0664 → 🟠** → `Task-unverified.md` ([SP-205] built; core only, no live pass).)*

*Last Updated: 2026-11-06 (**T-0664 → active** with SP-205's activation (EP-054 S1).)*

*Last Updated: 2026-11-05 (**T-0611 → 🟠** → `Task-unverified.md` ([SP-184] built; the live pass is the user's).)*

*Last Updated: 2026-11-05 (**T-0611 → active** with SP-184's activation (EP-048 S5c).)*

*Last Updated: 2026-11-05 (**T-0610 → 🟠** → `Task-unverified.md` ([SP-183] built; the live pass is the user's).)*

*Last Updated: 2026-11-05 (**T-0610 → active** with SP-183's activation (EP-048 S5b).)*

*Last Updated: 2026-11-05 (**T-0662 → 🟠** → `Task-unverified.md` ([SP-204] built; Mac string check and live pass pending).)*

*Last Updated: 2026-11-05 (**T-0662 → active** with SP-204's activation (EP-026 S3).)*

*Last Updated: 2026-11-04 (**T-0658 → 🟠** → `Task-unverified.md` (SP-202 measured and ruled).)*

*Last Updated: 2026-11-04 (**T-0658 filed → active** with SP-202's activation (EP-050 S3).)*

*Last Updated: 2026-11-02 (**T-0646 → 🟠** → `Task-unverified.md`: its rig pass moved to [T-0653] ([EP-057]).)*

*Last Updated: 2026-11-01 (**T-0400, T-0643–T-0645 → 🟠** → `Task-unverified.md`; T-0646 stays active for the live passes.)*

*Last Updated: 2026-11-01 (**T-0400, T-0643–T-0646 → active** with SP-200's activation (EP-052 AC11).)*

*Last Updated: 2026-10-31 (**T-0639–T-0642** filed from SP-199's live pass and implemented on Apple → 🟠 `Task-unverified.md`.)*

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 filed → active** with SP-199's activation (EP-055 S2).)*

---

## 🟡 T-0668 — `[Cross]` EP-054 S4: Word `.docx` export

**Epic:** [EP-054] (S4, AC5) · **Status:** 🟡 Active (2026-11-10) · **Sprint:** [SP-208] (🟡 Active) →
[`../Sprints/Sprint-SP-208.md`](../Sprints/Sprint-SP-208.md) · **Date Requested:** 2026-11-10 (user: *"plan SP-208 for EP-054 S4"*)

**What:** `scrivi_export_manuscript` writes the manuscript (or one chapter) as a Word `.docx` — every paragraph in a named
style, chapters on new pages, scene breaks, emphasis, headings and real lists as in Scrivi — zipped with miniz to the same
bytes on every platform; and Export Manuscript… offers it on the Mac, iPad and Linux. Typography, endpoint shape, breaks,
lists and scope are SP-208's Q1–Q7 (✅ ruled as recommended).

**Built (2026-11-10):** `exportDocx` (`ScriviCore/src/manuscript/ManuscriptExport.cpp`, beside HTML, sharing `sceneBlocks`);
`scrivi_export_manuscript` `format: "docx"` + `destPath` (`scrivi_c_api.cpp`, `scrivi.h`); `ScriviEngine.exportManuscript`
(+ stub); Export Manuscript… ▸ Word Document (.docx) in `ProjectArchiveFlow.swift` and Linux `EditorShell::exportManuscript`.
Evidence: `ManuscriptExportCApiTests` 11 cases (macOS ctest 826/826), interop `manuscriptExportThroughEngine`,
`bridge_parity_smoke` export checks (⚠️ not yet run on Linux).
