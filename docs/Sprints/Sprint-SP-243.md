# Sprint SP-243 — `[Linux]` [EP-060] S3: the Inkwyn glyphs and the 8-item toolbar on Linux

**Status:** 🟡 **ACTIVE**, built 2026-11-24 (Docker + Qt 6.10 green; ⏳ CI on the push, the live pass on `scrivi-vm`). Activated 2026-11-24 (user, 2026-11-24: *"Q1-Q8 as recommended, implement SP-243"*); ✅ Q1–Q8 as recommended. Created 2026-11-24
(user, 2026-11-24: *"close SP-242, then plan Sprint 3"*).
**Carries:** [T-0706].
**Epic:** [EP-060] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — Sprint 3 of 3; closes AC4 (*"Linux shows the same mapped
glyphs and the Inkwyn window icon, legible in light and dark themes"*).
**Adopts:** the Mac's shape from [SP-242] → [`Closed/Sprint-SP-242.md`](Closed/Sprint-SP-242.md) (memory: Linux always adopts Apple's
shape) — the 8-item toolbar, the menu headers, the glyph mapping, 1.5×, Large in the toolbar.
**Rig:** ⚠️ the live pass runs on the Linux VM (`scrivi-vm`); x86 / GPU are not involved. A rig-only check, if any, goes to EP-057.
**Size:** M–L.

---

## Findings (2026-11-24)

1. **Toolbar** (`ScriviWindow::buildToolbar`, `ScriviWindow.cpp:1048`, [SP-232]): a `QToolBar`, words under freedesktop THEME icons
   (`themeIcon({...})`), every item the menu bar's own `QAction`, drop-downs = `QToolButton` + `InstantPopup` holding the menus'
   actions. Today **15 items**: Navigator, Inspector, Timeline, Project Settings, Worlds, Navigate▾, New▾, Merge▾, View▾, Import▾,
   Export▾, Close Project, Purge…, About. The » overflow is a drawn "»" with the comment *"shortcut: … until the app's glyph set has
   its icon"* — it now has one (`Menu-Expansion`). Inkwyn Settings exists as **Edit ▸ Settings…** (`appSettingsAction`).
2. **Icons elsewhere:** almost none. Card headers (`SceneInspector.cpp:462`) are an arrow `QToolButton`, a bold title, a "✕" button —
   no kind icon. The tabs (`tabs_->addTab`) have none. The object detail dialog's bar has theme icons (Back, Save, Undo, More, Close).
   The Buffers palette has theme icons (Cut/Copy/Paste). The window icon is `QIcon::fromTheme("scrivi")` (`main.cpp:83`) — nothing
   unless an icon theme provides it.
3. **Warnings are TEXT**: "⚠" inside 5 UI strings — the Inspector's pending row (`SceneInspector.cpp:1080`), the detail dialog's
   pending relation (`ObjectDetailDialog.cpp:1548`), the History card's abandoned-branches line (`HistorySection.cpp:381`), and the
   landing / worlds status text (`WorldStatusText.hpp`, `LandingWindow.cpp`, `WorldsDialog.cpp`).
4. **The Navigator knows the Mac's three chapter states** (`EditorShell.cpp:2286`): hers (bold), current (italic), closed. Its arrows
   are the Qt style's branch indicators; `NavigatorTree` is a `QTreeView` subclass, so `drawBranches` can draw the glyphs.
5. **No SVG support is linked:** `find_package(Qt6 … Quick Widgets QuickWidgets Test Concurrent)`. The pattern for bundled resources
   exists: `qt_add_resources` already bundles `Resources/Fonts` (`CMakeLists.txt:54`). Qt is installed in four places that must all
   gain SVG: the Dockerfile, the Linux CI workflow (twice), the rig/VM setup doc, and the `scrivi-qt610` image (aqt; check).
6. **The glyphs need the script's rules** (crop to the viewBox, sprite groups, pips dropped for Worlds/Close, specks dropped,
   strokes outlined, fitted). Reading `Resources/Icons` directly from C++ would mean a second copy of those rules (Q1).

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where Linux gets its glyphs.** | **The same script writes them too**: `scripts/make-glyph-symbols.swift` also emits `platforms/linux/resources/glyphs/<name>-small|medium|large.svg` — the already-cleaned, fitted, single filled path in a square viewBox, black. One pipeline, so the art can never differ between platforms (EP-060 AC5). Committed, like the catalog. Not: C++ reading `Resources/Icons` (a second copy of every rule). || ✅ As recommended (2026-11-24) |
| **Q2** | **Drawing and tinting.** | One small `GlyphIcon` (a `QIconEngine`): it renders the SVG with `QSvgRenderer` at the requested size × device-pixel ratio and **tints it with the widget's palette** (text colour; the disabled colour when disabled) every time it paints — so light/dark and HiDPI follow without code. Drawing by size: **Small ≤ 16 px, Medium ≤ 24 px, Large above**; the toolbar always Large (as the Mac). || ✅ As recommended (2026-11-24) |
| **Q3** | **The toolbar.** | The Mac's 8, in its order: **View▾** (Show or Hide Scene Navigator / Scene Inspector / Timeline, —, World Warnings, Markup Hints, Buffers), **Worlds**, **Navigate▾**, **New▾**, **Merge▾**, **Import/Export▾** (an Import submenu, an Export submenu + Copy as HTML), **Tools▾** (Project Settings…, Settings… [Edit ▸ Settings…], Purge Stale History Branches…, —, About Inkwyn), **Close Project**. Each drop-down opens with its **header** (its own word and glyph, then a divider; choosing it only closes). Qt's own drop-down arrow hidden (the pips say "menu"). The » overflow gets `menu.expansion`. || ✅ As recommended (2026-11-24) |
| **Q4** | **Words under the toolbar glyphs?** [SP-232] Q2 ruled *"the word under the theme's icon"*; the Mac shows the glyph alone. | **Keep the words**: the toolbar stays readable, and the shape (which items, which order) is what "same for all platforms" ruled. Say if you want glyphs alone, as the Mac. || ✅ As recommended (2026-11-24) |
| **Q5** | **Icons in menus.** The drop-downs hold the menu bar's own `QAction`s, so a glyph on an item shows in the menu bar's item too. | **Accept it** (the menu bar gains the same glyphs as the toolbar's drop-downs); duplicating every action to keep the menu bar plain would break "one code path" (EP-040 AC2). || ✅ As recommended (2026-11-24) |
| **Q6** | **Warnings** (finding 3). | **Every "⚠" becomes `inkwyn.warnings`**: rows (tree/list items) get it as their icon; a label gets the glyph beside it. The character leaves the strings. || ✅ As recommended (2026-11-24) |
| **Q7** | **The rest of the mapping on Linux.** | Card headers: the **kind glyph** before the title, the collapse arrow → `chapter.collapsed` / `chapter.expanded.automatic` (Small), "✕" → `close`. Tabs: Worldbuilding = the tree, Properties = `info`. Detail dialog bar: More → `more.options`, Close → `close`. Navigator branch arrows → the three chapter glyphs, by the states of finding 4 (opened by her → `expanded.automatic`, current → `expanded.manual`, as the Mac). Buffers palette's Cut/Copy/Paste stay theme icons (as the Mac kept Paste). || ✅ As recommended (2026-11-24) |
| **Q8** | **The window icon.** | The Inkwyn app icon PNG (scaled to 256) bundled and set as the window icon in `main.cpp`. The installed `.desktop` / hicolor icon stays [EP-056]'s. || ✅ As recommended (2026-11-24) |

## Plan

1. **Dependencies:** `Qt6::Svg` in `CMakeLists.txt`; `qt6-svg-dev` in the Dockerfile, both CI Qt installs and the rig doc's apt line;
   the `scrivi-qt610` image checked. ⚠️ `libqt6svg6` noted as a runtime dependency for [EP-056] (the `.deb`).
2. **The script** emits the Linux SVGs (Q1); its self-check covers one; a second run changes nothing.
3. **`GlyphIcon`** (Q2) + `qt_add_resources` of `resources/glyphs` and the window icon (Q8); a smoke test that every mapped name
   loads and paints non-empty pixels, light and dark.
4. **The toolbar** (Q3–Q5) — `toolbar_smoke.cpp` updated to the 8 items, the headers, the submenus.
5. **Warnings, cards, tabs, detail bar, Navigator** (Q6, Q7).
6. Build and test in Docker (as non-root, per the memory) and on `scrivi-vm`; CI on the push.
7. Live pass on `scrivi-vm`, light and dark.

## Acceptance Criteria

- [ ] **AC1** The toolbar is the Mac's 8 items in its order, every one on its Large glyph, each drop-down opening with its header and
  working (Import/Export submenus, Tools); no Qt drop-down arrow; » shows `menu.expansion`.
- [ ] **AC2** Cards, tabs, the detail bar, the Navigator's chapter arrows and every former "⚠" show their glyphs (Q6, Q7).
- [ ] **AC3** Every glyph is legible in a light and a dark theme, and sharp at 1× and 2× (`GlyphIcon` tints and renders per paint).
- [ ] **AC4** The window shows the Inkwyn icon.
- [ ] **AC5** The Linux glyphs come from the same script run as the Apple symbols; a second run changes nothing.
- [ ] **AC6** Docker build + ctest/smokes green (non-root); CI green on the push; live pass on `scrivi-vm`.

## What was built (2026-11-24)

- **Dependencies:** `Qt6::Svg` (`find_package`); `qt6-svg-dev` in the Dockerfile, both Linux CI Qt installs and the rig doc's apt
  line. `libqt6svg6` recorded on [EP-056] (`Epic-backlog.md`). `scrivi-qt610` already has Qt SVG.
- **The script (Q1)** also writes `platforms/linux/resources/glyphs/<name>-small|medium|large.svg` — **162 files** (54 × 3), the
  cleaned drawings as one black path each, viewBox = the drawing's own box. Re-runs are byte-identical.
- **`GlyphIcon` (Q2)** (`src/GlyphIcon.{hpp,cpp}`): a `QIconEngine` that renders the SVG at paint time, aspect-fit, Small ≤ 16 /
  Medium ≤ 24 / Large above (or a forced drawing), tinted with the application palette's text colour (disabled colour when
  disabled; an optional fixed tint for the attention colour); `GlyphIcon::mark` puts a glyph in a layout where a `QLabel` would have
  been (a button with no clicks, so it repaints on a theme switch); `appIcon()`.
  ⚠️ **Found by its own smoke: Qt changed `QIconEngine::scaledPixmap`'s `size`** — device pixels before 6.8 (`qicon.cpp`
  v6.4.2:837), device-independent since (v6.10.0:933). Read one way, a 24-pt icon at 2× came out 96 px. ✅ Version-split; glyph_smoke
  §4 passes on **Qt 6.4** (Docker) **and Qt 6.10** (`scrivi-qt610`).
- **Build:** a static library `scrivi_linux_glyphs` holding `GlyphIcon` and the bundled SVGs + app icon; a loop at the end of
  `CMakeLists.txt` links it into every executable whose SOURCES include a glyph user (read from the target, not restated per target).
- **Toolbar (Q3–Q5)** (`ScriviWindow::buildToolbar`): **View▾, Worlds, Navigate▾, New▾, Merge▾, Import/Export▾, Tools▾, Close
  Project** — Large glyphs in a 34×24 icon box, words kept under them; each drop-down opens with its **header** (its word and glyph,
  a rule); Qt's drop-down arrow hidden; **Import/Export** = an Import submenu (File ▸ Import's items) and an Export submenu (File ▸
  Export's + Copy as HTML) with the ship glyphs; **Tools** = Project Settings…, Settings… (Edit ▸ Settings…), Purge…, —, About;
  the menus' own actions carry their glyphs, so the menu bar shows them too (Q5); the » overflow draws `menu.expansion`.
- **Q6 warnings:** the Inspector's pending row and the detail dialog's pending relation (icon, "⚠ " gone from the text); the History
  card's abandoned-branches line (a glyph beside it); the Worlds dialog's unavailable marker and the world-warning strip's mark
  ("▲" → `warnings` in the attention colour; the strip's "×" → `close`).
- **Q7:** card headers — the kind glyph (`inkwyn.<subdir>`, else `inkwyn.<kind>`: derived from the core's kinds, no table), the
  collapse arrow → the chapter glyphs at Small, "✕" → `close`; the tabs — the tree, `info`; the detail bar — `more.options`,
  `close`; a tag chip's "✕" → `close`; the **Navigator's branch arrows** → the three chapter glyphs (`NavigatorTree::drawBranches`,
  reading "hers" from the bold font EditorShell already gives it). The Buffers palette keeps its theme icons.
- **Q8:** the window icon falls back to the bundled Inkwyn icon when the theme has no `scrivi` icon.

## Tests

- **`glyph_smoke`** (new; CI step added): every bundled glyph loads and draws ink at 16/24/32 px; light/dark/disabled tint on the SAME
  icon object; a tint wins; unknown name → null; the app icon; HiDPI at 2×. ✅ Qt 6.4 and Qt 6.10.
- **`toolbar_smoke`** rewritten to the 8 items, the menus' own actions in each drop-down, the headers, no Qt arrow, the Import/Export
  submenus, Tools, the glyph overflow. ✅
- ✅ Also green (Docker, Qt 6.4, non-root): inspector_stack, inspector_objects, object_detail, world_warning, navigator_search, branch,
  buffers, settings.
- ⚠️ `scrivi-vm` was not running, so nothing was built there; the live pass builds it.
