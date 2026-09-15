# Verified Tasks — BACKFILL — EP-010 Manuscript Structure Editing — Delete, Reorder, and Title

**Created:** 2026-09-15 · **Audit ruling [R-11]** · ✅ **Epic attributions VERIFIED 2026-09-15**
**Tasks:** 12 (T-0097 … T-0111) · **All ✅ Verified**

⚠️ **WHY THIS FILE EXISTS.** ✅ **These Tasks were complete and Verified, but their ONLY record was
the closed Sprint archive.** ⚠️ **A prior audit (2026-08-19) recorded the gap and scoped it OUT;
✅ the user ruled 2026-09-15: *"This audit will not defer it. Backfill them."***

✅ **EVERY EPIC ATTRIBUTION IN THIS FILE IS VERIFIED AGAINST ITS SPRINT ARCHIVE.**
⚠️ **The first pass INFERRED 24 attributions from the first `EP-0xx` mention in the text;**
✅ **all 24 were then checked by opening the archives** — ⚠️ **16 were right, ⛔ 8 were WRONG.**
⚠️ **The method that failed: my checker looked for a `**Epic:**` markdown line and missed that five
archives declare the Epic in YAML FRONTMATTER (`epic: EP-0xx`).**

⚠️ **THE SPRINT ARCHIVE REMAINS THE RECORD OF TRUTH.**

| Task | Title | Sprint | Epic evidence | Record of truth |
| ---- | ----- | ------ | ------------- | --------------- |
| T-0097 | Implement deleteScene C++ facade method — remove scene file + sidecar, update chapter index | SP-027 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-027`](../../Sprints/Closed/Sprint-SP-027.md) |
| T-0098 | Implement deleteChapter C++ facade method — remove chapter directory, update manuscript index | SP-027 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-027`](../../Sprints/Closed/Sprint-SP-027.md) |
| T-0099 | Implement renameScene(metadataPath, newTitle) C++ facade method — write title to scene sidecar JSON | SP-028 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-028`](../../Sprints/Closed/Sprint-SP-028.md) |
| T-0100 | Implement renameChapter(metadataPath, newTitle) C++ facade method — write title to chapter sidecar JSON | SP-028 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-028`](../../Sprints/Closed/Sprint-SP-028.md) |
| T-0101 | Implement reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?) C++ facade method | SP-029 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-029`](../../Sprints/Closed/Sprint-SP-029.md) |
| T-0102 | Implement reorderChapter(chapterID, afterChapterID?) C++ facade method | SP-029 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-029`](../../Sprints/Closed/Sprint-SP-029.md) |
| T-0103 | Wire deleteScene, deleteChapter, renameScene, renameChapter through ScriviEngine.swift | SP-030 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-030`](../../Sprints/Closed/Sprint-SP-030.md) |
| T-0104 | SwiftUI context menu on scene and chapter Navigator rows — right-click (macOS) / long-press (iOS) with Rename and Delete items | SP-030 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-030`](../../Sprints/Closed/Sprint-SP-030.md) |
| T-0105 | Rename sheet (focused edit field, pre-populated title, save/cancel) and delete confirmation dialog (warns chapter deletes all scenes) | SP-030 | ✅ **VERIFIED** — frontmatter `epic: EP-010` | [`SP-030`](../../Sprints/Closed/Sprint-SP-030.md) |
| T-0109 | Global chapter title toggle + headings in writing surface | SP-032 | ✅ declared in archive | [`SP-032`](../../Sprints/Closed/Sprint-SP-032.md) |
| T-0110 | Navigator title fallback chain + delete-of-open-scene edge case + cursor placement | SP-032 | ✅ declared in archive | [`SP-032`](../../Sprints/Closed/Sprint-SP-032.md) |
| T-0111 | EP-010 acceptance criteria verification + macOS smoke test | SP-032 | ✅ declared in archive | [`SP-032`](../../Sprints/Closed/Sprint-SP-032.md) |

⚠️ **9 row(s) here were re-checked; ✅ see the Epic evidence column.**
