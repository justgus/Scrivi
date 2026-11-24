# Sprint SP-239 — `[Apple]` [EP-059] A8: Delete and Change Kind for objects on the Mac and iOS

**Status:** ✅ **CLOSED** 2026-11-23 (user, 2026-11-23: *"you may close SP-239"*). Was 🟡 **ACTIVE** 2026-11-23 (user, 2026-11-23: *"Q1-Q5 as recommended, implement SP-239"*); ✅ Q1–Q5 as recommended. Built 2026-11-23: AC1–AC4 met; AC5 green locally (CI on the
push). ⏳ AC6 live pass. Created 2026-11-23 (user, 2026-11-23: *"plan A8"*).
**Carries:** [T-0702] (A8).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **A**: A8. ⛔ [SP-240] (A9, Linux) is blocked by this Sprint
(user, 2026-11-21: *"The Linux sprint should block on the MacOS/iOS sprint's completion."*).
**Size:** S–M — both endpoints exist and are bridged in Swift with no caller; one menu, one confirmation, one core guard.

---

## Findings (2026-11-23)

1. **Both endpoints exist, and nothing calls them.** `scrivi_delete_object` and `scrivi_promote_object` are wrapped
   (`ScriviEngine.deleteObject`, `ScriviEngineGraph.swift` `promoteObject`) with no caller in the app, on the Mac or iOS.
2. **Delete** (`ObjectStore::remove`) removes the object's file and index entry, then prunes **this project's** relationships
   to it in the same call, its scene links ("appears in") among them. It is refused while the world is locked or away.
   ⚠️ An object of a shared world is removed from the **world**: every project using that world loses it, and their
   relationships to it are left dangling until each project's load-time repair drops them. There is **no undo**: object
   editing is outside EP-019's history (the D3-C ruling).
3. **Images stay.** Delete leaves the object's image in the asset store. That is right: an asset can be shared (Choose
   Existing…, `ExistingAssetPicker.swift`). Not changed here.
4. **Promote is item ↔ artifact** (Object Model v0.2 §3.1). Since §3.0 both kinds are world-scoped, so it now changes only the
   **kind**: the objectID and every relationship are kept, and nothing moves between worlds. §3.1 left open whether promotion
   still earns its keep. The A8 ruling (2026-11-21) says to build it.
5. ⚠️ **The core accepts ANY kind pair.** `ObjectStore::promote` refuses only the same kind and a missing world, so
   `character → map` or `source → item` would go through, and the object's kind-specific fields would no longer match its
   kind. The app has never asked, because nothing called it.
6. **The detail sheet** (`ObjectDetailSheet.swift`, shared by the Mac and iOS) has a toolbar of Save, Undo Changes and Close
   (user rulings 2026-09-23), and a Back stack for related objects ([SP-236]'s shape). The card's context menu has View
   Detail and Remove from Scene.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where they live.** | **The detail sheet only**, in a new **More** menu (`ellipsis.circle`) in its toolbar, after Undo Changes: **Change to Artifact** (on an item) or **Change to Item** (on an artifact), then **Delete <Kind>…**. Not on the card: there, Delete would sit beside Remove from Scene and be easy to mistake for it. Greyed, with the reason, while the world is away (the read-only banner's rule). || ✅ As recommended (2026-11-23) |
| **Q2** | **Delete's confirmation and what follows.** | An alert **"Delete “Myton”?"** with **Delete** (destructive) and **Cancel**. For a world object it says: *"Myton will be removed from the world Eskandar and from every project that uses it, with its relationships in this project. This can't be undone."* For a source: *"… removed from this project, with its citations. …"* Unsaved edits are discarded with it. Afterwards the sheet goes **Back** if there is a previous object, or closes, and the cards refresh. || ✅ As recommended (2026-11-23) |
| **Q3** | **Change to Artifact / Item.** | **No confirmation** (it can be reversed the same way), **in the object's own world** (no world choice: nothing moves). **Greyed while there are unsaved changes** ("Save your changes first"), so an edit is never saved under the old kind. Afterwards the sheet stays on the object under its new kind, and the cards refresh (it moves from Items to Artifacts). || ✅ As recommended (2026-11-23) |
| **Q4** | **The any-pair promote** (finding 5). | **Refuse it in the core**: only item ↔ artifact, with a message, and C ABI tests through `scrivi_*` (as [SP-237] Q6). No migration: nothing ever called it ([project_no_testers_yet]). || ✅ As recommended (2026-11-23) |
| **Q5** | **Linux.** | **[SP-240]**, blocked by this Sprint, adopts the shape built here (as A9 says). || ✅ As recommended (2026-11-23) |

## Plan

- Core: `ObjectStore::promote` refuses any pair but item ↔ artifact (`detail: "notPromotable"`); C ABI tests for the refusal and
  for delete pruning the project's edges.
- Swift: the More menu in `ObjectDetailSheet`'s toolbar; the delete alert; after delete, Back or close; after a change of kind,
  re-read the object under its new kind; the inspector's cards refreshed both times.
- Interop test: delete and change of kind through the engine.
- Live pass on the Mac (an existing project, "test …" objects made for it) and on iOS.

## What was built (2026-11-23)

**Core** (Q4): `ObjectStore::promote` refuses any pair but item ↔ artifact (`detail: "notPromotable"`, "only an item and an
artifact can change into each other, not 'character' into 'map'"), before touching any file.

**Swift** (`ObjectDetailSheet.swift`, the Mac and iOS):
- **More** (`ellipsis.circle`) in the toolbar after Undo Changes: **Change to Artifact** / **Change to Item** on items and
  artifacts only, under "Save your changes first" and greyed while there are unsaved changes; **Delete <Kind>…**. The menu is
  greyed while the world is away or a save is running.
- The alert **"Delete “<name>”?"** (Delete, Cancel): a world object "will be removed from the world “…” and from every project that
  uses it, with its relationships in this project. This can't be undone."; a source "… from this project, with its citations and
  relationships …"; "Your unsaved changes are discarded." when there are some. After it: Back if there is a previous object,
  otherwise the sheet closes; `onDidSave` refreshes the cards.
- Change of kind: the trail's entry takes the new kind, the object is re-read under it, `onDidSave` moves it between cards.
- `ObjectDetailHistory.visit`: the same object under a new kind is a new entry (it compared objectIDs only, so reopening a
  changed object would have opened it under its old kind).

**Found and fixed on the way:** the interop runner could not launch its test host while Inkwyn was running (`Could not launch
“ScriviInteropTests”`). SP-229's bundle-ID change had replaced `com.caposoft.scrivi$(SCRIVI_TEST_HOST_ID_SUFFIX)` with
`"com.caposoft.inkwyn--INKWYN-TEST-HOST-ID-SUFFIX-"` plus a Mac-only `com.caposoft.inkwyn`, so the test host and the app shared
one bundle ID. Both settings (Debug and Release) are now `"com.caposoft.inkwyn$(SCRIVI_TEST_HOST_ID_SUFFIX)"`; the app still
builds as `com.caposoft.inkwyn` (the suffix is empty), the test host as `com.caposoft.inkwyn.testhost`.

## Tests

- C ABI (`ObjectCApiTests.cpp`): character → map, item → location and source → item refused with `notPromotable`, nothing changed;
  item → artifact → item still works. Delete's pruning was already tested (T-0377). ✅ A mutant without the guard fails it (in a
  copy of the repo).
- Interop (`ScriviInteropTests.swift`): an item changes to an artifact and back through the engine, keeping its relationship;
  character → map refused; delete removes it and its relationship.
- Results: macOS ctest **882/882**; interop **354/354** (run with Inkwyn open); iOS tests **38/38**; the macOS build. No Linux change.

## Live pass (AC6) — the Mac, then iOS

On `dumas-prose-timelines-history` (iCloud Drive ▸ Scrivi ▸ projects ▸ Scrivi-Dumas-Fixture), world Dumas-Characters-Places.

1. Mac: open the project. Inspector ▸ Worldbuilding, the Items card (+ ▸ Items if it is not there): New Item, `test key`, Create.
2. Double-click test key. After Undo Changes the toolbar has ⋯; it holds Change to Artifact and Delete Item….
3. Type `x` in Notes: ⋯ shows Change to Artifact greyed under "Save your changes first". Undo Changes.
4. ⋯ ▸ Change to Artifact: the view stays on test key; ⋯ now holds Change to Item and Delete Artifact…; the Items card drops it.
5. ⋯ ▸ Change to Item. ⋯ ▸ Delete Item…: "Delete “test key”?", naming Dumas-Characters-Places. Cancel. Again, Delete: the view
   closes; the Items card no longer lists it.
6. Open Abbé Faria's detail: ⋯ holds only Delete Character…. Close without deleting.
7. Quit Inkwyn. In Finder rename `Dumas-Characters-Places.inkwynworld` to `Dumas-away`; open the project and Abbé Faria's detail:
   ⋯ is greyed. Quit, rename it back.
8. iOS: open the project, New Item `test key 2`, open its detail (View Detail): ⋯ ▸ Change to Artifact, then ⋯ ▸ Delete
   Artifact… ▸ Delete: the view closes.

## Acceptance criteria

- [x] AC1 — Delete <Kind>… in the detail sheet, confirmed, on the Mac and iOS; the object, its relationships here and its scene
      links go; the sheet goes Back or closes; the cards refresh.
- [x] AC2 — Change to Artifact / Change to Item on items and artifacts only, keeping relationships; greyed while unsaved.
- [x] AC3 — Both greyed while the object's world is away.
- [x] AC4 — The core refuses a promote other than item ↔ artifact, tested through `scrivi_*`.
- [x] AC5 — ctest, the interop tests and the iOS tests green; CI green. ✅ 882/882, 354/354, 38/38; CI: Apple and Linux green on `2aae842` and `5ac4e35`; ScriviCore's macOS sanitizers job was never given a runner on `2aae842` (no steps ran) — re-run at close: ✅ green (all four jobs, macOS sanitizers 6 min 29 s).
- [x] AC6 — Live pass on the Mac and iOS. ✅ Mac, iPad and iPhone (user, 2026-11-23: *"Steps 3 through 11 also pass.  On all three platforms (mac, iPad, and iPhone)"*), after [I-0371].

## Retrospective

- The live pass found what no test could: on macOS the sheet's bottom bar shows ONE view per placement, so a `ToolbarItemGroup`
  (and then three separate items) dropped Undo Changes and ⋯ silently. Undo Changes had been missing since some OS change after
  SP-137, unnoticed. Fix: one item holding an `HStack` ([I-0371]).
- My first diagnosis assumed a top toolbar; the user corrected it (a bottom button bar). Two guesses before the fix — next time ask
  for a screenshot after the first miss.
- The interop test host's bundle ID (SP-229 regression) was found by running the tests with Inkwyn open.

## Progress log

- **2026-11-23** — ✅ **CLOSED** (user, 2026-11-23: *"you may close SP-239"*). T-0702 and I-0371 archived. [SP-240] unblocked.
- **2026-11-23** — ✅ AC6: live pass steps 2–11 pass on the Mac, iPad and iPhone (user, 2026-11-23: *"Steps 3 through 11 also pass.  On all three platforms (mac, iPad, and iPhone)"*); I-0371 and T-0702 → ✅ Verified. ⏳ AC5 CI on the next push; close is the user's.
- **2026-11-23** — Live pass, Mac step 2: only Save and Close showed. Filed [I-0371] ; separate items failed the same way (user: "Only disk and X"); now one item holding an HStack; macOS build green. ⏳ re-run from step 2.
- **2026-11-23** — Built: AC1–AC4 met; AC5 green locally; T-0702 → 🟠. Fixed the interop test host's bundle ID (SP-229 regression). ⏳ AC6 live pass, CI.
- **2026-11-23** — 🟡 **ACTIVE** (user, 2026-11-23: *"Q1-Q5 as recommended, implement SP-239"*); Q1–Q5 as recommended; T-0702 → active.
- **2026-11-23** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-23: *"plan A8"*); carries [T-0702] (new). Q1–Q5 to rule.

*Last Updated: 2026-11-23 (✅ Closed.)*
