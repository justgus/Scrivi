# Verified Task — T-0666

**Sprint:** [SP-207] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-09** (the user's live passes on all three platforms, then *"I-0357 verified, close SP-207"*)
**Archived:** 2026-11-09.

---

## ✅ T-0666 — `[Cross]` EP-054 S3: Markdown and HTML export

**Epic:** [EP-054] (S3) · **Status:** ✅ **Verified 2026-11-09** (user) · **Sprint:** [SP-207] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-207.md`](../../Sprints/Closed/Sprint-SP-207.md) · **Date Requested:** 2026-11-09 (user: *"plan SP-207 for EP-054 S3"*)

**What:** one core endpoint, `scrivi_export_manuscript`, writing the manuscript (or one chapter) as Markdown or as HTML
built from what the writer sees — every paragraph its own `<p>` (EP-054 Q2) — and File ▸ Export Manuscript… / Copy as HTML
on the Mac, iPad and Linux. The scope, destinations, scene breaks, titles and tags are SP-207's Q1–Q8 (✅ ruled); the AO3 test page is its step 4.

**Implemented (2026-11-09, [SP-207]):** `scrivi_export_manuscript` (`scrivi.h`; `scrivi_c_api.cpp`) →
`ScriviCore/src/manuscript/ManuscriptExport.cpp` (Markdown and HTML from what the writer sees; `readSceneTextsInParallel`
shared with `scrivi_read_scene_texts`); `ScriviEngine.exportManuscript` (+ visionOS stub), `ScriviBridge::exportManuscript`;
File ▸ Export Manuscript… / Copy as HTML and the toolbar's Export menu (`ProjectArchiveFlow.swift`, `ScriviApp.swift`,
`ProjectToolbar.swift`); Linux `EditorShell::exportManuscript` / `copyManuscriptAsHTML`. Evidence: `ManuscriptExportCApiTests`
7 cases (macOS 822/822; Linux 826/826, the same bytes), interop `manuscriptExportThroughEngine`. The AO3 test page (a private
Artifact) ported from otwarchive `452391b6`.
**Live passes (user, 2026-11-09):** Copy as HTML on the Mac and iOS; the Linux live pass on scrivi-vm; Markdown in Ulysses
(Mac) and HTML in Safari (iPad) — *"perfect"*. The paste into AO3 itself → [T-0667] (AO3 offline).
