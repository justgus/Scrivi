# T-0155: Expand Timeline Forward/Backward Popover

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift` — `ExpandTimelinePopover`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**
`ExpandTimelinePopover` presents when a drag exceeds the current timeline bounds. It offers a `[amount] [Unit▾]` picker with Minutes/Hours/Days/Weeks/Months/Years and Cancel/Expand buttons. On Expand, the dragged dot is committed at the new out-of-bounds position. The popover is anchored on `panelContent` via `showExpandForward`/`showExpandBackward` state flags checked in `onDragEnd`.
