# Resources/Fonts — the manuscript typefaces Scrivi bundles

✅ **THE BUNDLED SET — chosen by the user 2026-10-07** (EP-047, ruling P5) from 28 candidates: Scrivi ships these seven and
uses ONLY these for the manuscript (no installed fonts, so a manuscript looks the same on every Mac, iPhone, iPad and Linux
machine). Specimen: [`../../docs/specimens/Scrivi-Typeface-Specimen.pdf`](../../docs/specimens/Scrivi-Typeface-Specimen.pdf).
Nothing builds or bundles these files yet — that is EP-047 S2 (Apple) and EP-048 L10 (Linux).

| Face | Role | Files | Weights |
| ---- | ---- | ----- | ------- |
| **Literata** | Serif — the DEFAULT (P6) | roman + italic, variable (opsz, wght) | 200–900 |
| **Newsreader** | Serif | roman + italic, variable (opsz, wght) | 200–800 |
| **Crimson Pro** | Serif | roman + italic, variable (wght) | 200–900 |
| **Inter** | Sans | roman + italic, variable (opsz, wght) | 100–900 |
| **Figtree** | Sans | roman + italic, variable (wght) | 300–900 |
| **Source Code Pro** | Monospaced | roman + italic, variable (wght) | 200–900 |
| **Courier Prime** | Manuscript submission | Regular, Italic, Bold, BoldItalic (static) | 400, 700 — ⚠️ bold inside a heading cannot be heavier than the heading |

## Licensing

✅ **Every family is under the SIL Open Font License 1.1** — confirmed 2026-10-07 from the license string EMBEDDED IN EACH FONT
FILE (Core Text `kCTFontLicenseNameKey`), not only from catalogue metadata. Each folder holds the family's `OFL.txt`, which
**must ship with the font files** (OFL §2). Bundling inside an app — including a sold one — is permitted;
selling the fonts by themselves is not.

⚠️ **Reserved Font Name — Source Code Pro: "Source"** (Adobe; read from its `OFL.txt`). Bundling it UNMODIFIED is fine; a
modified version (subset, converted, renamed instance) must not be called "Source". ✅ The other six declare none (checked).

## Provenance (fetched 2026-10-07)

All seven from [google/fonts](https://github.com/google/fonts) `ofl/<family>/`: the `OFL.txt` files and the later files pinned at
`5e8a3ba899557829a76cfdac30fa512bda91d7ca`; the first-fetched `.ttf` (Literata, Newsreader, Crimson Pro, Inter, Courier Prime) from
`main` the same day. Per-family files, styles and weights: [`../../docs/specimens/specimen-fonts.json`](../../docs/specimens/specimen-fonts.json).
(The 21 candidates not chosen — and what was learned about them: Tinos's 2026 relicense to OFL, the malformed glyph in iA Writer
Duo/Quattro's variable italics — are recorded in EP-047.)

## Platform notes (measured or fetched, 2026-10-07)

- **Apple (macOS, iOS, iPadOS, visionOS):** registered at launch with `CTFontManagerRegisterFontURLs(_:.process:…)` —
  macOS 10.15+, iOS 13+, visionOS 1+ (Apple docs). Core Text selects variable weights and optical sizes directly (the specimen
  was drawn this way).
- **Linux (Qt):** `QFontDatabase::addApplicationFont`. ⚠️ Qt 6.4's handling of VARIABLE fonts is not measured; if it cannot
  select weights, static instances are generated from these sources (EP-048 L10 decides).
