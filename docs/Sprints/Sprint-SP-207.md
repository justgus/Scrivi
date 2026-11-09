---
sprint: SP-207
epic: EP-054
status: Active
platform: Cross
created: 2026-11-09
---

# SP-207 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S3**: Markdown and HTML export

**Status:** 🟡 **ACTIVE** 2026-11-09 (user, 2026-11-09: *"yes, add it and activate SP-207."*) · ✅ Q1–Q8 ruled · planned 2026-11-09 (*"plan SP-207 for EP-054 S3"*).
**Carries:** [T-0666] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-206] (S2, ✅ closed): the export flow's shape on each platform (flush, off the main thread, progress
after 400 ms, the save panel / share sheet).
**Authority:** EP-054 Q2 (✅ *"every paragraph becomes its own `<p>…</p>`; never `<br>` runs, never bare text"*; ✅ the sites —
**Q1 here**: AO3, raw HTML pasted into its box), Q5 (HTML and Markdown come right after the archive), Q6 (both
formats produced in the core).
**Size:** L (one endpoint, two writers, three surfaces) — see Q8.

---

## What reading the code found (2026-11-09)

- **No manuscript export exists.** The only export endpoints are `scrivi_export_archive` and
  `scrivi_export_project_timeline` (`scrivi.h:91`, `:691`). EP-032's backlog entry already records that manuscript export
  has no path to attach to.
- **A paragraph is a Markdown paragraph.** Return stores `\n\n` (`ManuscriptEditing.swift:470`, EP-045 AC5), and the
  Dumas fixture's scenes are blank-line separated. So EP-054 Q2's *"each paragraph its own `<p>`"* maps one-to-one onto
  what the writer typed. A single `\n` inside a paragraph is NOT rare in the Dumas fixture (141 of 200 sampled scenes
  have one, between two sentences mid-paragraph) — Q4.
- **What the writer can format** is a small, closed set, and the core already parses it: italic, bold, headings 1–3,
  bullet and numbered lists, and backslash escapes (`scrivi_analyze_markdown`, `scrivi.h:1265`; `MarkdownAnalyzer.cpp`,
  md4c with `MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH`). Everything else is **drawn as stored** — a `>`, a backtick, a
  `[link](…)`, a `<tag>` all appear to the writer as the characters typed.
  ⚠️ So md4c's own HTML renderer (`md4c-html.c`, in the pinned tarball, not compiled) would turn text the writer sees as
  plain characters into links, code, block quotes and raw HTML — Q4.
- **Chapter titles and the title page are already in the package:** the chapter's title in `chapter.meta.json`; the
  project title in `project.json`; `subtitle` and `showChapterTitles` in `project-settings.json` (EP-047; absent = titles
  ON, [SP-184] Q1). The core reads all of them.
- **Scenes are read from disk.** The editor's unsaved text must be flushed first, as the archive export does
  (`saveAllDirty`, then the core drains the write queue — [EP-052]).
- **The surfaces exist:** File ▸ Export Archive… on the Mac, iPad and Linux and the toolbar's Export slot ([SP-206]); Export
  Manuscript… goes beside it, with the same flush / progress / save-panel / share-sheet flow.

## Goal

✅ A writer can take the manuscript — or one chapter — out of Scrivi as **Markdown** or as **HTML that pastes and posts
correctly** on the fanfic sites they use, from the Mac, Linux and iPad. Both formats come from one core endpoint, so every
platform produces the same bytes; the platforms only ask and deliver (a file, or the clipboard).

## Plan

0. **Red tests first:** golden-file fixtures (a small project with emphasis, headings, lists, escapes, a literal `<`, `>`,
   `` ` `` and `[x](y)`, a chapter with and without a title, several scenes) and their expected `.md` / `.html`; boundary
   tests through `scrivi_*` (never the facade). All fail until the endpoint exists.
1. **Core:** `scrivi_export_manuscript(projectRootPath, optionsJSON)` → `{text, chapterCount, sceneCount}`. Options:
   `format` (`markdown` | `html`), `scope` (Q2), `document` (a whole HTML page, or the body only — Q3). It drains the
   project's write queue first. Two writers behind it, both walking the manuscript in order:
   - **Markdown:** the scenes as stored (escapes kept — they are valid Markdown), chapter titles as `#` headings when
     titles show, scene breaks and the title page per Q5–Q6.
   - **HTML:** from the analyzer's blocks, not md4c-html (Q4): `<p>` per paragraph (a single `\n` → `<br />`), `<em>` / `<strong>`, `<h1>`–`<h3>`,
     `<ul>` / `<ol>`; escapes resolved; every other character HTML-escaped, so what the writer sees is what posts.
2. **Wrappers:** `ScriviEngine.exportManuscript` (+ the visionOS stub — `check-engine-stub-parity.sh`),
   `ScriviBridge::exportManuscript`.
3. **Surfaces** (Mac, iPad, Linux — Linux adopts the Mac's shape and wording):
   - File ▸ **Export Manuscript…** → format, scope (Q2), then the save panel (Mac, Linux) or share sheet (iOS).
   - **Copy as HTML** (Q3): the HTML source as plain text, for pasting into AO3's box (Q1).
   - Flushed first; off the main thread; the progress row after 400 ms.
4. **A test page standing in for AO3** (user, 2026-11-09, while AO3 is offline): a paste box and Preview that apply AO3's
   OWN rules, read from its public source (`otwcode/otwarchive`: `sanitizer_config.rb` — the allowed tags and attributes —
   and its paragraph-adding for text outside tags). It shows what AO3 would keep, and marks what it would strip. No AO3
   name, logo or styling. It stores nothing pasted. Where it is hosted — see the progress log.
5. **Live passes:** paste a chapter into the test page (step 4) — and into AO3's submission box when it is back — and check the preview; open the Markdown export in another
   Markdown app.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **EP-054 Q2's remainder — which sites, and how the writer pastes.** AO3 has a Rich Text tab and an HTML tab (HTML accepts `<p>`, `<em>`, `<strong>`, `<h1>`–`<h6>`, `<ul>`/`<ol>`, `<hr />`); FanFiction.net takes an uploaded `.html` or `.docx` in its Doc Manager; Wattpad and Tumblr take rich text only. ⚠️ I have not verified these sites' current rules — the live pass is the proof. | ✅ **RULED (user, 2026-11-09): AO3 (archiveofourown.org) first, and the writer pastes RAW HTML into the submission box** — *"you paste the html tags in the box … So, you submit raw html to the site."* The writer typically brings the text from Google Docs and edits it there with the tags in view: *"The writer would then see the `<p></p>` and `<br>` and likely `<strong></strong>` and `<em></em>` directly in the text submitted."* So **Copy as HTML puts the HTML SOURCE on the clipboard as plain text** (no rich-text flavour — Wattpad/Tumblr are not in scope), and the source is **readable to edit by hand**: one block per line. ⚠️ AO3 is reported offline (a data breach) as of 2026-11-09 — AC7 may wait for it. |
| **Q2** | **How much is exported.** The Epic says *"the whole manuscript or chosen chapters/scenes"*; fanfic is usually posted a chapter at a time. | ✅ **RULED as recommended (user, 2026-11-09).** **The whole manuscript, or the chapter the caret is in.** A free choice of chapters and scenes needs a picker; it can follow if it's missed. |
| **Q3** | **Where it goes.** | ✅ **RULED as recommended (user, 2026-11-09).** **Markdown → a file. HTML → a file (a whole page: `<meta charset="utf-8">`, the title, the body) or Copy as HTML (the body only, ready for an HTML field).** |
| **Q4** | **What the HTML is made from.** md4c-html renders standard Markdown, which Scrivi does NOT present: a writer who types `[Chapter 1](notes)` or `<3` sees those characters, and md4c-html would post a link and a broken tag. And a single `\n` inside a paragraph has no `<p>`-only equivalent. | ✅ **RULED with Q1 (user, 2026-11-09): what the writer sees, exactly** — the five things the editor formats become tags; everything else is escaped text. A single `\n` inside a paragraph becomes **`<br />`** inside that paragraph's `<p>` (the user lists `<br>` among the tags the writer sees; EP-054 Q2's *"never `<br>` runs"* forbids `<br>`s standing in for paragraph breaks, which `\n\n` → `</p><p>` already prevents). ✅ Confirmed (user, 2026-11-09: *"br is correct"*). |
| **Q5** | **Scene breaks.** | ✅ **RULED as recommended (user, 2026-11-09).** **HTML: `<hr />`** (AO3 and FanFiction.net both render it). **Markdown: a line of `* * *`** (a thematic break in every Markdown reader, and readable as plain text). Nothing between chapters but the next chapter's title. |
| **Q6** | **Chapter titles and the title page.** | ✅ **RULED as recommended (user, 2026-11-09).** Chapter titles follow the project's **Show chapter titles** setting: `<h2>` in HTML, `#` in Markdown (a writer's own headings inside scenes keep their levels). A **whole-manuscript file** opens with the project title and subtitle; a single chapter and Copy as HTML do not. |
| **Q7** | **Italic and bold tags.** | ✅ **RULED as recommended (user, 2026-11-09).** **`<em>` and `<strong>`.** AO3 accepts `<i>`/`<b>` too; `<em>`/`<strong>` are what its own rich-text editor writes. |
| **Q8** | **One Sprint or two** (the archive was split core / surfaces as S1 / S2). | ✅ **RULED as recommended (user, 2026-11-09).** **One Sprint.** The endpoint is smaller than the archive's, and the surfaces repeat [SP-206]'s Export flow; splitting adds a hand-off without lowering risk. |

⛔ **NOT in this Sprint:** `.docx` (S4), PDF and ePub (S5), any import (S6–S7); a free chapter/scene picker (Q2); inline
object references ([EP-032] — none exist yet, so there is nothing to resolve); Linux double-click / MIME (EP-056).

## Acceptance Criteria

- [ ] AC1 — `scrivi_export_manuscript` produces Markdown and HTML for the whole manuscript and for one chapter (Q2), after
      draining the write queue; the same bytes on every platform
- [ ] AC2 — HTML: every paragraph its own `<p>` (never `<br>` runs, never bare text — EP-054 Q2); a single `\n` → `<br />` (Q4); one block per line, so the source reads and edits by hand (Q1); italic, bold, headings and lists
      as tags; escapes resolved; every other character escaped (Q4); chapter titles, scene breaks and title page per Q5–Q7;
      a whole page or the body only (Q3)
- [ ] AC3 — Markdown: the scenes as stored, with chapter titles, scene breaks and title page per Q5–Q6; re-read through
      `scrivi_analyze_markdown`, every scene presents as it did in Scrivi
- [ ] AC4 — Boundary tests through `scrivi_*` against golden fixtures (EP-054 AC7), including the characters that must stay
      literal
- [ ] AC5 — File ▸ Export Manuscript… and Copy as HTML (the HTML source as plain text — Q1) on the Mac, iPad and Linux; flushed first, off the main thread, progress
      after 400 ms
- [ ] AC6 — Interop tests and Linux smokes through the wrappers, green; engine stub parity check green
- [ ] AC8 — The test page (step 4) applies AO3's sanitizer list and paragraph rule as its source has them, cited by file
- [ ] AC7 — ✅ **Live passes (user):** a chapter copied as HTML and pasted into the test page (step 4) previews correctly, nothing stripped (EP-054 AC4); the
      same paste into AO3 itself once it is back online; a Markdown
      export reads correctly in another Markdown app; on the Mac, iPad and Linux. The Linux half on the VM or rig → an
      [EP-057] Task at close (Sprint-GUIDELINES)

## Progress log

- **2026-11-09** — Q1, Q4–Q8 ruled (user). Q1: AO3, raw HTML pasted into its box; Q4/Q7 follow from it, with a single `\n` → `<br />` (my reading — confirmed: *"br is correct"*). Q2, Q3 ruled as recommended. All eight ruled.
- **2026-11-09** — 🟡 **Activated** (user, 2026-11-09: *"yes, add it and activate SP-207."*). The AO3 test page added as plan step 4 / AC8.
- **2026-11-09** — **Core endpoint built** (plan steps 0–1; user: *"Use a private Artifact; start the core endpoint"* — the
  AO3 test page will be a private Artifact). `scrivi_export_manuscript` (`scrivi.h`, after `scrivi_import_archive`) →
  `ManuscriptExport.cpp` (pure: Markdown and HTML from the analyzer's blocks, not md4c-html). The parallel scene reader moved
  out of `scrivi_read_scene_texts` into `readSceneTextsInParallel`, shared by both. `ManuscriptExportCApiTests.cpp`: 7 cases
  through `scrivi_*` (golden Markdown and HTML, one chapter, page vs body, titles hidden, inline edge cases, queued writes,
  bad options). macOS ctest **822/822**; ABI guard, boundary and stub parity checks clean. ⚠️ Not yet run on Linux.
  Two choices the rulings did not cover: with chapter titles hidden, a chapter boundary is an `<hr />` / `* * *` (else
  chapters would run together); the Markdown title page is `# Title`, the same level as the chapter titles (Q6 said `#`).
- **2026-11-09** — **Wrappers and surfaces built** (user: *"proceed"*). `ScriviEngine.exportManuscript` (+ visionOS stub) and
  `ScriviBridge::exportManuscript`. Mac/iPad: File ▸ Export Manuscript… and Copy as HTML (`ProjectArchiveFlow.swift`), and the
  toolbar's Export slot is now a menu (Archive, Manuscript, Copy as HTML). Linux: the same two File items
  (`EditorShell::exportManuscript` / `copyManuscriptAsHTML`). Scope = the chapter of the caret's scene (Mac: `cursorSceneID`,
  else the viewport's). macOS and iOS builds clean; interop `manuscriptExportThroughEngine` + the SP-206 archive test pass;
  stub parity clean. ⚠️ **Linux NOT compiled**: scrivi-vm did not answer and Docker was not running.
- **2026-11-09** — **AO3 test page published (step 4)** as a private Artifact: https://claude.ai/artifact/7y2DHB29kU7xTxGthn1c5g.
  A port of `lib/paragraph_maker.rb`, `html_cleaner.rb` `sanitize_value` and `sanitizer_config.rb` (ARCHIVE + class, the
  `content` field) at otwarchive `452391b6`. Checked once in jsdom: AO3's three `paragraph_maker_spec.rb` cases pass, and the
  core test's HTML comes back with no changes. Not emulated: the embed/media sanitizers and the user-class filter.
  Finding: AO3 skips the inside of `<p>`, lists and headings and strips whitespace between blocks, so every tag and
  paragraph of Scrivi's output is kept; only the newlines between blocks go, and `<br />` is written `<br>`.
