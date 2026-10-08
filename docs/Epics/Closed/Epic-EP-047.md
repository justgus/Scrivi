## EP-047: `[Apple]` ⚠️ **Manuscript Typography & Preferences**

**Status:** ✅ **CLOSED 2026-10-08 (user-approved):** *"Approved to close EP-047."* — activated 2026-10-07, created 2026-09-29. ✅ **All nine ACs met** across [SP-167]–[SP-170]; rulings P1–P11. ✅ Audit Check → [`../Audits/Audit-Check-20261008-EP047.md`](../../Audits/Audit-Check-20261008-EP047.md) — F-1–F-4 applied at the close; O-1, O-2 observations (O-2: [I-0280] since VERIFIED).
⚠️ **Carried forward:** Linux rendering of the type, indent and hints → [EP-048] **L10**; [I-0277] (VoiceOver) → Issue backlog. ✅ Fixed alongside, outside a Sprint: [I-0283] (macOS Smart Quotes off in the manuscript, ruled (a)).
EP-047."*); acceptance criteria from the four rulings below. Created 2026-09-29. ✅ Its seam exists ([EP-045] / [EP-046] closed).
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §5.1 (F1), §4D (the indent);
[`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §3.5–§3.7 (the presenter it extends).
**Tasks:** ✅ [T-0596] (SP-167 — VERIFIED, archived) · ✅ [T-0597] (SP-168 — VERIFIED, archived) · ✅ [T-0598] (SP-169 — VERIFIED, archived) · ✅ [T-0589] **Markup Hints on/off** (SP-170 — VERIFIED, archived).
**Issues:** ✅ [I-0278] `[Apple]` three Project Settings do not travel (✅ fixed + VERIFIED in [SP-167], archived) · ✅ [I-0282] `[Apple]` a typeface or size change lost the writer's place (found + fixed + VERIFIED in [SP-168] after four live checks, archived; added at the close, Audit Check F-3) · ✅ [I-0281] `[Apple]` emphasis
positions shift after an indented first line (fixed + VERIFIED in [SP-169], archived) → [`../Issues/Issue-backlog.md`](../../Issues/Issue-backlog.md).

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
| **P6** | Default face | ✅ ***"the default font should make the app look wonderful … a Book Serif"*** — ✅ **Literata — CONFIRMED 2026-10-07** (user: *"Yes, Literata should be the default."*). ✅ **Projects with NO typeface setting — existing ones included — open in the default** (user, 2026-10-07: *"yes that is the intention"*), so existing manuscripts change look on first open |
| **P9** | Size, spacing, chapter titles (ruled 2026-10-07, user) | ✅ **Default size 16 pt** (control 10–32 pt) · ✅ **line spacing FIXED at 1.45 × size, not a setting** (every face on one rhythm; switching faces does not reflow vertically) · ✅ **chapter titles at Heading 1 size, in the text colour** (today: system bold 15 pt, secondary grey) |
| **P10** | The indent (ruled 2026-10-07, user — S3 planning) | ✅ **Default: Book convention** for every project with no setting (*"Book Convention … but with an option for none in settings"*; P3's three modes stay: None · Every body paragraph · Book) · ✅ **amount in ems, default 1.5 em** (stepper 0.5–4) · ✅ **with an indent, the stored blank line between paragraphs draws as a SMALL GAP** (display only; the `.md` keeps it; None → the full blank line) · ✅ **Book convention: no indent after anything but body text** (scene start, heading, list, block quote, chapter title) |
| **P11** | Markup Hints (ruled 2026-10-08, user — S4 planning) | ✅ **Shortcut ⇧⌘H** ("Hints"; ⌥⌘M is macOS's Minimize All, ⌥⌘H Hide Others) · ✅ **default ON** for a project that never set it (T-0589: ON = today's behaviour) |
| **P7** | Headings | ✅ **Headings AND chapter titles in the manuscript face** |
| **P8** | Size | ✅ **Per project, for now** — ⛔ not per scene: that would put type information in the `.md` or the scene JSON (*"I don't want to open that can of worms yet"*) |

⛔ **THE TRAP IS WIDER THAN THIS ENTRY FIRST SAID (read 2026-10-07).** The body font is set at ~~**8 storage sites**~~ **NINE storage sites** (corrected at the close, F-2 — SP-168 measured) in
`ManuscriptTextView.swift` — 3 hard-code `monospacedSystemFont` (`:55`, the undo path `:482`, `rebuildStorage` `:741`), 5 use
`ManuscriptPresenter.bodyFont` (`:2616`, `:2635`, `:2696`, `:2846`, `:2858`). ⚠️ A typeface or indent read from a preference must come
from ONE source at all ~~eight~~ nine, or it vanishes on undo, on rebuild, on Replace All or on paste (AC4). ⚠️ The presenter returns NO
paragraph for a block with no markup (`ManuscriptPresenter.swift:328`), so a plain paragraph's indent cannot come from the presenter alone.

### Acceptance criteria

| AC | Criterion |
| -- | --------- |
| **AC1** | **A settings home in the package (P1):** `project-settings.json`, written and read by ScriviCore (`scrivi_get_project_settings` / `scrivi_put_project_settings`), the app defining its fields; `absent` and `unreadable` distinct and never overwritten on read (the `inspector-layout.json` contract); Linux binds both endpoints; the package-structure doc lists the file |
| **AC2** | **The title travels:** a new core endpoint writes `project.json`'s `title`; Project Settings' rename writes it; every title display reads it |
| **AC3** | **[I-0278] migration:** title, subtitle and "Show chapter titles" move from `UserDefaults` into the package ONCE, without losing a renamed title; the `scrivi.project.<id>.preferences` key is retired. ⚠️ Plus the user's note on the same review: "Find Stale Branches" is an ACTION, not a setting — its new home is ruled in the Sprint |
| **AC4** | **One source for body attributes:** all ~~8~~ **nine** storage sites (⚠️ corrected at the close, Audit Check F-2: SP-168 measured NINE — four hard-coded system fonts + five `bodyFont`) (and the presenter's heading/emphasis fonts) derive from one per-project typography value; a test proves undo, rebuild, Replace All, paste and Return keep it |
| **AC5** | **F1 typeface (P2, P5–P8):** Project Settings picks one of the BUNDLED faces and a size; body, headings, chapter titles and bold/italic render in it; the default is a bundled book serif; the OFL license texts ship with the app; changing it re-presents with no history event and no change to the `.md` |
| **AC6** | **First-line indent (P3):** None · Every body paragraph · Book convention, plus the amount; rendered by `firstLineHeadIndent`; **zero characters in the file** (save bytes unchanged — a test); headings and list items never indented; it survives typing, Return, undo and rebuild |
| **AC7** | **Markup Hints on/off ([T-0589]):** View menu (+ a keystroke if one is free — check the menu bar and `keyDown` first), persisted in `project-settings.json`; OFF follows the ruled caret rules (prefix and opener → AFTER, closer → before); toggling re-presents only — no rebuild, no history event |
| **AC8** | **[I-0281] fixed:** emphasis after an indented first line renders where its markers are; the L2 test's `knownDisagreements` entry flips and is removed |
| **AC9** | **Live pass on the Mac** (no rig needed) — steps in the chat reply |

### Planned Sprints (estimate — created one at a time)

| Sprint | Platform | Content | ACs |
| ------ | -------- | ------- | --- |
| **[SP-167]** S1 | `[ScriviCore]` + `[Apple]` | the settings home, the title endpoint, [I-0278]'s migration; Linux binding (bridge only) — ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-167.md`](../../Sprints/Closed/Sprint-SP-167.md) · ✅ AC1–AC3 met | AC1–AC3 |
| **[SP-168]** S2 | `[Apple]` | one body-attribute source; the typeface — ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-168.md`](../../Sprints/Closed/Sprint-SP-168.md) · ✅ AC4, AC5 met | AC4, AC5 |
| **[SP-169]** S3 | `[Apple]` | the indent; [I-0281] — ✅ **CLOSED 2026-10-08** (user-approved) → [`../Sprints/Closed/Sprint-SP-169.md`](../../Sprints/Closed/Sprint-SP-169.md) · ✅ AC6, AC8 met | AC6, AC8 |
| **[SP-170]** S4 | `[Apple]` | Markup Hints ([T-0589]) — ✅ **CLOSED 2026-10-08** (user-approved) → [`../Sprints/Closed/Sprint-SP-170.md`](../../Sprints/Closed/Sprint-SP-170.md) · ✅ AC7 met | AC7 |

✅ AC9's live pass closes each Sprint on the Mac. ⛔ **OUT:** Linux rendering ([EP-048] **L10**, P4) · F3 per-passage fonts (closed) ·
a "show all markup" mode (T-0589: hints only).
