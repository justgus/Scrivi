# Closed Issue (Not Verified) — I-0274

## I-0274: `[Apple]` Control-Return writes U+2028 into the scene file

**Status:** ⚪ **CLOSED — Not Verified (2026-10-04, user-directed): NOT A DEFECT**
**Reason for Closure:** ⛔ **Filed on a MIS-MEASUREMENT.** No default key reaches `insertLineBreak:` in the app.
**Date Identified:** 2026-10-04 · **Date Closed:** 2026-10-04 · **Found by:** [SP-156] AC-measure

---

### ✅ THE RULING

✅ **User, 2026-10-04:** *"Ctrl-Enter on Macos brings up a context menu (Ask Siri, Cut, Copy Paste, etc). Cmd-Enter
on Macos will Split the Chapter. Not sure what you are talking about."* → *"yes, close I-0274 as not a defect."*

### ⛔ WHY IT WAS FILED — the error, recorded

⚠️ The [SP-156] harness called `NSTextView.keyDown(with:)` DIRECTLY. That skips `NSApplication`'s key-equivalent
pass (menus — ⌘-Return is Split Chapter) and macOS's system shortcuts (Control-Return opens the context menu).
⛔ So the harness reported Control-Return → `insertLineBreak:` → U+2028, a path the real app never takes.
⚠️ Same class as [T-0579]'s `.string` defect: the test chose the dispatch, AppKit did not.

### What the record contained (kept, not acted on)

| **I-0274** | `[Apple]` ⛔ **PREMISE WRONG (user, 2026-10-04) — CLOSE AS NOT-A-DEFECT RECOMMENDED.** In the app, Control-Return opens the system context menu and ⌘-Return runs Split Chapter; ⛔ the harness below called `keyDown` directly and skipped both. No default key reaches `insertLineBreak:`. ---- *Original filing:* ⚠️ **Control-Return writes U+2028 (LINE SEPARATOR) into the scene file.** ✅ **MEASURED 2026-10-04 (SP-156 AC-measure, AppKit's own key dispatch on a TextKit 2 `NSTextView`):** Control-Return sends `insertLineBreak:`, which inserts U+2028 — ⛔ **not a newline**, so the stored Markdown gets an invisible non-ASCII character that CommonMark does not treat as a line ending, Linux shows as an unknown glyph or nothing, and `grep`/diff tools treat as part of the line. ⚠️ **Pre-existing; NOT introduced by SP-156** (`ManuscriptNSTextView` never handled it). ⚠️ The same path is reachable from any key binding that sends `insertLineBreak:`. ✅ **Options to rule:** (a) map `insertLineBreak:` to Return's `\n\n`; (b) map it to a deliberate hard break (`\` + `\n`, hidden — matches the meaning of "line break"); (c) make it a no-op. ⚠️ **Existing files are not scanned for U+2028 (R2).** | Low | Not Assigned |
