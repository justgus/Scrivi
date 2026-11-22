# Sprint SP-234 — `[Linux]` [EP-059] A: the object verbs on a card (A3–A6)

**Status:** 🟡 **ACTIVE** 2026-11-22 (user, 2026-11-22: *"Q1-Q5 as recommended, implement SP-234"*); ✅ Q1–Q5 as recommended. Built 2026-11-22: AC1–AC6 met; AC7 green locally (CI on the push). ⏳ AC8 live pass. Created 2026-11-22 (user, 2026-11-22: *"close SP-233.  Lets proceed to planning A3-A6."*).
**Carries:** [T-0696] (A3–A6).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — part **A**, second Sprint, on [SP-233]'s card stacks.
**Size:** M–L — every endpoint is bridged; none has a Linux caller yet (`createObject`, `createEdge`, `deleteEdge`, `saveObject`
are declared in `ScriviBridge.hpp` and called nowhere in `platforms/linux/src`).

---

## What reading the code found (2026-11-22)

**The Mac's object card** (`Scrivi/Views/Inspector/ObjectCard.swift`, `ObjectPickerView.swift`; iOS shares both):
- **Under the rows, two buttons:** **Add <Kind>…** (`:572`) and **New <Singular>** (`:607`, disabled while identity starts up).
- **Add <Kind>…** opens a picker (`ObjectPickerView`):
  - A search field, "Search all worlds…".
  - **Every object of the kind in the project, across every world**, grouped by world, with the project's own objects first.
  - Objects already in the scene are ticked and disabled.
  - Empty: "No <kind> in this project yet." / "No match.". At the foot: **Create new…**, which starts a draft holding what was typed.
  - Picking an object links it to the scene, one edge (`createEdge`).
  - The relation type comes from the project's **live vocabulary**: the kind's preferred code if it accepts the kind, else the first
    type that accepts the kind and targets a scene.
- **New** opens an **inline draft in the card**, never a dialog (§4.6). It has:
  - a Name field;
  - for a world-scoped kind, a World menu, defaulting to the first available world, or "No world yet — add one from the Worlds menu
    first.";
  - **Create** and **Cancel** / **Discard**.
  - Create runs `createObject` with `worldID`, then `createEdge`. Nothing is written before Create.
- **A row:**
  - A double-click opens View Detail (B1; Linux shows its "not built yet" message).
  - A **single click opens the same inline editor for a rename** (`rename`: open the object, patch `displayName`, `saveObject`;
    never reconstructed). The World is shown, not editable: "An object cannot be moved between worlds."
  - Context menu: **View Detail**, **Remove from Scene**.
  - A **⊖ button** per row: Remove from Scene, which deletes the edge only. The object stays; it is never spelled "Delete".
  - A pending row cannot be edited or removed. Its explanation is "Held pending — this object's world is …".
- **A draft left at a scene boundary** asks, in an alert (I-0119): **Create in Previous Scene** / **Save Changes**, **Keep Editing**,
  **Discard New …** / **Revert Changes**. It files the object against the scene the draft **started** in, and says so ("for: Scene 3").
- **Thumbnail:** a row whose object has an image shows it at 20 × 20 (`ObjectRowThumbnail`). The path is the core's `imagePath` from
  `listObjects`.
- ⚠️ **No project-wide object browser on the Mac.** The only project-wide list of objects is this picker; nothing else lists the
  project's objects to open them. EP-035's "project-wide list / browser" (A3, A4) has no Mac counterpart (Q1).
- ⚠️ **The Mac restates the kinds**: `ObjectCardKind.all` names all ten, each with its own `preferredRelationType` (location →
  `located-at`, the rest `appears-in`). The standing rule calls this a defect on sight (Q5).

**Linux** (`SceneInspector.cpp` after [SP-233]):
- A kind's card is a list of the scene's rows (`objectCardBody`): label, pending ⚠, and opening by double-click / Return / Open Object.
  There are no write verbs.
- `Entry` already carries `edgeID`, `worldID`, `pending`, `pendingStatus`. Every write endpoint is bridged. The writing-as author is
  adopted on load (`EditorShell.cpp:716`).
- A Linux writer can create a world since [SP-231] (T-0497).

**The core:** `appears-in` accepts any kind and targets a scene. `located-at` links a scene and a **location** (`targetKind`). So the
vocabulary itself says which type is specific to a kind.

## Plan

1. **Add <Kind>…** under each kind's card. It opens a picker as the Mac's (search, every world, grouped, already-linked ticked, Create
   new…), and a pick links with one edge.
2. **New <Singular>** opens an inline draft in the card: Name, plus World for a world-scoped kind. Create runs `createObject` then
   `createEdge`. Disabled and explained when no author is adopted yet.
3. **Rename** opens the same inline draft for an existing row (as Q2 rules). It opens the object, patches `displayName` and saves.
4. **Remove from Scene** is a ⊖ per row and an item in the row's context menu. It deletes the edge. A pending row disables both and
   explains why; a refusal is reported as pending, never as a failure.
5. **An unfinished draft at a scene boundary** asks as the Mac's alert does, and commits to the draft's origin scene (Q3).
6. **Thumbnails** (A6): a row shows the object's image at 20 × 20 when the core gives an `imagePath`, and nothing otherwise (Q4).
7. **The relation type** is resolved from the live vocabulary, derived (Q5).
8. **Tests** (AC6) and the **live pass** (AC7).

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **A3 and A4's "project-wide list / browser".** | **Match the Mac: the Add <Kind>… picker is the project-wide list** (every object of a kind, across worlds). Reword A3/A4 to it, and drop a separate browser, which the Mac does not have. If one is wanted, it is a Mac and iOS Sprint first, as you ruled for delete / promote. | ✅ As recommended (2026-11-22) |
| **Q2** | **How a rename is reached.** The Mac opens the editor on a **single click**. On Linux a single click selects the row, and Return opens it (SP-125). | **Rename… in the row's context menu, and F2**, which is the Linux convention. A single click only selects, so the keyboard path and Return-to-open still work. The editor is the Mac's, inline in the card. | ✅ As recommended (2026-11-22) |
| **Q3** | **A draft left at a scene boundary.** | **As the Mac** (I-0119): a dialog with Create in Previous Scene / Save Changes, Keep Editing, Discard / Revert. It commits to the scene the draft started in, and the draft names that scene ("for: Scene 3"). | ✅ As recommended (2026-11-22) |
| **Q4** | **A6 thumbnails, before B2 (image import).** | **Build it now.** The core already gives `imagePath`, and a Mac writer's images are on disk today. The row shows them, and an object without one is unchanged. Importing an image stays B2. | ✅ As recommended (2026-11-22) |
| **Q5** | **The relation type, derived or restated.** The Mac names each kind's type in a table. | **Derive it on Linux:** the type from the live vocabulary that names this kind (`sourceKind` / `targetKind`) with a scene at the other end; else one that accepts any kind and has a scene endpoint. This gives `located-at` for locations and `appears-in` for the rest, without naming a kind. **File an Issue for the Mac's table** under the standing rule, not fixed here. → [I-0367] | ✅ As recommended (2026-11-22) |

## What was built (2026-11-22)

All in `SceneInspector` (`platforms/linux/src/SceneInspector.cpp`, `.hpp`); no core or bridge change.
- **A kind's card** (`objectCardBody`) is now its rows, then the draft (when one is open), then **+ Add <Kind>…** and **New <Singular>**.
  New is disabled, and says why, until the bridge has an identity.
- **Add <Kind>…** (`openPicker`) is a small window as the Mac's popover. It reads `listObjects(kind)` on a worker; its other parts
  (search, grouping by world, the scene's objects ticked and disabled, Create “…”…) are as the Mac's. A pick writes one edge.
- **The draft** (`Draft`, `draftEditor`, `startDraft`, `startRename`) belongs to the panel, not to the card's widgets, so it survives
  the card being rebuilt on every read. It is the Mac's editor: Name; a World menu (new) or the world shown, not editable (rename);
  Create / Save, and Cancel / Discard / Done / Revert by whether anything would be lost. Return commits.
  - It stays until its write lands, so a failed Create keeps what was typed, as the Mac's.
- **Rename** (Q2): F2 on the selected row, and **Rename…** in the row's menu. It opens the object, patches `displayName`, and saves.
- **Remove from Scene**: a ⊖ on each row and an item in the row's menu. It deletes the edge only. Both are disabled on a pending row,
  and a refusal toward an absent world is reported as pending.
- **The boundary prompt** (Q3, `promptUnfinishedDraft`) is the Mac's alert. Create files the object against the draft's origin scene,
  and the editor names it ("for: Scene 1", "Create in Scene 1").
  - ⚠️ One difference from the Mac: an untouched draft (a New with no name, a rename with no change) closes without asking, as there
    is nothing to lose. The Mac asks even then.
- **The relation type** (Q5, `relationTypeToScene`) is derived from the vocabulary, on the worker, at the write. The Mac's table is
  [I-0367].
- **Thumbnails** (A6, Q4): the core's `imagePath` (it prefers the thumbnail), decoded at 20 × 20 on a worker, cached by path, and set
  on the rows already drawn.
- **Every write** (create, link, rename, remove) runs on a worker (`runObjectWrite`), then the scene is read again. The main thread
  only draws.
- **The live-pass fixture** (`inspector_fixture.cpp`) now gives its first world object an image, since Linux cannot import one
  until B2.

**Tests (AC7):** `inspector_objects_smoke` (new, in CI) runs through a real project window:
- §1 Add: grouped, ticked, searched, a pick linked by the derived type.
- §2 New: nothing written before Create; the world threaded; the general type, and a kind-specific one where the vocabulary has it.
- §3 F2 rename: Done → Revert; other fields kept.
- §4 ⊖: the edge only.
- §5 the boundary: an untouched draft closes; Keep Editing names the origin; Create files it there and not on screen; Discard writes
  nothing.
- §6 a row's image, and a row without one unchanged.

The smoke found one defect while being written: Create cleared the draft before its write landed, so a failure would have lost the
name. Fixed as above. Docker (Qt 6.4, non-root): ctest **884/884**, smokes **48/48**.

## Live pass (AC8) — the Linux VM

Build on the VM (`git pull`, then build as §9 of the rig doc). Then
`./build/platforms/linux/scrivi_linux_inspector_fixture ~/ObjectsPass.inkwyn`, open `~/ObjectsPass.inkwyn`, and show the Inspector.

1. Put the caret in the first scene and click the Worldbuilding tab. Click + ▸ Characters. Expected: the card lists character 1
   with a small teal square before it, and character 2 with none. Below the rows: + Add Characters… and New Character.
2. Click + Add Characters…. Expected: an "Add Characters" window with a "Search all worlds…" field, the line "Characters live in a
   world: Eskandar.", an Eskandar heading, character 1 ✓ and character 2 ✓ greyed out, and Create new… at the foot.
3. Type zz in the search. Expected: "No match.", and the button reads Create “zz”…. Click it. Expected: the window closes and the
   card shows a New form: Name "zz", a World menu on Eskandar, and Create and Discard.
4. Replace the name with Mara and click Create. Expected: the form closes and Mara is in the card.
5. Click Mara, then press F2. Expected: an Editing form with Name "Mara", "World  Eskandar" (not a menu), and Save and Done. Type
   Mara Vell in place of the name. Expected: Done becomes Revert. Click Save. Expected: the row reads Mara Vell.
6. Right-click character 2. Expected: Open Object, Rename…, Remove from Scene. Choose Remove from Scene. Expected: the row goes.
7. Click the ⊖ at the right of Mara Vell. Expected: the row goes.
8. Click + Add Characters…. Expected: character 2 and Mara Vell are offered (no ✓). Click character 2. Expected: it is back in the
   card.
9. Click + ▸ Locations, then New Location. Type Harbour and press Return. Expected: a row "Harbour — takes place at", where Mara
   read "— features".
10. Make a second scene (New Scene), put the caret in it, and click + ▸ Characters there. Click New Character and type Wren, but do
   not click Create. Put the caret back in the first scene. Expected: an "Unfinished character" dialog with Create in Previous
   Scene, Keep Editing and Discard New Character.
11. Click Keep Editing. Expected: the form is still there, with "for: Scene 2" at its top right and the button reading Create in
   Scene 2. Click it. Expected: Wren is not in the first scene's card. Move to the second scene. Expected: Wren is there.
12. In the second scene, click New Character, type Ghost, and move to the first scene. Choose Discard New Character. Expected: the
   form is gone, and + Add Characters… does not list Ghost.
13. Click New Character, type nothing, and move to the other scene. Expected: no dialog, and the form is gone.
14. Quit with Ctrl+Q and reopen the project. Expected: the first scene's Characters card has character 1 and character 2 and no Mara Vell;
   Harbour is under Locations; Wren is in the second scene.

## Acceptance Criteria

- [x] AC1 — (A5, A3, A4 as Q1) **Add <Kind>…** lists every object of the kind in the project, across worlds, grouped, searchable,
      with those already in the scene ticked. A pick links it to the scene; Create new… starts a draft.
- [x] AC2 — (A5) **New** creates an object of the kind in place, with `worldID` threaded (world-scoped: a world is required and
      offered; project-scoped: none), and links it to the scene the draft started in. Nothing is written before Create.
- [x] AC3 — (A5) **Rename** (as Q2) patches the object's `displayName` and keeps every other field; the world is shown, not editable.
- [x] AC4 — (A5) **Remove from Scene** deletes the edge only; the object survives and is offered again by Add. A pending row's verbs are
      disabled and explained.
- [x] AC5 — (A5, Q3) A draft left at a scene boundary asks; each answer does what it says, and the commit goes to the origin scene.
- [x] AC6 — (A6) A row shows its object's image when there is one.
- [~] AC7 — Tests: ✅ Docker (Qt 6.4, non-root): ctest **884/884**, smokes **48/48** (the new one in CI); ⏳ CI on the push.
- [ ] AC8 — Live pass on the Linux VM.

## Progress log

- **2026-11-22** — Built: AC1–AC6 met; AC7 green locally (884/884, 48/48); T-0696 → 🟠. ⏳ AC8 live pass, CI.
- **2026-11-22** — 🟡 **ACTIVE** (user, 2026-11-22: *"Q1-Q5 as recommended, implement SP-234"*); Q1–Q5 as recommended; T-0696 → active; [I-0367] filed (Q5).
- **2026-11-22** — 🔵 **Created in Planning** via `next-id.py`; carries [T-0696] (new). Q1–Q5 to rule.

*Last Updated: 2026-11-22 (Built; ⏳ AC8 live pass, CI.)*
