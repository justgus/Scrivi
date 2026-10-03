# EP-045: `[Apple]` **The Manuscript Renderer — Foundations**

**Status:** 🟡 **ACTIVE 2026-10-03** (user-approved) — created 2026-09-29. ✅ **[SP-153] CLOSED 2026-10-03 — AC1, AC2, AC9 MET.** ⚠️ Remaining: AC3–AC8, AC10. ✅ R1 ruled (paste escaped). ✅ R2 ruled (no escape pass). ✅ R3 ruled (c). **[SP-154]** (AC3) ACTIVE.
**Platform:** ⚠️ **`[Apple]` ONLY** — ✅ **Linux parity is [EP-048], SCHEDULED not assumed.**
**Design:** → [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md)
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md)
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
| **AC5** | ⛔ **Enter inserts `\n\n`**; ✅ **Backspace at paragraph start deletes ONE `\n`** | ✅ **Study §3A.6** (user ruling) | ✅ **Unit test on the edit path** |
| **AC6** | ✅ **Trailing spaces reduced to AT MOST ONE on Enter**; ⚠️ trailing `\\` → `\` | ✅ **Study §4B.6** | ✅ **Unit test: `"x␣␣␣"` + Enter → `"x␣\n\n"`** |
| **AC7** | ✅ **Unexposed block intents SUPPRESSED** (`codeBlock`, `blockQuote`, `table`) | ✅ **Study §4D.4(a)** (user: *"option 2"*) | ✅ **Unit test: a 4-leading-space paragraph renders as prose** |
| **AC8** | ✅ **ONE parsing mode chosen and STATED** | ⛔ **Study §4C.2 — the study itself mixes them** | ✅ **Code comment naming the mode and why** |
| **AC9** | ✅ **No regression in save fidelity** | ✅ **Study §2** | ✅ **Integration test on a real temp project** |
| **AC10** | ✅ **The caret path MEASURED on a 1.85 MB manuscript** | ⚠️ **[I-0206]'s re-open condition** | ⛔ **A recorded measurement — ⚠️ not a pass/fail gate** |

---

### ⚠️ AC status at activation (2026-10-03) — read in the code, not carried over

- ✅ **AC2 — LARGELY MET BY [T-0554]** (→ `../Tasks/Verified/Task-verified-0554-0564.md`). ✅ Its cause
  WAS found (third attempt), the divider is now a `DividerTextAttachment` with its own drawing, ✅ the
  user verified it by eye on macOS (*"on macOS the line is now fully visible"*, 2026-09-29), ✅ and a
  bitmap measurement of a real `NSTextView` found it drawn in BOTH Dark and Light. ⚠️ **What the AC's
  evidence obligation still lacks is a LIVE look in LIGHT mode** — [SP-153] collects it.
  ⛔ **Known trap #1 below is therefore OUT OF DATE.**
- ✅ **AC3 + AC8 — MET 2026-10-03** ([T-0578] user-verified; `.full` confirmed). [SP-154] complete.
- ✅ **AC2 — MET 2026-10-03** (user's Light-mode live look). ✅ **AC1 + AC9 — MET 2026-10-03**
  ([T-0577] user-verified by live look; [SP-153] CLOSED). ✅ Design ruled by the user: one `DividerTextAttachment`, rendering
  states in an enum carried by the `.scriviDivider` key.
- ⛔ ~~**AC1 — STILL OPEN, and WIDER than the design says.**~~ ⚠️ The design (§2.2) names *"two call sites"*;
  ✅ **SIX code sites read `.attachment` today** (`ManuscriptTextView.swift` `:1700`, `:2060`, `:2153`,
  `:2487`, `:2718`, `:2722`), every one treating ANY attachment as a divider. ✅ The divider already has its
  own class (`DividerTextAttachment`, `:2749`), which is a candidate for the type test.

## ⛔ TWO RULINGS OWED — ✅ recorded up front

⚠️ **Neither blocks the START. ✅ Both block the CLOSE.**

| # | ⚠️ Question | ⚠️ Recommendation |
| - | ---------- | ----------------- |
| **R1** | ⛔ **Is PASTED text escaped like typed text?** ⚠️ The ruling said *"that the user types"* | ✅ **RULED 2026-10-03 — YES** (user: *"R1 is yes."*). Pasted text is escaped exactly like typed text. |
| **R2** | ⛔ **Is there a one-time escape pass over EXISTING manuscripts?** ⚠️ They hold unescaped `*` today | ✅ **RULED 2026-10-03 — NO ESCAPE PASS** (user: *"R2 is no escape pass. The dumas projects do have ## at the top of every scene, but these are intended to represent formatted MArkup text, so no escape pass for that either."*). ✅ Existing scene files are never rewritten; ✅ existing Markdown in them (e.g. the `dumas` projects' `##` scene headings) is INTENDED markup and will render as such. |
| **R3** | ⛔ **How is the escape BACKSLASH hidden in E1?** ⚠️ **Raised 2026-10-03 at SP-154 planning — the design never says.** AC4 stores `*` as `\*`; the study measured that rendering attributes CANNOT hide a character (§4A.1) and scoped hiding to [EP-046]. ⛔ **As designed, the writer would SEE `\*`.** ⚠️ **Blocks AC4, not AC3.** | ✅ **RULED 2026-10-03 — (c)** (user: *"YEs R3 should be (c)."*): storage attributes hide the backslash; ONE caret-snap hook skips its unreachable boundary. Measured first (design §4.4). |

---

## ⛔ Known traps — each already paid for once

1. ⛔ **AC2's CAUSE IS UNKNOWN AND TWO FIXES HAVE ALREADY FAILED.** ✅ **Diagnose LIVE first
   (`feedback_prove_code_is_reached`); ⛔ do NOT ship a third speculative colour change.**
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
| **[SP-154]** | ✅ **AC3 two coordinate spaces** + **AC8 parsing mode** + ⚠️ **AC-R3 measurement for ruling R3** | 🟢 **COMPLETE 2026-10-03** (awaiting close) → [`../Sprints/Sprint-SP-154.md`](../Sprints/Sprint-SP-154.md) |
| **[SP-153]** | ⛔ **AC1 typed attachments (the data-loss item, all six readers)** + **AC9 save fidelity** + **AC2 Light-mode live look** | ✅ **CLOSED 2026-10-03** → [`../Sprints/Closed/Sprint-SP-153.md`](../Sprints/Closed/Sprint-SP-153.md) |

⚠️ Later Sprints (AC3–AC8, AC10) are NOT pre-allocated; IDs come from `docs/tools/next-ids.json` when each is created.
