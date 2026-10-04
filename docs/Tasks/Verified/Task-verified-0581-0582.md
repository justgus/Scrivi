# Verified Tasks — T-0581 · T-0582

**Sprint:** [SP-157] · **Epic:** [EP-045] (AC7, AC10) · **Platform:** `[Apple]`
**Implemented:** 2026-10-04 · ✅ **USER-VERIFIED 2026-10-04:** T-0581 by live test (*"yes no backslashes appeared."*);
T-0582's measurement accepted with the close (*"You may close SP-157, archive T-0581 and T-0582."*).
**Archived:** 2026-10-04, with the Sprint close.
⚠️ **Follow-ups filed from T-0582:** [T-0583] (cache scene boundaries, backlog) and [I-0275] (keystroke cost grows
with position — the user is *"kind of concerned"*, not ruled as biting).

---

## ✅ T-0581 — `[Apple]` Unexposed block intents are prose ([EP-045] AC7) — Implemented 2026-10-04 · ✅ **VERIFIED 2026-10-04 (user):** *"yes no backslashes appeared."*

**Sprint:** [SP-157] → [`../../Sprints/Closed/Sprint-SP-157.md`](../../Sprints/Closed/Sprint-SP-157.md) · ✅ ✅ Q-AC7 = (a), RULED by the user 2026-10-04.
✅ **No behaviour change — the map ALREADY read indented text as prose; this codifies it.** `ManuscriptEscapes.swift`:
the AC7 rule stated on `MarkdownEscapes.map`. Design §7.1 amended. ✅ [EP-046]'s backlog entry gains the carried AC
(*"unexposed block intents are DRAWN as prose"*).
✅ **Tests:** `indentedParagraphIsProse` (design AC7's own test, 4-space + tab), `corpusIsProse` (2,000/2,000 against the
prose oracle; exactly 42 differ from plain `.full`), suite "Block intents as prose (EP-045 AC7)" through the real view.
✅ **Mutation check:** making the map read indented lines as code blocks fails all three (plus the AC3 corpus) — 50
issues; restored. ✅ 165/165; macOS / iOS / visionOS BUILD SUCCEEDED.
✅ **Live test PASSED 2026-10-04.**

---

## ✅ T-0582 — `[Apple]` The caret path, measured on the 1.85 MB manuscript ([EP-045] AC10) — Implemented 2026-10-04 · ✅ **VERIFIED 2026-10-04 (user)** — archived with the close

**Sprint:** [SP-157] → [`../../Sprints/Closed/Sprint-SP-157.md`](../../Sprints/Closed/Sprint-SP-157.md)
✅ **AC10a:** `[SCRIVI-EDIT] restyle=` (`EscapeHidingStyler`'s delegate) and `[SCRIVI-EDIT] caretSnap=`
(`setSelectedRanges`), logged only above 0.5 ms. Logging only.
✅ **AC10b (harness, real `dumas-prose-timelines` text, 1,185 scenes / 1,855,198 chars, real styler + map extracted at
build time):** full-document restyle **~4 ms** (rebuild); per-keystroke restyle **~0.002 ms** at all three [I-0206]
offsets (0.008 ms in a paragraph with a backslash); a storage edit near the end incl. the styler **0.018 ms**;
caret snap **< 0.001 ms**. ✅ **E1's own additions are noise against [I-0206]'s 58–89 ms keystroke.**
⚠️ Harness limits: no on-screen layout; dividers only (no chapter headings); `ManuscriptNSTextView` overrides not
included — AC10c covers them.
✅ **AC10c (the user's run, 2026-10-04):** keystroke cost is LINEAR IN OFFSET (typing ~20–43 ms at the start → ~89–122
ms at the end; arrows 1 → 90 ms), our own work flat at 2.3–4.3 ms; E1's restyle ≤0.7 ms, snap never measurable;
`center` now ~1 ms (was ~92). ⚠️ Revises [I-0206]'s "typing is constant". Full tables → [SP-157].
