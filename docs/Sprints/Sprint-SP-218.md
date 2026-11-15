---
sprint: SP-218
epic: EP-054
status: Active
platform: Cross
created: 2026-11-14
---

# SP-218 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S6a**: Markdown and plain-text import

**Status:** 🟡 **ACTIVE** 2026-11-14 (user, 2026-11-14: *"Ok, activate andimplement SP-218"*) · planned 2026-11-14 (user: *"We're ready to begin planning S6, the Markdown/text and .docx impor."*);
✅ Q1–Q8 ruled as recommended (user, 2026-11-14: *"Q1-Q8 as recommended."*).
**Carries:** [T-0679] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md); [I-0362] → [`../Issues/Issue-active.md`](../Issues/Issue-active.md) (found in the Mac live pass); [I-0363] (found in the Linux live pass).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Followed by:** S6b — `.docx` import (Q1), not yet created; Q6 and Q7 below are ruled for it.
**Builds on:** [SP-207] (the Markdown export this must read back), [SP-215] (File ▸ Import), EP-029's structured paste
(`scrivi_fragment_paste` / `scrivi_fragment_uncut_paste`) and its one-step undo.
**Authority:** EP-054 AC6 (import from Markdown / text, chapters and scenes mapped, nothing dropped silently), AC7
(boundary-tested, round-trip fixtures), AC8 (surfaces on every platform), Q5 (order).
**Size:** M — one parser behind one read-only endpoint, one menu item and one report sheet on three platforms.

---

## What reading the code found (2026-11-14)

- **No manuscript import exists.** `scrivi.h` has the archive, timeline, asset and inbox imports only.
- **The insertion is already built.** `scrivi_fragment_paste` (`scrivi.h:1078`) inserts a `scrivi.fragment.v1` — pieces that
  each open a scene or a chapter (`chapterTitle`) and carry text — creating every scene and chapter, and
  `scrivi_fragment_uncut_paste` undoes it as one step. So the core needs only a **parser from file to fragment, plus a
  report**; no new create, write or undo code. A new project is `scrivi_create_project`, then the paste into its first scene.
- **The parser's parts are in the core:** md4c (`markdown/MarkdownAnalyzer`), and `markdown/Escapes` for characters that must
  stay literal.
- ⚠️ **The S3 Markdown export does not round-trip as written** (`ManuscriptExport.cpp:267`): chapter titles are `#`, the
  writer's own `#` headings inside a scene are `#`, and the title page is `# Title` → Q3.
- ⚠️ **Not measured:** a whole book (dumas: ~1,000 scenes) as one fragment through the paste — its time and the size of its
  undo step. Plan step 0 measures it before Q2 (a) is built on it.

## Goal

A writer can bring a manuscript written elsewhere into Scrivi from a Markdown or plain-text file — as a new project, or into
the open project at the caret as one undoable step — with chapters and scenes mapped, and is told about anything Scrivi could
not map. The same on the Mac, the iPad and Linux; the core parses, the platforms only ask and insert.

## Plan

0. **Measure first:** dumas exported to Markdown, built into a fragment, pasted into an empty project through `scrivi_*`:
   the time, and the undo step's size. If it is not acceptable for a whole book (EP-054's "visible progress" rule), stop and
   bring it back before step 3.
1. **Red tests** (`ManuscriptImportCApiTests.cpp`, through `scrivi_*`): Markdown per Q3, text per Q4–Q5, the report per Q8,
   the encodings, and the round trip — S3's fixtures exported, imported, exported again: the same bytes.
2. **Core:** `scrivi_import_manuscript(sourcePath, optionsJSON)` — READ-ONLY, writes nothing. optionsJSON `{format?:
   "markdown"|"text"}` (else from the extension). Result `{fragment, title?, encoding, report:[{kind, count, example}]}`.
   `ManuscriptImport.cpp` beside `ManuscriptExport.cpp`. Size cap on the source; refuses a file that is not text.
3. **Wrappers:** `ScriviEngine.importManuscript` (+ the visionOS stub, `check-engine-stub-parity.sh`);
   `ScriviBridge::importManuscript`.
4. **Surfaces** (Mac, iPad, Linux — Linux adopts the Mac's shape and wording): File ▸ Import ▸ **Manuscript…** (Q2). With a
   project open: **New Project / Into This Project** (at the caret, through the existing structured-paste path, so one undo
   step); with none: a new project, titled from the file's title page or its name, created where New Project creates one.
   The report sheet (Q8). Off the main thread, progress after 400 ms.
5. **Checks:** macOS ctest and interop; the iOS build; Linux ctest and a smoke that imports through the bridge; stub parity,
   ABI guard and boundary checks; CI read after the push.
6. **Live passes (user):** a Scrivi Markdown export imported back as a new project and into an open project (then undone);
   a `.txt` with `*` and `_` that must stay literal; a Windows-1252 file; a hard-wrapped file (the report says so).

## Questions — ✅ all ruled as recommended (user, 2026-11-14)

| # | Question | Ruling |
| - | -------- | ------ |
| **Q1** | One Sprint, or split as S5 was? | ✅ **Split.** **S6a** (this Sprint): Markdown and `.txt`, the endpoint, the menu item, the report sheet. **S6b**: `.docx`, which needs an XML reader (Q6) — a second parser behind the same endpoint and surfaces. |
| **Q2** | Where an import lands: (a) a new project, (b) the open project at the caret, (c) both. | ✅ **(c).** One fragment serves both. (b) is one undoable paste (a single chapter brought in); (a) brings in a whole book and works with no project open, as Import ▸ Archive… does. One prompt. |
| **Q3** | How Markdown maps. | ✅ `#` opens a chapter; `##`–`######` stay in the text as the writer's headings; a thematic break (`* * *`, `***`, `---`, `___`) opens a scene; text before the first `#` is an untitled first chapter. **Exactly** S3's title-page shape (`# Title`, an optional subtitle line, a `by …` line, then a `#`) is the project title, not a chapter; nothing else is guessed. ⚠️ A writer's own `#` inside a scene comes back as a chapter (the export writes it as `#`). |
| **Q4** | What `.txt` means. | ✅ **Literal text:** every Markdown character escaped, so `*` and `_` read as typed; paragraphs at blank lines; scenes at lines of only `* * *`, `***` or `#`; one chapter, **no "Chapter N" guessing**. Encoding: UTF-8 (BOM stripped), UTF-16 with a BOM, else Windows-1252 — reported. Line endings normalised. |
| **Q5** | Hard-wrapped text (every line ends in `\n`). | ✅ **Each `\n` stays a line break**, as Scrivi treats it; joining lines would wreck verse and dialogue. The report flags a file whose lines are nearly all ≤ 80 characters. |
| **Q6** | *(S6b)* The XML reader for `.docx`. | ✅ **pugixml**, pinned by FetchContent and compiled into the library as md4c and miniz are (MIT, one `.cpp`). Untrusted files (entities, CDATA, namespaces) at a trust boundary. |
| **Q7** | *(S6b)* How `.docx` maps. | ✅ Title / Heading 1 opens a chapter; Heading 2–3 stay as `##`/`###`; a scene break is S4's empty bordered Scene Break paragraph, a centred `* * *`/`***`/`#`, or a page break with no heading after it; italic / bold runs → `*` / `**`; numbered paragraphs → lists; `<w:br/>` → `\n`; tracked changes read as accepted. **Reported, never dropped silently:** images, tables, footnotes and endnotes, comments, text boxes, headers and footers. |
| **Q8** | The report. | ✅ The import returns each kind it could not map, with a count and the first example; a sheet shows it after the import. Nothing dropped, no sheet. |

⛔ **NOT in this Sprint:** `.docx` (S6b), Scrivener (S7), images as assets, chapter-detection heuristics, HTML or ePub
import, the byline as the project's authors (authors are identities, [EP-058]; the title-page `by …` line is consumed and
reported), inline object references ([EP-032]).

## Acceptance Criteria

- [ ] AC1 — Step 0 measured and recorded: a ~1,000-scene import through the paste, its time and its undo step; ruled
      acceptable before the surfaces are built — ✅ measured (log, 2026-11-14); ⏳ the ruling is the user's
- [x] AC2 — `scrivi_import_manuscript` reads Markdown per Q3 and text per Q4–Q5 into a `scrivi.range.v1` (⚠️ not a fragment: see the log), writes nothing,
      and reports per Q8 (the encoding, a hard-wrapped file, the consumed byline, anything unmapped)
- [x] AC3 — Round trip: each S3 Markdown fixture exported, imported and exported again gives the same bytes (fixtures without
      a writer's `#` inside a scene, Q3)
- [x] AC4 — Boundary tests through `scrivi_*` (EP-054 AC7): every Q3–Q5 case, the three encodings, a refused binary file, a
      file over the size cap; macOS and Linux ctest green
- [ ] AC5 — File ▸ Import ▸ Manuscript… on the Mac, iPad and Linux: New Project / Into This Project with a project open, a
      new project with none; into a project is one undo step; the report sheet; off the main thread, progress after 400 ms —
      ✅ built; ⚠️ the iPad offers New Project only — ruled for this Sprint, Into This Project → [T-0680]
- [ ] AC6 — Interop tests and a Linux bridge smoke green; engine stub parity, ABI guard and boundary checks green; CI green
      — ✅ all but CI, which runs on the push
- [ ] AC7 — **Live passes (user)** on the Mac, iPad and Linux: plan step 6

## Progress log

- **2026-11-14** — 🔵 **Created in Planning** (user: *"We're ready to begin planning S6, the Markdown/text and .docx impor."*),
  issued via `next-id.py`; [T-0679] filed. ✅ **Q1–Q8 ruled as recommended** (user, 2026-11-14: *"Q1-Q8 as recommended."*):
  S6 split into S6a (this Sprint) and S6b (`.docx`, Q6–Q7 ruled for it).
- **2026-11-14** — 🟡 **Activated** (user, 2026-11-14: *"Ok, activate andimplement SP-218"*).
- **2026-11-14** — **Step 0 measured (AC1).** ⚠️ The hosts' structured paste is no longer the fragment paste: since [SP-191]
  it is a **Range** (`scrivi_range_paste`, one undo step = `scrivi_range_cut` of what it inserted). So the importer returns a
  `scrivi.range.v1` whose chapter IDs are `~` placeholders (the paste mints real ones), and "Into This Project" is the editor's
  own `pasteStructuredRange` / `EditorShell::pasteRange` — its "Pasting…" progress, its undo, unchanged. A 100-chapter,
  1,000-scene (2 MB) Markdown file, Debug builds, local disk, `ManuscriptImportCApiTests "[.import-perf]"`:

  | | macOS (arm64) | Linux (Docker arm64, GCC 13) |
  |---|---|---|
  | read the file into a Range | 47 ms | 67 ms |
  | **into a project at the caret** (`scrivi_range_paste`) | **125 ms** | **126 ms** |
  | its undo (`scrivi_range_cut`) | 93 ms | 79 ms |
  | **as a new project** (create + fill) | **1.47 s** | **0.88 s** |

  The legacy fragment paste took 3.0 s for the same book (and 0.46 s to undo). A new project is filled through
  `FragmentPaster`, because a Range pasted at offset 0 of the new project's empty scene goes BEFORE it ([SP-191] Q7), leaving
  the project's own chapter behind; both show progress after 400 ms. The undo step holds the 2 MB Range in memory.
- **2026-11-14** — **Built** (plan steps 1–5). Core: `ManuscriptImport.cpp` (pure: decoding, blocks, Q3–Q5, the notes) and
  `scrivi_import_manuscript`. ⚠️ **A new project is created BY THE CORE** (`optionsJSON.newProject`), not by the host followed
  by a paste: renaming the project's first chapter needs its metadata path, which no endpoint returns, and the shape would
  have been written twice (Swift, C++). ⚠️ **Found while testing: a new-project import over an existing project folder
  succeeded** (`createProject` does not refuse one) — it is now refused (`error.detail "exists"`) and tested. The Mac's
  save panel and Linux's save dialog ask about replacing; on Replace the old folder goes to the **Trash** first. Empty scenes
  are left out (the export leaves them out too), except a chapter's only scene, so its heading survives.
  Apple: `ScriviEngine.importManuscript` (+ stub); File ▸ Import ▸ **Manuscript…** first in the submenu, enabled with no
  project (`ImportExportCommands`); `presentImportManuscript` / `importManuscript` / `tellNotes` (`ProjectArchiveFlow.swift`);
  `ProjectSession.importRangeAction` → the editor's `pasteStructuredRange` (`ManuscriptTextView.swift`). ⚠️ **The iPad offers
  New Project only:** its editor has no structured paste or structural undo yet (`ManuscriptUITextView.swift:12`, EP-055
  S2/S3), so Into This Project could not be one undo step there. Linux: `ScriviBridge::importManuscript`;
  `ShellController::importManuscript` / `runManuscriptImport` (`ScriviWindow.cpp`), the same items, wording and notes.
  A new project is credited to the writer's default pen name, or "Scrivi" when there is none (EP-058's author prompt then asks).
  ⚠️ Q3 as ruled: the title page needs its `by …` line, so an export with no named author imports its title as a chapter.
  **Evidence:** `ManuscriptImportCApiTests` 11 cases (264 assertions) — the round trip byte-identical (AC3); macOS ctest
  **853/853**; Linux (Docker, GCC 13, Qt 6.4) ctest **857/857** as uid 1000; interop **350/350** (2 new:
  `manuscriptImportThroughEngine`, `importedManuscriptPastesAndUndoes` — one undo takes the import back); macOS and iOS builds
  clean; Linux **43/43** smokes (`bridge_parity_smoke` 78 checks, 4 new; `settings_smoke` §6/§7 updated for Import ▸
  Manuscript…); ABI guard, stub parity, package boundary, TextKit 2, layout and type-source checks clean. ⚠️ Not yet: CI (on
  the push), scrivi-vm (did not answer) and the rig.
- **2026-11-14 — Mac live pass (user):** steps 1 and 2 pass; **step 3 fails**: "Into This Project" beeps and pastes nothing (`could not find a free chapter order key`). The project's chapters have pre-T-0358 lowercase keys (`chapter-k`, `chapter-s`), and `keyBetween` could not make a key next to one, so every chapter- or scene-creating paste in such a project failed, not only the import → filed [I-0362], fixed in `util/OrderKey.cpp`. ctest **855/855** (2 new). The new import test fails on the old `OrderKey.cpp` with the live pass's error. Step 3 to be re-run.
- **2026-11-14 — after the push (`4c1bf2a`):** Mac step 3 ✅ passes (user: *"step 3 passes now"*; the byline note shown) → [I-0362] ✅ Verified. Interop **350/350**. scrivi-vm (pulled and built by the user) ctest **860/860**. CI: Apple ✅, ScriviCore ✅, Linux App still running when logged. Still to run: Mac steps 4–6, iPad, Linux.
- **2026-11-15 — live pass (user):** Mac steps 4–6 ✅ pass (literal `.txt`, Windows-1252 note, hard-wrap note). iPad steps 1–5 ✅
  pass (new project, byline note). ⚠️ **iPad with a project open:** the import closes it and offers New Project only. That is the
  build's limitation logged above, but **it departs from Q2 (c)**, ruled for every platform, and was never put to the user.
  ✅ Ruled (user: *"option 1. And yes file the landing page as a Task."*): New Project only on the iPad for this Sprint; Into This
  Project there → [T-0680] (waits for EP-055 S2/S3); the Landing's one **Import…** button → [T-0681]. Fixed now: with a project
  open, the iPad first says why it closes (Continue / Cancel) — `presentImportManuscript`, `ProjectArchiveFlow.swift`; iOS build ✅.
  ⚠️ The pasted step list ran the Linux steps 3–4 on the iPad (iPad step 6 and Linux step 1 lost in the copy): iPad step 6 and
  every Linux step still to run.
- **2026-11-15 — live pass (user), next round:** iPad steps 1–2 ✅ pass (the new alert: Cancel keeps the project, Continue closes it;
  the existing project refused). Linux, import as a new project ❌: the save dialog's Save went grey and nothing happened → filed
  [I-0363], fixed to Import Archive's name-then-folder dialogs (`ScriviWindow.cpp`); compiled on scrivi-vm, not yet run.
