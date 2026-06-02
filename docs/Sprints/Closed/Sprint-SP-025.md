---
id: SP-025
title: "Scroll-Driven Scene Switching and EP-009 Close"
epic: EP-009
status: 🔵 Planning
start_date: TBD
end_date: TBD
---

## Goal

Wire scroll-driven scene promotion: scrolling past a divider makes the next or previous scene the current scene (saving and releasing the scene that scrolled out of the buffer). Verify the complete EP-009 acceptance criteria and close the Epic.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0087 | Scroll-driven scene switching — scrolling down past a divider promotes the next scene to current; the scene that scrolled off the top is saved and released from the viewport buffer | 🔵 Planned |
| T-0088 | Scroll-driven scene switching — scrolling up past a divider promotes the previous scene to current; the scene that scrolled off the bottom is saved and released | 🔵 Planned |
| T-0089 | EP-009 verification — all 16 acceptance criteria checked; all 165 ctests green; macOS end-to-end smoke test (create project, type across 3 scenes, use Navigator, scroll between scenes) | 🔵 Planned |

## Acceptance Criteria

- [ ] Scrolling down past a scene divider: the scene above the divider becomes the previous scene in the buffer, the scene below becomes the new current scene, and the scene that scrolled off the top edge is saved immediately and released from memory
- [ ] Scrolling up past a scene divider: the scene below the divider becomes the next scene in the buffer, the scene above becomes the new current scene, and the scene that scrolled off the bottom edge is saved immediately and released from memory
- [ ] Scene switching is driven by scroll position, not by explicit user action — the author does not need to do anything except scroll
- [ ] The Navigator selection highlight updates to reflect the new current scene when scroll-driven switching occurs
- [ ] All 16 EP-009 acceptance criteria are verified green (see Epic-active.md)
- [ ] All 165 ctests remain green
- [ ] End-to-end smoke test on macOS: open project → type in scene 1 → `⌘↩` to create scene 2 → type in scene 2 → scroll up to scene 1 → verify scene 1 text preserved → click scene 2 in Navigator → verify cursor lands in scene 2

## Notes

- "Scrolling past a divider" is detected by observing the `NSScrollView` content offset and comparing it against the stored character range boundaries of each divider in the `NSTextView`. The scene that owns the range crossing the top edge of the visible rect is the current scene.
- Scroll-driven switching must debounce the save-and-release slightly (suggested: 100ms after the scroll position stabilizes) to avoid thrashing on fast scroll.
- If EP-009 verification reveals gaps, additional tasks are created inline in this Sprint before it closes.
- On Sprint close, update `Epic-active.md` to 🟠 Complete and request user approval to close EP-009.

---

*Last Updated: 2026-06-01 (created as Planning)*
