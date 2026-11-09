# Verified Task — T-0665

**Sprint:** [SP-206] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-09** (user, 2026-11-09: *"Step 3 passes on the iPad now.  Steps 4 and 6 pass on all three platforms."* — with steps 1, 2 and 5 passed earlier the same day)
**Archived:** 2026-11-09.

---

## ✅ T-0665 — `[Cross]` EP-054 S2: Export Archive… / Import Archive… on the Mac, Linux and iPad

**Epic:** [EP-054] (S2) · **Status:** ✅ **Verified 2026-11-09** (user) · **Sprint:** [SP-206] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-206.md`](../../Sprints/Closed/Sprint-SP-206.md) · **Date Requested:** 2026-11-06 (user: *"You may complete the planning for EP-054."*)

**What:** the two archive endpoints on each platform's own file surfaces — File menu on the Mac and Linux, the share sheet
and Files on iPad/iPhone — off the main thread with progress, asking whether to include the world (Q3) and where an
unknown world goes (Q8). Live-passed across machines: Mac → Linux, Linux → Mac, Mac → iPad.

**Implemented (2026-11-06, [SP-206]):** `Scrivi/App/ProjectArchiveFlow.swift` (Mac + iOS: Export / Import, the world question,
Cancel / Rename / Replace, Q1, Q3), `ScriviEngine.exportArchive` / `importArchive` (`ScriviEngine.swift`), File menu + toolbar
Export slot + Landing's Import button, `Info.plist` (`com.caposoft.scrivi.archive`); Linux `EditorShell::exportArchive`,
`ShellController::importArchive` (`ScriviWindow.cpp`), `ScriviBridge::exportArchive` / `importArchive`, Landing.qml. Evidence:
interop `archiveRoundTripThroughEngine` ✅, `bridge_parity_smoke` 70/70 ✅ (Docker). AC7 live passes (user) ✅ 2026-11-09.
**Fixed (2026-11-07, live pass):** the export refused a trailing-slash project root (Mac: *"invalid filename"*) and reported
a removed reference's leftover `worlds/` folder as an unreachable world (Linux) — `ProjectArchive.cpp` (`folderPath`,
`listWorlds`); `ArchiveCApiTests` covers both.
**Fixed (2026-11-09, live pass):** iOS did not declare `.scrivarchive`, so its picker greyed the archive out —
`Scrivi/App/Info-iOS.plist` (new); the picked archive was then dropped (the step was read after the picker cleared it) —
`ArchiveFlowHost` keeps it in `@State`. **Added (user):** *"Import “<name>”"* between the pickers on every platform — a name
field (→ `projectFolderName`) and **Choose Folder…**; on iOS the folder picker opens in the archive's folder.

