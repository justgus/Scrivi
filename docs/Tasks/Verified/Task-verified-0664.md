# Verified Task — T-0664

**Sprint:** [SP-205] · **Epic:** [EP-054] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-06** (user, 2026-11-06: *"verify T-0664"*)
**Archived:** 2026-11-06.

---

## ✅ T-0664 — `[ScriviCore]` EP-054 S1: the project archive and its worlds — export, import, miniz

**Epic:** [EP-054] (S1) · **Status:** ✅ **Verified 2026-11-06** (user) · **Sprint:** [SP-205] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-205.md`](../../Sprints/Closed/Sprint-SP-205.md) · **Date Requested:** 2026-11-06 (user: *"You may complete the planning for EP-054."*)

**What:** `scrivi_export_archive` and `scrivi_import_archive` in the core, with miniz compiled into the static library (EP-054
Q7). The archive holds the package less Q9's list, history and `.git/` included, written after the write queue drains; it
optionally carries the bound worlds. Importing it unpacks the project, same ID, into the folder the writer chooses; a
folder name already there is Cancel / Rename / Replace (Q4); a world already on the machine is bound, not copied (Q8). Boundary-tested through `scrivi_*`.

**Done:** `ScriviCore/src/project_package/ProjectArchive.{hpp,cpp}`; `scrivi_export_archive` / `scrivi_import_archive`
(`scrivi.h:91–92`); miniz 3.0.2 in `ScriviCore/CMakeLists.txt`. Details, line references and evidence:
[`../../Sprints/Closed/Sprint-SP-205.md`](../../Sprints/Closed/Sprint-SP-205.md) progress log.

**Tested:** `ArchiveCApiTests.cpp` (8 cases, `[archive]`), mutation-checked (10/10 caught); ctest macOS 814/814, Linux Docker
non-root 818/818; ScriviCore builds for the iOS simulator; the Mac app builds.


**Verified (user, 2026-11-06):** *"verify T-0664"* — on the core evidence above (no surface exists yet; the platforms' live passes are [SP-206]'s).
