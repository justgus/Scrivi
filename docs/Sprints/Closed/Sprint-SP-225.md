---
sprint: SP-225
epic: EP-055
status: Closed
closed: 2026-11-19
platform: Apple
created: 2026-11-19
---

# SP-225 — `[Apple]` [EP-055] S5: worlds and the timeline's dots on iPad and iPhone

**Status:** ✅ **CLOSED** 2026-11-19 (user, 2026-11-19: *"all steps pass, close SP-225.  In addition the hover worked for both the pointer and the pencil."*). Activated (user, 2026-11-19: *"Q1-Q3 as recommended, activate and implement SP-225"*) · planned 2026-11-19 (user: *"plan SP-225"*) · ✅ Q1–Q3 ruled as recommended.
**Carries:** [T-0688] — ✅ Verified 2026-11-19 → [`../../Tasks/Verified/Task-verified-0688.md`](../../Tasks/Verified/Task-verified-0688.md).
**Epic:** [EP-055] → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md) — AC6 (part): world Create / Add Existing / Relink, the timeline's
touch gestures, pointer and Pencil hover. [T-0657] and what this Sprint's iPad pass finds are S6.
**Builds on:** [SP-224] (world bookmarks record and activate on iOS), SP-199 (the iOS timeline: pan by drag, pinch), T-0173 / T-0174
(dot select, aggregate popover, drag-to-join).
**Size:** M — three world actions on an existing pattern; the timeline half starts with a reproduction, not a known fix.

---

## What reading the code found (2026-11-19)

**Worlds.**
- ⚠️ All three actions refuse on iOS: `createWorld`, `relinkWorld`, `bindExistingWorld` set *"… is not available on this platform
  yet."* in their `#else` (`WorldsView.swift:291–296`, `:365–369`, `:405–409`). Each Mac branch is a panel, then one engine call,
  then `WorldBookmarkStore.record`, then `onWorldsChanged()` + `load()`. Only the panel is AppKit.
- ✅ **The iOS pattern already exists.** New Project picks a **folder** with `.fileImporter(allowedContentTypes: [.folder])` and creates
  `<folder>/<slug>.scrivi` inside it under that folder's scope (`NewProjectSheet.swift:106`, `AppEnvironment.createProjectInPickedDirectory`,
  `:811`). Opening a project picks the **package** by its type (`LandingView.swift:114`).
- ✅ Since [SP-224] `WorldBookmarkStore.record` / `activate` work on iOS, so a world picked or created here survives a relaunch.
- ⚠️ A picker's URL is readable only between `startAccessingSecurityScopedResource` and `stop…`. The Mac panel's grant lasts the session;
  on iOS the world must stay readable after the picker returns, so the picked scope is handed to the store's held scope
  (`WorldBookmarkStore.activate`) and the picker's own released.

**Timeline.**
- The single scene dot already has, in shared code, a tap to select (`.onTapGesture { onSelect() }`) and a drag to move or join
  (`combinedDragGesture`, `minimumDistance: 4`) (`TimelineStripView.swift:2909–2975`). The user reports both do nothing by touch
  (2026-11-19). ⚠️ **Why is not known yet.** The suspects are the strip's simultaneous `MagnifyGesture` (`:1390`), the background's
  iOS pan drag (`:1010`) and the dot's own `DragGesture(minimumDistance: 0)` (`:2911`). ⛔ No fix is chosen until it is reproduced on the
  simulator.
- ⚠️ **The aggregate dot opens its member popover only on hover** (`AggregateDotView`, `.onHover` → `onHover()`, `:2387`). A finger
  never hovers, so on touch it cannot open. The members inside the popover select by tap (`:2609`), which touch can reach once it is
  open. The imported rows' aggregates work the same way (`:1225`).
- **Hover on iPad.** Every hover here is SwiftUI's `.onHover` / `.onContinuousHover`, with no AppKit underneath except the Mac's
  scroll capture (`:1405`, `#if os(macOS)`). ⏳ Whether iPadOS drives these from a trackpad / mouse pointer and from Apple Pencil hover
  is to be confirmed against Apple's documentation (the `.json` route) before anything is added.

## Goal

On an iPad or iPhone a writer can **create a world, add an existing one, and relink a moved one** from the worlds sheet, and on the
timeline can **tap a dot to go to its scene, tap an aggregate to list and pick its scenes, and drag a dot to a new time**. Where the
iPad has a pointer or a Pencil that hovers, the Mac's hover behaviour appears too. Nothing changes on the Mac except Q2.

## Plan

1. **World Create (Q1):** pick a folder (`.fileImporter`, `[.folder]`, as New Project does); create `<folder>/<name>.scrivworld` under
   the folder's scope; record the world's bookmark; activate it so it stays readable; release the folder's scope.
2. **Add Existing / Relink:** pick the package (`.fileImporter`, the `.scrivworld` type, as Open Project does); start its scope; the
   same engine call as the Mac; record and activate the bookmark; release the picker's scope. A refused package (wrong `worldID`)
   records nothing, as now.
3. The three Mac branches keep their panels. The shared tail (record → refresh) moves to one helper only if the iOS code would
   otherwise repeat it three times.
4. **Timeline, reproduce first:** on the simulator, tap and drag a single dot; find which gesture takes the touch. Fix that, and only
   that.
5. **Aggregate by tap (Q2).**
6. **Hover (Q3):** confirm from Apple's docs; add only what the docs say is missing.
7. **Checks:** iOS tests on the simulator: a world created in a picked folder records a bookmark that activates; Mac interop green
   (`scripts/run-interop-tests.sh`, N > 0); macOS and iOS builds; CI after the push.
8. **Live passes (user):** iPad (touch, then a trackpad or mouse pointer, then the Pencil if it hovers), iPhone, the Mac re-checked.
   The relaunch steps kill the app first (Xcode, or the device).

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where Create World puts the package on iOS.** iOS has no save panel for a folder package. | **As New Project:** the writer picks a folder in Files, and the world is created there as `<name>.scrivworld`. One extra tap against the Mac, the same pattern the writer already knows. | ✅ As recommended (2026-11-19) |
| **Q2** | **Aggregate dots by tap.** | **Tap opens the member popover on every platform**, the Mac included; the Mac's hover stays. One code path; a Mac click on an aggregate does nothing today. | ✅ As recommended (2026-11-19) |
| **Q3** | **Pointer and Pencil hover.** | **System hover only:** whatever SwiftUI's `.onHover` / `.onContinuousHover` deliver on iPadOS, nothing Pencil- or pointer-specific (the Epic's Q8 spirit). If the docs show they already fire for both, this AC is met by the live pass alone. | ✅ As recommended (2026-11-19) |

⛔ **NOT in this Sprint:** [T-0657] collapsible Navigator chapters (S6); the worlds sheet's compact-width layout, and how the
aggregate popover presents on an iPhone (S7, AC8); I-0223 world versions.

## Acceptance Criteria

- [x] AC1 — ✅ built 2026-11-19 (✅ AC7) — On iOS, Create World makes a `.scrivworld` in a folder the writer picks (Q1); it is bound, readable, and still readable after
      the app is killed and relaunched
- [x] AC2 — ✅ built 2026-11-19 (✅ AC7) — On iOS, Add Existing World binds a picked `.scrivworld`, and Relink re-points a moved one; a package with the wrong `worldID`
      is refused with nothing recorded; both survive a relaunch
- [x] AC3 — ✅ built 2026-11-19 (✅ AC7: the inferred cause held) — On iPad and iPhone, tapping a scene dot goes to its scene, and dragging it moves it in story time (the Time Delta Picker,
      or a join onto an aggregate), as on the Mac
- [x] AC4 — ✅ built 2026-11-19 (✅ AC7) — Tapping an aggregate dot lists its scenes, and tapping one goes to it (Q2); the imported rows' aggregates too
- [x] AC5 — ✅ nothing to build, per Apple's docs; ✅ pointer and Pencil hover both confirmed (user) — On iPad with a pointer or a hovering Pencil, the Mac's hover behaviour appears: dots grow, aggregates open (Q3)
- [x] AC6 — ✅ tests and builds 2026-11-19 (⏳ CI runs on the user's push) — iOS tests on the simulator for AC1–AC2's bookmark path; Mac interop green; macOS and iOS builds; CI green
- [x] AC7 — ✅ 2026-11-19 — **Live passes (user):** iPad, iPhone, the Mac re-checked

## Progress log

- **2026-11-19** — 🔵 **Created in Planning** (user, 2026-11-19: *"plan SP-225"*), issued via `next-id.py`; [T-0688] filed. Q1–Q3 to rule.
- **2026-11-19** — ✅ Q1–Q3 ruled as recommended. 🟡 **Activated** (user, 2026-11-19: *"Q1-Q3 as recommended, activate and implement SP-225"*); [T-0688] → active.
- **2026-11-19** — **Built.** [T-0688] → 🟠.
  - **Worlds (Q1):** each action splits into picking (the Mac's panel, unchanged; on iOS `.fileImporter`) and applying (the engine
    call, the bookmark, the refresh — one function both use). On iOS `handlePicked` holds the picker's scope while the action runs:
    Create makes `<folder>/<name>.scrivworld`; Add Existing and Relink take the package. Relink now forgets the old grant before
    recording the new one, or `activate` would keep reporting the old path as held (both platforms).
  - ⚠️ **Found while building, an [SP-224] gap:** `activateWorlds` / `activateWorldsOffMain` were still `#if os(macOS)`, so on iOS no
    world bookmark was ever activated at open or on reconnect. Removed; without it AC1–AC2 could not survive a relaunch. (SP-224's
    imported world passed because it sits inside the project's folder scope.)
  - **Timeline, AC3:** ⛔ **not reproduced** — the simulator cannot be given a touch from here (no `idb`, no UI-test target), so
    the plan's step 4 was replaced by reading. The scene dot's tap and drag are intact; its hit circle is **19 pt** (`dotRadius` 3.5
    + 12), against Apple's 44 pt minimum, so a finger mostly lands on the strip behind it (pan, context menu). ✅ On iOS the dot now
    claims 44 pt (`kMinTouchTarget`); the drawn dot is unchanged. ⚠️ If the live pass still fails, the cause is elsewhere and this
    is re-opened.
  - **Aggregates (Q2):** both aggregate views open on tap as well as hover (`onHover` renamed `onOpen`); their hit frames are at least
    44 pt on iOS, and since `hitFrame` is the bucket pitch's single source (I-0210) the spacing follows. ⚠️ **One addition beyond
    the ruling:** on iOS the popover is a **list of names**, not the Mac's ring — the ring's 7 pt dots are 15 pt apart and named
    only on hover, so a finger could neither hit nor identify one (the user's own words: *"show and select the listed scenes"*).
    Scenes select; historical and imported events are read-only, as on the Mac.
  - **Hover (Q3):** Apple's docs: `onHover` (iPadOS 13.4+) and `onContinuousHover` (17+) are pointer hover, and the hover
    recognizer under them reacts to *"a pointer from a pointing device such as Apple Pencil"* (UIKit, *"Adopting hover support for
    Apple Pencil"*, fetched 2026-11-19). Nothing added.
  - **AC6:** iOS 35 passed, 0 failed (1 new: `worldCreatedInFolder` — a world created inside a folder binds `available`, records its
    bookmark, and relinks after a move). ⚠️ It checks the engine and the store on iOS, not the view; the view is the live pass.
    Mac interop 352 passed, 0 failed; macOS and iOS builds ✅; the six CI guard scripts ✅ locally. ⏳ CI after the push.

- **2026-11-19** — ✅ **Live passes (user): all steps pass** (iPad, iPhone, the Mac); *"the hover worked for both the pointer and the
  pencil"* — Q3's reading of Apple's docs confirmed on the device. The 44 pt target fixed tap and drag. AC7 met; [T-0688] → ✅ Verified
  and archived. ✅ **CLOSED** (user, 2026-11-19: *"all steps pass, close SP-225.  In addition the hover worked for both the pointer and the pencil."*).

*Last Updated: 2026-11-19 (**✅ CLOSED** (user, 2026-11-19: *"all steps pass, close SP-225.  In addition the hover worked for both the pointer and the pencil."*).)*

*Last Updated: 2026-11-19 (🟡 **Activated and built** (user, 2026-11-19: *"Q1-Q3 as recommended, activate and implement SP-225"*).)*

*Last Updated: 2026-11-19 (🔵 **Created in Planning** (user, 2026-11-19: *"plan SP-225"*).)*
