# Verified Task — T-0629

**Sprint:** [SP-197] · **Epic:** [EP-052] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0650] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0629 — `[ScriviCore]` EP-052 S3: one Inspector read per scene, and the per-scene searchable text

**Epic:** [EP-052] (AC8, AC9) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0650] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-197] (🟡 Active) → [`../../Sprints/Closed/Sprint-SP-197.md`](../../Sprints/Closed/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** (a) one endpoint returning everything the Inspector shows for a scene: its notes, its edges with each far
object's name, kind, image path and pending state, the sources' citations, and the bound worlds' status, with the object
index built once (v0.2 R3b; [SP-197] Q1). It replaces the 32 calls the Mac makes today (`ObjectCard.swift:157,179,614`,
`SourcesCard.swift:89,107`, `WritingToolCards.swift:37`, `ScenePropertiesView.swift:133`) and Linux's
(`SceneInspector.cpp:404–409,856`). An unreachable world keeps today's behaviour ([I-0222], D8): its edges still list by
cached name. (b) `scrivi_extract_searchable_text` for a list of scene IDs (`scrivi.h:684` takes a project only).
**Tests:** boundary tests through `scrivi_*`; the file-operation count for one scene asserted (CountingFileSystem); an
unreachable world; Engine stub parity (`check-engine-stub-parity.sh`) and the Linux bridge.

**Done (2026-10-27):** `scrivi_load_scene_inspector` (`scrivi_c_api.cpp:3477`): notes, edges, the far objects (the
object index built once, narrowed to the scene's far endpoints), worlds and, when asked, each reachable far object's own
edges, under ONE project lock, each part its standalone endpoint's builder (`EndpointPart`, shared with
`scrivi_load_timeline`), a failing part as `<part>Error`. An empty part is `{}`, never `null`. `scrivi_extract_searchable_scenes`
(`:3588`): the named scenes' records only (`ExtractSearchableTextRequest::scenesOnly`). Bound in Swift (`loadSceneInspector`,
`extractSearchableScenes`) and on Linux (`ScriviBridge::loadSceneInspector`). Each envelope's trace now names its project, so a
test can count one project's main-thread calls apart from the others running beside it. **Evidence:** ctest 786/786 (5 new
`[SP-197]` boundary cases: parts equal the standalone endpoints, one revision; a world away; nothing related; a missing root;
the per-scene extraction). File operations on the fixture: the composite **67**, against **353+** for the per-card calls it
replaces. ⚠️ One unidentified ctest failure in the first full run after a rebuild; 18 full runs since are clean.
