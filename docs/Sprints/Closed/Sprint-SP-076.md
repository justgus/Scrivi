# SP-076 (Closed)

## SP-076: [Linux] Scene & chapter merge parity

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Epic:** EP-028: [Cross] Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix (3rd of 3 sprints — the final leg)
**Goal:** Bring the Linux app to parity with macOS on merging. A writer can:
- **Scene-merge** with `Ctrl-Backspace` at the **start of a scene** → the scene joins the previous scene.
- **Chapter-merge** with `Ctrl-Shift-Backspace` at the **start of a chapter's first scene** → the whole
  chapter merges into the previous chapter.

Both are backed by SP-074's atomic `scrivi_merge_scene` / `scrivi_merge_chapter` endpoints (already exported
in `libScriviCore.a`), so Linux gets the same filesystem-coherent, no-loss-on-reopen behavior macOS now has.
**No confirmation dialogs** — the keystroke is the approval (matching macOS, per the EP-028 scope decision).
No-op at the very start of the manuscript. History barriers (`sceneMerge` / `chapterMerge`) recorded.
**Start Date:** 2026-07-21
**End Date:** 2026-07-22
**Capacity:** ~5–7 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0304 | `[Linux]` `ManuscriptEditor::keyPressEvent` — detect `Ctrl-Backspace` (scene) / `Ctrl-Shift-Backspace` (chapter) at the start of a scene/chapter and emit `mergeSceneRequested` / `mergeChapterRequested` signals (swallow the key; no-op at manuscript start). | High | ✅ Verified (2026-07-22) — scene-merge live; chapter-merge via SP-077 menu |
| T-0305 | `[Linux]` `ScriviBridge::mergeScene` / `mergeChapter` `Q_INVOKABLE`s (peers to `reorderScene`/`reorderChapter`) calling the C ABI + decoding the envelopes; `SceneDocument` reload from disk (I-0081 stale-path contract). | High | ✅ Verified (2026-07-22) — both smokes green; scene-merge live |
| T-0306 | `[Linux]` `EditorShell` merge slots (`onMergeSceneRequested` / `onMergeChapterRequested`): resolve caret→segment, guard the no-op cases, call the bridge, reload from disk, re-anchor the caret at the join seam, `rebuildNavigator`. New `scene_merge_smoke`. | High | ✅ Verified (2026-07-22) — scene-merge live (incl. #1 no-recenter fix); chapter-merge live via SP-077 menu |
| T-0307 | Update `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md` — merge is now supported on both platforms (§8 clarified + new §8.1). | Medium | ✅ Implemented (doc) |
| T-0308 | `[Linux]` **Faint between-scene separator rule** — `ManuscriptEditor::paintEvent` overlay: 1px theme-aware (`QPalette::Mid`) hairline, inset 20px, centered in each within-chapter scene-separator gap; positions from new `SceneDocument::sceneSeparatorPositions()`. Purely visual. | Medium | ✅ Verified (2026-07-22) |
| T-0309 | `[Linux]` **`Ctrl+Return` mid-scene split (macOS `⌘↩` parity)** — split head/tail at the caret (was: always append empty). Save head→current + tail→new + reload; caret to tail. | High | ✅ Verified (2026-07-22, regression passed) |

### Assigned Issues

_(none — no new issues logged during SP-076.)_

### Sprint Notes

- **Backend already done (SP-074), macOS reference done (SP-075).** Both endpoints exported in
  `libScriviCore.a` (`_scrivi_merge_scene`, `_scrivi_merge_chapter`). This sprint was Linux-only (Qt/C++);
  `scrivi.h` and ScriviCore untouched.
- **Result envelopes decoded (same as SP-075):**
  - `scrivi_merge_scene` → `survivorSceneID`, `mergedSceneID`, `chapterID`, `survivorMetadataPath`,
    `survivorContentPath`, `chapterMetadataPath`, `merged`.
  - `scrivi_merge_chapter` → `survivorChapterID`, `mergedChapterID`, `survivorChapterMetadataPath`,
    `scenesRelocated`, `merged`.
- **Stale-path contract (I-0081):** both merges relocate/rename files, so `EditorShell` reloads from disk
  (disk authoritative after the atomic endpoint) rather than keeping pre-merge paths.
- **Design choice — reload, not splice.** Both merges relocate/rename files on disk, so every captured
  segment path is stale. Rather than hand-write two `SceneDocument` merge-splice methods + a `refreshScenePaths`
  dance, the slots reload from disk via `load(...)`. A merge is a rare structural op (reload cost trivial), and
  reload is provably in-sync with disk. Deviates from the plan's "SceneDocument merge splices" wording by design.
- **AC6 history barrier — N/A on Linux.** Linux has **no history subsystem** (EP-019 `HistoryCapture` is
  Apple-only); its create/split/delete/reorder ops record no barrier either. The merge records none here — a
  documented platform gap, not an omission. The macOS side (SP-075) records the barriers.
- **Blank-line join (parity):** scene-merge joins the two bodies with a **blank line** (the endpoint's
  `SceneMerger.joinBodies` default; SP-075 adopted this for macOS). Linux matches so both platforms and the
  on-disk result agree.
- **pbxproj:** N/A — every file touched is Linux/CMake/CI/docs; no Apple sources changed. New Linux
  `scene_merge_smoke.cpp/.sh` registered in the Linux CMake + CI, not `Scrivi.xcodeproj/project.pbxproj`.

### Verification (2026-07-22)

- ✅ **Container build green** — Qt 6 container build (CI toolchain) compiled the app + all smokes, exit 0.
- ✅ **Smokes green** — `scene_merge_smoke` PASS (scene merge joins bodies; chapter merge relocates all scenes,
  nothing lost = I-0083), `scene_create_smoke` (T-0309 split case) PASS, `scene_reorder_smoke` PASS (regression).
- ✅ **Live VNC — scene-merge** (T-0304/T-0305/T-0306): `Ctrl-Backspace` at a scene start merges into the
  previous scene, blank-line joined, survives quit→reopen with nothing lost; caret lands at the seam, no
  view re-center (GUI finding #1 fixed).
- ✅ **Live VNC — chapter-merge** (AC5/AC6 Linux): verified via the **SP-077 Chapter ▸ Merge menu action**
  (the `Ctrl-Shift-Backspace` keystroke is swallowed by the macOS→VNC input path when Shift is held — proven
  not an app bug via key-log instrumentation, see GUI finding #4). Chapter merges into the predecessor,
  every scene relocated, survives quit→reopen. **This closed AC5/AC6 on Linux.**
- ✅ **Ctrl+Return mid-scene split** (T-0309) and **between-scene separator rule** (T-0308) verified live.

### GUI findings (first VNC pass 2026-07-21) + dispositions

1. **Scene-merge re-centered the view.** Was calling `moveCaretToSegment` → `centerCursor()`. **FIXED:** place
   the caret at the seam directly + `ensureCursorVisible()` (scrolls only if the seam fell off-screen). View
   now stays put.
2. **`Ctrl+Return` split at the bottom, not the caret.** `onCreateSceneRequested` always appended an empty
   scene. **FIXED (T-0309):** compute head/tail at caret; mid-body → save head→current + tail→new + reload.
3. **No key-repeat over VNC; macOS Opt/Cmd-arrow word/line nav absent.** Environmental + platform convention:
   added `-repeat` to `run-vnc.sh`; Linux/X11 word-nav is `Ctrl+arrow` / line is `Home/End` (Qt defaults). No
   app change warranted.
4. **Chapter-merge did nothing over VNC — ROOT CAUSE: the keystroke never reached the app.** Instrumented
   `keyPressEvent` proved `Ctrl+Shift+Backspace` over VNC delivers only `Key_Control` + `Key_Shift`; the
   `Key_Backspace` is swallowed upstream whenever Shift is held. Scene-merge (`Ctrl+Backspace`, no Shift) works
   fine. **NOT an app bug** — the handler/guards/signal are correct. **Disposition:** the native menu bar
   (SP-077) gives chapter-merge a trigger the remote path doesn't eat → **AC5/AC6 verified via Chapter ▸ Merge**
   in SP-077. TEMP diagnostics stripped in SP-077 T-0313.

### Retrospective

- **What went well:** The SP-074 core + SP-075 macOS reference made the Linux port a small, well-scoped mirror —
  the bridge/shell/editor changes cloned proven patterns and passed the container smokes on the first green build.
  The reload-from-disk decision sidestepped the entire stale-path class (I-0081) with fewer moving parts.
- **What surfaced:** VNC input-path limitations, not app bugs — the `Ctrl-Shift-Backspace` swallowing forced the
  native menu bar (SP-077) earlier than planned, but that menu is standard desktop UX and made chapter-merge
  testable and discoverable regardless. Live GUI testing again caught what smokes couldn't (the re-center jump,
  the split-at-bottom parity gap).
- **Outcome:** EP-028 AC5, AC6 (Linux), AC7 all Verified. Combined with SP-074/SP-075, EP-028 is complete.

---

*Closed 2026-07-22 (Human-approved). SP-076 delivered the `[Linux]` merge parity — scene-merge live over VNC,
chapter-merge live via the SP-077 menu; between-scene separator rule + `Ctrl+Return` mid-scene split as Apple
parity extras. Combined with the SP-077 native menu bar, this closed EP-028 AC5/AC6/AC7 and the Epic.*
