# Verified Task: T-0543

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0543** | ✅ **The toolbar** — `NSToolbar` + title/subtitle + the verbs that already exist (S1+S2) | [SP-134] | [EP-040] | **2026-09-22** |

✅ **VERIFIED by the user's live pass, 2026-09-22:** *"All the buttons behave properly in the live
test. It looks good too."*

---

## ✅ What it delivered

- ✅ **A real toolbar on the project window** — ⚠️ **`NSToolbar` appeared ZERO times in the app before
  this Task**, while the window asked SwiftUI for `.navigationTitle`.
- ✅ **All EIGHT structure verbs**, ⚠️ **each calling the closure the MENU BAR already calls**
  ([EP-040] AC2) — ⛔ **no duplicated logic, no second path to keep in step.**
- ✅ **Visibility toggles** (Inspector · Timeline · Buffers) and ✅ **project surfaces**
  (Worlds · Project Settings).
- ✅ **`columnVisibility` BOUND on the macOS `NavigationSplitView`** — ⚠️ **the Sprint's only
  non-additive edit, and the one that matters.**
- ✅ **The duplicate "View" menu MERGED** into the system one.

## ⚠️ THE FINDING THAT MADE THE SPRINT WORTH MORE THAN ITS SCOPE

⛔ **THE SCENE NAVIGATOR HAD NO VISIBILITY STATE AT ALL.** ✅ **Inspector and Timeline are `Bool`s the
app owns, so the View menu could always restore them.** ⚠️ **The Navigator was a `NavigationSplitView`
sidebar whose `columnVisibility` was `#if os(iOS)` ONLY** — ⛔ **so on macOS the app could neither READ
nor SET it.**

✅ **THAT IS WHY IT WAS THE PANE THE WRITER LOST** ([I-0203]). ⚠️ **The platform's sidebar control was
always present; what was missing was STATE BEHIND IT.**

## ⚠️ FIVE DEFECTS, NONE COMPILER-VISIBLE — all found by the user's live pass

⛔ **`xcodebuild` reported SUCCESS over every one of them.**

| ⛔ Found | ✅ Fixed |
| -------- | ------- |
| ⚠️ **A BLANK BUTTON** — `arrow.triangle.merge.circle` **does not exist**; ⛔ **SwiftUI renders a missing symbol as an EMPTY button with a working tooltip** | ✅ **a real symbol**; ⚠️ **ALL 13 verified against `NSImage(systemSymbolName:)`** |
| ⚠️ **WRONG GROUPING** — "structure" vs "panes" ⛔ **split the Chapter verbs; Merge Chapter sat alone** | ✅ **ONE GROUP PER MENU** |
| ⚠️ **GROUPING WAS COSMETIC** — ⛔ **`ToolbarItemGroup` is layout adjacency ONLY; at narrow widths its members scatter into the generic overflow** | ✅ **`ControlGroup` NESTED inside it** — ⚠️ **that is what makes macOS 27 collapse a group into its own TITLED popup** |
| ⚠️ **A DUPLICATE NAVIGATOR BUTTON** — ⛔ **same icon and function as the system control top-left** | ⛔ **REMOVED** |
| ⚠️ **TWO "VIEW" MENUS** — ⛔ **`CommandMenu` CREATES a menu; macOS already synthesizes one** | ✅ **`CommandGroup(after: .sidebar)`** |

⚠️ **THE VIEW-MENU DEFECT WAS ALREADY DOCUMENTED — ON THE OTHER PLATFORM.**
✅ **`ScriviApp.swift:320`:** *"No `CommandMenu("View")` — iOS already synthesizes a 'View' menu;
adding our own duplicated it."* ⛔ **Diagnosed on iOS, never applied to macOS**
(`feedback_look_for_existing_pattern_first`).

## ⚠️ AND A SIXTH ROUND: every verb needed its OWN glyph

⚠️ **Three symbols were reused between Scene and Chapter.** ✅ **Resolved by RENDERING the candidates
to a contact sheet and LOOKING**, ⛔ **not by reading their names — and two ideas died on contact:**
⛔ **`.alt` is NOT a double line** (it draws the same as the plain symbol) and ⛔ **`arrow.merge` /
`arrow.trianglehead.merge` draw the SAME PICTURE as `arrow.triangle.merge`.**
✅ **Final: Chapter takes `rectangle.compress.vertical` (two containers collapsing into one) and the
`.circle` variants of the line arrows** — ⚠️ **the user's own "enclosed" metaphor.**

## How it was verified

✅ **USER LIVE PASS 2026-09-22 — all buttons behave correctly; appearance approved.**
✅ **`xcodebuild` build clean · `TEST SUCCEEDED`, 132/132 in 12 suites · `ctest` 626/626 · boundary
guard clean.**
⚠️ **A test run FAILED TO LAUNCH mid-Sprint** — ✅ **because the user's app was running**
(`feedback_check_scrivi_running_before_tests`); ⛔ **the app was NOT killed.**

## ⛔ What it did NOT resolve

⚠️ **THE PROJECT TITLE RENDERS THREE TIMES** — ✅ **filed as [I-0243]**, ⚠️ **found precisely BECAUSE
this Task added a toolbar.** ✅ **User ruled OPTION A (windows only, `tabbingMode = .disallowed`).**
⛔ **[EP-040] AC1 is not fully judged until [I-0243] is resolved.**

⚠️ **AC7's iOS/visionOS half is DEFERRED** (user ruling) — ⛔ **the app cannot RUN there:
`ManuscriptView` will not render.** ✅ **A future `[Apple]` Epic owns that layout**, ⚠️ **and this
work helps define its gaps.**

---

*Verified 2026-09-22 by the user's live pass. Implemented under [SP-134] / [EP-040].*
