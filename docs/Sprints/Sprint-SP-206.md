---
sprint: SP-206
epic: EP-054
status: Active
platform: Cross
created: 2026-11-06
---

# SP-206 — `[Apple]` + `[Linux]` [EP-054] **S2**: Export Archive… and Import Archive… on the Mac, Linux and iPad

**Status:** 🟡 **ACTIVE** 2026-11-06 · ✅ built, AC1–AC6 met · ⏳ AC7 live passes · (user, 2026-11-06: *"Q1–Q3 as recommended, activate and implement SP-206"*) — created 2026-11-06 (user: *"You may complete the planning for EP-054."*); planned in full 2026-11-06 (user: *"proceed to SP-206 planning"*). [SP-205] ✅ closed 2026-11-06. ✅ Q1–Q3 ruled as recommended.
**Carries:** [T-0665] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-205] (S1, ✅ closed): `scrivi_export_archive` / `scrivi_import_archive` — the contract is the comment above them in `scrivi.h`.
**Authority:** EP-054 Q1 (one Epic, a platform AC each), Q3 (world optional, asked at export), Q4, Q8 (✅ ruled 2026-11-06).
**Size:** M.

---

## What reading the code found (2026-11-06)

- **Mac File menu:** `ScriviApp.swift:306` (New / Open / Close). **iOS:** `ScriviApp.swift:533`, with Open going through
  `presentOpenImporter` → `LandingView.swift:84`'s `.fileImporter`. **Linux:** `ScriviWindow.cpp:125`, with Import / Export
  Timeline already in File (`:158–170`) — the archive items go beside them.
- **Opening after import** reuses each platform's open path: Mac `loadProject(at:bookmarkURL:)` (`AppEnvironment.swift:663`
  is the panel's), iOS `openProjectFromPickedURL` (`:698`), Linux the landing's open flow.
- ⚠️ **Both platforms raise an already-open window for the same project ID** — Mac `AppEnvironment.swift:437` (R3), Linux
  `OpenProjectRegistry.hpp` (`[R-Q2]`: *"two copies of one project at different paths are the SAME project and must
  resolve to ONE window"*). So importing an archive while its original is open would show the **original's** window, and
  the writer would edit the old copy believing it is the imported one (→ Q1; this is [I-0354]'s sharpest case).
- ⚠️ **The Mac app is sandboxed** (`Scrivi.entitlements`). The core can only find a world "already here" (SP-205 Q1) if the
  app can read the world's folder. `WorldBookmarkStore.activate(worldID:)` re-acquires the grant for a world this Mac has
  used before; without it the core would see no world and **unpack a second copy** — the thing EP-054 Q8 forbids.
- **Document types:** one `Scrivi/App/Info.plist` serves the Mac and iOS apps; it exports `com.caposoft.scrivi.project` and
  `com.caposoft.scrivi.world`, both packages. A double-clicked `.scrivi` already routes to the open path
  (`AppEnvironment.swift:825`).
- **Progress:** the Mac and iOS have `loadProgress` (`LandingView.swift:48`) and the 400 ms progress row
  (`ViewportSceneLoader.swift:544`); Linux has its own.

## Goal

✅ A writer can send a project, with its history and optionally its world, from one machine and import it on another —
Mac, Linux and iPad — through the platform's own file surfaces. No archive logic in the platform layers: each asks the
writer and calls the two endpoints.

## Plan

0. **Red tests first:** an interop test that exports and imports through `ScriviEngine` (`scripts/run-interop-tests.sh`),
   and a Linux smoke that does the same through `ScriviBridge`. Both fail until the wrappers exist.
1. **Wrappers:** `ScriviEngine.exportArchive` / `importArchive` (and the visionOS stub — `check-engine-stub-parity.sh`),
   `ScriviBridge::exportArchive` / `importArchive`. Off the main thread, with the progress row after 400 ms.
2. **Export** — File ▸ **Export Archive…**, for the project in the focused window (Q2). The editor's unsaved text is
   flushed first (the core then drains the queue). A save panel defaults to "<project name>.scrivarchive". When the project
   binds a world: *"Include the world?"* (Q3 of EP-054). A world skipped as unavailable is named afterwards.
3. **Import** — File ▸ **Import Archive…**: choose the `.scrivarchive`, then the folder to unpack into. On a conflict the
   writer gets **Cancel / Rename / Replace** for the project, **Cancel / Rename** for a world; Rename asks for the new name,
   and the call is made again. Replace on the project open in a window: that window must be closed first (the core
   refuses it too). Then the project opens in its own window, as File ▸ Open does — subject to Q1.
   - **Mac:** before the call, activate the world bookmarks this Mac holds, so a world already here is found and not copied.
   - **iPad / iPhone:** `fileImporter` for the archive, a folder picker for the destination, the share sheet /
     `fileExporter` for export. System defaults only.
4. **Document type:** export `com.caposoft.scrivi.archive` (`.scrivarchive`, conforming to `public.zip-archive`) in
   `Info.plist`, so Finder and Files show it as Scrivi's; Q3 decides whether opening one starts an Import.
5. **Linux:** the same two items in File, beside the timeline items; Linux adopts the Mac's shape and wording.
6. **Cross-machine live passes** — the Epic's AC1: Mac → Linux, Linux → Mac, Mac → iPad; each imported, edited, undone
   and redone; plus Cancel / Rename / Replace on one machine.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Importing while a project with the same ID is open** (the original, usually). Both platforms raise the open window for that ID by ruling ([R-Q2] / R3), so the writer would be shown the OLD copy. | ✅ **Unpack as usual, but do not open it: say so.** *"A copy of this project is already open. Close it to open the one you imported."*, naming both folders. Allowing both copies open at once is [I-0354]'s question (it reverses [R-Q2] on both platforms) and stays out of this Sprint. |
| **Q2** | **Which project Export Archive… exports.** | ✅ **The project in the focused window** (disabled with no project open), flushed first. Exporting a project that is not open would need a project picker and an open-for-export path; not needed for the alpha. |
| **Q3** | **Double-clicking a `.scrivarchive`** in Finder or Files. | ✅ **Starts Import:** Scrivi opens, asks for the folder to unpack into, and goes on as File ▸ Import Archive…. On Linux, double-click needs the `.deb`'s MIME registration — that is [EP-056]'s, not this Sprint's. |

⛔ **NOT in this Sprint:** two copies of one project open at once ([I-0354]); Linux double-click (EP-056); manuscript
export formats (S3+).

## Acceptance Criteria

- [x] AC1 — Mac: File ▸ Export Archive… / Import Archive…, the world question at export, the folder choice and Cancel / Rename /
      Replace at import; a world already on this Mac is bound, not copied (sandbox grants activated)
- [x] AC2 — Linux: the same, in the File menu
- [x] AC3 — iPad/iPhone: export by the share sheet, import from Files; `.scrivarchive` declared on the Apple apps (Q3)
- [x] AC4 — Both calls off the main thread with progress; the window never freezes on a large project
- [x] AC5 — Importing while the same project is open does what Q1 rules, on every platform
- [x] AC6 — Interop tests and Linux smokes through the wrappers, green; engine stub parity check green
- [ ] AC7 — ✅ **Live passes (user):** Mac → Linux, Linux → Mac, Mac → iPad; reads, edits, undoes and redoes as it did
      (EP-054 AC1, AC2, AC3, AC8). The Linux half on the VM or rig → an [EP-057] Task at close (Sprint-GUIDELINES)

## Progress log

**2026-11-06 — built (Mac, iOS, Linux).** No archive logic in any platform layer: each asks and calls the two endpoints.
- **Wrappers:** `ScriviEngine.exportArchive` / `importArchive` + `ExportArchiveResult` / `ImportArchiveResult` and the
  visionOS stub entries (`check-engine-stub-parity.sh` ✅); `ScriviBridge::exportArchive` / `importArchive`, which also
  carry the error's `detail` (`parseEnvelope` drops it; the flow branches on `projectOpen`).
- **Apple — one flow for the Mac and iOS:** `Scrivi/App/ProjectArchiveFlow.swift` (new; pbxproj `C2A5`, all three
  targets). Export flushes (`saveAllDirty`), asks *"Include the world?"* when the project binds one, and names a skipped
  world afterwards. ⚠️ **Sandbox:** the core writes beside its target, and the save panel grants only the file named, so the
  Mac writes into `itemReplacementDirectory` and moves the result into place. Import activates every world grant this Mac
  holds first (`WorldBookmarkStore.activateEverything`), records a grant for each world it unpacks, and opens through
  `openProjectFile` (now internal) — the same path as a double-clicked `.scrivi`. The questions are `NSAlert` on the Mac
  and an `.alert` on iOS (`ArchiveFlowHost`, over the iOS root), with a name field for Rename.
  - **Surfaces:** File ▸ Export Archive… / Import Archive… (Mac and iPad menu bars); the toolbar's declared Export slot
    (EP-040) — the iPhone/iPad way without a keyboard; Landing's **Import Archive…** button. iOS exports to a temporary
    file and offers the share sheet; it picks the archive, then the folder, with one `fileImporter`.
  - **Progress (AC4):** the work runs detached; after 400 ms the project window's progress capsule shows *"Exporting
    archive…"* / *"Importing archive…"*, or Landing does when no project window hosts it.
  - **Q1:** an imported project whose ID is open is not opened; the writer is told, with both folders. **Q3:** a
    `.scrivarchive` reaching `openProjectFile` (Finder / Files) asks for the folder and imports. `Info.plist` exports
    `com.caposoft.scrivi.archive` (`.scrivarchive`, conforms to `public.zip-archive`), Viewer / Owner.
- **Linux:** File ▸ Export Archive… (`EditorShell::exportArchive`, the editor's progress row) and Import Archive…
  (`ShellController::importArchive`, also Landing's **Import Archive** button); both calls on a worker (`AsyncCall`, no
  timeout: they write). The import opens through Landing's own open flow (`openPathRequested` → `openPath`), so repair,
  recents and R3 are unchanged; Q1 asks `existingWindowFor` first.
- **Tests:** interop `archiveRoundTripThroughEngine` (export, import keeps the ID, a clash is a conflict, Rename) — ✅ 1 test
  passed; `bridge_parity_smoke` gains the same checks through `ScriviBridge` — ✅ **70 checks, 0 failures** (Docker, non-root).
  ⚠️ Its first run SEGFAULTED: ASan showed a pre-existing spy lambda (the EP-026 history block) capturing a block-local by
  reference and never disconnected — the archive's failing call was the first `errorOccurred` after that block. Fixed in the
  smoke (disconnected); not a product defect. Builds: macOS ✅, iOS Simulator ✅, Linux (Docker) ✅; stub parity ✅.
- ⚠️ **visionOS does not build — NOT this Sprint's:** `EditorView.swift:563`/`:570` (`.inspector`, `.inspectorColumnWidth`
  are unavailable on visionOS), untouched here; the new files compile there.
- ⏳ **AC7** — the cross-machine live passes are the user's.
