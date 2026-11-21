---
sprint: SP-231
epic: EP-059
status: Closed
platform: Linux
created: 2026-11-21
---

# SP-231 — `[Linux]` [EP-059] D: create a world, Reconnect Worlds, the world-warning strip

**Status:** ✅ **CLOSED** 2026-11-21 (user, 2026-11-21: *"Close SP-231 and plan SP-232"*). Was 🟡 **ACTIVE** (user, 2026-11-21: *"Q1-Q4 as recommended, implement SP-231"*); ✅ Q1–Q4 as recommended. Built 2026-11-21: AC1–AC4 met; AC5 ✅ (CI green on `73ee9d0`); AC6 ✅ (user, 2026-11-21: *"All the live tests pass."*). **All ACs met — awaiting close approval.** Planned 2026-11-21 (user: *"Then you can begin planning SP-231."*).
**Carries:** [T-0497] (D1), [T-0693] (D2, D3).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **D** (D1–D3), next by the Epic's order ("E and D
first"). Part F, the toolbar ([SP-232]), follows it: its View item carries Show World Warnings, which this Sprint builds.
**Size:** M — three surfaces, each with an Apple original to copy; the core endpoints are already bridged.

---

## What reading the code found (2026-11-21)

**D1 — create a world.**
- **Mac** (`WorldsView.swift:92–100, 291–341`): a "New world name" field and **Create…** in the Worlds sheet. The Mac asks where with
  a save panel (`<name>.inkwynworld`); the iPad and iPhone pick a folder and the world is made inside it ([SP-225] Q1). Then
  `createWorld(projectRootPath:packagePath:displayName:epochLabel: "")`, and the whole app refreshes ([I-0130]).
- **Linux:** the Worlds dialog has Add Existing World… only (`WorldsDialog.cpp:170`). `ScriviBridge::createWorld` already exists
  (`ScriviBridge.cpp:1288`) and has no caller. Linux New Project asks for a name, then a folder (`ScriviWindow.cpp:225–237`).
- The Mac's bookmark step (`WorldBookmarkStore`) is a sandbox grant; Linux has none to keep ([T-0497]'s note).

**D2 — Reconnect Worlds.**
- **Mac** (`WorldsView.swift:124–128`): a **Reconnect Worlds** button in the Worlds sheet — *"Re-check worlds that were unavailable —
  use after reconnecting a drive."* It runs `AppEnvironment.reconnectWorlds()` (`:586`): for every open project, re-read each
  world's status and the pending links off the main thread, refresh the warning strip, and tell the Inspector to re-read. The
  same runs by itself when the app comes to the front.
- **Linux:** no button, and nothing re-checks on coming to the front. The dialog's own actions do refresh the Inspector
  (`EditorShell.cpp:3961`, the I-0130 wiring).

**D3 — the world-warning strip.**
- **Mac** (`WorldWarningView.swift`, placed at `EditorView.swift:434`): an amber, translucent strip laid over the bottom edge of the
  manuscript, just above the Timeline. "1 world is unavailable" / "N worlds are unavailable", one line per world (its name, why it is away, and how
  many links are held pending), **Manage Worlds…**, and × to hide it until another world goes away.
  Nothing destructive in it. View ▸ Show World Warnings (on by default, not saved) hides it altogether. Read off the main thread:
  `listWorlds`, then `listPendingEdges` only when a world is away ([I-0330]).
- **Linux:** no strip; only each object in the Inspector says its world is away (`SceneInspector.cpp:1100, 1229`).
  `ScriviBridge::listPendingEdges` exists (`:1674`). Linux's red "Project File Not Available" line is the project's drive, not a
  world's (`EditorShell.cpp:1835`).
- ⚠️ **The Epic's D3 also says a rejected imported timeline is reported in this strip. The Mac does not do that:** it reports it in
  the Timeline panel's own banner (`TimelineStripView.swift:950`, "1 imported timeline could not be loaded", file and reason).
  Linux only logs it (`EditorShell.cpp:4707–4729`, [I-0214]). See Q4.

## Goal

On Linux a writer with no world can create one; Reconnect Worlds picks up a world whose drive came back; and when a world is away,
a strip says so, as on the Mac.

## Plan

1. **D1:** a name field and **Create…** in the Worlds dialog; then the folder (Q1); `createWorld` → `<folder>/<name>.inkwynworld`,
   bound to the project; the dialog's list, the strip and the Inspector refresh. An error is shown in the dialog, as Add Existing's.
2. **D2:** **Reconnect Worlds** in the Worlds dialog, with the Mac's help text: re-reads every open project's worlds off the main
   thread and refreshes its strip and Inspector. The same when the app comes to the front (Q2).
3. **D3:** the strip over the bottom of the manuscript (Q3), with the Mac's wording, Manage Worlds… and ×; View ▸ Show World
   Warnings. Read on a worker (`WorkerBridge`, as the Worlds dialog reads).
4. **Rejected imported timelines** in the Timeline panel's own banner, with file and reason (Q4).
5. **Tests:** a new `world_warning_smoke` (in CI): create a world through the dialog; move its folder away → the strip with the
   world's name and its pending count; move it back + Reconnect → the strip gone, the Inspector's object back; × hides it until a
   second world goes away; the View toggle; a broken imported timeline → the panel's banner.
6. **Live pass** on the Linux VM (a world moved away with `mv` stands in for an unplugged drive).

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **How Create… asks where.** | **The name in the dialog, then a folder chooser; the world is made inside it** — Linux New Project's shape and the iPad's ([SP-225] Q1). A save dialog would let the writer edit the name a second time and drop the extension. | ✅ As recommended (2026-11-21) |
| **Q2** | **When worlds are re-checked.** | **The button, and also when the app comes to the front**, as the Mac (coalesced, off the main thread). Plugging a drive back in and switching back to Inkwyn then needs no click. | ✅ As recommended (2026-11-21) |
| **Q3** | **Where the strip sits.** | **Laid over the bottom edge of the manuscript, above the Timeline, as the Mac** (user, 2026-09-30, for the Mac: *"make it translucent and ZStack it"*) — so its appearing moves nothing. Show World Warnings on by default and not saved, as the Mac. | ✅ As recommended (2026-11-21) |
| **Q4** | **Rejected imported timelines** (the Epic's D3 puts them in the strip). | **In the Timeline panel's own banner, as the Mac**, not the world strip, and correct D3's text to say so. A broken timeline file is not a missing world. | ✅ As recommended (2026-11-21) |

## What was built (2026-11-21)

- **D1** — the Worlds dialog's "New world name" field and **Create…** (enabled by a name; Return also creates). Create… asks for
  the folder (Q1) and `WorldsDialog::createWorldIn` makes `<folder>/<name>.inkwynworld` through `ScriviBridge::createWorld`, which
  also binds it (`WorldStore::createWorld`: package and binding in one operation). Something of that name already there is refused
  in the dialog; a folder inside a package resolves to the package's own folder. The list, the strip and the Inspector refresh.
- **D2** — **Reconnect Worlds** in the dialog (the Mac's help text). It emits `reconnectRequested`; the shell passes it to its window,
  which runs `AppEnvironment::reconnectWorlds` — every open project's `EditorShell::refreshWorlds`. The same when the app comes back
  to the front (`applicationStateChanged` → Active, `main.cpp`; Q2). `refreshWorlds` reads `listWorlds`, and `listPendingEdges` only
  when a world is away ([I-0330]), on a worker (`WorkerBridge`, `AsyncCall`), one read at a time (a burst becomes one more read);
  then the strip and `inspector_->reload()` ([I-0128]). After every load, and after any change in the Worlds dialog, too.
- **D3** — `WorldWarningStrip` (new): the Mac's headline, one line per world ("“Eskandar” is unavailable. 1 link is held pending." — the status word is the core's),
  Manage Worlds…, and × (until another world goes away). Laid over the bottom edge of the manuscript column, placing itself on
  each resize (Q3); never takes focus. View ▸ Show World Warnings (after Show Timeline, as on the Mac; on by default, not saved).
- **Q4** — the Timeline panel's banner along its top: "1 imported timeline could not be loaded", each file and the core's reason
  (`TimelinePanel::setRejectedImports`, fed by `reloadImportedTimelines`; still logged). The Epic's D3 text is corrected.
- ⚠️ T-0497's record says the Mac *prompts* for a world when there is none: that prompt is in the Mac's New Object form
  (`ObjectCard.swift:836`, "No world yet — add one from the Worlds menu first."), which Linux gets with object creation (EP-059 A5).

**Tests (AC5, 2026-11-21):** `world_warning_smoke` (new, in CI): Create… through Project ▸ Manage Worlds…, a duplicate refused, a
world moved away (the strip, its name and pending link, along the bottom edge), × until a second world goes, the View toggle,
both moved back + the dialog's Reconnect Worlds (the strip gone), and a broken imported timeline named in the banner. Linux
(Docker, Qt 6.4) ctest **884/884** as non-root, **45/45** CI smokes, every target built. No core or Apple code changed. ✅ CI green on `73ee9d0`
(Linux App CI run 44947811319, `world-warning-ok`).

## Live pass (AC6) — the Linux VM

A world moved with `mv` stands in for an unplugged drive. Have a project open with nothing else in Inkwyn.

**A. Create a world**
1. Project ▸ Manage Worlds…: a "New world name" field with Create… beside it, greyed until you type a name.
2. Type a name and click Create…: a folder chooser asks where to put it. Choose a folder in your home folder.
3. The world appears in the list, available. In a terminal, the folder you chose holds `<name>.inkwynworld`.
4. Type the same name again and Create… in the same folder: the dialog says something of that name is already there.

**B. The warning strip**
5. Close the dialog. In a terminal, move the world away: `mv ~/<folder>/<name>.inkwynworld /tmp/`
6. Switch to the terminal and back to Inkwyn: an amber strip along the bottom of the manuscript, above the Timeline, says
   "1 world is unavailable" and names it. Nothing else moves.
7. View ▸ Show World Warnings: uncheck it (the strip goes), then check it again (it returns).
8. Click Manage Worlds… in the strip: the Worlds dialog opens, the world marked unavailable. Close it.
9. Click × on the strip: it goes, and switching to the terminal and back does not bring it back.

**C. Reconnect**
10. Move the world back: `mv /tmp/<name>.inkwynworld ~/<folder>/`
11. Project ▸ Manage Worlds…, then Reconnect Worlds: the world reads available.
12. Close the dialog and check View ▸ Show World Warnings is on: no strip (no world is away).

## Acceptance Criteria

- [x] AC1 — (D1) A world is created from the Worlds dialog, named by the writer, where she chooses; it is bound to the project and
      listed at once; objects can be added to it.
- [x] AC2 — (D2) Reconnect Worlds re-reads every open project's worlds; a world whose folder came back shows as available, the strip
      clears and the Inspector's objects return, without a scene change. The same on coming back to the app (Q2).
- [x] AC3 — (D3) When a world is away, the strip names it and the links held pending, with Manage Worlds… and ×; × hides it until
      another world goes away; View ▸ Show World Warnings hides it; nothing in it deletes.
- [x] AC4 — A rejected imported timeline is reported with its file and reason (Q4), not only logged.
- [x] AC5 — Linux ctest (non-root, tests on), the smokes (the new one in CI), CI green.
- [x] AC6 — Live pass on the Linux VM (user, 2026-11-21: *"All the live tests pass."*).

## Progress log

- **2026-11-21** — ✅ **CLOSED** (user, 2026-11-21: *"Close SP-231 and plan SP-232"*); T-0497, T-0693 archived.
- **2026-11-21** — ✅ AC6 live pass (user, 2026-11-21: *"All the live tests pass."*); CI green; T-0497, T-0693 → ✅ Verified. Awaiting close approval.
- **2026-11-21** — Built: AC1–AC4 met; AC5 green locally; T-0497, T-0693 → 🟠.
- **2026-11-21** — 🟡 **ACTIVE** (user, 2026-11-21: *"Q1-Q4 as recommended, implement SP-231"*); Q1–Q4 as recommended; T-0497, T-0693 → active.
- **2026-11-21** — 🔵 **Created in Planning** (user, 2026-11-21: *"Then you can begin planning SP-231."*), issued via `next-id.py`;
  carries [T-0497] and [T-0693]. Q1–Q4 to rule.

*Last Updated: 2026-11-21 (✅ **CLOSED**.)*

*Last Updated: 2026-11-21 (✅ All ACs met — live pass, CI green; awaiting close approval.)*

*Last Updated: 2026-11-21 (Built; ⏳ AC6 live pass, CI.)*

*Last Updated: 2026-11-21 (🟡 **ACTIVE**.)*

*Last Updated: 2026-11-21 (🔵 **Created in Planning**.)*
