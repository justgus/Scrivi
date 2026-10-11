---
sprint: SP-176
epic: EP-048
status: Closed
closed: 2026-10-11
activated: 2026-10-11
platform: Linux
created: 2026-10-11
---

# SP-176 — `[Linux]` [EP-048] **S4**: Find over what the writer SEES

**Status:** ✅ **CLOSED 2026-10-11** (user: *"close SP-176 and activate SP-177"*) — complete 2026-10-11, rig live pass passed · 🟡 ACTIVE 2026-10-11 (user: *"activate SP-176 and implement it."*) — created 2026-10-11 (user: *"plan S4"*). ✅ Q1–Q4 ruled; ✅ **SPLIT** (user: *"Find can go ahead now"*): this Sprint is **Find**; **Replace waits for Linux undo** ([EP-026]) → EP-048 **S4b**. ⏳ Q5 recommended, not ruled.
**Carries:** [T-0603] → [`../../Tasks/Verified/Task-verified-0603.md`](../../Tasks/Verified/Task-verified-0603.md).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Authority:** EP-048 **L8** (its Find half; Replace → S4b) — Find/Replace on presented text (Q-E2-5); design
[`../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §7. Apple's shape:
[SP-164] (EP-046 E2-S4, T-0585) and the presented map of [SP-171] (`Scrivi/Views/ManuscriptPresentedMap.swift`,
`ManuscriptFind.swift`).
**Size:** S–M. ⚠️ Needs the rig for the live pass.

---

## What reading the code found (2026-10-11)

- ⛔ **Linux has NO Find.** No find UI, no Find items in the Edit menu, no search anywhere in the manuscript. ⛔ Qt has no find
  bar to point at presented text (Apple's [SP-164] Q1 used AppKit's `NSTextFinder`) — ✅ **Linux builds one.**
- ✅ **The rules that decide what is hidden already exist on Linux:** `ManuscriptPresenter` holds each block's core analysis
  (`scrivi_analyze_markdown`: markers, heading/list prefixes) and its hidden backslashes (`ManuscriptEscapes::hiddenBackslashes`,
  the EP-049 port). The presented text is those, **at rest** — hidden whatever the caret or reveal.
- ✅ **For S4b — the write path exists:** `ManuscriptEscapes::escape` (typed text is escaped) and `ManuscriptEditor::replaceRange`, which
  balances through the core's `scrivi_markdown_balanced_edit` ([SP-175]). A replacement is typed text, so it takes both.
- ⚠️ **Linux has no undo** (`setUndoRedoEnabled(false)`; recorded in [SP-175]). Apple's [SP-164] Q2 — *Replace All is one undo
  step* — has nothing to attach to (→ Q3: undo first, so Replace waits).
- ⚠️ **The Linux navigator has no search or filter**, so Apple's [SP-164] Q5 (the Navigator's search jump on presented text) has
  no Linux surface. ⛔ Not in this Sprint.

## What Linux must match (Apple, as verified in [SP-164] / [SP-171])

| Behaviour | Apple | Linux today |
| --------- | ----- | ----------- |
| Find matches the **presented** text: across hidden escapes (`Mr. Smith` → stored `Mr\. Smith`), markers (`bold here` → `**bold** here`) and heading prefixes; list prefixes are visible, so they match | `PresentedText` + `NSTextFinderClient` | ❌ no Find |
| A match never crosses a scene break; chapter titles and dividers are never searched ([SP-164] Q4) | one chunk per scene | ❌ |
| Contains · Starts With · Whole Word; Ignore Case on by default ([SP-164] Q6) | AppKit's bar | ❌ |
| The bar shows the match count; all matches highlighted; Find Next / Previous wrap | AppKit's bar | ❌ |
| ➡️ **S4b** — Replace: the replacement is **escaped** (typed text) and **balanced** (a match across a span edge stays balanced, AC12); it takes the style of the match's first character ([SP-164] Q3, the type-over rule) | `replaceFound` | ❌ |
| ➡️ **S4b** — Replace All: every match, in one pass — fast on 1.85 MB (Apple: 1,173 edits in 83 ms) | `applyReplacements` | ❌ |
| ➡️ **S4b** — Replace All is one undo step ([SP-164] Q2) | grouped history | ⚠️ needs [EP-026] |
| Edit ▸ Find: Find… ⌘F · Find and Replace… ⌥⌘F (➡️ S4b) · Find Next ⌘G · Find Previous ⇧⌘G · Use Selection for Find ⌘E | menu | ❌ → **Q2** |

## Goal

✅ **The Linux writer finds the text they SEE** — escapes, markers and heading prefixes never get in the way. (Replacing it is
S4b, after Linux undo.) ✅ Same results as Apple; the bar is Qt's, so its LOOK differs.

## Plan

1. **The presented map (Linux)** — the manuscript as presented at rest: per scene, the presented text and a presented ↔ document
   offset map, built from the presenter's per-block analysis and hidden backslashes (Q1). Chapter titles and dividers are not in
   it; a match never crosses a scene. Built LAZILY at the first Find (a writer who never searches never pays), invalidated per
   scene on an edit. **Measure** on 1.85 MB dumas (Docker): first build, per-keystroke search, rebuild after an edit.
2. **The match engine** — Contains / Starts With / Whole Word, Ignore Case on by default (Apple's Q6); case- and
   ⚠️ AS BUILT: case-insensitive only (Ignore Case) — the diacritic-insensitive matching was Apple's NAVIGATOR jump (`firstMatch`), not its find bar. A match maps to a document range snapped so it never splits an escape pair or a marker run
   (the existing `snapSelection` / stop-run rules).
3. **The find bar** (Q4) — a strip at the top of the manuscript: query field, match count (*"3 of 120"*), Previous / Next, the
   options. All matches highlighted
   (`QPlainTextEdit::ExtraSelections` — presentation only, the document untouched). Esc closes it and returns focus to the text.
   The bar follows the window's manuscript (one per project window). ⚠️ Built so S4b ADDS a Replace row — no Replace controls
   now (nothing that cannot work is shown).
4. ➡️ **S4b (after [EP-026]), not this Sprint — Replace:** the replacement is escaped (`ManuscriptEscapes::escape`) and written through `replaceRange`, so it is balanced
   by the core and takes the first character's style (Q3 of [SP-164]). Then Find Next.
5. ➡️ **S4b — Replace All:** all matches in ONE edit pass (one `QTextCursor` edit block, highest offset first), each escaped and
   balanced as a single Replace; every touched scene marked dirty and autosaved. ⚠️ Apple's live pass found the per-edit path
   took a minute on 1,172 matches — **measure** on dumas and keep it in the same order as Apple's 83 ms class; ONE grouped
   undo step through [EP-026].
6. **Menus** — Edit ▸ Find submenu with the shortcuts of Q2 (Find and Replace… arrives with S4b). ⚠️ Each shortcut **measured on the rig** against GNOME's bindings
   before it is used (the SP-175 Ctrl+Alt+T lesson).
7. **Tests** — `find_smoke` (new; in CI from the start): the presented map over the shared escape corpus with markers and
   prefixes (exact matches; `Mr. Smith`, `bold here`, a heading; never across a scene; never in a chapter title); the options;
   the count and wrap; the document unchanged by any Find; the shortcuts through the real dispatch (QTest key clicks on an
   active window); mutation-checked (a map hiding nothing → red).
8. **Live pass** (user, rig) — steps in the chat reply; Scrivi autosaves.

## Questions — Q1–Q4 RULED 2026-10-11; Q5 recommended

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | Where the presented map is computed | ✅ **RULED: Linux, from its presenter's analysis** — the hidden set is already Linux's (S2), Apple's map is UI-side Swift too, and a third copy in the core would need Apple re-pointed. Alternative: a core endpoint returning a scene's hidden offsets at rest |
| **Q2** | Shortcuts (Ctrl in place of ⌘, Alt in place of ⌥ — the [SP-174]/[SP-175] rule) | ✅ **RULED: Ctrl+F · Ctrl+Alt+F (Find and Replace) · Ctrl+G and F3 (Next) · Shift+F3 (Previous — ⛔ Ctrl+Shift combos are avoided) · Ctrl+E (Use Selection)**; each measured on the rig. ✅ **AMENDED 2026-10-11 (user): + Ctrl+Shift+G (Previous)** — Apple's ⇧⌘G; Ctrl+Shift chords are now in use ([EP-026] Q3) |
| **Q3** | Replace All with no undo on Linux | ⛔ **RULED: UNDO FIRST** — user: *"We need to implement undo first on Linux. This is a large activity that is (or should be) planned in a subsequent epic given that Apple has the branching undo history and multiple copy buffers already in place."* ✅ That Epic exists as a draft: **[EP-026]** `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste (rescoped 2026-10-11). ⛔ Neither recommendation (a prompt, or no prompt) is taken. ✅ **Split ruled:** Find now (this Sprint); Replace → S4b after EP-026 |
| **Q4** | The find UI | ✅ **RULED: a bar at the top of the manuscript** (Apple's find bar sits in the manuscript's scroll view). Alternative: a separate Find dialog |
| **Q5** | The Linux navigator's search (Apple's [SP-164] Q5) | ⏳ **Recommended: not in this Sprint** — the Linux navigator has no search to re-point; a later Task if wanted |

⛔ **NOT in this Sprint:** Replace and Replace All (→ S4b, after [EP-026]) · project-wide (multi-project) search · regular expressions (Apple's bar offers none) · the navigator
filter (Q5) · undo · L10 (S5).

## Acceptance Criteria

- [x] Plan step 1 measured and recorded (map build, per-keystroke search, rebuild after an edit, on 1.85 MB) ✅
- [x] **L8 (Find half)** — Find matches presented text across escapes, markers and heading prefixes; never across a scene; never in a
      chapter title; Contains / Starts With / Whole Word / Ignore Case ✅ (smoke; live pass owed)
- [x] Edit ▸ Find with the Q2 shortcuts, proven through the real dispatch ✅; none bound by GNOME ✅ (rig live pass, step 12)
- [x] `find_smoke` in CI; mutation-checked ✅
- [x] ✅ **Live pass (user, rig)** — passed 2026-10-11

---

## Progress log

### 🔵 2026-10-11 — Sprint CREATED in Planning

✅ User: *"plan S4"*. ✅ SP-176 and [T-0603] issued by `next-id.py`. ✅ Read first: Linux has no Find of any kind; the presenter
already holds the hidden set; `replaceRange` already balances through the core; Linux has no undo; the navigator has no search.

### 🔵 2026-10-11 — Q1, Q2, Q4 RULED (as recommended); Q3 ruled UNDO FIRST — ⛔ blocked on [EP-026]

✅ User: Q1 Linux presenter · Q2 Ctrl in place of ⌘ · Q4 bar atop the manuscript. ⛔ Q3: *"We need to implement undo first on
Linux. This is a large activity that is (or should be) planned in a subsequent epic given that Apple has the branching undo
history and multiple copy buffers already in place."* ✅ The Epic exists: [EP-026] (🔵 Draft, Epic backlog). ⏳ Open: whether
Find alone proceeds before undo; Q5.

### 🔵 2026-10-11 — SPLIT ruled: Find now, Replace after Linux undo — ready to activate

✅ User: *"we rule that Find can go ahead now. We'll keep EP-048 open until EP-026 lands."* ✅ This Sprint is Find (plan 1–3, 6–8);
Replace and Replace All (plan 4–5) → EP-048 **S4b**, after [EP-026] (rescoped the same day). [T-0603] narrowed to Find.

### 🔵 2026-10-11 — Q2 AMENDED: + Ctrl+Shift+G for Find Previous

✅ User: *"add Ctrl+Shift+G"*. Find Previous = **Shift+F3 and Ctrl+Shift+G** (Apple's ⇧⌘G); measured on the rig with the rest.

### 🟡 2026-10-11 — Sprint ACTIVATED (user-approved)

✅ User: *"activate SP-176 and implement it."* ✅ Left `Sprint-backlog.md`; [T-0603] → active.

### 🟠 2026-10-11 — IMPLEMENTED ([T-0603] Implemented - Not Verified); ⏳ rig live pass owed

✅ User: *"activate SP-176 and implement it."*

**What was built:**
- `ManuscriptFind.hpp/.cpp` (new) — `PresentedText`: the manuscript as presented AT REST, one chunk per scene body (chapter
  titles and dividers are not in it; a match never crosses a scene), each unit mapped to its document position. `find` →
  document ranges: a match starting at an escaped mark takes its hidden `\`; its end is just after its last presented
  character (Apple's `storageRange`). Contains / Starts With / Whole Word (an apostrophe between letters is inside a word);
  Ignore Case. `presentedOf` for Use Selection. ✅ **Kept per scene:** an edit marks its scene; `refresh` rebuilds only marked
  scenes, or any whose id or body length changed (how an insert, delete or move shows up).
- `ManuscriptPresenter::hiddenAtRest` / `escapesOf` — the at-rest hidden set (escapes, markers, heading prefixes; never list
  prefixes) from the analysis the presenter already holds (Q1).
- `FindBar.hpp/.cpp` (new) — the bar (Q4) above the manuscript: query, count (*"1 of 3"*, *"4 matches"*, *"Not found"*),
  Previous / Next, mode, Ignore Case (on), Done; Return / Shift+Return; Esc closes and returns focus. Incremental search as
  you type. Only the matches ON SCREEN are highlighted (extra selections, refreshed on scroll). An edit while the bar is open
  refreshes the count and highlights 150 ms after typing pauses — never per keystroke. `addFindMenu` — Edit ▸ Find ▸ Find…
  Ctrl+F · Find Next Ctrl+G, F3 · Find Previous Ctrl+Shift+G, Shift+F3 · Use Selection for Find Ctrl+E (Q2 as amended).
- `EditorShell` — the bar + viewport in one column of the splitter; attached at load; `find(FindCommand)`.
  `ScriviWindow` — Edit ▸ Find (editor-only actions).
- `CMakeLists.txt` — the new sources in the app and the two other targets that compile `EditorShell.cpp`; `find_smoke`.
  CI: `Find smoke test` step (29 smokes, 29 in CI). Build stamp → **70**.

**Found while building:**
- ⚠️ **The editor SNAPS a find selection** — `bold` in `**bold**` is held as `**bold`: L5's rule (a selection never starts just
  after hidden markup; Apple's `snapSelection`, design §7's "snapped"). Correct, so the bar remembers the selection it made
  and still counts it as the current match.
- ⚠️ `Qt::UniqueConnection` needs a member-function slot — with a lambda the connection silently failed and edits never
  reached the bar (the smoke caught it).

**Measured (Docker, aarch64; ~1.86 MB, 1,186 scenes, every paragraph UNIQUE — the presenter caches analysis by block text, so
repeated paragraphs measured a flattering 69 ms):**
| Measure | Result |
| ------- | ------ |
| presented text, whole build (first Find) | **292 ms** (Apple: 166–178 ms) |
| `Mr. Defarge` (16,614 matches) / `e` (132,912) | **1 ms / 2 ms** |
| an edit in one scene, then refresh | **1 scene rebuilt, 0 ms** |

**Tests:** ✅ `find_smoke` PASS — presented matching (escape, marker, heading prefix; visible list prefix), never across a
scene, never a chapter title, the options, the stored ranges and the snapped selection, the real shortcuts (Ctrl+F, Ctrl+G,
F3, Ctrl+Shift+G, Shift+F3, Return, Shift+Return, Ctrl+E, Esc), Find changes nothing and dirties nothing, no rebuild per
keystroke, one scene rebuilt after an edit. ✅ **Mutation bites:** a presenter hiding nothing → 5 checks red. ✅ All **29**
Linux smokes PASS in Docker; the whole Linux build has **no warnings**.
⚠️ **Not covered (live only):** GNOME's own bindings for the Find keys; the bar's look on the rig's theme; a real manuscript.

### ✅ 2026-10-11 — RIG LIVE PASS: ALL STEPS PASS; [T-0603] VERIFIED

✅ User: *"All steps in the SP-176 Livra pass on the rig passed."* Steps 1–12 (build 70; the menu; the bar; matching across emphasis, escapes and headings; Next / Previous with
wrap; chapter titles not searched; the options; Use Selection; no lag typing with the bar open; Esc; none of the Find keys
taken by GNOME). ✅ A pass the user reports is the instruction to verify: [T-0603] → ✅ Verified. ⏳ Sprint close awaits approval.

### ✅ 2026-10-11 — Sprint CLOSED (user-approved)

✅ User: *"close SP-176 and activate SP-177"* ✅ [T-0603] → `Verified/Task-verified-0603.md` (same step).
