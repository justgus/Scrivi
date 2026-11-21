---
sprint: SP-232
epic: EP-059
status: Active
platform: Linux
created: 2026-11-21
---

# SP-232 — `[Linux]` [EP-059] F: the project window's toolbar

**Status:** 🟡 **ACTIVE** (user, 2026-11-21: *"Q1-Q5 as recommended, implement SP-232"*); ✅ Q1–Q5 as recommended. Built 2026-11-21: AC1–AC3 met; AC4 green locally (CI on the push). AC5: steps 1–12 pass on the VM (user, 2026-11-21: *"The only think I'd change is that, in the absence of an icon for the >> button (Step 10), use the actual text ">>". It has no text below it, so without an icon it becomes invisible. Other than that all 12 steps passed."*); ⏳ step 10 again, on the » fix. Planned 2026-11-21 (user: *"Close SP-231 and plan SP-232"*).
Created 2026-11-21 (user, 2026-11-21: *"I'd like to scan the Epic for any tasks that involve creating a menu bar of buttons across
the top like macOS and iOS have. If missing, I'd like to make sure its added to the Epic as a new Sprint and AC."*).
**Carries:** [T-0694].
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — part **F** (F1), new 2026-11-21; after [SP-231] (✅ closed),
whose Show World Warnings the toolbar's View item carries.
**Size:** S–M — every verb exists; the work is the surface, its names and its icons.

---

## What the scan found (2026-11-21)

- **EP-059 had no toolbar AC**, and the gap review ([`../Scrivi_Linux_Capability_Gap_Review_v0_1.md`](../Scrivi_Linux_Capability_Gap_Review_v0_1.md))
  lists no toolbar gap, though it read the toolbar; only G8 names the toolbar's Navigate item.
- **[I-0244]** has recorded since 2026-09-22 that Linux has **no `QToolBar` at all**; it was ruled (2026-09-26) to become its own
  Epic after [EP-043], and none was filed. Re-counted 2026-11-21: still zero `QToolBar` / `addToolBar` in `platforms/linux/src/`.

## What reading the code found (2026-11-21)

**The Mac** (`ProjectToolbar.swift`; one shape on every Apple platform, ordered by use, user 2026-11-03 [SP-199]). Every item calls
the menu bar's own closure — *"the toolbar is a SURFACE for what the menu bar already does"* (EP-040 AC2):

| # | Item | Kind | What |
|---|------|------|------|
| 1 | Inspector | toggle | Show or hide the Scene Inspector |
| 2 | Timeline | toggle | Show or hide the Timeline |
| 3 | Project Settings | button | Project Settings… |
| 4 | Worlds | button | Manage Worlds… |
| 5 | Navigate ▾ | menu | Scene Start / End, Chapter Start / End, Manuscript Start / End |
| 6 | New ▾ | menu | New Scene, New Chapter |
| 7 | Merge ▾ | menu | Merge Scene with Previous, Merge Chapter with Previous |
| 8 | View ▾ | menu | World Warnings, Markup Hints, Buffers (toggles) |
| 9 | Import ▾ | menu | as File ▸ Import |
| 10 | Export ▾ | menu | as File ▸ Export, then Copy as HTML |
| 11 | Close Project | button | Close Project |
| 12 | Purge Stale History Branches… | button | (last; first into the overflow) |
| 13 | About Inkwyn | button | |

(iOS adds Inkwyn Settings, for want of a menu bar.) The Mac's Navigator button is not in this list: it comes with the split view, at
the window's leading edge. As the window narrows, the system moves items into a ⋯ overflow from the END.

**Linux** (`ScriviWindow::buildMenuBar`, `ScriviWindow.cpp:440`): a `QMainWindow` with a menu bar and no toolbar. Every one of the 13
verbs is already a `QAction` there, enabled with the editor (`editorOnlyActions`), checks kept in step (`updateMenuState`, `:979`):
View ▸ Show Inspector / Timeline / World Warnings / Buffers / Markup Hints, Project ▸ Project Settings… / Manage Worlds… / Purge
Stale…, the six Go To items, File ▸ Import ▸ / Export ▸ / Copy as HTML / Close Project, Help ▸ About Inkwyn.
- ⚠️ **Two names differ from the Mac's:** Linux's Scene ▸ **Split Scene** and Chapter ▸ **Split Chapter** are the Mac's **New Scene**
  and **New Chapter** (same keys, Ctrl+Return / Ctrl+Shift+Return); Linux's **Merge Scene** / **Merge Chapter** are the Mac's **Merge
  Scene with Previous** / **Merge Chapter with Previous**. A toolbar of the menus' own actions would show the Linux names (Q3).
- **Icons:** Linux uses text glyphs, not theme icons, where it marks state (`WorldsDialog.cpp`: *"the rig is driven over RDP where
  icon themes vary, and a glyph that fails to load leaves NO signal at all"*; the Timeline's ◎ + −). The Docker image has no icon
  theme at all; the VM and the rig have Ubuntu's (Yaru) (Q2).
- `QToolBar` overflows by itself: items that do not fit go behind a » button at the end — the Mac's behaviour.
- The Landing window (`LandingWindow.cpp:26`) shares the menu bar builder with no project window; it gets no toolbar.

## Goal

Every Linux project window has a toolbar across the top with the Mac's items, in the Mac's order, each the menus' own action.

## Plan

1. A `QToolBar` on each project window, built in `buildMenuBar` beside the menus (`w != nullptr`): fixed (not movable, not floatable),
   no right-click hide menu; shown only while the window shows a project.
2. Items (Q1): the menus' own `QAction`s for the buttons and toggles; for Navigate, New, Merge, View, Import and Export a tool button
   with a drop-down menu holding the menus' own actions (Import and Export: the File menu's submenus themselves). So the enabled
   state, the checks and the shortcuts are the menus', with one code path, as the Mac.
3. Icons and labels (Q2); names (Q3); the Navigator (Q4); hiding (Q5).
4. **Tests:** a new `toolbar_smoke` (in CI): the toolbar is on a project window and not on Landing; the items in order; each item IS
   the menu's action (or its drop-down holds the menu's actions); disabled with no project; a toggle on the toolbar checks the menu
   item and changes the pane; a Go To item moves the caret; View ▸ Show Toolbar. Any smoke that finds a renamed menu item by its
   text, updated (Q3).
5. **Live pass** on the Linux VM: each item, the overflow at a narrow width, and the icons on Ubuntu's theme.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Which items.** | **The Mac's 13, in its order** (Inspector … About Inkwyn), each the menus' own action. Not iOS's Inkwyn Settings: Linux has Edit ▸ Settings… in its menu bar, as the Mac. | ✅ As recommended (2026-11-21) |
| **Q2** | **Icons and labels.** | **The desktop theme's standard icons with the label under each** (`Qt::ToolButtonTextUnderIcon`). Where a theme lacks an icon (or over RDP, or in Docker with none) the button still shows its word, so it never goes blank. Bundled icons would need artwork; text alone is wide. The Mac shows icons alone by default, but a Linux theme's icons are not the Mac's symbols. | ✅ As recommended (2026-11-21) |
| **Q3** | **The two names that differ.** | **Rename Linux's menu items to the Mac's**: Split Scene → **New Scene**, Split Chapter → **New Chapter**, Merge Scene → **Merge Scene with Previous**, Merge Chapter → **Merge Chapter with Previous** (keys unchanged). Linux adopts Apple's shape, and the toolbar and the menu then say the same thing. | ✅ As recommended (2026-11-21) |
| **Q4** | **A Navigator item.** | **Yes — first, at the left, a toggle for View ▸ Show Scene Navigator**, where the Mac's sidebar button sits. Linux's Navigator is a pane with no built-in button. | ✅ As recommended (2026-11-21) |
| **Q5** | **Hiding the toolbar.** | **View ▸ Show Toolbar, on by default, per window, not saved** — as Show Inspector and Show Timeline are on Linux. The Mac has View ▸ Hide Toolbar from the system. | ✅ As recommended (2026-11-21) |

## What was built (2026-11-21)

- **The toolbar** (`ScriviWindow::buildToolbar`, called from `buildMenuBar` for a project window): a fixed `QToolBar` across the top,
  no right-click menu, shown only while the window shows a project (`updateMenuState`). Fourteen items in the Mac's order with the
  Navigator first (Q1, Q4): Navigator · Inspector · Timeline · Project Settings · Worlds · Navigate ▾ · New ▾ · Merge ▾ · View ▾ ·
  Import ▾ · Export ▾ · Close Project · Purge Stale History Branches… · About Inkwyn.
- **The menus' own actions.** Each button IS its menu item's `QAction` (the toolbar sets only its short word, its tooltip — the Mac's
  help text — and an icon hidden in the menus); each drop-down holds the menus' actions (Import: File ▸ Import itself; Export: File ▸
  Export's items, then Copy as HTML). Enabled, checked and keyed with the menu, one code path.
- **Q2** — the theme's standard icon (the first of a few freedesktop names the theme has) with the word under it; no icon → the word
  alone. **Q3** — Scene ▸ **New Scene**, Chapter ▸ **New Chapter**, **Merge Scene with Previous**, **Merge Chapter with Previous** (keys
  unchanged). **Q5** — View ▸ **Show Toolbar** at the top of View, on by default, per window, not saved; greyed on the Landing window
  (one menu bar for both, [SP-216] Q2 — `settings_smoke` §7 holds the two menu bars equal, and caught it).
- A narrow window puts the items that do not fit behind Qt's » button, from the end.

**Tests (AC4, 2026-11-21):** `toolbar_smoke` (new, in CI): the fourteen items in order; each button the menu's own action and each
drop-down the menus' actions; the old names gone; the toolbar's Inspector toggles the pane and the menu's check; Navigate ▸
Manuscript Start moves the caret; the » button at 420 px; View ▸ Show Toolbar; none on Landing (its Show Toolbar greyed); Close
Project takes it with the project. Linux (Docker, Qt 6.4) ctest **884/884** as non-root, **46/46** CI smokes, every target built. No
core or Apple code changed. ⏳ CI on the push. (Docker has no icon theme: the icons are seen only in the live pass.)

⚠️ **Live-pass finding (2026-11-21):** with no icon from the theme the » overflow button had no word either, and was invisible on
the VM. ✅ Fixed: its text is ">>", drawn whenever its icon cannot be (`buildToolbar`, the `qt_toolbar_ext_button`); `toolbar_smoke`
§4 checks it is never blank. ⚠️ Docker's Fusion style always has the icon, so the text-only case is seen only on the VM.

## Live pass (AC5) — the Linux VM

With a project open:

1. A toolbar across the top of the window, under the menu bar: Navigator, Inspector, Timeline, Project Settings, Worlds, Navigate,
   New, Merge, View, Import, Export, Close Project, Purge Stale History Branches…, About Inkwyn — each a word under an icon (a
   button without an icon still shows its word).
2. Hover Inspector: a tooltip, "Show or hide the Scene Inspector".
3. Click Navigator, Inspector and Timeline in turn: each pane hides; click again, it returns. Open the View menu: its Show items
   are checked as the panes are.
4. Click Project Settings: the Project Settings dialog. Close it. Click Worlds: the Worlds dialog. Close it.
5. Navigate ▾ ▸ Go to Manuscript End, then Go to Manuscript Start: the caret moves there.
6. Open the Scene menu: New Scene and Merge Scene with Previous (were Split Scene, Merge Scene). The Chapter menu: New Chapter and
   Merge Chapter with Previous.
7. New ▾ ▸ New Scene with the caret mid-scene: the scene splits there. Then Merge ▾ ▸ Merge Scene with Previous: they rejoin.
8. View ▾: Show World Warnings, Show Markup Hints and Show Buffers, checked as in the View menu.
9. Import ▾ and Export ▾: the File menu's Import and Export items; Export ends with Copy as HTML. (Press Escape; nothing to import.)
10. Narrow the window: the last items go behind a button at the right end — its icon, or ">>" when the theme has none; click it to
    reach them. Widen it again. (✅ Re-check after the 2026-11-21 fix.)
11. View ▸ Show Toolbar: unchecked, the toolbar goes; checked, it returns.
12. Click Close Project: back to the start window, with no toolbar; its View ▸ Show Toolbar is greyed.

## Acceptance Criteria

- [x] AC1 — (F1) A toolbar across the top of every project window with the Mac's items in its order (Q1, Q4), each running the same
      action as its menu item and enabled, disabled and checked with it; drop-downs for Navigate, New, Merge, View, Import, Export.
- [x] AC2 — Icons and labels as Q2; the names as Q3, in the toolbar and the menus.
- [x] AC3 — Items that do not fit go behind the overflow button; View ▸ Show Toolbar hides and shows it (Q5); none on the Landing window.
- [ ] AC4 — Linux ctest (non-root, tests on), the smokes (the new one in CI), CI green.
- [~] AC5 — Live pass on the Linux VM: ✅ steps 1–9, 11, 12 (user, 2026-11-21: *"The only think I'd change is that, in the absence of an icon for the >> button (Step 10), use the actual text ">>". It has no text below it, so without an icon it becomes invisible. Other than that all 12 steps passed."*); ⏳ step 10 on the ">>" fix.

## Progress log

- **2026-11-21** — Live pass: every step passes but 10 (the overflow button invisible without a theme icon); fixed — ">>" as its text. ⏳ Step 10 again.
- **2026-11-21** — Built: AC1–AC3 met; AC4 green locally; T-0694 → 🟠.
- **2026-11-21** — 🟡 **ACTIVE** (user, 2026-11-21: *"Q1-Q5 as recommended, implement SP-232"*); Q1–Q5 as recommended; T-0694 → active.
- **2026-11-21** — Planned in full (user, 2026-11-21: *"Close SP-231 and plan SP-232"*): the Mac's 13 items read, every Linux verb
  found as a menu action, two naming differences found; Q1–Q5 to rule.
- **2026-11-21** — 🔵 **Created in Planning**, issued via `next-id.py`; carries [T-0694].

*Last Updated: 2026-11-21 (Live pass: step 10's finding fixed; ⏳ step 10 again, CI.)*

*Last Updated: 2026-11-21 (Built; ⏳ AC5 live pass, CI.)*

*Last Updated: 2026-11-21 (🟡 **ACTIVE**.)*

*Last Updated: 2026-11-21 (Planned in full; Q1–Q5 to rule.)*

*Last Updated: 2026-11-21 (🔵 **Created in Planning**.)*
