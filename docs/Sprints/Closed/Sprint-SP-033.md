# SP-033: Writing Surface — All-in-Memory Viewport, Separator, Cursor, and Focus

**Status:** ✅ Closed
**Epic:** EP-011: Writing Surface Behavior — Scroll, Cursor, and Focus
**Goal:** Replace the dynamic scene load/release cycle with an all-in-memory model, introduce the virtual inter-scene separator, implement global cursor tracking and correct cursor placement, and replace the `focusManuscriptView` workaround with a direct first-responder model.
**Start Date:** 2026-06-08
**End Date:** 2026-06-08
**Capacity:** Single session

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0113 | EP-011 Behavior Spec — scroll, cursor, separator, focus, delete rules | High | ✅ Verified |
| T-0114 | All-scenes-in-memory viewport strategy — replace load/release cycle | High | ✅ Verified |
| T-0115 | Global cursor position tracking + scene start position map | High | ✅ Verified |
| T-0116 | Correct cursor placement after navigate and delete | High | ✅ Verified |
| T-0117 | Reliable first-responder transfer — replace `focusManuscriptView` workaround | High | ✅ Verified |
| T-0119 | EP-011 verification — macOS smoke test + deferred EP-010 AC | High | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0010 | Cursor jumps to scene start when crossing a scene boundary | High | ✅ Verified |
| I-0011 | Chapter title headings editable; text leaks into scene content on toggle | High | ✅ Verified |
| I-0012 | First chapter title never shown when "Show Chapter Titles" is enabled | Medium | ✅ Verified |
| I-0013 | Project settings (toggle, title, subtitle) do not persist between app launches | High | ✅ Verified |
| I-0014 | Navigation panel shows internal project ID instead of project title | High | ✅ Verified |
| I-0015 | No way to edit project title or subtitle | Medium | ✅ Verified |
| I-0016 | Scene Navigator selection on load — superseded by I-0018 (backlog) | Medium | ⚪ Returned to Backlog |

### Returned to Backlog

| ID | Title | Reason |
| -- | ----- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | Deferred to SP-034 |
| I-0017 | Window maximized state not restored on app relaunch | Not resolved — partial fix only |
| I-0018 | Scene Navigator shows no selection on app load | Not resolved — requires layout hook not yet available |

### Retrospective

**Completed:**
- All-in-memory viewport model replacing the load/release cycle
- Virtual inter-scene separator (NSTextAttachment) with non-editable, non-deletable behavior
- Global cursor position tracking and scene start position map
- Correct cursor placement after navigate and delete (using map-based storage offsets)
- Reliable first-responder transfer via `takeFocus()` — `focusManuscriptView` binding removed
- Scroll-driven viewport scene tracking (Navigator highlight follows scroll without moving cursor)
- Navigator tap scrolls to scene without moving cursor
- Chapter title headings non-editable (`.scriviHeading` attribute + `shouldChangeText` override)
- First chapter title now rendered when toggle is enabled
- Project settings persistence (`ProjectPreferences` to `UserDefaults`)
- Project title/subtitle display in Navigator and editing in Project Settings
- Window frame (position/size) persistence across launches
- Issues I-0010–I-0015 all identified, fixed, and verified

**Returned to Backlog:**
- T-0118 (scroll bar fidelity) — deferred; evaluate after SP-034
- I-0017 (window maximized state) — SwiftUI `WindowGroup` overrides `zoom(nil)` after layout; needs deeper AppKit hook
- I-0018 (Navigator no selection on load) — scroll observer fires only on user scroll; needs post-layout callback

**What went well:**
- The `@ObservationIgnored` split between cursor-tracking and viewport-display state cleanly solved the SwiftUI feedback loop
- Separating `viewportSceneID` (observable, Navigator) from `cursorSceneID` (non-observable, AppKit) is the right long-term architecture
- `ProjectPreferences` as a `UserDefaults`-backed `@Observable` is simple and correct

**What to improve:**
- The delete guard logic in `ManuscriptNSTextView` was over-engineered on first pass; the `isSeparatorPosition` helper is the right abstraction
- Window persistence should be tackled earlier in future sprints before the UI stabilizes

**Carry-forward notes:**
- SP-034 should evaluate whether the all-in-memory model already gives accurate NSScrollView scroll bar behavior before implementing T-0118
- I-0017 and I-0018 are low-severity; can be deferred past SP-034 if SP-034 is focused on scroll bar fidelity

---

*Closed: 2026-06-08*
