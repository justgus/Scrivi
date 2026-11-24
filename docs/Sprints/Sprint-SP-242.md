# Sprint SP-242 — `[Apple]` [EP-060] S2: the glyphs in the app, and the smaller toolbar (Mac and iOS)

**Status:** 🟡 **ACTIVE** — AC1–AC4, AC6 ✅ (live pass (user, 2026-11-24: *"All steps pass on the Mac, iPad, and iPhone."*)); ⏳ AC5: CI on the push. Activated 2026-11-24 (user, 2026-11-24: *"Q1-Q6 as recommended, implement SP-242"*); ✅ Q1–Q6 as recommended. Created 2026-11-24
(user, 2026-11-24: *"close SP-241, then plan Sprint 2"*).
**Carries:** [T-0705].
**Epic:** [EP-060] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — Sprint 2 of 3. ⛔ Sprint 3 (`[Linux]`, the same toolbar
shape and glyphs) is blocked by this one: Linux adopts the shape built here.
**Builds from:** [SP-241]'s approved mapping and rulings (a)–(f) → [`Closed/Sprint-SP-241.md`](Closed/Sprint-SP-241.md) §"The mapping"
and §"Rulings".
**Size:** L, but mostly mechanical. The risk is in three places, so each is proved before the bulk of the work: menu-item icons on the
Mac (Q1), the 2× glyphs at toolbar size (Q2), and the toolbar's new shape (Q3).

---

## Findings (2026-11-24)

1. **The toolbar has 14 items** (`ProjectToolbar.swift`): Inspector, Timeline, Project Settings, Worlds, Navigate▾, New▾, Merge▾,
   View▾, Import▾, Export▾, Close Project, Inkwyn Settings, Purge Stale History Branches…, About Inkwyn. On iOS they sit in
   `.secondaryAction` (the overflow); on the Mac, `.automatic`. ✅ Rulings (e) and (f) make it **10**: Import▾ + Export▾ → one
   **Import/Export▾** (an Import submenu and an Export submenu), and Project Settings + Inkwyn Settings + Purge + About → one **Tools▾**.
2. ⚠️ **macOS draws no icon in a SwiftUI menu item** — the Add Card menu shows none, SF Symbols included (SP-241 run 2), and I-0035
   (2026-06-12) recorded the same for a context menu. Apple's `titleAndIcon` page says *"some containers might apply a different default
   label style to their content"* and that `.labelStyle(.titleAndIcon)` opts in. Not yet tried in this app. **iOS menus do show icons.**
   This decides whether ruling (c), and every glyph on a menu ITEM, is visible on the Mac at all (Q1).
3. ⚠️ **The glyphs are 2× Apple's size** (SP-241 run 2, at the user's request, judged in the card headers). A toolbar item draws its
   image at the toolbar's own symbol size, so a 2× glyph may be too big or clipped there. Never rendered yet (Q2).
4. **Call sites** (excluding tests): warnings `exclamationmark.triangle` 15 + `.fill` 7 in 15 files; `ellipsis.circle` 5; `xmark` 6;
   `minus.circle` 5; `link` 3; `lock` 2; `info.circle` 2; `doc.on.clipboard` 2; `plus` 3; card chevrons 1 site
   (`InspectorCardStackView.swift:297`); Navigator disclosure 1 site (`SceneNavigatorView.swift:879`); the 10 kinds in one table
   (`ObjectCard.swift` `ObjectCardKind.all`), already drawn through `Image(glyph:)` / `Label(_:glyph:)` from SP-241.
5. **Other warning-like symbols** are composite badges: `photo.badge.exclamationmark` (3), `externaldrive.badge.exclamationmark`,
   `externaldrive.badge.xmark`, `exclamationmark.circle` (Q4).
6. **The Mac menu bar** (`ScriviApp.swift` command menus) is outside the approved mapping; it is not touched.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Icons in Mac menu items** (finding 2). | **Spike first, on the Add Card menu**: `.labelStyle(.titleAndIcon)` on its items, checked on screen. If macOS then draws them, the same goes on every menu this Sprint touches. If it does not, menu-item glyphs show **on iOS only**, and the Mac keeps glyphs on buttons, cards and rows; I come back to you before going further. || ✅ As recommended (2026-11-24) |
| **Q2** | **2× glyphs in the toolbar** (finding 3). | Render the toolbar early (step 2). If they are too big, the toolbar draws its glyphs one scale down (`.imageScale(.small)`, the Small drawing at 0.79×) and everywhere else stays 2×. One modifier on the toolbar, not per item. || ✅ As recommended (2026-11-24) |
| **Q3** | **The new toolbar's order.** | Today's order, merged in place: **Inspector, Timeline, Worlds, Navigate▾, New▾, Merge▾, View▾, Import/Export▾, Tools▾, Close Project.** Tools▾ holds Project Settings… (⌥⌘,), Inkwyn Settings…, Purge Stale History Branches…, About Inkwyn, with a divider before About. || ✅ As recommended (2026-11-24) |
| **Q4** | **Ruling (a) — which icons are "warning icons".** | `exclamationmark.triangle` and `.fill` (22 sites) → `inkwyn.warnings`. **The composite badges stay system** (finding 5): a photo or drive with a mark says *what* is wrong, which the warnings glyph alone would lose. || ✅ As recommended (2026-11-24) |
| **Q5** | **Menu items with no glyph** (Tools: Project Settings, Inkwyn Settings, Purge). | They keep their system symbol (`gearshape`, `gearshape.2`, `arrow.triangle.branch`), as the mapping says for anything without a glyph. About takes `inkwyn.info`. || ✅ As recommended (2026-11-24) |
| **Q6** | **How call sites name a glyph.** | A literal becomes `Image("inkwyn.x")` / `Label("…", image: "inkwyn.x")`; a name held in a variable goes through SP-241's `Image(glyph:)` / `Label(_:glyph:)`. No new helper. || ✅ As recommended (2026-11-24) |

## Plan (in order; each ⛔ is a stop-and-check)

1. **Q1 spike:** the Add Card menu with `.labelStyle(.titleAndIcon)` and the kind glyphs. ⛔ You check it on the Mac.
2. **Q2 + Q3:** the 10-item toolbar (Import/Export▾ with two submenus, Tools▾), every toolbar item on its glyph: `right.sidebar`,
   `timeline`, `menu.worlds`, `menu.navigate`, `menu.create`, `menu.merge`, `menu.view`, `menu.import.export`, `menu.tools`,
   `menu.close`. Rendered by me at the toolbar's size before you see it. ⛔ You check it on the Mac and iPhone.
3. **Menu items** (as far as Q1 allows): Navigate (`scene/chapter/manuscript.start/end`), New (`new.scene`, `new.chapter`), Merge
   (`merge.scene/chapter.with.previous`), View (`warnings`, `hints`, `buffers`), Import/Export submenus (`import`, `export`) and their
   items (`manuscript`, `chapter`, `timeline`, `archive`), Tools (Q5).
4. **The kinds:** the other 9 in `ObjectCardKind.all` (cards, Add Card, relation rows).
5. **Warnings everywhere** (Q4), **utility glyphs** (`more.options`, `close`, `remove.delete`, `create.citation.link`, `locked`,
   `info`, `add.create`, `buffers` in the palette).
6. **Chevrons:** the Navigator's chapter disclosure → `chapter.collapsed` / `chapter.expanded.manual` / `chapter.expanded.automatic`;
   the Inspector cards' collapse → `chapter.collapsed` / `chapter.expanded.automatic` at **Small** (ruling (b)).
7. Build macOS and iOS (never visionOS), no new warnings; interop and iOS suites; a grep that no mapped SF Symbol is left.
8. Live pass on the Mac, the iPhone simulator and Remus.

## Acceptance Criteria

- [x] **AC1** The toolbar has the 10 items of Q3, on the Mac and iOS. Import/Export▾ opens Import and Export submenus with today's
  items; Tools▾ opens Project Settings…, Inkwyn Settings…, Purge Stale History Branches… and About Inkwyn, and each still works.
- [x] **AC2** Every toolbar item shows its Inkwyn glyph, at a size that fits the toolbar (Q2), in light and dark.
- [x] **AC3** Every mapped place shows its glyph: the 10 kinds, warnings (Q4), the utility glyphs, the chevrons (ruling (b)), and menu
  items as far as Q1 allows. A grep finds none of the replaced SF Symbol names left in `Scrivi/` outside tests.
- [x] **AC4** The Add Card menu shows the kind glyphs on iOS, and on the Mac if Q1's spike works (otherwise the reason is recorded).
- [ ] **AC5** macOS and iOS build with no new warnings; interop and iOS tests green; CI green on the push.
- [x] **AC6** Live pass on the Mac and iOS.

## What was built (2026-11-24)

⚠️ **Order changed:** the plan's two ⛔ stops for your checks (Q1 spike, toolbar) moved into the live pass instead of pausing the
build. Nothing built depends on Q1's outcome: if Mac menus still drop item icons, the glyphs there remain right for iOS.

- **Toolbar** (`ProjectToolbar.swift`): **10 items** in Q3's order. **Import/Export▾** has an **Import** submenu (`inkwyn.import`) and
  an **Export** submenu (`inkwyn.export`, with Copy as HTML); **Tools▾** holds Project Settings… (`gearshape`), Inkwyn Settings…
  (`gearshape.2`; on the Mac a `SettingsLink` to the Settings scene, on iOS the sheet as before), Purge Stale History Branches…
  (`arrow.triangle.branch`), a divider, About Inkwyn (`inkwyn.info`). Every toolbar item on its glyph; Worlds without pips.
- **Q2 triggered:** rendered at toolbar size (15 pt symbols in 32×28 buttons, an approximation of the Mac toolbar), the 2× glyphs
  overflowed badly at Medium. ✅ Every toolbar label is `.imageScale(.small)`. ⚠️ Even at Small the wide ones (Inspector, Timeline)
  are wider than an SF button in that render; the real toolbar may size buttons to content — the live pass decides.
- ✅ **Superseded on the Mac — aspect fit, Large** (user, 2026-11-24: *"Yes, add it to SP-242, Large in the toolbar"*): a new
  `FittedGlyph` (`Glyph.swift`) draws a glyph `.resizable().scaledToFit()` in a square box tied to the text size
  (`@ScaledMetric`, 20 pt at body) with the **Large** drawing; the Mac toolbar's 10 labels use it (`toolbarLabel` in
  `ProjectToolbar.swift`). Rendered first: every glyph, wide or tall, fits a 22×22 box, and `imageScale` still picks the drawing.
  ⚠️ **iOS keeps the Small symbol form**: the iPhone's toolbar items live in the ⋯ overflow MENU, and a menu takes the image alone,
  ignoring a fitted view. ⚠️ Aspect fit sizes a drawing by its whole extent, so a stray speck shrinks the glyph: **Close-Small**
  (speck bottom-right) and **Import-Export-Large** (speck top-left) — for the artist, or a speck filter in the script.
- **Q1:** `.labelStyle(.titleAndIcon)` on the content of every toolbar menu and on the Add Card items (never on a Menu itself,
  where it would put titles on toolbar buttons).
- **Menu items:** Navigate (6), New (2), Merge (2), View (`warnings`, `hints`, `buffers`), Import/Export items (`manuscript`,
  `timeline`, `archive`; Export also `chapter`; Scrivener Project has none). ⚠️ `ImportMenuItems`/`ExportMenuItems` are shared with
  **File ▸ Import / Export** in the Mac menu bar, so those items carry the glyphs there too.
- **Kinds:** all 10 in `ObjectCardKind.all` → cards, Add Card, relation rows.
- **Warnings (Q4):** every `exclamationmark.triangle` / `.fill` → `inkwyn.warnings` (22 sites); composite badges kept.
- **Utility:** `more.options` (5), `close` (6), `remove.delete` (object cards, Worlds), `create.citation.link` (Cite…), `locked` (2),
  `info` (About, the Properties tab), `add.create` (Add Card, Add <Kind>…).
- **Chevrons:** Inspector cards → `chapter.collapsed` / `chapter.expanded.automatic` at Small (ruling (b)); Navigator →
  `chapter.collapsed` / `.expanded.manual` (opened by her) / `.expanded.automatic` (open because current or searched).

**Deviations from the mapping — for your ruling if any is wrong:**
1. The **Navigator** chevrons are also at **Small**: at Medium the 2× glyph overflows the Mac's 16-pt disclosure frame.
2. The **History graph**'s node markers keep `minus.circle` / `minus.circle.fill`: the filled/hollow difference is the meaning,
   and the glyph has one form. (The mapping listed "History" under remove rows; these are not remove buttons.)
3. The **Buffers palette**'s Paste action keeps `doc.on.clipboard`: there it means Paste, not Buffers.
4. The **Timeline strip**'s small `chevron.down` (a pop-up indicator) stays system: ruling (b) named the Inspector cards.

## Tests

- macOS and iOS build, no new warnings. Mac interop **355 passed**; iOS **38 passed**.
- AC3 grep: no replaced SF Symbol name left in `Scrivi/` outside tests, except the deliberate ones above and strings that are not
  symbols (`"link"` import notes, the kind name `"map"`).
- Renders (approximations via `ImageRenderer` against the built app): the toolbar at Medium vs Small (Q2), and all 10 kind card headers.

## Live pass — run 1 (2026-11-24) and the changes it asked for

> User, 2026-11-24: *"I have these beautiful icons but the menu indicators are making them too small to see! … the adaptation is
> too small. Also Chapter expanded Automatic and Chapter expanded Manual need to be switched. I also recommend a scaled version of the
> tree as the icon for the worldbuilding tab. … move the View Menu to leftmost (leading) in the toolbar and incorporate Show/Hide
> Scene Inspector and Show/Hide Timeline. We also want to add Show/Hide Navigator to the View Toolbar button. The icons in the Add
> menu on Macos are visible."* · *"There should already be a menu item for show/Hide Navigator. I want to make the toolbar button
> option do the same thing."*

- ✅ **Q1 passed:** `.labelStyle(.titleAndIcon)` makes macOS draw icons in a SwiftUI menu (the Add Card menu shows the kind glyphs).
- **Toolbar glyphs bigger:** `FittedGlyph`'s box 20×20 → **26 high × 36 wide** (wider than tall, so the pips sit beside the art);
  the system menu chevron is hidden on every toolbar menu (`.menuIndicator(.hidden)`) — the pips already say "menu".
- **Close Project drops its pips too** — it opens no menu (the Worlds ruling's reason). Its pips are SUBPATHS of the main path, so the
  script's pip rule now drops pip-shaped subpaths (< 12×12 at x ≥ 105 of the tile); of all 54 symbols only `inkwyn.menu.close`
  changed. ✅ This also explains "Close is very thin at Small": the "speck" was a pip.
- **The toolbar is 8 items, View▾ leading:** **View▾, Worlds, Navigate▾, New▾, Merge▾, Import/Export▾, Tools▾, Close Project.**
  View▾ = Scene Navigator, Scene Inspector, Timeline, —, World Warnings, Markup Hints, Buffers (Mac). The separate Inspector and
  Timeline buttons are gone.
  - **Mac:** the SAME actions as the menu bar — *Show or Hide Scene Navigator* sends `toggleSidebar:` (NavigationSplitView's own
    View ▸ Show/Hide Sidebar), *Show or Hide Scene Inspector / Timeline* flip the value as View ▸ Show or Hide … do.
  - **iPad:** toggles; the Navigator's split-view column now FOLLOWS `session.navigatorVisible` (`EditorView.followNavigatorVisible`,
    a no-op when they agree, so it cannot ping-pong with the recorder). **iPhone:** Scene Navigator / Scene Inspector go to the page.
- **Navigator chevrons swapped:** opened by her → `chapter.expanded.automatic`; open because current/searched → `chapter.expanded.manual`.
  (The Inspector cards keep `chapter.expanded.automatic` for expanded, as ruling (b) named it.)
- **Worldbuilding tab = the tree** (`inkwyn.menu.worlds`, no pips), drawn fitted in the tab's 17-pt icon box (Small drawing); the
  Properties tab's `inkwyn.info` the same way. Writing keeps `pencil.line`.
- ⚠️ Still open: **Merge** and **Import/Export** (Large) each carry a stray speck top-left, which makes them smaller than the rest
  under aspect fit — for the artist, or a speck filter in the script.
- Rebuilt macOS and iOS; interop **355**, iOS **38** passed.

**1.5× instead of 2×** (user, 2026-11-24: *"I'd like to see it when the icons are at 1.5x instead of 2x. In the toolbar and the
inspector they are a bit oversized."*): the script's box 212×280 → **159×210** (`inkwyn.characters` 190×212 → **143×159** at 100 pt);
the Mac toolbar's fitted box 26 → **19.5** high (× 1.4 wide); the Inspector tabs' 17 → **13** (the SF Writing icon's size). Rendered
first (card headers, toolbar, tabs); macOS and iOS rebuilt; interop green.

**A dismissing header in each toolbar menu** (user, 2026-11-24: *"as the top of the popup, display the menu item that it was
generated from. Selecting that menu item will simply dismiss the popup as if I had tapped outside it."*): View, Navigate, New, Merge,
Import/Export and Tools open with their own title and glyph, then a divider (`menuHeader` in `ProjectToolbar.swift`). It does
nothing, and choosing any item closes a menu, so it dismisses. Mac and iPad; **not on the iPhone**, where every item is in the ⋯
overflow and the system already shows that header. ⚠️ An iPad item the system overflows into ⋯ shows both. (These are system
menus with the arrow hidden, not sheets.) macOS and iOS rebuilt.

✅ **Menu headers work as intended** (user, 2026-11-24: *"The items are not duplicated. It works as intended."*).

**Stray specks ignored** (user, 2026-11-24: *"Have the script ignore the stray specks."*): `withoutSpecks` drops a piece that is
**tiny** (< 0.15% of the canvas) **and isolated** (near no other piece within 2% of the canvas). ⚠️ A first rule — "outside the main
body" — also dropped Item's handle diamond and three Vehicles edge knots (real art next to other ink); caught by diffing all 54 and
printing every dropped piece, then replaced. ✅ Now only `inkwyn.menu.merge` and `inkwyn.menu.import.export` change; the self-check
gained a lone 1×1 speck that must vanish, and a mutant without the filter fails it (run in a scratch copy). Mac rebuilt; interop green.

**Run 2** ✅ (user, 2026-11-24: *"All steps pass on the Mac, iPad, and iPhone."*).
