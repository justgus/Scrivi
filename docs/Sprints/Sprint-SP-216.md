---
sprint: SP-216
epic: EP-054
status: Planning
platform: Linux
created: 2026-11-13
---

# SP-216 — `[Linux]` [EP-054] The start window gets the project window's menu bar

**Status:** 🔵 **Planning** 2026-11-13 (user, 2026-11-13: *"Close SP-214 and plan the import/export and Linux start window
sprints"*). The shape is ruled (below); ✅ Q1–Q3 ruled (2026-11-13; Q1: EP-054).
**Carries:** [T-0677] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-054] (Q1) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Follows:** [SP-215] — this mirrors its finished File menu, so it activates after SP-215 closes.
**Size:** S–M — one refactor (the menu bar built once, used by two windows), no new command.

---

## Ruled before planning (user, 2026-11-13)

- **Mirror the project window's menu bar on the start window, with the project-only items greyed out** — Apple's shape: the
  Mac has one menu bar, and with only the Welcome window open its project items are disabled
  (`ScriviApp.swift`, `.disabled(focusedSession == nil)`). (User: *"Linux's start window should eventually have a menu bar, so
  we should create one that mirrors the Project menu bar, only with all the project-only items greyed out"*; *"I agree with
  your recommendation that in this case we should mirror to menu items in both windows."*)
- On Linux each window carries its own menu bar, so a different one would not be wrong in itself (user: *"having them be
  different is no issue. Just saying."*); mirroring is chosen so the writer never hunts, and so Linux keeps closer to the Mac.
- **Its own Sprint, after the Import / Export menus** ([SP-215]) — *"it also allows the Linux implementation to more closely
  mirror the Mac implementation."*
- **Later, not now:** when the start window becomes a listing of the manuscripts in use, some greyed items may act on the
  selected row (e.g. Export ▸ Archive… of the highlighted project) — decided then.

## What reading the code found (2026-11-13)

- **The start window is `LandingWindow`** (`LandingWindow.hpp:43`, built in `main.cpp:149`): a `QMainWindow` hosting
  `Landing.qml`, with **no menu bar**. Its one action is Settings… by its shortcut, added after [SP-211]'s live pass found
  Ctrl+, dead there (`LandingWindow.cpp:26`).
- **The menu bar is built inline in `ScriviWindow::buildMenuBar`** (`ScriviWindow.cpp:233`, ~400 lines: File, Edit, View,
  Scene, Chapter, Project, Help), each action wired to that window's editor or shell.
- ✅ **The "no project" state already exists:** project-only actions are collected in `editorOnlyActions_` and
  `updateMenuState(false)` greys them (`ScriviWindow.cpp:649`) — left from the single-window app, whose landing page was
  page 0 of a `ScriviWindow`. So which items stay live is already decided once, in one place.
- New Project… and Open Project… already raise the start window (`env_->showLanding()`, `ScriviWindow.cpp:254–281`);
  Import Archive… goes through `ShellController::importArchive` (`ScriviWindow.cpp:91`), which `Landing.qml:296` also calls.

## Goal

On Linux, the start window has the same menus, items and order as a project window, with every project-only item greyed out —
so whatever a writer can do with no project open, they find where they find it on the Mac, and no command has to be wired to
the start window by hand again.

## Plan

1. **One builder** (Q2): the menu bar built by one function for both windows, given the window it belongs to and whether a
   project can be open there; the start window builds it and greys `editorOnlyActions_` for good.
2. **The start window's live items** (Q3): New Project…, Open Project…, Import ▸ Archive…, Settings…, Quit; Help ▸ About
   Scrivi, Show Log Folder — each wired so it works from there.
3. **Retire the hand-wired Settings… action** (`LandingWindow.cpp:26`): the menu now carries it, with its Ctrl+,.
4. **Checks:** a smoke builds a `LandingWindow` and a project window and compares their menu bars item by item (titles, order,
   separators, submenus), then checks the start window's enabled set and that Ctrl+, and Ctrl+Q reach it; ctest green.
5. **Live pass (user)** on scrivi-vm: the start window's menus beside a project window's; each live item used from the start
   window.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Epic.** The active Epics are EP-054 (Import & Export), EP-055 (iPad and iPhone) and EP-057 (holding Linux rig passes); none is "Linux shell parity". | **None** — a standalone Sprint, as [T-0656] (the landing's background) is a standalone Task. | ✅ **RULED: EP-054** (user, 2026-11-13: *"For SP-216, Q1: add it to EP-054.  Q2: as recommended.  Q3: same as the Mac (as recommended)."*) — not as recommended. |
| **Q2** | **How the bar is shared.** (a) Extract `buildMenuBar` into one builder both windows call; the start window passes no editor. (b) Build the start window's bar by copying the project window's `QMenu`s. (c) Write the start window's bar by hand. | **(a)** — the only one where the two bars cannot drift; (b) copies wiring that points at another window, (c) is a second list to keep in step (the defect the derived-lists rule exists for). | ✅ **RULED as recommended** (user, 2026-11-13: *"For SP-216, Q1: add it to EP-054.  Q2: as recommended.  Q3: same as the Mac (as recommended)."*) |
| **Q3** | **What stays live on the start window.** Mirror the Mac with no project open: New Project…, Open Project…, Import ▸ Archive…, Settings…, Quit, About Scrivi, Show Log Folder; everything else greyed — Close Project, Copy as HTML, Import ▸ Timeline…, all of Export, Edit's editing commands, View, Scene, Chapter and Project. | As stated. | ✅ **RULED as recommended** (user, 2026-11-13: *"For SP-216, Q1: add it to EP-054.  Q2: as recommended.  Q3: same as the Mac (as recommended)."*) |

⛔ **NOT in this Sprint:** the manuscript listing the start window will become, any new command, keyboard-shortcut changes,
retiring the single-window path's page 0 ([SP-147]'s, if still pending).

## Acceptance Criteria

- [ ] AC1 — The start window has a menu bar with the same menus, items, submenus and order as a project window, built by one
      builder (Q2)
- [ ] AC2 — On the start window only Q3's items are enabled, and each works from there (Settings… by Ctrl+, too; Quit by
      Ctrl+Q); the hand-wired Settings… action is gone
- [ ] AC3 — A smoke compares the two windows' menu bars item by item and checks the start window's enabled set; ctest green
- [ ] AC4 — **Live pass (user)** on scrivi-vm: the two windows' menus side by side; each live item used from the start window

## Progress log

- **2026-11-13** — 🔵 **Created in Planning** (user, 2026-11-13: *"Close SP-214 and plan the import/export and Linux start
  window sprints"*), issued via `next-id.py`; [T-0677] filed. The mirror and the order after [SP-215] ruled in chat; Q1–Q3 to
  rule.
- **2026-11-13** — ✅ **Q1–Q3 ruled** (user, 2026-11-13: *"For SP-216, Q1: add it to EP-054.  Q2: as recommended.  Q3: same as the Mac (as recommended)."*). Q1: under EP-054 (the File menu it mirrors is EP-054's).
