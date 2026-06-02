---
id: SP-023
title: "Writing Surface — NSTextView, Viewport Loader, and Auto-Save"
epic: EP-009
status: ✅ Closed
start_date: 2026-06-01
end_date: 2026-06-01
---

## Goal

Replace the current single-scene `TrackingTextEditor` with a viewport-based writing surface that presents the manuscript as continuous text. The author sees the current scene, its neighbors, and thin horizontal rule dividers — no system-generated text on the writing surface. Auto-save fires automatically; the author never thinks about saving.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0077 | `ViewportSceneLoader` — load current scene + neighbors until viewport fills + 1 buffer scene each direction | 🔵 Planned |
| T-0078 | `ManuscriptTextView` — `NSViewRepresentable` wrapping `NSTextView`; displays concatenated scene segments separated by `NSTextAttachment` divider views | 🔵 Planned |
| T-0079 | Divider rendering — thin horizontal rule (1pt, system separator color) with no text, no labels; implemented as a custom `NSTextAttachment` or paragraph style border | 🔵 Planned |
| T-0080 | Auto-save — 1-second debounce `Task` after last keystroke; scene-exit immediate save; app-resign immediate save + backup placeholder hook | 🔵 Planned |
| T-0081 | `⌘↩` key binding — save current scene, call `createScene`, insert divider, move cursor to new scene segment | 🔵 Planned |
| T-0082 | `⌘⇧↩` key binding — save current scene, call `createChapter`, insert divider, move cursor to new chapter's first scene | 🔵 Planned |

## Acceptance Criteria

- [ ] Opening a project presents a full-height plain-text editor with the first scene's content loaded and the cursor ready
- [ ] The viewport loader loads scenes forward and backward from the current scene until the combined content fills the visible area plus one scene of buffer in each direction; short or empty scenes are absorbed transparently (the loader continues until the buffer is filled, not until a fixed count is reached)
- [ ] Scene segments are joined with a thin horizontal rule divider; no text of any kind appears in the divider
- [ ] Auto-save fires 1 second after the last keystroke in the currently active scene segment; the author is never prompted to save
- [ ] When the author scrolls a scene out of the viewport window (scene-exit), that scene is saved immediately before being released from memory
- [ ] On app resign (`NSApplicationDelegate.applicationWillResignActive` or equivalent), the current scene is saved immediately; a `onAppResign()` hook stub exists in `AppEnvironment` for a future backup operation (body is a `// TODO: backup` comment, not implemented)
- [ ] `⌘↩` inserts a new scene: saves the current scene content, calls `createScene`, inserts a divider in the text view, positions the cursor at the start of the new empty segment
- [ ] `⌘⇧↩` inserts a new chapter: saves the current scene content, calls `createChapter`, inserts a divider, positions the cursor at the start of the new chapter's first scene
- [ ] The editor text is plain text / raw Markdown — no rendering, no bold, no syntax colouring applied to author text
- [ ] All 165 ctests remain green; the macOS app builds and runs without crash

## Notes

- `ViewportSceneLoader` is a Swift `@Observable` class (or actor) that holds the ordered array of loaded `SceneSegment` values and the `AppEnvironment` reference. It calls `openScene` via `ScriviEngine` as scenes enter the buffer.
- `ManuscriptTextView` wraps a single `NSTextView` whose `NSTextStorage` is managed by `ViewportSceneLoader`. Scene boundaries are tracked as character ranges, not separate text views.
- The backup hook stub is specifically deferred to a future Epic — do not implement backup logic.
- `EditorView.swift` is superseded by the new `ManuscriptTextView` wrapper; the old `TrackingTextEditor` struct is removed.

---

## Completion Summary

All 6 tasks implemented and verified. `ViewportSceneLoader` and `ManuscriptTextView` replace the old single-scene `TrackingTextEditor`. Viewport-based scene loading, `DividerAttachmentCell` dividers, 1s debounce auto-save, scene-exit save, app-resign save + backup stub, and `⌘↩`/`⌘⇧↩` key bindings all implemented. 21/21 Swift tests and 171/171 ctests green.

---

*Last Updated: 2026-06-01 (closed; all tasks verified)*
