---
sprint: SP-134
epic: EP-040
status: CLOSED 2026-09-22 (user-approved)
activated: 2026-09-22
closed: 2026-09-22
task: T-0543
planned: 2026-09-22
platform: Apple
---

# Closed Sprint — SP-134 — ✅ **The toolbar** — `NSToolbar` + title/subtitle + the verbs that already exist

**Status:** ✅ **CLOSED 2026-09-22 — user-approved.** ✅ **T-0543 VERIFIED by live pass** →
[`../../Tasks/Verified/Task-verified-0543.md`](../../Tasks/Verified/Task-verified-0543.md).
⚠️ **Prior status line follows.** 🟢 **ACTIVE — T-0543 IMPLEMENTED 2026-09-22.**
✅ **Q1–Q3 ruled before activation.** ✅ **Build clean · `TEST SUCCEEDED` 132/132 · `ctest` 626/626.**

⚠️ **ONE LIVE PASS HAS ALREADY HAPPENED AND IT FOUND FIVE DEFECTS** — ⛔ **every one invisible to the
compiler, and `xcodebuild` reported SUCCESS over all of them.** ✅ **All five fixed; ⚠️ a second pass
is owed (AC8).**

⛔ **S1b REMAINS OPEN BY DESIGN** — ⚠️ **`.navigationTitle`/`.navigationSubtitle` are untouched
pending a LOOK at the title area.** ✅ **AC1 is not fully judged until then.**
**Epic:** 🟡 [EP-040](../../Epics/Epic-active.md) — `[Apple]` **The Editor Shell**
**Task:** **T-0543** — ✅ **ISSUED 2026-09-22 at activation.**
**Steps:** **S1 + S2** of the app-shape study §4.3.
**Serves:** **[EP-040] AC1, AC2, AC3, AC7** · contributes to **[I-0203]**
**Blocks on:** ⛔ **NOTHING.** ✅ **It is the Epic's opener and its lowest-risk Sprint.**
**Design:** ✅ [`../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`](../../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md) §4.2–4.3
(user-approved 2026-09-14) · [`../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md`](../../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md) F1/F2

---

## ✅ Goal

⚠️ **The project window has real chrome.** ✅ **An `NSToolbar` carries the window's title and subtitle
where macOS puts them, and surfaces the verbs a writer already has in the menu bar** — ⚠️ **including
the pane toggles, so a writer who loses a pane can bring it back FROM THE WINDOW.**

⛔ **NO new features. ⛔ No new logic.** ✅ **S2 populates a toolbar S1 just created with closures that
already exist** — ⚠️ **which is exactly why S1 and S2 are ONE Sprint: splitting them ships an EMPTY
TOOLBAR as a milestone.**

---

## ⚠️ What is actually there — measured 2026-09-22

| Check | Result |
| ----- | ------ |
| `NSToolbar` in `Scrivi/Views` + `Scrivi/App` | ⛔ **ZERO occurrences** |
| The project window | ✅ **a real AppKit `NSWindow`** (`ProjectWindowManager.swift:87`) hosting SwiftUI via `NSHostingView` |
| Structure verbs | ✅ **ALL FOUR EXIST** — `createSceneAction`, `createChapterAction`, `mergeSceneAction`, `mergeChapterAction` (`ProjectSession.swift:57-60`) |
| Inspector / Timeline visibility | ✅ **plain `Bool`s on `ProjectSession`** (`:92`, `:98`), ⚠️ **already bound by View-menu `Toggle`s** (`ScriviApp.swift:238-240`) |
| **Navigator visibility** | ⛔ **NO `navigatorVisible` STATE AND NO MENU TOGGLE — see the finding below** |

✅ **THE WINDOW BEING REAL APPKIT IS THE GOOD NEWS.** ⚠️ **A SwiftUI `WindowGroup` would have forced
the toolbar through scene modifiers;** ✅ **here it attaches natively (`window.toolbar = …`), which is
why this Sprint is LOW risk.**

---

## ⛔ THE FINDING THAT SHAPES THIS SPRINT — ⚠️ **the Navigator has no visibility state at all**

⚠️ **[I-0203] IS *"the Scene Navigator hid itself and I couldn't see it."*** ✅ **[EP-040] AC3 is
*"a writer who loses the Navigator, Inspector or Timeline can bring it back FROM THE WINDOW."***

⛔ **BUT THE NAVIGATOR IS NOT LIKE THE OTHER TWO.** ✅ **Inspector and Timeline are `Bool`s this app
owns and the View menu already toggles.** ⚠️ **The Navigator is a `NavigationSplitView` SIDEBAR
(`EditorView.swift:211`)** — ⛔ **and the macOS branch binds NO `columnVisibility`**, ⚠️ **so its
state is neither observable nor settable by this app.**

⚠️ **CONSEQUENCE, STATED PLAINLY: a toolbar button for the Navigator CANNOT be written the way the
other two can.** ⛔ **There is nothing to bind to.**
⚠️ **AND THIS IS WHY THE PANE IS THE ONE THE USER LOST** — ✅ **the two panes with app-owned state were
recoverable from the View menu; the one without state was not.**

✅ **THAT MAKES IT THIS SPRINT'S REAL WORK, not a footnote** — ⚠️ **and it is why Q1 below must be
ruled before any toolbar item is written.**

---

## Acceptance Criteria

- [ ] **AC1** — ✅ **The project window has a real `NSToolbar`**, ⚠️ **and the window title and subtitle
      render IN IT** rather than as a stray in-content band. ✅ **Closes conformance F1/F2.**
      ⚠️ **The subtitle is `prefs.projectSubtitle`, UNCHANGED** (⚠️ **Q3 ruled**) — ✅ **a pure
      relocation of what `.navigationSubtitle` already renders**, ⛔ **not a new value.**
      ⚠️ **Title and subtitle must survive a project rename** — ✅ **both read the SAME source the app
      already maintains, ⛔ never a copy.**
- [ ] **AC2** — ✅ **The toolbar surfaces the FOUR EXISTING Structure verbs** (Scene New/Merge, Chapter
      New/Merge) ⚠️ **by calling the SAME `focusedSession?.<verb>Action?()` closures the menu bar
      calls.** ⛔ **No duplicated logic, and no second code path to keep in step.**
      ⚠️ **They must DISABLE when their closure is `nil`**, ✅ **exactly as the menu items do.**
- [ ] **AC3** — ✅ **Inspector and Timeline can be toggled FROM THE WINDOW**, ⚠️ **bound to the same
      `ProjectSession` `Bool`s the View menu uses** — ⛔ **so the two controls can never disagree.**
      ✅ **Declared as SwiftUI `.toolbar` inside `ProjectWindowContent`** (⚠️ **Q2 ruled**), ⛔ **so the
      session is IN SCOPE and no `@FocusedValue` workaround is needed.**
- [ ] **AC4** — ✅ **THE NAVIGATOR IS RECOVERABLE FROM THE WINDOW** (⚠️ **Q1 RULED: bind
      `columnVisibility`**). ✅ **The macOS `NavigationSplitView` gains a bound
      `NavigationSplitViewVisibility`, and a toolbar control reads AND sets it.**
      ⚠️ **"Recoverable" means PROVEN: hide the Navigator, confirm the toolbar brings it back.**
      ⛔ **A control that only hides is half the AC** — ✅ **[I-0203] is about getting it BACK.**
- [ ] **AC5** — ✅ **DECLARED SLOTS for Export and the text-size lens exist and are documented as
      unimplemented.** ⛔ **Building either is OUT OF SCOPE** ([EP-040] AC7) — ⚠️ **they are what the
      toolbar makes ROOM for.**
- [ ] **AC6** — ⛔ **NOTHING ELSE MOVES.** ✅ **`ManuscriptTextView`'s internals are untouched**
      (app-shape §4.3: *"S1–S3 do NOT touch"*), ✅ **the three bars remain `VStack` siblings until
      [SP-135]**, ⚠️ **and the Inspector stays an `HStack` member until [SP-136].**
- [~] **AC7** — ✅ **MET FOR macOS; ⛔ THE iOS/visionOS HALF IS DEFERRED (user ruling 2026-09-22).**
      ⚠️ **THE APP CANNOT RUN ON THOSE PLATFORMS AT ALL: `ManuscriptView` will not render there.**
      ✅ **So "still builds" was never the real bar** — ⛔ **a build that cannot render proves nothing
      about a layout change.** ✅ **Deferred to a FUTURE `[Apple]` EPIC that deals with the iOS/visionOS
      view layout**, ⚠️ **and this Sprint's work HELPS DEFINE the gaps that Epic must close.**
      ⚠️ **Original text:** ✅ **`xcodebuild` builds and tests clean on macOS; ⚠️ iOS and visionOS still build** —
      ⛔ **the iOS branch has its own `.toolbar` (`EditorView.swift:189`) and MUST NOT be disturbed**
      (⚠️ **`feedback_prove_code_is_reached`: I-0161 was a fix applied to the wrong branch**).
- [ ] **AC8** — ⚠️ **A LIVE PASS ON A REAL PROJECT.** ✅ **Open `the-stairs-of-tintagael`, use each
      toolbar verb, toggle each pane, resize the window narrow, and confirm the toolbar collapses the
      way macOS does rather than losing items silently.**

---

## ✅ THE THREE RULINGS — user, 2026-09-22

⚠️ **ALL THREE MADE BEFORE ACTIVATION.** ✅ **Two of them CHANGED during planning, on evidence read
from the code** — ⛔ **and in both cases my first recommendation was the weaker answer.**

---

### ✅ Q1 — RULED: **BIND `columnVisibility` NOW**

✅ **The macOS `NavigationSplitView` (`EditorView.swift:211`) gains a bound
`NavigationSplitViewVisibility`**, ⚠️ **exactly as the iOS branch already has (`:171`).**

✅ **WHY:** ⛔ **without it the app cannot READ or RESTORE the Navigator's state at all** — ⚠️ **which
is why it is the one pane the writer lost.** ✅ **Inspector and Timeline were recoverable from the View
menu because they are `Bool`s this app owns; the Navigator was not because it owns nothing.**
✅ **With the binding, [EP-040] AC3 is met for ALL THREE panes in this Sprint.**

⚠️ **THE ACCEPTED COST, recorded so it is not rediscovered as scope creep: this touches a surface
[I-0203] is about, inside a Sprint otherwise scoped as purely additive.** ✅ **~2 lines, and the iOS
branch already proves the pattern.** ⛔ **The alternative was shipping two panes out of three and
leaving the one that actually failed for [SP-135].**

---

### ✅ Q2 — RULED: **SwiftUI `.toolbar`, in `ProjectWindowContent`**

⚠️ **THE DECIDING EVIDENCE WAS NOT THE STUDY — IT WAS `ScriviApp.swift:77`:**

> ⚠️ *"AppKit NSWindows don't feed SwiftUI's `@FocusedValue`."*

✅ **The menu bar already works around this with `env.frontmostSession`.** ⛔ **An AppKit `NSToolbar`
would need the SAME workaround for every item** — ⚠️ **a second hand-maintained path to the session,
which is precisely what [EP-040] AC2 forbids.**
✅ **A SwiftUI toolbar declared inside `ProjectWindowContent` has the session IN SCOPE** — ⚠️ **no
focus plumbing at all.**

⚠️ **THE OBJECTION IS REAL AND IS ACCEPTED, NOT DISMISSED:** ⛔ **the conformance study's complaint is
a SwiftUI `.navigationTitle` with no AppKit toolbar.** ✅ **But that defect was a title with NOWHERE TO
GO, not SwiftUI itself** — ⚠️ **and the fix is the toolbar existing, not who declares it.**
✅ **THE WINDOW KEEPS OWNING `window.title`**, which `ProjectWindowController` already maintains.

---

### ✅ Q3 — RULED: **KEEP `projectSubtitle`** — ⛔ **the question was based on a wrong premise**

⚠️ **I ASKED WHAT THE SUBTITLE SHOULD SAY AS IF IT WERE AN OPEN DESIGN QUESTION. IT IS NOT.**
✅ **`projectSubtitle` is a REAL PERSISTED FIELD the writer edits in Project Settings**
(`ProjectPreferences.swift:20`), ✅ **and `.navigationSubtitle(prefs.projectSubtitle)` already renders
it** (`EditorView.swift:187`, `:231`).

⛔ **MY EARLIER SUGGESTION — the current scene's title — WOULD HAVE OVERWRITTEN A FIELD THE WRITER
OWNS**, ⚠️ **and turned S1 from a relocation into a feature.**
✅ **The ruling makes S1 exactly what the study scoped: MOVE the title and subtitle into the toolbar,
changing nothing about what they SAY.**

---


## Plan of work

| Step | Work | ⚠️ Note |
| ---- | ---- | ------ |
| ~~S0~~ | ✅ **DONE 2026-09-22 — all three ruled before activation** | ✅ **Gate discharged** |
| **S1a** | ✅ Add the toolbar; move title/subtitle into it | ⚠️ **AC1** |
| **S1b** | ⚠️ **Decide what happens to `.navigationTitle`/`.navigationSubtitle`** (`EditorView.swift:186-187`, `:230-231`) | ⚠️ **AC1** — ⛔ **only these; the bars stay** |
| **S2a** | ✅ Wire the four Structure verbs to existing closures, with disabled states | ⚠️ **AC2** |
| **S2b** | ✅ Wire Inspector + Timeline toggles to the session `Bool`s | ⚠️ **AC3** |
| **S2c** | ✅ **Bind `columnVisibility` on the macOS `NavigationSplitView`; wire its toolbar control** | ⚠️ **AC4** — ⛔ **the only non-additive edit in the Sprint** |
| **S2d** | ✅ Declare the Export and text-size slots, documented unimplemented | ⚠️ **AC5** |
| **S3** | ✅ `xcodebuild` macOS + iOS + visionOS | ⚠️ **AC7** |
| **S4** | ⚠️ **Live pass on a real project** | ⚠️ **AC8** |

---

## ⚠️ LIVE PASS 1 — 2026-09-22, ✅ **FIVE DEFECTS, NONE COMPILER-VISIBLE**

⚠️ **THE BUILD WAS GREEN THROUGH ALL FIVE.** ✅ **This is `feedback_live_pass_finds_what_suites_cannot`
in its plainest form** — ⛔ **a toolbar that compiles is not a toolbar a writer can use.**

| ⛔ Found | ✅ Fixed |
| -------- | ------- |
| ⚠️ **A BLANK BUTTON** — `arrow.triangle.merge.circle` **does not exist**; ⛔ SwiftUI renders a missing symbol as an EMPTY button with a working tooltip | ✅ `arrow.triangle.merge`; ⚠️ **all 12 symbols verified against `NSImage(systemSymbolName:)`** |
| ⚠️ **WRONG GROUPING** — "structure" vs "panes" split the Chapter verbs; ⛔ Merge Chapter sat alone | ✅ **ONE GROUP PER MENU**: Scene · Chapter · Visibility |
| ⚠️ **GROUPING WAS COSMETIC, NOT FUNCTIONAL** — ⛔ `ToolbarItemGroup` is layout adjacency ONLY; at narrow widths its items drop into the GENERIC overflow rather than staying together | ✅ **`ControlGroup` NESTED INSIDE each `ToolbarItemGroup`** — ⚠️ **that is what makes macOS collapse a group into its own titled popup** |
| ⚠️ **A DUPLICATE NAVIGATOR BUTTON** — ⛔ same icon and function as the system control top-left | ⛔ **REMOVED**; ✅ the platform control was always there — ⚠️ **what was missing was STATE (Q1's binding)** |
| ⚠️ **HALF THE VERBS** — only 4 of 8 structure verbs surfaced | ✅ **all 8**, ✅ **plus Buffers · Worlds · Project Settings** |
| ⚠️ **TWO "VIEW" MENUS** — ⛔ `CommandMenu` CREATES a menu; macOS already synthesizes one | ✅ `CommandGroup(after: .sidebar)` |

⚠️ **THE VIEW-MENU DEFECT WAS ALREADY DOCUMENTED — ON THE OTHER PLATFORM.** ✅ **`ScriviApp.swift:320`:**
*"No `CommandMenu("View")` — iOS already synthesizes a 'View' menu; adding our own duplicated it."*
⛔ **Diagnosed on iOS, never applied to macOS** (`feedback_look_for_existing_pattern_first`).

⚠️ **AND A SIXTH, ENVIRONMENTAL:** ⛔ **the first test run could not launch** — ✅ **the user's app was
running** (`feedback_check_scrivi_running_before_tests`). ⛔ **The app was NOT killed; ✅ the user quit
it and the run then passed 132/132.**

---

## ✅ LIVE PASS 1a — ⚠️ **the grouping had to become FUNCTIONAL** (user, 2026-09-22)

⚠️ **THE USER ACCEPTED THE BUTTONS AND CORRECTED THE MECHANISM.** ✅ **The ruling was not "regroup
them" — it was that macOS 27 MANAGES grouped controls automatically when the context is narrow,
collapsing them to a popup**, ⛔ **and the implementation must actually opt into that.**

⛔ **`ToolbarItemGroup` ALONE DOES NOT.** ⚠️ **It is layout adjacency; at narrow widths its members
scatter into the generic overflow menu** — ✅ **losing exactly the "Scene" / "Chapter" identity the
grouping exists to carry.**
✅ **`ControlGroup` nested inside it is the construct that collapses**, ⚠️ **and its `label:` is what
the collapsed popup is TITLED with** — ✅ **so each is named for its MENU, and a writer who loses the
buttons still finds "Scene" and "Chapter".**

✅ **RULED SHAPE:** ⚠️ **Scene (ControlGroup) · Chapter (ControlGroup) · Visibility (ControlGroup) ·
Worlds and Project Settings FREE-STANDING.**
⛔ **The last two are deliberately NOT grouped:** ✅ **they are project-level surfaces, not a verb
family** — ⚠️ **collapsing them under a shared label would invent a grouping the menu bar does not
have.**

✅ **VERIFIED BEFORE WRITING:** ⚠️ **`ControlGroup` inside `ToolbarItemGroup` was `swiftc -typecheck`d
against the macOS 27 SDK** — ⛔ **not assumed from documentation** (⚠️ **the same discipline the blank
symbol defect should have had**).

---

## ⚠️ One thing S1b must settle DURING implementation

⚠️ **`.navigationTitle` / `.navigationSubtitle` are declared TWICE on macOS** — `EditorView.swift:186-187`
and `:230-231` — ✅ **and with a real toolbar in place, macOS may render them CORRECTLY rather than as
the stray band the conformance study flagged.**
⛔ **DO NOT DELETE THEM BLIND.** ✅ **Add the toolbar FIRST, look at the window, THEN decide whether the
modifiers stay, move, or go** — ⚠️ **the study's complaint was a title with nowhere to go, and the
toolbar may be the "somewhere".**
⚠️ **If they stay, AC1 is met by the toolbar EXISTING; ⛔ if a stray band persists alongside it, that
is a finding worth recording, not a silent deletion.**

---

## ⚠️ Risks

- ⚠️ **THE `columnVisibility` BINDING CHANGING BEHAVIOUR, not just exposing it.** ✅ **Q1 ruled it in**
  — ⛔ **but binding state SwiftUI currently manages itself can alter WHEN the sidebar shows**, ⚠️ **and
  this is a surface [I-0203] is already about.** ✅ **Verify the Navigator still appears by default on a
  normal open, at both wide and narrow window widths**, ⛔ **before judging the toolbar control.**
- ⚠️ **THE BINDING'S DEFAULT VALUE.** ⛔ **`.automatic` is not the same as `.all`** — ✅ **picking the
  wrong initial value could hide the Navigator on first open**, ⚠️ **which would REPRODUCE [I-0203]
  rather than fix it.**
- ⚠️ **Disturbing the iOS branch.** ✅ **It already has `.toolbar`** — ⛔ **and `feedback_prove_code_is_reached`
  records I-0161, where a fix landed on the iOS call site and macOS was never touched.** ⚠️ **Build all
  three platforms, and confirm the macOS path is the one that changed.**
- ⚠️ **Scope creep into [SP-135].** ⛔ **The banner, the bars and the Inspector column are NOT this
  Sprint** — ✅ **AC6 exists to make that checkable rather than assumed.**
- ⚠️ **A toolbar that looks right and does nothing.** ✅ **AC2 requires the SAME closures** —
  ⛔ **a re-implemented verb would be a second code path and is the defect this Epic exists to stop.**
- ⚠️ **Toolbar items vanishing at narrow widths.** ✅ **macOS collapses into an overflow menu;**
  ⛔ **a writer who cannot find a verb has the [I-0203] complaint again in a new form.** ⚠️ **AC8
  checks it deliberately.**

---

## ⛔ Out of scope

- ⛔ **The `safeAreaBar` conversion and [I-0203]'s structural fix** — ✅ **[SP-135].**
- ⛔ **The Inspector as a real column** — ✅ **[SP-136].** ⛔ **The Object Detail Sheet** — ✅ **[SP-137].**
- ⛔ **Export and the text-size lens.** ✅ **Slots only (AC5).**
- ⛔ **[I-0205]** (is the banner correct-but-ugly or FALSE?) — ⚠️ **[EP-040] AC9 requires answering it
  BEFORE the banner is restyled**, ✅ **which is [SP-135]'s problem, not this Sprint's.**
- ⛔ **iOS / iPadOS / visionOS.** ⚠️ **Explicitly deferred by the Epic** — ✅ **they must still BUILD (AC7).**
- ⛔ **Rich text, fonts, styling.** ⚠️ **`isRichText = false`; the body is Markdown and has nowhere to
  persist character styling.**

---

## ⚠️ Estimate

✅ **SHORT — the Epic's lowest-risk Sprint, and the study calls S1+S2 "LOW".**
⚠️ **The cost is S0 (three rulings) and Q1's answer**, ⛔ **not the toolbar itself.**
⚠️ **Q1 ruled the `columnVisibility` binding IN.** ✅ **Small — ~2 lines plus its toolbar control** —
⛔ **but it is the Sprint's ONLY non-additive edit, and the two risks above are both its.**
✅ **Q3's ruling made S1 SMALLER than planned:** ⚠️ **the subtitle is a relocation, not a decision.**


---

## ✅ Outcome

✅ **THE WINDOW HAS REAL CHROME.** ⚠️ **`NSToolbar` appeared ZERO times in the app before this Sprint.**
✅ **All eight structure verbs, the visibility toggles and the project surfaces are reachable from the
window**, ⚠️ **each calling the closure the menu bar already calls.**

✅ **[EP-040] AC2 and AC3 are MET.** ⚠️ **AC1 is MET IN SUBSTANCE — the toolbar exists and carries the
title** — ⛔ **but is NOT fully judged: [I-0243] (the title renders THREE times) is open.**
✅ **AC7's slot is declared; ⛔ building Export and the text-size lens stays out of scope.**

⚠️ **THE SPRINT'S REAL FINDING WAS NOT THE TOOLBAR.** ⛔ **The Scene Navigator had NO visibility state
at all** — ✅ **`columnVisibility` was `#if os(iOS)` only** — ⚠️ **which is why it was the one pane a
writer could not recover** ([I-0203]). ✅ **The platform's sidebar control was always there; STATE was
what was missing.**

## ⚠️ Audit-check findings, ruled as part of this close

⚠️ **SIX ROUNDS OF LIVE-PASS DEFECTS, AND THE BUILD WAS GREEN THROUGH EVERY ONE.**
✅ **`feedback_live_pass_finds_what_suites_cannot`, demonstrated six times in one Sprint.**
⚠️ **The sharpest: a BLANK BUTTON from an invented SF Symbol name** — ⛔ **SwiftUI renders a missing
symbol as an empty control with a working tooltip, so nothing failed.**
✅ **RULE ESTABLISHED AND NOW FOLLOWED: verify every symbol against `NSImage(systemSymbolName:)`, and
RENDER candidates before choosing between them** — ⚠️ **names lie (`arrow.merge` and
`arrow.trianglehead.merge` draw the SAME picture; `.alt` is NOT a double line).**

⚠️ **ONE DEFECT WAS ALREADY WRITTEN DOWN IN THIS REPO, ON THE OTHER PLATFORM** — ✅ **the duplicate
View menu** (`ScriviApp.swift:320`). ⛔ **That is `feedback_look_for_existing_pattern_first`, and it is
the SECOND time this Epic's work has hit a trap the codebase had already recorded.**

✅ **[I-0243] FILED, NOT ABSORBED.** ⚠️ **The title triplication and the tab-bar behaviour are real and
bigger than a toolbar Sprint** — ✅ **so they get their own record rather than growing this one.**
✅ **User ruled OPTION A (windows only) for the tab bar.**

⚠️ **AC7 PARTIALLY DEFERRED by user ruling** — ⛔ **iOS/visionOS cannot RUN the app (`ManuscriptView`
will not render)**, ✅ **so a future `[Apple]` layout Epic owns them**, ⚠️ **informed by this work.**

---

*Closed 2026-09-22 with user approval. T-0543 Verified by live pass; [EP-040] AC2/AC3 met.*
