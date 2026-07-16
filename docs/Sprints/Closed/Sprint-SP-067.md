# SP-067: [Linux] Scene drag-reorder + I-0064 chapter-split + I-0063 renumber

**Status:** ✅ **CLOSED (2026-07-16, Human-approved).**

**Close summary:** SP-067 **delivered EP-023 AC4 (scene drag-reorder)** and **closed I-0063** (renumber created
chapters), both Verified over VNC on 2026-07-16. Its two other originally-scoped pieces — the **I-0064
Ctrl+Shift+Return chapter-split** and its residual defects **I-0069/I-0070** — were **re-homed to the new EP-027**
(`[ScriviCore]` Filesystem-Authoritative Chapter/Scene Identity & Ordering) after VNC diagnosis traced the split
failures to a `[ScriviCore]` slug-collision corruption (**I-0072**) that EP-027 fixes at the root. Rather than
fix the split path twice (once on the old on-disk model, once on EP-027's new one), the split/renumber work moves
to EP-027. SP-067 thus closes having delivered the drag-reorder deliverable + I-0063.

**Delivered & Verified (2026-07-16, VNC on a fresh project):**
- **AC4 scene drag-reorder** — drag a scene within its chapter or across a chapter boundary; insertion-line drop
  highlight; `scrivi_reorder_scene` + `SceneDocument::moveScene` re-splice; order persists across quit→reopen.
- **I-0067 / I-0068** (found in the first VNC pass, fixed + verified): the scene vanished on drop and the reorder
  didn't persist because `NavigatorTree` ran the drag as `Qt::MoveAction`, so `QAbstractItemView::startDrag`
  auto-removed the source row regardless of the backend result. **Fix:** force `Qt::CopyAction` throughout
  (ctor `setDefaultDropAction`, `startDrag`, `dragMoveEvent`/`dropEvent`), so Qt never mutates the model; only
  our `rebuildNavigator` does. User confirmed "It's clean."
- **I-0063** renumber created ("Chapter N") chapters on delete (T-0262) — Verified (delete-renumber correct;
  custom titles untouched).

**Re-homed to EP-027 (2026-07-16 Human decision):**
- **I-0064** (chapter-split at caret), **I-0069** (end-of-scene-with-followers → no visible chapter),
  **I-0070** (end-of-scene-no-followers → appends at manuscript end), **I-0072** (root `chapter-<count+1>` slug
  collision), and **I-0071** (last-scene drag orphans an empty chapter → EP-027 scenes phase).
- The mid-scene split (VNC case 3b) *did* work on the old model, but the whole split path is rebuilt on EP-027's
  filesystem-authoritative model, so T-0261 is superseded there rather than completed here.

**Follow-on flagged:** **I-0073** — a ~1–2 s drag-hover lag observed over Docker+VNC; the per-move code path is
cheap (no I/O), so it's most likely the VNC/Xvfb pipeline. Needs a real-hardware datapoint before any action.

**Tasks at close:**
- T-0258 (reorder bridge wrappers) — ✅ Done (used by AC4; verified via the drag path).
- T-0259 (`SceneDocument::moveScene` re-splice) — ✅ Done (drives AC4; smoke + VNC verified).
- T-0260 (`NavigatorTree` scene drag-drop) — ✅ **Verified** (AC4; I-0067/I-0068 CopyAction fix).
- T-0261 (I-0064 chapter-split) — 🔵 **Superseded → EP-027** (mid-scene worked; end-of-scene failed; rebuilt on
  the new model).
- T-0262 (I-0063 renumber) — ✅ **Verified**.
- T-0263 (`scene_reorder_smoke` + verify) — ✅ Done (smoke green; VNC verification completed for AC4/I-0063; the
  split cases move with I-0064 to EP-027).

**Activated:** 2026-07-15 · **Closed:** 2026-07-16 · **Epic:** EP-023 `[Linux]` (third of 4 sprints).
`scrivi.h` untouched (AC8). No ScriviCore source change in this sprint.

---

## Original sprint record (as activated 2026-07-15)

**Goal:** Deliver **scene drag-and-drop reorder** in the Linux `QTreeView` navigator (EP-023 **AC4**) — drag a
scene row within its chapter or across a chapter boundary, with an insertion-line drop highlight, calling
`scrivi_reorder_scene` and surgically re-splicing the continuous viewport + `SceneDocument` map to the new
manuscript order (no full rebuild — caret/scroll preserved). In the same sprint, fix the two structure defects
that share this orchestration: **I-0064** (Ctrl+Shift+Return must **split the chapter at the caret**) and
**I-0063** (deleting/inserting a chapter must renumber later **created** stored-`"Chapter N"` chapters).

### Design decisions (locked at planning 2026-07-15)

- **Scope = all three, one sprint.** AC4 scene drag-reorder + I-0064 chapter-split + I-0063 renumber shipped
  together because they shared the `reorder_scene`/`reorder_chapter` orchestration and the `SceneDocument`
  splice/reflow primitives. *(Split/renumber later re-homed to EP-027 — see close summary.)*
- **Drag mechanics = app-mediated, not model auto-move.** The navigator is a `QStandardItemModel` projection;
  the drop is intercepted and resolved to `(sceneID, sourceChapterID, targetChapterID, afterSceneID)`, the bridge
  is called, then the document + map re-splice and the navigator rebuilds from authoritative segments. *(The
  I-0067/I-0068 fix made this a CopyAction drag so Qt never auto-removes rows.)*
- **Drop highlight = insertion line** (`setDropIndicatorShown(true)` + `dragMoveEvent` gating; illegal drops
  rejected).
- **Reorder re-splice = move, not rebuild** (`SceneDocument::moveScene` lifts the body span + one boundary and
  re-inserts with the correct heading/separator; `reflowAllChapterHeadings()` renumbers untitled chapters).
- **I-0064 chapter-split orchestration (app layer)** and **I-0063 renumber = Option A (app-side, `^Chapter \d+$`
  pattern)** — *split path re-homed to EP-027; I-0063 delivered.*
- **No ScriviCore work; `scrivi.h` untouched (AC8).**
- **Splice discipline (inherited from SP-062/065).**

### Exit criteria (as written at activation)

- **AC4** — ✅ met & Verified (2026-07-16).
- **I-0064** — ↦ EP-027 (rebuilt on the new model).
- **I-0063** — ✅ met & Verified (2026-07-16).
- **Verification** — `scene_reorder_smoke` + all prior smokes green; AC4 + I-0063 VNC-verified; `scrivi.h`
  unchanged; ScriviCore `ctest` unaffected; macOS + EP-020–022 + SP-065/066 flows unbroken.

### Issues in scope (final disposition)

| ID | Title | Disposition |
| -- | ----- | ----------- |
| I-0063 | Renumber later created ("Chapter N") chapters | ✅ Resolved-Verified (T-0262) |
| I-0064 | Ctrl+Shift+Return split at the caret | 🔵 Re-homed → EP-027 |
| I-0067 | Scene drag doesn't persist | ✅ Resolved-Verified (CopyAction fix) |
| I-0068 | Scene vanishes on drop | ✅ Resolved-Verified (CopyAction fix) |
| I-0069 | End-of-scene-with-followers split shows no new chapter | 🔵 Re-homed → EP-027 |
| I-0070 | End-of-scene-no-followers split appends at end | 🔵 Re-homed → EP-027 |
| I-0071 | Last-scene drag orphans an empty chapter | 🔵 Re-homed → EP-027 (scenes phase) |
| I-0072 | `chapter-<count+1>` slug collision corrupts the index | 🔵 Root defect of EP-027 |
| I-0073 | ~1–2 s drag-hover lag (VNC) | 🔵 Open — triage (likely environmental) |
