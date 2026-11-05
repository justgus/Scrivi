# Verified Task — T-0662

**Sprint:** [SP-204] · **Epic:** [EP-026] · **Platform:** `[Linux]` (+ `[Apple]` string)
✅ **USER-VERIFIED 2026-11-05** (user: the Mac pass, then the Linux live pass on scrivi-vm, build 1.0.357 `6c5f3ab`: *"all steps pass."*)
**Archived:** 2026-11-05.

---

## ✅ T-0662 — `[Linux]` EP-026 S3: branching — chooser, margin mark, History section, stale purge, Undo History settings

**Epic:** [EP-026] (S3) · **Status:** ✅ **Verified 2026-11-05** (user) · **Sprint:** [SP-204] →
[`../../Sprints/Closed/Sprint-SP-204.md`](../../Sprints/Closed/Sprint-SP-204.md) · **Date Requested:** 2026-11-05 (user: *"plan EP-026 S3"*)

**What:** bind the six branching endpoints; Ctrl+Shift+Z at a branch point opens the branch chooser ([I-0320]); the
branch-point margin mark ([T-0620]'s Linux half); a History section in the Writing tab; Project ▸ Purge Stale History
Branches…; Project Settings ▸ Undo History. `[Apple]` too ([SP-204] Q3): `HistoryCard.swift:207`'s stale-badge text names
Project ▸ Purge Stale History Branches…, not Project Settings. Tested by a new `branch_smoke`, mutation-checked.

**Verified (user, 2026-11-05):** the Mac pass (`HistoryCard.swift:207`'s badge text) and the Linux live pass on scrivi-vm
(build 1.0.357), all steps pass.

**Done (2026-11-05):** built as [SP-204]'s plan; AC1–AC7 met on Docker + macOS evidence (all 40 Linux smoke steps, `branch_smoke`
5/5 mutations caught, ctest macOS 806 + Linux 810, interop 337). Files: `ScriviBridge`, `HistoryCapture`, `EditorShell`,
`ManuscriptEditor`, `HistorySection` (new), `ProjectSettingsDialog`, `ProjectPreferences`, `SceneInspector`, `ScriviWindow`,
`tests/branch_smoke.*`, CMake, Linux CI; `[Apple]` `Scrivi/Views/Inspector/HistoryCard.swift:207`.
