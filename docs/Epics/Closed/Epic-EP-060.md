# Epic EP-060 — CLOSED

## EP-060: `[Cross]` **Inkwyn Look and Feel** — the Inkwyn app icon and glyph set on every platform

**Status:** ✅ **CLOSED** 2026-11-24 (user, 2026-11-24: *"Close EP-060, delete the orphan file"*). Was 🟠 Complete 2026-11-24.
**Actual Close Date:** 2026-11-24
**Goal:** Inkwyn looks like one app. The Inkwyn app icon replaces the Scrivi one, and wherever the Inkwyn glyph set has
a glyph for a panel, toolbar verb or object kind, the app shows it — drawn from the SVG source, sharp at every size —
in place of the generic SF Symbol (Apple) or Qt standard icon (Linux).
**Date Created:** 2026-11-24
**Tag:** `[Cross]` by the user's ask (2026-11-24: *"a new Epic. Then we can incorporate icons per platform … in different
Sprints"*) — ⚠️ the Epic guidelines say one Epic per codebase; this follows EP-058/EP-059's one-Epic, Sprint-per-platform shape.

### The source art (`Resources/`, untracked at creation)

- **App icon:** `Resources/AppIcon/Inkwyn-AppIcon.png` — 1254×1254, fully opaque (square, full-bleed). PNG only, no layers.
- **Glyphs:** `Resources/Icons/` — **47 glyphs × Large/Medium/Small = 141 SVGs** (46 distinct: the two Timelines are the same art) (+ PNGs, + 6 `*_Specification.md`).
  Ancillary 7 · Create/Merge 4 · Import/Export 6 · Lock 2 · Navigate 6 · Utility 3 · View 9 · World kinds 10
  (Artifact, Item, Buildings, Characters, Chronicles, Factions, Locations, Maps, Rules, Vehicles).
- ✅ **Large/Medium/Small are OPTICAL variants, not scales** — Small has heavier strokes and simpler shapes
  (e.g. Add-Create-Small is a plain `+`, Large a four-point star). They map onto SF Symbols' symbol scales.
- ✅ Pure vector: no embedded rasters, no text. Monochrome: `currentColor` or `#000000` only — both tint as a template.
- ✅ Renders on macOS: `NSImage(contentsOfFile:)` drew Add-Create-Small and Characters-Large correctly (probe, 2026-11-24).
- ⚠️ **Canvases are inconsistent:** 30+ distinct viewBoxes, many non-square (Artifact 325×213, one 622×211). At one point
  size the glyphs will NOT read as the same size unless normalised (Q5).
- ⚠️ Folder layout is inconsistent (flat; `SVG/` + `PNG/`; one folder per glyph) — any import script walks the tree.

### What it replaces (2026-11-24 counts)

- **Apple:** 124 `systemName:`/`systemImage:` call sites, ~70 distinct SF Symbols in `Scrivi/`. Many have no Inkwyn
  equivalent (`checkmark`, `magnifyingglass`, `gearshape`, `exclamationmark.triangle` vs the View ▸ Warnings glyph …).
  App icon: `Scrivi/Assets.xcassets` — `AppIconMacOS`, `AppIconIOS`, `AppIconVisionOS.solidimagestack` (Scrivi art).
- **Linux:** 5 `QStyle::standardIcon` uses; no icon resources. ✅ The pattern exists: `qt_add_resources` already bundles
  `Resources/Fonts` (`platforms/linux/CMakeLists.txt:54`). ⚠️ `Qt6::Svg` is NOT linked yet.

### Questions to rule

| Q | Question | Recommendation |
|---|---|---|
| **Q1** | **Apple: how the SVGs enter the asset catalog.** (a) An imageset per glyph per size, SVG with *Preserve Vector Data*, *Template*. (b) A **custom SF Symbol** (`.symbolset`) per glyph, Small/Medium/Large as its three scales. | **(b).** It drops into the 124 call sites as `Image("name")`/`Label(…, image:)` and sizes with the font, weight and `imageScale` like today's symbols, in menus and toolbars too. (a) shows an SVG at its natural size (100–600 pt here) unless every call site sizes it, and the size choice is by hand. ⚠️ (b) needs each SVG converted to Apple's symbol template (strokes outlined, margins) — **Sprint 1 spikes ONE glyph end to end before converting 47**; fallback (a). |
| **Q2** | **The app icon on macOS/iOS.** (a) Replace the PNGs in the two existing appiconsets. (b) An **Icon Composer `.icon`** with the PNG as its one layer. | **(b).** macOS/iOS 26+ icons are Icon Composer documents (Liquid Glass, light/dark/tinted); one file serves both. Layers later if the art is split. ⚠️ Downscale 1254 → 1024. |
| **Q3** | **visionOS app icon.** | **Out of scope** — visionOS is not built (I-0356). Replace the Scrivi art in its layers only when it is. |
| **Q4** | **Which glyph replaces which symbol.** | Sprint 1 writes **one mapping table** (glyph ↔ SF Symbol ↔ Qt icon ↔ call sites) for approval. ⛔ No glyph where the set has none: unmatched places keep the system symbol. |
| **Q5** | **The inconsistent canvases.** | Normalise in the conversion (fit each glyph to one cap-height/box), never by editing the art. Anything that still looks wrong goes back to the artist. |
| **Q6** | **Linux.** | `Qt6::Svg` + a `qt_add_resources` of the SVGs (like the fonts) + one loader that picks Small/Medium/Large by size and tints to the palette's text colour (the `#000000` files won't follow a dark theme otherwise). Window icon now; the installed `.desktop` icon belongs to [EP-056]. ⚠️ `libqt6svg6` becomes a runtime dependency (Docker, VM, rig, `.deb`). |
| **Q7** | **Windows.** | **Out of scope** — no Windows app yet. |

### Sprints

| Sprint | Title | Status |
|---|---|---|
| [SP-241] | `[Apple]` S1: the glyph pipeline and the Inkwyn app icon ([T-0704]) | ✅ Closed 2026-11-24 → [`../Sprints/Closed/Sprint-SP-241.md`](../../Sprints/Closed/Sprint-SP-241.md) |
| [SP-242] | `[Apple]` S2: the glyphs in the app, and the smaller toolbar ([T-0705]) | ✅ Closed 2026-11-24 → [`../Sprints/Closed/Sprint-SP-242.md`](../../Sprints/Closed/Sprint-SP-242.md) |
| [SP-243] | `[Linux]` S3: the Inkwyn glyphs and the 8-item toolbar on Linux ([T-0706]) | ✅ Closed 2026-11-24 → [`../Sprints/Closed/Sprint-SP-243.md`](../../Sprints/Closed/Sprint-SP-243.md) |

Still to plan:

1. ~~**[Apple] the pipeline and the icon**~~ = [SP-241] — the Q1 spike, the import script, the Q4 mapping table, the app icon.
2. ~~**[Apple] the glyphs**~~ = [SP-242] — swap the mapped call sites on macOS and iOS (one shared catalog), per [SP-241]'s approved mapping and rulings (a)–(f): ⚠️ including ONE Import/Export and ONE Tools toolbar button in place of six, the warnings glyph everywhere, and icons in Add Card.
3. ~~**[Linux] the glyphs and the window icon**~~ = [SP-243] — — Q6, and the same toolbar shape as Sprint 2.

### Acceptance Criteria (draft)

- [x] **AC1** The Mac and iOS apps show the Inkwyn app icon (Dock, Finder, Home Screen, App Switcher); no Scrivi icon art remains for macOS or iOS (visionOS's stays until visionOS is built: Q3, [SP-241] Q7).
- [x] **AC2** Every glyph in the approved mapping (Q4) is shown at every mapped place on the Mac and iOS, sharp at 1×/2×/3× and in light and dark.
- [x] **AC3** Small/Medium/Large are chosen by size, not by hand at each call site.
- [x] **AC4** Linux shows the same mapped glyphs and the Inkwyn window icon, legible in light and dark themes.
- [x] **AC5** The glyphs are built from the SVG sources in `Resources/Icons/` by a repeatable step — re-running it after the artist changes a file updates every platform.

---

### Completion Summary (2026-11-24)

**Delivered:** Inkwyn looks like one app on the Mac, iPad, iPhone and Linux.
- **The app icon** — an Icon Composer `Inkwyn.icon` for the Mac and iOS ([SP-241]); the bundled Inkwyn icon as the Linux window icon ([SP-243]).
- **One glyph pipeline** — `scripts/make-glyph-symbols.swift` turns `Resources/Icons` (47 glyphs + the 8-glyph Runic Whimsy set → 54
  symbols) into custom SF Symbols for Apple and plain SVGs for Linux: strokes outlined, knock-outs, crops, sprite groups, pips dropped
  where a button opens no menu (Worlds, Close), stray specks dropped, 1.5× Apple's size; self-checked; re-runs byte-identical.
- **The toolbar reshaped on every platform** — 8 items, View▾ leading with the panels, one Import/Export▾ and one Tools▾, Large
  aspect-fit glyphs, each menu opening with a dismissing header ([SP-242], [SP-243]); on the iPhone the header steps aside in the ⋯
  overflow, which shows its own ([I-0373]).
- **Every mapped place on its glyph** — the 10 kinds, warnings everywhere, utility glyphs, chapter chevrons, the tree tab — on Apple
  ([SP-242]) and Linux ([SP-243], `GlyphIcon`: palette-tinted, HiDPI on Qt 6.4 and 6.10).

**Rulings along the way:** the mapping and (a)–(f) ([SP-241]); 2× then 1.5×; Large in the Mac toolbar; Worlds without pips; the
toolbar's shape "same for all platforms" (user, 2026-11-24).

**Not done here:** visionOS's icon stack (visionOS is not built — Q3); Windows (no app — Q7); the installed Linux `.desktop` icon and
the `.deb`'s `libqt6svg6` dependency belong to [EP-056] (recorded there).

### Close (2026-11-24)

Audit Check: **F1** (the Sprint table still showed [SP-243] Active) fixed in [SP-243]'s close; **F2** (`docs/Sprints/Rig-Live-Pass-Checklist-2026-11-02.md`,
an orphan from EP-057's 2026-11-02 rig pass, pre-existing) **deleted** (user, 2026-11-24: *"Close EP-060, delete the orphan file"*). Its results live in T-0648–T-0655's archives. ✅ The
13 references to it — links in those 8 Tasks, mentions in `Closed/Sprint-SP-195/196/197/200/201.md` — removed (user, 2026-11-24: *"Remove the dead links"*).

