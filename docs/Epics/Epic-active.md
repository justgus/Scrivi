# Active Epics

## EP-046: `[Apple]` ⚠️ **The Manuscript Renderer — Inline Rendering** (WYSIWYG)

**Status:** 🟡 **ACTIVE 2026-10-05** (user: *"activate SP-159"*) — created 2026-09-29. ✅ Unblocked: [EP-045] CLOSED 2026-10-04 → [`Closed/Epic-EP-045.md`](Closed/Epic-EP-045.md).
**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-159]** | S1 — Planning: spikes, design, rulings, ACs | ✅ **CLOSED 2026-10-05** (user-approved) → [`../Sprints/Closed/Sprint-SP-159.md`](../Sprints/Closed/Sprint-SP-159.md) |
| **[SP-161]** | **E2-S1** — presenter (route (a′)) + headings: AC1, AC2, AC4 (line), AC5, AC7, AC8, AC11 | ✅ **CLOSED 2026-10-05** (user-approved) → [`../Sprints/Closed/Sprint-SP-161.md`](../Sprints/Closed/Sprint-SP-161.md) · ✅ [T-0588] verified |
| **[SP-162]** | **E2-S2** — bold / italic + span reveal: AC3, AC4 (span), AC7 (inline); AC5 + AC8 re-checked | ✅ **CLOSED 2026-10-05** (user-approved) → [`../Sprints/Closed/Sprint-SP-162.md`](../Sprints/Closed/Sprint-SP-162.md) · ✅ [T-0590] verified |
| E2-S3 | commands + [T-0584]: AC6, AC9 | not yet created |
| E2-S4 | Find/Replace + [T-0585]: AC10 | not yet created |
**Tasks:** ✅ [T-0590] (SP-162 E2-S2 — VERIFIED, archived) · ✅ [T-0588] (SP-161 E2-S1 — VERIFIED, archived) · ✅ [T-0587] (SP-159 design — VERIFIED, archived) · [T-0584] Option-Return (ruled; → E2-S3) · [T-0585] Find/Replace across hidden characters (ruled; → E2-S4) → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §4.3, §4A.

**Goal:** ⚠️ **What the user actually asked for — *"my inclination is to wysiwyg."*** ✅ **Bold, italic
and headings RENDER; ⚠️ markers hide unless the caret is inside them (Model B).**
✅ **Plus the formatting COMMANDS — ⛔ which under the escape ruling are the feature's ENTIRE input
surface, ⚠️ not a toolbar nicety.**

⛔ **THIS IS THE EXPENSIVE ONE AND THE STUDY MEASURED WHY.** ✅ **[Q7 SPIKE, 2026-09-28]: rendering
attributes CANNOT hide a marker** — ⚠️ **`.font(0.01)` and `.kern(-100)` as RENDERING attributes left
layout at `184.83 pt`, identical to baseline; ✅ the SAME font in STORAGE collapsed it to `152.71 pt`.**
⛔ **So Model B needs STORAGE attributes (⚠️ which `ManuscriptTextView.swift:366-369` STRIPS on every
undo) or an `NSTextLayoutFragment` subclass. ✅ Measured, not feared.**

⚠️ **Whole-LINE markers (`#`, `##`, bullets) are much easier to hide than INLINE ones (`**`)** —
✅ **take them in that order.**

✅ **CARRIED FROM [EP-045] AC7 ([SP-157], Q-AC7 = (a), 2026-10-04) — an acceptance criterion of THIS Epic:**
⚠️ **unexposed block intents (`codeBlock`, `blockQuote`, `table`) are DRAWN as ordinary prose** (study §4D.4(a)).
✅ E1 has no renderer, so E1 met AC7 only for what it interprets — the escape map reads indented text as prose
(2,000/2,000 against the prose oracle). ⛔ **The first renderer to read block intents must demote these three**, or
an indented paragraph turns into monospace with visible backslashes.

⛔ **OUT:** ⚠️ **the scene-SPLIT command** (✅ needs a core endpoint — see [EP-045]).

✅ **ACCEPTANCE CRITERIA — WRITTEN 2026-10-05 ([SP-159]) from rulings Q-E2-1…Q-E2-8 (user: *"I also approve your
recommendations for the 8 rulings"*).** Verification method for each → [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §11.
✅ **Mechanism (Q-E2-6): route (a′)** — an `NSTextContentStorageDelegate` presents each paragraph with the SAME characters
and styled attributes; ⚠️ storage stays plain. (Revises R3 = (c)'s mechanism, not what it shows.)

| AC | Criterion | Sprint |
| -- | --------- | ------ |
| **AC1** | Storage stays plain: rendering writes no storage attribute; save bytes unchanged | E2-S1 |
| **AC2** | Headings render (`#`–`######`: heading font, prefix hidden), incl. every dumas `##` | E2-S1 |
| **AC3** | Bold and italic render, nested included; markers hidden | E2-S2 |
| **AC4** | Re-entry: **span** for inline markers, **line** for prefixes (Q-E2-1); attributes-only — no history event, no undo step. ✅ **Amended 2026-10-05 ([SP-162] Q1, user):** a caret arriving at the start of a span or a heading lands AFTER the opening marker / `## `, so the hint shows to its LEFT. ✅ **Amended again 2026-10-05 ([SP-162] live pass, user):** a span's markers show only with the caret at its FIRST or LAST character, not anywhere inside it | E2-S1 (line) / E2-S2 (span + the amendment) |
| **AC5** | The caret never rests on an invisible stop (hidden-run snap), both directions, clicks and shift-selection | E2-S1 |
| **AC6** | Commands (Q-E2-2): ⌘B `**`, ⌘I `*` (never `_`), Heading 1–3 ⌥⌘1–3, Body ⌥⌘0, bulleted + numbered lists, Return continues a list; one edit each; 100% exact coverage on the S5 corpus with the flank rule | E2-S3 |
| **AC7** *(carried from EP-045)* | Unexposed block intents draw as prose (indented → body font with inline rendering; quote keeps `>`; table as stored) | E2-S1 |
| **AC8** | No added rebuild cost: 1.85 MB open/rebuild within 10 ms of today; < 1 ms added per keystroke | E2-S1 (re-checked each Sprint) |
| **AC9** | Option-Return stores a hard break `\` + `\n` (Q-E2-4, [T-0584]) | E2-S3 |
| **AC10** | Find/Replace matches the PRESENTED text; replacements written escaped (Q-E2-5, [T-0585]) | E2-S4 |
| **AC11** | E1's escape hiding moved onto the presenter; `EscapeHidingStyler` retired; E1's suite stays green | E2-S1 |
| **AC12** *(added 2026-10-05, user)* | **A cut or copy that partly covers a formatted span SPLITS it, never unbalances it:** the span is closed/re-opened at the cut point and the cut text carries its own markers, so an in-app paste keeps the formatting (both directions, nested). ✅ **Every selection replacement does the same** — ⌫/⌦ over it, typing over it, pasting over it ([SP-162] Q3, ruled 2026-10-05: *"Q3: yes."*) | E2-S2 |

✅ Malformed markup renders as the parser reads it (Q-E2-7). ✅ Sprint order E2-S1 → E2-S4 (Q-E2-3).

---

## ✅ **[EP-049]** — `[Linux]` **The Manuscript Storage Format on Linux** — **CLOSED 2026-10-05 (user-approved)**

→ [`Closed/Epic-EP-049.md`](Closed/Epic-EP-049.md). ✅ One Sprint, [SP-160], closed. ✅ All ACs met; [T-0586] and [I-0276]
verified. ✅ Audit Check → [`../Audits/Audit-Check-20261004-EP049.md`](../Audits/Audit-Check-20261004-EP049.md).
⚠️ **Carried forward:** the display half → [SP-159] → [EP-048]; [T-0584] (Alt/Option-Return).
✅ [EP-046] is now active (above).
---

## ✅ **[EP-045]** — `[Apple]` **The Manuscript Renderer — Foundations** — **CLOSED 2026-10-04 (user-approved)**

→ [`Closed/Epic-EP-045.md`](Closed/Epic-EP-045.md). ✅ **Six Sprints: [SP-153] · [SP-154] · [SP-155] · [SP-156] ·
[SP-157] · [SP-158]**, all closed. ✅ **All eleven ACs met.** ✅ Audit Check → [`../Audits/Audit-Check-20261004.md`](../Audits/Audit-Check-20261004.md).
⚠️ **Carried forward:** block-intent drawing → [EP-046]; Linux parity → [EP-048]; [I-0275] (Issue backlog).
⛔ **No Epic is active.**
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

