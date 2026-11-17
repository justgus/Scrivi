---
sprint: SP-219
epic: EP-054
status: Active
platform: Cross
created: 2026-11-15
---

# SP-219 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S6b**: `.docx` import

**Status:** 🟡 **ACTIVE** 2026-11-15 (user, 2026-11-15: *"Make the test files.  Put them in /Users/justgus/ScriviLinux/projects/imports.  Q1-Q5 as recommended, activate and implement SP-219"*) · planned 2026-11-15 (user: *"close SP-218 then plan S6b."*); ✅ Q1–Q5 ruled as recommended.
**Carries:** [T-0682] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-218] (S6a: `scrivi_import_manuscript`, the Markdown reader, the surfaces, the notes), [SP-208] (S4: the `.docx`
export this must read back), [SP-205] (miniz, the zip reader).
**Already ruled** in [SP-218] (user, 2026-11-14: *"Q1-Q8 as recommended."*): **Q6** pugixml, pinned by FetchContent and compiled into
the library as md4c and miniz are; **Q7** the mapping (below, with Q1 here). Q2 (new project or into the open one) and Q8 (the notes)
carry over unchanged.
**Size:** M — one reader behind the existing endpoint; the surfaces only learn a file type.

---

## What reading the code found (2026-11-15)

- **The endpoint, the surfaces and the paste exist.** `scrivi_import_manuscript` (`scrivi.h:131`) picks `markdown` or `text` from the
  extension (`scrivi_c_api.cpp:1392`), caps the file at 64 MB, and hands bytes to `manuscript::importManuscript`. The Mac, iPad and
  Linux pickers list `.md`, `.markdown`, `.txt` (`ProjectArchiveFlow.swift` `manuscriptTypes`; `ScriviWindow.cpp`).
- **The shortest route is `.docx` → Markdown → the S6a reader.** Each Word paragraph becomes one Markdown block: its text escaped
  with S6a's own `escaped()` (`ManuscriptImport.cpp:166`), so `*` typed in Word stays `*`; italic and bold runs wrapped in `*` /
  `**`; Heading 1 → `#`, Heading 2–3 → `##`/`###`; a scene break → `* * *`; `<w:br/>` → `\n`. The chapters, scenes, empty-scene
  rule and title page then come from code already tested. No second Range builder.
- **The zip reader is in the library** (miniz, [SP-205]); **pugixml is not yet** (Q6 ruled it).
- ⚠️ **Q7 says "Title opens a chapter", but S4 writes its title page as `Title`, `Subtitle`, `Author` paragraphs**
  (`ManuscriptExport.cpp:433–444`, `:497`). Read as ruled, a Scrivi `.docx` comes back with its title as an extra chapter → Q1.
- **Producers differ.** Word names styles by ID (`Heading1`); Pages, Google Docs and LibreOffice write their own IDs and the name
  `heading 1`, or only an outline level → Q3. `textutil -convert docx` (built into macOS) is a third producer to test with, besides
  Word and Pages, which are on this Mac.

## Goal

A writer can bring in a Word document — from Word, Pages, Google Docs or Scrivi's own export — as a new project or into the open
project at the caret as one undo step, with chapters, scenes, italics and bold kept, and is told about everything Scrivi could not
keep. The same on the Mac, the iPad and Linux.

## Plan

1. **Red tests** (`ManuscriptImportCApiTests.cpp`, through `scrivi_*`): every Q7 case, Q1–Q4, the refusals (a `.doc`, a zip that
   is not Word, a part over the size cap, XML with a DTD), and the round trip — S4's fixtures exported to `.docx`, imported,
   exported to Markdown: the same bytes as their own Markdown export.
2. **Core:** pugixml by FetchContent (pinned, hashed, PRIVATE). `ManuscriptDocx.cpp` beside `ManuscriptImport.cpp`: unzip
   `word/document.xml` and `word/styles.xml` (a size cap on each), write the Markdown, then `importManuscript(…, markdown)`; its
   own notes join the report (images, tables, footnotes, endnotes, comments, text boxes, headers and footers, Q4). The endpoint
   takes `format: "docx"`, or finds it per Q2. `scrivi.h` documented.
3. **Surfaces:** `.docx` in the pickers (Mac and iPad: `org.openxmlformats.wordprocessingml.document`; Linux: the filter). No other
   change; the iPad still makes a new project only ([T-0680]).
4. **Checks:** macOS ctest and interop; the iOS build; Linux ctest (scrivi-vm) and the bridge smoke; stub parity, ABI guard,
   boundary checks; a fuzz-style run of the reader over truncated and mangled fixtures under the sanitizers; CI read after the push.
5. **Live passes (user):** a Scrivi `.docx` export imported back; a document written in Word and one in Pages (italics, bold, a
   scene break, a table and a footnote, which the notes name); into an open project then undone, on the Mac and Linux.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **The title page.** Q7 maps `Title` to a chapter, but S4 writes the project's title page as `Title`, then `Subtitle`, then `Author`. | **As Markdown's Q3:** exactly that shape at the start (a `Title` paragraph, an optional `Subtitle`, then an `Author` paragraph or a `by …` line) is the project's title, subtitle and byline (the byline note), not a chapter. A `Title` anywhere else opens a chapter, as Q7. So Scrivi's own export round-trips. | ✅ **RULED as recommended** (user, 2026-11-15) |
| **Q2** | **How the format is found.** | **By the bytes, not only the name:** a zip holding `word/document.xml` is `.docx`, whatever its extension. A binary Word 97–2003 `.doc` is refused with "Save it as .docx in Word or Pages and import that". `.odt`, `.rtf` and `.pages` are out (refused as not text, as today). | ✅ **RULED as recommended** (user, 2026-11-15) |
| **Q3** | **How headings are recognised across producers.** | **By the style's name, not its ID:** look the paragraph's style up in `styles.xml`, follow `basedOn` until one is `Title`, `heading 1`–`heading 3` (or `heading 4`–`6`, kept as `####`–`######`), or fall back to the paragraph's own outline level. Covers Word, Pages, Google Docs and LibreOffice without a list per producer. | ✅ **RULED as recommended** (user, 2026-11-15) |
| **Q4** | **Formatting Scrivi has no place for** (underline, strikethrough, small caps, colours, fonts, sizes, alignment). | **Kept as plain text, one note** ("formatting not kept", a count and the first example). ⚠️ Standard Manuscript Format underlines for italics; it is **not** guessed into italics, as Q3 guesses nothing — the note names it so the writer can choose. | ✅ **RULED as recommended** (user, 2026-11-15) |
| **Q5** | **Test files from real producers.** | **Commit small fixtures:** Scrivi's own `.docx` (made by the tests), one from `textutil`, and one each from Word and Pages that **I make on this Mac** from a short test text (no writing of yours), each holding the Q7 cases. Your live pass then uses your own documents. | ✅ **RULED as recommended** (user, 2026-11-15) |

⛔ **NOT in this Sprint:** `.doc`, `.odt`, `.rtf`, `.pages` (Q2); images brought in as assets (reported, Q7); Scrivener (S7); Into
This Project on the iPad ([T-0680]); inline object references ([EP-032]).

## Acceptance Criteria

- [x] AC1 — `scrivi_import_manuscript` reads a `.docx` per Q7 and Q1–Q4 into the same `scrivi.range.v1` and report as S6a, writes
      nothing without `newProject`, and refuses per Q2; pugixml pinned and PRIVATE
- [x] AC2 — Round trip: each S4 fixture exported to `.docx` and imported gives the same Markdown export as the original
- [ ] AC3 — Boundary tests through `scrivi_*`: every Q7 case, Q1–Q4, the Q5 fixtures, every refusal, and mangled files under the
      sanitizers with no crash; macOS and Linux ctest green — ✅ macOS; ⏳ Linux ctest, the Word and Pages fixtures (Q5)
- [ ] AC4 — `.docx` offered by File ▸ Import ▸ Manuscript… on the Mac, iPad and Linux — ✅ built; ⏳ the Linux build is unchecked
      (scrivi-vm unreachable)
- [ ] AC5 — Interop tests and the Linux bridge smoke green; stub parity, ABI guard and boundary checks green; CI green
- [ ] AC6 — **Live passes (user)** on the Mac, iPad and Linux: plan step 5

## Progress log

- **2026-11-15** — 🔵 **Created in Planning** (user, 2026-11-15: *"close SP-218 then plan S6b."*), issued via `next-id.py`;
  [T-0682] filed. Q1–Q5 to rule.
- **2026-11-15** — 🟡 **Activated**; ✅ Q1–Q5 ruled as recommended (user, 2026-11-15: *"Make the test files.  Put them in /Users/justgus/ScriviLinux/projects/imports.  Q1-Q5 as recommended, activate and implement SP-219"*). The Q5 files also go to
  `~/ScriviLinux/projects/imports` (the user's share), named `sp219-*`.
- **2026-11-15 — Built.** Core: pugixml 1.14 by FetchContent (pinned, hashed, PRIVATE, `PUGIXML_NO_XPATH`; `ScriviCore/CMakeLists.txt`).
  `manuscript/ManuscriptDocx.cpp`: unzip `word/document.xml`, `styles.xml`, `numbering.xml` (each capped at 256 MB before it
  is inflated), write Markdown, read it with S6a's `importManuscript` (new `reportMarkup` flag: Scrivi's own marks are not
  reported). Styles by name through basedOn, else the outline level (Q3); S4's title page kept raw for S6a's rule (Q1); text
  escaped only where Scrivi would read markup (`\ * _ \``, and a line start that opens a heading, quote, list or rule), so a
  Scrivi export round-trips byte for byte (AC2). The endpoint finds a zip by its bytes, refuses a `.doc` (`legacyDoc`) (Q2).
  Notes: picture, docxTable (text kept), footnote, endnote, comment, textBox, equation, headerFooter, formatting (Q4: a font or
  size counts only when it is not the document's most common, or Pages' and Google Docs' per-run fonts would flag every line),
  link. ⚠️ Beyond Q7's letter: an uncentred `* * *` / `***` line is a scene break too, as in S6a's Markdown and text.
  Surfaces: `.docx` in `manuscriptTypes` (Mac and iPad) and the Linux filter; the notes worded on both (`tellNotes`,
  `importNotes`). `scrivi.h` documented.
  **Tests:** 11 `[docx]` cases in `ManuscriptImportCApiTests.cpp` (synthetic parts for every case; Scrivi's own export back to
  the same Markdown; `fixtures/docx/sp219-textutil.docx`; `sp219-every-case.docx`; a DTD's entities never expanded; a part
  claiming 512 MB refused; truncated and bit-flipped files). Mutation-checked in a copy of the repo: styles by ID, no escaping, a
  page break before a heading as a scene, no size cap, deletions kept, the title page not consumed — each fails a test.
  ctest **866/866** (one earlier run had 2 failures, not captured; four reruns clean). Interop **351/351** (1 new). iOS build ✅;
  stub parity, ABI guards (135), boundary ✅. ⚠️ **Not run:** Linux build and ctest (scrivi-vm did not answer; Docker not
  running). ⚠️ **Q5's Word and Pages files not made:** scripting Word was refused by the permission system — the user to make
  them. The textutil file has no styles at all (headings are larger bold text): it imports as one chapter, with a formatting
  note, as Q3 rules. Files copied to `~/ScriviLinux/projects/imports`: `sp219-textutil.docx`, `sp219-every-case.docx`.
