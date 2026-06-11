# T-0153: Scene Dot Tooltip / Popover — Title, Chapter, Human-Readable Story-Time

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift` — `SceneDotView.dotTooltip`, `storyTimeDescription`, `humanReadableDuration`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**
`.onHover` shows a tooltip popover (anchored on the `ZStack`, I-0026/I-0028 fix) containing scene title, chapter title, and story-time description. For `offsetSource == "default"`: "Immediately after [previous scene title]." For `offsetSource == "manual"`: computes `gap = dot.offsetMs - previousSceneEndMs` and passes to `humanReadableDuration(ms:epochLabel:)` which formats as "N hours, M minutes after [previous scene] ends." `humanReadableDuration` uses the two largest non-zero units.
