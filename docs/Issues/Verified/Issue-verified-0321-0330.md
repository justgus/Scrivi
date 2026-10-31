# Verified Issues: I-0321 – I-0330

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0311–I-0320 are in [`Issue-verified-0311-0320.md`](Issue-verified-0311-0320.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0321** | `[ScriviCore]` ⚠️ **The branch chooser offered a branch with no text: a change that only typed spaces (then undone) became a branch of its own.** Found by the user 2026-10-24 ([SP-188] Mac pass). → [details](#i-0321) | **Medium** | [SP-188] | ✅ **VERIFIED 2026-10-24** |
| **I-0325** | `[Apple]` ⚠️ **iOS: the caret jumps to the start of a scene — the last character of the manuscript sends it to the previous scene's start, and typing or scrolling moves it elsewhere.** The iOS selection `onChange` had none of the Mac's [I-0132] loop break, so every viewport change echoed back as a navigation. ⚠️ Re-pass 2026-10-30: the end-of-manuscript case passes; a click then typing still lands elsewhere — diagnosing. → [details](#i-0325) | High | [SP-198] | ✅ Verified |
| **I-0327** | `[Apple]` ⚠️ **The Scene Navigator showed an untitled scene's first line RAW — `## Youth against age`, escapes and emphasis markers included.** ✅ Ruled (user, 2026-10-30): *"The Navigator should show what the manuscript shows and hide what the manuscript hides."* → [details](#i-0327) | Medium | [SP-198] | ✅ Verified |

---

## I-0321

**Title:** The branch chooser offers a branch with no text
**Status:** ✅ **VERIFIED 2026-10-24** (user, Mac: *"I-0321 passes, mark it verified"*)
**Platform:** `[ScriviCore]` (the Mac shows what the core offers; Linux has no chooser)
**Component:** `HistoryService::forkAheadAt` (`ScriviCore/src/history/HistoryService.cpp`)
**Severity:** Medium. A choice with nothing in it gives the writer nothing to decide by.
**Sprint:** [SP-188]
**Date Identified:** 2026-10-24, user, Mac, on `dumas-prose-timelines`

**What happened (user):** *"I type "One", Undo, "Two", Undo, Redo "One", Then I type some more text after one. Undo, Undo,
Redo "One", Redo again. Choicepoint, somehow I have a choicepoint with no actual text in it. Or its all spaces. That is
unhelpful. Historical branches should be merged with the nearest sibling so that the writer is always presented with text
to choose from."*

**Cause (read from the project's own history log, today's session):** the fork after "Three\." had five branches, one of
them `inserted: "  "`: spaces typed after the sentence and then undone. The undo committed them as an event first, and the
next change from that point became its sibling. A branch can also START with whitespace (`"  One"`), and its preview showed
the leading spaces.

**✅ Ruling (user, 2026-10-24):** the writer is always presented with text to choose from.

**Resolution (2026-10-24):** the chooser's branches (`forkAheadAt`, used by `redoFork`) leave out a branch that has no
text anywhere in it. That branch is folded into its siblings: still in the history tree and the History card, just not
offered. Each branch that is offered shows the first text down its primary path, trimmed. If only one branch with text is
left, there is no chooser. Text = an insertion or removal with a non-whitespace character; a structural step always counts.
Core test `[I-0321]`.
- ⚠️ Not handled: if the blank branch is the PRIMARY one and only one other branch has text, ⇧⌘Z (no chooser) still
  follows the primary, and re-types the spaces.
- ⚠️ Not changed: an undo still commits typed spaces as their own event, which is what creates such a branch. Whether
  whitespace alone should ever be an undo step is a separate question.

---

*Last Updated: 2026-10-24 — **I-0321** archived (user-verified, Mac).*

---

## I-0325

**Title:** iOS: the caret jumps to the start of a scene
**Status:** ✅ Verified (2026-10-30)
**Platform:** `[Apple]` iOS / iPadOS
**Component:** `EditorView.container`, the iOS `NavigationSplitView`'s `.onChange(of: selectedSceneID)`
(`Scrivi/Views/EditorView.swift:222–230`)
**Severity:** High. The writer cannot put the caret where they click.
**Sprint:** [SP-198] (AC5 live pass)
**Date Identified:** 2026-10-30, the user's iPad simulator pass on dumas-prose-timelines: *"if I try and position the caret
as the last character in the document, it gets moved to the start of the previous scene. Also a couple of times the caret
moved to other places in the document seemingly at random, or when I start to type."*

**Root cause:** the navigator and the manuscript form a deliberate cycle: a caret move or scroll → `setViewportScene` → the
navigator mirrors it into `selectedSceneID` → `onChange` → `navigateToSceneID` → the manuscript puts the caret at that
scene's start. The Mac breaks the cycle with two guards ([I-0132], `EditorView.swift:297–298`); the iOS branch had neither,
so every viewport change came back as a navigation. The console showed it: `setCurrentIndex` and `setViewportScene`
flipping between scenes (1173 ↔ 1174 at the end of the manuscript).

**Resolution:** the iOS `onChange` takes the Mac's two guards (`isMirroringViewportToSelection`, then the selection equal to
the viewport). Mac code unchanged.
**Files Affected:** `Scrivi/Views/EditorView.swift`
**Verification:** iOS tests 10/10, Mac interop 325/325. ⚠️ No automated test: the cycle runs through SwiftUI's update of a
`NavigationSplitView`. ⏳ User: the iPad live pass — caret at the very end, typing in several scenes, scrolling.

**Re-pass (user, 2026-10-30):** B (caret after the last character) ✅. ⛔ C/D: *"I click and position the caret in one
scene and the highlighted scene in the Scene Navigator is something else entirely. Then I start typing an l and the Scene
Navigator scrolls up to the beginning … None of the l's I typed went where I intended them to go."* The history records
where they went: ONE edit `ll` in scene `…2ea5` at byte 793 (20:02:32) — not the clicked scene, not the manuscript's end.
- **The highlight (explained):** the Navigator highlight is the viewport scene, the scene at the CENTRE of the view, as on
  the Mac (`scrollDidChange`, `ManuscriptTextView.swift:~875`). On iOS the keyboard appearing after a click scrolls the text,
  so the centre moves to another scene; the Mac's click never scrolls. Not yet decided what iOS should do.
- **Where the typing goes (not explained):** reading found nothing that moves the caret per keystroke (`insertText` hands
  a plain letter to UIKit; nothing a keystroke writes triggers `rebuildStorage`). A type-to-select hypothesis was disproven
  (the scene jumped to does not start with "l"). ✅ A Debug-only `[SCRIVI-CARET]` diagnostic now logs every selection
  change (scene, first responder, rebuilding), every `insertText` location and every rebuild, and the call stack of any
  move over 200 characters no keystroke made (`ManuscriptUITextView.swift`). ⏳ The next pass's console decides the fix.

**Pass 3 console (user, 2026-10-30) — the cause, measured:** *"when I click a scene in the Scene Navigator, the scroll does
not put the caret anywhere, and it does not scroll to the center of the viewport, instead opting for either the top or
bottom."* The `[SCRIVI-CARET]` lines: the resume and the Navigator's `navigateToScene` set the caret with the view NOT first
responder; then the writer's tap moved it to **1,848,832 — the manuscript's end** (stack all UIKit); every `l` went there.
- ✅ Measured on the simulator: focus alone keeps the caret (`focusKeepsCaret`). ⛔ After a long programmatic jump with
  `scrollRangeToVisible`, the hit-test at the screen's centre landed **35,507 characters** from the visible text in a
  1.7 MB manuscript WITH scene dividers (none without them): TextKit 2 was left on estimated positions, so a tap resolved
  elsewhere.
- ✅ **Fix — the Mac's shape:** `ManuscriptUITextView.centre(_:)` is the Mac's `centerStorageOffset` — scroll, lay out the
  viewport, measure the target, centre it, lay out again, repeated until it stays put (the Mac re-centres a restore the
  same way). `place()` uses it, so a navigation and the resume CENTRE the scene and leave TextKit 2's layout true. The iOS
  Navigator `onChange` now calls `loader.takeFocus()` after navigating, as the Mac does, so the caret it placed is live.
  Test `jumpCentresAndHitTests`: within one screen (was 35,507 characters away).
- ⚠️ Still open: the highlight is the scene at the view's centre; the keyboard's own scroll can move it. Re-judge after
  this pass. The `[SCRIVI-CARET]` diagnostic stays until the pass confirms.

**Pass 4 (user, 2026-10-30): all six steps pass** — resume centred, Navigator click centres with a live caret, typing lands
where clicked (also after a long scroll), close keeps the place and the typing ([I-0328]). Caveat: *"when the project is
reloaded … the focus originates in the Scene Navigator instead of in the Manuscript. We would prefer it if the writer could
just begin typing."* ✅ `restoreWritingSurface` now calls `loader.takeFocus()` after placing the caret, as the Mac's window
opens with the manuscript first responder. iOS 12/12. ⏳ User: reopen and type at once.

✅ **Verified** (user, 2026-10-30): pass 4 — all six steps; *"Reopen passes, typing works immediately."*

---

## I-0327

**Title:** The Scene Navigator showed an untitled scene's first line raw
**Status:** ✅ Verified (2026-10-30)
**Platform:** `[Apple]` (Mac and iOS: shared code). Linux is not affected: an untitled scene reads "Untitled Scene"
(`platforms/linux/src/EditorShell.cpp`, `rebuildNavigator`) — ⚠️ a parity gap with Apple's first-line titles, not tracked here.
**Component:** the live title — five copies of "first non-blank line as stored" (`ViewportSceneLoader.swift:223`, `:646`,
`:968`; `ManuscriptEditing.swift:822`; `ManuscriptTextView.swift:2193`), shown by `SceneNavigatorView.swift:796` and the
timeline's dot titles (`TimelineStripView.swift:513`)
**Severity:** Medium.
**Sprint:** [SP-198]
**Date Identified:** 2026-10-30, the user on dumas-prose-timelines: the heading formatting *"appears in the Scene Navigator"*.
**Ruling (user, 2026-10-30):** *"The Navigator should show what the manuscript shows and hide what the manuscript hides."*

**Resolution:** `ManuscriptPresenter.presentedFirstLine(of:)` (`ManuscriptPresenter.swift:196–217`): the first line as the
manuscript draws it at rest — escapes, emphasis markers and a heading prefix hidden, a list prefix shown (SP-163 Q7). It reads
the whole first block, so an emphasis pair that closes on a later line is still a pair, and it uses the presenter's own
analysis (`ManuscriptPresenter.analyze`, now static; `info` caches it), so the two cannot drift. All five sites call it. ⛔ Not
`scrivi_markdown_balanced_copy`'s `presented`: that is what OTHER APPS get, and it drops a list's `- `.
**Files Affected:** `Scrivi/Views/ManuscriptPresenter.swift`, `ViewportSceneLoader.swift`, `ManuscriptEditing.swift`,
`ManuscriptTextView.swift`; test `presentedFirstLine` (`Scrivi/Tests/ScriviInteropTests.swift:3802`).
**Verification:** Mac interop 326/326, iOS 10/10. ⏳ User: the Navigator on dumas shows `Youth against age`, no `##`.

✅ **Verified** (user, 2026-10-30): pass 3 step A — the Navigator shows the heading text with no `##`.
