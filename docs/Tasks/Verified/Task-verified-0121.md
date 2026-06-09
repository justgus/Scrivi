# T-0121: Add Edit menu with standard responder-chain commands

**Status:** ✅ Verified
**Epic:** EP-012
**Sprint:** SP-035

**Description:**
The system Edit menu (Undo, Redo, Cut, Copy, Paste, Select All) is left intact — no `CommandGroup` replacement. AppKit's `NSTextView` wires all Edit commands to the first-responder chain automatically. A previous attempt to replace the undo/redo group with empty closures was removed, restoring native undo/redo behaviour.

**Files Affected:**
- `Scrivi/App/ScriviApp.swift` — empty `CommandGroup(replacing: .undoRedo)` removed

**Verification:**
- ✅ Edit menu present with standard items
- ✅ Cut, Copy, Paste work in the manuscript editor
