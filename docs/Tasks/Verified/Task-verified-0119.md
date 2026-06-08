## T-0119: EP-011 verification — macOS smoke test + deferred EP-010 AC

**Status:** ✅ Implemented - Verified
**Component:** All EP-011 components
**Priority:** High
**Date Requested:** 2026-06-08
**Sprint Assigned:** SP-033

**Rationale:**
End-to-end verification that all EP-011 SP-033 tasks (T-0113–T-0117) work together correctly in a real macOS session. Also covers the EP-010 acceptance criterion deferred from SP-032.

**Current Behavior:**
EP-011 work not yet started.

**Desired Behavior:**
All of the following pass in a live macOS Scrivi session:

**EP-011 acceptance criteria:**
1. Scroll past 3+ scene boundaries — no scroll jump, position preserved
2. Cursor rests visibly in the separator when arrowed to a scene boundary
3. Arrow right from separator → cursor moves to Scene N+1; arrow left → cursor moves to Scene N
4. Typing while cursor is in separator → text inserted into Scene N+1
5. Delete (backward) at scene start → no effect
6. Delete (backward) from separator → deletes last character of Scene N
7. Navigator click places cursor at first character of correct scene
8. Scene delete (non-last) → cursor at first character of next scene, focus in text view
9. Scene delete (last) → cursor at end of manuscript, focus in text view
10. `focusManuscriptView` binding is gone; no SwiftUI focus workaround in code

**Deferred EP-010 AC (from SP-032):**
11. Chapter title toggle and scene headings work correctly in the all-in-memory model

**Requirements:**
1. All 11 criteria pass manually
2. `swift test` green
3. `ctest` green
4. No regressions in existing Scene Navigator, drag reorder, or save behavior

**Test Steps:**
Run each of the 11 criteria above in a live macOS session. Document results.

**Notes:**
This task is the gate for EP-011 SP-033 close. It must be the last task completed in SP-033.
