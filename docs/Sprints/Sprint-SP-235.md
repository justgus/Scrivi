# Sprint SP-235 — `[Linux]` [EP-059] B: the object detail view and its image (B1–B4)

**Status:** 🔵 **PLANNING** — created 2026-11-22 (user, 2026-11-22: *"close SP-234, then plan B1-B4"*). Q1–Q6 to rule.
**Carries:** [T-0697] (B1–B4).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — part **B**, on [SP-233]'s stacks and [SP-234]'s card verbs.
**Size:** L — one new surface. Every endpoint it needs is already bridged (`openObject`, `saveObject`, `listObjects`, `importAsset`,
`listAssets`, `removeAsset`, `listWorlds` in `ScriviBridge.hpp`); none of the asset ones has a Linux caller yet.

---

## What reading the code found (2026-11-22)

**The Mac's detail sheet** (`Scrivi/Views/Detail/ObjectDetailSheet.swift`, hosted at `Scrivi/Views/EditorView.swift:604`):
- **A real sheet over the project window** (`.sheet`, 520 × 520 minimum), opened by a double-click on a card row or **View Detail** in its
  context menu. Click-outside does not dismiss it (I-0245); Esc runs the same guarded close as ✕ (I-0250).
- **The fields:** Name, Subtitle ("A one-line descriptor"), Notes (a plain multi-line editor, D3-A), Tags (chips with ✕, and an "Add a
  tag…" field; a duplicate or blank tag is refused). Below them: the Image section, then Sources (C4) and Relationships (C1), then
  metadata — the kind, "World: <name>" as text (never a picker, I-0142), "Modified <date>".
- **The toolbar:** **Save** (⌘S, a disk icon) and **Undo Changes** (revert to disk, not undo history), both disabled until something
  changed; **Close** (✕) trailing-most. Words on tooltips.
- **Save keeps the sheet open** and says so: a bottom banner "Changes saved" / "Changes reverted", cleared at the next edit (I-0246).
- **Save re-reads before it writes** (I-0155): it opens the object as it is on disk, takes from disk every field the writer did not edit,
  patches the document (never rebuilds it — `image`, `attributes` and unknown keys survive) and saves. Tags are written as `[{"v": …}]`,
  and an empty list removes the key (`ObjectDetail.applyingEdits`).
- **A failed save keeps the drafts** and shows the error under the fields.
- **Closing with unsaved changes asks:** "Save your changes?" — Save · Discard · Cancel.
- **A re-read merges per field** (I-0165b): a field the writer has not touched takes disk's value, an edited one is kept.
- **Read-only (B3, R9):** an object whose world is not available is shown, never hidden. A banner says why (the core's explanation, or
  "“<world>” is <status>, so this object is read-only until it is available again."), every field is inert (Notes as plain text, tags
  without ✕), and Save cannot run. An object bound to a world the project cannot see is read-only too.
- **A world away at open:** the sheet names the object and says "“<world>” is <status>." / "Its details will appear when the world is
  available again." — never an error code (I-0166).
- **A world going away while open (B4):** the sheet re-reads on `worldRevision` (I-0162). A failed re-read **keeps what it has**
  (I-0165): the fields stay, turn read-only, and the image stops pointing at unreachable bytes. With unsaved edits it adds a second,
  orange banner: "Your unsaved changes are held here only. They will be saved when this object's world is available again — but they
  will be lost if Inkwyn quits first." (T-0450). When the world returns, the fields unlock and Save writes them.
- **The Image section (B2, `ObjectImageSection.swift`):** the image scaled to fit (max 220 high) and its file name; "No image."; or
  "This object's image is in a world that is unavailable.". Buttons: **Add Image…** / **Replace…** (a file chooser, images only),
  **Choose Existing…** (the world's images, `ExistingAssetPicker` → `listAssets`), **Remove**. Import is `importAsset` with the object's
  `worldID`, then a patch of `image.assetID`; Replace removes the old asset after the new one is attached; Remove drops the `image` key
  and then the asset. All disabled, with a reason, when read-only.
- **Navigation inside the sheet** (a related object pushed, with Back) belongs to the Relationships section, **C3**.

**Linux today:**
- A row's double-click and **Open Object** reach `EditorShell::onOpenObjectRequested` (`EditorShell.cpp:4463`), which opens the object
  and shows "The object detail view is not built yet — it is the next Epic's work." (`:4503`) — wrong now: it is this Epic's.
- [EP-059]'s B1 points at `EditorShell.cpp:4197`; the code is at `:4503`.
- World state reaches the window through `EditorShell::refreshWorlds()` (on activation, Reconnect Worlds and the Worlds dialog), which
  already reloads the Inspector on the way out (`:4045`, I-0128). There is no mount watcher; that is how Linux learns a world went away.
- The scene's Tags card on Linux is a **comma-separated line edit** (`SceneInspector.cpp:188`), where the Mac's is chips (Q3).

## Plan

1. **`ObjectDetailDialog`** (new, `platforms/linux/src/`): a window-modal dialog over the project window (Q1). It takes the bridge
   handle, the root, the entry (kind, objectID, worldID, name), the worlds and the project ID — nothing from its host, as the Mac's.
   Every read and write runs on a worker (`AsyncCall::run`, `WorkerBridge`).
2. **The fields** (B1): Name, Subtitle, Notes (`QPlainTextEdit`), Tags (Q3), the metadata lines. Save patches the document as the Mac's:
   re-read, per-field merge, `[{"v"}]` tags. Revert re-reads. The status line reports Saved / Reverted and clears on the next edit.
   The guarded close asks Save · Discard · Cancel, from ✕, Esc and the window's close button alike (Q2).
3. **The image** (B2): show, Add / Replace…, Choose Existing…, Remove, in the Mac's order and wording (Q5).
4. **Read-only** (B3): the banner, inert fields, Save off; the "world away at open" view.
5. **World going away** (B4): `refreshWorlds()` hands the new world list to an open dialog, which re-reads and keeps what it has; a save
   refused as world-unavailable keeps the drafts and turns the dialog read-only; the at-risk banner while edits are held.
6. **Wiring:** `onOpenObjectRequested` opens the dialog in place of the message; a save reloads the Inspector (I-0155). The row's
   **Open Object** is renamed **View Detail** (Q6).
7. **Tests** (AC6) and the **live pass** (AC7).

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **What the surface is.** The Mac's is a sheet over the window. | **A window-modal `QDialog` over the project window** (`open()`, 520 × 520 minimum, resizable) — Qt's equivalent of a sheet. It blocks the window behind it, as the Mac's does, so the Inspector cannot change the object underneath it. | |
| **Q2** | **Where Save, Undo Changes and Close go.** | **As the Mac:** a toolbar at the top of the dialog — Save (Ctrl+S) and Undo Changes beside Close (✕), using the desktop theme's standard icons (`document-save`, `edit-undo`, `window-close`) with the words on tooltips. Esc and the title bar's close both run the guarded close. | |
| **Q3** | **Tags: chips or a comma field.** The Mac uses chips with ✕; Linux's scene Tags card is a comma field. | **Chips, as the Mac** (a ✕ per tag and an "Add a tag…" field, duplicates and blanks refused): a comma field cannot hold a tag that contains a comma, and the Mac's shape is the rule. **File an Issue** for the scene Tags card's comma field, not changed here. | |
| **Q4** | **What the dialog holds in this Sprint.** The Mac's also has Sources (C4), Relationships (C1) and in-sheet navigation (C3). | **B1–B4 only:** fields, image, metadata. C1–C4 are the next Sprint and add their sections to this dialog. Nothing is shown for them yet — no placeholder. | |
| **Q5** | **Choose Existing… (B2).** EP-036 AC2 names import, show, replace, remove; the Mac also offers the world's existing images. | **Include it**, as the Mac — `listAssets` is bridged, and without it a writer re-imports an image the world already has. | |
| **Q6** | **The row's menu item.** Linux says **Open Object**; the Mac says **View Detail**. | **View Detail**, as the Mac. Double-click and Return still open it. | |

## Acceptance criteria

- [ ] AC1 — (B1) A double-click, Return or View Detail on a card row opens the detail view of that object; Name, Subtitle, Notes and Tags
      are shown and editable, with the kind, world (as text) and modified date.
- [ ] AC2 — (B1) Save writes the edits and keeps every other field (`image`, `attributes`, unknown keys); it merges per field with what
      is on disk; the dialog says Saved, Undo Changes says Reverted; a failed save keeps the drafts. Closing with unsaved changes asks.
- [ ] AC3 — (B2) An image is added, shown, replaced (the old asset removed), chosen from the world's existing images, and removed.
- [ ] AC4 — (B3) An object whose world is away opens read-only, says why, and is never written; one whose world is away at open is named
      and explained, never an error code.
- [ ] AC5 — (B4) A world going away while the dialog is open loses nothing: the fields stay, turn read-only, the at-risk banner shows
      while edits are held, and Save writes them once the world is back.
- [ ] AC6 — Tests: a smoke through a real project window (open, edit, save, revert, guarded close, tags, image add / replace / remove,
      read-only); ctest and every smoke green in Docker; CI green.
- [ ] AC7 — Live pass on the Linux VM.

## Progress log

- **2026-11-22** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-22: *"close SP-234, then plan B1-B4"*); carries [T-0697]
  (new). Q1–Q6 to rule.

*Last Updated: 2026-11-22 (🔵 Planning; Q1–Q6 to rule.)*
