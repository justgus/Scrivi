# Verified Task — T-0674

**Sprint:** [SP-213] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-12** (user, 2026-11-12: *"The iPad is present. Steps 1-4 pass."*)
**Archived:** 2026-11-12.

---

## ✅ T-0674 — `[Cross]` EP-054 S5: ePub export

**Epic:** [EP-054] (S5; AC5, AC7, AC8) · **Status:** ✅ **Verified 2026-11-12** (user) · **Sprint:** [SP-213] (🟡 Active) →
[`../../Sprints/Sprint-SP-213.md`](../../Sprints/Sprint-SP-213.md) · **Date Requested:** 2026-11-12 (user: *"plan EP-054 S5."*)

**What:** `scrivi_export_manuscript` writes an EPUB 3 of the whole manuscript (title page with byline, contents, a file per
chapter, the writer's size and indent rule, metadata) that passes epubcheck; Export Manuscript… offers it on the Mac, iPad and
Linux. Shape per SP-213's Q1–Q8 (✅ ruled as recommended); by Q1 the PDF is its own Sprint, not this Task.

**Built (2026-11-12):** `exportEpub` + `zipParts` (`ScriviCore/src/manuscript/ManuscriptExport.cpp`); `format: "epub"`,
`language`, `uuidOf` (`scrivi_c_api.cpp`, `scrivi.h`); `ScriviEngine.exportManuscript(… language:)` (+ stub);
`ProjectArchiveFlow.swift`; Linux `EditorShell::exportManuscript`; `bridge_parity_smoke`. Evidence: macOS ctest 840/840,
interop 1/1, iOS build, Linux ctest 845/845 and smoke 74/0; epubcheck 0 errors / 0 warnings on two real projects; Mac and
Linux ePubs identical but the modified time. **Verified (2026-11-12):** the user's live passes — Apple Books on the Mac and iPad, Foliate on Linux (SP-213 AC5).
