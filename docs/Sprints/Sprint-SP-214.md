---
sprint: SP-214
epic: EP-054
status: Active
platform: Cross
created: 2026-11-12
---

# SP-214 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S5b**: PDF export

**Status:** 🟡 **ACTIVE** 2026-11-12 (user, 2026-11-12: *"Q1–Q5 as recommended, activate and implement SP-214."*) · ✅ Q1–Q5 ruled as recommended · planned 2026-11-12 (*"plan EP-054 S5b."*).
**Carries:** [T-0675] → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-207] (S3: the walker, the HTML writer, Export Manuscript…), [SP-208] (S4: the writer's typography, the
scope question), [SP-212] (the byline), [SP-213] (S5: the ePub stylesheet; **its Q1 split PDF out to here, and its Q2–Q4 are
ruled and carried**).
**Authority:** EP-054 AC5 (PDF: chapters, scene breaks and emphasis preserved), Q6 (PDF on each platform from the HTML), AC9.
**Size:** M–L: one core option, then three platform renderers, each its own API.

---

## Carried rulings (SP-213, user 2026-11-12)

- **Q2 (a)** — the core writes a styled **print page** (HTML + CSS); each platform paginates it: Apple with WebKit, Linux with
  `QTextDocument` + `QPdfWriter` (Qt GUI, already linked). The page keeps to what both engines read.
- **Q3** — the system's default paper; 1-inch (2.54 cm) margins; **page numbers bottom centre, none on the title page**; each
  chapter on a new page. ⚠️ A platform that cannot number pages without a date or file-name header is surfaced, not shipped.
- **Q4** — the writer's typography (face, size, indent rule, the screen's 1.45 line height); **the face embedded**.
- **SP-213 Q7** — PDF: **the whole manuscript or the caret's chapter**, as `.docx`.

## What reading the code and Apple's documentation found (2026-11-12)

- ✅ **The core already has the switch:** `scrivi_export_manuscript` takes `document: "page" | "body"` for HTML
  (`scrivi_c_api.cpp:1271`, `:1282`); a third value `"print"` is the print page. The ePub stylesheet (`epubStyle`,
  `ManuscriptExport.cpp:613`) already turns the writer's typography into CSS — size, 1.45 line height, the three indent rules,
  the heading scale, the scene-break rule.
- ⚠️ **`QTextDocument` reads a CSS subset.** It has `page-break-before`, `text-indent`, margins, `line-height`, `font-family`;
  it is not known to read sibling selectors (`p + p`, which the ePub's book indent uses). **The print page sets classes on the
  elements themselves** (e.g. `<p class="first">`), no combinators, so both engines read the same rules.
- ✅ **Apple APIs, from Apple's documentation (fetched 2026-11-12, `tutorials/data/documentation/…json`):**
  - macOS: `WKWebView.printOperation(with:)` (macOS 11) paginates; `NSPrintInfo` `jobDisposition = .save` writes it to a file.
    ⚠️ `NSPrintInfo.AttributeKey.headerAndFooter` adds **"a standard header and footer"** (title, date) — exactly what Q3
    refuses. So the Mac's page numbers must come from somewhere else → **Q1**.
  - iOS / iPadOS: `UIPrintPageRenderer` with the web view's `viewPrintFormatter()`; its footer draw method numbers pages
    the way we choose, rendered into a PDF context → **Q1**.
  - `WKWebView.createPDF(configuration:)` makes **one tall page**, not pages, so it is no use here.
  - PDFKit: `PDFPage.draw(with:to:)` (macOS 10.12, iOS 11) can draw onto pages already laid out.
- ⚠️ **SP-213's finding that "a PDF made in-process embeds the fonts it uses" is FALSE for WebKit.** WebKit lays out in its
  own web-content process; the bundled fonts the app registers with `CTFontManagerRegisterFontURLs`
  (`ManuscriptTypography.swift:80`) are registered for the **app's** process, so WebKit falls back to a system face.
  Linux is unaffected (`QTextDocument` draws in-process; `addApplicationFont`, `ManuscriptTypography.cpp:62`) → **Q2**.
- **Paper:** macOS has the default paper (`NSPrintInfo.shared.paperSize`). iPadOS and Linux (without QtPrintSupport) have no
  printer default to read → **Q3**.
- **Readers and tools:** scrivi-vm has `pdffonts`, `pdftotext` (poppler) and Papers; the Mac has Preview and PDFKit; none
  of the poppler tools are on the Mac.
- ⚠️ **A PDF is not byte-identical across platforms** (WebKit and Qt lay out differently) — no "same bytes" criterion here.

## Goal

A writer can take the manuscript — or the caret's chapter — out as a **PDF in their own typeface**, on their paper, with a
title page and byline, each chapter on a new page, scene breaks and emphasis as in Scrivi, and page numbers; from the Mac,
iPad and Linux.

## Plan

0. **Red tests first** (`ManuscriptExportCApiTests.cpp`, through `scrivi_*`): `format: "html"`, `document: "print"` against a
   golden file — the title page (title, subtitle, byline), each chapter starting a new page, the classes for the indent rule,
   the stylesheet from the writer's typography; the chapter scope (no title page).
1. **Core — the print page:** `document: "print"` beside `page` and `body` in `exportHtml`; the stylesheet derived from
   `epubStyle`'s values (one source of the numbers), plus `page-break-before` for chapters and the title page's own page;
   classes, no combinators; the face named (Q4 of SP-213).
2. **Wrappers:** `ScriviEngine.exportManuscript(document:)` (+ the visionOS stub, `check-engine-stub-parity.sh`);
   `ScriviBridge` takes the option.
3. **Mac:** the print page in an off-screen `WKWebView` → `printOperation(with:)` with headers and footers off, paper per Q3,
   1-inch margins, saved to a partial file → page numbers per Q1 → renamed into place. Main thread for WebKit, but the load
   and layout are WebKit's (async); progress after 400 ms.
4. **iPad:** the print page in a `WKWebView` → `UIPrintPageRenderer` + `viewPrintFormatter()`, page numbers per Q1, into a PDF
   context → the share sheet as `.docx` does.
5. **Linux:** `QTextDocument` (the print page as HTML) → `QPdfWriter` with paper per Q3 and 1-inch margins; pages drawn one by
   one so the footer is ours (number centred, none on the title page). In a worker (`AsyncCall`), progress after 400 ms;
   ⚠️ if Qt objects to font or painter use off the GUI thread, that is surfaced and the step runs on the main thread with
   progress.
6. **Surfaces** (Linux in the Mac's shape and wording): Export Manuscript…'s format prompt gains **PDF**; then the scope
   question (whole / the caret's chapter, as `.docx`); then the save panel / share sheet.
7. **Checks:** macOS and Linux ctest; interop test (a PDF of the fixture: page count > 1, page 1 has no number, page 2 does,
   via PDFKit); Linux smoke (the PDF written, `%PDF-` and the page count); on scrivi-vm, `pdffonts` shows the writer's face
   **embedded** and `pdftotext` shows the numbers and chapter starts — the same checks on a Mac PDF copied there.
8. **Live passes (user):** a whole-manuscript and a one-chapter PDF in Preview (Mac), Files (iPad) and Papers (Linux): the
   writer's face, the title page and byline, chapters on new pages, scene breaks, emphasis, lists, page numbers bottom centre
   and none on the title page, the paper size.

## Questions

| # | Question | Ruling |
| - | -------- | -------------- |
| **Q1** | **The page numbers on Apple.** WebKit's own header and footer adds a date and title (Q3 refuses). (a) **Mac:** print with headers and footers off, then draw each number onto the finished pages with PDFKit (`PDFPage.draw(with:to:)`); **iPad:** `UIPrintPageRenderer`'s footer draws them as the pages are made. (b) Both with PDFKit after the fact, one code path. (c) CSS page-margin boxes (`@page { @bottom-center { content: counter(page) } }`) — not known to be supported by WebKit; would be tried first and dropped if not. | ✅ **RULED as recommended (user, 2026-11-12).** **(a).** Each platform uses its documented way to own a footer; the iPad needs no second pass. ⚠️ If (a)'s Mac pass loses the PDF's text or links, that is surfaced. |
| **Q2** | **The writer's face inside WebKit** (bundled fonts are invisible to WebKit's process). (a) the **host** adds an `@font-face` rule to the print page pointing at the bundled font file (read access granted with `loadFileURL(_:allowingReadAccessTo:)`), for the bundled faces only; system faces need nothing; (b) the same, but the font file inlined as a `data:` URL (no file access; a few hundred KB per face in memory); (c) the core writes `@font-face` (it would have to know app bundle paths — a platform fact in the core). | ✅ **RULED as recommended (user, 2026-11-12).** **(a).** The face stays a platform fact, the core page stays the same on every platform, and Linux needs nothing. (b) is the fallback if WebKit refuses the file access. |
| **Q3** | **"The system's default paper" where there is none to read.** Mac: `NSPrintInfo.shared.paperSize`. iPad and Linux: (a) **from the region** — US Letter where the region measures in US units (`Locale.measurementSystem == .us`; `QLocale::measurementSystem() == ImperialUSSystem`), A4 elsewhere; (b) Linux links QtPrintSupport for the default printer's paper (a new module, and a machine without a printer still has none). | ✅ **RULED as recommended (user, 2026-11-12).** **(a).** The same answer on both, no new module; it is the paper a writer in that region buys. |
| **Q4** | **Numbering.** (a) **The first chapter's first page is page 1**; the title page counts nothing (book convention: front matter); (b) the title page is page 1, its number hidden, so text starts on 2. A one-chapter PDF has no title page and starts at 1 either way. | ✅ **RULED as recommended (user, 2026-11-12).** **(a).** A writer reading "page 1" expects their opening line. |
| **Q5** | **The scene break.** (a) **The rule, as in Scrivi and the ePub** (`<hr />`, inset); ⚠️ if `QTextDocument` cannot inset it, a full-width rule on Linux; (b) a centred `#` (Standard Manuscript Format's mark — but S4 Q1 ruled SMF out). | ✅ **RULED as recommended (user, 2026-11-12).** **(a).** One look across export formats; the Linux limit, if real, is recorded at the AC. |

⛔ **NOT in this Sprint:** any import (S6–S7), print to a printer (File ▸ Print), a page-size or margin chooser, running
headers (title / author on each page), Standard Manuscript Format (S4 Q1), a table of contents page, cover image, the ePub.

## Acceptance Criteria

- [x] AC1 — `scrivi_export_manuscript` `format: "html"`, `document: "print"`: the title page with the byline (whole
      manuscript only), each chapter on a new page, the writer's typography as classes both engines read, scene breaks per Q5;
      golden-file boundary tests through `scrivi_*`; macOS and Linux ctest green
- [ ] AC2 — Mac and iPad write a PDF through WebKit: the writer's face **embedded** (Q2), the paper (Q3), 1-inch margins,
      page numbers bottom centre from the first chapter page, none on the title page (Q1, Q4), no date or title header;
      interop test through the engine — ✅ Mac; ⏳ iPad: its check is the live pass (user, 2026-11-12: *"I'll check the iPad on
      the live pass"*)
- [x] AC3 — Linux writes the same through `QTextDocument` + `QPdfWriter`; Linux smoke through the bridge; on scrivi-vm
      `pdffonts` shows the face embedded and `pdftotext` the numbers, for a Linux PDF and a Mac PDF
- [x] AC4 — Export Manuscript… offers **PDF** on the Mac, iPad and Linux, then whole / the caret's chapter; flushed first,
      off the main thread where the platform allows, progress after 400 ms; engine stub parity green
- [ ] AC5 — **Live passes (user):** a whole-manuscript and a one-chapter PDF in Preview (Mac), Files (iPad) and Papers
      (Linux) — face, title page and byline, chapters on new pages, scene breaks, emphasis, lists, page numbers, paper

## Progress log

- **2026-11-12** — 🔵 **Created in Planning** (user, 2026-11-12: *"plan EP-054 S5b."*). Q1–Q5 to rule; SP-213's Q2–Q4 carried
  as ruled. ⚠️ SP-213's font-embedding finding corrected for WebKit (→ Q2).
- **2026-11-12** — 🟡 **Activated, Q1–Q5 ruled as recommended** (user, 2026-11-12: *"Q1–Q5 as recommended, activate and implement SP-214."*).
- **2026-11-12** — ✅ **Built** — AC1, AC3, AC4 met; AC2 met on the Mac — the iPad builds, its PDF is checked in the live pass.
  **Core:** `exportPrint` / `printStyle` (`ManuscriptExport.cpp:711`, `:735`), `document: "print"` + `typeface` in the result
  (`scrivi_c_api.cpp:1362`, `scrivi.h`); 2 boundary cases (19 / 919 assertions).
  **Apple:** `Scrivi/App/ManuscriptPDF.swift` (new, in the pbxproj for the Mac, iOS and visionOS targets) — an off-screen
  `WKWebView`; Mac: `printOperation(with:)` saved with headers and footers off, then each page redrawn with its number
  (Q1); iPad: `UIPrintPageRenderer` with our footer into `UIGraphicsPDFRenderer`; Q2's `@font-face` rules on the bundle's
  font files, `loadHTMLString(baseURL: Fonts/)`. Export Manuscript… ▸ **PDF** (`ProjectArchiveFlow.swift:140`).
  **Linux:** `platforms/linux/src/ManuscriptPdf.hpp` (new, header-only so none of the 14 targets that compile the shell
  changes) — `QTextDocument` paginated onto `QPdfWriter` page by page, our number in the foot; in the export's worker thread
  (`EditorShell.cpp:4621`). `settings_smoke` §5 (`settings_smoke.cpp:428`).
  ⚠️ **What QTextDocument does not read (found by probing Qt 6.4 on scrivi-vm, so the plan's CSS assumption was wrong):**
  a length in **pt** is dropped (margins, indents) — px works; a `line-height` in **%** is of the font's own line spacing,
  not its size (lines came out ~1.9×) — px is exact in both; and a heading's built-in size step overrides its CSS size —
  the Linux writer clears it after parsing. So the page's lengths and line heights are **px** (1/96 in, the same in both
  engines; the Linux writer works at 96 dpi), font sizes pt.
  ⚠️ **Bold on Linux (Qt 6.4):** the bundled variable faces expose only Regular and Italic to Qt
  (`QFontDatabase::styles("Literata")` = Italic, Regular), so headings and **bold** print in the regular weight on Linux.
  The Mac's PDF is bold. Not this Sprint's code — a Qt 6.4 font limit; raised with the user, not filed.
  **Evidence:** macOS ctest **842/842**; interop `manuscriptPDFThroughEngine` + `manuscriptExportThroughEngine` passed (2
  pages, title page unnumbered, "1" on the chapter page, Literata embedded, no WebKit header); iOS build succeeded; stub
  parity, ABI guard and package boundary clean. scrivi-vm: ctest **847/847**; `settings_smoke` pass (§5: Newsreader embedded,
  2 pages); `bridge_parity_smoke` 74 / 0. A copy of `the-river-of-time` through the core and the Linux writer: 4 pages,
  Letter, `pdffonts` Literata embedded, `pdftotext` numbers 1–3 and none on the title page, chapters on new pages; a Mac
  sample: Letter, Literata (regular, italic) embedded, numbers 1–6. ⏳ AC5: the live passes.
