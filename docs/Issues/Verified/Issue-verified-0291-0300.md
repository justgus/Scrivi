# Verified Issues: I-0291 – I-0300

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0281–I-0290 are in [`Issue-verified-0281-0290.md`](Issue-verified-0281-0290.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0291** | `[Apple]` ⚠️ **The Buffers palette's button switched to PASTE on ⌃ alone after paste moved to ⌃⌘ ([I-0290]).** Reported by the user 2026-10-12. ✅ Fixed: it follows ⌃⌘, as the chord does. → [details](#i-0291) | **Low** | [SP-179] | ✅ **VERIFIED 2026-10-12** |

---

## I-0291

**Title:** The Buffers palette's button switches to paste on ⌃ alone, not ⌃⌘
**Status:** ✅ **VERIFIED 2026-10-12** (user, Mac: *"I-0291 passes"* — the four steps below, including the ⌃⌘-click paste).
**Platform:** `[Apple]`
**Component:** `BuffersPalette.swift` `BufferAction.current`
**Severity:** Low — the palette's paste was reachable, just on a modifier that no longer matches the chord
**Sprint:** [SP-179]
**Date Identified:** 2026-10-12 — user, on the Mac: *"I-0290 passes. But the view buffers icon changes on holding Ctrl down, not
Ctrl-Cmd."*

**Root cause:** my choice in [I-0290] — I kept the palette on ⌃ alone (*"a click, not a chord"*) and said so; it does not match
the chord the hint names.

**Fix:** `BufferAction.current` returns paste only for ⌃ AND ⌘; ⌃ alone → copy. Hint: *"hold ⌃⌘ or ⌥ to change the button"*.
`BufferAction` made internal (was private) so a test can reach it.

**Tests:** `BufferPasteChordTests.paletteButtonFollowsTheChord` — FAILED on the old code (⌃ alone → paste), passes now. Interop
284/284. ✅ A ⌃⌘-CLICK on the button fires it — checked by the user on the Mac (verify step 4); no test covers the click.

**To verify (Mac):** open View ▸ Show Buffers. Hold ⌃ → the icons stay Copy. Hold ⌃⌘ → Paste icon; click a filled slot's button
with ⌃⌘ held → it pastes at the caret.

---

*Last Updated: 2026-10-12 (batch created: **I-0291** VERIFIED (user).)*
