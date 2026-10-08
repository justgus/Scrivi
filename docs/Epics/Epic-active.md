# Active Epics

## EP-050: `[Apple]` ⚠️ **Manuscript Accessibility** — VoiceOver reads the page the writer reads

**Status:** 🟡 **ACTIVE 2026-10-08** (SP-171 activated) — created 2026-10-08 (user: *"let's look at I-0277"*; rulings A1–A3 the same day). ✅ S1 = [SP-171], ✅ closed 2026-10-08; S2 next.
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-171]** | **S1** — the translation layer (one map, shared with Find), the caret both ways, cost: AC1, AC2, AC3 + AC5 reading half | ✅ **CLOSED 2026-10-08** (user-approved) → [`../Sprints/Closed/Sprint-SP-171.md`](../Sprints/Closed/Sprint-SP-171.md) · Q2 re-ruled: dividers read as WORDS · ✅ AC1, AC2, AC3, AC5 reading half met |
| S2 | the Headings rotor: AC4 + AC5 rotor half | not yet created |
| S3 | Linux: AC6 (measure Orca, then rule) — ⚠️ needs the rig | not yet created |

**Tasks:** ✅ [T-0599] (SP-171 — VERIFIED, archived).
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
| **AC4** | **The Headings rotor (A2):** Markdown headings and chapter titles, in order, each a `targetRange` in presented positions; next/previous from the caret |
| **AC5** | **Live pass with VoiceOver** (user, Mac): it reads the page without markup at default punctuation verbosity; the rotor moves heading to heading; typing with VoiceOver on behaves |
| **AC6** | **Linux** (Qt accessibility reads the stored text too, I-0277): MEASURE what Orca hears, then rule — a criterion here or EP-048's |

⛔ **OUT:** iOS/visionOS (no manuscript surface yet) · spoken formatting ("bold") beyond what the attributed string carries.

---

## ✅ **[EP-046]** — `[Apple]` **The Manuscript Renderer — Inline Rendering** — **CLOSED 2026-10-06 (user-approved)**

→ [`Closed/Epic-EP-046.md`](Closed/Epic-EP-046.md). ✅ **Five Sprints: [SP-159] · [SP-161] · [SP-162] · [SP-163] ·
[SP-164]**, all closed. ✅ **All twelve ACs met.** ✅ Audit Check → [`../Audits/Audit-Check-20261006-EP046.md`](../Audits/Audit-Check-20261006-EP046.md).
⚠️ **Carried forward:** Linux parity → [EP-048] (unblocked); [T-0589] → [EP-047]; [I-0277], [I-0278] (Issue backlog).
---

## ✅ **[EP-047]** — `[Apple]` **Manuscript Typography & Preferences** — **CLOSED 2026-10-08 (user-approved)**

→ [`Closed/Epic-EP-047.md`](Closed/Epic-EP-047.md). ✅ **Four Sprints: [SP-167] · [SP-168] · [SP-169] · [SP-170]**, all closed.
✅ **All nine ACs met** — settings travel with the project; seven bundled faces, Literata 16 pt default; the first-line indent (Book
by default); Markup Hints. ✅ Audit Check → [`../Audits/Audit-Check-20261008-EP047.md`](../Audits/Audit-Check-20261008-EP047.md).
⚠️ **Carried forward:** Linux rendering → [EP-048] L10; [I-0277] (Issue backlog).

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

