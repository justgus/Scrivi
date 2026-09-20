# Verified Issues: I-0231 – I-0240

| ID | Issue | Priority | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| **I-0231** | `[ScriviCore]` ⚠️ **Project open re-read every sidecar ~96× — 55,574 reads / 8,044 opens for a 150-file project.** ✅ **LOCATED BY MEASUREMENT (SP-144 AC1), not by reading the code**: a `CountingFileSystem` decorator attributed every call to the pass that made it, showing **six independent walks over the same manuscript** (`migrateScenes` 241 calls, `validate` 203, `resolveOrder` 151). ✅ **Fixed by `util::ReadThroughCache`** — scoped to ONE `openProject`, write/rename-invalidating (the passes REPAIR, so a stale hit would silently undo a repair), ⛔ **never persistent: the filesystem stays authoritative (EP-027) and Scrivi does no filesystem watching.** ✅ **Plus a shared `BindingCache` in `repairDangling`, the pass [I-0207] missed while fixing `listPending`.** ⚠️ **MEASURED: 622 → 196 calls/open; absent `binding.json` 25 → 1 (the 25 verified by REVERTING the fix, not assumed).** | **High** | **[SP-144]** | 2026-09-20 |
| **I-0232** | `[Linux]` ⚠️ **The landing-flow `openProject` was synchronous on the UI thread — 155 s frozen on the launch screen before [I-0195]'s progress bar could appear.** ✅ **AC4: `ScriviBridge::openProjectAsync` reuses `AsyncCall`** (⛔ not a second mechanism — this was never a missing mechanism, only an unconverted call site); ⚠️ **only the C ABI call crosses to the worker, `parseEnvelope` runs back on the UI thread**, ⛔ **which is exactly the split [I-0199] got wrong.** ✅ **AC5: the project is opened ONCE** — `Landing.qml` hands its envelope through to `EditorShell::load`. ⚠️ **This is APPLE'S shape; Linux had drifted.** ✅ **The empty-envelope path is retained for the two structural-edit reloads and New Project.** | **High** | **[SP-144]** | 2026-09-20 |
| **I-0233** | `[Apple]` ⚠️ **Apple never called `scrivi_close_project`, so the core's `ProjectIndex` registry leaked one index per project opened, for the life of the process.** ⚠️ **Found while verifying Linux/Apple parity — ✅ and the answer INVERTED: Linux was the platform doing it correctly.** ⚠️ **[T-0512] was recorded ✅ Verified claiming BOTH platforms were wired; only Linux actually was.** ✅ **Released from `ProjectSession.close()`** — ⚠️ **the single chokepoint the red button, a fullscreen tab's ✕, ⌘W and both File ▸ Close Project commands all reach** — ⚠️ **before `projectRootPath` is cleared, since the registry is keyed by root path.** | **Medium** | **[SP-144]** | 2026-09-20 |
| **I-0234** | `[Cross]` ⚠️ **The viewport's bulk load performed one atomic read-modify-write of `workspace-state.json` PER SCENE — 61 durable writes to record a value only the last of which survives.** ✅ **THIS IS WHY SMALL PROJECTS LOADED SLOWLY** — ⛔ **falsifying "it is slow because the manuscript is big".** ⚠️ **MEASURED through the shipped ABI: the manuscript was NOT the cost (every sidecar opened just 3×); the workspace file was 62 opens + 61 `.tmp` opens + 61 renames.** ✅ **Fixed by `scrivi_open_scene_for_bulk_load`** — ⚠️ **a SEPARATE endpoint, not a changed signature** (⛔ changing `scrivi_open_scene` would break every caller on both platforms at once), ✅ **sharing one `openSceneImpl` so the two cannot drift.** ✅ **Writes went from O(scenes) to O(1): at 240 scenes, 242 writes → 1.** | **High** | **[SP-144]** | 2026-09-20 |
| **I-0235** | `[Linux]` ⚠️ **Resume landed at the TOP of the scene instead of the scroll offset ("minus one line") when the writer scrolled without typing.** ✅ **Found by the USER on the real rig (build 45) by the exact no-typing check this Sprint asked for.** ⚠️ **A regression introduced by [SP-144] itself** — ⛔ it could not exist before, because `openScene` used to stamp the surface as a side effect of every read ([I-0234] removed that, correctly). ⛔ **CAUSE: `stampWritingSurface()` delegated to `saveScene(activeSegment_)`, and `saveScene` writes the scroll fraction ONLY when the CARET is in that segment** — ✅ **correct for a background flush**, ⛔ **wrong for the stamp, whose whole purpose is the writer who scrolled and never typed.** ✅ **Fixed by saving directly, carrying the REAL scroll fraction ALWAYS.** ⚠️ **Apple was never affected** (`ViewportSceneLoader.swift:464` passes it unconditionally). ⚠️ **LESSON: "delegate to the existing save" looked like reuse and was not — it adopted the CALL without the BEHAVIOUR behind it.** | **Medium** | **[SP-144]** | 2026-09-20 |

---

## ✅ How these were verified — ⚠️ **on the REAL rig, not in Docker**

⚠️ **[SP-144] AC6 was explicit that Docker could not close these**: it has no slow
mount and the page cache hides the defect — ✅ **the exact gap that let [I-0195]
be marked resolved while it was still broken.**

✅ **User-run on 🐧 `Oathkeeper`, build 45/46, against the real project
`/mnt/scrivi-worlds/the-stairs-of-tintagael.scrivi`, mount confirmed
`cifs …cache=none…` by the probe itself:**

| | ⚠️ BEFORE | ✅ AFTER |
| - | --------- | -------- |
| Wall-clock | ⚠️ **24.01 s** | ✅ **13.46 s** (−44%) |
| Syscalls | ⚠️ **5,002** | ✅ **2,999** (−40%) |
| `binding.json` opens | ⚠️ **188** | ✅ **2** |

✅ **Plus the UI pass: the load happens ONCE, both progress bars appear and update
(indeterminate on the launch screen, then determinate in the editor — seeing BOTH
is the handoff working), the load is much faster, and the drive pull still asserts
correctly.**

---

## ⚠️ What this decade is worth remembering for

✅ **Two of these were found by MEASUREMENT that the Sprint mandated in advance**
([I-0231] AC1: *"locate it by measurement, not by reading"*), ⚠️ **and one
([I-0235]) was found by a LIVE PASS doing something no suite did — scrolling
without typing.**

⚠️ **[I-0233] and [I-0232] are the same lesson from opposite directions: the two
platforms had drifted apart.** ✅ **The user made it a STANDING RULE on
2026-09-20 — Linux must ALWAYS adopt Apple's shape, and a shape change on Apple
must be made the same way on Linux, in the same work.**

⚠️ **[I-0234] is the one that reframed the Epic**: the user's observation that
*"none of the projects I've loaded lately have been large"* ⛔ **falsified the
working theory** and pointed at a per-scene WRITE, not a per-file read.
