# Active Epics

## EP-050: `[Apple]` ⚠️ **Manuscript Accessibility** — VoiceOver reads the page the writer reads

**Status:** 🟡 **ACTIVE 2026-10-08** (SP-171 activated) — created 2026-10-08 (user: *"let's look at I-0277"*; rulings A1–A3 the same day). ✅ S1 = [SP-171], ✅ closed 2026-10-08; S2 = [SP-172], ✅ closed 2026-10-09.
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-171]** | **S1** — the translation layer (one map, shared with Find), the caret both ways, cost: AC1, AC2, AC3 + AC5 reading half | ✅ **CLOSED 2026-10-08** (user-approved) → [`../Sprints/Closed/Sprint-SP-171.md`](../Sprints/Closed/Sprint-SP-171.md) · Q2 re-ruled: dividers read as WORDS · ✅ AC1, AC2, AC3, AC5 reading half met |
| **[SP-172]** | **S2** — the Headings rotor: AC4 + AC5 rotor half | ✅ **CLOSED 2026-10-09** (user-approved) → [`../Sprints/Closed/Sprint-SP-172.md`](../Sprints/Closed/Sprint-SP-172.md) · rulings R1–R3 there · ✅ AC4 + AC5 rotor half met (live pass 2026-10-09); T-0600 verified |
| S3 | Linux: AC6 (measure Orca, then rule) — ⚠️ needs the rig | not yet created |

**Tasks:** ✅ [T-0599] (SP-171 — VERIFIED, archived) · ✅ [T-0600] (SP-172 — VERIFIED, archived).
**Issues:** ✅ [I-0277] `[Apple]` VoiceOver reads the STORED manuscript — fixed and VERIFIED in [SP-171], archived. · 🔵 [I-0284] (found in SP-171 → Issue backlog).
**Authority:** design [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §13 (*"VoiceOver /
accessibility reads storage. Hidden markers may be spoken; not checked."*); SDK `AppKit/NSAccessibilityProtocols.h`,
`NSAccessibilityCustomRotor.h` (read 2026-10-08).

**Goal:** ✅ **A writer using VoiceOver hears the manuscript as it reads on the page** — no backslashes, no `##`, no `**` — and can
move from heading to heading the way she would on the web.

### ⚠️ What the SDK says the work is (read 2026-10-08)

A text view speaks to assistive technology through **~15 members, every one in CHARACTER POSITIONS**: `accessibilityValue`,
`accessibilityNumberOfCharacters`, `accessibilitySelectedText`, `accessibilitySelectedTextRange(s)` (get AND set — VoiceOver moves the
caret through them), `accessibilityVisibleCharacterRange`, `accessibilityInsertionPointLineNumber`, `accessibilityLine(for:)`,
`accessibilityRange(forLine:)`, `accessibilityString(for:)`, `accessibilityAttributedString(for:)`, `accessibilityRange(forPosition:)`,
`accessibilityRange(for index:)`, `accessibilityFrame(for:)`, `accessibilityRTF(for:)`. ⛔ So the fix is not "a different string": it is a
**TRANSLATION LAYER** between the PRESENTED text and STORAGE, consistent across every member — ⚠️ or VoiceOver's cursor and Scrivi's
caret drift apart (the [I-0282] lesson). ✅ The pieces exist: `MarkdownEscapes.map`, the presenter's `stopTest` / `isHidden`, the
caret-snap rules. ✅ `NSAccessibilityCustomRotor` (macOS 10.13+) has `NSAccessibilityCustomRotorTypeHeading` and a text result's
`targetRange` — the Headings rotor.

### ✅ Rulings — 2026-10-08 (user)

| # | Question | Ruling |
| - | -------- | ------ |
| **A1** | What text VoiceOver gets | ✅ **The page AT REST** — every Markdown mark hidden (escapes, heading `#`, emphasis markers) whatever the caret or Markup Hints; list prefixes kept, as on screen. Stable as the caret moves |
| **A2** | Headings | ✅ **A Headings rotor** — VoiceOver moves heading to heading (and chapter titles) |
| **A3** | Tracking | ✅ **This Epic**; [I-0277] its first Issue; Linux its own criterion |

### Acceptance criteria

| AC | Criterion |
| -- | --------- |
| **AC1** | **One translation layer (A1):** the accessibility text is the page at rest; EVERY member above maps presented ⇄ storage through ONE map; a corpus test proves round trips and agreement between members (e.g. `string(for: r)` = the substring of `value` at `r`; `line(for:)` / `range(forLine:)` agree) |
| **AC2** | **The caret, both ways:** a selection set by VoiceOver lands where Scrivi's caret rules put it (never inside hidden markup); Scrivi's caret is reported in presented positions |
| **AC3** | **Cost:** the map is cached and invalidated per edit — measured on 1.8 MB (`numberOfCharacters`, `string(for:)`, a keystroke with VoiceOver's queries); recorded |
| **AC4** | **The Headings rotor (A2):** Markdown headings and chapter titles, in order, each a `targetRange` in presented positions; next/previous from **VoiceOver's reading position** (amended 2026-10-08, SP-172 R3: VoiceOver never passes the caret) |
| **AC5** | **Live pass with VoiceOver** (user, Mac): it reads the page without markup at default punctuation verbosity; the rotor moves heading to heading; typing with VoiceOver on behaves |
| **AC6** | **Linux** (Qt accessibility reads the stored text too, I-0277): MEASURE what Orca hears, then rule — a criterion here or EP-048's |

⛔ **OUT:** iOS/visionOS (no manuscript surface yet) · spoken formatting ("bold") beyond what the attributed string carries.

---

## EP-048: `[Linux]` ⚠️ **Manuscript Renderer Parity**

**Status:** 🟡 **ACTIVE** since 2026-10-07 — S1, S2 and SP-173 closed; S3a closed (SP-174, 2026-10-10); **S3b active** (SP-175, 2026-10-10). L1–L6 met; L7 and L9 partly met; L8 and L10 open. The rig is available.
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-165]** | **S1** — md4c analyzer in ScriviCore + `scrivi_analyze_markdown` + agreement test: L1, L2 (no rig) | ✅ **CLOSED 2026-10-07** (user-approved) → [`../Sprints/Closed/Sprint-SP-165.md`](../Sprints/Closed/Sprint-SP-165.md) · ✅ [T-0594] verified · ✅ L1, L2 met |
| **[SP-166]** | **S2** — the presenter: L3, L4, L5, L6 (⚠️ live pass needs the rig) | ✅ **CLOSED 2026-10-09** (user-approved) → [`../Sprints/Closed/Sprint-SP-166.md`](../Sprints/Closed/Sprint-SP-166.md) · ✅ L3–L6, L9 (part) met; [T-0595] verified |
| **[SP-173]** | `[Cross]` **I-0285** (found in S2): project load — no frozen UI, timeline built once, load measured; core calls synchronised (both platforms) | ✅ **CLOSED 2026-10-10** (user-approved) → [`../Sprints/Closed/Sprint-SP-173.md`](../Sprints/Closed/Sprint-SP-173.md) · ✅ I-0285 verified: load 172 s → 15.4 s |
| **[SP-174]** | **S3a** — `[Cross]` the command and balanced-edit logic moves into ScriviCore; Apple re-pointed (no visible change) | ✅ **CLOSED 2026-10-10** (user-approved) → [`../Sprints/Closed/Sprint-SP-174.md`](../Sprints/Closed/Sprint-SP-174.md) · ✅ Q1–Q5 ruled |
| **[SP-175]** | **S3b** — `[Linux]` the Format commands (L7) and balanced edits + copy (L9 rest) on the core logic; navigator toggle moves off Ctrl+Alt+N; rig live pass | 🟡 **ACTIVE 2026-10-10** → [`../Sprints/Sprint-SP-175.md`](../Sprints/Sprint-SP-175.md) · ✅ Q1–Q4 ruled |
| S4 | Find/Replace: L8 | not yet created |
| S5 | EP-047's preferences on Linux: L10 (EP-047 closed 2026-10-08, so not blocked) | not yet created |

**Tasks:** ✅ [T-0594] (SP-165 — VERIFIED, archived) · ✅ [T-0595] (SP-166 — VERIFIED, archived) · ✅ [T-0601] (SP-174 — VERIFIED, archived) · 🟡 [T-0602] (SP-175).
**Issues:** ✅ [I-0285] (SP-173 — VERIFIED, archived) · 🟠 [I-0280] (found + fixed in SP-165, Apple and core) · 🔵 [I-0281] `[Apple]` (found by L2 → Issue backlog, ➡️ linked forward to [EP-047]).
This Epic is the escape layer's DISPLAY half on Linux; the WRITE half was [EP-049] (closed 2026-10-05). The design for both platforms was ruled in [SP-159].
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §9.

**Goal:** ✅ **The same manuscript surface on Linux.**

**Why:** `feedback_linux_adopts_apple_shape` — a shape change on Apple is made the same way on Linux. Where Qt cannot match
Apple's mechanism, the RESULT is matched and the difference recorded.

**Established (design: [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §2.4, §9):**
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

---
