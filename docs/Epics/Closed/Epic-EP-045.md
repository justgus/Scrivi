# EP-045: `[Apple]` **The Manuscript Renderer — Foundations**

**Status:** ✅ **CLOSED 2026-10-04 (user-approved):** *"fix all findingd as per your reccomendations.  Close the Epic."* — activated 2026-10-03, created 2026-09-29. ✅ **All eleven ACs met** across [SP-153]–[SP-158]. ✅ Rulings R1, R2, R3, Q-AC7 (and SP-156's Q1–Q3). ✅ Audit Check → [`../../Audits/Audit-Check-20261004.md`](../../Audits/Audit-Check-20261004.md), all findings remediated.
**Platform:** ⚠️ **`[Apple]` ONLY** — ✅ **Linux parity is [EP-048], SCHEDULED not assumed.**
**Design:** → [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md)
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md)
— ✅ **sixteen questions RULED by the user 2026-09-29.**

---

## Goal

✅ **Make the manuscript a RENDERED surface possible, and pay the data-loss debt that blocks it.**

⚠️ **This Epic renders almost nothing the writer asked to see.** ⛔ **That is deliberate.** ✅ **It fixes
an existing corruption path, installs the two-coordinate-space model, puts the escape layer in front of
typing, and restores the scene divider.** ⚠️ **The ONE writer-visible outcome is the user's original
complaint: *"the writer can SEE she is at a boundary."***

⛔ **THE TEMPTATION TO ADD E2's RENDERING HERE MUST BE REFUSED** — ✅ **§4A.1 of the study MEASURED that
marker hiding needs storage attributes the undo path strips; ⚠️ folding it in makes this Epic
unclosable.**

---

## Why this Epic exists now

⚠️ **The user, 2026-09-28:** ***"I'd like ManuscriptView to become a Markdown Renderer."***
✅ **A trade study answered it, and two of the user's own volunteered rulings reshaped the work:**

1. ⛔ **ALL typed Markdown reserved characters are ESCAPED** (⚠️ study §3A.0) — ✅ **markup is authored
   ONLY by command.** ⛔ **This DELETED a whole problem (input-side parsing of typed markup) and
   CREATED a smaller one (an escape/unescape layer).**
2. ⛔ **THE CARET POSITION IS NOT THE FILE OFFSET** (⚠️ study §3.4A) — ✅ **a SOURCE↔PRESENTED mapping
   becomes a required component, ⚠️ and it is required for ESCAPES ALONE, even before any marker is
   hidden.**

---

## Acceptance Criteria

| # | Criterion | ✅ Authority | ⚠️ Evidence obligation |
| - | --------- | ----------- | ---------------------- |
| **AC1** | ⛔ **Attachments are TYPED** — only a divider attachment bounds a scene | ✅ **Study §2.3** | ✅ **Unit test: a non-divider attachment leaves `sceneBoundaries` unchanged** |
| **AC2** | ✅ **The scene divider is VISIBLE** in Light and Dark | ✅ **Study §1.4A** | ⛔ **A LIVE PASS with a screenshot — ⚠️ NOT a suite** |
| **AC3** | ✅ **A SOURCE↔PRESENTED mapping exists**; every caret path uses it | ✅ **Study §3.4A** (user ruling) | ✅ **Corpus test against the §4B.3 oracle** |
| **AC4** | ✅ **Typed reserved characters are ESCAPED — all 32** | ✅ **Study §4B.4** (user: *"use the READ set"*) | ✅ **Round-trip: type → store → parse → equals what was typed** |
| **AC5** | ⛔ **Enter inserts `\n\n`**; ⚠️ ~~**Backspace at paragraph start deletes ONE `\n`**~~ → ✅ **AMENDED by Q3 (user, 2026-10-04): JOINS the paragraphs with ONE space** (`a.⏎⏎b.` → `a. b.`); ⛔ except after a deliberate trailing `\` (one `\n` goes, keeping the hard break) — [SP-156] | ✅ **Study §3A.6** (user ruling) | ✅ **Unit test on the edit path** |
| **AC6** | ✅ **Trailing spaces reduced to AT MOST ONE on Enter**; ⚠️ trailing `\\` → `\` | ✅ **Study §4B.6** | ✅ **Unit test: `"x␣␣␣"` + Enter → `"x␣\n\n"`** |
| **AC7** | ✅ **Unexposed block intents SUPPRESSED** (`codeBlock`, `blockQuote`, `table`) | ✅ **Study §4D.4(a)** (user: *"option 2"*) | ✅ **Unit test: a 4-leading-space paragraph renders as prose** |
| **AC8** | ✅ **ONE parsing mode chosen and STATED** | ⛔ **Study §4C.2 — the study itself mixes them** | ✅ **Code comment naming the mode and why** |
| **AC9** | ✅ **No regression in save fidelity** | ✅ **Study §2** | ✅ **Integration test on a real temp project** |
| **AC10** | ✅ **The caret path MEASURED on a 1.85 MB manuscript** | ⚠️ **[I-0206]'s re-open condition** | ⛔ **A recorded measurement — ⚠️ not a pass/fail gate** |
| **AC11** | ✅ **The scene-boundary table is MAINTAINED across edits, not rescanned over the whole manuscript per keystroke** | ✅ **User, 2026-10-04** ([SP-157] AC10 finding; T-0583 linked here at the user's direction) | ✅ **`[SCRIVI-KEY] bounds` ≈ 0 on 1.85 MB + a test that the maintained table equals a full rescan** |

---

### ✅ AC status at close (2026-10-04)

- ✅ **AC11 — MET 2026-10-04** ([T-0583] user-verified; [SP-158]): `[SCRIVI-KEY] bounds=0.0` on 1.85 MB. ⚠️ Added 2026-10-04 — ⛔ It had been filed unlinked (Epic "None"); the user's rule is that a Task belongs to the Epic it was created in unless stated otherwise.
- ✅ **AC7 — MET 2026-10-04** ([T-0581] user-verified: *"no backslashes appeared"*; Q-AC7 = (a), drawing carried to [EP-046]).
- ✅ **AC10 — MEASURED 2026-10-04** ([SP-157], [T-0582]) → full tables in [`../Sprints/Closed/Sprint-SP-157.md`](../../Sprints/Closed/Sprint-SP-157.md). ✅ E1's additions negligible (restyle ≤0.7 ms/edit, 4.7 ms/rebuild; snap unmeasurable). ⚠️ **Keystroke cost is LINEAR IN OFFSET** (~20–43 ms at the start → ~89–122 ms at the end, arrows 1 → 90 ms) — AppKit's, our work flat at 2.3–4.3 ms; revises [I-0206]'s *"typing is constant"*. ⛔ Not a gate; a NEW Issue only if it bites.
- ✅ **AC5 + AC6 — MET 2026-10-04** ([T-0580] user-verified by live check; [SP-156]). ⚠️ AC5's Backspace half AMENDED by Q3: ⌫ joins with one space until/unless re-ruled.
- ✅ **AC4 — MET 2026-10-03** ([T-0579] user-verified; [SP-155] CLOSED). ⚠️ Linux shows backslashes until [EP-048] (ruled); Find/Replace across hidden backslashes is a follow-up.
- ✅ **AC3 + AC8 — MET 2026-10-03** ([T-0578] user-verified; `.full` confirmed). [SP-154] CLOSED.
- ✅ **AC2 — MET 2026-10-03** (user's Light-mode live look). ✅ **AC1 + AC9 — MET 2026-10-03**
  ([T-0577] user-verified by live look; [SP-153] CLOSED). ✅ Design ruled by the user: one `DividerTextAttachment`, rendering
  states in an enum carried by the `.scriviDivider` key.

## ✅ THE RULINGS — R1, R2, R3: all RULED (2026-10-03)

| # | ⚠️ Question | ⚠️ Recommendation |
| - | ---------- | ----------------- |
| **R1** | ⛔ **Is PASTED text escaped like typed text?** ⚠️ The ruling said *"that the user types"* | ✅ **RULED 2026-10-03 — YES** (user: *"R1 is yes."*). Pasted text is escaped exactly like typed text. |
| **R2** | ⛔ **Is there a one-time escape pass over EXISTING manuscripts?** ⚠️ They hold unescaped `*` today | ✅ **RULED 2026-10-03 — NO ESCAPE PASS** (user: *"R2 is no escape pass. The dumas projects do have ## at the top of every scene, but these are intended to represent formatted MArkup text, so no escape pass for that either."*). ✅ Existing scene files are never rewritten; ✅ existing Markdown in them (e.g. the `dumas` projects' `##` scene headings) is INTENDED markup and will render as such. |
| **R3** | ⛔ **How is the escape BACKSLASH hidden in E1?** ⚠️ **Raised 2026-10-03 at SP-154 planning — the design never says.** AC4 stores `*` as `\*`; the study measured that rendering attributes CANNOT hide a character (§4A.1) and scoped hiding to [EP-046]. ⛔ **As designed, the writer would SEE `\*`.** ⚠️ **Blocks AC4, not AC3.** | ✅ **RULED 2026-10-03 — (c)** (user: *"YEs R3 should be (c)."*): storage attributes hide the backslash; ONE caret-snap hook skips its unreachable boundary. Measured first (design §4.4). |

---

## ⛔ Known traps — each already paid for once

1. ✅ **AC2's cause — FOUND on the third attempt ([T-0554]).** ⚠️ The lesson stands: two speculative colour fixes failed before a LIVE diagnosis found it (`feedback_prove_code_is_reached`).
2. ⛔ **`rebuildStorage` IS WHOLE-DOCUMENT** (`:568-668`) — ✅ **[I-0196] names it *"the prime suspect for
   the hang"*.** ⚠️ **AC3's scanner must be PER FRAGMENT; ⛔ never a document-wide table.**
3. ⛔ **THE UNDO PATH STRIPS STORAGE ATTRIBUTES** (`:366-369`, only `.font` + `.foregroundColor`).
   ⚠️ **Anything E1 puts in storage must be re-applied there AND in `rebuildStorage` — ✅ both sites or
   neither.**
4. ⛔ **THE ORACLE IS VERIFIED, NOT PROVEN** — ⚠️ **10/10 probes, ✅ but probe 10 failed until the
   all-32 ruling.** ⛔ **AC3 requires a CORPUS test** (`feedback_boundary_tests_not_facade`).

---

## Explicitly OUT of scope

⛔ **Marker hiding / Model B** (→ **[EP-046]**) · ⛔ **bold/italic/heading RENDERING** (→ **[EP-046]**) ·
⛔ **the formatting COMMANDS** (→ **[EP-046]**) · ⛔ **`paragraphIndent` preference and font choice**
(→ **[EP-047]**) · ⛔ **Linux parity** (→ **[EP-048]**) · ⛔ **[EP-032] inline object references**
(✅ **study §8: this Epic sequences FIRST and unblocks it**) · ⛔ **the scene-SPLIT command**
(⚠️ **needs a core endpoint that does not exist — see below**).

---

## ⛔ VERIFIED GAP — there is NO scene-split endpoint

✅ **CHECKED 2026-09-29 against `ScriviCore/include/scrivi/scrivi.h`:** ⚠️ **`scrivi_merge_scene` exists
(`:533`); ⛔ `scrivi_split_scene` DOES NOT.**

⚠️ **Study §10.1 ruled the scene break is a COMMAND that splits a scene at the caret.** ⛔ **The core
cannot do it, ✅ so that command needs a NEW `[ScriviCore]` endpoint** — ⚠️ **`[Cross]` work, scoped
where the core is scoped, ⛔ NOT here.** ✅ **E1 makes the EXISTING divider visible (AC2); ⚠️ authoring
a NEW break waits.**

---

## Sprints

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-158]** | **AC11 maintain the scene-boundary table** ([T-0583]; the Linux shape) | ✅ **CLOSED 2026-10-04** → [`../Sprints/Closed/Sprint-SP-158.md`](../../Sprints/Closed/Sprint-SP-158.md) |
| **[SP-157]** | **AC7 block intents as prose** + **AC10 caret-path measurement** (✅ Q-AC7 = (a), ruled) | ✅ **CLOSED 2026-10-04** → [`../Sprints/Closed/Sprint-SP-157.md`](../../Sprints/Closed/Sprint-SP-157.md) — filed [T-0583], [I-0275] |
| **[SP-156]** | **AC5 Enter/Backspace** + **AC6 trailing whitespace** (✅ Q1 = (a); ✅ Q2; ✅ Q3: ⌫ joins with a space) | ✅ **CLOSED 2026-10-04** → [`../Sprints/Closed/Sprint-SP-156.md`](../../Sprints/Closed/Sprint-SP-156.md) |
| **[SP-155]** | ✅ **AC4 the escape layer** (typing, paste, pair-deletion, copy un-escape, hiding styler) | (Linux shows backslashes until [EP-048] — ruled) → ✅ **CLOSED 2026-10-03** [`../Sprints/Closed/Sprint-SP-155.md`](../../Sprints/Closed/Sprint-SP-155.md) |
| **[SP-154]** | ✅ **AC3 two coordinate spaces** + **AC8 parsing mode** + ⚠️ **AC-R3 measurement for ruling R3** | ✅ **CLOSED 2026-10-03** → [`../Sprints/Closed/Sprint-SP-154.md`](../../Sprints/Closed/Sprint-SP-154.md) |
| **[SP-153]** | ⛔ **AC1 typed attachments (the data-loss item, all six readers)** + **AC9 save fidelity** + **AC2 Light-mode live look** | ✅ **CLOSED 2026-10-03** → [`../Sprints/Closed/Sprint-SP-153.md`](../../Sprints/Closed/Sprint-SP-153.md) |

---

## ✅ Close — 2026-10-04 (user-approved)

✅ User: *"fix all findingd as per your reccomendations.  Close the Epic."*
✅ **Delivered:** typed scene dividers (a live save-path corruption fixed) · the SOURCE↔PRESENTED caret mapping ·
the escape layer (typed and pasted Markdown marks can never become formatting) · a visible divider · Return /
Backspace paragraph semantics · indented text read as prose · the caret path measured · the scene-boundary table
maintained instead of rescanned.
⚠️ **Carried forward, NOT closed by this Epic:** drawing unexposed block intents as prose → [EP-046] (its AC);
Linux parity → [EP-048]; Find/Replace across hidden backslashes (follow-up); Option-Return storing one `\n`
(unruled); [I-0275] keystroke cost that grows with position (Issue backlog; first step a profile).
⚠️ **Lesson:** a Task created inside an Epic belongs to it unless stated otherwise — [T-0583] was first filed
unlinked and became AC11 at the user's direction.

