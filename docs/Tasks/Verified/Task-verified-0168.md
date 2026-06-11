# T-0168: Scene Duration, Chain Propagation, Project Timeline Defaults, "Immediately After"

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift`, `SceneMetaJson.hpp/.cpp`, `ProjectJson.hpp/.cpp`, `ScriviCore.cpp`, `scrivi_c_api.cpp`, `scrivi.h`, `ScriviEngine.swift`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**

**Schema additions (C++):**
- `SceneStoryTime` gains `gapMs` (canonical gap from `previousSceneEnd` to scene start; 0 for default scenes) and `durationMs`/`durationSource`.
- `ProjectJsonData` gains `timelineDefaults` block: `defaultSceneDurationMs` (1 hour) and `dragPositioningMode` ("proportional"). Written by `createProject`. Optional on parse — absent in old projects.

**Model (Swift):**
- `SceneDot` carries `gapMs`, `durationMs`, `durationSource`. `offsetMs` is derived, never canonical.
- `recomputeAllOffsets` on load: walks manuscript order, computes `offsetMs = previousEnd + gapMs` for every scene.
- `recomputeAndPersistFrom(idx)` on commit: recomputes and persists absolute positions for all scenes from `idx` forward, regardless of `offsetSource`. Ensures manual scenes maintain their gap when any predecessor moves.
- `setStoryTime` now accepts `gapMs`; `applyPickerResult` computes `gap = targetMs - previousSceneEndMs`.
- `resetToDefault` sets `gapMs = 0`, `offsetSource = "default"`, recomputes and propagates.

**Picker additions:**
- Duration row: `Scene duration: [amount] [Unit▾]`.
- "Immediately after" footer button: fires `resetDefault`.
- Spinner pre-populates to nearest natural fit of `abs(rawOffsetMs - previousSceneEndMs)`.

**Default timeline span:** 7 scenes with no manual positioning now spans 7 hours (1 hour each), not 7 days.
