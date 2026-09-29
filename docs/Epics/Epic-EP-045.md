# EP-045: `[Apple]` **The Manuscript Renderer — Foundations**

**Status:** 🔵 **Draft** — created 2026-09-29. ⛔ **No Sprint assigned; not activated.**
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

## ⛔ TWO RULINGS OWED — ✅ recorded up front

⚠️ **Neither blocks the START. ✅ Both block the CLOSE.**

| # | ⚠️ Question | ⚠️ Recommendation |
| - | ---------- | ----------------- |
| **R1** | ⛔ **Is PASTED text escaped like typed text?** ⚠️ The ruling said *"that the user types"* | ⚠️ **Escape it** — ✅ consistency beats the rarer case |
| **R2** | ⛔ **Is there a one-time escape pass over EXISTING manuscripts?** ⚠️ They hold unescaped `*` today | ⚠️ **NO migration** — ⛔ a bulk rewrite of a writer's prose is the larger risk |

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

⛔ **NONE ASSIGNED.** ⚠️ **Sprint numbering is a HIGH-WATER MARK, not a free list** — ✅ **allocate from
`docs/tools/next-ids.json` at activation, ⛔ honouring [EP-043]'s SP-146–SP-148 reservation.**
