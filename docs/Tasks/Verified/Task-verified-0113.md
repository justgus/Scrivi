## T-0113: EP-011 Behavior Spec — scroll, cursor, separator, focus, delete rules

**Status:** ✅ Implemented - Verified
**Date Implemented:** 2026-06-08

**Implementation Notes:**
Spec written at `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md`. All rules captured; separator approach confirmed as NSTextAttachment; Q3 resolved; no open questions remain.
**Component:** `ManuscriptTextView`, `ViewportSceneLoader`, design documentation
**Priority:** High
**Date Requested:** 2026-06-08
**Sprint Assigned:** SP-033

**Rationale:**
Before any code changes, the correct behavior of scroll, cursor, separator, first-responder, and delete must be written down precisely so that implementation tasks (T-0114–T-0117) have unambiguous targets.

**Current Behavior:**
Behavior is implicitly defined by existing code, which has known defects (scroll jump, imprecise cursor placement, unreliable focus). No authoritative spec document exists.

**Desired Behavior:**
A spec document exists in `docs/` that states:
- Viewport memory model (all-in-memory, eviction threshold)
- Scroll coordinate model (absolute manuscript Y, AppKit-owned)
- Scene separator design (visual bar, virtual blank line, **NSTextAttachment** — decided 2026-06-08)
- Cursor coordinate spaces (manuscript position, scene position, null scene cursor)
- Boundary cursor rules (arrive-forward / arrive-backward / click / typing)
- Delete rules (non-deletable separator, backward-from-separator, forward-from-separator)
- First-responder transfer model

**Requirements:**
1. Spec document created at `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md`
2. All rules from EP-011 Scope section are captured verbatim or refined
3. Q3 (scroll ownership) resolution is documented with rationale
4. Separator implementation is documented as **NSTextAttachment** (decided 2026-06-08; TextKit 2 delegate approach rejected — would require overriding all cursor movement methods to synthesize a fake cursor position)
5. Open questions from Q3 (if any remain) are listed explicitly

**Design Approach:**
Write the spec first; do not write code in this task. The spec becomes the acceptance criteria for T-0114–T-0117.

**Components Affected:**
- `docs/`: new spec file

**Test Steps:**
1. Open `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md`
2. Confirm all rules from EP-011 Scope section are present
3. Confirm separator implementation approach is chosen (not left open)
4. Confirm Q3 rationale is stated

**Notes:**
This task is the gate for all other SP-033 implementation tasks. It should be completed first.
