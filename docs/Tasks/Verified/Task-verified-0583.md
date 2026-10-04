# Verified Task — T-0583

**Sprint:** [SP-158] · **Epic:** [EP-045] (AC11) · **Platform:** `[Apple]`
**Implemented:** 2026-10-04 · ✅ **USER-VERIFIED 2026-10-04 by live check:** *"yes the text all landed where I put it and showed up where I expected. you have my approval to mark T-0583 verfied, close SP-158, and do the Audit Check for EP-045."*
**Archived:** 2026-10-04, with the Sprint close.

---

## ✅ T-0583 — `[Apple]` Cache the scene boundaries instead of rescanning the whole manuscript on every keystroke — Implemented 2026-10-04 · ✅ **VERIFIED 2026-10-04 (user)** ([SP-158])

**Created:** 2026-10-04 (user: *"Yes, lets create the Task to cache scene boundaries. Every little bit helps."*)
**Origin:** [SP-157] AC10 measurement → [`../../Sprints/Closed/Sprint-SP-157.md`](../../Sprints/Closed/Sprint-SP-157.md)
**Epic:** ✅ **[EP-045] AC11** · **Sprint:** 🟡 **[SP-158]** → [`../../Sprints/Closed/Sprint-SP-158.md`](../../Sprints/Closed/Sprint-SP-158.md) — ⛔ first filed with Epic "None" (an orphan); linked 2026-10-04 at the user's direction.
✅ **Linux clause RESOLVED at planning:** Linux already maintains this table (`SceneDocument::applyContentsChange`, `platforms/linux/src/SceneDocument.cpp:216`) — Apple adopts that shape; no Linux work.

✅ **What happens today:** every `textDidChange` calls `recomputeBoundaries` → `SceneDivider.sceneBoundaries`
(`ManuscriptTextView.swift:1714`, `:2917`), which enumerates `.scriviDivider` AND `.scriviHeading` over the WHOLE
storage to rebuild 1,185 scene ranges — only to learn which scene the edit landed in. ✅ Measured on the 1.85 MB
`dumas-prose-timelines`: **`[SCRIVI-KEY] bounds` 2.3–3.9 ms per keystroke**, flat across the document. ⚠️ The same
two scans predate E1 (`d4e279e:1692`).

✅ **The change (the user's design):** keep `sceneBoundaries` as a maintained table. An edit inside scene *i* changes
only scene *i*'s length and shifts every later scene by the edit's `changeInLength` — either shift them at once
(~1,185 integer adds, microseconds) or mark *i+1…* dirty and fix them on demand. ✅ **Full rescan ONLY after a
structural change:** project load / `rebuildStorage`, import, paste or delete ACROSS scenes, merge, split, a
structural undo, toggling chapter titles.

⛔ **Constraint — ONE authority:** [I-0131] was a SECOND scene-offset table (`sceneStorageOffsetMap`) drifting from
the real one. The cached table must REPLACE the per-keystroke scan, not sit beside another copy.
✅ **Acceptance:** (1) `[SCRIVI-KEY] bounds` ≈ 0 for an ordinary keystroke on the 1.85 MB fixture; (2) a test that
after typing, Return, ⌫-join, paste and cross-scene delete the cached table EQUALS a full rescan; (3) no regression
in save fidelity (EP-045 AC9 test).
⚠️ **Expectation, stated so it is not oversold:** this saves ~3 ms of a 20–120 ms keystroke. The offset-linear
cost is AppKit's → [I-0275].
⚠️ **Linux** (`feedback_linux_adopts_apple_shape`): check whether the Linux editor rescans per keystroke and adopt
the same shape in the same work.


✅ **Implemented 2026-10-04:** `SceneBoundaryTable` maintained from `willProcessEditing` (⛔ `didProcessEditing` measured
WRONG — `editedRange` widened to whole lines); `sceneBoundaries` reads it; `rebuildStorage` installs exact ranges;
twelve call sites → `ensureBoundaries`. ✅ Tests equal a full rescan AND prove no rescan for in-scene edits; mutation
fails 8; 167/167; three platforms build. ✅ Live check PASSED: `bounds=0.0`; only 2 of ~150 keystrokes logged `[SCRIVI-KEY]` at all (both Returns' undo commit, 0.7–0.9 ms); every edit saved to the caret's scene.
