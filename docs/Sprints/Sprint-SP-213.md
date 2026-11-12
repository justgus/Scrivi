---
sprint: SP-213
epic: EP-054
status: Active
platform: Cross
created: 2026-11-12
---

# SP-213 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S5**: ePub export

**Status:** 🟡 **ACTIVE** 2026-11-12 (user, 2026-11-12: *"Q1–Q8 as recommended, activate and implement SP-213."*) · ✅ Q1–Q8 ruled as recommended — **Q1: ePub here; PDF its own Sprint**, carrying Q2–Q4 as ruled · planned 2026-11-12 (*"plan EP-054 S5."*).
**Carries:** [T-0674] → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-207] (S3: `scrivi_export_manuscript`, the walker, the HTML writer, Export Manuscript… on three platforms),
[SP-208] (S4: miniz zips at a fixed time, the writer's typography read from `project-settings.json`), [SP-212] (the byline).
**Authority:** EP-054 AC5 (PDF and ePub, chapters, scene breaks and emphasis preserved), Q5 (PDF and ePub after `.docx`),
Q6 (ePub in the core; **PDF on each platform from the HTML** — page layout is a platform strength), AC9 (the byline).
**Size:** M after Q1 (ePub: a core writer and one more choice in the prompt). PDF: its own Sprint (Q1).

---

## What reading the code found (2026-11-12)

- **ePub is S4's shape again:** a zip of XHTML, CSS and XML. The S3 walker's HTML is already well-formed XML (`<br />`,
  `<hr />`, `&amp;`/`&lt;`/`&gt;` escaped, `ManuscriptExport.cpp` `sceneHtml`), so it is the chapter body as it stands.
  miniz writes it; ⚠️ an ePub's first entry must be `mimetype`, **stored uncompressed** — `mz_zip_writer_add_mem_ex_v2`
  already takes the level (`ManuscriptExport.cpp:567`).
- ⚠️ **EPUB 3 requires two things the package does not hold:**
  - `dc:language` — Scrivi has **no language setting** anywhere (no `language` key in the core or the schemas) → Q6.
  - `dcterms:modified`, a timestamp — which breaks S4's "the same bytes on every export" → Q6.
- **PDF has no code on any platform:** no `NSPrintOperation`, `WKWebView`, `UIPrintPageRenderer`, `QPrinter` or
  `QPdfWriter` anywhere. Linux links Qt Quick, Widgets, QuickWidgets, Test and Concurrent; `QPdfWriter` and
  `QTextDocument` are in Qt GUI (already linked), QtWebEngine is not.
- ⚠️ **Epic Q6 says "PDFKit"; PDFKit reads and annotates PDFs, it does not lay out HTML.** On Apple the HTML-to-pages path is
  WebKit's printing. The exact API (macOS and iOS differ) is to be confirmed against Apple's documentation (fetched, per
  the UI-conformance study's §0 route) before it is written → Q2.
- **The S3 HTML page carries no style** — by ruling, it is the AO3 paste. A PDF needs the writer's typography, a title
  page and chapters on new pages, so PDF needs its own page → Q2.
- ✅ **Unlike `.docx`, a PDF can carry the writer's face:** both apps register the bundled fonts in-process
  (`ManuscriptTypography.swift:80` `CTFontManagerRegisterFontURLs`; Linux `ManuscriptTypography.cpp:62`
  `addApplicationFont`), and a PDF made in-process embeds the fonts it uses → Q4.
- **Validators and readers on hand:** Java 11 is on this Mac, so **epubcheck** (the W3C's ePub validator, a jar) can run
  locally; Apple Books on the Mac and iPad; Preview / Pages for PDF. ⚠️ **scrivi-vm has no ePub or PDF reader installed**
  → Q8.
- ⚠️ **A PDF cannot be byte-identical across platforms** (WebKit and Qt lay out differently); the "same bytes" criterion
  holds for ePub only.

## Goal

A writer can take the manuscript out as an **ePub** that passes epubcheck and reads correctly in Apple Books and other
readers — title page with byline, a contents list of chapters, chapters, scene breaks and emphasis as in Scrivi — and as a
**PDF** in their own typeface, paginated by the platform, from the Mac, iPad and Linux.

## Plan

0. **Red tests first** (`ManuscriptExportCApiTests.cpp`, through `scrivi_*`): the ePub unzipped — `mimetype` first and
   stored, `container.xml`, the package document's metadata (title, authors in order, identifier, language), the nav, one
   chapter's XHTML against a golden file, the stylesheet; the same bytes twice (Q6); the print page against a golden file.
1. **Core — ePub** (`ManuscriptExport.cpp`, beside `.docx`): `format: "epub"` + `destPath`, written as S4 writes `.docx`;
   structure per Q5, metadata per Q6, scope per Q7.
2. **Core — the print page** (Q2): `format: "html"`, `document: "print"` — the S3 page plus a stylesheet from the writer's
   typography, the title page with the byline, chapters on new pages, the scene-break rule.
3. **Wrappers:** `ScriviEngine.exportManuscript` (+ the visionOS stub, `check-engine-stub-parity.sh`); `ScriviBridge`.
4. **PDF on each platform** (Q2–Q4): the Mac and iPad lay out the print page with WebKit and save the PDF; Linux with
   `QTextDocument` → `QPdfWriter`. Off the main thread where the platform allows; progress after 400 ms.
5. **Surfaces** (Mac, iPad, Linux — Linux in the Mac's shape and wording): Export Manuscript…'s format prompt gains
   **ePub** and **PDF**; then scope (PDF only, Q7); then the save panel / share sheet.
6. **Checks:** macOS and Linux ctest; the ePub's SHA-256 equal on both; **epubcheck** clean on the test fixture and on a
   real project (Q8).
7. **Live passes (user):** an ePub in Apple Books (Mac, iPad) and a reader on Linux; a whole-manuscript and a one-chapter
   PDF in Preview (Mac), Files (iPad) and a viewer on Linux — the writer's face, title page and byline, chapters on new
   pages, scene breaks, emphasis, lists, page numbers.

## Questions

| # | Question | Ruling |
| - | -------- | -------------- |
| **Q1** | **One Sprint or two?** ePub is S4's shape (core writer + one more choice in the prompt); PDF is three platform renderers, each its own API. | ✅ **RULED as recommended (user, 2026-11-12).** **Two.** This Sprint (SP-213) does **ePub** (plan steps 0–1, 3, 5–7 for ePub); **PDF** follows as its own Sprint with steps 2 and 4, so a renderer problem on one platform never holds the ePub. If ruled, Q2–Q4 move with PDF. |
| **Q2** | **Where the PDF's layout comes from.** (a) the core writes a styled **print page** (HTML + CSS: the writer's typography, title page, chapters on new pages) and each platform paginates it — Apple with WebKit's printing, Linux with `QTextDocument` + `QPdfWriter` (Qt GUI, already linked); (b) Linux adds QtWebEngine so both use a browser engine (a large new dependency, and not on Qt 6.4's rig set); (c) the core writes PDF itself (a PDF library, font embedding and text shaping in C++ — what Epic Q6 ruled out). | ✅ **RULED as recommended (user, 2026-11-12).** **(a).** One description of the page, in the core, as Q6 intends; each platform does only the pagination it is good at. ⚠️ `QTextDocument` reads a CSS **subset** (it has `page-break-before`, `text-indent`, margins, `<hr>`); the print page keeps to what both engines read, and plan step 2 lists it. |
| **Q3** | **The page.** | ✅ **RULED as recommended (user, 2026-11-12).** **The system's default paper** (Letter in the US, A4 elsewhere, from the platform's print settings); 1-inch (2.54 cm) margins; **page numbers** bottom centre, **none on the title page**; each chapter on a new page (S4 Q3). ⚠️ If one platform's printing cannot number pages without also printing a date or file name header, that is surfaced, not shipped. |
| **Q4** | **The PDF's typography.** | ✅ **RULED as recommended (user, 2026-11-12).** **The writer's** (S4 Q1): face, size, indent rule, the screen's line height — and here the **face is embedded**, since both apps register the bundled fonts in-process. |
| **Q5** | **The ePub's structure.** | ✅ **RULED as recommended (user, 2026-11-12).** **EPUB 3**: a title page (title, subtitle, byline), a **nav** (contents) listing the chapters by their headings ("Chapter N" when untitled), **one XHTML file per chapter**, a stylesheet with the writer's size, indent rule and scene-break rule but **no fixed face** (readers apply their own; Apple Books and Kobo override it anyway), scene breaks as `<hr />`. No cover image (Scrivi has none). |
| **Q6** | **The ePub's metadata.** EPUB 3 requires an identifier, a language and a modified time. | ✅ **RULED as recommended (user, 2026-11-12).** **Identifier** `urn:uuid:<projectID>` — the same book on every export, so a reader updates it rather than adding a second copy. **Authors** one `dc:creator` each, in byline order. **Language** passed by the host from the system's preferred language (as S4 passes `defaultTypeface`) — there is no project setting, and adding one is out of scope. **Modified** the export time (UTC, to the second): readers use it to see a newer edition; the "same bytes" check compares two exports with that one field masked. |
| **Q7** | **Scope.** | ✅ **RULED as recommended (user, 2026-11-12).** **ePub: the whole manuscript only** (a book; a one-chapter ePub has no use the HTML and `.docx` don't serve). **PDF: the whole manuscript or the caret's chapter**, as `.docx`. |
| **Q8** | **Verification.** | ✅ **RULED as recommended (user, 2026-11-12).** **epubcheck** (Java 11 is on this Mac) run on the test fixture and a real project as recorded evidence — not added to CI (a Java download per run). Readers: Apple Books (Mac, iPad); on scrivi-vm, install **Foliate** (`apt`, ePub) — PDF opens in GNOME's own viewer if present, else install **Papers/Evince**. |

⛔ **NOT in this Sprint:** any import (S6–S7), a cover image, embedded fonts in the ePub (Q5), a project language setting
(Q6), Standard Manuscript Format (S4 Q1), inline object references ([EP-032]).

## Acceptance Criteria (re-cut by Q1, 2026-11-12: ePub only)

- [x] AC1 — `scrivi_export_manuscript` `format: "epub"` writes an EPUB 3 of the whole manuscript through a partial file
      renamed into place: `mimetype` first and stored; title page with the byline; nav; one XHTML per chapter; stylesheet
      (Q5); metadata (Q6); refused for one chapter (Q7); the same bytes on every platform, the modified time aside
- [x] AC2 — epubcheck reports no errors on the test fixture's shape and on real projects (Q8)
- [x] AC3 — Boundary tests through `scrivi_*` against golden files; macOS and Linux ctest green
- [x] AC4 — Export Manuscript… offers ePub on the Mac, iPad and Linux (no scope question, Q7); flushed first, off the main
      thread, progress after 400 ms; the host's language passed; interop test and Linux smoke through the wrappers; engine
      stub parity green
- [ ] AC5 — **Live passes (user):** an ePub from the Mac and from Linux opens in Apple Books (Mac, iPad) and Foliate (Linux):
      title page with the byline, the contents list, chapters, scene breaks, emphasis and lists as in Scrivi

➡️ **Moved to the PDF Sprint by Q1:** plan steps 2 and 4 (the print page, the three PDF renderers), with Q2–Q4 as ruled.

## Progress log

- **2026-11-12** — 🔵 **Created in Planning** (user: *"plan EP-054 S5."*), issued via `next-id.py`; [T-0674] filed. Q1–Q8
  to rule.
- **2026-11-12** — 🟡 **Activated, Q1–Q8 ruled as recommended** (user, 2026-11-12: *"Q1–Q8 as recommended, activate and implement SP-213."*). By Q1 this Sprint is the ePub; the PDF (plan steps 2
  and 4, Q2–Q4) becomes its own Sprint.
- **2026-11-12** — **Built.** Core: `exportEpub` (`ManuscriptExport.cpp`) — the S3 walker's HTML cut into one XHTML per
  chapter, a title page with the byline, `nav.xhtml`, `style.css` from the writer's size and indent rule, `content.opf`; the
  zip loop shared with `.docx` as `zipParts` (`mimetype` stored first). `scrivi_export_manuscript` `format: "epub"` +
  `destPath`, `language` (sanitised to a tag; `und` when absent), refused for one chapter (`scrivi_c_api.cpp`, `scrivi.h`).
  ⚠️ **Found by epubcheck:** a project ID is `project_<UUID>`, so `urn:uuid:` takes the UUID alone (`uuidOf`), else
  OPF-085. `ManuscriptExportCApiTests`: 3 new cases (17 / 828 assertions). Wrappers: `ScriviEngine.exportManuscript(…
  language:)` (+ stub); Export Manuscript… ▸ **ePub** on the Mac and iPad (`ProjectArchiveFlow.swift`,
  `Locale.preferredLanguages`) and Linux (`EditorShell::exportManuscript`, `QLocale().uiLanguages()`).
  **Evidence:** macOS ctest **840/840**; interop `manuscriptExportThroughEngine` (1 test, ePub included) passed; iOS build
  succeeded; stub parity, ABI guard and package-boundary checks clean. scrivi-vm (scratch copy of the working tree): ctest
  **845/845**, `bridge_parity_smoke` **74 checks, 0 failures** (new ePub check). **epubcheck 5.1.0: 0 errors, 0 warnings** on
  copies of `the-river-of-time` and `the-stairs-of-tintagael` (60 scenes, 10 chapters). The Tintagael ePub from the Mac and
  from Linux arm64: **all 14 files identical** but `dcterms:modified` → AC1–AC4.
  ⏳ AC5: the live passes. ⏳ CI after the push.
