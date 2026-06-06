# Active Epics

## EP-011: Writing Surface Behavior — Scroll, Cursor, and Focus

**Status:** 🔵 Draft
**Goal:** Redesign the interaction model of `ManuscriptTextView` and `ViewportSceneLoader` so that scroll position, cursor placement, and keyboard focus behave correctly and predictably for an author working through a multi-scene manuscript. The current scroll-driven scene promotion mechanism jumps the viewport to an unexpected position when a scene boundary is crossed while scrolling; cursor placement after structural operations (delete, navigate) is imprecise; and keyboard focus does not transfer reliably. This Epic defines the correct behavior from first principles and implements it.
**Date Created:** 2026-06-06
**Start Date:** —
**Target Close Date:** TBD
**Actual Close Date:** —

### Known Defects Driving This Epic

1. **Scroll jump on scene promotion:** When the author scrolls upward past a scene boundary, the departing scene is saved and released and the viewport loader promotes the previous scene to current. This triggers a `rebuildStorage` call which resets the scroll position — the author's reading position is lost and the view jumps to an unexpected location mid-scene.

2. **Cursor placement after navigate/delete:** After a Navigator tap or a scene delete, the cursor lands at the start of the scene's character range in the NSTextStorage, but the scene start position is not reliably tracked relative to the overall storage layout when chapter heading content is present.

3. **Focus transfer after delete:** The `focusManuscriptView` binding mechanism added in SP-032 is a workaround; a cleaner first-responder transfer model is needed.

### Scope (to be defined in Sprint planning)

Questions to answer before Sprint 1:
- Should the viewport loader hold all scenes in memory (eliminating the dynamic load/release cycle) for manuscripts up to a reasonable size threshold? Or retain load/release but fix the position restoration?
- What is the correct scroll behavior when a scene boundary is crossed: preserve absolute scroll Y, preserve relative position within the scene, or something else?
- Should `NSTextView` scroll position be owned by the SwiftUI layer or the AppKit layer?
- How should cursor position be tracked globally (character offset into NSTextStorage)?
- How should the scene start position map be maintained and exposed to the rest of the app (see T-0112)?

### Candidate Tasks (to be formalized in Sprint planning)

| Candidate | Description |
| --------- | ----------- |
| TC-A | Define and document the precise scroll, cursor, and focus behavior spec |
| TC-B | Implement the chosen scroll/viewport strategy |
| TC-C | Implement global cursor position tracking and scene start position map (T-0112) |
| TC-D | Implement correct cursor placement after navigate and delete |
| TC-E | Implement reliable first-responder transfer |
| TC-F | Scroll bar fidelity (T-0096) |
| TC-G | Verification and macOS smoke test (including the deferred EP-010 AC) |

### Sprints

*None yet — planning not started.*

### Tasks

*None yet — to be defined during Sprint planning.*

---

*Last Updated: 2026-06-06 (EP-011 created; EP-010 closed)*
