# Verified Task — T-0647

**Sprint:** [SP-200] · **Epic:** [EP-052] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01: *"All steps pass."*) and [T-0654] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0647 — `[Cross]` Properties tab: scene links, Project and World metrics

**Epic:** [EP-052] (AC11) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01: *"All steps pass."*) and [T-0654] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-200] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-200.md`](../../Sprints/Closed/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What (user, 2026-11-01):** *"Your recommendations for the metrics are approved … do both the scene links in the scene
portion and the total world links in the project section. Perhaps we can also do a 'World' metrics with a breakdown of the
number of what kind of objects exist (assuming you have that information without resorting to counting them)."*
**Done (2026-11-01):**
- **Core:** one new endpoint, `scrivi_get_project_metrics` (`scrivi.h:285`, `scrivi_c_api.cpp:1684`): scenes and chapters
  from the warm project index; links and world links from the relationship log; history steps and the active segment's
  bytes from the open store (`HistoryStore::segmentBytes`, memory only); per bound world, objects by kind from the world's
  own `index.json`, or the binding's cached index while the world is away (`fromCache`). Kinds in `kAllStorableKinds`
  order, never a restated list. No scene, object or history file is opened. ABI: one endpoint added.
- **Mac / iOS** (`ScenePropertiesView.swift`): Scene section gains **Links** (from the Inspector's existing read, no extra
  call); new **Project** section (Scenes, Chapters, Links, World links, History "4,612 steps · 18.7 MB") and one **World —
  name** section per bound world (Objects, then each kind). Read off the main actor each time the scene's read lands.
  `ScriviEngine.getProjectMetrics` + visionOS stub.
- **Linux** (`SceneInspector.cpp:1001`, `ScriviBridge::getProjectMetrics`): the same rows and sections, read on the
  Inspector's worker after the scene's read.
**Tests:** `[T-0647]` (`ObjectCApiTests.cpp:1337`, through `scrivi_*`); Mac interop `projectMetricsCrossTheBoundary`.
ctest 800/800, Mac interop 329/329, iOS 17/17, Linux smokes 38/38 (Docker, offscreen, non-root).
⚠️ The figures refresh when the scene's read lands (scene change, world or object change), not on every keystroke: the
history count can trail typing until the next scene change.
