# Unverified Tasks

*Last Updated: 2026-11-01 (**T-0647** filed in SP-200 and implemented → 🟠.)*

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0647 — `[Cross]` Properties tab: scene links, Project and World metrics

**Epic:** [EP-052] (AC11) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-01) — ✅ Mac live pass (user, 2026-11-01: *"All steps pass."*); ⏳ rig live pass · **Sprint:** [SP-200] (🟡 Active) →
[`../Sprints/Sprint-SP-200.md`](../Sprints/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What (user, 2026-11-01):** *"Your recommendations for the metrics are approved … do both the scene links in the scene
portion and the total world links in the project section. Perhaps we can also do a 'World' metrics with a breakdown of the
number of what kind of objects exist (assuming you have that information without resorting to counting them)."*
**Done (2026-11-01):**
- **Core:** one new endpoint, `scrivi_get_project_metrics` (`scrivi.h:285`, `scrivi_c_api.cpp:1684`): scenes and chapters
  from the warm project index; links and world links from the relationship log; history steps and the active segment's
  bytes from the open store (`HistoryStore::segmentBytes`, memory only); per bound world, objects by kind from the world's
  own `index.json`, or the binding's cached index while the world is away (`fromCache`). Kinds in `kAllStorableKinds`
  order, never a restated list. No scene, object or history file is opened. ABI: one endpoint added.
- **Mac / iOS** (`ScenePropertiesView.swift`): Scene section gains **Links** (from the Inspector's existing read, no extra
  call); new **Project** section (Scenes, Chapters, Links, World links, History "4,612 steps · 18.7 MB") and one **World —
  name** section per bound world (Objects, then each kind). Read off the main actor each time the scene's read lands.
  `ScriviEngine.getProjectMetrics` + visionOS stub.
- **Linux** (`SceneInspector.cpp:1001`, `ScriviBridge::getProjectMetrics`): the same rows and sections, read on the
  Inspector's worker after the scene's read.
**Tests:** `[T-0647]` (`ObjectCApiTests.cpp:1337`, through `scrivi_*`); Mac interop `projectMetricsCrossTheBoundary`.
ctest 800/800, Mac interop 329/329, iOS 17/17, Linux smokes 38/38 (Docker, offscreen, non-root).
⚠️ The figures refresh when the scene's read lands (scene change, world or object change), not on every keystroke: the
history count can trail typing until the next scene change.

## 🟠 T-0645 — `[ScriviCore]` History load b3: the structural payload stored as JSON

**Epic:** [EP-052] (AC11) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-01) — ⏳ Mac and rig live passes · **Sprint:** [SP-200] (🟡 Active) →
[`../Sprints/Sprint-SP-200.md`](../Sprints/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** design §3 b3. Written as an object, read as an object or a string. Core only. 38% of dumas's log.
**Done (2026-11-01):** written as an object, read as an object or a string (`HistoryStore.cpp:460`). Test `[T-0645]`:
the log's shape, and a log rewritten into the old string shape still returns the payload. Linux's
`structured_paste_smoke` read the log's payload as a string; it reads both now. ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).

## 🟠 T-0644 — `[ScriviCore]` History load b2: barrier texts stored as changes

**Epic:** [EP-052] (AC11) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-01) — ⏳ Mac and rig live passes · **Sprint:** [SP-200] (🟡 Active) →
[`../Sprints/Sprint-SP-200.md`](../Sprints/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** design §3 b2. `sceneTexts` as per-scene diffs against the history's text at the parent; `sceneTextsBefore` only
when it differs (Q2); expanded to full texts during the load. 27% of dumas's log.
**Done (2026-11-01):** a barrier's texts are stored as a change against the history's text at its parent
(`sceneTextDiffs`), a before-text equal to it as its name only (`sceneTextsBeforeSame`), a different one as a change;
a scene with no history text then keeps its full text (`persistableBarrierTexts` / `expandBarrierTexts`,
`HistoryService.cpp:280`, `:295`; `HistoryStore.cpp:79`). The load expands them in one walk (`:1332`), which also gives
an older log's barriers their changes, so a compaction writes the new shape. ⚠️ That walk and the consistency check now
share `walkWithTexts` (`:1243`): one text map changed on the way down and restored on the way up — a copy per fork was
the whole manuscript per branch (generated history's open 626 → 331 ms). Test `[T-0644]`: the scene's text is in the log
once (its floor), and after a reopen the next edit undoes to what the op left; a mutation (no expansion) is caught. The
I-0288 legacy test's fabricated old log strips either shape. ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).

## 🟠 T-0643 — `[ScriviCore]` History load b1: a diff is a list of hunks

**Epic:** [EP-052] (AC11) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-01) — ⏳ Mac and rig live passes · **Sprint:** [SP-200] (🟡 Active) →
[`../Sprints/Sprint-SP-200.md`](../Sprints/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** design §3 b1. `Diff` holds hunks; the old one-change shape is read as one hunk; apply, reverse, the match check,
the I-0107 rebase and the tree's change span follow. `computeDiff` finds several hunks as Q3 rules. Replace All's
diffs are 20% of dumas's log.
**Done (2026-11-01):** `Diff` keeps its first change and adds `more`; `computeDiff` (`HistoryService.cpp:168`) splits a
changed middle over 4 KB with a Myers diff (`:124`) — ⚠️ **by word, not by character** (300 replaced short words are ~600
word edits but ~1,800 character edits; a word never splits a UTF-8 character); merged within 8 bytes, capped at 64.
Apply, reverse, the match check and the I-0107 rebase handle every change; on disk one change keeps the v0.1 shape,
several are `{"hunks":[…]}` (`HistoryStore.cpp:41`). Tests `[T-0643]` (ASCII with a length change, UTF-8, 350 matches
merged to 64; the log's shape and an exact undo after a reopen); a mutation (reverse not shifting) is caught — the first
cases kept every length equal and missed it. ⚠️ **Measured on dumas: 6.86 of its 6.91 MB of Replace All diffs are UNDER
4 KB, so the ruled threshold changes nothing there; 1 KB would take them to 4.6 MB.** ✅ Kept at 4 KB, and the by-word diff accepted (user, 2026-11-01: *"no 4KB is sufficient. These departures are accepted."*). ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).

## 🟠 T-0400 — `[ScriviCore]` History log-segment rotation → (c) compaction

**Epic:** EP-052 (AC11) — was EP-019 (deferred) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-01) — ⏳ Mac and rig live passes · **Sprint:** [SP-200] (🟡 Active) →
[`../Sprints/Sprint-SP-200.md`](../Sprints/Sprint-SP-200.md). Was 🟢 Nice to have (user ruling 2026-08-11); scheduled
2026-11-01 (user: *"I'd like to do all three (a), (b), and (c)"*).

**Now (2026-11-01):** design [`../Scrivi_History_Load_Design_v0_1.md`](../Scrivi_History_Load_Design_v0_1.md) §2 — the
live tree written to `log-(N+1).jsonl` through the write queue at open and close when worth it (Q1), `state.json` moved to
it, old segments removed; the open replays only that segment. ⚠️ **Correction:** the text below says the reader honours
`activeLogSegment`; it does not — `state.json` is read only after `log-000001.jsonl` has been replayed
(`HistoryStore.cpp`), so reading it first is part of this Task. dumas's log is now 34 MB.

**As filed:**

`activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); nothing rotates it, so
a project accumulates one unbounded segment (a real project is already ~3.4 MB). Capacity/eviction
bounds the **tree**, not the **log**. The reader already honours `activeLogSegment` from `state.json`,
so rotation is **additive, with no format change**. Documented as not-implemented in package-structure
§16a. Raised by T-0217; deliberately **not** folded into a verification sprint.
**Done (2026-11-01):** `HistoryStore::resolveActiveSegment` (`HistoryStore.cpp:640`) opens the segment `state.json`
names (else the newest) and removes the others; `compactIfWorthIt` / `compact` (`:664`, `:678`) write the live tree as log
records — the last session closed, floors, a promoted root named by a new `ctl:root`, every live node parents-first, fork
primaries, the current node, the persisted hashes, `ctl:compacted` — through the write queue, then `state.json`. Called at
open (after the load) and at close (`scrivi_c_api.cpp:4238`); the queue is connected before the open so the open's
compaction is queued. ⚠️ Q1's "10,000 in all" is counted since the LAST compaction: a 20,000-event live tree always holds
more, and would be rewritten at every open. Tests `[T-0400]` (`HistoryCApiTests.cpp`): a 2,500-event history at capacity
50 with a re-selected fork, a barrier and an edit group compacts at close and reopens as the same tree and undo texts; the
three crash points of design §2.2; a short history is not compacted. A mutation (no fork-primary records) is caught — the
first version of the test missed it ([I-0332] notes why). Real logs (a scratch harness): dumas and the generated history
open as the same tree and the same 300 undo texts before and after. ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).





## 🟠 T-0641 — `[Cross]` A time scale along the top of the Timeline

**Epic:** [EP-055] · **Status:** 🟠 **Implemented - Not Verified** (2026-10-31) — ✅ Apple: iPad and Mac live pass (user, 2026-11-01: *"All these steps passed on both iPad and the Mac."*); ⏳ Linux rig live pass · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What (user):** ticks for decades, years, months, weeks, days, hours as the zoom allows, as a reference for where a scene
sits in time and as a sign of movement while scrolling across empty stretches. **Ruling (2026-10-31):** labels are offsets
from Story Open (no calendar, Timeline design principle 5).
**Done (Apple):** `TimelineScale` (`TimelineStripView.swift:3712`) picks the smallest step from 1 min to 1,000 years that keeps
labels 90 pt apart and labels each tick "Day 3 14:00", "Day 3", "Week 7", "Month 15" or "Year 2", counted from 1 at Story Open
(month = 30 days, year = 365, the Time Delta Picker's units). Drawn as a 16 pt row under the header (`:1840`), in the dots' x
mapping. Checks: iOS 17/17 (`TimelineScaleTests` 3 new), Mac interop 327/327, Mac build, `check-layout-convergence.sh` clean.
**Linux (2026-11-01):** `TimelinePanel::scaleTicks` / `scaleLabel` (`platforms/linux/src/TimelinePanel.cpp:729`), the same steps
and labels as Apple, painted as a 16 px row along the top of the strip (`:948`), above the bands; the strip's minimum height
80 → 96. Checks: `timeline_cluster_smoke` (the cases of Apple's `TimelineScaleTests`), Linux smokes 38/38 (Docker, Qt 6.4,
offscreen, non-root). ⏳ Rig live pass.

## 🟠 T-0640 — `[Cross]` Zoom to the current scene

**Epic:** [EP-055] · **Status:** 🟠 **Implemented - Not Verified** (2026-10-31) — ✅ Apple: iPad and Mac live pass (user, 2026-11-01: *"All these steps passed on both iPad and the Mac."*); ⏳ Linux rig live pass · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What (user):** a small Timeline button that zooms in until single dots show and centres on the scene the Manuscript and
Navigator have selected. **Ruling (2026-10-31):** zoom until nearby dots separate.
**Done (Apple):** a `scope` button in the Timeline header (disabled with no current scene). `zoomToCurrentScene`
(`TimelineStripView.swift:1827`) finds the nearest other main-row item and zooms so it sits a twelfth of the window away
(capped by the existing `maxZoom`), then centres on `loader.viewportSceneID`. Checks: iOS 17/17 (`TimelineScaleTests` 3 new), Mac interop 327/327, Mac build, `check-layout-convergence.sh` clean.
**Linux (2026-11-01):** a `◎` button left of the +/− control (`platforms/linux/src/TimelinePanel.cpp:51`), disabled with no
active scene; `TimelinePanel::zoomToActiveScene` (`:695`) is Apple's rule (nearest other project-row dot a twelfth of the
window away, zoom capped at 500), centres the active scene and saves the view (T-0338). Checks: `timeline_cluster_smoke`
(a clustered scene separates and is centred; the view is saved), Linux smokes 38/38. ⏳ Rig live pass.

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

## 🟠 T-0629 — `[ScriviCore]` EP-052 S3: one Inspector read per scene, and the per-scene searchable text

**Epic:** [EP-052] (AC8, AC9) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ⏳ the Mac and rig live passes ([SP-197] AC5) · **Sprint:** [SP-197] (🟡 Active) → [`../Sprints/Sprint-SP-197.md`](../Sprints/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** (a) one endpoint returning everything the Inspector shows for a scene: its notes, its edges with each far
object's name, kind, image path and pending state, the sources' citations, and the bound worlds' status, with the object
index built once (v0.2 R3b; [SP-197] Q1). It replaces the 32 calls the Mac makes today (`ObjectCard.swift:157,179,614`,
`SourcesCard.swift:89,107`, `WritingToolCards.swift:37`, `ScenePropertiesView.swift:133`) and Linux's
(`SceneInspector.cpp:404–409,856`). An unreachable world keeps today's behaviour ([I-0222], D8): its edges still list by
cached name. (b) `scrivi_extract_searchable_text` for a list of scene IDs (`scrivi.h:684` takes a project only).
**Tests:** boundary tests through `scrivi_*`; the file-operation count for one scene asserted (CountingFileSystem); an
unreachable world; Engine stub parity (`check-engine-stub-parity.sh`) and the Linux bridge.

**Done (2026-10-27):** `scrivi_load_scene_inspector` (`scrivi_c_api.cpp:3477`): notes, edges, the far objects (the
object index built once, narrowed to the scene's far endpoints), worlds and, when asked, each reachable far object's own
edges, under ONE project lock, each part its standalone endpoint's builder (`EndpointPart`, shared with
`scrivi_load_timeline`), a failing part as `<part>Error`. An empty part is `{}`, never `null`. `scrivi_extract_searchable_scenes`
(`:3588`): the named scenes' records only (`ExtractSearchableTextRequest::scenesOnly`). Bound in Swift (`loadSceneInspector`,
`extractSearchableScenes`) and on Linux (`ScriviBridge::loadSceneInspector`). Each envelope's trace now names its project, so a
test can count one project's main-thread calls apart from the others running beside it. **Evidence:** ctest 786/786 (5 new
`[SP-197]` boundary cases: parts equal the standalone endpoints, one revision; a world away; nothing related; a missing root;
the per-scene extraction). File operations on the fixture: the composite **67**, against **353+** for the per-card calls it
replaces. ⚠️ One unidentified ctest failure in the first full run after a rebuild; 18 full runs since are clean.

---

## 🟠 T-0630 — `[Cross]` EP-052 S3: the Inspector and the timeline read off the main thread, revision-checked

**Epic:** [EP-052] (AC8) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ⏳ the Mac and rig live passes ([SP-197] AC5) · **Sprint:** [SP-197] (🟡 Active) → [`../Sprints/Sprint-SP-197.md`](../Sprints/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** Mac and Linux in the same work (`feedback_linux_adopts_apple_shape`). The Inspector shows the new scene's cards
at once ([SP-197] Q2), runs [T-0629]'s read as ONE job off the main thread, and fills the cards when it lands if
`scrivi_get_project_revision` has not moved (D4); otherwise it reads again. The timeline's reloads do the same: the open's
main-thread fallback (`ProjectSession.swift:328`) and Linux's `reloadTimeline()` (`EditorShell.cpp:3351`). Linux's
object-card worker (`SceneInspector.cpp:399`) is the shape; its notes read (`:856`) joins it.
**Tests:** the inventory probes (`MainThreadInventoryTests`; `SCRIVI_INVENTORY=1 structural_baseline_smoke`) assert 0 file
operations on the main thread for click, select, scroll, navigator and the Inspector following; a result whose revision
moved is dropped.

**Done (2026-10-27):** Mac: `SceneInspectorReader` (`InspectorCard.swift:199`), owned by `SceneInspectorView`, makes ONE read
per scene off the main actor; every card (tags, outline, todo, the ten object cards, Sources) and the Properties tab are built
from it (`CardContext.scenePart`); a result whose revision moved is read again; while the read is out the cards keep their last
contents dimmed and take no edits, "Loading…" after 400 ms (Q2, `sceneReadGate`); a card's own write re-reads the scene. The
open's timeline fallback is re-read off the main actor (`ProjectSession.swift:560`). Linux: the Inspector's worker makes the one
call, the notes included (`SceneInspector.cpp:374`), revision-checked, the last contents dimmed, "Loading…" after 400 ms; the
load's stale timeline is re-read on a worker (`EditorShell.cpp:3355`). **Evidence:** interop 323/323, new
`MainThreadNoFileOpsTests` (0 main-thread file ops for click, select, scroll, navigator and the Inspector following; a card's
write re-reads); Linux smokes 38/38, `structural_baseline_smoke`'s new SP-197 gate (0 GUI-thread file ops for click, select,
scroll) — a mutation in a copy (the notes read back on the GUI thread) fails it on all four gestures.

---

## 🟠 T-0631 — `[Apple]` EP-052 S3: Spotlight on one serial background queue, per-scene, behind the saves

**Epic:** [EP-052] (AC9) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ⏳ the Mac and rig live passes ([SP-197] AC5) · **Sprint:** [SP-197] (🟡 Active) → [`../Sprints/Sprint-SP-197.md`](../Sprints/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** v0.2 §5 S1–S4. Every `CSSearchableIndex` call (donate, re-index, delete on close) runs on one serial queue
(Apple: one thread at a time). The open's donation and the app-switch re-index (`ProjectSession.swift:541,550`) leave the
main thread; the extraction is a read behind the queue's saves; the app switch re-indexes only the scenes typed in
([T-0629] (b)); a close cancels a pending job. Only `spotlightDomainIdentifier` is set on the main actor, when the job lands.
**Tests:** the inventory's open and app-switch rows read 0 file operations on the main thread; a close during a donation.

**Done (2026-10-27):** every Spotlight call runs on one serial background queue (`SpotlightDonor.enqueue`,
`SpotlightDonor.swift:40`): the open's donation and the app-switch re-index extract there, behind the queue's saves; the app
switch re-indexes only the scenes typed in (`extractSearchableScenes`); a close cancels a pending job and deletes the domain
behind any donation already started (by the projectID, so it is known before the first donation lands). **Evidence:** interop
323/323; build clean. ⚠️ No test drives a close during a donation.

---

## 🟠 T-0627 — `[Cross]` EP-052 S2b: structural edits through the queue, and the gesture cues

**Epic:** [EP-052] (AC4, AC5, AC10) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ✅ Mac live pass; ⏳ the rig live pass ([SP-196] AC4) · **Sprint:** [SP-196] (🟡 Active) →
[`../Sprints/Sprint-SP-196.md`](../Sprints/Sprint-SP-196.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** on both platforms a structural edit is `scrivi_enqueue_range_op` behind the queued saves; the delta comes back in
its outcome and is applied on the main thread; its history node is recorded from it. Each platform's structural worker is
removed (W7h). The v0.2 Q5 gesture cues.

**Done (2026-10-27):** each structural edit — paste, split, chapter split, merge, chapter merge, cut / Backspace / Delete over
scenes, and every undo and redo of them — is queued (`enqueueRangePaste` / `Cut` / `Relabel`, `ScriviEngine.swift:1334`,
`ScriviBridge.cpp:397`) behind the unsaved scenes' saves (W7b), OFF the main thread; its outcome comes through the ONE write
waiter (`ViewportSceneLoader.swift` `apply(_ wait:)`, `EditorShell.cpp:1689`), and its history node and story times are made
off the main thread before the delta is applied. The synchronous structural call and its worker are gone (W7h)
(`ViewportSceneLoader.swift:551`, `ManuscriptTextView.swift:1372`, `EditorShell.cpp:2037`). Gesture cues (v0.2 Q5): the
cue at once, a repeat ignored, and ⌘Z / Ctrl+Z pressed while an edit is in flight WAITS ITS TURN
(`ManuscriptTextView.swift:390`, `EditorShell.cpp:2502`). A structural edit behind a held queue is refused at once; one that
is itself held keeps the editor read-only until it lands, and a close asks rather than waits. **Evidence:** interop 321/321
(2 new: `ScriviInteropTests.swift:7799`, `:7828`); Linux smokes 38/38 (`background_wait_smoke` §2b new, caught by a mutation
in a copy; `structural_baseline_smoke` Q5 updated). Details: [SP-196]'s log.

---

## 🟠 T-0626 — `[Cross]` EP-052 S2a: every scene save through the queue, Mac and Linux

**Epic:** [EP-052] (AC4, AC5, AC6, AC7) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-26) — ✅ Mac live pass ([SP-195] AC7); ⏳ the rig live pass ([SP-195] AC6) · **Sprint:** [SP-195] (🟡 Active) →
[`../Sprints/Sprint-SP-195.md`](../Sprints/Sprint-SP-195.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** on both platforms the autosave, app switch, undo and redo writes, Replace All, the drive-returned flush, quit, and the
resume stamp go through `scrivi_enqueue_save_scenes`; a background waiter applies each outcome on the main thread (written →
clean; failed → dirty). The platforms' `noteScenePersisted` save-path calls and W6's ordering code are removed. Replace All
and its undo stop locking the manuscript; Linux Replace All shows a real count ([SP-185] Q3). Close and quit wait for the
queue and ask when it is held ([T-0570] on the Mac; on Linux, [I-0322] folded in per [SP-195] Q3: the Mac's banner and question).

**Done (2026-10-26):** Mac (`ViewportSceneLoader.swift` `enqueueSaves` / `startWaiting` / `apply`) and Linux (`EditorShell.cpp`
`enqueueSaves` / `startWaiting` / `applyWriteOutcomes`): every scene save queued, outcomes on the main thread, W6 removed,
Replace All and undo never lock the manuscript, a close and quit wait for the queue and ask when it is held (Linux [I-0322]:
the banner and the question), Linux Replace All counts *"Replacing N of M"*. **Evidence:** interop 319/319; smokes 38/38;
main-thread file operations for the saves 0 on both (Mac autosave / ⌘Z / ⇧⌘Z 5 → 0; Linux 6 / 8 / 7 → 0); ctest Mac 781,
Linux 784. Details: [SP-195]'s log.

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

