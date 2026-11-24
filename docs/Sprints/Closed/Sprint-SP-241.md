# Sprint SP-241 — `[Apple]` [EP-060] S1: the glyph pipeline and the Inkwyn app icon

**Status:** ✅ **CLOSED** 2026-11-24 (user, 2026-11-24: *"close SP-241, then plan Sprint 2"*). Was 🟠 Complete 2026-11-24. AC1–AC6 met: live pass ✅ (run 3, user, 2026-11-24: *"Step 3 passes
now"*); mapping approved with rulings (a)–(f); CI ✅ on `2892f96` (Scrivi Apple CI — the boundary, TextKit, layout and stub-parity
guards; Core and Linux CI not triggered: no C++ or Linux change). Was 🟡 ACTIVE 2026-11-24 (user, 2026-11-24: *"Q1-Q7 as
recommended, implement SP-241"*); ✅ Q1–Q7 as recommended. Created 2026-11-24 (user, 2026-11-24: *"Ok we're activating EP-060 and
planning the first sprint."*).
**Carries:** [T-0704] (✅ Verified 2026-11-24 → [`../../Tasks/Verified/Task-verified-0704.md`](../../Tasks/Verified/Task-verified-0704.md)).
**Epic:** [EP-060] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — proposed Sprint 1 of 3. Sprint 2 (`[Apple]` the glyphs at
their call sites) needs this Sprint's mapping table; Sprint 3 (`[Linux]`) needs nothing from it but the table.
**Size:** M. Most of it is one script; the risk is concentrated in whether Xcode accepts what the script writes, so that is
proved on one glyph first (Q5).

---

## Findings (2026-11-24)

1. **Tools on this Mac:** SF Symbols.app and Icon Composer (in `/Applications` and inside Xcode). No Inkscape, `rsvg-convert` or
   SwiftDraw, and none are needed (finding 3).
2. **The SVGs use a small subset of SVG.** Across all 141 files: `<path>` (358), `<g>` (95), `<circle>` (34), `<rect>` (11);
   path commands `M L H V Q C Z`, absolute only; `transform=` 31 times; `fill-rule` 67 times. No arcs, text, images, masks, clips,
   dashes or opacity.
3. ⚠️ **92 of the 141 files draw with STROKES.** A custom symbol has to be filled outlines, so every stroke must be turned into a
   fill first. ✅ CoreGraphics does this (`CGPath.copy(strokingWithWidth:lineCap:lineJoin:miterLimit:)`, then `union`), so a Swift
   script can convert the whole set with **no new dependency**.
4. **A custom symbol is a `.symbolset` in the asset catalog**: one SVG in Apple's symbol template, holding up to three drawings
   `Regular-S`, `Regular-M` and `Regular-L`, plus baseline, cap-height and margin guides. The art's Small/Medium/Large go straight
   into S/M/L. The art has one weight, so there is only `Regular` (Q3).
5. **App icon today:** `ASSETCATALOG_COMPILER_APPICON_NAME` = `AppIconMacOS` (macOS), `AppIconIOS` (iOS), `AppIconVisionOS`
   (visionOS), in `Scrivi.xcodeproj/project.pbxproj`, two configurations each. All three hold Scrivi art.
6. **The Inkwyn PNG is 1254×1254, square and fully opaque.** An Icon Composer `.icon` is a folder (`icon.json` + `Assets/`); Xcode
   builds it into the app icon when the build setting names it.
7. **`Resources/Icons` (14 MB) and `Resources/AppIcon` are untracked.** **13 MB of the 14 are the PNG previews**, which nothing
   will read (Q6).
8. **There is no repeatable conversion step anywhere in `scripts/`** to copy; the nearest pattern is `gen-unicode-classes.py`, a
   generator whose output is committed.

## Questions

**First: EP-060's Q1–Q7** (in [`../Epics/Epic-active.md`](../../Epics/Epic-active.md)). ✅ Taken as recommended with this Sprint's
activation (it was planned on them):
custom symbols (Q1), an Icon Composer icon (Q2), visionOS and Windows left out (Q3, Q7), a mapping table (Q4), canvases
evened out in the conversion (Q5).

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **How the conversion is done.** | **A Swift script**, `scripts/make-glyph-symbols.swift`: it reads each glyph's three SVGs, turns strokes into fills (finding 3), fits each drawing into the template's S/M/L box (EP-060 Q5), and writes `Scrivi/Assets.xcassets/Glyphs/<name>.symbolset`. Not by hand in SF Symbols.app: 47 glyphs × 3 is 141 manual imports, and they would all have to be done again whenever the artist changes something (EP-060 AC5). || ✅ As recommended (2026-11-24) |
| **Q2** | **Symbol names.** | **`inkwyn.` plus the file name, lowercased, with dots in place of dashes**: `Add-Create` → `inkwyn.add.create`, `Chapter-Expanded-Manual` → `inkwyn.chapter.expanded.manual`, `Characters` → `inkwyn.characters`. Generated from the file name, so nobody has to keep a list of names. || ✅ As recommended (2026-11-24) |
| **Q3** | **Weight.** | **Regular only.** The art comes in one weight, so a glyph next to bold text stays Regular. If that looks wrong, the artist draws more weights later. || ✅ As recommended (2026-11-24) |
| **Q4** | **Where the output lives.** | **Committed** to the catalog, like `gen-unicode-classes.py`'s output. Re-run the script when the art changes. Not a build phase: that would slow every build and run a script inside Xcode's sandbox. || ✅ As recommended (2026-11-24) |
| **Q5** | **The spike gate.** | The script is first run on **one glyph** (`Characters`, which draws with strokes), and that glyph is put in **one real place** on the Mac and iOS (the Characters card header in the Scene Inspector). Only when it passes (it follows the text size, `imageScale` picks the S/M/L drawing, and it tints in light and dark) are the other 46 converted. **If it fails, I stop and come back to you** with EP-060 Q1(a) (image sets) as the fallback. || ✅ As recommended (2026-11-24) |
| **Q6** | **What gets committed from `Resources/`.** | **The SVGs, the `*_Specification.md` files and the app-icon PNG.** Add `Resources/Icons/**/*.png` to `.gitignore`: the PNG previews are 13 MB that nothing reads. They stay on your disk. || ✅ As recommended (2026-11-24) |
| **Q7** | **The old icons.** | **Delete `AppIconMacOS` and `AppIconIOS`** (Scrivi art) once the `.icon` is building. **Leave `AppIconVisionOS`** as it is (EP-060 Q3); EP-060 AC1 is reworded to say so. || ✅ As recommended (2026-11-24) |

## Plan

1. **The script** (Q1, Q2), with a built-in self-check: it reads one hand-written SVG with a known shape and checks the box it
   produces.
2. **The spike** (Q5): `Characters` → `inkwyn.characters` → the Characters card header on the Mac and iOS. ⛔ Stop here if it fails.
3. **The other 46 glyphs**, plus a **contact sheet**: one PNG showing all 47 at S, M and L side by side, so you can see in one
   look whether the sizes match (EP-060 Q5) before Sprint 2 places any of them.
4. **The mapping table** (EP-060 Q4), written into this record for your approval: glyph ↔ SF Symbol it replaces ↔ Qt icon ↔ call
   sites (from the 124 `systemName:`/`systemImage:` uses). Places with no matching glyph are listed as keeping their system symbol.
   ⛔ Nothing is swapped in the UI yet except the spike glyph: that is Sprint 2's job.
5. **The app icon** (EP-060 Q2): `Scrivi/Inkwyn.icon` built from the PNG scaled to 1024, one layer; the macOS and iOS
   `ASSETCATALOG_COMPILER_APPICON_NAME` point to it; `AppIconMacOS` and `AppIconIOS` are deleted (Q7); `project.pbxproj` gets the
   new file. You can refine the layers later in Icon Composer without changing anything else.
6. Build macOS and iOS (never visionOS) with no new warnings, and run the suites.
7. Live pass on the Mac, the iPhone simulator and Remus.

## Acceptance Criteria

- [x] **AC1** One command (`swift scripts/make-glyph-symbols.swift`) writes all **47** `inkwyn.*` symbolsets from
  `Resources/Icons/`. Running it a second time changes nothing (`git status` stays clean).
- [x] **AC2** The spike glyph `inkwyn.characters` shows in the Characters card header on the Mac and iOS. It grows and shrinks with
  the text size, `imageScale` picks its Small/Medium/Large drawing, and it is tinted and legible in light and dark.
- [x] **AC3** The contact sheet shows all 47 glyphs at the same visual size per scale. Any glyph that doesn't is listed for the artist.
- [x] **AC4** The mapping table is in this record and approved by you.
- [x] **AC5** The Inkwyn icon shows in the Dock, Finder and About on the Mac, and on the Home Screen and in the App Switcher on iOS.
  No Scrivi icon art remains for macOS or iOS.
- [x] **AC6** macOS and iOS build with no new warnings; ScriviCore, interop and iOS tests are green; CI is green on the push.

## What was built (2026-11-24)

**The script** — [`scripts/make-glyph-symbols.swift`](../../../scripts/make-glyph-symbols.swift) (Q1), no dependencies beyond the
SDK: a small SVG reader (`<svg>` nested too, `<g>`, `<path>` M L H V Q C Z, `<circle>`, `<rect rx>`, inherited fill/stroke and
transforms; anything else stops it with the file named) → strokes outlined with `CGPath.copy(strokingWithWidth:)` → painted in
order (ink is unioned, **white is cut out**: Info-Small, Factions-Small) → **cropped to the viewBox** → fitted into one box
(~~84 × 105~~ → ~~106 × 140~~ → **212 high or 280 wide** at Medium after live-pass runs 1 and 2 — twice Apple's kind symbols, centred on the cap height; Small 0.79×,
Large 1.29×, measured from system symbols) → written
into Apple's symbol template (geometry copied from the template SF Symbols.app ships) as `Scrivi/Assets.xcassets/Glyphs/*.symbolset`.
`--only <Name>` for one glyph, `--sheet <png>` for the contact sheet. It checks itself first on a known shape (box, knock-out, stroke, fit).

**46 symbols, not 47.** `Timeline` is in both the View and Import/Export sets, byte-identical, so it is one symbol
(`inkwyn.timeline`); the script refuses two DIFFERENT drawings with one name.

**The spike** (Q5) — `inkwyn.characters` in the Characters card header (`ObjectCard.swift`). It reaches the card header, the
Add Card menu and the relation rows through one small helper, [`Glyph.swift`](../../../Scrivi/Views/Glyph.swift): `Image(glyph:)` and
`Label(_:glyph:)` draw an `inkwyn.*` name from the catalog and anything else as an SF Symbol. Checked headless on the built app:
actool compiled it with three renditions (Glyph Size Small/Medium/Large), and at 100 pt it loads as 61×81 / 76×98 / 98×123
(Apple's `person.fill` Medium: 108×105). ⏳ How it looks in the app is the live pass.

**The app icon** — `Scrivi/Inkwyn.icon` (Icon Composer: the PNG scaled to 1024, one layer); `ASSETCATALOG_COMPILER_APPICON_NAME =
Inkwyn` for the macOS and iOS targets; `AppIconMacOS` and `AppIconIOS` deleted (Q7); `AppIconVisionOS` untouched. The built Mac
app's `Inkwyn.icns` shows the art inside the macOS squircle with the glass edge; the iOS app lists `CFBundleIconName = Inkwyn`.

**Q6** — `.gitignore`: `Resources/Icons/**/*.png`.

**Found on the way (art, for the artist; nothing changed in `Resources/`):**
- **Item-Small and Item-Medium** each hold one leftover shape far outside their visible area (y ≈ 290–307 under a viewBox that
  ends at ≈ 114). A renderer hides it; the first run drew it. The crop to the viewBox fixed it.
- **Unlocked-Small**: the shackle's line pokes 3.4 units above its 120-unit canvas, so its top edge is cut (as in the artist's PNG).
- **Item** is a long, thin spoon: at caption size it is a short line. **Vehicles** (the knotwork sea-horse) is very fine at Small.
- **Import** and **Export** are the same ship facing opposite ways — by design, but easy to confuse side by side.

## Tests

- The script's self-check passes; **AC1**: two runs give byte-identical output (one `shasum` over all 46 files).
- **AC3**: the contact sheet — all 46 at S, M, L on baseline/cap-height guides — shows one visual size per scale.
- macOS and iOS build with no new warnings. iOS suite: **38 tests in 9 suites passed**. Mac interop: **355 tests in 46 suites passed**.
- Not run: ScriviCore `ctest` (no C++ changed); visionOS (never built).

## The mapping (AC4) — ✅ APPROVED (user, 2026-11-24: *"the proposed mapping is approved."*), with rulings (a)–(f) below

Sprint 2 swaps these. **Where** is the Apple call site; **Linux** is Sprint 3's. ⛔ Every SF Symbol not listed keeps its system
symbol (checkmark, magnifyingglass, gearshape, globe, eye, photo, scope, person.crop.circle, doc.text, the history-graph dots …).

| Glyph (`inkwyn.…`) | Replaces | Where (Apple) |
|---|---|---|
| `characters` ✅ spike | `person.2` | Characters card, Add Card, relation rows (`ObjectCard.swift` kind table) |
| `locations` | `mappin.and.ellipse` | the same three places, Locations |
| `buildings` | `building.2` | — Buildings |
| `vehicles` | `car` | — Vehicles |
| `item` | `shippingbox` | — Items |
| `maps` | `map` | — Maps |
| `artifact` | `crown` | — Artifacts |
| `chronicles` | `book.closed` | — Chronicles |
| `factions` | `flag.2.crossed` | — Factions |
| `rules` | `scroll` | — Rules |
| `scene.start` / `scene.end` | `arrow.up.to.line.compact` / `arrow.down…compact` | Navigate menu (`ProjectToolbar.swift`) |
| `chapter.start` / `chapter.end` | `arrow.up.to.line.circle` / `arrow.down…circle` | Navigate menu |
| `manuscript.start` / `manuscript.end` | `arrow.up.to.line` / `arrow.down.to.line` | Navigate menu |
| `new.scene` / `new.chapter` | `plus.rectangle` / `plus.rectangle.on.folder` | New menu |
| `add.create` | `plus` | New menu's label, Add Card, Add <Kind>… |
| `merge.scene.with.previous` | `arrow.triangle.merge` | Merge menu item **and** its label |
| `merge.chapter.with.previous` | `rectangle.compress.vertical` | Merge menu |
| `import` / `export` | `square.and.arrow.down` / `…up` | toolbar Import, Export |
| `manuscript` / `chapter` / `timeline` / `archive` | — (no icons today) | **items** of Import/Export (Manuscript, Current Chapter, Timeline…, Archive…) — ⚠️ adds icons where there are none |
| `timeline` | `calendar.day.timeline.left` | toolbar Timeline |
| `buffers` | `doc.on.clipboard` | toolbar Buffers, Buffers palette |
| `hints` | `textformat` | View ▸ Markup Hints |
| `warnings` | `exclamationmark.triangle` (+ `.fill`) | toolbar World Warnings and the world-warning strip — ⚠️ **not** the 15 other inline warnings (Q below) |
| `right.sidebar` | `sidebar.trailing` | toolbar Inspector |
| `info` | `info.circle` | About Inkwyn, the Properties tab |
| `more.options` | `ellipsis.circle` | Stack Options, detail sheet More, Authorship, Timeline strip |
| `close` | `xmark` | Close Project, a card's ×, banners' × |
| `remove.delete` | `minus.circle` | remove rows (History, object cards, Worlds) |
| `create.citation.link` | `link` | Cite… |
| `locked` | `lock` | world-locked banner, built-in relation types |
| `chapter.collapsed` / `chapter.expanded.manual` / `chapter.expanded.automatic` | `arrowtriangle.right` / `.down.fill` / `.down` | Navigator chapter disclosure (closed / opened by her / open because current) |
| `expand.all` / `collapse.all` | — (no icons today) | Navigator ▸ Expand/Collapse All Chapters menu items |
| `unlocked`, `left.sidebar`, `full.screen`, `menu.expansion` | — | **Not used on Apple**: the system draws the sidebar button, full screen and toolbar overflow. Linux (Sprint 3). |

**Added 2026-11-24 — the Runic Whimsy set** (user, 2026-11-24: *"I added a new folder containing an additional set. These are
intended to replace the menu bar buttons."*): `Resources/Icons/Inkwyn_Runic_Whimsy_Glyphs/`, 8 glyphs in a NEW format — one SVG per
glyph holding `<g id="large|medium|small">`, each in its own 128-unit tile named by `<view id="…-view">`. ✅ The script reads it (each
group cropped to its tile) and names the set `inkwyn.menu.*` (its Close is different art from the Utility set's Close). **54
symbols**; the other 46 are byte-identical with or without the new folder; the Mac app builds with all 54.

| Glyph (`inkwyn.menu.…`) | Replaces (toolbar button) | Supersedes in the table above |
|---|---|---|
| `worlds` | Worlds (`globe`) | — |
| `navigate` | Navigate (`arrow.up.arrow.down`) | the Navigate menu's label |
| `create` | New (`plus`) | `add.create` on the New menu's label (Add Card / Add <Kind>… keep `add.create`) |
| `merge` | Merge (`arrow.triangle.merge`) | `merge.scene.with.previous` on the Merge menu's label |
| `view` | View (`eye`) | — |
| `close` | Close Project (`xmark`) | `close` on Close Project (a card's × and banners keep `close`) |
| `import.export` | Import **and** Export? (`square.and.arrow.down` / `…up`) | ⚠️ Q (e) |
| `tools` | ⚠️ which button? | ⚠️ Q (f) |

⚠️ **Size pips.** Merge, Create, Navigate, Tools, View, Worlds and Import-Export carry small diamonds to the right of the glyph —
**1 at Small, 2 at Medium, 3 at Large** — as do the earlier set's **Import** and **Export** ships. They look like the concept
sheet's size labels, not art; they also widen the drawing, so the glyph itself is fitted smaller. Q (d).
**For the artist:** Menu-Close is very thin at Small; Menu-Import-Export and Menu-Merge (Large) each have a tiny stray mark top-left.

**To rule with the table:** (a) `warnings` only on the world-warning surfaces (recommended), or for every inline ⚠️ too?
(b) the Inspector cards' own collapse chevron (`chevron.right/.down`) stays a system chevron (recommended) or takes the chapter glyphs?
(c) the Add Card menu has no icons at all today: give it the kind glyphs in Sprint 2, or leave it?
(d) the size pips: strip them in the script (recommended: no art edit, one rule — drop the diamonds right of the glyph), or does the
artist remove them?
(e) `menu.import.export`: one glyph on BOTH the Import and Export buttons, or merge the two into one Import/Export menu?
(f) `menu.tools`: which button — Project Settings (`gearshape`), Inkwyn Settings (`gearshape.2`), or a new Tools menu gathering
Inkwyn Settings, Purge Stale History Branches… and About?

## Live pass (2026-11-24)

**Run 1** (user, 2026-11-24: *"Step 5 failed. There is no icon on the Add Card menu in the worldbuilding tab on macOS. All other
steps passed."*) — and: *"compared to Maps and Locations and Buildings, my Characters icon looks very small … I really want my
glyphs to be bigger."*

- **Too small:** the box was 84×105 at 100 pt, so the narrow Characters portrait came out 76×84 of ink. Apple's kind symbols are
  ~90–113 tall and up to 175 wide (measured: `building.2` 96×113, `mappin.and.ellipse` 92×106, `person.2` 137×90). ✅ The box is now
  **106×140**: `inkwyn.characters` measures **96×106** in the built app, level with Locations and Buildings. All 46 regenerated.
- **Step 5:** `Label(_:glyph:)` built its icon with the `Label { } icon: { }` closure, and a macOS menu item takes its image only from
  the `Label(_:image:)` / `Label(_:systemImage:)` forms. ✅ It now uses those. ⚠️ Not checked by me: it needs the menu open on screen.
- Rebuilt macOS and iOS (no new warnings); Mac interop **355 passed** again.

**Run 2** (user, 2026-11-24: *"3: No it isn't. not even close. In fact I'm having difficulty seeing any difference in the size at
all."* · *"4: Not true. This menu still shows no icons at all."* · *"5: My icon art has a lot of detail, so maybe a 2x increase will
be sufficient."*)

- **Size:** +26% is not visible at caption size. ✅ The box is now **212×280**, twice Apple's kind symbols; `inkwyn.characters`
  measures **190×212** in the built app. Checked this time as the app draws it: an `ImageRenderer` of the card header (caption icon,
  callout title) beside Locations, Buildings and Maps shows the portrait clearly larger, its detail readable. ⚠️ Its header row is a
  little taller than the SF-symbol rows; Sprint 2 evens that out as the other kinds take their glyphs.
- **Step 4 was MY wrong expectation, not a defect:** the Add Card menu shows no icon for ANY card, SF Symbols included, and never
  has. I wrote the step without checking. The `Label(_:glyph:)` change stays (it is the standard form) but fixed nothing visible.
  Icons in that menu are not in this Sprint; they would be a mapping question for Sprint 2.
- Rebuilt macOS and iOS; Mac interop **355 passed**.

**Run 3** ✅ (user, 2026-11-24: *"Step 3 passes now"*). With run 1's passes (icon in the Dock and About, the glyph in dark mode, the
iPhone icon and card), the live pass is complete.

## Rulings on the mapping (user, 2026-11-24) — Sprint 2 builds from these

> *"The Diamond Pips are part of the glyph language. They indicate that a menu has a sub-menu. The intent is to put a single
> Import/Export button on the toolbar, which will have an import menu and an export menu. This would replace the two buttons
> currently on the toolbar. The directional ship icons are intended to be the icons for the import and export submenu items.
> Tools is intended to bring together project settings, inkwyn settings, purge stale history and about. Use the warnings glyph
> for every warning icon. replace system collapse with the small versions of the automatic chapter chevrons. add icons to the
> add card menu."* · *"the proposed mapping is approved."*

- **(d) Pips are art:** they mean *"this button opens a menu"*. ✅ Kept; nothing stripped. So a pipped glyph goes only on a button
  that opens a menu (Navigate, New, Merge, View, Import/Export, Tools, Worlds?). ⚠️ **Worlds** is a pipped glyph on a button that
  opens a SHEET today (Manage Worlds…), and **Import / Export** (pipped ships) go on submenu items — Sprint 2 to confirm.
- **(e) One Import/Export button** (`inkwyn.menu.import.export`) replaces the toolbar's Import and Export buttons; it opens an
  **Import** submenu (`inkwyn.import`) and an **Export** submenu (`inkwyn.export`), each holding today's items.
- **(f) One Tools button** (`inkwyn.menu.tools`) replaces Project Settings, Inkwyn Settings, Purge Stale History Branches… and
  About Inkwyn as toolbar items; it opens a menu of those four.
- **(a) `inkwyn.warnings` for EVERY warning icon** — `exclamationmark.triangle` and `.fill` at all ~21 sites, not only world warnings.
- **(b) The Inspector cards' collapse control** takes the chapter chevrons at **Small** scale: `inkwyn.chapter.collapsed` when
  collapsed, `inkwyn.chapter.expanded.automatic` when expanded (the Navigator keeps all three).
- **(c) The Add Card menu gets icons** — the kind glyphs. ⚠️ It shows NO icon today, SF Symbols included (run 2), so Sprint 2 must
  first find why a macOS menu item there draws no image.
- ✅ **Worlds drops its pips** (user, 2026-11-24: *"drop the pips for worlds."*): it opens a sheet, not a menu. The script draws
  `inkwyn.menu.worlds` without them (`withoutPips`, by geometry: a diamond under 12 wide at x ≥ 105 of the 128 tile). Checked: of
  all 54 symbols, only `inkwyn.menu.worlds` differs from a run without the rule. Mac app rebuilt.
- ✅ **The toolbar changes shape on EVERY platform** (user, 2026-11-24: *"yes, we're changing the shape of the toolbar. making it a
  bit smaller and better organized. same for all platforms."*): one Import/Export and one Tools button, on the Mac, iOS and Linux.

## Retrospective

- **What worked:** proving the format on one glyph first; a self-checking converter with no dependencies; a contact sheet before any UI.
- **What cost runs:** two live-pass runs on my own mistakes — a size change judged from numbers instead of a render at the app's size,
  and a step ("as the other cards show theirs") whose premise I never checked. Both are now a standing rule (render visual changes as
  the app draws them; check a step's premise first).
- **Carried to Sprint 2:** everything in the approved mapping and rulings (a)–(f), including the toolbar reshape.
