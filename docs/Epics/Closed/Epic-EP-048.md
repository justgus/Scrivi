# EP-048: `[Linux]` ⚠️ **Manuscript Renderer Parity**

**Status:** ✅ **CLOSED 2026-11-06** (user, 2026-11-06: *"F1–F5 as recommended, close EP-048."*) — ✅ **L1–L10 all met**: L1–L2 (S1, [SP-165]), L3–L6 (S2, [SP-166]), L7 and L9 (S3a/S3b, [SP-174], [SP-175]), L8 (Find, [SP-176]; Replace, [SP-181]), L10 (S5a–S5c, [SP-182]–[SP-184]); the load ([SP-173]). Linux's undo and structured cut/copy/paste, missing when L9 was met, came with [EP-026] (closed 2026-11-05), which this Epic waited for (user, 2026-10-11). ✅ Audit Check run at the close (findings below). — activated 2026-10-07 with [SP-165].
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-165]** | **S1** — md4c analyzer in ScriviCore + `scrivi_analyze_markdown` + agreement test: L1, L2 (no rig) | ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-165.md`](../../Sprints/Closed/Sprint-SP-165.md) · ✅ [T-0594] verified · ✅ L1, L2 met |
| **[SP-166]** | **S2** — the presenter: L3, L4, L5, L6 (⚠️ live pass needs the rig) | ✅ **CLOSED 2026-10-09** (user-approved) → [`../Sprints/Closed/Sprint-SP-166.md`](../../Sprints/Closed/Sprint-SP-166.md) · ✅ L3–L6, L9 (part) met; [T-0595] verified |
| **[SP-173]** | `[Cross]` **I-0285** (found in S2): project load — no frozen UI, timeline built once, load measured; core calls synchronised (both platforms) | ✅ **CLOSED 2026-10-10** (user-approved) → [`../Sprints/Closed/Sprint-SP-173.md`](../../Sprints/Closed/Sprint-SP-173.md) · ✅ I-0285 verified: load 172 s → 15.4 s |
| **[SP-174]** | **S3a** — `[Cross]` the command and balanced-edit logic moves into ScriviCore; Apple re-pointed (no visible change) | ✅ **CLOSED 2026-10-10** (user-approved) → [`../Sprints/Closed/Sprint-SP-174.md`](../../Sprints/Closed/Sprint-SP-174.md) · ✅ Q1–Q5 ruled |
| **[SP-175]** | **S3b** — `[Linux]` the Format commands (L7) and balanced edits + copy (L9 rest) on the core logic; navigator toggle moves off Ctrl+Alt+N; rig live pass | ✅ **CLOSED 2026-10-11** (user-approved) → [`../Sprints/Closed/Sprint-SP-175.md`](../../Sprints/Closed/Sprint-SP-175.md) · ✅ [T-0602] verified · ✅ L7, L9 met |
| **[SP-176]** | **S4** — `[Linux]` **Find** over the presented text: L8's Find half | ✅ **CLOSED 2026-10-11** (user-approved) → [`../Sprints/Closed/Sprint-SP-176.md`](../../Sprints/Closed/Sprint-SP-176.md) · ✅ Q1–Q4 ruled; ✅ split ruled 2026-10-11 · ✅ rig live pass passed; [T-0603] verified · ✅ L8 Find half met |
| **[SP-181]** | **S4b** — `[Linux]` **Replace / Replace All** over the presented text: L8's Replace half — escaped, balanced, Replace All ONE undo step. ✅ Unblocked: Linux undo's grouped edit passed the rig ([SP-178], 2026-10-13) | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-181.md`](../../Sprints/Closed/Sprint-SP-181.md) · ✅ Q1–Q3 ruled 2026-10-15 · ⚠️ `[Apple]` [I-0303] fixed first (Replace All writes off the main thread) · ✅ rig live pass 2026-10-21 |
| **[SP-182]** | **S5a** — `[Linux]` L10's character half: ✅ **builds Linux's Project Settings dialog** (replacing the *"coming soon"* stub; Project + Typography), the bundled typefaces and size, Markup Hints. EP-026 later adds its Undo History section | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-182.md`](../../Sprints/Closed/Sprint-SP-182.md) · ✅ Q1–Q5 ruled 2026-10-15 (incl. the S5 split) · ✅ Q6 ruled 2026-10-21 (synthesized bold) · ✅ rig live pass 2026-10-21 |
| **[SP-183]** | **S5b** — `[Linux]` L10's block half: line spacing 1.45 and the first-line indent (Qt block formats — measured before the mechanism is ruled) | ✅ **CLOSED 2026-11-05** (user-approved) → [`../Sprints/Closed/Sprint-SP-183.md`](../../Sprints/Closed/Sprint-SP-183.md) · [T-0610] ✅ · ➕ [I-0351] ✅ |
| **[SP-184]** | **S5c** — `[Linux]` the **"Show chapter titles" toggle** ([EP-026] Q4) and the chapter-ending divider ([T-0575]) | ✅ **CLOSED 2026-11-06** (user-approved) → [`../Sprints/Closed/Sprint-SP-184.md`](../../Sprints/Closed/Sprint-SP-184.md) · [T-0611] ✅ · ✅ Q1–Q7 ruled (Q7: 60% on both) |

**Tasks:** ✅ [T-0594] (SP-165 — VERIFIED, archived) · ✅ [T-0595] (SP-166 — VERIFIED, archived) · ✅ [T-0601] (SP-174 — VERIFIED, archived) · ✅ [T-0602] (SP-175 — VERIFIED, archived) · ✅ [T-0603] (SP-176 — VERIFIED, archived) · ✅ [T-0608] (SP-181 — VERIFIED 2026-10-21, archived) · ✅ [T-0609] (SP-182 — VERIFIED 2026-10-21, archived) · ✅ [T-0610] (SP-183 — VERIFIED 2026-11-05, archived) · ✅ [T-0611] (SP-184 — VERIFIED 2026-11-06, archived).
**Issues:** ✅ [I-0285] (SP-173 — VERIFIED, archived) · ✅ [I-0280] (found + fixed in SP-165, Apple and core — VERIFIED, archived) · ✅ [I-0281] `[Apple]` (found by L2; fixed in [SP-169] — VERIFIED, archived) · ✅ [I-0303] `[Apple]` (fixed first in [SP-181] — VERIFIED, archived) · ✅ [I-0351] (found in [SP-183] — VERIFIED, archived). Found here, open in the backlog and not holding the close: [I-0284] (md4c renders emphasis in a hard-break block; an accepted difference, [SP-174]) · [I-0308] (a crash typing in the Find bar; kept open by the user, 2026-10-21) · [I-0352] (a ctest flake) · [I-0353] (a format on an empty line; ruled for every platform).
This Epic is the escape layer's DISPLAY half on Linux; the WRITE half was [EP-049] (closed 2026-10-05). The design for both platforms was ruled in [SP-159].
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §9.

**Goal:** ✅ **The same manuscript surface on Linux.**

**Why:** `feedback_linux_adopts_apple_shape` — a shape change on Apple is made the same way on Linux. Where Qt cannot match
Apple's mechanism, the RESULT is matched and the difference recorded.

**Established (design: [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §2.4, §9):**
- The parser is **md4c inside ScriviCore**, reached through `scrivi_analyze_markdown` (Linux is a C ABI client); the core
  analyzer agrees with Apple's parser over the shared corpora (L2).
- The presenter is a `QSyntaxHighlighter` on `ManuscriptEditor` — presentation only; stored text and save bytes untouched (L3).
- ⛔ `QTextDocument::toMarkdown()` is NOT a save path (it re-wraps and writes escapes unescaped); Linux saves Apple's format.
- Measured differences, recorded: no hanging indent for list items (a block format is a document edit — ✅ accepted, SP-174 Q3);
  heading sizes are Apple's ratios over the body size.

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

Measured (SP-166): attach + first highlight of 1.8 MB ≈ 185 ms (Docker), a keystroke ~1 ms; the load as a whole → [SP-173].

### ✅ Audit Check at the close (2026-11-06) — ruled by the user ("F1–F5 as recommended")

| # | Finding | Ruling |
|---|---|---|
| F1 | The Status line still read *"L8 and L10 open"* and *"Linux has no undo yet"* | ✅ Fixed at the close (above) |
| F2 | The index row in `Epic-Documentation.md` named *"ACs L1–L9"*, missing L10 | ✅ Fixed |
| F3 | The Issues line named only I-0280, I-0281 and I-0285 — not [I-0303], [I-0351] (verified) or [I-0284], [I-0308], [I-0352], [I-0353] (open) | ✅ All six added; the four open stay in the backlog |
| F4 | The Issue index counted batch 29 (I-0281–I-0290) as 8; the file holds 9 | ✅ Re-counted to 9 |
| F5 | `Sprint-backlog.md` kept an empty *"EP-048 — S5a–S5c"* section | ✅ Removed |

Clean: Sprint status (SP-165, SP-166, SP-173–SP-176, SP-181–SP-184 closed everywhere, each with `closed:`), evidence (T-0594, T-0595,
T-0601–T-0603, T-0608–T-0611 archived; every cited Issue archived), Epic status (index and `Epic-active.md` agree; 4 → 3 active),
no orphan files, ID continuity (next EP-058, SP-205, T-0664, I-0354).
