---
sprint: SP-175
epic: EP-048
status: Active
activated: 2026-10-10
platform: Linux
created: 2026-10-10
---

# SP-175 — `[Linux]` [EP-048] **S3b**: the Format commands and balanced edits on Linux, on the core logic

**Status:** ✅ **COMPLETE 2026-10-10 — awaiting close approval** (rig live pass passed). Activated 2026-10-10 (user: *"activate SP-175 and implement it"*) — created 2026-10-10 (user: *"plan S3b"*); Q1–Q4 ruled the same day.
**Carries:** [T-0602] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Authority:** EP-048 **L7** (commands as EP-046 AC6, Ctrl in place of ⌘) and **L9**'s rest (balanced edits and copy, as
EP-046 AC12). Rulings carried from [SP-174]: Q3 (no hanging indent on Linux, accepted), Q4 (Numbered List takes Ctrl+Alt+N;
the navigator moves), Q5 (the pending pair follows Apple's rule).
**Built on:** [SP-174] — the logic is ScriviCore's `scrivi_markdown_*` (8 endpoints, UTF-16 offsets, one scene's text), proven
against Apple's implementation by a 5,661-case fixture. ✅ **Linux writes no editing logic of its own** — only wiring.
**Size:** M. ⚠️ Needs the rig for the live pass.

---

## What Linux must match (Apple, as verified in [SP-163] / [SP-174])

| Behaviour | Apple | Linux today |
| --------- | ----- | ----------- |
| **Format menu** between Edit and View: Bold, Italic, Heading 1–3, Body, Bulleted List, Numbered List | ⌘B ⌘I ⌥⌘1–3 ⌥⌘0 ⌥⌘L ⌥⌘N | ❌ no menu |
| Bold / Italic on a selection: toggle, split, nest; an edge that cannot carry it shrinks; refused with a beep if it cannot be written balanced | core `emphasis_edit` | ❌ |
| Caret IN a word → the word, caret kept; BETWEEN words → the pending pair | core `word_range` + view | ❌ |
| Headings / body / lists per paragraph; numbering kept sequential | core `paragraph_edit` | ❌ |
| Return in a list item continues it; on an empty item ends it | core `list_return_edit` | ❌ (plain Return) |
| A cut / ⌫ / ⌦ / typing / paste / Return that partly covers a span keeps it balanced; Scrivi's own paste keeps its formatting | core `balanced_edit` | ❌ (only: ⌫/⌦ never delete a hidden marker alone, [SP-166]) |
| Copy: Scrivi's own clipboard gets the BALANCED source; other apps the presented text | core `balanced_copy` | presented ✅ ([SP-160]); source unbalanced ❌ |
| Across scenes: a cut is balanced per scene; a copy is copied scene by scene | per-scene core calls | per-scene cut ✅ ([I-0270]), unbalanced ❌ |
| A Format command across a divider or a chapter title is refused (beep) | ✅ (SP-174) | ❌ |

**Not in this Sprint (recorded, not owed here):**
- **Undo.** Linux has no undo at all (`setUndoRedoEnabled(false)`, EditorShell.cpp:136 — reserved for the custom history).
  Apple's "⌘Z undoes exactly the command" has no Linux counterpart until that history exists.
- **Structured cross-scene paste** ([T-0591]'s paste pieces). Linux has no scene-preserving cross-scene copy/paste (the bridge
  binds `fragment*`, but nothing calls them), so `balance_paste_pieces` has no caller on Linux yet.
- **The hanging indent** for list items — accepted difference ([SP-174] Q3).

## Measured on the rig, 2026-10-10 (read-only `gsettings`)

GNOME's Ctrl+Alt bindings on Oathkeeper: **Ctrl+Alt+T = open a terminal** · Ctrl+Alt+Left/Right/Up/Down = switch workspace
(+Shift = move window) · Ctrl+Alt+Tab / Escape = switch panels · Ctrl+Alt+Delete = log out · Ctrl+Alt+F1–F12 = switch session.
✅ **Free:** Ctrl+Alt+0/1/2/3, B, I, L, N, S. ⚠️ So Ctrl+Alt+L (feared from older Ubuntu) is free here, and **Linux Scrivi's own
Ctrl+Alt+T (Show Timeline, ScriviWindow.cpp:253) collides with GNOME's terminal** — see Q3.

## ✅ Rulings — 2026-10-10 (user)

| # | Ruling |
| - | ------ |
| **Q1** | ✅ **Show Scene Navigator → Ctrl+Alt+S** |
| **Q2** | ✅ **The pending pair is in the document, excluded** — `SceneDocument` leaves its range out of `bodyText` and never marks the scene dirty for it |
| **Q3** | ✅ **Fix the Ctrl+Alt+T collision in this Sprint** — Show Timeline → **Ctrl+Alt+M** |
| **Q4** | ✅ **Format menu between Edit and View**, as Apple |

## Questions as raised

- **Q1 — The navigator's new key** (SP-174 Q4 ruled it moves). Apple's sidebar toggle is ⌃⌘S. **Recommend Ctrl+Alt+S**: free on
  the rig, keeps Apple's letter, and sits with Ctrl+Alt+I (inspector) in Linux's View family.
- **Q2 — How the pending pair stays out of the scene text on Linux.** Apple puts it in the text view's storage only; Linux's
  autosave reads the scene from the DOCUMENT (`SceneDocument::bodyText`), and every document change marks the scene dirty.
  - **(a)** Insert it into the document like Apple, flagged: `SceneDocument` excludes the pair's range from `bodyText` and does
    not mark the scene dirty for its insertion or removal. Same look and caret behaviour as Apple.
  - **(b)** Never insert it: paint the dimmed `****` as an overlay at the caret and insert `**x**` in one edit at the first
    typed character. Nothing can leak into a save, but the caret cannot sit visibly BETWEEN painted glyphs that take no space.
  - **Recommend (a)** — it is Apple's shape; the smoke proves `bodyText` and the dirty flag never see the pair.
- **Q3 — Ctrl+Alt+T (Show Timeline) collides with GNOME's terminal.** Found by this check, not part of S3b. (a) fix it in this
  Sprint with Q1 (one shortcut pass over the View menu); (b) leave it and file it as an Issue; (c) leave it. **Recommend (a)**,
  e.g. **Ctrl+Alt+M** (free; not GNOME's) — or another key you prefer.
- **Q4 — Format menu position.** Apple ruled it between Edit and View ([SP-163]). **Recommend the same on Linux.**

## Plan (S3b)

1. **Bridge:** `ScriviBridge` wrappers for the 8 `scrivi_markdown_*` endpoints (QString is UTF-16, so offsets pass straight
   through; a document position maps to a scene offset by subtracting the scene body's start — the paragraph separator counts as
   one unit, as `\n` does in the scene text).
2. **Scene slicing:** a helper on `SceneDocument` — the scene body holding a range (nil across a boundary), its text and start;
   the Linux twin of Apple's `sceneRange`.
3. **Format menu + shortcuts** (Q1, Q3, Q4 — ruled): Bold Ctrl+B, Italic Ctrl+I, Heading 1–3 Ctrl+Alt+1–3, Body Ctrl+Alt+0, Bulleted
   Ctrl+Alt+L, Numbered Ctrl+Alt+N; Show Scene Navigator → Ctrl+Alt+S, Show Timeline → Ctrl+Alt+M. Each command = ONE `replaceRange`,
   then the returned selection; refusal = beep.
4. **Return in a list** (`handleReturn`), and **balanced edits** wherever a selection is replaced or a span edge is touched:
   typing over a selection, ⌫/⌦, Return, paste (own copy keeps its formatting), cut — including the per-scene cross-scene cut.
5. **Copy:** `createMimeDataFromSelection` takes source and presented from `balanced_copy`, scene by scene.
6. **The pending pair** (Q2), ⌘B/⌘I between words: shown dimmed with the caret between; typing into it makes it real; moving away
   removes it; never saved, never marks the scene dirty.
7. **Tests:** a Linux smoke (offscreen Qt) that drives the REAL dispatch (the QActions' shortcuts, `keyPressEvent`, the mime
   paths) on a multi-scene document with chapter titles and an emoji: each command equals the core's answer moved to document
   offsets; a cross-divider command refused; the pending pair absent from `bodyText` and the dirty set; a cross-scene cut
   balanced per scene. Full ctest + smokes (Docker, CI image); Apple interop untouched.
8. **Live pass on the rig** (Mac-side steps of [SP-174] in Ctrl form, minus undo), on a test project.

## Progress log

- **2026-10-10** — created in 🔵 Planning; Q1–Q4 raised; GNOME bindings measured on the rig.
- **2026-10-10** — Q1–Q4 ruled (Ctrl+Alt+S; pending pair in the document, excluded; Timeline → Ctrl+Alt+M in this Sprint; Format menu between Edit and View). Planning complete.
- **2026-10-10** — activated; T-0602 → active. Plan 1 started.
- **2026-10-10 — Plans 1–7 implemented** (no Linux editing logic; all of it is ScriviCore's):
  - **Bridge:** `ScriviBridge::markdown*` for the six endpoints Linux calls (word range, emphasis, paragraph, list Return,
    balanced edit, balanced copy). `style_at_caret` / `balance_paste_pieces` have no Linux caller: they serve the structured
    cross-scene paste Linux does not have.
  - **Scene slice:** `SceneDocument::sceneHolding` (the twin of Apple's `sceneRange`).
  - **Balancing at ONE hook:** every Linux edit already went through `ManuscriptEditor::replaceRange` (Apple's
    `shouldChangeText`); it now re-issues an unbalanced edit as the core's balanced one. Routed through it as well: the
    in-scene cut (was Qt's `cut()`), the cross-scene cut (each scene's part), an IME commit over a selection. Paste of
    Scrivi's own copy keeps its formatting. Copy puts the balanced source on Scrivi's clipboard, scene by scene.
  - **Commands:** `FormatMenu` (one builder, shared by the window and the smoke): Format between Edit and View (Q4),
    Ctrl+B / Ctrl+I / Ctrl+Alt+1–3 / Ctrl+Alt+0 / Ctrl+Alt+L / Ctrl+Alt+N; refusal beeps. Return in a list item continues or
    ends it. Show Scene Navigator → **Ctrl+Alt+S** (Q1); Show Timeline → **Ctrl+Alt+M** (Q3).
  - **Pending pair (Q2):** in the document, excluded. `SceneDocument` holds it (moved or ended by other edits per Apple's
    rules); `bodyText` leaves it out; its insert and removal never mark the scene dirty; the presenter draws it dimmed.
    Whitespace typed into it goes before it; ⌫/⌦ in it or the same command again removes it; leaving it removes it. Every
    structural edit (the shell's six `loading_` windows) removes it first; a rebuild clears it — as Apple's rebuild drops it.
  - **Tests:** `format_smoke` (new; in CI) drives the real shortcuts (QTest on an active window) on two scenes under two
    chapter titles with an emoji: every command equals the core's answer at scene offsets; the pending pair's whole life
    (not scene text, not dirty, real once typed into, gone when left); balanced cut / copy / own paste / ⌫ / typing over a
    span edge; refusal across a title; cross-scene cut balanced per scene. ✅ Proven to bite: with balancing disabled, its
    six balance checks fail. `escape_smoke` now builds with the bridge (the escape corpus runs with balancing on).
  - Results: Linux (Docker, CI image) builds with no warnings; smokes 28/28. Core and Apple unchanged.
  - ⚠️ Measured while writing the smoke: `QTest::keyClick(Ctrl+C)` sends the text "c", which the editor takes as typing;
    X11 sends U+0003. The smoke sends the control character, as a keyboard does.
  - Linux build stamp bumped to **69** (not committed).
- **2026-10-10 — CI runs every Linux smoke** (user: *"add the missing smokes to CI too"*): eleven existed that CI never ran
  (bridge_parity, escape, presenter, scene_inspector, inspector_layout, package_folder, theme_contrast, worlds_path,
  open_progress, session_store, restore_guard) — added beside format_smoke; 28/28 in the workflow. ⚠️ Each passes in the CI
  image (Docker, non-root); the first GitHub run is their first run on the hosted runner.
- ✅ **2026-10-10 — Plan 8: rig live pass PASSED** (user: *"All live pass rig tests (1-10) pass."*) — build 69: the Format menu
  and its shortcuts; bold/italic toggle, split and nest; word-for-caret and the pending pair; headings, body, lists, Return and
  renumbering; Alt-Return; balanced cut, own paste, presented copy to another app; refusal and balanced cut across scenes;
  Ctrl+Alt+S / Ctrl+Alt+M (Ctrl+Alt+T is GNOME's terminal again); the pending pair never saved across a quit.
- ✅ **Sprint complete** — ⏳ awaiting the user's approval to close.
