---
sprint: SP-188
epic: EP-053
status: Planning
platform: Cross
created: 2026-10-20
---

# SP-188 — `[Apple]` + `[Linux]` [EP-053] **S3**: structural cut, Backspace / Delete over scenes, and their undo

**Status:** 🔵 **PLANNING** — created 2026-10-20 (user: *"complete planning for EP-053"*). ⏳ Q1–Q2 below.
**Carries:** [T-0616] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1, `scrivi_range_cut`) · [SP-187] + [SP-191] (S2a + S2b: the scene list as the model of record, the delta applied,
`rangePaste` history).
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_3.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_3.md) (v0.3, 2026-10-21, supersedes v0.2) — R4,
§4.3, §4.5, Q2, Q4. ⛔ R4 **reverses** the 2026-10-02 ruling ([I-0270]: a cross-scene cut deletes text and keeps scenes).
**Ruling that shaped it (user, 2026-10-20):** ✅ each operation ships with its undo; ✅ the `rangeCut` history op name is
approved, backward-compatible.
**Size:** L.

---

## What reading the code found (2026-10-20)

- **Apple:** ⌘X (`ManuscriptTextView.swift:3285`), ⌥N cut into a buffer (`cutIntoBuffer`, `:927`), and ⌫ / ⌦ over a
  cross-scene selection all go to `deleteAcrossScenes` (`:2581`). It removes the selected text from each scene, keeps every
  scene, and records ONE edit group ([I-0270]).
- **Linux:** the same shape. `ManuscriptEditor::deleteAcrossScenes` (`ManuscriptEditor.cpp:1183`) is reached from the cut
  key (`:498`), `cutSelection` (`:1067`, `:1090`), Delete (`:966`) and the buffer cut (`:1131`).
- **History:** old `structuredCut` nodes undo through `applyStructuralInverse` on both (Apple `:615`, Linux
  `EditorShell.cpp:2573`). [I-0270]'s grouped-delete nodes undo as ordinary edit groups.
- ✅ So every cross-scene cut and delete already funnels through ONE function per platform. That is the switch point.

## Goal

✅ A cut or a Backspace / Delete across scene or chapter boundaries **removes the structure** on both platforms: the start
scene's head joins the end scene's tail directly, the scenes between and the end scene go, emptied chapters go, and the end
chapter's later scenes join the start scene's chapter (Q2). One `scrivi_range_cut` off the main thread, the delta applied,
no reload. Undo is a range paste; redo is the cut again.

## Plan

0. **Red tests first:** a cross-scene cut on each platform must leave the scene count lower and make one core call off the
   main thread. Both fail against `deleteAcrossScenes` today.
1. **Cut** (⌘X / Ctrl+X, and the buffer cut ⌥N / Alt+N): the editor builds the Range from its list (S2), puts it on the
   clipboard or in the buffer, then calls `scrivi_range_cut` off the main thread, read-only with the progress row after
   400 ms. The delta is applied.
2. **Backspace / Delete over scenes = Cut minus the clipboard** (design Q4). The same call; the clipboard and buffers are
   left alone.
3. **History:** a `rangeCut` node. Undo = `scrivi_range_paste` of the cut's Range at the cut's start; redo = the cut again.
   Delta applied both ways.
4. **Old nodes still undo:** `structuredCut` (through the fragment-to-Range conversion, per [SP-187] Q2's ruling) and
   [I-0270]'s grouped deletes (unchanged: they are text edits).
5. **Retire** the cross-scene branch of `deleteAcrossScenes` on both. A selection inside one scene keeps the ordinary path.
6. **Tests** on both: cut, then undo, restores the disk and the list exactly; a cut that empties a chapter removes it; Q2's
   promotion; Backspace leaves the clipboard unchanged; an old `structuredCut` node and an old grouped delete still undo.
   Mutation-checked.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **A selection that starts or ends on a chapter title** (Apple draws it; Linux draws it when titles are on, [SP-184]). Does the cut take the chapter? | ⏳ **Only when the selection covers the whole chapter.** Otherwise the range starts at the first scene body the selection reaches. A title is presentation (it is not in any scene's text). |
| **Q2** | **Backspace over a selection of exactly one scene divider** (no text either side): merge the scenes, or refuse? | ⏳ **Merge**, the same as ⌘⌫ / Ctrl+Backspace's scene merge, recorded as a `rangeCut` of an empty range. Today both platforms refuse to remove a divider by an ordinary edit ([I-0270]). |

⛔ **NOT in this Sprint:** the background sync verification ([SP-189]).

## Acceptance Criteria

- [ ] AC1 — Cut across scenes and chapters removes the structure (§4.3, Q2), one `scrivi_range_cut` off the main thread,
      delta applied, no reload, on both
- [ ] AC2 — Backspace / Delete over scenes: the same, with the clipboard and buffers untouched (Q4)
- [ ] AC3 — The buffer cut (⌥N / Alt+N) puts the Range in the buffer and cuts the same way
- [ ] AC4 — Undo = range paste, redo = cut, `rangeCut` node; cut∘undo restores disk and memory exactly
- [ ] AC5 — Old `structuredCut` nodes and [I-0270] grouped deletes still undo on both
- [ ] AC6 — Tests on both, mutation-checked; ctest, interop and Linux smokes green
- [ ] AC7 — ✅ **Live passes (user):** Mac and rig, over the share

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-187], [SP-189], [SP-190] via `next-id.py`; [T-0616] filed.
- Q1–Q2 await rulings.

### 🔵 2026-10-21 — Affected by [SP-191]'s Q3 and Q4
- [SP-191] Q3 (ruled): split and merge become undoable range operations, with no barriers. Its Q3b (✅ agreed 2026-10-21)
  puts each whole scene's sidecar (and a removed chapter's metadata) in the Range, built in SP-191's Plan 1. This Sprint's
  cut undo uses it, so an undone cut restores the tags, story time, band, outline, to-dos and status. ⚠️ A consequence: a
  cut-then-paste elsewhere keeps the scenes' IDs (`pickSceneID`), so the scenes move WITH their sidecars.
- [SP-191] Q3e (ruled): the manuscript is one Range, and a cut of any sub-range joins the scenes at its ends, including a
  zero-length one at a chapter break. This Sprint's cut follows the same rule (design §4.3). Chapter merge is a relabel.
- [SP-191] Q4/Q4a (ruled): only the clipboard and copy buffers drop old-style objects (they read as empty) and store Ranges.
  History keeps [SP-187] Q2's conversion, so this Sprint's "old `structuredCut` nodes still undo" (Plan 4, AC5) stands.
