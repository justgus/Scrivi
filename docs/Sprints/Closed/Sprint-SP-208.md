---
sprint: SP-208
epic: EP-054
status: Closed
platform: Cross
created: 2026-11-10
---

# SP-208 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S4**: Word `.docx` export

**Status:** ✅ **CLOSED** 2026-11-10 (user, 2026-11-10: *"They opened cleanly, close SP-208"*) · activated 2026-11-10 (user: *"Q1–Q7 as recommended, activate and implement SP-208."*) · ✅ Q1–Q7 ruled as recommended · planned 2026-11-10 (*"plan SP-208 for EP-054 S4"*).
**Carries:** [T-0668] → [`../../Tasks/Verified/Task-verified-0668.md`](../../Tasks/Verified/Task-verified-0668.md).
**Epic:** [EP-054] → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Builds on:** [SP-207] (S3, ✅ closed): `scrivi_export_manuscript`, its walker over what the writer sees, and the Export
Manuscript… flow on the Mac, iPad and Linux. [SP-205] (S1, ✅ closed): miniz, already compiled into the core.
**Authority:** EP-054 AC5 (`.docx` with chapters, scene breaks and emphasis preserved), Q5 (`.docx` after HTML and
Markdown), Q6 (`.docx` produced in the core), Q7 (miniz).
**Size:** M (one more writer behind the S3 endpoint, one more choice in the existing format prompt) — one Sprint.

---

## What reading the code found (2026-11-10)

- **The S3 endpoint returns text in a JSON envelope** (`{text, chapterCount, sceneCount}`, `scrivi.h:101`). A `.docx` is a
  zip, so it cannot travel in `text` → Q2.
- **The walker is half shared already.** `ManuscriptExport.cpp`'s `walk` (scene order, empty scenes skipped, chapter
  boundaries) is format-free, but `blockHtml` / `inlineHtml` emit HTML strings directly. A `.docx` needs the same
  decisions (paragraphs, `\n` breaks, bold-outside-italic runs, headings, lists, resolved escapes) as Word XML, so those two
  are split into a block/run walk with two emitters. The HTML golden tests guard the split.
- **The zip is ready:** miniz is in the core ([SP-205], `ProjectArchive.cpp`). ⚠️ But `mz_zip_writer_add_mem` stamps each
  entry with the CURRENT time, so two exports of the same manuscript differ by bytes. For "the same bytes on every platform"
  (S3's AC1) each part is added with a fixed time (`mz_zip_writer_add_mem_ex_v2`, `last_modified`).
- **The writer's own typography is in the package:** `project-settings.json` holds `typeface`, `textSize`,
  `paragraphIndent` (`book` | `every` | `none`) and `indentEm` (`ProjectPreferences.swift:80–86`, EP-047). The core does
  not read them today → Q1.
  - `book` (the default) indents a body paragraph only when it follows body text in the same scene
    (`ManuscriptTypography.swift:100`): no indent after a title, heading or scene break — the book convention.
  - The typeface is one of Scrivi's bundled faces, which a reader's Word may not have → Q1.
- ⚠️ **CORRECTED 2026-11-10 (user):** *"Scrivi has an identity built into the project and it has the capability to save an
  author's name … the author of a work may not be the same as the identity or legal name of the person writing."* The
  planning read above said no author field exists; that was wrong. **The pen name is designed in and half built:**
  `identities/project-personas.json` (`scrivi.projectPersonas.v1`: `personaID`, `displayName`, …) is created in every
  package (`ProjectCreator.cpp`), and `AuthorshipRef {identityID, personaID, displayName}` threads every authorship call.
  ⛔ **But nothing lets the writer set it**: no persona endpoint exists, and the app passes the machine's host name as the
  persona's `displayName` (`AppEnvironment.swift:222`; the Tintagael package reads `"displayName": "flitwick-5.local"`).
  The byline is item 4 of [`Scrivi_Identity_Authorship_and_Settings_Design_v0_1.md`](../../Scrivi_Identity_Authorship_and_Settings_Design_v0_1.md)
  (⚠️ DRAFT, its Q2–Q5 unruled; Q5 asks exactly where a byline should appear). Printing today's `displayName` would put a
  host name on the title page, so this Sprint carries **no byline**, as Q1/Q7 were ruled.
- **What a `.docx` must hold at minimum:** `[Content_Types].xml`, `_rels/.rels`, `word/document.xml`, and for named styles
  and real lists `word/styles.xml`, `word/numbering.xml` and `word/_rels/document.xml.rels`; `docProps/core.xml` for the
  title. All are short, fixed text except `document.xml`.
- **Readers on hand:** Microsoft Word and Pages on this Mac; LibreOffice Writer (Ubuntu's default office suite) to check on
  scrivi-vm. No pandoc or LibreOffice on the Mac, so the automated check is the core's own: unzip with miniz and compare
  `document.xml` with a golden file.

## Goal

A writer can take the manuscript — or one chapter — out of Scrivi as a **Word document** that opens cleanly in Word,
Pages and LibreOffice, with its chapters, scene breaks, emphasis, headings and lists intact, every paragraph in a **named
style** (so one change in Word restyles the whole book), from the Mac, iPad and Linux. The core writes it; the platforms
only ask and deliver.

## Plan

0. **Red tests first:** extend `ManuscriptExportCApiTests.cpp` with `.docx` cases through `scrivi_*` — the S3 fixture
   exported, unzipped with miniz, `word/document.xml` compared with a golden file; the fixed parts present; one chapter;
   titles hidden; the inline edge cases (`&`, `<`, `>`, escapes, bold inside italic); two exports byte-identical; a bad
   destination path. All fail until the writer exists.
1. **Core — split the walker:** `blockHtml` / `inlineHtml` become a walk that reports blocks (paragraph, heading level,
   list item with ordered/number, line break) and runs (text + bold/italic) to an emitter; HTML is the first emitter and
   its golden bytes do not change.
2. **Core — the `.docx` writer** (`ManuscriptExport.cpp`, beside HTML): `document.xml` from the second emitter, the fixed
   parts as constants, styles per Q1, page breaks and scene breaks per Q3–Q4, lists per Q5, `\n` per Q6; zipped with miniz
   at a fixed time to `destPath` through a partial file renamed into place (as the archive does). Endpoint per Q2.
3. **Wrappers:** `ScriviEngine.exportManuscript` gains the `.docx` case (+ the visionOS stub, `check-engine-stub-parity.sh`);
   `ScriviBridge::exportManuscript`.
4. **Surfaces** (Mac, iPad, Linux — Linux adopts the Mac's shape and wording): Export Manuscript…'s format prompt gains
   **Word Document (.docx)**; then scope; then the save panel (Mac, Linux) or share sheet (iOS). Same flush, off the main
   thread, progress after 400 ms. No clipboard for `.docx`.
5. **Live passes:** export the whole manuscript and one chapter; open in Word and Pages (Mac), Files ▸ Pages or Word (iPad),
   LibreOffice Writer (Linux, scrivi-vm); check chapters, page breaks, scene breaks, emphasis, lists and the styles pane.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Whose typography does the `.docx` carry?** (a) **The writer's Scrivi settings** — typeface, size, and the indent rule (`book` / `every` / `none`) — written into the named styles. (b) **Standard Manuscript Format** — Times New Roman 12, double-spaced, 1" margins, ½" first-line indent, a "Name / Title / page" header — what agents and editors ask for. (c) **Word's defaults**, styles named but unset. ⚠️ Scrivi's bundled faces may not be on the reader's machine; Word then substitutes one silently. ⚠️ (b) needs an author name Scrivi does not store. | ✅ **RULED as recommended (user, 2026-11-10).** **(a), with the face named but not embedded.** It is what the writer sees, as S3's HTML is. Every paragraph is in a named style, so a writer who needs (b) changes Normal and Heading 1 once in Word. `book` maps to two styles — **First Paragraph** (no indent: after a title, heading or scene break) and **Body Text** (indented); `every` indents both; `none` indents neither and adds space after. Line spacing single (Scrivi has no setting for it). Standard Manuscript Format can follow as an option if it's missed — with an author field. |
| **Q2** | **The endpoint.** The `.docx` is binary; the S3 result carries text. (a) the same endpoint with `format: "docx"` and a required `destPath` option, writing the file and returning `{path, chapterCount, sceneCount}`; (b) a separate `scrivi_export_manuscript_docx`. | ✅ **RULED as recommended (user, 2026-11-10).** **(a).** One option set (format, scope, chapter) and one scene read; the archive export already writes to a path the host chose. |
| **Q3** | **Page breaks.** | ✅ **RULED as recommended (user, 2026-11-10).** **Each chapter starts on a new page** (the chapter heading style has *page break before*), and a whole-manuscript file's **title page** (Title + Subtitle styles) is a page of its own. A one-chapter file starts at its chapter heading, as S3 has no title page there. With chapter titles hidden, chapters still start on a new page — the break replaces S3's `<hr />`. |
| **Q4** | **Scene breaks.** | ✅ **RULED as recommended (user, 2026-11-10).** **A centred `* * *` paragraph** in a **Scene Break** style — the same three asterisks as the Markdown export, and what survives every reader and FanFiction.net's Doc Manager. Word's horizontal rule is a paragraph border that some readers drop. ⚠️ **Superseded (user, 2026-11-10: *"switch scene breaks to the rule"*):** an empty Scene Break paragraph with a 1 pt grey bottom border, inset 20 pt each side — the screen's rule. The Markdown export keeps `* * *`. |
| **Q5** | **Headings and lists.** | ✅ **RULED as recommended (user, 2026-11-10).** **Chapter titles in Heading 1**; the writer's own `#`–`###` inside scenes **keep their levels** (Heading 1–3), exactly as the Markdown export (S3 Q6). **Real Word lists** (`numbering.xml`, List Bullet / List Number), each numbered list restarting at its own first number — so Word renumbers when the writer edits, and S3's `<ol start>` case holds. |
| **Q6** | **A single `\n` inside a paragraph.** | ✅ **RULED as recommended (user, 2026-11-10).** **A line break inside the paragraph** (`<w:br/>`, Shift-Return in Word) — the `.docx` form of S3's `<br />` ruling. Two `\n` remain a new paragraph. |
| **Q7** | **Scope and where it goes.** | ✅ **RULED as recommended (user, 2026-11-10).** **As S3:** the whole manuscript or the chapter the caret is in, to a file only (save panel / share sheet). Document properties: the title only (no author, no dates — a date would change the bytes on every export). |

⛔ **NOT in this Sprint:** PDF and ePub (S5), any import (S6–S7), Standard Manuscript Format or an author field (Q1), embedded
fonts (Q1), a free chapter/scene picker (S3 Q2), inline object references ([EP-032]), Linux double-click / MIME (EP-056).

## Acceptance Criteria

- [x] AC1 — `scrivi_export_manuscript` with `format: "docx"` writes a `.docx` for the whole manuscript and for one chapter,
      after draining the write queue, through a partial file renamed into place; the same bytes on every platform and on
      every export of the same text
- [x] AC2 — Every paragraph in a named style; typography, indent rule and line spacing per Q1; chapter pages and the title
      page per Q3; scene breaks per Q4; headings and real lists per Q5; a single `\n` as a line break per Q6; italic and
      bold as runs (bold outside italic, as S3); escapes resolved; `&`, `<`, `>` XML-escaped
- [x] AC3 — The S3 HTML output is byte-identical after the walker split (its golden tests unchanged and green)
- [x] AC4 — Boundary tests through `scrivi_*` against a golden `document.xml` (EP-054 AC7), including the characters that
      must stay literal and the repeat-export byte check; macOS and Linux ctest green, the same golden bytes on both
- [x] AC5 — Export Manuscript… offers Word Document (.docx) on the Mac, iPad and Linux; flushed first, off the main thread,
      progress after 400 ms — ⚠️ the iPad half moved to [T-0669] (user, 2026-11-10)
- [x] AC6 — Interop tests and Linux smokes through the wrappers, green; engine stub parity check green
- [x] AC7 — **Live passes (user):** a whole-manuscript and a one-chapter `.docx` open cleanly — no repair prompt — in Word and
      Pages on the Mac, in Pages or Word on the iPad, and in LibreOffice Writer on Linux; chapters, page breaks, scene breaks,
      emphasis and lists as in Scrivi; the styles named in Q1 listed in the reader's styles pane — ⚠️ the iPad half moved to [T-0669] (user, 2026-11-10)

## Progress log

- **2026-11-10** — 🔵 **Created in Planning** (user: *"plan SP-208 for EP-054 S4"*), issued via `next-id.py`; [T-0668] filed.
  Q1–Q7 to rule.
- **2026-11-10** — 🟡 **Activated, Q1–Q7 ruled as recommended** (user, 2026-11-10: *"Q1–Q7 as recommended, activate and implement SP-208."*). The user corrected the
  planning read: the author name (pen name) does exist, as the project's persona. It has never been settable, and holds
  the host name today. Recorded under "What reading the code found"; no byline in this Sprint.
- **2026-11-10** — **Core built** (plan steps 0–2). The walker split: `sceneBlocks` reports paragraphs, headings and list items
  as runs; HTML and `.docx` emit from it. S3's HTML golden tests unchanged and green (AC3). `exportDocx`
  (`ManuscriptExport.cpp`): seven parts, the styles from the project's `typeface` / `textSize` / `paragraphIndent` /
  `indentEm` (clamped to `ManuscriptTypography`'s ranges; `defaultTypeface` from the host when none is set), each chapter's
  first paragraph `pageBreakBefore` (on the paragraph, not in Heading 1 — the writer's own `#` headings are Heading 1 too, Q5),
  a numbering instance per numbered list, zipped by miniz at local 1980-01-02 (the same DOS date in every zone) and written
  through `atomicWriteTextFile`. `scrivi_export_manuscript` takes `format: "docx"` + `destPath` → `{path, …}` (Q2).
  `ManuscriptExportCApiTests`: 4 new cases (golden `document.xml`, the book rule and a list from 3, the writer's typography
  with titles hidden, the same bytes twice and a failed write leaving nothing). macOS ctest **826/826**. A copy of the
  Tintagael project (60 scenes) exported and read back by macOS `textutil` (Cocoa's `.docx` reader).
- **2026-11-10** — **Wrappers and surfaces built** (steps 3–4). `ScriviEngine.exportManuscript(… destPath:defaultTypeface:)`
  (+ stub; `text` and `path` now optional); Export Manuscript… offers **Word Document (.docx)** on the Mac and iPad
  (`ProjectArchiveFlow.swift`; the Mac stages the file and moves it into place, as the archive does — helpers
  `stagingFolder` / `moveIntoPlace` now shared) and Linux (`EditorShell::exportManuscript`). Interop
  `manuscriptExportThroughEngine` (1 test, passed); macOS and iOS builds clean; stub parity, ABI guard and boundary checks
  clean. `bridge_parity_smoke` gains the export checks (Markdown text, a `.docx` file, refused without `destPath`) — the S3
  bridge had no smoke.
  ⚠️ **Found: the package-boundary check was red since [SP-207]** — S3's save-panel write
  (`ProjectArchiveFlow.swift`) had no `boundary-ok:` marker, and S3's log called the checks clean. Marked, with its reason.
  ⚠️ **Linux NOT compiled or run:** scrivi-vm did not answer and Docker was not running.
- **2026-11-10** — **Linux on scrivi-vm** (build of `67d1028`, the user's push): ctest **830/830**; `bridge_parity_smoke` **73
  checks, 0 failures** (the new export checks pass); a 60-scene `.docx` (a copy of Tintagael) has the **same SHA-256 on the Mac
  and on Linux arm64** (`02eeae49…`) — AC1's "same bytes on every platform", for that build. 41 of 42 smokes green.
  ⚠️ `accessibility_smoke` failed 4 runs in 5: its [I-0346] assertion ("a split under a tenth of a full build") compared whole
  milliseconds (1 vs 10) and timed the 1.8 MB page copy as part of the split. Now nanoseconds, with that copy's own cost
  subtracted: 8/8 in a scratch copy (split < 1 ms, full build 11 ms). `platforms/linux/tests/accessibility_smoke.cpp`.
- **2026-11-10** — ⚠️ **CI had been red, unread, since [SP-206]/[SP-207]** (found while answering the user's boundary-check
  question): Scrivi Apple CI and Linux App CI failed on both SP-207 pushes — the boundary check (fixed above, green on
  `67d1028`); and **ScriviCore CI's sanitizer job fails the archive tests since "SP-206 live pass"**: UBSan, null pointer to
  `fwrite` in miniz, because `ProjectArchive.cpp` added folder entries with a null buffer. Fixed (`""`, `ProjectArchive.cpp:86,
  103`); the full sanitized ctest on scrivi-vm passes (the two failures under `-j4` pass alone — parallel collisions).
- **2026-11-10** — **The `.docx` matches the screen** (user: *"I want the docx to match the screen as much as possible"*; Q1's
  "single" spacing superseded): body lines an exact 1.45 × size, headings 1.25 × their size with no added space, the stored
  blank line as a third of a body line after each paragraph (a whole line with `none`), lists with no left margin, the marker
  hanging, `-` and `1.` as the editor shows them. macOS ctest 826/826. ⚠️ Linux not yet re-run on this change, so AC1/AC4
  stay open until it is. Still different from the screen: the scene break (`* * *`, Q4 — the screen draws a 1 pt rule).
- **2026-11-10** — **Scene breaks are the rule** (user: *"switch scene breaks to the rule"*): the Scene Break style is a 1 pt grey
  bottom border inset 20 pt (`ManuscriptExport.cpp` `docxStyles`), the paragraph empty. Export tests 11 cases / 424 assertions.
  ⚠️ The screen's rule is itself marked for replacement by a typeset `* * *` (`ManuscriptEditing.swift:290`); if that lands,
  the `.docx` follows it back. [EP-058] design v0.2 approved (Q1–Q9 as recommended).
- **2026-11-10** — **[EP-058] Authorship created** (user); [EP-054] gains **AC9** (the byline on every export), blocked by EP-058
  S1. This Sprint stays byline-free as ruled.
- **2026-11-10** — **Linux re-run on `3221180`** (pushed, pulled and built on scrivi-vm by the user): ctest **830/830**;
  `bridge_parity_smoke` **73 checks, 0 failures**; `accessibility_smoke` **6/6** (the timing fix holds); a 60-scene `.docx`
  with the same SHA-256 on the Mac and Linux arm64 (`0cf972f2…`) → **AC1, AC4, AC6** ticked. CI on `3221180`: ✅ both sanitizer
  jobs green (the miniz fix); ⛔ `macos-latest` failed **[I-0342]**'s read-count test, `fileOps 48 < 40` — flaky by
  construction: `ReadThroughCache::prefetch` runs `work()` on the calling thread too, so a slow runner counts most sidecar
  reads against the open. The bound is now `2 × kScenes` (`ObjectIndexRebuildTests.cpp`): the regression it guards adds two
  more reads of every sidecar, above that either way. Passes on the Mac; CI to read after the next push.
- **2026-11-10** — The user exported a chapter and the manuscript on the Mac and on Linux. **The iPad half of AC5/AC7 moved
  to [T-0669]** (user: *"my iPad is far away right now. Let's not hold this sprint open waiting for testing on it."*).
- **2026-11-10** — ✅ **Live passes (user):** a chapter and the whole manuscript exported on the Mac and on Linux open cleanly
  (AC5, AC7; the iPad half → [T-0669]). ✅ **CLOSED** (user: *"They opened cleanly, close SP-208"*); [T-0668] → ✅ Verified → `Tasks/Verified/Task-verified-0668.md`.
