# Verified Task — T-0616

**Sprint:** [SP-188] · **Epic:** [EP-053] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-04** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`)
**Archived:** 2026-11-04.

---

## T-0616 — `[Cross]` EP-053 S3: structural cut, Backspace / Delete over scenes, and their undo

**Epic:** [EP-053] (S3) · **Status:** ✅ **Verified 2026-11-04** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`) · **Sprint:** [SP-188] (✅ Closed 2026-10-26) →
[`../Sprints/Closed/Sprint-SP-188.md`](../Sprints/Closed/Sprint-SP-188.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** on the Mac and on Linux, a cut, buffer cut, or Backspace / Delete across scenes removes the structure (design R4,
§4.3, Q2, Q4) through one `scrivi_range_cut` off the main thread, the delta applied, no reload; undo is a range paste
(`rangeCut` node), redo the cut. The core balances the cut's join and returns the markers it added, so the undo is exact.
The clipboard (or slot) holds the cut's returned Range with its sidecars; a selection crossing a divider takes that scene
whole; Backspace over one scene divider merges; paste over a cross-scene selection is the cut, then the paste. Old
`structuredCut` nodes and [I-0270]'s grouped deletes still undo. Retires both platforms' `deleteAcrossScenes`.

**Done (2026-10-22):** ScriviCore (`RangeJoin`, the balanced join in `cut()`, applied and checked in `paste()`); the Mac
(`selectionRun`, `cutAcrossScenes(_:keeping:then:)`) and Linux (`ManuscriptEditor::selectionRun`,
`EditorShell::cutAcrossScenes`) — every cross-scene cut, Backspace / Delete and paste-over one range call off the main
thread, the delta applied, undone in one step; `deleteAcrossScenes` retired on both. ctest 763 (Mac) / 767 (Linux),
interop 311, Linux smokes 38; mutations core 6, Mac 8, Linux 7, all caught.

**Verified (2026-11-04):** ✅ the rig over the share — [T-0661] run 2 (build 1.0.348): 24 edits, no stall after any, every
verification 0 corrected. ✅ The Mac on a local disk (AC re-ruled 2026-11-04) — build 1.0.349: 22 edits, undos and redos, every
verification `0 corrected, 0 skipped, 0 not correctable`; `… landed after` 62–108 ms (scene edits), 180–253 ms (chapter edits);
the only stalls ~1 s at launch, before any edit.
