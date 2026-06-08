## T-0116: Correct cursor placement after navigate and delete

**Status:** ✅ Implemented - Verified
**Date Implemented:** 2026-06-08
**Component:** `ManuscriptTextView`, `ViewportSceneLoader`, `SceneNavigatorView`
**Priority:** High
**Date Requested:** 2026-06-08
**Sprint Assigned:** SP-033

**Rationale:**
After a Navigator tap or a scene delete, the cursor must land at a predictable, correct position. Currently it does not (EP-011 Known Defect 2).

**Current Behavior:**
After a Navigator tap, cursor lands at the start of the scene's character range in `NSTextStorage`, but the position is not reliably computed when chapter heading content is present. After a scene delete, cursor placement is undefined.

**Desired Behavior:**
- **Navigator tap:** cursor is placed at the first character of the tapped scene (manuscript position = `sceneStartMap[sceneID]`). The separator preceding the scene is not the cursor target.
- **Scene delete (not last scene):** cursor is placed at the first character of the next scene.
- **Scene delete (last scene):** cursor is placed at the end of the manuscript.
- In both delete cases, keyboard focus transfers to `ManuscriptNSTextView` immediately so the writer can type without a second click.

**Requirements:**
1. Navigator tap uses `sceneStartMap` (T-0115) to compute the correct character offset
2. `deleteScene` result handler places cursor using the same map after the storage is updated
3. Focus transfer after delete uses the first-responder model from T-0117 (not the `focusManuscriptView` binding)
4. Cursor placement is correct in the presence of chapter headings

**Design Approach:**
Replace the ad-hoc cursor placement logic with `sceneStartMap` lookups. After delete, recompute the map, derive the new cursor target, set selection on `NSTextView`, and call `window?.makeFirstResponder(textView)`.

**Components Affected:**
- `SceneNavigatorView.swift`: update tap handler to use `sceneStartMap`
- `ViewportSceneLoader.swift`: update `deleteScene` result handler
- `ManuscriptTextView.swift`: expose a `setCursorPosition(_:)` method

**Dependencies:**
- T-0115 (scene start map) must be complete first

**Test Steps:**
1. Tap Scene 3 in Navigator — confirm cursor lands at first character of Scene 3, not in heading
2. Delete the currently open scene (not last) — confirm cursor lands at first character of next scene
3. Delete the last scene — confirm cursor lands at end of manuscript
4. After delete, confirm writer can type immediately without clicking

**Notes:**
Chapter heading characters ARE part of `NSTextStorage` but are NOT part of the scene's stored text. The start map must account for heading offsets correctly.
