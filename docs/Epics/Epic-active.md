# Active Epics

## EP-045: `[Apple]` ⚠️ **The Manuscript Renderer — Foundations**

**Status:** 🟡 **ACTIVE 2026-10-03** (user: *"Let's activate that epic next (EP-045?)."*) — created 2026-09-29. ✅ **[SP-153] CLOSED 2026-10-03 — AC1, AC2, AC9 MET** → [`../Sprints/Closed/Sprint-SP-153.md`](../Sprints/Closed/Sprint-SP-153.md). ⚠️ Remaining: AC3–AC8, AC10.
**Full record:** → [`Epic-EP-045.md`](Epic-EP-045.md) — ⚠️ **AC1–AC10 and TWO owed rulings.**
**Design:** → [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md)
**Authority:** → [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md)
(✅ **sixteen questions RULED 2026-09-29**).

**Goal:** ✅ **Make a RENDERED manuscript possible, and pay the data-loss debt that blocks it.**

⚠️ **IT RENDERS ALMOST NOTHING THE WRITER ASKED FOR, AND THAT IS DELIBERATE.** ✅ **Typed attachments
(⛔ a live corruption path — ⚠️ every attachment is currently treated as a scene divider, and
`sceneBoundaries` is what the save path slices with) · the SOURCE↔PRESENTED caret mapping · the escape
layer · Enter/Backspace · block-intent suppression · and the divider restored.**

⚠️ **SUPERSEDED 2026-10-03 — see AC2 in the full record: [T-0554] FOUND AND FIXED the divider's cause (third attempt), verified by the user 2026-09-29.** ~~THE DIVIDER'S CAUSE IS STILL UNKNOWN AFTER TWO FAILED FIXES~~ — ✅ **AC2 diagnoses LIVE before it
fixes.** ⚠️ **First: [T-0526] dropping [I-0112]'s appearance guard — ⛔ DISPROVEN by measurement.
Then: `separatorColor` at 1.34:1 — ⛔ FIXED, and the user reported *"they are still invisible."***

⚠️ **TWO RULINGS OWED:** ⛔ **PASTE** (⚠️ the escape ruling's words were *"that the user types"*) ·
⛔ **EXISTING MANUSCRIPTS** (⚠️ they hold unescaped `*` and will change appearance).

⛔ **VERIFIED GAP:** ✅ **`scrivi_merge_scene` exists (`scrivi.h:533`); ⛔ `scrivi_split_scene` DOES
NOT.** ⚠️ **The ruled scene-break COMMAND needs a core endpoint nobody has written — ✅ `[Cross]` work,
⛔ not in this Epic.**

✅ **SEQUENCES BEFORE [EP-032]** — ⚠️ **user ruling (study §8, Q1): ⛔ two Epics answering the same
FORMAT question independently will diverge, ✅ and [EP-032] cannot be built safely on today's
attachment handling anyway.** ✅ **[EP-032] keeps SP-107–SP-114 and its planning.**

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

