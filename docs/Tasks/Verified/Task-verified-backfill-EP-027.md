# Verified Tasks — BACKFILL — EP-027 [ScriviCore] Filesystem-Authoritative Chapter/Scene Identity & Ordering

**Created:** 2026-09-15 · **Audit ruling [R-11]** · ✅ **Epic attributions VERIFIED 2026-09-15**
**Tasks:** 29 (T-0264 … T-0293) · **All ✅ Verified**

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
| T-0264 | P1: FileSystem::renamePath port method + LocalFileSystem impl — no-clobber + missing-source guards + std::filesystem::rename (atomic-within-… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0265 | P2: util/OrderKey — fractional (LexoRank-style) base-62 order keys: keyBetween/keyBefore/keyAfter/isOrderKey. 8 property tests (3119 assertion… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0266 | P2: manuscript/ChapterIndex — disk-authoritative helpers + rebuildIndexIfInconsistent (open-time self-heal). Unit tests [ChapterIndex] (5 ca… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0267 | P2: ChapterCreator order-key slug — collision-free (fixes I-0072); ProjectCreator initial chapter-001 kept. Integration regression [I-0… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0268 | P2: disk-authoritative order + reorder — ManuscriptOrderResolver folder-key sort (B3); ChapterReorderer = keyBetween + renamePath the one… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0269 | P2: open-time index self-heal — rebuildIndexIfInconsistent wired into ProjectOpener; I-0072-corrupt index rebuilt from disk, idempotent. Integ… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0270 | P3: legacy-project migration — migrateChapterOrderKeys assigns fresh ascending order-keys in index-array order via renameChapterFolder, then s… | SP-069 | ✅ declared in archive | [`SP-069`](../../Sprints/Closed/Sprint-SP-069.md) |
| T-0271 | SceneRef filename-only schema + serde — {metadataFilename} (drops sceneID+path); dual-scheme read keeps a legacy metadataPath/sceneID re… | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0272 | util/OrderKey scene filenames in SceneCreator/ChapterCreator/ProjectCreator — new scenes get <orderKey>-<slug> filenames, filename-only… | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0273 | manuscript/SceneIndex — listScenesByOrder (folder-scan, order-key sort, id from sidecar), sceneMetaRelPath/chapterDirOf, rebuildChapterSc… | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0274 | SceneReorderer relocates files — cross-chapter move = renamePath both files into the dest folder + between-neighbours order key + rebuild both… | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0275 | Simplify renameChapterFolder — remove per-scene metadataPath/contentPath rewriting (filename-only refs need none); only the chapter slug s… | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0276 | Scene consumers updated — SceneDeleter, SceneRenamer.findSceneMetadataPath, ManuscriptOrderResolver, ProjectValidator, ExternalChangeScan… | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0277 | Scene migration + orphan repair wired into ProjectOpener (before validation) + 5 integration tests [EP-027][scenes]. Full suite 298/298 macOS. | SP-070 | ✅ declared in archive | [`SP-070`](../../Sprints/Closed/Sprint-SP-070.md) |
| T-0279 | CreateChapterRequest.afterChapterID (empty = append). | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0280 | ChapterCreator create-in-place — (lo,hi) window + keyBetween; guard empty-key error. | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0281 | C ABI + facade — scrivi_create_chapter trailing afterChapterID (additive; scrivi.h updated); facade passthrough. | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0282 | ScriviBridge::createChapter(..., afterChapterID = {}). | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0283 | EditorShell split rewrite — one create-in-place call; drop reorder + stale re-resolve; C1/C3/C4 fixes; remove [split] debug logs. | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0284 | Tests — create-in-place unit test (create after Ch2 → chapter-c, never chapter-w; K0 path valid); split repro updated to the one-call form… | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0285 | 2nd defect (found in VNC verify): same-named scene stolen on reopen. After a follower reordered OUT of a chapter, rebuildChapterScenesIfInconsist… | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0286 | App hardening: EditorShell split now checks every saveScene/reorderScene return and aborts+warns on the first failure (was silently cont… | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0287 | Remove the split confirmation dialog — Ctrl+Shift+Return IS the approval. A modal "Split into New Chapter?" prompt on every break destroyed drafti… | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0288 | I-0075 — arrows cross scene/chapter boundaries. normalizeCaret snapped to the nearest editable edge (tie → previous), so Down at a scene end /… | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0289 | Strip [split] diagnostic logging — removed the verbose segment-dump + per-call qInfo (their job is done). Kept the failSplit abort + a singl… | SP-071 | ✅ declared in archive | [`SP-071`](../../Sprints/Closed/Sprint-SP-071.md) |
| T-0290 | I-0078 — macOS createChapter wrapper gains afterChapterID (append default) | SP-072 | ✅ declared in archive | [`SP-072`](../../Sprints/Closed/Sprint-SP-072.md) |
| T-0291 | I-0076 — parseSceneMeta bares content.path; migrateScenes normalises stale sidecars | SP-072 | ✅ declared in archive | [`SP-072`](../../Sprints/Closed/Sprint-SP-072.md) |
| T-0292 | I-0077 — migrateChapterOrderKeys path-folder fallback on id mismatch | SP-072 | ✅ declared in archive | [`SP-072`](../../Sprints/Closed/Sprint-SP-072.md) |
| T-0293 | 2 regression tests in SceneIdentityTests.cpp (I-0076 full-path normalise; I-0077 mismatch migrates), each verified RED-without-fix | SP-072 | ✅ declared in archive | [`SP-072`](../../Sprints/Closed/Sprint-SP-072.md) |
