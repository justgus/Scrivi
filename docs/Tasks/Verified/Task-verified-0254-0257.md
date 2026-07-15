# Verified Tasks — T-0254–T-0257 (SP-066, EP-023 [Linux] Manuscript Structure Editing — rename)

_Archived 2026-07-15 on SP-066 close. All four tasks ✅ Verified over Docker+VNC. Original active-task detail
preserved below verbatim._

**Sprint SP-066** — `[Linux]` Rename scene/chapter (second of 4 EP-023 sprints): the **rename** structure op —
navigator context menu, `QInputDialog` edit, the sidecar-title round-trip, the live navigator label + in-document
chapter heading update, and the blank-title fallback chain. Also **closed I-0062** (live new-chapter ordinal
heading). Epic: EP-023 `[Linux]`. **No ScriviCore work** — `scrivi_rename_scene`/`_chapter` already exist
(`scrivi.h:211/216`) and `open_project` carries `chapterMetadataPath`; pure `platforms/linux/` bridge +
`QTreeView`/viewport UI. Delivered **EP-023 AC3**.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0254 | Rename bridge wrappers + `chapterMetadataPath` capture — `ScriviBridge::renameScene`/`renameChapter` (`scrivi_rename_scene`/`_chapter`, `parseEnvelope`, RAII `scrivi_free`, `errorOccurred` on error); added `chapterMetadataPath` to `SceneDocument::Input`/`SceneSegment` (from the `open_project` payload). No identity args. | ✅ Verified |
| T-0255 | Context-menu Rename + inline edit + live label/heading — `EditorShell::onNavigatorContextMenu` gained **Rename Scene…/Rename Chapter…** → `QInputDialog` pre-filled with the current title; commit → bridge writes sidecar title, `applyDerivedLabels` re-derives labels, navigator + live in-document chapter heading update via `reflowBoundaryAt`. Blank/whitespace → cleared sidecar + fallback (scene: first prose line/"Untitled"; chapter: derived ordinal). | ✅ Verified |
| T-0256 | Close I-0062 — live new-chapter heading — `SceneDocument::chapterHeadingText` derives the ordinal "Chapter N" from segment order (custom title wins), matching macOS `ManuscriptTextView`; `insertSceneAfter` reflows a new untitled chapter's heading to its derived ordinal on splice, so it shows "Chapter N" immediately. (Superseded the planned Option-B fetch — order-based derivation is the macOS mechanism.) **I-0062 ✅ Resolved-Verified.** | ✅ Verified |
| T-0257 | Verify AC3 + rename headless smoke — new `scene_rename_smoke` (`.cpp`+`.sh`, CMake target, CI step): Cases A–C (scene rename no-doc-change, chapter rename live-heading reflow + offset delta, blank→derived ordinal); `scene_create_smoke` gained the I-0062 "Chapter 2" derived-heading assertion. Docker build + all 9 headless smokes green; VNC walk-through passed. `scrivi.h` untouched. | ✅ Verified |

## Verification

**AC3 + I-0062 user-verified over Docker+VNC 2026-07-15.** Right-click → Rename → `QInputDialog` → commit
updates the navigator label live; chapter rename also updates the live in-document heading; blank chapter →
app-derived ordinal "Chapter N" (from segment order, macOS parity); blank scene → first-line/"Untitled"; cancel
is a no-op; renames persist across quit→reopen; a newly-created chapter shows its ordinal immediately (I-0062).
Docker build + nine headless smokes green (adds `scene_rename`; `scene_create` also asserts the I-0062 derived
heading); ScriviCore `ctest` + macOS/EP-020–022 unaffected; `scrivi.h` unchanged.

**Follow-ups surfaced (not SP-066 scope):** I-0064 (Ctrl+Shift+Return chapter-split → SP-067), I-0063
(renumber created chapters on delete → backlog/SP-067).

---

*Archived 2026-07-15 on SP-066 close (user-approved). See `Sprints/Closed/Sprint-SP-066.md`.*
