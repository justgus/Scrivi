# Verified Issues: I-0371 – I-0380

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0361–I-0370 are in [`Issue-verified-0361-0370.md`](Issue-verified-0361-0370.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0371** | `[Apple]` The object detail sheet on macOS showed only Save and Close: Undo Changes and SP-239's ⋯ (Change to Artifact/Item, Delete) were missing (user, 2026-11-23, SP-239 live pass step 2). **Cause:** the sheet draws `.cancellationAction` items in a **bottom** button bar, which shows one view per placement: the group showed only Save, and so did three separate items (2nd try). **Fix:** one `ToolbarItem` holding an `HStack` (`Scrivi/Views/Detail/ObjectDetailSheet.swift:452`). macOS build green; user saw all four buttons, Mac, iPad and iPhone. | Medium | [SP-239] | ✅ **Verified** (2026-11-23) |

*Last Updated: 2026-11-23 (**I-0371** archived with [SP-239]'s close.)*
