# Verified Issues: I-0301 – I-0310

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0291–I-0300 are in [`Issue-verified-0291-0300.md`](Issue-verified-0291-0300.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0302** | `[Apple]` ⚠️ **When the newest history step is a barrier, ⌘Z only flashes the screen — the barrier's notice (e.g. "This scene was changed outside Scrivi; undo stops here") never appears.** Linux shows it. Reported by the user 2026-10-14. ✅ Fixed: Undo stays available at a barrier, on both platforms. → [details](#i-0302) | **Low** | [SP-180] | ✅ **VERIFIED 2026-10-15** |

---

## I-0302

**Title:** On the Mac, ⌘Z at a barrier flashes instead of showing the barrier's notice
**Status:** ✅ **VERIFIED 2026-10-15** (user, Mac: *"I-0302 passes, the dialog appeared"* — ⌘Z in dumas-prose-timelines at the 14:56 barrier). ⚠️ **The Linux half (the Edit ▸ Undo menu item) is covered by `undo_smoke` only, not checked live on the rig.** Fixed 2026-10-15 in [SP-180] (user: *"Ok fix it in SP-180."*)
**Platform:** `[Apple]` (and Linux's Edit ▸ Undo menu item, the same rule)
**Component:** `ManuscriptTextView.swift` `validateUserInterfaceItem` (`#selector(undo(_:))` → `coordinator.canUndo`)
**Severity:** Low — nothing is lost, but the writer is told nothing about why undo does nothing
**Date Identified:** 2026-10-14 (user: *"I load the project and the very first thing I do is type Cmd-Z. The screen flashes as if
there is nothing to undo … I get no dialogs. Just the flash."*)

**Cause (read; matches the project's log):**
- The newest step of dumas-prose-timelines' history is an `externalChange` barrier (14:56 local, 2026-10-14), from a session
  that ended without closing under a build without [I-0299]'s fix.
- The core reports `canUndo: false` at a hard barrier. The Mac DISABLES Edit ▸ Undo on that, so ⌘Z only beeps or flashes,
  and `apply(step:)`'s barrier notice is never reached.
- Linux calls undo regardless and shows the notice (`EditorShell::applyHistoryStep`).

**Fix direction:** the core's capture state says when the newest step is a barrier (`atBarrier`). The Mac enables Undo
then, and ⌘Z shows the barrier's own notice, as Linux does. A failing interop test first.

**Resolution (2026-10-15):**
- **ScriviCore:** every capture result carries `atBarrier`: the newest step is a hard barrier
  (`HistoryService::atBarrier`, added once in `withCapture`). A reversible structural step is not one.
- **Mac:** `CaptureState.atBarrier`, `HistoryCapture.atBarrierNow`, and `Coordinator.undoAvailable` (`canUndo ||
  atBarrier`) decide Edit ▸ Undo. ⌘Z at a barrier now shows its notice. `onBarrierNotice` replaces the modal in tests.
- **Linux:** Ctrl+Z already reached the notice, but the Edit ▸ Undo menu item was disabled at a barrier. It now follows
  `EditorShell::undoAvailable()`, the same rule.

**Tests:**
- `CaptureCApiTests` `[I-0302]`: the flag at the root, after an edit, at a barrier, after an undo back onto it, and not for a
  reversible structural step. Red before the fix.
- Mac interop `undoAtBarrierShowsTheNotice`: a split, then Undo validated through `validateUserInterfaceItem` (the
  validation AppKit calls), then ⌘Z shows "Can't undo past creating a scene". Red without the Mac change (Undo disabled).
- Linux `undo_smoke` §5: at the split's barrier, `!canUndo && undoAvailable`. Red without the Linux change.
- ctest macOS 724/724, Linux 727/727; 32/32 Linux smokes; interop 291/291.

**To verify (Mac):** in dumas-prose-timelines (its newest step is the 14:56 barrier), press ⌘Z. A dialog says *"This scene was
changed outside Scrivi; undo stops here."* instead of a flash.
