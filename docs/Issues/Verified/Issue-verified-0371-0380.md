# Verified Issues: I-0371 – I-0380

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0361–I-0370 are in [`Issue-verified-0361-0370.md`](Issue-verified-0361-0370.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0371** | `[Apple]` The object detail sheet on macOS showed only Save and Close: Undo Changes and SP-239's ⋯ (Change to Artifact/Item, Delete) were missing (user, 2026-11-23, SP-239 live pass step 2). **Cause:** the sheet draws `.cancellationAction` items in a **bottom** button bar, which shows one view per placement: the group showed only Save, and so did three separate items (2nd try). **Fix:** one `ToolbarItem` holding an `HStack` (`Scrivi/Views/Detail/ObjectDetailSheet.swift:452`). macOS build green; user saw all four buttons, Mac, iPad and iPhone. | Medium | [SP-239] | ✅ **Verified** (2026-11-23) |
| **I-0372** | `[Apple]` The object detail sheet's bottom bar (Save · Undo Changes · ⋯ · Close) was missing on every object opened FROM the sheet — a related object pushed by double-click showed no buttons until Back returned to the first (user, 2026-11-23: *"in step 6 when I double clicked L, L appeared, but there were no bottom bar buttons.  Same for the subsequent double click on test key.  Back button twice and it showed."*). **Cause:** `.toolbar { detailToolbar }` was attached to the `NavigationStack`'s root view only; toolbar items belong to the view that declares them, so each `navigationDestination` level had none. Likely since [T-0547] (2026-09-23). **Fix:** the destination's `editorSurface` declares the same toolbar (`Scrivi/Views/Detail/ObjectDetailSheet.swift:216`). re-pass: the bar on every pushed level, also with L → test key loaded twice and the second deleted (user, 2026-11-23). | Medium | [SP-240] | ✅ **Verified** (2026-11-23) |

*Last Updated: 2026-11-23 (**I-0372** archived with [SP-240]'s close.)*

*Last Updated: 2026-11-23 (**I-0371** archived with [SP-239]'s close.)*
