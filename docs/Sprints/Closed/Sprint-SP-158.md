---
sprint: SP-158
epic: EP-045
status: Closed
closed: 2026-10-04
activated: 2026-10-04
platform: Apple
created: 2026-10-04
---

# SP-158 — `[Apple]` [EP-045] S6: Maintain the scene-boundary table instead of rescanning it (AC11)

**Status:** ✅ **CLOSED 2026-10-04 (user-approved):** *"yes the text all landed where I put it and showed up where I expected. you have my approval to mark T-0583 verfied, close SP-158, and do the Audit Check for EP-045."*
**Epic:** [EP-045] → [`../../Epics/Closed/Epic-EP-045.md`](../../Epics/Closed/Epic-EP-045.md) — ⚠️ **AC11, added 2026-10-04.** EP-045 cannot close until it is met.
**Task:** [T-0583] → [`../../Tasks/Verified/Task-verified-0583.md`](../../Tasks/Verified/Task-verified-0583.md)
**Origin:** [SP-157] AC10 → [`Closed/Sprint-SP-157.md`](Sprint-SP-157.md)
**Size:** ⚠️ **SMALL–MEDIUM.** The change is small; it touches nine call sites, and a wrong table puts edits in the wrong scene file.

---

## Why this Sprint exists

✅ User, 2026-10-04: *"My assumption is always that the new task is linked to the Sprint/Epic in which it was created
unless expressly stated otherwise. … Please either correct my assumption or create a new Sprint for this one task."*
⛔ **I had filed [T-0583] with Epic "None" and no Sprint — the orphan pattern the user described.** ✅ It is now
EP-045's AC11, and this Sprint carries it.

---

## ⚠️ Measured at planning (2026-10-04)

### What the scan is, and where it runs

✅ `sceneBoundaries` holds the storage range of each scene's text: 1,185 ranges on `dumas-prose-timelines`. It is how
the editor knows which scene an edit, the caret or the viewport is in. ⛔ `recomputeBoundaries` → `SceneDivider.sceneBoundaries`
(`ManuscriptTextView.swift:1714`, `:2917`) REBUILDS it by enumerating `.scriviDivider` and `.scriviHeading` over the
WHOLE storage. ✅ Measured in the app: **`[SCRIVI-KEY] bounds` 2.3–3.9 ms per keystroke**, flat across the document.

⚠️ **It runs at NINE call sites, not one:**

| Line | Caller | How often |
| ---- | ------ | --------- |
| `:909` | `textDidChange` | ⛔ **every keystroke** — the cost measured above |
| `:442`, `:463` | `applySceneChange` (undo / redo apply) | per undo step |
| `:673` | `scrollDidChange` | once per settled scroll (120 ms debounce) |
| `:1744` | `restoreWritingSurface` | on open / restore |
| `:1842` | `navigateToScene` | per Navigator jump |
| `:1968` | `sceneStorageRange(containing:)` | per chapter/manuscript move |
| `:1982`, `:2014` | `moveToChapterBoundary`, `moveToManuscriptBoundary` | per command |

### Linux already has this design

✅ **Linux's `SceneDocument` keeps an offset map that is the single authority for scene ownership**, and
`applyContentsChange(pos, charsRemoved, charsAdded)` (`platforms/linux/src/SceneDocument.cpp:216-242`) adjusts the
edited scene's length and shifts every LATER scene by the delta, without rescanning. Structural changes go through
`reflowBoundaryAt`. ✅ **So Apple adopts the shape Linux already ships. No Linux work** (checked for
[T-0583]'s Linux clause).

### The one constraint carried from history

⛔ **[I-0131]** was a SECOND scene-offset table (`sceneStorageOffsetMap`) drifting from the real one, which put the
caret about one scene early. ✅ The maintained table REPLACES the per-keystroke rescan. It must never sit beside
another copy. A full rescan stays as the fallback whenever the table cannot be trusted.

---

## Plan

1. **Keep `sceneBoundaries` current from each edit's own facts:** the edited range and `changeInLength` that
   `NSTextStorage` reports after every edit. An edit wholly inside scene *i* changes scene *i*'s length and shifts
   scenes *i+1…* by the delta, as Linux does (eager: about 1,185 integer adds, microseconds).
   ⚠️ **The storage DELEGATE slot is already taken** by `EscapeHidingStyler` (AC4). The delta must come from the
   `didProcessEditing` NOTIFICATION, or be chained from the styler. ✅ This is measured and decided at activation,
   not assumed.
2. **Anything else marks the table DIRTY**, and the next reader rescans in full. That covers an edit that touches a
   divider or heading run or spans scenes, `rebuildStorage` (project load, import, reload), cross-scene delete and
   paste, merge, split, and toggling chapter titles.
3. **The nine call sites become one `ensureBoundaries()`:** it rescans only when the table is dirty. `textDidChange`
   stops rescanning.

---

## Acceptance Criteria (= EP-045 AC11)

- [x] **AC11a — No whole-manuscript scan on an ordinary keystroke:** `[SCRIVI-KEY] bounds` ≈ 0 (well below 0.5 ms) on the
  1.85 MB fixture, typing at the start, middle and end (it was 2.3–3.9 ms).
- [x] **AC11b — The maintained table EQUALS a full rescan**, tested through the real view after: typing,
  Return, ⌫-join (Q3), paste within a scene, cross-scene delete, an undo apply, and a rebuild. ✅ A debug-build
  self-check (maintained vs rescanned, compared on demand) is used by the tests and never runs per keystroke in release.
- [x] **AC11c — Save fidelity unchanged:** the EP-045 AC9 test (edit → save → reload round-trips every scene) stays
  green. ⚠️ A wrong table saves text into the wrong scene file, so this is the guard that matters.
- [x] **AC11d — Live check:** the user types and navigates on `dumas-prose-timelines`; the console shows `bounds` ≈ 0
  and every edit lands in the right scene.
- [x] **AC-build** — macOS + iOS + visionOS; interop green via `scripts/run-interop-tests.sh`.

⚠️ **Expectation, stated so it is not oversold:** this removes about 3 ms of a 20–120 ms keystroke. ⛔ The cost that grows
with position is AppKit's → [I-0275] (Issue backlog, unassigned; not in this Sprint).

---

## Progress log

### 🔵 2026-10-04 — Sprint CREATED in Planning

✅ Created at the user's request (above). ✅ ID issued by `next-id.py`. ✅ Measured first: nine call sites of the
scan; Linux's `SceneDocument::applyContentsChange` already implements the design. ✅ [T-0583] linked to EP-045
(AC11) and this Sprint.

### 🟡 2026-10-04 — Sprint ACTIVATED (user-approved); [T-0583] moved to `Task-active.md`

✅ User: *"yes, add it to I-0275 and activate SP-158"*. ⚠️ **First implementation step:** measure where the edit's
`editedRange` / `changeInLength` can be read (the storage delegate slot belongs to `EscapeHidingStyler`) — decided by
measurement, not assumed.

### 🟢 2026-10-04 — AC11 implemented ([T-0583]); ⚠️ AC11a + AC11d need the user's live check

✅ **Built:** `SceneBoundaryTable` (`ManuscriptTextView.swift`, next to `SceneDivider`) — the maintained table, with a
binary search for the edited scene, the Linux shift, and DIRTY → full rescan for anything it cannot prove local (an
edit spanning scenes, or inserted text carrying a divider or heading). ✅ `sceneBoundaries` is now a computed
property reading the table (current, or rescanned when dirty — ONE authority). ✅ `rebuildStorage` builds its ranges
locally and installs them with `reset` after `endEditing`, so a load costs no rescan. ✅ All twelve
`recomputeBoundaries(tv)` calls → `ensureBoundaries(tv)` (a no-op unless dirty).
⛔→✅ **MEASURED, and it changed the design:** observing `didProcessEditing` FAILED 3 of the new checks — by then
`editedRange` is widened to whole lines, and a scene's last line holds its divider character, so every edit there
looked cross-scene and rescanned. ✅ `willProcessEditing` carries the raw edit: all pass.
✅ **Tests** (suite "Scene-boundary table (EP-045 AC11)", through the real view and AppKit's key dispatch): typing with
escapes, Return, ⌫-join, typing after a heading and at the very end, an undo-style whole-scene replacement, an
emptied and refilled scene — each EQUALS a full rescan AND left the table clean (no rescan); a cross-scene
replacement and an inserted divider mark it dirty and the rescan is exact; rebuild + `reset`.
✅ **Mutation:** removing the shift of later scenes fails 8 checks; restored. ✅ **167/167**, incl. the AC9 save
round-trip; macOS / iOS / visionOS BUILD SUCCEEDED.
⚠️ **Owed (user):** AC11a (`[SCRIVI-KEY] bounds` ≈ 0 on the 1.85 MB fixture) + AC11d (edits land in the right scene).

### ✅ 2026-10-04 — live check PASSED; [T-0583] VERIFIED; Sprint CLOSED (user-approved)

✅ User: *"yes the text all landed where I put it and showed up where I expected. you have my approval to mark T-0583 verfied, close SP-158, and do the Audit Check for EP-045."*
✅ **AC11a — measured in the user's console (`dumas-prose-timelines`, 1,855,917 chars):** `bounds=0.0`. ✅ Only **2** of
~150 keystrokes logged `[SCRIVI-KEY]` at all (it logs above 0.5 ms; the previous run logged EVERY keystroke at
2.3–4.3 ms) — both Returns, 0.9 / 0.7 ms, all in `rest` (the undo commit a Return triggers). ✅ Load: one 4.7 ms
styler pass, no rescan.
✅ **`keyDown`:** start 16–37 ms (was 20–43), scene 15 18–43 ms, end 89–121 ms (was 89–122) — ✅ as expected: ~3 ms
saved; the position-dependent cost is AppKit's ([I-0275], unchanged).
✅ **AC11d:** every `saveSceneBlocking` wrote the caret's scene (0, 15, 1184 — including at the end, where the viewport
scene 1183 differed from the caret's), ✅ and the user confirmed the text landed where typed.
✅ **EP-045 AC11 MET.**

## Retrospective

**Completed:** ✅ AC11 ([T-0583]): `SceneBoundaryTable` maintained from each edit (the Linux shape), one authority,
rescan only when dirty; twelve call sites → `ensureBoundaries`.
**Returned to Backlog:** none.
**What went well:** ✅ the tests asserted "no rescan happened" as well as "equals a rescan" — ✅ which is what caught
the `didProcessEditing` widening; a table that silently rescanned would have passed equality. ✅ Checking Linux
first found the design already shipped there.
**What to improve:** ⛔ [T-0583] was first filed with Epic "None" — the orphan pattern the user named. ✅ A Task
belongs to the Sprint/Epic it was created in unless stated otherwise.
**Carry-forward notes:** ✅ EP-045 AC1–AC11 met → Audit Check, then the close.

