---
sprint: SP-223
epic: EP-055
status: Active
platform: Apple
created: 2026-11-18
---

# SP-223 — `[Apple]` [EP-055] S3b: Find and Replace, Go To, branch marks, copy buffers and Format on iOS

**Status:** 🟡 **ACTIVE** 2026-11-18 (user, 2026-11-18: *"activate and implement SP-223"*) · planned 2026-11-18 (user: *"plan S3b for EP-055"*) · ✅ Q1–Q6 ruled 2026-11-18 (Q5 amended).
**Carries:** [T-0686] (new) → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-055] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — AC5, its second half ([SP-222] Q1). With this Sprint
AC5 is met.
**Builds on:** [SP-222] (`ManuscriptStructure.swift`: the history, the structured clipboard, the fork chooser on iOS); on the Mac,
[SP-164] (Find over the presented text), [SP-171] (`ManuscriptPresentedMap`), [SP-163] / [SP-174] (Format, computed by ScriviCore),
[I-0333] / [SP-201] (Replace All off the main thread), the copy buffers (EP-019, [SP-188]) and the branch marks.
**Size:** L — five surfaces, but four of them reach logic that is already shared or already in ScriviCore.

---

## What reading the code found (2026-11-18)

- **Find.** The Mac searches the manuscript **as presented** (no escapes, emphasis markers or heading prefixes; [SP-164] Q1)
  through `ManuscriptPresentedMap.swift`, whose `PresentedText` / `PresentedLayout` are plain Foundation — but the whole file sits
  in `#if os(macOS)`, and one class in it (`:376–409`) holds an `NSTextView`. `ManuscriptFind.swift` is the `NSTextFinderClient`
  (AppKit only). Replace All (`ManuscriptTextView.swift:1419`, `replaceAll`) is one history step, computed off the main thread,
  applied in slices — in the Mac `Coordinator`.
- **Go To.** The iOS host moves the caret (`ManuscriptUITextView.swift:53–58`, `move`), and the project toolbar already reaches
  all six on the Mac, iPad and iPhone (`ProjectToolbar.swift:73–78`, its Go To item). ⛔ *First written as "nothing on the iPad
  calls them" — wrong; corrected by the user, 2026-11-18: "The toolbar has all the goto buttons, 5th from the left."* What the iPad
  lacks is the MENU items. The Mac has them in Scene, Chapter and Project (Manuscript Start/End,
  ⇧⌘0 / ⇧⌘1, `ScriviApp.swift:356`).
- **Branch marks.** Mac only (`ManuscriptTextView.swift:395–460`: `scheduleBranchMarks`, `branchMarkFrames`, a margin button per
  line where a fork's branches part). The data (`HistoryForkAhead`) and the chooser are already shared or on iOS ([SP-222] AC4).
- **Copy buffers.** `BufferService` is shared; the three operations live in the Mac `Coordinator` (`:733–785`: `cutIntoBuffer`,
  `pasteFromBuffer`, copy), each a history bracket or the structured paste. The menus (`ScriviApp.swift:492–524`) and the
  palette (`BuffersPalette.swift`, a floating `NSPanel`) are Mac only.
- **Format.** The edits come from ScriviCore (`ManuscriptCommands.swift`, no platform guard — already compiled for iOS);
  `applyFormat` (`ManuscriptTextView.swift:1849`) is on `ManuscriptNSTextView`. The Format menu is Mac only (`:278`).

## Goal

On an iPad or iPhone the writer **finds and replaces** in what they see, **goes to** a scene's, chapter's or the manuscript's start
and end, **sees where the history branches**, uses the **nine copy buffers**, and applies the **Format** commands — each writing the
same bytes and history as the Mac.

## Plan

1. **Shared code ([SP-222] Q2's rule):** move `applyFormat`, the three buffer operations and `replaceAll` out of the Mac host into
   shared code (`ManuscriptStructure.swift` or a sibling), as SP-222 did; take `ManuscriptPresentedMap`'s view-free part out of
   `#if os(macOS)`. The Mac calls the same code; the interop suite guards it.
2. **Find (Q1):** `UIFindInteraction` with a `UITextSearching` source over `PresentedText`, matches mapped to storage; Replace All
   through the shared `replaceAll`.
3. **Go To (Q5):** the iPad menu bar's Scene, Chapter and Project menus get the Mac's Go To items and shortcuts, where the Mac
   has them; the toolbar's Go To stays on every platform.
4. **Branch marks (Q3):** the Mac's marks in the iOS text view's margin; a tap opens [SP-222]'s chooser.
5. **Buffers (Q2)** and **Format (Q4):** the iPad menu bar, the Mac's key chords as key commands, and the text's edit menu.
6. **Checks:** iOS tests on the simulator for each (find maps to storage; Replace All is one undo step; each buffer operation and
   each Format command writes the Mac's bytes and one history event; Go To lands the caret); Mac interop green; builds; CI.
7. **Live passes (user):** iPad and iPhone; the Mac re-checked for Find, Replace All, a buffer cut/paste and a Format command.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Find: the system's find bar, over what?** | **`UIFindInteraction` (the system bar: ⌘F, Edit ▸ Find on the iPad, the edit menu's Find on the iPhone) over the PRESENTED text, as the Mac ([SP-164] Q1)** — markers and escapes are not found, chapter titles are skipped. Replace All is one undo step, off the main thread, as [I-0333] ruled. The shared map is what makes the iOS and Mac results agree. | ✅ As recommended (2026-11-18) |
| **Q2** | **The copy buffers without a floating palette.** | **Menus and keys, no palette in this Sprint:** the iPad's Edit menu gets Copy To / Paste From / Cut To Buffer (1–9) with the Mac's chords (⌘1–9, ⌃⌘1–9, ⌥1–9) as key commands; the text's edit menu gets a **Buffers** submenu, each slot labelled by the start of what it holds, so the iPhone reaches them. A buffers panel on the iPad belongs with AC6's panels. | ✅ As recommended (2026-11-18) |
| **Q3** | **Branch marks on iOS.** | **Draw them as the Mac does** (a margin mark on each line where a fork's branches part); a tap opens the same chooser as redo at a branch ([SP-222] Q4) — popover on the iPad, sheet on the iPhone. Without them the writer cannot see that a redo will ask. | ✅ As recommended (2026-11-18) |
| **Q4** | **Format: where, and the system's ⌘B / ⌘I.** | **The iPad menu bar gets Format** (Bold, Italic, Headings 1–3, Body, the lists) with the Mac's shortcuts; the edit menu gets a **Format** submenu; UIKit's own `toggleBoldface` / `toggleItalics` (⌘B / ⌘I, and the keyboard's B/I/U if it shows) go to Scrivi's commands, so styling still enters only as Markdown (the escape ruling). | ✅ As recommended (2026-11-18) |
| **Q5** | **Go To: where.** | **The iPad menu bar, the Mac's places and keys:** Go to Scene Start/End (Scene), Chapter Start/End (Chapter), Manuscript Start/End ⇧⌘0 / ⇧⌘1 (Project). **Not** in the edit menu. | ✅ **Amended** (user, 2026-11-18): *"don't remove the toolbar buttons on either mac or iphone.  Add the menu items on ipad in the same place they live on the mac.  Leave them out of the long press menu.  The navigator is still a way to jump, but, thousands of scenes makes using the navigator tedious and error prone, same as on the mac."* → the menu items are ADDED on the iPad in the Mac's places; the toolbar's Go To stays on the Mac, iPad and iPhone; none in the long-press menu. The navigator is not a substitute for them at thousands of scenes. |
| **Q6** | **One Sprint or two?** | **One.** Find and the marks are the real work; Go To is menu items, and buffers and Format are a move of Mac code plus surfaces — the same shape as [SP-222], which held in one. | ✅ As recommended (user, 2026-11-18: *"…Q6: as recommended.  Q1-Q4 also as recommended."*) |

⛔ **NOT in this Sprint:** the buffers palette on iOS (with AC6); the navigator, inspector and timeline on the iPad (AC6); a
key equivalent for Go to Scene / Chapter (still open on the Mac, `ScriviApp.swift:403`).

## Acceptance Criteria

- [x] AC1 — Find, Find Next/Previous, Use Selection, Replace and Replace All on iOS, over the presented text (Q1); Replace All one undo step
- [x] AC2 — Go to Scene, Chapter and Manuscript Start/End on the iPad menu bar, in the Mac's places with its shortcuts; the toolbar's Go To unchanged on every platform (Q5)
- [x] AC3 — Branch-point marks on iOS; a tap opens the chooser (Q3)
- [x] AC4 — The nine copy buffers on iOS: copy to, cut to, paste from — the Mac's bytes and history, across scenes too (Q2)
- [x] AC5 — The Format commands on iOS, ⌘B / ⌘I included (Q4)
- [ ] AC6 — iOS tests for AC1–AC5 on the simulator; Mac interop green; macOS and iOS builds; CI green
- [ ] AC7 — **Live passes (user):** iPad, iPhone, and the Mac re-checked

## Progress log

- **2026-11-18** — 🔵 **Created in Planning** (user, 2026-11-18: *"plan S3b for EP-055"*), issued via `next-id.py`; [T-0686] filed.
  Q1–Q6 to rule.
- **2026-11-18** — ✅ Q1–Q4, Q6 ruled as recommended; Q5 amended (keep the toolbar's Go To everywhere; menu items on the iPad in the
  Mac's places; none in the long-press menu) — user, 2026-11-18. The Go To finding corrected (the toolbar already reaches them).
- **2026-11-18** — 🟡 **Activated** (user, 2026-11-18: *"activate and implement SP-223"*); [T-0686] → active.
- **2026-11-18** — **Built.**
  - **Shared (Q2 of [SP-222], extended):** `ManuscriptStructure` gains Replace All and the three buffer operations, MOVED out of
    the Mac `Coordinator` (it keeps forwarders; `deleteSelectionForBuffer` deleted, now unused); `ManuscriptPresenter.formatStep`
    is the Format decision both views apply; `ManuscriptPresentedMap.swift` is no longer Mac-only.
  - **AC1 Find (Q1):** UITextView's own find bar with a session over the presented text (`ManuscriptTextSearcher`,
    `ManuscriptFind.swift`): matches off the main actor, mapped to storage, never across a scene break or in a chapter title;
    Replace is the typing path; Replace All the shared one-step path; Use Selection for Find takes the presented text.
  - **AC2 Go To (Q5):** the iPad's Scene, Chapter and Project menus, in the Mac's places, ⇧⌘0 / ⇧⌘1; the toolbar unchanged.
  - **AC3 Branch marks (Q3):** margin buttons on laid-out lines where the branches part (hints on), refreshed on report, scroll
    and landing; a tap opens the chooser anchored at the mark.
  - **AC4 Buffers (Q2):** ⌘N / ⌃⌘N / ⌥N key commands; the iPad Edit menu's three submenus; a Buffers submenu in the edit menu,
    slots labelled by their start, Paste listing only full slots.
  - **AC5 Format (Q4):** the iPad's Format menu with the Mac's shortcuts; a Format submenu in the edit menu; `toggleBoldface` /
    `toggleItalics` routed to it, underline refused; the pending pair on iOS.
  - **AC6 so far:** macOS and iOS builds ✅, no warnings in the touched files; Mac interop 352 passed, 0 failed; iOS 29 passed,
    0 failed — 5 new (Find over the presented text; Replace and Replace All, one undo step; Format incl. the pending pair; the
    buffers; a branch mark and its tap). ✅ Mutation run (in a copy): 4 of 4 mutants caught after one added check (a second
    Format press removing the pair). The harness window now uses the host's scene (`UIWindow(frame:)` is deprecated). ⏳ CI after
    the push.
- **2026-11-18** — ⛔ **iPad live pass (user):** *"1. Cmd-F showed nothing. 2. Opt-Cmd-F also showed nothing. … 4. passes. 5. … the
  View menu on the iPad is completely empty … make the iPad menus look like the … mac menus … The marker do not show. Furthermore,
  arrow movement of the caret does not advance properly. I must advance 3 times to move into a bold word and 2 times to move into an
  italicized word. Also there is no Format Menu on the iPad … There is a Format menu on the popup menu … But it has only Bold and
  Italic … 6. passes. 7. no view menu items. Can't turn branch marks on."* Steps 4 (Go To) and 6 (buffers) pass.
  - **Cause, menus:** iPadOS builds Edit ▸ Find, Format and View only for a scene that asks for SwiftUI's `TextEditingCommands`,
    `TextFormattingCommands` and `SidebarCommands` — so ⌘F / ⌥⌘F had no item to fire, and the Format and View groups had no menu.
    ✅ Fixed: the three sets; the iPad's menus now follow the Mac's (Project, Worlds, Scene, Chapter; Project's order; View ▸ Show
    or Hide Scene Inspector ⌥⌘I, Timeline ⌥⌘T, World Warnings, **Show Markup Hints ⇧⌘H** (was not on iOS at all — so the marks
    could not be shown); Project Settings ⌥⌘,; User Manual (disabled, as the Mac)). ⛔ Not on iOS: Show Buffers (no palette, Q2),
    Show Log Folder (no Finder).
  - **Cause, edit menu:** iOS's own Format (Bold / Italic) stood ahead of Scrivi's. ✅ Scrivi's takes its place.
  - **Cause, the caret and the markers:** the iOS host never had the Mac's caret snap or reveal (a gap since S1). ✅ Fixed in
    `ManuscriptUITextView.swift` (`snapSelectionOutOfStopRun`; `reveal` on selection change). ⚠️ The PENDING pair's markers: NOT
    reproduced — the simulator draws them dimmed with or without a re-present (a mutation-checked attempt, removed). Suspected:
    with no Format menu, ⌘B reached nothing; the re-pass will tell.
  - ✅ Checks: iOS 31 passed, 0 failed (2 new: the caret snap + reveal + pending pair drawn; the edit menu's Format); macOS and iOS
    builds ✅. Mutation run (in a copy): the snap, the reveal and the edit-menu Format each caught. ⚠️ An earlier run reported 29 from a stale test object (Xcode skipped recompiling the test target); its intermediates
    were removed and the counts above are from the rebuilt bundle.
- **2026-11-18** — ✅ **Re-pass (user):** *"Step 1 came through this time.  And it passed!  Step 2: passed.  Step 3: failed.  Cmd-E did
  nothing.  Steps 4-10 passed.  Step 11: passed.  Yes it showed four * with the caret between them.  Cmd-I showed ** with the caret
  between it.  Steps 12-15 passed.  Step 16 and 17 passed.  Step 18-20 passed."* — iPad (Find, Replace All, the menus, View's
  toggles, ⇧⌘H, the caret snap and hints, the pending pair, Format, branch marks), iPhone and Mac all pass, except ⌘E.
  - **Cause, ⌘E:** iPadOS's Find menu binds no ⌘E, so Use Selection for Find was never called (proven: its code sets the search
    text when called directly; the view had no ⌘E key). ✅ Fixed: ⌘E is the text view's own key command. ✅ iOS 31 passed, 0 failed
    (the Find test now checks the key and the search text; it failed before the fix). ⏳ AC7: ⌘E re-checked on the iPad.

