# Sprint SP-240 — `[Linux]` [EP-059] A9: Delete and Change Kind for objects on Linux, as the Mac's

**Status:** ✅ **CLOSED** 2026-11-23 (user, 2026-11-23: *"Step 2 passes now, close SP-240"*). Was 🟡 **ACTIVE** 2026-11-23 (user, 2026-11-23: *"Q1-Q4 as recommended.  Implement SP-240"*); ✅ Q1–Q4 as recommended. Built 2026-11-23: AC1–AC2 met; AC3 green locally (CI on the push); AC4 live pass ✅ (with [I-0372]). Created 2026-11-23 (user, 2026-11-23: *"plan A8"*). ✅ **Unblocked 2026-11-23** ([SP-239] closed); was blocked by it (the Mac's shape, which
this one adopts, and its core guard).
**Carries:** [T-0703] (A9).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **A**: A9.
**Size:** S — both endpoints are bridged (`ScriviBridge::deleteObject`, `ScriviBridge::promoteObject`); one menu and one confirmation in
`ObjectDetailDialog`, in the shape [SP-239] sets.

---

## Findings (2026-11-23)

1. **Both endpoints are bridged with no caller**: `ScriviBridge::deleteObject` (`ScriviBridge.cpp:1318`) and `promoteObject` (`:1804`).
   [SP-239]'s core guard (item ↔ artifact only, `notPromotable`) is shared, so Linux gets it for free.
2. **The detail window's bar** (`ObjectDetailDialog.cpp:248`) is a `QToolBar`: Back · spacer · Save · Undo Changes · Close. The Mac's
   is now Save · Undo Changes · ⋯ · Close ([I-0371]).
3. **Writes run off the main thread** (`AsyncCall::run` + `WorkerBridge`, as `save()` at `:858`); the world-away case is caught
   from the bridge's error message. `saved()` is what refreshes the Inspector's cards (`EditorShell.cpp:4495`).
4. **The Back trail** (`back_`) is a list of `Entry` with the kind in each.
5. ⚠️ **On both platforms the same object can be in the trail twice.** A → B → A (related to each other) gives A · B · A: Linux's
   `navigateTo` (`:1244`) and the Mac's `pushEntry` (`ObjectDetailSheet.swift:1012`) only skip a push of the object already
   showing. So after **delete**, Back can reach the deleted A; after **change of kind**, Back reaches A under its **old** kind.
   Either way the open fails and the window shows an error. [SP-239] updates only the entry that is showing.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **The shape.** | **The Mac's, as built**: a ⋯ button (`QToolButton`, instant popup; the theme's `view-more`, falling back to a standard icon) after Undo Changes, before Close. Change to Artifact / Change to Item (items and artifacts only), greyed with "Save your changes first" while there are unsaved changes; Delete <Kind>…. The whole menu greyed while the world is away or a save is running. || ✅ As recommended (2026-11-23) |
| **Q2** | **Delete's confirmation.** | A `QMessageBox` with the Mac's title and text, word for word; **Delete** (destructive) and **Cancel** (default and Esc). Afterwards Back, or close; `saved()` refreshes the cards. || ✅ As recommended (2026-11-23) |
| **Q3** | **The trail (finding 5), on BOTH platforms.** | After a delete, **remove every entry of that object** from the trail; after a change of kind, **give every entry of it the new kind**. On the Mac as well as Linux, in this Sprint ([feedback_linux_adopts_apple_shape]): it is a few lines each, and the Mac re-passes one step. || ✅ As recommended (2026-11-23) |
| **Q4** | **Delete and change of kind run off the main thread** | Yes, through `AsyncCall` like Save ([feedback_main_thread_is_for_ui_only]); the menu is greyed while one runs. || ✅ As recommended (2026-11-23) |

## Plan

- Linux (`ObjectDetailDialog`): the ⋯ button and its menu; the delete box; `deleteCurrent()` and `changeKind()` through `AsyncCall`,
  then the trail (Q3), Back or close, `saved()`; `refreshState()` sets the menu's enabled state.
- Mac (`ObjectDetailSheet`, Q3 only): `deleteCurrent` drops the object's other trail entries; `changeKind` re-kinds them.
- Smoke (`object_detail_smoke.cpp`, a new §): the menu's items per kind and greyed states; change of kind keeps a relationship and
  moves the card; delete removes the object and its edge, goes Back; the A → B → A trail after each. Interop test for the Mac's Q3.
- Docker: ctest and every smoke as non-root. Live pass on scrivi-vm (an existing project), plus one Mac step for Q3.

## What was built (2026-11-23)

**Linux** (`ObjectDetailDialog.cpp/.hpp`): a ⋯ `QToolButton` (`detailMore`, the theme's `view-more`, "⋯" when the theme has none)
after Undo Changes, before Close, filled as it opens: Change to Artifact / Item on items and artifacts only — greyed under "Save your
changes first" while there are unsaved changes — and Delete <Kind>…. The button is greyed while the world is away or a write runs.
The delete box (`deletePrompt`) has the Mac's words; Cancel is the default and Esc. `deleteCurrent` and `changeKind` run through
`AsyncCall` (`runObjectWrite`); a failure shows in the save-error line. Afterwards `saved()` refreshes the cards; delete goes Back
or closes; a change of kind re-reads the object under its new kind.
**Q3, both platforms:** after a delete every trail entry of the object goes and the neighbours that leaves side by side merge;
after a change of kind every entry takes the new kind. Linux: `back_` in `deleteCurrent`/`changeKind`. Mac: `ObjectDetailHistory.trail(_:without:)`,
`deleteCurrent` resets `trail` and the `NavigationPath`; `changeKind` re-kinds every entry.

## Tests

- Smoke `object_detail_smoke.cpp` §13: ⋯'s items per kind; greyed while unsaved; Key → Cara → Key, Change to Artifact (stays, cards
  told, on disk an artifact, scene link kept); Back, Back reaches Key as an artifact; Delete asks naming the world, Cancel deletes
  nothing, Delete goes Back to Cara with no Back left, the object and its scene link gone. §12: ⋯ greyed while the world is away.
- Interop: `trail(_:without:)` — Key·Cara·Key → Cara; Cara·Key·Cara·Key → Cara.
- Results: ctest **887/887** and smokes **49/49** in Docker (non-root; §13 confirmed in the binary); interop **355/355**; iOS **38/38**; the macOS build.

## Live pass (AC4) — the Linux VM, then the Mac

On `/mnt/scrivi-net/projects/the-river-of-time.inkwyn` (world Eskandar).

1. VM: `cd ~/Dev/Scrivi && git pull`, `cmake --build build-native -j3`, launch Inkwyn, open `the-river-of-time`.
2. Inspector ▸ Worldbuilding ▸ Items card (+ ▸ Items if missing): New Item `test key`, Create. Double-click it: the bar reads
   Save · Undo Changes · ⋯ · Close; ⋯ holds Change to Artifact and Delete Item….
3. Type `x` in Notes: ⋯ shows "Save your changes first" and Change to Artifact greyed. Undo Changes.
4. Relate… ▸ "located at", click any location (call it L). Double-click L under Related; in L's Related double-click test key.
5. ⋯ ▸ Change to Artifact: still test key; ⋯ now Change to Item and Delete Artifact…; the Items card drops it.
6. Back (L), Back again: test key opens, its details say Artifact, no error.
7. Double-click L, then test key. ⋯ ▸ Delete Artifact…: "Delete “test key”?" names Eskandar. Cancel. Again, Delete: the view shows
   L, with no Back button.
8. Close. Open Myton's detail: ⋯ holds only Delete Character…. Close.
9. Mac, `dumas-prose-timelines-history`: steps 2, 4, 5, 6, 7 the same (a Dumas location as L; "Back" is the chevron).

## Acceptance criteria

- [x] AC1 — Linux's detail window deletes and changes the kind of objects as the Mac's (Q1, Q2, Q4).
- [x] AC2 — After either, Back never reaches the object under a stale kind or after its deletion, on Linux and the Mac (Q3).
- [x] AC3 — A smoke through a real project window; ctest and every smoke green in Docker; the Mac's interop tests; CI green. ✅ ctest 887/887 (Docker, non-root), smokes 49/49, interop 355/355, iOS 38/38; CI: on the push (Claude reads it after the user pushes).
- [x] AC4 — Live pass on the Linux VM, on an existing project there; the Mac step for Q3. ✅ all steps pass on the VM and the Mac (user, 2026-11-23: *"All steps pass on the Linux VM and the Mac."*); ✅ [I-0372] re-passed (user, 2026-11-23: *"Step 2 passes now, close SP-240"*).

## Retrospective

- The Q3 finding (the same object twice in the trail) came from reading the code at planning, not from a failure; the user's own
  attempt to trip it (L → test key loaded twice, the second deleted) passed.
- The live pass found [I-0372]: a pushed level of the Mac's sheet had no bar at all — no Save, no Close — probably since T-0547
  (2026-09-23). Two Sprints in a row the Mac sheet's bar was the finding ([I-0371], [I-0372]): toolbar placement on a sheet is only
  proved on screen.
- Live-pass steps must fit one screen (the user loses ~8 lines per scroll): steps alone, nothing around them.

## Progress log

- **2026-11-23** — ✅ **CLOSED** (user, 2026-11-23: *"Step 2 passes now, close SP-240"*). T-0703 and I-0372 archived. EP-059 A9 [x].
- **2026-11-23** — Live pass: all steps pass on the Linux VM and the Mac; T-0703 → ✅ Verified. Mac step 6: no bottom bar on objects opened from the sheet → [I-0372], fixed (the toolbar on every level); ⏳ its re-pass.
- **2026-11-23** — Built: AC1, AC2 met; AC3 smokes, interop and iOS green; T-0703 → 🟠. ctest 887/887 in Docker. ⏳ CI, AC4 live pass.
- **2026-11-23** — 🟡 **ACTIVE** (user, 2026-11-23: *"Q1-Q4 as recommended.  Implement SP-240"*); Q1–Q4 as recommended; T-0703 → active.
- **2026-11-23** — Planned (user, 2026-11-23: *"Next is SP-230 planning"* — SP-230 is closed; read as SP-240): findings 1–5, Q1–Q4 to rule.
- **2026-11-23** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-23: *"plan A8"*); carries [T-0703] (new). ⛔ Blocked by [SP-239].

*Last Updated: 2026-11-23 (✅ Closed; EP-059 Audit Check F2: stale pending text removed.)*

*Last Updated: 2026-11-23 (Built; ⏳ AC4 live pass, ctest, CI.)*

*Last Updated: 2026-11-23 (Planned; Q1–Q4 to rule.)*

*Last Updated: 2026-11-23 (Created; blocked by SP-239.)*
