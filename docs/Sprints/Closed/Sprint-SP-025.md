---
id: SP-025
title: "Scroll-Driven Scene Switching and EP-009 Close"
epic: EP-009
status: ✅ Closed
start_date: 2026-06-03
end_date: 2026-06-03
---

## Goal

Wire scroll-driven scene promotion: scrolling past a divider makes the next or previous scene the current scene, saving the departing scene. Verify all EP-009 acceptance criteria and close the Epic.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0088 | Scroll-down past divider — promote next scene, save departing scene | ✅ Verified |
| T-0089 | Scroll-up past divider — promote previous scene, save departing scene | ✅ Verified |
| T-0095 | EP-009 verification — all 18 ACs green; 171 ctests green; macOS smoke test | ✅ Verified |

## Acceptance Criteria

- [x] Scrolling down past a scene divider promotes the next scene to current; the departing scene is saved immediately
- [x] Scrolling up past a scene divider promotes the previous scene to current; the departing scene is saved immediately
- [x] Scene switching is driven by scroll position — the author does nothing except scroll
- [x] The Navigator selection highlight updates to reflect the new current scene on scroll-driven switching
- [x] All 18 EP-009 acceptance criteria verified green
- [x] 171/171 ctests green
- [x] End-to-end smoke test passed

## Notes

- T-0088 and T-0089 implemented together via the same `scrollDidChange` mechanism.
- `fillForward`/`fillBackward` updated to guarantee at least one buffer scene in each direction regardless of viewport budget, fixing the long-scene adjacency gap.
- T-0096 (scroll bar fidelity) added to backlog during this sprint — deferred to a future Epic.
- ctest count grew from 165 to 171 since EP-009 was written; all 171 pass.

---

*Closed: 2026-06-03*
