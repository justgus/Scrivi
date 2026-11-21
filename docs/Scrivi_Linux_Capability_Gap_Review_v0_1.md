# Scrivi — Linux Capability Gap Review v0.1

**Date:** 2026-11-20 · **Asked for by:** the user (*"conduct a review of the capabilities of the Linux implementation as compared to
the macOS Implementation … ignoring any differences we implemented because it was the right thing to do on Linux"*).
**Status:** a report. It changes no tracking document; what to file is the user's to rule.
⚠️ **Corrected 2026-11-20:** the first draft marked G1–G3 untracked. They are not: the backlog's three Linux port Epics —
[EP-035] Object Foundations, [EP-036] Object Detail Sheet & Media, [EP-037] Relationships & Sources (created 2026-08-25, never
activated) — already hold them.
**Method:** read from the code on 2026-11-20 (`main` after SP-227): the C ABI each platform calls, then each surface side by side —
menus, toolbar, Navigator, manuscript, Inspector, object detail, worlds, timeline, settings, history.

---

## 1. The core: at parity

Every `scrivi_*` endpoint Apple's app calls, Linux's calls too, with two exceptions that are right as they are:
- `scrivi_extract_searchable_scenes` — Spotlight, which Linux does not have.
- `scrivi_history_record_event` / `_record_barrier` / `_seed_scene` / `_undo` / `_redo` — Linux records and undoes through the
  core's capture session (`scrivi_capture_*`) instead. The same history, a different route; not a gap.

Linux calls four endpoints Apple does not (`scrivi_resolve_timeline_project_times`, `scrivi_set_timeline_epoch_offset`,
`scrivi_set_world_epoch_offset` — [T-0473], Apple's gap — and `scrivi_upsert_relation_type`, which has no screen on either platform,
[T-0472]).

**So every gap below is in the screens:** the core can do it; Linux shows no way to.

## 2. Gaps

### Worldbuilding — the largest

| # | Gap | Mac | Linux | Tracked |
|---|-----|-----|-------|---------|
| **G1** | **No object detail view.** Opening an object shows a message: *"The object detail view is not built yet — it is the next Epic's work."* Nothing about an object can be read or edited: name, subtitle, notes, tags, image, relations, sources. | `Detail/ObjectDetailSheet.swift` (double-click a card) | `EditorShell.cpp:4197–4204` (`QMessageBox::information`) | ✅ [EP-036] (Object Detail Sheet & Media, backlog) |
| **G2** | **Objects cannot be created, linked to a scene, or unlinked.** The Worldbuilding tab is a read-only list of the scene's objects. | `ObjectCard.swift:572` (Add existing…), `:607` (New), `:1136` (Remove from Scene) | `SceneInspector.cpp:205` (list + Open Object only) | ✅ [EP-035] AC4 (object CRUD); linking is [EP-037]'s relationship creation |
| **G3** | **No Sources card:** a scene cannot cite the project's sources. | `Inspector/SourcesCard.swift` | — | ✅ [EP-037] (Relationships & Sources) |
| **G4** | **The Inspector is not configurable:** fixed tabs, no per-scene Add Card / remove card; no sort or card reorder. | `InspectorCardStackView.swift:59,154` | `SceneInspector.cpp:86–88` | Sort/reorder: [T-0491] ([EP-035]); per-scene Add / Remove Card ❌ |
| **G5** | **No world creation**, and no **Reconnect Worlds** button. A Linux-only writer with no world cannot proceed. | `WorldsView.swift` (Create…, Reconnect Worlds) | `WorldsDialog.cpp:170` (Add Existing only) | Create: [T-0497] ([EP-035]); Reconnect ❌ |
| **G6** | **No world-warning strip** (View ▸ Show World Warnings): when a world goes away, the project-wide notice and its Manage Worlds… link are missing; only each object in the Inspector says so. The same strip is where Apple reports an imported timeline that failed to read; Linux only logs it. | `WorldWarningView.swift`, `EditorView.swift:436` | `EditorShell.cpp:4526` (*"⛔ The Linux surface is a KNOWN GAP"*) | ❌ (noted in code, from [I-0214]) |

### Manuscript and navigation

| # | Gap | Mac | Linux | Tracked |
|---|-----|-----|-------|---------|
| **G7** | **Untitled scenes read "Untitled Scene"**, where the Mac shows the scene's first line as the manuscript draws it, or "Scene N" when the scene is empty. | `SceneNavigatorView.swift:771–782`, `ManuscriptPresenter.presentedFirstLine` ([I-0327]) | `EditorShell.cpp:2063` (the timeline's dots already say "Scene N", `:4003`, `:4263`, so Linux disagrees with itself too) | ❌ ([I-0327] names it as untracked) |
| **G8** | **No Go To:** Scene Start/End, Chapter Start/End, Manuscript Start/End (menus, keys, the toolbar's Navigate). | `ScriviApp.swift` Scene / Chapter / Project menus; `ProjectToolbar.swift` Navigate | — | Manuscript start/end only: [T-0249] |
| **G9** | **No Navigator search.** | `SceneNavigatorView.swift` (T-0569) | — | ❌ (left out of [SP-226] by scope, not ruled right for Linux) |

## 3. At parity (checked, no gap)

File ▸ New / Open / Close, Import and Export (every format), Copy as HTML · Find, Find and Replace, Replace All · Format · the copy
buffers and the Buffers palette · undo and redo, the branch chooser and branch marks · the History section, Purge Stale History
Branches · Project Settings (every field) · pen names, authors, "I write as" · worlds: Add Existing, Locate…, Authors…, Remove ·
the timeline: import / export, epoch offset, historical events, acts and story structure, Set Time Delta, aggregates, zoom ·
the Navigator: rename, delete, drag scenes and chapters, collapsible chapters · scene properties · several windows and session
restore.

**Not counted as gaps:** the Mac timeline's Set Story Time…, Copy / Paste Story-Time Position and Set Epoch Label… are empty
placeholders on the Mac too (`TimelineStripView.swift:1283, 2953, 2958, 3091`). Comments, the inbox, snapshots, orphaned objects
and repair have no screen on either platform. Spotlight and deep links are Apple's by nature.

## 4. Sizes, roughly

- **G1–G4 are one piece of work:** Linux's object surface (detail view, create / link / unlink, sources, the card stack). It is
  the Worldbuilding half of the app, about an Epic.
- **G5–G6:** a Sprint (create follows Apple's shape; the strip is one passive widget).
- **G7–G9:** small, one Sprint between them. G7 is the core of the ask: the Mac's rule lives in Swift
  (`presentedFirstLine`), so Linux either re-implements it — the presenter's rules already exist in C++ on Linux — or the core
  takes it on for both.

**Filed 2026-11-20:** every gap is tracked in **[EP-059]** (user, 2026-11-20), which supersedes EP-035/036/037: G1 → B1 · G2 → A5 · G3 → C4 · G4 → A7, B5, B6 · G5 → D1, D2 · G6 → D3 · G7 → E1 · G8 → E2 · G9 → E3.

*Last Updated: 2026-11-20 (filed as EP-059.)*

*Last Updated: 2026-11-20 (v0.1 — written from the code.)*
