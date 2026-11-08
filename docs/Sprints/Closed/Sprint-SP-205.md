---
sprint: SP-205
epic: EP-054
status: Closed
platform: ScriviCore
created: 2026-11-06
---

# SP-205 — `[ScriviCore]` [EP-054] **S1**: the project archive and its worlds, in the core

**Status:** ✅ **CLOSED** 2026-11-06 (user, 2026-11-06: *"yes, file the issue, then close SP-205."*) — active 2026-11-06. ✅ Q1–Q3 ruled 2026-11-06.
**Carries:** [T-0664] → [`../../Tasks/Verified/Task-verified-0664.md`](../../Tasks/Verified/Task-verified-0664.md) (✅ Verified 2026-11-06).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Authority:** EP-054 Q3, Q4, Q7, Q8, Q9 (✅ ruled 2026-11-06, all as recommended) ·
[`../Scrivi_Project_Package_Structure_v0_1.md`](../../Scrivi_Project_Package_Structure_v0_1.md) §11.2 (world bindings), §16a
(history inside the package), §17 (what is not content).
**Next:** [SP-206] (S2) puts it on the Mac, Linux and iPad. ⛔ **No surface in this Sprint.**
**Size:** L.

---

## What reading the code found (2026-11-06)

- `scrivi.h` has no project, world or manuscript import/export. The only zip-shaped need so far is this one.
- ⚠️ **No zip library in ScriviCore.** Its dependencies are nlohmann/json and md4c. md4c is the pattern Q7 follows:
  FetchContent, one `.c` file compiled straight into the static library, never in a public header
  (`ScriviCore/CMakeLists.txt:10–21`, `:120–123`).
- `history/` lives inside the package (§16a, Trade T6) and is excluded only from Git (§17), so undo travels by design.
- A world binding records the world's path on the **sending** machine (`worlds/`, §11.2). A world is its own package with
  its own `.lock` (`WorldStore.cpp:75`). `scrivi_relink_world` exists (`scrivi.h:369`) and is what Q8's arrival uses.
- The write queue can be drained: `scrivi_wait_for_writes` (`scrivi.h:1181`, [EP-052]). Q9's "never a half-written state".

## Goal

✅ Two endpoints. `scrivi_export_archive` writes one zip file holding the whole project package (Q9's contents) and,
when asked, its bound worlds. An archive is never treated as a project: nothing reads, searches or edits inside it.
`scrivi_import_archive` unpacks it into the folder the writer chooses, as a project that reads, edits, undoes and redoes
exactly as it did, with the same project ID; the platform then opens it. A name already in that folder is Cancel / Rename
/ Replace (Q4); a world already on the machine is bound, not copied (Q8).

## Plan

0. **Red tests first, through `scrivi_*`** (a facade test cannot see a boundary gap — [I-0113]): export, import into a temp folder,
   and compare — every content file byte-identical, the same project ID, and the history replays the same undo/redo. All
   fail today (no endpoints).
1. **miniz** (Q7): FetchContent pinned to a release tag, its single `.c` compiled into `ScriviCore` as md4c is, `SYSTEM
   PRIVATE` include. `libScriviCore.a` stays the app's only link input — macOS, the iOS simulator and Linux Docker all
   build.
2. **`scrivi_export_archive(projectRoot, destPath, includeWorlds)`**: drain the write queue first; walk the package
   leaving out Q9's list (`.lock`, `.cache/`, `indexes/`, OS noise — §17); keep `history/` and `.git/`. With
   `includeWorlds`, each bound world goes under its own top-level folder with a small manifest naming its world ID. Write
   to a temp file and rename (the existing atomic write), so a failed export leaves nothing half-written at `destPath`.
3. **`scrivi_import_archive(archivePath, destFolder, …)`** → the unpacked project's root, which the platform opens:
   - unpack to a temp folder beside `destFolder`, validate (it must be a `.scrivi` package), then move into place;
   - **Q4:** if the project's folder name is already in `destFolder`, the call reports it and does nothing; the platform
     asks **Cancel / Rename / Replace** and calls again with the writer's answer. **Rename** unpacks under the new folder
     name. **Replace** unpacks and validates in the temp folder first, then removes the old project folder and moves
     the new one into place, so a failure never leaves the writer with neither (user, 2026-11-06: a temp folder or a
     temporary name, *"either one I don't care which"*). A world name already there is Cancel / Rename only (Q3). ⛔ Replace is refused while that project is open in a
     window;
   - **Q8:** for each world in the archive — a world with the same world ID at the binding's recorded path or in the chosen
     folder → bind to it, unpack nothing; otherwise unpack it to the chosen folder and `scrivi_relink_world` the binding.
     A world the archive does not carry but the project binds is left unresolved, exactly as opening a project whose world
     is missing is today (repair matrix);
   - ⛔ every rejection path (not a zip, not a package, a zip entry that escapes the destination — `..` or an absolute path)
     returns an error envelope and leaves nothing behind.
4. **Tests:** the round trip (with and without worlds, with Git snapshots on and off), Cancel / Rename / Replace (Replace failing mid-way
   keeps the old project), a world already present vs. absent, each rejection path, and an export during queued writes. Mutation-checked in a **copy** of the repo.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **How does the core know a project or world is "already there"** (Q4, Q8)? Neither platform keeps a list of projects. | ✅ **Ruled 2026-11-06 (user): it asks the filesystem, not a list.** **Projects:** the folder name in the chosen folder → Q4's Cancel / Rename / Replace. **Worlds** (*"I approve the worlds."*): a world with the same world ID at the path the binding records or in the chosen folder → bind, unpack nothing; not found → unpack there. |
| **Q2** | **The archive's file extension** (EP-054 Q3: "its own extension"). | ✅ **`.scrivarchive`** (user, 2026-11-06), matching `.scrivworld`; the file takes the project's name. A zip inside, so it can still be unzipped by hand. |
| **Q3** | **A world folder name already in the chosen folder, with a DIFFERENT world ID.** Replacing it would break every other project bound to that world. | ✅ **Cancel / Rename only — no Replace for a world** (user, 2026-11-06: *"Q3. agreed. no replace for worlds."*). |

## Acceptance Criteria

- [x] AC1 — miniz compiled into `libScriviCore.a` (Q7); macOS, the iOS simulator and Linux Docker build, the app links
      nothing new
- [x] AC2 — `scrivi_export_archive` writes Q9's contents after draining the write queue; a failed export leaves no file
- [x] AC3 — `scrivi_import_archive` round trip: every content file identical, the history replays the same undo/redo (EP-054
      AC1 at the core)
- [x] AC4 — A project folder name already in the chosen folder: Cancel extracts nothing, Rename unpacks under the new name,
      Replace swaps only after the archive has unpacked and validated; the ID is kept (EP-054 AC3, Q4)
- [x] AC5 — Worlds: a known world is bound and nothing unpacked; an unknown one is unpacked where asked and relinked; its
      objects resolve (EP-054 AC2, Q8)
- [x] AC6 — Every rejection path (not a zip, not a package, path escape) errors and leaves nothing behind
- [x] AC7 — Boundary tests through `scrivi_*`, mutation-checked; ctest on macOS and in the Linux Docker test image as
      non-root, green (EP-054 AC7's first fixtures)

## Progress log

**2026-11-06 — built** (user: *"activate and implement SP-205."*). T-0664 → 🟠 Implemented - Not Verified.

- **Code:** `ScriviCore/src/project_package/ProjectArchive.{hpp,cpp}` — `excluded` (Q9, `:152`), `exportArchive` (`:169`),
  `importArchive` (`:270`; Replace at `:447`). Endpoints `scrivi_export_archive` / `scrivi_import_archive`
  (`scrivi.h:91–92`, `scrivi_c_api.cpp:1105`, `:1155`). miniz 3.0.2, pinned by SHA-256, in `ScriviCore/CMakeLists.txt`
  (`MINIZ_NO_ZLIB_COMPATIBLE_NAMES`).
- **Shape the plan did not spell out:**
  - The archive's first entry is `scrivarchive.json` — `{format: "scrivi.archive.v1", projectFolder, projectID,
    worlds: [{worldID, folder}]}`. A zip without it is refused (`notAnArchive`).
  - A name clash is not an error: `{imported: false, conflict: {kind, folderName, worldID?}}`, nothing extracted. The
    platform asks, then calls again with `optionsJSON` (`projectFolderName`, `replaceProject`, `worldFolderNames`).
  - **"Open in this process"** (Replace's refusal) = the core holds the project's history or index. The platform must
    still close its own window first (SP-206).
  - The world destination is the chosen folder, the same one as the project.
  - A bound world that is unavailable at export is listed in `skippedWorlds`, and the export goes on.
  - Symlinks inside a package are not followed or archived.
  - The archive is zip64, so a package over 4 GB fits.
- **Tests:** `ScriviCore/tests/integration/ArchiveCApiTests.cpp`, 8 cases `[archive]`, all through `scrivi_*`: round
  trip (byte-identical, the ID, empty folders, history undo/redo, Q9's junk absent), a non-ASCII project name, queued
  writes land first, export refused behind a held queue (`writeQueueHeld`), Cancel / Rename / Replace / Replace while
  open / Replace on a non-project, worlds (not carried, bound in place, unpacked + `available`, name clash → Rename),
  rejections (not a zip, no table of contents, `../` entry, bad Rename), failed export leaves no file.
- **Mutation-checked** in a copy of the repo: 10 mutations, 10 caught. ⚠️ The first run left one alive (export not
  waiting for the queue: the queued save landed too fast to tell); the held-queue case was added for it, and it is caught.
- **Evidence:**

  | Check | Result |
  | ----- | ------ |
  | ctest, macOS (`build-tests`, Debug) | ✅ 814/814 (incl. `AbiProjectGuards`) |
  | ctest, Linux (Docker `scrivi-linux`, Ubuntu 24.04, GCC 13, non-root uid 1000) | ✅ 818/818, 0 compiler warnings |
  | ScriviCore for the iOS simulator (arm64, 27.1) | ✅ builds |
  | macOS app (`xcodebuild -scheme ScriviApp`) | ✅ BUILD SUCCEEDED, no new link input |

  ⚠️ One earlier macOS parallel run had two unrelated failures (`openProject - returns repairRequired when scene
  .meta.json is corrupt`, `applyRepair - regenerateMetadata (scene) creates new .meta.json`). Both passed when re-run, and
  three more full parallel runs were green. Not caused by this Sprint. A likely cause, not proven: 41 test files name temp
  folders only by `steady_clock`, so two test processes can pick the same folder.

## Retrospective

**Completed:** `scrivi_export_archive` and `scrivi_import_archive` with miniz, AC1–AC7, through the C ABI; 8 test cases,
10/10 mutations caught; ctest macOS 814/814, Linux Docker non-root 818/818.

**Verified:** [T-0664] ✅ 2026-11-06 (user, 2026-11-06: *"verify T-0664"*).

**Returned to backlog:** nothing.

**What went well:**
- Planning in the open. The Q1/Q4 discussion replaced "copy with a new ID" with Cancel / Rename / Replace, keeping the ID
  as the work's identity, before any code depended on it.
- The mutation run found a real test gap (export not waiting for the queue survived), and it was closed in the Sprint.

**What to improve:**
- My first Q1 assumed a registry the platforms do not keep, and I named the import "Open Archive", which made an archive
  sound like a project. Both cost the user a round of questions.

**Carry-forward notes (SP-206):**
- A name clash returns `{imported: false, conflict}`, not an error. The platform asks the writer, then calls again with
  `optionsJSON`.
- Replace's "open" check only sees what the core holds (history or index). The platform must close its own window for
  that project first.
- The world destination is the folder the writer chose for the project.
- [I-0354] (two copies of one work collide on one machine) becomes reachable once SP-206 ships import.
- [I-0355] filed from this Sprint's flaky parallel run.
