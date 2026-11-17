---
sprint: SP-220
epic: EP-054
status: Active
platform: Cross
created: 2026-11-17
---

# SP-220 — `[ScriviCore]` + `[Apple]` + `[Linux]` [EP-054] **S7**: Scrivener import

**Status:** 🟡 **ACTIVE** 2026-11-17 (user, 2026-11-17: *"activate and implement SP-220"*) · planned 2026-11-17 (user, 2026-11-17: *"plan S7 Scrivener import"*) · ✅ Q1–Q6, Q3a, Q7a ruled as recommended.
**Carries:** [T-0683] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) (AC6, AC7, AC8; Q5 put Scrivener last as the most complex format).
**Builds on:** [SP-218] (S6a: `scrivi_import_manuscript`, the Markdown reader, the notes, New Project / Into This Project), [SP-219]
(S6b: pugixml, `.docx` → Markdown → S6a's reader), [SP-205] (miniz), [SP-191] (a Range entry may carry `sceneMeta`).
**Size:** L — a new text format (RTF) and a structure that comes from a binder, not from headings.

---

## What reading the code found (2026-11-17)

- **The endpoint takes a FILE.** `scrivi_import_manuscript` (`scrivi_c_api.cpp:1383`) reads one file (64 MB cap) and picks the
  format from its bytes or extension. A `.scriv` is a **folder** (a package on the Mac), so the endpoint must learn a directory.
- **The structure is in the binder, not the text.** S6a and S6b find chapters and scenes from headings and breaks in one text.
  A Scrivener project already HAS them: its binder is a tree of folders and documents, each document its own file. So S7 builds
  the Range from the binder, and only each document's body goes through S6a's paragraph and escape rules — the reverse of S6b,
  where Markdown was the route to the structure.
- **Scrivi can hold more than a title per scene.** A Range entry may carry `sceneMeta` (`scrivi.h:1150`, [SP-191]), and a scene's
  sidecar has `outline` ("freeform synopsis prose") and `tags` (`SceneMetaJson.hpp:57–58`). Scrivener's synopsis and keywords
  have a place to go (→ Q4). Scene `status` is `draft | revised | final | archived` (schema §7.5).
- ⚠️ **No RTF reader exists in ScriviCore**, and Scrivener stores every document as RTF (→ Q5).
- ⚠️ **No Scrivener on this Mac and no `.scriv` anywhere on it** (`mdfind`, 2026-11-17). Literature & Latte publish no format
  specification. What follows about the format is **from my knowledge, unconfirmed**, and must be checked against real projects
  before code is written (→ Q7, plan step 0):
  - **Scrivener 3** (Mac and Windows): `<Name>.scrivx` (XML) holds the binder — `BinderItem` elements with a `UUID`, a `Type`
    (`DraftFolder`, `ResearchFolder`, `TrashFolder`, `Folder`, `Text`, and media types), a `Title`, `MetaData`
    (`IncludeInCompile`, label and status IDs), `Keywords`, and nested `Children`. Each document's files are in
    `Files/Data/<UUID>/`: `content.rtf`, `notes.rtf`, `synopsis.txt`.
  - **Scrivener 2** (Mac) and **Scrivener 1** (Windows): binder items have a numeric `ID`; files are `Files/Docs/<ID>.rtf`,
    `<ID>_notes.rtf`, `<ID>_synopsis.txt`.
  - Also present: snapshots, inline annotations and footnotes and comments, document links, images inside the RTF, the
    Research folder's PDFs and pictures, label and status lists, custom metadata, and compile settings.
- **The pickers.** The Mac open panel can choose a package as one item; Qt's file dialog and the iPad's document picker choose a
  file OR a folder, not both in one dialog (→ Q6).

## Goal

A writer can bring a Scrivener project into Scrivi — as a new project, or into the open project at the caret as one undo step —
with its chapters and scenes as the binder has them, their titles, italics and bold, and is told about everything Scrivi could not
keep. The same on the Mac, the iPad and Linux.

## Plan

0. **Confirm the format** on the Q7 fixtures before any code: the binder's elements, the file layout per version, the RTF Scrivener
   writes (its control words, footnotes, annotations, comments). Corrections go into this record.
1. **Red tests** (`ManuscriptImportCApiTests.cpp`, through `scrivi_*`): every Q1–Q4 case, the refusals, the Q7 fixtures, and the
   RTF reader on synthetic input — every control word it handles, Unicode and code pages, nested groups, skipped destinations, and
   truncated and mangled RTF under the sanitizers with no crash.
2. **Core:** `manuscript/ManuscriptRtf.cpp` (Q5): RTF → Markdown paragraphs, italic and bold as `*` / `**`, S6a's escaping, its own
   notes. `manuscript/ManuscriptScrivener.cpp`: read the binder with pugixml, walk the Draft folder per Q2–Q3, read each document's
   RTF, and build the same `scrivi.range.v1` and report as S6a, with `sceneMeta` per Q4. The endpoint accepts a `.scriv` folder,
   and a zip holding one (Q6), with a total size cap; `format: "scrivener"`. `scrivi.h` documented.
3. **Surfaces** (Q6): the Mac, iPad and Linux pickers; the notes worded on both (`tellNotes`, `importNotes`). The iPad still
   makes a new project only ([T-0680]).
4. **Checks:** macOS ctest and interop; the iOS build; Linux ctest (scrivi-vm) and the bridge smoke; stub parity, ABI guard,
   boundary checks; the RTF reader over mangled input under the sanitizers; CI read after the push.
5. **Live passes (user):** a real Scrivener project brought in as a new project and into an open one, then undone, on the Mac and
   Linux; as a new project on the iPad.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Which Scrivener versions.** | **Scrivener 3 (Mac and Windows) and the older layout (Mac 2, Windows 1).** Only the file lookup differs (`Files/Data/<UUID>/content.rtf` vs `Files/Docs/<ID>.rtf`); the binder and the RTF are the same in kind, and Windows 1 projects are common. Anything else is refused with "Open it in Scrivener 3 and import that". |  ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q2** | **What comes in.** | **The Draft folder only** (Scrivener's "Manuscript"). Research, Trash, and folders outside the Draft (character and setting sheets) are **reported, not imported**: their names and counts in one note. Documents marked *Include in Compile: No* **are** imported (nothing dropped silently), and named in a note. ⚠️ Mapping character and place sheets to Scrivi objects is a separate, later question, not guessed here. |  ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q3** | **The binder's depth.** Scrivi has chapters and scenes; a Scrivener Draft can nest Part ▸ Chapter ▸ Scene, or put documents straight in the Draft. | **Every text document is a scene; its nearest folder is its chapter.** A text document with children is a chapter named by it, its own text that chapter's first scene. Documents straight in the Draft are grouped into an untitled chapter per run, as S6a's text with no headings. Folders above a chapter (Parts) are flattened: **one note** names them. An empty folder becomes an empty chapter. |  ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q4** | **The metadata.** | **Titles** → scene and chapter titles. **Synopsis** → the scene's `outline`. **Keywords** → `tags`. **Status:** Scrivener's default names map (*First Draft* → `draft`, *Revised Draft* → `revised`, *Final Draft* / *Done* → `final`); any other status and every **label** are reported, not kept. **Notes** (`notes.rtf`), snapshots, custom metadata, comments, annotations and footnotes: reported with a count, as S6b's notes. |  ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q5** | **The RTF reader.** | **Our own small reader in the core,** as Epic Q6 puts formats in the core. It handles groups, `\par` / `\line`, `\i` / `\b` and their `0` forms, `\uN` with `\ucN`, `\'hh` by `\ansicpg`, and skips the `\*` destinations and the font, colour, style and info tables. Pictures, footnotes, annotations and other formatting are reported (S6b's Q4 rule). No library: I know of no maintained single-file C/C++ RTF reader to pin as miniz and pugixml were. Apple reads RTF (`NSAttributedString`) but Qt does not, and the import must read the same on every platform. |  ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q6** | **How the writer picks it.** | **Its own item, File ▸ Import ▸ Scrivener Project…,** on all three, so each platform can open the right picker: the Mac a package, Linux and the iPad a folder. Scrivener's zipped backups (a `.zip` holding one `.scriv`) are accepted by the same item and found by their bytes. Manuscript… stays as it is. |  ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q7** | **Test files from real Scrivener.** There is no Scrivener on this Mac. | **You provide them:** one Scrivener 3 project made for the purpose (no writing of yours), holding the Q2–Q4 cases (Parts, chapters, scenes, a document straight in the Draft, an excluded document, a synopsis, keywords, a status, notes, a footnote, a comment, a picture, italics and bold, Research and Trash items), plus one Scrivener 2 or Windows 1 project if Q1 keeps the older layout. The Scrivener 3 trial runs 30 days of use, and it can save a copy as Scrivener 2. They are committed as fixtures, as Q5 of [SP-219] intended. | |

| **Q3a** | ➕ *(step 0, 2026-11-17)* **Text documents with children.** The tutorial nests text inside text three deep (*Main Interface* ▸ *The Inspector* ▸ *Synopsis & Notes*), and its own Section Types treat a text with children as **Text**, not a heading (`LevelTypes`: Folders → Heading, Containers and Files → Text). Q3 as ruled makes each such text a chapter, which splits a folder's documents into two chapters around it (*Get Oriented* would become *Main Interface*, *The Inspector*, then a second *Get Oriented* holding only *Composition Mode*). | **Amend Q3: only a FOLDER is a chapter.** Every text document, with children or not, is a scene, in binder order, in its nearest folder's chapter. Matches Scrivener's own default Section Types, never splits a chapter, and is simpler. | ✅ **RULED as recommended** (user, 2026-11-17) |
| **Q7a** | ➕ *(2026-11-17)* **The fixtures.** The tutorial is Literature & Latte's text, so it should not be committed. | **Use the tutorial only for uncommitted local runs and step 0. You make one short project in Scrivener** from a test text I write (no writing of yours) holding the cases below, and, if Scrivener 3 can export one, a Scrivener 2 copy of it for Q1. | ✅ **RULED as recommended** (user, 2026-11-17) |

⛔ **NOT in this Sprint:** character and setting sheets as Scrivi objects (Q2); Research files as assets; snapshots as history;
compile settings; Into This Project on the iPad ([T-0680]); inline object references ([EP-032]).

## Acceptance Criteria

- [x] AC1 — `scrivi_import_manuscript` reads a `.scriv` (folder or zipped backup) per Q1–Q4 into the same `scrivi.range.v1` and
      report as S6a, writes nothing without `newProject`, and refuses what Q1 does not read
- [x] AC2 — The RTF reader per Q5: every handled control word, Unicode and code pages; mangled and truncated RTF never crash
- [x] AC3 — Boundary tests through `scrivi_*`: every Q1–Q4 case, the Q7 fixtures, every refusal, and mangled input under the
      sanitizers with no crash; macOS and Linux ctest green — ✅ macOS 876/876; ✅ Linux (scrivi-vm) 881/881
- [x] AC4 — File ▸ Import ▸ Scrivener Project… on the Mac, iPad and Linux (Q6) — ✅ built on all three
- [ ] AC5 — Interop tests and the Linux bridge smoke green; stub parity, ABI guard and boundary checks green; CI green — ✅ all but CI (after the push)
- [ ] AC6 — **Live passes (user)** on the Mac, iPad and Linux: plan step 5

## Progress log

- **2026-11-17** — 🔵 **Created in Planning** (user, 2026-11-17: *"plan S7 Scrivener import"*), issued via `next-id.py`;
  [T-0683] filed. Q1–Q7 to rule.
- **2026-11-17** — ✅ **Q1–Q6 ruled as recommended** (user, 2026-11-17: *"Q1-Q6 as recommended, I installed Scrivener (the name for
  my app is alarmingly similar to Scrivener.  We should address that at some point).  There is a tutorial project at
  ~/Documents/Scriviner/Tutorial.scriv"* — the path is `~/Documents/Scrivener/Tutorial.scriv`). Q7 → Q7a.
- **2026-11-17 — step 0 on the tutorial** (Scrivener 3.5.2, `Creator="SCRMAC-3.5.2-17486"`, `Version="2.0"`; read only):
  ✅ confirmed: `Tutorial.scrivx` holds the binder (`BinderItem` with `UUID`, `Type`, `Title`, `MetaData/IncludeInCompile`,
  `LabelID`, `StatusID`, `Keywords/KeywordID`, `Children`); files in `Files/Data/<UUID>/` — `content.rtf` (75), `synopsis.txt`
  (18), `notes.rtf` (2), and media as `content.jpg` / `.pdf` / `.wav`. Top level: two Text items OUTSIDE the Draft
  (*START HERE*, *Key Concepts*), then `DraftFolder`, `ResearchFolder`, `TrashFolder` (Q2 covers them). Statuses and labels
  are project lists (`Status`, `Label` with IDs): the defaults are *No Status, To Do, First Draft, Revised Draft, Final
  Draft, Done* — Q4's names hold.
  ➕ **New:** **comments and footnotes are NOT in the RTF**: they are `content.comments` (XML, each `Comment` an RTF CDATA,
  `Footnote="Yes"` for a footnote) anchored in the RTF by `HYPERLINK "scrivcmt:…"` fields. Document links are
  `HYPERLINK "scrivlnk:…"` (30). The RTF also carries lists (`\listtext`, `\ls`), tables (`\trowd` … `\cell` … `\row`),
  pictures (`\pict` in `\shppict`), `\strike`, `\ul`, centring (`\qc`), and Cocoa's own words (`\cocoartf`,
  `\expandedcolortbl`). The reader keeps a field's result text (`\fldrslt`) and reports the link; a list item's text stays,
  its marker from `\listtext` kept as typed; a table's cell text kept, reported, as S6b. A text with children → Q3a.
  ⚠️ Not yet seen: the Scrivener 2 layout (Q1) — no such project exists here.
- **2026-11-17** — ✅ **Q3a, Q7a ruled as recommended** (user, 2026-11-17: *"Q3a and Q7a as recommended, yes file the name Task"*).
  Q3 now reads with Q3a: only a folder is a chapter. The Q7a test text is in the chat reply; the user makes the project.
- **2026-11-17 — Q7a fixtures made (user)** (*"the test project is ready.  The older format was availble I think scrivener put
  Copy in the file name."*). Checked against the script, both projects: ✅ every case present — the binder as scripted (Parts,
  chapters, *Customs* with children, documents straight in the Draft, *Empty Chapter*, *Left Out* with no `IncludeInCompile`);
  Dock's synopsis, keywords (`KeywordID` → the project's `Keyword` list), status *First Draft*, label, notes; Customs' *To Do*;
  Stalls' list, comment and footnote (`content.comments`, `Footnote="Yes"`); Voyage's underline, table, picture (a JPEG of
  Scrivi's own artwork) and `scrivlnk:` link to Dock. Two extras beyond the script, harmless: *Before Anything* also has a
  label and status; Trash holds an untitled empty item and an accent-character test.
  ✅ **Scrivener 2 layout confirmed** on the export: `.scrivx` `Version="1.5"` (3's is `2.0`), `Files/version.txt` `18` (3's is
  `23`), each `BinderItem` has BOTH `UUID` and a numeric `ID`, files `Files/Docs/<ID>.rtf`, `<ID>_notes.rtf`,
  `<ID>_synopsis.txt`, `<ID>.comments`. The `.scrivx` inside keeps Scrivener's name (`sp220-every-case v2 Copy.scrivx`): the
  reader finds the one `.scrivx`, never by the folder's name.
  Committed as `ScriviCore/tests/fixtures/scrivener/sp220-every-case.scriv` and `sp220-every-case-v2.scriv`, less
  `Files/search.indexes` (Scrivener's, rebuilt). ⚠️ **One file changed:** the Research item held a Project Gutenberg text
  (600 KB); Q2 never reads Research, so its RTF is a one-line stub in both copies (Scrivener's `docs.checksum` no longer
  matches it — irrelevant to the reader). The originals stay in `~/ScriviLinux/projects/imports`.
- **2026-11-17** — 🟡 **Activated** (user, 2026-11-17: *"activate and implement SP-220"*).
- **2026-11-17 — Built.** Core: `manuscript/ManuscriptRtf.cpp` (Q5) — groups (depth capped at 1,024), `\par` / `\line` /
  `\tab`, `\i` / `\b`, `\uN` + `\ucN` (surrogate pairs joined), `\'hh` as Windows-1252, the quote and dash words;
  the font, colour, style, list and info tables, every `\*` destination, footnotes, annotations, headers and footers skipped
  (their own `\par` never ends the writer's paragraph); pictures, tables, footnotes, annotations and each field's
  `HYPERLINK` target counted; formatting = underline, strikethrough, caps, super/subscript, a colour that is not black,
  centred or right alignment, and a face (font|size) not the project's most common — ⚠️ judged from PLAIN runs only: macOS
  writes italic and bold in their own fonts (Palatino-Italic), which otherwise flagged every emphasised paragraph.
  `manuscript/ManuscriptScrivener.cpp` — the binder by pugixml (Version 2.x → `Files/Data/<UUID>/`, 1.x → `Files/Docs/<ID>`,
  else "notScrivener"); an ID becomes a path only when it is hex digits and `-` (never `/` or `..`); Q2–Q4, Q3a as ruled;
  each paragraph through the .docx's `inlineMarkdown` (now shared), so the text is literal exactly as S6b's. A folder
  (`FileSystem`) or a zip (miniz, from the file); each file read capped at 64 MB. The endpoint takes a folder or a `.zip`
  (format "scrivener"); a trailing `/` on the folder is dropped. **The seed (Q4):** an entry with NO sceneID and a
  `sceneMeta` gives the new scene its outline, tags and status — in `scrivi_range_paste` (`RangeOperations.cpp`) and in a
  new project's paste (`FragmentPaster`: `FragmentPiece::sceneMeta`, and `firstPieceNamesTarget` so the project's first
  scene takes the first document's title and seed). An entry WITH an ID is unchanged (SP-191 Q3b). `scrivi.h` documented.
  Surfaces: Apple `presentImportManuscript(_:scrivener:)` — the Mac panel takes a `.scriv` package, a folder or a `.zip`; the
  iPad picker the same types, then S6a's folder step; the eleven new note kinds worded (`tellNotes`). Linux: Import ▸
  Scrivener Project… asks **Project Folder… / Zipped Backup…** first (Qt's dialogs choose a folder or a file, never both),
  then S6a's flow; `importNotes` worded. The settings smoke's menu lists and the bridge-parity smoke (a `.scriv` read
  through the bridge, its seed intact) updated.
  ⚠️ Beyond the letter, to rule if wrong: (1) a paragraph of only `* * *`, `***` or `#` inside a document opens a scene,
  as S6a's text does — writers who keep one document per chapter mark their scenes so; (2) a folder's documents that come
  AFTER a nested folder start a second chapter with the same title (reading order is kept; never seen in the fixtures);
  (3) empty documents are KEPT as scenes (each is titled, and may carry a synopsis), unlike S6a's empty scenes.
  **Tests:** 10 `[scrivener]` cases in `ManuscriptImportCApiTests.cpp` — the structure (Q3, Q3a), the RTF reader (every
  handled word, a footnote mid-sentence, colour), the metadata and every note (Q2, Q4), the refusals (Q1, no Draft, not XML,
  a non-zip, a sparse 64 MB + 1 file), a path-escaping ID, both fixtures giving the SAME Range (Q1, Q7a), a zipped backup
  (with a `__MACOSX/` entry), a new project's sidecars, into a project and cut back, and mangled input (the project file
  truncated, an RTF truncated and bit-flipped, 5,000-deep groups). Mutation-checked in a copy of the repo — each of 12
  mutations fails a test (the ID guard, both seed paths, the first scene's title, Q3a, a skipped destination's words, the
  `\uN` fallback, black as colour, italic fonts as faces, in-document breaks, empty folders, the v2 layout); a `__MACOSX/`
  check that no test could fail was redundant (the one-folder-deep rule already excludes it) and was REMOVED.
  ctest **876/876**; the import tests under ASan + UBSan ✅ (33 cases); interop **352/352** (1 new:
  `importedScrivenerPastesWithItsSynopsis` — the seed survives `ScriviRange`, one undo takes it back); iOS build ✅; stub
  parity, ABI guards (135), boundary ✅. ⚠️ **Not run:** the Linux build, ctest and smokes — scrivi-vm stopped answering
  mid-session (no ping) and Docker is not running.
- **2026-11-17 — Linux (scrivi-vm, a copy of the working tree; the VM back up):** build ✅; ctest **881/881**; smokes
  **43/43** (`bridge_parity_smoke` 80 checks, 2 new: the `.scriv` read through the bridge with its seed; `settings_smoke`'s
  Import lists with Scrivener Project…). CI follows the push. Left: AC6, the live passes.
