# Active Epics

## ✅ **[EP-046]** — `[Apple]` **The Manuscript Renderer — Inline Rendering** — **CLOSED 2026-10-06 (user-approved)**

→ [`Closed/Epic-EP-046.md`](Closed/Epic-EP-046.md). ✅ **Five Sprints: [SP-159] · [SP-161] · [SP-162] · [SP-163] ·
[SP-164]**, all closed. ✅ **All twelve ACs met.** ✅ Audit Check → [`../Audits/Audit-Check-20261006-EP046.md`](../Audits/Audit-Check-20261006-EP046.md).
⚠️ **Carried forward:** Linux parity → [EP-048] (unblocked); [T-0589] → [EP-047]; [I-0277], [I-0278] (Issue backlog).
---

## EP-047: `[Apple]` ⚠️ **Manuscript Typography & Preferences**

**Status:** 🟡 **ACTIVE 2026-10-07** (user: *"this plan is approed.  I am ready."*) with [SP-167]. Planned 2026-10-07 (user: *"Ok, lets complete the planning for
EP-047."*); acceptance criteria from the four rulings below. Created 2026-09-29. ✅ Its seam exists ([EP-045] / [EP-046] closed).
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §5.1 (F1), §4D (the indent);
[`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §3.5–§3.7 (the presenter it extends).
**Tasks:** ✅ [T-0596] (SP-167 — VERIFIED, archived) · 🔵 [T-0589] **Markup Hints on/off** (caret rules ruled 2026-10-05) → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Issues:** ✅ [I-0278] `[Apple]` three Project Settings do not travel (✅ fixed + VERIFIED in [SP-167], archived) · 🔵 [I-0281] `[Apple]` emphasis
positions shift after an indented first line (linked forward 2026-10-07) → [`../Issues/Issue-backlog.md`](../Issues/Issue-backlog.md).

**Goal:** ✅ **The writer sets how her manuscript LOOKS — and it travels with the project.** She never types formatting to get it.

✅ **What it delivers:** the manuscript **typeface** (F1, ruled 2026-09-29; ⛔ F3 per-passage fonts closed) · a **first-line indent**
(⚠️ the user's own proposal: a tab or 4 spaces makes a `codeBlock`; ✅ `firstLineHeadIndent` costs ZERO characters, study §4D.3) ·
**Markup Hints on/off** ([T-0589]) · and a **home in the package** for every project setting, which [I-0278] showed does not exist.

### ✅ Planning rulings — 2026-10-07 (user)

| # | Question | Ruling |
| - | -------- | ------ |
| **P1** | Where project preferences live | ✅ **A new `project-settings.json` in the package**, stored by the core with get/put endpoints like `inspector-layout.json`; the app defines the fields. The TITLE goes to `project.json` through a new set-title endpoint. ✅ [I-0278]'s three settings move there, with a one-time migration from `UserDefaults` |
| **P2** | Typeface and indent: per project or per writer | ✅ **Per project — they travel** |
| **P3** | Which paragraphs are indented | ✅ ***"Both, actually. We provide a project preference for either or none."*** — **None · Every body paragraph · Book convention** (all but the first of a scene and the first after a heading). Headings and list items are never indented |
| **P4** | Linux parity | ✅ **Linux rendering joins [EP-048] as L10.** EP-047 builds the core storage (which Linux shares) and the Apple side |
| **P5** | Which typefaces | ✅ **BUNDLED ONLY** (user, 2026-10-07): Scrivi ships ~6 open-licensed faces (SIL OFL — verified per family from Google Fonts metadata); no installed fonts for now. ⚠️ **They must work on iOS** (registration measured as no obstacle: `CTFontManagerRegisterFontURLs`, process scope, macOS 10.15+ / iOS 13+ / visionOS 1+; Linux: `QFontDatabase::addApplicationFont`). Neo (reference, read 2026-10-07) uses macOS SYSTEM serifs, which would not travel to Linux |
| **P5a** | The set | ✅ **CHOSEN 2026-10-07 (user):** *"Literata, Newsreader, Crimson Pro, Inter, Fig Tree, Courier Prime, and Source Code Pro."* — 3 serif, 2 sans, 1 monospaced, 1 submission; 5.7 MB; all SIL OFL 1.1 (read from each font file). Picked from a 28-face Core Text specimen. ⚠️ Learned on the way, for the record: **Tinos** (not chosen) was relicensed Apache → OFL in 2026 though Google Fonts' description still says Apache; **iA Writer Duo/Quattro**'s variable italics contain a malformed glyph Core Text blanks (their static files are clean) → `Resources/Fonts/README.md` |
| **P6** | Default face | ✅ ***"the default font should make the app look wonderful … a Book Serif"*** — **Literata** (proposed; ⏳ to be confirmed by the user). ✅ **Projects with NO typeface setting — existing ones included — open in the default** (user, 2026-10-07: *"yes that is the intention"*), so existing manuscripts change look on first open |
| **P7** | Headings | ✅ **Headings AND chapter titles in the manuscript face** |
| **P8** | Size | ✅ **Per project, for now** — ⛔ not per scene: that would put type information in the `.md` or the scene JSON (*"I don't want to open that can of worms yet"*) |

⛔ **THE TRAP IS WIDER THAN THIS ENTRY FIRST SAID (read 2026-10-07).** The body font is set at **8 storage sites** in
`ManuscriptTextView.swift` — 3 hard-code `monospacedSystemFont` (`:55`, the undo path `:482`, `rebuildStorage` `:741`), 5 use
`ManuscriptPresenter.bodyFont` (`:2616`, `:2635`, `:2696`, `:2846`, `:2858`). ⚠️ A typeface or indent read from a preference must come
from ONE source at all eight, or it vanishes on undo, on rebuild, on Replace All or on paste (AC4). ⚠️ The presenter returns NO
paragraph for a block with no markup (`ManuscriptPresenter.swift:328`), so a plain paragraph's indent cannot come from the presenter alone.

### Acceptance criteria

| AC | Criterion |
| -- | --------- |
| **AC1** | **A settings home in the package (P1):** `project-settings.json`, written and read by ScriviCore (`scrivi_get_project_settings` / `scrivi_put_project_settings`), the app defining its fields; `absent` and `unreadable` distinct and never overwritten on read (the `inspector-layout.json` contract); Linux binds both endpoints; the package-structure doc lists the file |
| **AC2** | **The title travels:** a new core endpoint writes `project.json`'s `title`; Project Settings' rename writes it; every title display reads it |
| **AC3** | **[I-0278] migration:** title, subtitle and "Show chapter titles" move from `UserDefaults` into the package ONCE, without losing a renamed title; the `scrivi.project.<id>.preferences` key is retired. ⚠️ Plus the user's note on the same review: "Find Stale Branches" is an ACTION, not a setting — its new home is ruled in the Sprint |
| **AC4** | **One source for body attributes:** all 8 storage sites (and the presenter's heading/emphasis fonts) derive from one per-project typography value; a test proves undo, rebuild, Replace All, paste and Return keep it |
| **AC5** | **F1 typeface (P2, P5–P8):** Project Settings picks one of the BUNDLED faces and a size; body, headings, chapter titles and bold/italic render in it; the default is a bundled book serif; the OFL license texts ship with the app; changing it re-presents with no history event and no change to the `.md` |
| **AC6** | **First-line indent (P3):** None · Every body paragraph · Book convention, plus the amount; rendered by `firstLineHeadIndent`; **zero characters in the file** (save bytes unchanged — a test); headings and list items never indented; it survives typing, Return, undo and rebuild |
| **AC7** | **Markup Hints on/off ([T-0589]):** View menu (+ a keystroke if one is free — check the menu bar and `keyDown` first), persisted in `project-settings.json`; OFF follows the ruled caret rules (prefix and opener → AFTER, closer → before); toggling re-presents only — no rebuild, no history event |
| **AC8** | **[I-0281] fixed:** emphasis after an indented first line renders where its markers are; the L2 test's `knownDisagreements` entry flips and is removed |
| **AC9** | **Live pass on the Mac** (no rig needed) — steps in the chat reply |

### Planned Sprints (estimate — created one at a time)

| Sprint | Platform | Content | ACs |
| ------ | -------- | ------- | --- |
| **[SP-167]** S1 | `[ScriviCore]` + `[Apple]` | the settings home, the title endpoint, [I-0278]'s migration; Linux binding (bridge only) — ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-167.md`](../Sprints/Closed/Sprint-SP-167.md) · ✅ AC1–AC3 met | AC1–AC3 |
| S2 | `[Apple]` | one body-attribute source; the typeface | AC4, AC5 |
| S3 | `[Apple]` | the indent; [I-0281] | AC6, AC8 |
| S4 | `[Apple]` | Markup Hints ([T-0589]) | AC7 |

✅ AC9's live pass closes each Sprint on the Mac. ⛔ **OUT:** Linux rendering ([EP-048] **L10**, P4) · F3 per-passage fonts (closed) ·
a "show all markup" mode (T-0589: hints only).

---

## EP-048: `[Linux]` ⚠️ **Manuscript Renderer Parity**

**Status:** 🟡 **ACTIVE 2026-10-07** (user: *"We do want to activate EP-048"*) — created 2026-09-29. ✅ Scoped 2026-10-05 ([SP-159]), ACs L1–L8 below. ⚠️ **The Linux rig is unavailable (2026-10-07)** — work that needs no rig goes first; at the first live pass work switches to [EP-047] (user).
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-165]** | **S1** — md4c analyzer in ScriviCore + `scrivi_analyze_markdown` + agreement test: L1, L2 (no rig) | ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-165.md`](../Sprints/Closed/Sprint-SP-165.md) · ✅ [T-0594] verified · ✅ L1, L2 met |
| **[SP-166]** | **S2** — the presenter: L3, L4, L5, L6 (⚠️ live pass needs the rig) | 🟡 **ACTIVE 2026-10-07** → [`../Sprints/Sprint-SP-166.md`](../Sprints/Sprint-SP-166.md) · + L9 atomic markers |
| S3 | commands: L7; balanced edits + copy: L9 (rest) | not yet created |
| S4 | Find/Replace: L8 | not yet created |
| S5 | EP-047's preferences on Linux: L10 (after EP-047 S1–S4) | not yet created |

**Tasks:** ✅ [T-0594] (SP-165 — VERIFIED, archived) · 🟡 [T-0595] (SP-166) → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Issues:** 🟠 [I-0280] (found + fixed in SP-165, Apple and core) · 🔵 [I-0281] `[Apple]` (found by L2 → Issue backlog, ➡️ linked forward to [EP-047]).
⚠️ **2026-10-04:** the escape layer's WRITE half moved OUT to **[EP-049]** (now). ✅ This Epic keeps the DISPLAY half, whose design for BOTH platforms is ruled in **[SP-159]** (widened).
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §9.

**Goal:** ✅ **The same manuscript surface on Linux.**

⚠️ **THIS EPIC EXISTS TO HONOUR A RULE, NOT BECAUSE IT IS UNDERSTOOD.** ✅ **`feedback_linux_adopts_apple_shape`:
a shape change on Apple must be made the same way on Linux.** ⛔ **Naming it is how that is honoured
without blocking Apple on Qt** — ⚠️ **but it MUST be scheduled, ⛔ not assumed.**

⛔ **NOTHING HERE IS MEASURED.** ⚠️ **Qt's text stack is NOT TextKit: ⛔ there is no
`setRenderingAttributes`, no `NSTextLayoutFragment`, and the format decisions (escaping, `\n\n`,
block-intent suppression) are `[Cross]` and bind Linux permanently — ✅ while the MECHANISM is entirely
different.** ⛔ **Assuming parity is cheap would be the error `feedback_design_to_capability_not_lcd`
warns about.**

⚠️ **ALSO UNKNOWN: ⛔ Linux has no Markdown parser chosen.** ✅ **Apple's `AttributedString(markdown:)`
won the Q7(b) spike on CORRECTNESS** — ⚠️ **Linux gets no such gift and must pick one, ⛔ or re-earn the
`2 * 3 * 4` class of defect the spike caught.**

✅ **SCOPED 2026-10-05 ([SP-159], Q-E2-8 ruled) — design → [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §2.4, §9.**
⚠️ The two "unknowns" above are now MEASURED (W3, Qt 6.4.2): ⛔ `QTextDocument::toMarkdown()` cannot be the save path
(0/1,185 dumas files identical, 1,179 re-wrapped, **escapes written unescaped**). ✅ `QSyntaxHighlighter` on today's
`QPlainTextEdit` is the same shape as Apple's route (a′): presentation-only, survives undo, hidden `**` residue 0.00 pt,
heading line 32 vs 20. ✅ Parser ruled: **md4c inside ScriviCore** (L-b). ✅ **2026-10-07 ([SP-165] Q1–Q2):** ⛔ Linux does NOT call C++ directly — it is a C ABI client (`<scrivi/scrivi.h>` only), so L-b's route is a new endpoint `scrivi_analyze_markdown`; md4c 0.5.2 via pinned FetchContent, not the Ubuntu package. ✅ md4c's escape reporting MEASURED: the escaped mark is covered text at its own offset, the backslash uncovered (→ [SP-165] spike).

| AC | Criterion |
| -- | --------- |
| **L1** | **A source-mapped Markdown analyzer in ScriviCore (md4c)**: per block → kind, markers, bold, italic, prefixes, spans; ⚠️ first measurement: how md4c reports escaped characters |
| **L2** | **Agreement test:** the core analyzer and Apple's `AttributedString` parser agree over the AC3 corpus + the S5 corpus (interop test) |
| **L3** | **The Linux presenter:** a `QSyntaxHighlighter` on `ManuscriptEditor`; the document's stored text and formats untouched; save bytes unchanged |
| **L4** | **Escape backslashes hidden** (Linux shows them today) with E1's rule; hard-break backslash per E1 AC6 |
| **L5** | **Hidden-run caret snap** (W3 measured the same invisible stops as Apple) |
| **L6** | **Headings + bold/italic render; re-entry span/line** (Q-E2-1) via `rehighlightBlock`; AC7 prose demotion |
| **L7** | **Commands** as EP-046 AC6 (Ctrl in place of ⌘) and Option/Alt-Return = hard break (Q-E2-4). ✅ **The Alt-Return STORAGE half is done** — matched in [SP-163] (2026-10-05) with the shared corpus, `escape_smoke` PASS |
| **L8** | **Find/Replace on presented text** (Q-E2-5) |
| **L9** | ✅ **ADDED 2026-10-07 (user, [SP-166] Q1)** — **Markers are atomic and edits stay balanced** (EP-046 AC12, design §3.6): ⌫/⌦ never delete a hidden marker alone (→ S2); balanced edits and copy without markers (→ S3, with the commands) |
| **L10** | ✅ **ADDED 2026-10-07 (user, [EP-047] P4)** — **[EP-047]'s preferences rendered on Linux**: the project typeface, the first-line indent (None · Every · Book convention), Markup Hints on/off — read from the shared `project-settings.json`. ⚠️ Qt's first-line indent is a BLOCK format (a document format, not a highlighter one — the SP-166 hanging-indent finding); measure it before ruling the mechanism |

⚠️ Where Qt cannot match Apple's mechanism, match the RESULT and record the difference (T-0576 precedent).
⚠️ Not measured yet: `QPlainTextDocumentLayout` hanging indent for lists; md4c per-block cost on 1.85 MB.

---

---

## ✅ **[EP-049]** — `[Linux]` **The Manuscript Storage Format on Linux** — **CLOSED 2026-10-05 (user-approved)**

→ [`Closed/Epic-EP-049.md`](Closed/Epic-EP-049.md). ✅ One Sprint, [SP-160], closed. ✅ All ACs met; [T-0586] and [I-0276]
verified. ✅ Audit Check → [`../Audits/Audit-Check-20261004-EP049.md`](../Audits/Audit-Check-20261004-EP049.md).
⚠️ **Carried forward:** the display half → [SP-159] → [EP-048]; [T-0584] (Alt/Option-Return).
✅ [EP-046] followed it (closed 2026-10-06, above).
---

## ✅ **[EP-045]** — `[Apple]` **The Manuscript Renderer — Foundations** — **CLOSED 2026-10-04 (user-approved)**

→ [`Closed/Epic-EP-045.md`](Closed/Epic-EP-045.md). ✅ **Six Sprints: [SP-153] · [SP-154] · [SP-155] · [SP-156] ·
[SP-157] · [SP-158]**, all closed. ✅ **All eleven ACs met.** ✅ Audit Check → [`../Audits/Audit-Check-20261004.md`](../Audits/Audit-Check-20261004.md).
⚠️ **Carried forward:** block-intent drawing → [EP-046]; Linux parity → [EP-048]; [I-0275] (Issue backlog).
---

## ✅ **[EP-043]** — `[Linux]` **The Session** — **CLOSED 2026-09-30 (user-approved)**

→ [`Closed/Epic-EP-043.md`](Closed/Epic-EP-043.md). ✅ **Four Sprints: [SP-145] · [SP-146] · [SP-147] ·
[SP-148]**, all closed. ✅ **All ten ACs met** (R1–R8, AC-build, AC-live) — ✅ **[I-0176], [I-0177], [I-0178]
VERIFIED**: Linux opens many projects, one window each, reopening where the writer left them (size,
maximized, splitters). ⚠️ **Window POSITION is ruled out on Wayland** ([I-0264]).
⚠️ **Carried forward, NOT closed by this Epic:** [I-0255] (Timeline visibility persistence, `[Cross]`);
[I-0244] (the Linux shell gap — its SIBLING Epic, sequenced after this one per [R-Q5]); the untested
desktop-logout path; Landing staying up beside restored windows (a parity question — Apple dismisses its
Welcome).

---

