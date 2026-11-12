---
sprint: SP-212
epic: EP-054
status: Closed
platform: ScriviCore
created: 2026-11-12
---

# SP-212 — `[ScriviCore]` [EP-054] **AC9**: the byline on every export

**Status:** ✅ **CLOSED** 2026-11-12 (user, 2026-11-12: *"close SP-212."*) · activated 2026-11-12 (user, 2026-11-12: *"Q1-Q4 as recommended.  Please activate and implement SP-212."*) · ✅ Q1–Q4 ruled as recommended · planned 2026-11-12 (*"Next is EP-054 AC9."*).
**Carries:** [T-0673] → [`../../Tasks/Verified/Task-verified-0673.md`](../../../Tasks/Verified/Task-verified-0673.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Builds on:** [SP-207] (Markdown, HTML), [SP-208] (`.docx`) — `scrivi_export_manuscript`; [EP-058] S1 ([SP-209]) — the
ordered authors (`AuthorStore`).
**Authority:** EP-054 AC9; [`Scrivi_Authorship_Design_v0_2.md`](../../Scrivi_Authorship_Design_v0_2.md) Q6 (✅ ruled
2026-11-10), which re-opens SP-208 Q7 ("title only"); EP-058 AC4 (no host name and no "Scrivi" ever exported as a pen name).
**Size:** S — core only. Every platform already exports through the one endpoint, so no wrapper or surface changes.

---

## What reading the code found (2026-11-12)

- **One place builds every export's input:** `scrivi_export_manuscript` (`scrivi_c_api.cpp:1254`) fills
  `manuscript::ExportInput` (title, subtitle, titlePage, scenes) and hands it to the pure writers in
  `ManuscriptExport.cpp`. The byline is one more field there, read once, used by all three formats.
- **The authors are already readable:** `identity::AuthorStore::read` (`AuthorStore.cpp:102`) returns the active authors in
  byline order, each flagged `unnamed` when its name is "Scrivi" or this identity's recorded host name (SP-209 Q1). It only
  reads, so it fits the endpoint's READ guard. An empty identity skips the writing-as part.
- **Where each format puts the title today:**
  - Markdown `# Title`, then the subtitle line (`ManuscriptExport.cpp:251`); no metadata block.
  - HTML `<title>` in `<head>` (page only), `<h1>` + `<p>` subtitle (`:268–273`). Copy as HTML (body only) has no title page.
  - `.docx` Title / Subtitle styles (`:412–418`, `:467`); `docProps/core.xml` holds `dc:title` only (`:513`).
- **The existing golden tests should not move:** a fresh package's only author is unnamed, so it gets no byline (Q1).

## Goal

Every manuscript export says who wrote it: **"by A"**, **"by A and B"**, **"by A, B and C"** under the title and subtitle,
and the authors in the file's metadata — in Markdown, HTML and `.docx`, the same bytes on every platform — and never a host
name or "Scrivi".

## Plan

0. **Red tests first** (`ManuscriptExportCApiTests.cpp`, through `scrivi_*`): `scrivi_set_authors` one, two and three
   authors, then export each format — the byline line, `<meta name="author">`, the `.docx` Author paragraph and
   `dc:creator`; a retired author left out; an unnamed author left out, and a package of only unnamed authors with no
   byline and no metadata (the existing goldens unchanged); a one-chapter file per Q2; body-only HTML with none.
1. **Core:** `ExportInput` gains `authors` (the named ones, in order); the endpoint fills it from `AuthorStore::read`; one
   pure helper joins "by A, B and C"; each writer adds its line and metadata (Q2, Q3).
2. **Docs:** `scrivi.h` (the result note), the design's "Exports" row (`Scrivi_Authorship_Design_v0_2.md:46`).
3. **Checks:** macOS ctest; Linux ctest on scrivi-vm; the `.docx` SHA-256 equal on both.
4. **Live passes (user):** a project with two authors exported as Markdown, HTML and `.docx` on the Mac and Linux; the
   `.docx` opened in Word / Pages (Mac) and LibreOffice (Linux), its byline under the title and its author in File ▸
   Properties. iPad: the existing [T-0669] (waits for the iPad).

## Questions

| # | Question | Ruling |
| - | -------- | -------------- |
| **Q1** | **Unnamed authors** (credited to "Scrivi" or this machine's name). EP-058 AC4 forbids exporting them. | ✅ **RULED as recommended (user, 2026-11-12).** **Left out of the byline and the metadata; a work with no named author exports with no byline.** No prompt at export: opening the project already asks the writer to name the author (SP-209 Q3). |
| **Q2** | **A one-chapter file**, which has no title page (S3 Q6). | ✅ **RULED as recommended (user, 2026-11-12).** **No byline line, but the metadata carries the authors** (HTML page `<meta>`, `.docx` `dc:creator`): the metadata says who wrote the file, whatever part of the book it is. Markdown has no metadata, so nothing. |
| **Q3** | **The metadata text.** Design Q6 gives `.docx` `dc:creator` as "A; B". | ✅ **RULED as recommended (user, 2026-11-12).** **"A; B" in both** `dc:creator` and HTML `<meta name="author">` — a list a machine can split; the "by … and …" sentence is for the page. |
| **Q4** | **The `.docx` Author style.** | ✅ **RULED as recommended (user, 2026-11-12).** **Centred, the Subtitle's line spacing, the body text size, not bold**, after the Subtitle on the title page — named **Author** so a writer restyles it once in Word. |

⛔ **NOT in this Sprint:** PDF and ePub (S5 — they carry the byline when built, design Q6), world authors (exports are of a
project), a localised "by"/"and", any surface change.

## Acceptance Criteria

- [x] AC1 — A whole-manuscript Markdown, HTML page and `.docx` carry "by A" / "by A and B" / "by A, B and C" under the title
      and subtitle, in the project's author order; the `.docx` line in an Author style (Q4)
- [x] AC2 — HTML `<meta name="author">` and `.docx` `dc:creator` carry the authors (Q3), on a one-chapter file too (Q2);
      body-only HTML carries none
- [x] AC3 — Retired and unnamed authors never appear (Q1); a work with no named author exports exactly as before
- [x] AC4 — Boundary tests through `scrivi_*`; macOS and Linux ctest green; the same `.docx` bytes on both
- [x] AC5 — **Live passes (user):** the Mac and Linux, per plan step 4

## Progress log

- **2026-11-12** — 🔵 **Created in Planning** (user: *"Next is EP-054 AC9."*), issued via `next-id.py`; [T-0673] filed.
  Q1–Q4 to rule.
- **2026-11-12** — 🟡 **Activated, Q1–Q4 ruled as recommended** (user, 2026-11-12: *"Q1-Q4 as recommended.  Please activate and implement SP-212."*).
- **2026-11-12** — **Built** (plan steps 0–2). `ExportInput.authors` (`ManuscriptExport.hpp`), filled by `scrivi_export_manuscript`
  from `AuthorStore::read` with the unnamed left out (`scrivi_c_api.cpp`); `byline` / `creators` and each writer's line and
  metadata (`ManuscriptExport.cpp`); the Author style is written only when there is a byline, so an unbylined `.docx` is
  byte-for-byte as before. `ManuscriptExportCApiTests`: the fixture's only author is now "Scrivi", so every existing golden
  is the no-named-author case (AC3), plus 3 new cases — every format with two authors, one / three / retired / unnamed, a
  one-chapter file. macOS ctest **837/837** → AC1–AC3. `scrivi.h` and the design's "Exports" row updated.
  ⏳ AC4: Linux ctest on scrivi-vm and the cross-platform `.docx` SHA-256, after the push. ⏳ AC5: the live passes.
- **2026-11-12** — ✅ **AC4** — the user exported `the-river-of-time.scrivi` (two authors) as `.docx` on the Mac and Linux
  (`~/ScriviLinux/projects/exports`): the whole manuscript has the **same SHA-256 on both** (`bc38b179…`), and so has Chapter 2
  (`4398d817…`). Each carries `dc:creator` "Terence Strong; Mike Stoddard"; the whole-manuscript files have the Author paragraph
  "by Terence Strong and Mike Stoddard", the chapter files none (Q2). The Markdown and HTML exports carry the byline line and
  `<meta name="author">`. Linux ctest on scrivi-vm (build of `c979b63`): **842/842**, `[manuscript-export]` 14 cases / 614
  assertions. Linux App CI on `c979b63` green; ScriviCore CI still running at this entry.
- **2026-11-12** — ✅ **AC5 — live passes** (user, 2026-11-12: *"For AC5 I ran the same project, which has two authors. a-f passed."*): all four `.docx` open and read correctly in Pages and LibreOffice Writer.
  The user has no Word, so the Author property was not read in Word; it is proved from the files above (`docProps/core.xml`).
  T-0673 → ✅ Verified, archived. Close awaits approval.
  ⚠️ The exports folder also holds `~$e-river-of-time-Chapter-2-mac.docx` (162 bytes, the text "Mike Stoddard"): an
  Office-style owner/lock file a reader left behind, not a Scrivi export. Left in place.
- **2026-11-12** — ScriviCore CI on `c979b63`: four jobs green; **macOS sanitizers still in Build after 30 min**. Not this
  Sprint's code — the job has run 3 to 56 min for weeks: an unbounded `make -j` → filed **[I-0358]**, workflow fixed.
- **2026-11-12** — ✅ **CLOSED** (user, 2026-11-12: *"close SP-212."*). EP-054 AC9 met.
