# Scrivi Writing Surface Behavior Spec v0.1

**Status:** Approved  
**Date:** 2026-06-08  
**Epic:** EP-011 — Writing Surface Behavior: Scroll, Cursor, and Focus  
**Implements:** T-0113

---

## 1. Purpose

This document is the authoritative behavioral contract for `ManuscriptTextView`, `ViewportSceneLoader`, and their interaction with `SceneNavigatorView`. All EP-011 implementation tasks (T-0114–T-0117, T-0119) are governed by these rules. Any contradiction between this document and the code is a bug in the code.

---

## 2. Viewport Memory Model

**Rule:** All manuscript scenes are loaded into `NSTextStorage` at once when a project is opened.

- The dynamic load/release cycle (`fillForward`, `fillBackward`, `scrollPromoteTo`) is removed.
- `loadInitial()` becomes `loadAll()`: iterate `allScenes` in order, read each scene's content, append to `NSTextStorage` with a separator between each pair.
- `rebuildStorage` is called exactly once on open, and again only on structural changes: scene create, scene delete, chapter create, chapter delete, scene reorder, chapter reorder, chapter title toggle. It is **never** triggered by scroll or cursor movement.
- `saveAndRelease` is renamed `saveScene` and no longer removes the segment from memory. Segments remain in memory for the lifetime of the project session.
- **Memory eviction** (for very large manuscripts) is explicitly deferred and not implemented in SP-033.

---

## 3. Scroll Coordinate Model

**Rule:** AppKit owns scroll position. SwiftUI does not drive or override it.

- `NSScrollView` / `NSTextView` manage the scroll offset natively.
- Swift/SwiftUI may read the scroll offset (via delegate or KVO) to update UI state (e.g., a future scroll position indicator), but never writes to it.
- The `scrollDidChange` notification observer and `scrollPromoteTo` are removed with the load/release cycle.
- With all scenes in memory, `NSScrollView`'s scroll bar thumb automatically reflects the author's true position in the full manuscript — no custom `NSScroller` logic is needed in SP-033.

**Author experience:** Scrolling through the manuscript feels like scrolling through a single continuous document. Scene boundaries are invisible to the scroll gesture.

---

## 4. Scene Separator

### 4.1 Implementation

**Rule:** Scene separators are `NSTextAttachment` objects (one per inter-scene boundary).

- Each separator is a single attachment character (Unicode U+FFFC) in `NSTextStorage`, followed by a `\n`.
- The attachment carries a `DividerAttachmentCell` (already implemented) that draws a 1pt horizontal rule across the text column with 24pt height.
- Separators are inserted by `rebuildStorage` between each pair of adjacent scenes. They are never stored in scene content files.
- The `NSTextLayoutManagerDelegate` approach was rejected: it provides no character position for the cursor to rest in, requiring overrides of every cursor movement method.

### 4.2 Position Map Rule

Each separator occupies exactly 2 characters in `NSTextStorage` (attachment + `\n`). The position map must account for this:

**Manuscript position = NSTextStorage offset − (number of separator pairs before that offset)**

Where "separator pair" = the attachment character + the following `\n` (2 chars total).

This transform is applied in two places:
1. **Building `sceneStartMap`**: after `rebuildStorage`, compute each scene's NSTextStorage start offset, then subtract preceding separator chars to get the manuscript position.
2. **Translating cursor offset**: when `textViewDidChangeSelection` fires, subtract separator chars before the cursor to get manuscript position.

### 4.3 Separator Exclusions

Separators must be excluded from:
- Word count and character count
- Copy/paste (the attachment char must not be included in copied text — handled by `NSTextView` naturally for non-selectable attachments, but verify)
- Find/replace

---

## 5. Cursor Coordinate Spaces

Two coordinate spaces exist simultaneously for any cursor position:

| Space | Definition | Example (scenes: 241 / 619 / 128 / 583 chars) |
|-------|-----------|------------------------------------------------|
| **Manuscript position** | Character offset from the first character of Scene 1 in stored text (separators excluded) | Start of Scene 3 = 860 |
| **Scene position** | Character offset within the owning scene's stored text | Start of Scene 3 = 0 |

- Cursor position is **device-local**: not persisted, not shared between collaborators.
- For every scene except the one currently containing the cursor, scene-level cursor position is **null**.
- `sceneStartMap[sceneID]` = manuscript position of the first character of that scene.

---

## 6. Scene Start Position Map

`sceneStartMap: [String: Int]` is maintained on `ViewportSceneLoader`.

- Built (or rebuilt) at the end of every `rebuildStorage` call.
- Keys are `sceneID` strings. Values are manuscript-coordinate character offsets (separator chars excluded).
- Used by: cursor placement after navigate/delete (T-0116), Navigator highlight derivation (T-0115), and future scroll bar fidelity (T-0118).

**Build algorithm:**
```
manuscriptOffset = 0
for scene in allScenes:
    sceneStartMap[scene.sceneID] = manuscriptOffset
    manuscriptOffset += scene.text.count   // stored chars only, no separators
```

---

## 7. Boundary Cursor Rules

The separator absorbs the scene-boundary ambiguity. The cursor can visibly rest in the separator between Scene N and Scene N+1.

| Gesture | Result |
|---------|--------|
| Arrow right/down from end of Scene N | Cursor enters separator (logically still "end of Scene N") |
| Arrow right/down from separator | Cursor moves to beginning of Scene N+1 |
| Arrow left/up from beginning of Scene N+1 | Cursor enters separator |
| Arrow left/up from separator | Cursor moves to end of Scene N |
| Click within a scene | `NSTextView` places cursor natively — no boundary logic needed |
| Click on separator | Cursor rests in separator; ownership resolved by next arrow/typing |
| Typing while cursor is in separator | Text inserted into Scene N+1 (forward bias) |

The Scene Navigator always highlights the current scene, giving the writer unambiguous visual feedback about which scene she is typing into.

---

## 8. Delete Rules

The separator is **non-deletable**. Scenes cannot be merged via keyboard input.

| Cursor position | Delete backward (⌫) | Delete forward (⌦) |
|----------------|---------------------|---------------------|
| At beginning of Scene N | No effect | Deletes first char of Scene N |
| In separator | Deletes last char of Scene N | No effect on separator; deletes first char of Scene N+1 |
| At end of Scene N | Deletes last char of Scene N | No effect |

**Implementation:** Override `deleteBackward:` and `deleteForward:` in `ManuscriptNSTextView`. Before delegating to `super`, check whether the cursor is at a separator boundary using `recomputeBoundaries` and the known attachment character positions. If at a protected boundary, return without calling `super`.

---

## 9. First-Responder Transfer Model

**Rule:** AppKit owns first-responder transfer. The `focusManuscriptView` SwiftUI binding is removed.

- `ManuscriptNSTextView` (or its `Coordinator`) exposes a method `takeFocus()` that calls `window?.makeFirstResponder(self)` directly.
- All callers that previously set `focusManuscriptView = true` are updated to call `takeFocus()` instead.
- The `@Binding var focusManuscriptView: Bool` property on `ManuscriptTextView` and all its call sites are removed entirely.

---

## 10. Navigator Highlight and `currentSceneID`

- `ViewportSceneLoader` publishes `currentSceneID: String?` derived from the cursor's manuscript position and `sceneStartMap`.
- Updated on every `textViewDidChangeSelection` callback.
- `SceneNavigatorView` reads `currentSceneID` to set `selectedRowID`, replacing the current `loader.currentSegment?.sceneID` observation where applicable.

---

## 11. Cursor Placement After Navigate and Delete

### Navigator tap
- Cursor is placed at `sceneStartMap[sceneID]` translated to NSTextStorage offset (add preceding separator chars).
- `NSTextView.setSelectedRange` + `scrollRangeToVisible` are called directly from the tap handler.
- `navigateToSceneID` binding is retained for this purpose (it already works; its implementation is updated to use the map).

### Scene delete (non-last scene)
- After `removeScene`, the next scene in manuscript order becomes current.
- Cursor is placed at `sceneStartMap[nextSceneID]` (translated to NSTextStorage offset).
- `takeFocus()` is called immediately after placement.

### Scene delete (last scene)
- Cursor is placed at `NSTextStorage.length` (end of document).
- `takeFocus()` is called immediately after placement.

---

## 12. Open Questions

None. All scope questions from EP-011 planning are resolved.

---

## 13. Files Affected by SP-033

| File | Change |
|------|--------|
| `Scrivi/Views/ViewportSceneLoader.swift` | Remove fill/release cycle; add `loadAll()`; add `sceneStartMap`; add `currentSceneID`; remove `scrollPromoteTo` |
| `Scrivi/Views/ManuscriptTextView.swift` | Remove `focusManuscriptView` binding; add `takeFocus()`; override `deleteBackward:`/`deleteForward:`; update `textViewDidChangeSelection` to publish manuscript position; update `navigateToSceneID` handler to use map |
| `Scrivi/Views/SceneNavigatorView.swift` | Update delete handler to call `takeFocus()` instead of `onDeleteNavigate`; update Navigator highlight to consume `currentSceneID` |
| `Scrivi/App/AppEnvironment.swift` | Remove `focusManuscriptView` state if present |
| `docs/` | This file (T-0113) |
