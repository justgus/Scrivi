# Unverified Tasks

*Last Updated: 2026-11-02 (**T-0400, T-0626, T-0627, T-0629–T-0631, T-0640, T-0641, T-0643–T-0647 → ✅ Verified and archived** (user, 2026-11-02).)*

*Last Updated: 2026-11-02 (**T-0656** filed and implemented → 🟠.)*

*Last Updated: 2026-11-02 (**T-0646 → 🟠** from `Task-active.md`; its rig pass → [T-0653] ([EP-057]).)*

*Last Updated: 2026-11-01 (**T-0647** filed in SP-200 and implemented → 🟠.)*

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0656 — `[Linux]` Landing: a warm grey background with a little depth

**Epic:** none (a live-pass finding, EP-057) · **Status:** 🟠 Implemented - Not Verified · **Sprint:** none · **Date Requested:**
2026-11-02 (user: *"A slightly warm grey color should be used for the launch panel. maybe with a slight gradient, as if it had
some depth."*)

**Why:** on GNOME Wayland, Qt's Adwaita frame draws no shadow — it reserves a 10 px shadow margin and paints nothing in it
(checked in Qt 6.10.2's `qwaylandadwaitadecoration.cpp`: `ceShadowsWidth = 10`; `paint()` fills the title bar and draws a
1-line border only). Landing and a project window in the same system colour could not be told apart.

**What:** `platforms/linux/qml/Landing.qml` — a vertical gradient behind the `StackView`, light `#ece7e0` → `#d8d1c7`, dark
`#45403b` → `#2e2a27` (from `Application.styleHints.colorScheme`); the landing `Page` and `NewProjectDialog.qml` draw no
background (`background: null`). Linux only: on the Mac the system's window shadow does this.

**Tested:** builds; smokes 38/38; an offscreen launch logs no QML warning. ⏳ The look is the user's call, on the VM.

---

## 🟠 T-0637 — `[Apple]` EP-055 S2: the iOS gestures and the Timeline grabber, with routes that need no gesture

**Epic:** [EP-055] (AC6, AC8) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-31) — ⏳ the simulator live passes · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** design §8. Compact: swipe left goes to the next page and swipe right to the previous one (the system back swipe).
Regular: edge swipes show and hide the Navigator and Inspector, and full-width swipes combine them (G1). Both: the Timeline
grabber (G2). G4: the View menu and shortcuts, VoiceOver actions, and the ⋯ menu. ⛔ Nothing may take the manuscript's
scrolling, selection or the system's text gestures.
**Tests:** an iOS test for the gesture → panel-state mapping; live passes.
**Done (2026-10-31):** new `Scrivi/Views/PanelSwipes.swift`. `PanelSwipe.actions` is the rule table: start within 24 pt of an edge,
end in the outer 12 %. `PanelSwipeInstaller` puts one pan recognizer on the window. It recognises alongside everything, cancels no
touches, ignores the Timeline's frame, and acts only on a quick, mostly sideways swipe (|dx| > 2|dy|, > 60 pt, > 300 pt/s).
`TimelineGrabber` sits above the Timeline at every width. G4: the ⋯ menu (at width compact, "Scene Inspector" goes to the page),
VoiceOver actions on the editor (Scene Navigator, Scene Inspector, Show/Hide Timeline), and the iPad Project menu's existing
⇧⌘I / ⇧⌘T. ⚠️ Those are not the Mac's ⌥⌘I / ⌥⌘T: ⌥⌘ collides with the menu iOS synthesizes (`ScriviApp.swift:531`).
Tests: `PanelSwipeTests` (2), iOS 14/14; Mac interop 327/327; `check-layout-convergence.sh` clean.

**iPad live pass 1 (user, 2026-10-31):** the panels and gestures work, but swipes often needed a second try, swiping up on
the grabber also raised the Dock, the Timeline's horizontal scroll stopped working, and ⇧⌘I showed a panel but did not hide it.
**Fixed:**
- Panels start hidden on iOS and their show/hide is kept for the session only (`ProjectSession.swift` restore and didSets).
- Swipes are counted only when they start on `ManuscriptUITextView`, with a 44 pt edge zone and a looser speed test.
- ⚠️ The Timeline exclusion had never been wired in (the edit silently missed).
- Tapping the grabber toggles the Timeline.
- The iOS Project-menu toggles are now Buttons that flip the live value.
iOS 14/14, Mac interop 327/327.

**iPad live pass 2 (user, on a real iPad with a hardware keyboard, 2026-10-31):** panels hidden on open, the grabber tap,
⇧⌘I / ⇧⌘T, and the Mac's panels left unchanged all passed. Swipes were better but still sometimes needed a second try, and the
Timeline still would not drag. ⚠️ **Correction to pass 1:** the swipe recognizer never took the Timeline's scroll. iOS never had a
Timeline pan: the only pan is the Mac's scroll-wheel capture (`TimelineStripView.swift`, `TimelineScrollCaptureView`).
**Fixed:** a drag on the Timeline's empty space pans on iOS, sharing `pan(by:usable:)` with the Mac's wheel. Like the wheel, it
pans only when zoomed in (pinch). Added a temporary `[SCRIVI-SWIPE]` log in `PanelSwipes.swift` so the swipe misses can be
measured. iOS 14/14, macOS build succeeded.
## 🟠 T-0636 — `[Apple]` EP-055 S2: panels by size class — the compact stack and the regular split

**Epic:** [EP-055] (AC6, AC8) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-31) — ⏳ the simulator live passes · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** `EditorView`'s iOS container laid out by horizontal size class. **Compact:** one navigation stack, Navigator →
Manuscript → Inspector, the Inspector pushed as a page (G3), opening on the Manuscript. **Regular:** the `NavigationSplitView`
sidebar plus the `.inspector` trailing column, as on the Mac, with two- and three-panel versions shown to the user for a
choice. The `userInterfaceIdiom` branches (`EditorView.swift:481`) go.
**Tests:** the iOS target, the Mac interop tests, live passes on the iPad, iPhone and Duo simulators.
**Done (2026-10-31):** `EditorView.swift` iOS `container`. **Width compact** is a three-column `NavigationSplitView(preferredCompactColumn:)`
(Navigator | Manuscript | Inspector) opening on `.content`; the system collapses it to the ruled stack, with its own back button and back
swipe. Choosing a scene goes to the Manuscript page. **Width regular** is the two-column split plus the `.inspector` column; `columnVisibility`
is seeded from the saved navigator state and recorded back, as on the Mac (I-0255). The iOS `HStack` Inspector and both
`userInterfaceIdiom` gates (the Inspector, the Timeline) are gone. **Measured on the Duo simulator** with a probe: setting
`preferredCompactColumn` shows each of the three pages, and the Duo's window was 466×678 at width compact, with the back control on the
**right**. ⏳ Two- or three-panel at width regular is the user's call, made by showing and hiding panels on the iPad simulator.

## 🟠 T-0638 — `[Apple]` EP-055 S2: the Mac's toolbar buttons on iOS, at the system's placement

**Epic:** [EP-055] (AC6) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-31) — ⏳ the user judges look and position on the simulators · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** the Mac's toolbar content (`ProjectWindowManager.swift`: the Scene and Chapter groups, Worlds, Project Settings) shared
with iOS, at the default placement: the top on iPad and iPhone, the right side on the Duo. ⛔ No Show group (G4) and no
placement code ([SP-198] R3). A button whose action is not wired on iOS yet is disabled, as on the Mac.
**Tests:** builds on both platforms; the user judges look and position on the simulators.

*Last Updated: 2026-10-30 (**T-0632–T-0635 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-30 (**T-0635 filed → active** in SP-198; T-0634 rescoped by [SP-198] R3.)*

*Last Updated: 2026-10-29 (**T-0632, T-0633, T-0634 filed → active** with SP-198's activation (EP-055 S1).)*

*Last Updated: 2026-10-27 (**T-0629, T-0630, T-0631 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-27 (**T-0629, T-0630, T-0631 → active** with SP-197's activation (EP-052 S3).)*

*Last Updated: 2026-10-27 (**T-0627 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-27 (**T-0627 → active** with SP-196's activation (EP-052 S2b).)*

*Last Updated: 2026-10-26 (**T-0626 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-26 (**T-0625 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-26 (**T-0625, T-0626 → active** with SP-195's activation (EP-052 S2a).)*

*Last Updated: 2026-10-26 (**T-0623, T-0624 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-26 (**T-0623, T-0624 → active** with SP-194's activation (EP-052 S1).)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-24 (**T-0622 → active** with SP-193's activation (EP-054 S0).)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-24 (**T-0620 → active** with SP-192's activation.)*


*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-22 (**T-0616 → active** with SP-188's activation (EP-053 S3).)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-21 (**T-0619 → active** with SP-191's activation (EP-053 S2b).)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → active** with SP-187's activation (EP-053 S2a).)*

*Last Updated: 2026-10-20 (**T-0614 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0614 filed → active** with SP-186 (EP-053 S1).)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0613 filed** in [SP-185] (planned, awaiting go-ahead).)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0612 filed → active** with SP-185's activation.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-15 (**T-0609 → active** with SP-182's activation.)*

*Last Updated: 2026-10-15 (**T-0608 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-15 (**T-0608 → active** with SP-181's activation.)*

*Last Updated: 2026-10-13 (**T-0607 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-13 (**T-0607 → active** with SP-180's activation.)*

*Last Updated: 2026-10-12 (**T-0606 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-12 (**T-0606 → active** with SP-179's activation.)*
**Done (2026-10-31):** new `Scrivi/Views/ProjectToolbar.swift` holds the Mac's toolbar as a `ToolbarContent`, moved unchanged from
`ProjectWindowManager.swift`. The Show group is `#if os(macOS)` (G4). The Mac window uses it at `ProjectWindowManager.swift:353`,
and the iOS editor at `EditorView.swift:217`, beside the ⋯ menu. ⚠️ On iOS the Scene and Chapter buttons are **disabled**,
because the `UITextView` host does not set `session.*Action` yet (that is structural edits, EP-055 AC5). Worlds and Project
Settings work. Mac interop 327/327, iOS 12/12.

## 🟠 T-0632 — `[Apple]` EP-055 S1: the manuscript host gap audit — what is AppKit, what is shared, the iOS shape

**Epic:** [EP-055] (AC1) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-29 (user: *"we should focus on the Manuscript View first"*)

**What:** every behaviour in `ManuscriptTextView.swift` (the representable `:37`, the `Coordinator` `:287`, `ManuscriptNSTextView`
`:2614`, `SceneBoundaryTable` `:3537`) and in `ManuscriptCommands` / `ManuscriptFind` / `ManuscriptAccessibility`, each given one
disposition: **shared as is**, **shared after a font/colour swap**, **moved out of the host into shared code**, or
**re-implemented on UIKit** (and why). Ordered by what S1, S2 and S3 need. Answers [SP-198] Q1 for the user to rule.
**Output:** `docs/Scrivi_iOS_Manuscript_Design_v0_1.md`.
**Done (2026-10-30):** `docs/Scrivi_iOS_Manuscript_Design_v0_1.md`. Finding: the bytes a keystroke stores were decided in
`ManuscriptNSTextView`'s AppKit overrides. ✅ Q1 = (c) and undo off in S1, both ruled by the user 2026-10-30.

---

## 🟠 T-0633 — `[Apple]` EP-055 S1: the presentation layer compiles for iOS, the Mac unchanged

**Epic:** [EP-055] (AC2) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-29

**What:** `ManuscriptPresenter`, `ManuscriptPresentedMap` and `ManuscriptTypography` (and whatever else [T-0632] rules shared)
built for both platforms, with UIKit's font and colour types on iOS. No behaviour change on the Mac.
**Tests:** the Mac's interop tests (`scripts/run-interop-tests.sh`), `check-textkit2.sh`, `check-manuscript-type-source.sh`; the
iOS target builds.
**Done (2026-10-30):** `ManuscriptPresenter`, `ManuscriptCommands` and `ManuscriptTypography`'s drawing half build on every Apple
platform (`PlatformFont` / `PlatformColor`, `ManuscriptTypography.swift:1–22`). New `ManuscriptEditing.swift`: the shared types
moved out of `ManuscriptTextView.swift` (`SceneDivider`, `SceneBoundaryTable`, `DividerTextAttachment` with a UIKit drawing and
the ONE factory `DividerTextAttachment.make`); **C1** `ManuscriptEditRules` (the edit verdict, Return, ⌫/⌦) and **C2**
`ManuscriptSceneSync` (render, edit and caret sync, autosave). The Mac's `shouldChangeText`, `insertNewline`, `deleteBackward`,
`deleteForward`, `textDidChange`, `textViewDidChangeSelection` and render now call them. `check-manuscript-type-source.sh` also
catches `UIFont` / `PlatformFont`. **Evidence:** Mac interop 324/324 after each move; both guards pass; macOS, iOS and visionOS build
with no warnings. ⚠️ The `ScriviInteropTests` target was raised to macOS 27.1 to match the app (it no longer compiled).

---

## 🟠 T-0634 — `[Apple]` EP-055 S1: the `UITextView` manuscript on iPad and iPhone

**Epic:** [EP-055] (AC2, AC3) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-29 (user: *"It should work with the keyboard overlay and a hardware keyboard on both the iPad, iPhone, and iPhone Duo"*)

**What:** the iOS branch of `ManuscriptTextView` replaced by a TextKit 2 `UITextView` host on [T-0633]'s presenter: the viewport
scenes load, read and scroll; typing autosaves through the same path as the Mac; selection; dividers and chapter titles.
⛔ Nothing device- or keyboard-specific: the system's default keyboard and layout behaviour ([SP-198] R3).
**Tests:** [T-0635]'s target (a scene typed on the simulator reaches its `.md` file on disk); live passes on the user's iPad
and iPhone.
**Done (2026-10-30):** `ManuscriptUITextView.swift` — the `UIViewRepresentable`, its coordinator (rebuild, restore, navigate, the
viewport scene from scrolling) and `ManuscriptUITextView` (typing escaped, Return, ⌫, copy/paste as on the Mac; a cross-scene
copy/cut does nothing yet; system undo refused). The iOS stub is gone. ⚠️ Measured: UIKit does not ask `shouldChangeTextIn` for
every `insertText`/`deleteBackward`, so the overrides apply the rules themselves; and `UITextView(usingTextLayoutManager:)`
left the subclass's stored properties uninitialised (nil), so it has a designated `init()` (TextKit 2, asserted).
**Evidence:** the iOS suite (T-0635); a render of the real host on the simulator showed titles, headings, emphasis, hidden escapes
and both dividers.

---

## 🟠 T-0635 — `[Apple]` EP-055 S1: an iOS test target

**Epic:** [EP-055] (AC4) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-30 (user: *"We will need to create a test target for iOS."*)

**What:** a test target hosted by `ScriviApp-iOS`, at 27.1, run on the iOS Simulator by `xcodebuild test`, with the
manuscript's tests in it. Whether the Mac's interop tests that are not AppKit-bound also run there is [T-0632]'s to say.
**Tests:** it runs, and reports more than zero tests.
**Done (2026-10-30):** the `ScriviTests-iOS` target (hosted by `ScriviApp-iOS`, 27.1) in the `ScriviApp-iOS` scheme;
`Scrivi/Tests/ScriviManuscriptTests-iOS.swift`. **Evidence:** 10 tests in 2 suites pass on the iPhone 18 Pro simulator (iOS 27.2),
including a scene typed on iOS autosaved to disk, escaped. ⚠️ It needs a 27.1+ simulator runtime (the 27.0 devices are refused).

---

## 🟠 T-0616 — `[Cross]` EP-053 S3: structural cut, Backspace / Delete over scenes, and their undo

**Epic:** [EP-053] (S3) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-22) — ✅ rig over the share, ✅ Mac on a local disk; ⏳ the Mac over a share → [SP-190] · **Sprint:** [SP-188] (✅ Closed 2026-10-26) →
[`../Sprints/Closed/Sprint-SP-188.md`](../Sprints/Closed/Sprint-SP-188.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** on the Mac and on Linux, a cut, buffer cut, or Backspace / Delete across scenes removes the structure (design R4,
§4.3, Q2, Q4) through one `scrivi_range_cut` off the main thread, the delta applied, no reload; undo is a range paste
(`rangeCut` node), redo the cut. The core balances the cut's join and returns the markers it added, so the undo is exact.
The clipboard (or slot) holds the cut's returned Range with its sidecars; a selection crossing a divider takes that scene
whole; Backspace over one scene divider merges; paste over a cross-scene selection is the cut, then the paste. Old
`structuredCut` nodes and [I-0270]'s grouped deletes still undo. Retires both platforms' `deleteAcrossScenes`.

**Done (2026-10-22):** ScriviCore (`RangeJoin`, the balanced join in `cut()`, applied and checked in `paste()`); the Mac
(`selectionRun`, `cutAcrossScenes(_:keeping:then:)`) and Linux (`ManuscriptEditor::selectionRun`,
`EditorShell::cutAcrossScenes`) — every cross-scene cut, Backspace / Delete and paste-over one range call off the main
thread, the delta applied, undone in one step; `deleteAcrossScenes` retired on both. ctest 763 (Mac) / 767 (Linux),
interop 311, Linux smokes 38; mutations core 6, Mac 8, Linux 7, all caught.

---

*Last Updated: 2026-10-26 (**T-0616**: [SP-188] closed; its last check, the Mac pass over a share, → [SP-190].)*

*Last Updated: 2026-10-25 (**T-0622 → ✅ Verified** (user) and archived → `Verified/Task-verified-0622.md`, with [SP-193]'s close.)*

*Last Updated: 2026-10-25 (**T-0622**: ✅ Mac half passed (user: About shows the version and hash); ⏳ the rig pass.)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** ([SP-193]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-24 (**T-0620 → ✅ Verified** (user) and archived → `Verified/Task-verified-0620.md`, with [SP-192]'s close.)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** ([SP-192]); ⏳ Mac live pass.)*

*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** ([SP-188]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-22 (**T-0612 → ✅ Verified** (user) and archived → `Verified/Task-verified-0612.md`, with [SP-185]'s close.)*

*Last Updated: 2026-10-22 (**T-0619 → ✅ Verified** (user) and archived → `Verified/Task-verified-0619.md`, with [SP-191]'s close.)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** ([SP-191]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-21 (**T-0605, T-0607, T-0608, T-0609, T-0613, T-0615 → ✅ Verified** (user: Mac + rig live passes, build 75) and archived → `Verified/Task-verified-XXXX.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** ([SP-187]); ⏳ live passes.)*

*Last Updated: 2026-10-20 (**T-0614 → ✅ Verified** (user) and archived → `Verified/Task-verified-0614.md`, with SP-186's close.)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** ([SP-182]); ⏳ rig live pass.)*

