---
sprint: SP-174
epic: EP-048
status: Closed
closed: 2026-10-10
activated: 2026-10-10
platform: Cross
created: 2026-10-10
---

# SP-174 — `[Cross]` [EP-048] **S3a**: the Format-command and balanced-edit logic moves into ScriviCore; Apple re-pointed

**Status:** ✅ **CLOSED 2026-10-10** (user: *"close SP-174"*; live pass passed). Activated 2026-10-10 (user: *"activate SP-174 and start step 1"*) — created 2026-10-10 (user: *"begin S3 planning"*); Q1–Q5 ruled the same day.
**Carries:** ✅ [T-0601] (Verified) → [`../../Tasks/Verified/Task-verified-0601.md`](../../Tasks/Verified/Task-verified-0601.md).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Authority:** EP-048 **L7** (commands, as EP-046 AC6; Alt-Return = hard break) and **L9**'s rest (balanced edits and copy, as
EP-046 AC12); the hanging-indent ruling carried from [SP-166]. Apple reference: [SP-163] (commands, lists, pending pair, T-0591),
[SP-162] (AC12, balanced edits).
**Size:** M–L. ✅ Needs no rig: S3a changes nothing visible; its proof is Apple's suites plus a short Mac live pass. The Linux commands are **S3b** (planned after this Sprint).

---

## What Linux must match (Apple, as built and verified)

| Behaviour | Apple (where) | Linux today |
| --------- | ------------- | ----------- |
| Bold ⌘B / Italic ⌘I as a STYLE edit: toggle, `*` never `_`, whitespace trimmed, per paragraph, nesting kept, flank rule (shrink to what can carry it) | `ManuscriptCommands.emphasisEdit` + `MarkdownEmphasis` (tokens → normalise → serialise) | none |
| Caret in a word → format the word; between words → a PENDING pair (kept if typed into; removed if left empty; never saved, never in history) | `ManuscriptTextView.applyFormat`, `insertPendingPair` | none |
| Heading 1–3 ⌥⌘1–3, Body ⌥⌘0, Bulleted ⌥⌘L, Numbered ⌥⌘N, per paragraph; a paragraph is ONE of body / heading / list item | `ManuscriptCommands.paragraphEdit` | none |
| Return in a list item continues the list (`\n\n` + next prefix, numbers kept sequential); Return on an empty item ends it | `ManuscriptCommands.listReturnEdit`, `renumbered` | none |
| Alt-Return stores a hard break `\` + `\n` | `insertNewlineIgnoringFieldEditor` | ✅ storage half done ([SP-163], shared corpus) |
| A cut/copy/⌫/⌦/type/paste over a selection that partly covers a span SPLITS it (closed and reopened), never unbalances it; the cut text carries its own markers; other apps get the presented text | `ManuscriptPresenter.balancedEdit`, `balancedCopy`, `balancePastePieces` (+ cross-scene, T-0591) | presented text to other apps ✅ ([SP-160]); ⌫/⌦ never delete a hidden marker alone ✅ ([SP-166]); balancing ❌ |
| List prefix visible + dimmed, with a hanging indent | `headIndent` = prefix width | dimmed ✅; ⛔ no hanging indent (a block format is a document edit) — ruling owed (Q3) |

## ✅ Rulings — 2026-10-10 (user)

| # | Ruling |
| - | ------ |
| **Q1** | ✅ **Move the logic into ScriviCore** — one implementation; both platforms call it |
| **Q2** | ✅ **Two Sprints:** S3a (this) = core + Apple re-pointed; S3b = Linux commands + rig live pass |
| **Q3** | ✅ **Accept the difference** — no hanging indent for list items on Linux, recorded as a known difference |
| **Q4** | ✅ **Move the navigator toggle** — Numbered List takes Ctrl+Alt+N (Apple's ⌥⌘N); the navigator gets a new key, chosen after checking GNOME's bindings on the rig (S3b) |
| **Q5** | ✅ **The pending pair follows Apple's rule on Linux** — editor only; never saved by autosave, never in history; removed if left empty (S3b) |

## Questions as raised

- **Q1 — Where the editing logic lives.** All of the above is pure text transformation (source text + selection + command →
  new text + new selection), ~650 lines of Swift today. [SP-163]'s T-0591 ruling put balancing Apple-side *because the core had
  no parser*, and said: *"Revisit when EP-048 brings md4c into the core (the core could then own ALL balancing, for both
  platforms)."* md4c is in the core now ([SP-165]).
  - **(a) Move it into ScriviCore** as text-transform endpoints both platforms call (Apple re-pointed in the same work, its
    AC6/AC12 suites as the regression net). One implementation; CLAUDE.md: *"No backend logic is reimplemented in Swift."*
  - **(b) Port the Swift to Qt/C++** in `platforms/linux` — a second copy that must stay identical (as SP-166 did for the caret
    rules).
  - **Recommend (a).** It is what T-0591 deferred, and it is the only option with one rule for both platforms.
- **Q2 — One Sprint or two.** (a) one Sprint: core + Apple re-point + Linux. (b) two: S3a core + Apple re-point (no visible
  change on the Mac; its suites prove parity), then S3b Linux commands + live pass. **Recommend (b)** if Q1 = (a): the Apple
  re-point is a regression risk worth closing on its own.
- **Q3 — The hanging indent for list items on Linux** (carried from [SP-166]): (a) accept the difference (prefix dimmed, wrapped
  lines start at the margin); (b) implement it as a `QTextBlockFormat` — a document edit, so it must be kept out of the dirty
  flag, history and saving. **Recommend (a) for now**, recorded as a known difference; (b) needs its own measurement.
- **Q4 — Linux shortcuts.** Ctrl in place of ⌘: Ctrl+B, Ctrl+I, Ctrl+Alt+1/2/3, Ctrl+Alt+0, Ctrl+Alt+L, Ctrl+Alt+N.
  ⛔ **Conflict:** Linux binds **Ctrl+Alt+N** to *Show Scene Navigator* (`ScriviWindow.cpp:235`). ⚠️ Also check GNOME's own
  bindings on the rig (Ctrl+Alt+L has locked the screen on some Ubuntu setups). Options: move the navigator toggle, or give
  Numbered List another key. **Ruling owed.**
- **Q5 — The pending pair on Linux.** Apple keeps it in the text view only (never in the scene text, so autosave cannot write
  it, and never in history). Linux's autosave works from the scene document: the same rule must hold there. **Confirm.**

## Plan 1 in detail (read from the Swift, 2026-10-10)

The logic is ~650 lines: `MarkdownEmphasis.swift` (the token model: tokens → normalise → serialise with the fewest markers;
`sameRendering`; `presented`), `ManuscriptCommands.swift` (word range, emphasis edit, paragraph edit, list Return, renumbering,
paste pieces) and the presenter's balancing core (`tokens(in:)`, `balancedEdit`, `insertionStyle`, `rewrite`, `balancedCopy`).
It works on the manuscript STORAGE, but a block never crosses a scene, so the core takes ONE SCENE's text.

- **Representation:** UTF-16 units and UTF-16 offsets, as both platforms index text (the Swift algorithm is defined on UTF-16
  units, so a byte-for-byte port keeps its results). The md4c analysis (UTF-8 bytes) is mapped to UTF-16 once, in the core —
  the rule Linux's bridge uses today (every byte of a scalar → its first unit; a range end → the unit after the scalar).
- **1a** `markdown::analyzeUtf16` — the UTF-16 analysis. **1b** `markdown::Emphasis` — the token model. **1c** `markdown::Escapes`
  — the escape map (`presented`, hidden backslashes), ported from the existing Linux C++ (Apple and Linux each have a copy today).
  **1d** `markdown::Editing` — scene-level edits: word range, emphasis edit, paragraph edit, list Return, balanced replace,
  balanced copy, insertion style, paste pieces. **1e** Unicode word/punctuation classes for the word range (Apple uses
  `CharacterSet.alphanumerics` / `punctuationCharacters ∪ symbols`): a generated table of the Unicode general categories.
- **ABI:** pure functions on text (no project root, so no project lock): each returns ONE edit `{range, replacement, selection}`
  in UTF-16 offsets, or a refusal.
- **Parity (Plan 2):** Apple's own implementation generates a fixture of cases (input, operation → Swift's exact output) BEFORE it
  is removed; the core must reproduce every one.

## Plan (S3a)

1. **Core:** text-transform endpoints over a block's source text — emphasis edit (toggle, flank rule, pending-pair case),
   paragraph format edit (heading / body / lists, one-of), list Return (continue / end, renumber), balanced replacement and
   balanced copy (both directions, nested; cross-scene pieces). Built on the md4c analysis already in the core. Each a pure
   function: no file access.
2. **Core tests:** port Apple's AC6 / AC12 / list / pending-pair cases as core tests (exact bytes), plus the S5 corpus (2,000
   selections × `**`/`*`) — the core must give the SAME bytes Apple gives today, checked against the Swift implementation before
   it is removed.
3. **Apple re-point:** `ManuscriptCommands` / `balancedEdit` / `balancedCopy` call the core; the Swift logic is deleted.
   Apple's interop suites (AC6, AC12, lists, pending pair, T-0591) must pass unchanged.
4. **Live pass (Mac):** a short re-run of SP-163's command steps — nothing should look different.

## Progress log

- **2026-10-10** — created in 🔵 Planning; Q1–Q5 raised.
- **2026-10-10** — Q1–Q5 ruled (core; two Sprints; accept the hanging-indent difference; move the navigator toggle; pending pair as Apple). Planning complete.
- **2026-10-10** — activated; T-0601 created. Plan 1 (core text transforms) started.
- **2026-10-10 — Plan 1: 1a–1c in the core.** `markdown/MarkdownText` (UTF-16 types; `analyzeUnits` maps the md4c analysis to
  UTF-16 units — the rule Linux's bridge used), `markdown/Escapes` (the escape map, ported from Apple's `ManuscriptEscapes`;
  checked line for line against the Swift), `markdown/Emphasis` (Apple's `MarkdownEmphasis`, line for line: tokens, normalise,
  serialise, presented, sameRendering). Compiles with no warnings. Tests `MarkdownEditingCoreTests` (7 cases, the examples Apple's
  comments document, plus a surrogate-pair mapping): all pass. ⚠️ Risk recorded: Apple's EDITS will use the core's md4c analysis
  while its RENDERING still uses Apple's parser; L2 found them agreeing except in known cases (I-0281). The parity fixture (Plan 2)
  shows whether any reach the edits. ⏳ Next: 1d (scene-level edits) and 1e (Unicode classes for the word range).
- **2026-10-10 — Plan 1: 1d and 1e in the core.** `markdown/Editing` — Apple's `ManuscriptCommands` and the presenter's balancing
  core, line for line, on one scene's text (`Scene`: block lookup reproducing Apple's `sceneLine` — a line includes its newline; a
  scene's last line "ends the scene" when no newline follows it): word range, emphasis edit, paragraph edit, list Return,
  renumbering, balanced edit, insertion style, balanced copy, paste pieces. `markdown/UnicodeClasses.hpp` — GENERATED by
  `scripts/gen-unicode-classes.py` (Apple's `CharacterSet.alphanumerics` = L/M/N and `punctuationCharacters ∪ symbols` = P/S, BMP
  ranges; ⚠️ Python's Unicode 13.0 — differs from Apple's newer Unicode only for characters added since). No warnings.
  - Tests: 7 more cases ported from Apple's interop tests with Apple's exact expected TEXT (⌘B / ⌘I / toggle / split, word for a
    caret, headings Q5, lists Q4 + Return continue / end, AC12 cuts and copy, Q3 typing / Return / ⌫). ctest 701/701.
  - ⚠️ Two of Apple's expectations are the VIEW's, not the function's: after ⌘B the view shows {5,15} and after Return in bold the
    caret 10 — Apple's `setSelectedRange` override snaps to marker homes. The function returns {7,13} and 8 (hand-derived from the
    Swift; asserted). Plan 2's fixture, generated by the Swift FUNCTIONS, confirms them.
- **2026-10-10 — the C ABI.** Eight endpoints, offsets in UTF-16 units, no project root (pure functions, so no lock):
  `scrivi_markdown_word_range`, `_emphasis_edit`, `_paragraph_edit`, `_list_return_edit`, `_balanced_edit`, `_balanced_copy`,
  `_style_at_caret`, `_balance_paste_pieces`. Bad ranges and unknown formats return an error envelope, never a read past the text.
  Swift: `ScriviEngine.markdown*` statics (+ stubs). Tests through `scrivi_*`: `MarkdownEditingCApiTests` (an emoji proves the
  offsets are UTF-16).
- **2026-10-10 — Plan 2: parity.** `MarkdownEditingParityTests` (interop) runs Apple's Swift functions and the endpoints on the
  same inputs: 12 scenes (headings, lists, nested emphasis, escapes, a hard break, accented text and emoji) × 60 seeded rounds ×
  every operation = **5,661 cases**. Selections snap to whole characters (a text view never splits a surrogate pair). It does not
  use the S5 corpus file; this random corpus covers the same operations on more varied text.
  - Found and fixed in the core while getting there: a lone surrogate became invalid UTF-8 (now U+FFFD, as Swift does).
  - ⚠️ **152 cases differ by I-0284** (all in the hard-break scene): Apple's parser renders no emphasis in a block with a hard
    break, md4c does. Expected; accepted with I-0284.
  - ⚠️ **1 case differs by the SYMBOL class** (SP-165 Q4: md4c counts an emoji, S*, as punctuation). Pasting Scrivi's own
    `a *i* c` before a bolded emoji: Swift writes `c**👋 text**` — Apple's parser renders it, md4c does not. The core first fell back
    to stripping EVERY style (the pasted italic too). ✅ Fixed in the core: the emphasis normaliser now classifies a
    supplementary-plane character as one scalar by md4c's rule (generated table, P*/S*), and moves the edge past it —
    `c👋 **text**`, which both parsers render. The BMP class is unchanged (Apple's), so no agreeing case can move.
  - ✅ The fixture: `ScriviCore/tests/fixtures/markdown_editing_parity.jsonl` pins the CORE's answer for all 5,661 (Apple's,
    wherever they agree; differing lines also carry `swift` and `why`). Replayed through `scrivi_*` by a core test (proven to fail
    on a corrupted answer), so the answers stay checked after the Swift is deleted. The interop suite fails if any difference is
    unexplained, and if a symbol-class entry starts agreeing.
  - ctest 708/708.
- **2026-10-10 — Plan 3: Apple re-pointed.** The presenter keeps its `in: storage` signatures, so no call site changed:
  - `sceneRange(in:containing:)` finds the scene text holding a range: the stretch `sceneBoundaries` slices for the save,
    between dividers, with chapter titles and a divider's own line excluded. `inScene` hands the core that text and moves
    offsets back. A range reaching into a divider or a title gets nil.
  - `ManuscriptCommands.swift` (word range, ⌘B/⌘I, headings/body/lists, list Return, style at caret, paste pieces) and the
    presenter's `balancedEdit` / `balancedCopy` now call `ScriviEngine.markdown*`. A copy across scenes is copied scene by
    scene, with the dividers and titles between them raw, as the Swift did.
  - Deleted: the Swift balancing core (`tokens(in:)`, `rewrite`, `insertionStyle`, `spans(in:)`), the command logic
    (paragraph kinds, renumbering, block walking), and `MarkdownEmphasis`'s writing half (normalise, serialise, sameRendering,
    presented, mayHaveMarkers). Kept: `MarkdownEmphasis`'s reading half (the Navigator's `searchable`, and the tokens Apple's
    tests read results with), and `isWordUnit` (those tests' oracle).
  - The parity suite was replaced by `MarkdownEditingSceneTests`: scene lookup around dividers, titles, an empty-scene caret
    and storage end; every command on a scene after a divider and a title equals the core's answer moved by the scene's
    offset; refusal across a divider; a cross-scene copy.
  - ⚠️ One audible difference: a Format command on a selection that crosses a divider or a title now BEEPS. Before, the
    Swift produced an edit the view's divider and title guard then silently refused.
  - Noted, unchanged: Return at the very END of a scene's last list item (no newline after it) is a plain Return. The
    Swift treated a caret on the divider as "the divider itself", and the core keeps that rule.
  - Results: app builds with no warnings; interop 274/274 (AC6, AC12, lists, pending pair, T-0591 unchanged); ctest macOS
    708/708; Linux (Docker, GCC) ctest 712/712, smokes 27/27.
- **2026-10-10 — the two GCC `-Wformat-truncation` warnings fixed** (user: *"fix the two GCC warnings too"*): the scene-key
  buffers in `OpenProjectReadAmplificationTests.cpp` and `StructuralOpIndexRebuildTests.cpp` were 8 bytes for a `%03d` of an
  `int`; now 12 (any int). Linux build: 0 warnings; ctest Linux 712/712, macOS 708/708.
- ✅ **2026-10-10 — Plan 4: Mac live pass PASSED** (user: *"Live Pass (Mac): 1-7 passes."*). The SP-163 steps (⌘B/⌘I toggle,
  split and nest; word-for-caret and the pending pair; headings, body, lists, Return and renumbering; Option-Return; the
  cross-scene cut and paste; undo), plus the new ones (a cross-divider ⌘B beeps; an emoji after a letter stays outside the bold).
- ✅ **2026-10-10 — CLOSED** (user: *"close SP-174"*). T-0601 archived → `Tasks/Verified/Task-verified-0601.md`.

