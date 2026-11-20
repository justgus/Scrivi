---
sprint: SP-227
epic: EP-055
status: Active
platform: Apple
created: 2026-11-20
---

# SP-227 — `[Apple]` [EP-055] S7: the whole app at iPhone width

**Status:** 🟡 **Active** (user, 2026-11-20: *"implement SP-227"*). ✅ Q1–Q4 ruled as recommended (user, 2026-11-20: *"Q1-A4 as recommended.  Tick AC6."*).
**Carries:** [T-0689].
**Epic:** [EP-055] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — **AC8** (the phone surface, per Q1: the whole app at
compact width, the worlds sheet reformatted for it) and the iPhone and Duo half of **AC10**.
**Builds on:** SP-199 (panels by size class: the Navigator and Inspector as pages, the toolbar in the system's ⋯ at compact width),
[SP-222] / [SP-223] (the edit menu as the keyboard-less path for edits, Find, Format and buffers), [SP-225] (worlds on iOS).
**Size:** M — sizes and placements in existing views; no new surface.

---

## What reading the code found (2026-11-20)

An iPhone 18 Pro is about 402 pt wide in portrait. A sheet on the phone fills the width, so any fixed or minimum width above
that pushes the content past the screen's edges.

**Too wide for a phone (iOS, not inside `#if os(macOS)`):**
- ⚠️ **The worlds sheet:** `.frame(width: 460)` (`WorldsView.swift:127`). Each world's row is one `HStack` of name and path, status,
  Locate…, Authors… and Remove (`:171–212`); the bottom bar is Add Existing World…, Reconnect Worlds and Done in one `HStack`.
- ⚠️ **The object detail sheet:** `.frame(minWidth: 520, …)` on the sheet (`EditorView.swift:652`) and `minWidth: 420` inside it
  (`Detail/ObjectDetailSheet.swift:346`).
- ⚠️ **New Project:** `.frame(minWidth: 420)` (`NewProjectSheet.swift:101`), reached from the Landing view.
- ✅ Already Mac-only: Project Settings (`ProjectSettingsSheet.swift:112`), Stale Branches (`StaleBranchesSheet.swift:72`), the
  authorship sheets (`AuthorshipViews.swift:23`, `:308`), the editor window (`EditorView.swift:299`). Under 402 pt and fine: the author
  prompt (380, `AuthorshipViews.swift:341`), the epoch offset sheet (380, `TimelineStripView.swift:3210`), the popovers (≤ 340, which
  become sheets at compact width anyway).

**Reachable only from the iPad's menu bar, so not on an iPhone without a keyboard:**
- **Import ▸ Manuscript…, Scrivener Project…, Archive…** for the open project (`ProjectArchiveFlow.swift:38–44`). The toolbar has
  Export only (`ProjectToolbar.swift:126`). Import ▸ Timeline… is on the timeline panel.
- **Show Markup Hints** (⇧⌘H, View; `ScriviApp.swift`, the iOS `CommandGroup(after: .sidebar)`).
- **Purge Stale History Branches…** (Project).
- **About Scrivi.**
- ✅ Everything else already has a touch path: the toolbar (Inspector, Timeline, Project Settings, Worlds, Navigate, New, Merge,
  View, Export, Close, Scrivi Settings), the text's edit menu (structural edits, Find, Format, buffers), the chapter header's menu
  (Expand / Collapse All), UIKit's undo gestures.

## Goal

On an iPhone, and on the Duo folded or open, every sheet fits the screen and every command the iPad offers can be reached by
touch. Nothing changes on the Mac.

## Plan

1. **The worlds sheet (Q1).**
2. **The detail sheet and New Project:** their fixed minimum widths become Mac-only, as Project Settings' already are.
3. **The menu-bar-only commands (Q2).**
4. **A width check (Q3):** an iOS test that lays out each sheet at 402 pt and fails if it asks for more.
5. **Checks:** iOS tests; Mac interop (`scripts/run-interop-tests.sh`, N > 0); macOS and iOS builds; CI after the push.
6. **Live passes (user, Q4):** the real iPhone and the Duo, portrait and landscape; the Mac and iPad re-checked for the worlds sheet.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **The worlds sheet at phone width.** | **Let each row and the bottom bar re-flow when they do not fit:** SwiftUI's `ViewThatFits` keeps today's single line where it fits (the Mac, the iPad) and stacks the actions under the world's name where it does not; the fixed 460 pt becomes Mac-only. The Mac's sheet is unchanged. *The alternative, a separate iOS sheet in a `Form` with a Done button (the shape of `WorldAuthorsSheet`), is a second view to keep in step with the first.* | ✅ As recommended (2026-11-20) |
| **Q2** | **Where the menu-bar-only commands go on the phone.** | **The toolbar, in its existing menus, on every platform (one shape, ruled 2026-11-03):** an **Import** menu beside Export (Manuscript…, Scrivener Project…, Archive…); **Show Markup Hints** in the View menu; **Purge Stale History Branches…** and **About Scrivi** at the end, the first into the ⋯ overflow. The iPad's menu bar keeps all four. | ✅ As recommended (2026-11-20) |
| **Q3** | **How the widths are proved.** | **One iOS test** that sizes each sheet at 402 pt wide and fails if it reports more: it catches a fixed width added later, which a live pass would find only on the sheet someone happened to open. | ✅ As recommended (2026-11-20) |
| **Q4** | **Devices for the live pass.** | **The real iPhone and the Duo (Remus)**, portrait and landscape, the Duo folded and open; the simulator only for the tests. AC10's Duo pass is met by this one. | ✅ As recommended (2026-11-20) |

## Acceptance Criteria

- [x] AC1 — The worlds sheet fits an iPhone in portrait, every action reachable (Q1); the Mac's and iPad's unchanged
- [x] AC2 — The object detail sheet and New Project fit an iPhone in portrait
- [x] AC3 — Import, Show Markup Hints, Purge Stale History Branches and About reachable on the iPhone by touch (Q2)
- [x] AC4 — The width test (Q3) passes for every sheet the iPhone can show
- [ ] AC5 — iOS tests, Mac interop, macOS and iOS builds, CI green
- [ ] AC6 — **Live passes (user, Q4):** the iPhone and the Duo; the Mac and iPad re-checked

## Live-pass steps (AC6)

**A. The worlds sheet on the iPhone, portrait**
1. Open a project that has at least one world. Tap ⋯ ▸ Worlds. The sheet fits the screen: nothing is cut off at either side.
2. Each world shows its Authors… (or Locate…) button and the red remove button, under the world's name when they do not fit beside it.
3. Add Existing World…, Reconnect Worlds and Done are all visible at the bottom.

**B. Landscape**
4. Turn the iPhone to landscape with the worlds sheet open. The sheet scrolls, and Done can be reached.

**C. The other sheets**
5. In the Inspector, double-tap an object (a character, or a source on the Sources card). Its detail sheet fits the screen. Close it.
6. Close the project. On the Landing, tap New Project. It fits the screen. Cancel it.

**D. The toolbar's new items (iPhone, in ⋯)**
7. Open a project. ⋯ ▸ Import lists Manuscript…, Scrivener Project…, Timeline… and Archive…. Tap Timeline…; the Files picker opens. Cancel it.
8. ⋯ ▸ View ▸ Markup Hints. Turning it off hides the markup hints in the manuscript; turning it on shows them again.
9. ⋯ ▸ Purge Stale History Branches… opens the Stale Branches sheet. Close it.
10. ⋯ ▸ About Scrivi opens the About sheet. Close it.

**E. The Duo**
11. Repeat steps 1–4 on the Duo, folded, then open.

**F. Mac and iPad unchanged**
12. Mac: Worlds sheet — one line per world, the three buttons on one line at the bottom, as before.
13. Mac: the toolbar's Import, View ▸ Markup Hints, Purge Stale History Branches… and About Scrivi work (About opens the About window).
14. iPad: Worlds sheet — one line per world, as before.

## Progress log

- **2026-11-20** — 🔵 **Created in Planning** (user, 2026-11-20: *"plan SP-227"*), issued via `next-id.py`; carries [T-0689]. Q1–Q4 to
  rule.
- **2026-11-20** — ✅ **Q1–Q4 ruled as recommended** (user, 2026-11-20: *"Q1-A4 as recommended.  Tick AC6."*).
- **2026-11-20** — 🟡 **Activated and 🔨 Built** (user, 2026-11-20: *"implement SP-227"*). [T-0689] → 🟠 `Task-unverified.md`.
  - **Worlds (AC1, Q1):** each row and the bottom bar are a `ViewThatFits`: one line where it fits, the actions under the name
    (row) or stacked beside Done (bar) where it does not. The name and path have a fixed ideal width (160 pt), so a long path, which
    truncates, never decides the layout. `.frame(width: 460)` is Mac-only; on iOS the sheet scrolls (landscape).
  - **AC2:** the detail sheet's 520 / 420 pt and New Project's 420 pt minimum widths are Mac-only.
  - **AC3 (Q2):** the toolbar gains Import (`ImportMenuItems`, now shared with File ▸ Import), Markup Hints in View, Purge Stale
    History Branches… and About Scrivi (the Mac's About panel; on iOS `env.showAbout`, which the iPad menu bar now uses too).
  - **AC4 (Q3):** `PhoneWidthSheetTests` — Worlds (with an unavailable world, the widest row), object detail (a source), New
    Project, Project Settings, Stale Branches, World Authors, Scrivi Settings, About, Epoch Offset, Historical Event, each offered
    402 pt; plus a control proving the measure sees a too-wide view inside a `ScrollView`. Half a point of slack for the 3× pixel
    grid (Worlds reports 402.33). `HistoricalEventEditorSheet` is internal now, for the test.
  - **AC5:** iOS **38/38** (2 new); Mac interop **352/352**; macOS and iOS builds ✅, no warnings in the touched files. ⏳ CI on the push.
  - ⏳ **AC6:** the live passes.

*Last Updated: 2026-11-20 (🔨 **Built**; ⏳ CI and the live passes.)*

*Last Updated: 2026-11-20 (✅ Q1–Q4 ruled as recommended.)*

*Last Updated: 2026-11-20 (🔵 **Created in Planning** (user, 2026-11-20: *"plan SP-227"*).)*
