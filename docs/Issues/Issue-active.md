# Active Issues

Issues awaiting **user verification**. An Issue leaves this file only when the user verifies it
(→ `Verified/Issue-verified-XXXX-YYYY.md`, batched in decades of ten) or approves its closure
(→ `Closed/`).

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0285** | `[Linux]` ⚠️ **Opening dumas takes ~3 minutes and FREEZES the UI for ~55 s: the timeline is built TWICE on the main thread, each build asking for every scene's story time one call at a time.** ✅ **MEASURED 2026-10-09** on the rig (build 62, `SCRIVI_LOAD_LOG`), found in [SP-166]. Linked to [EP-048]. → [details](#i-0285) | High | [SP-173] | 🟡 In Progress |
| **I-0242** | `[Linux]` ⛔ **THE WRITER'S CARD STACK IS IGNORED — Linux ENUMERATES KINDS instead of honouring the chosen stack, and DROPS any card that is empty.** ⚠️ **FOUND BY THE USER on the real rig, 2026-09-21**, during [SP-142]'s live pass on `the-stairs-of-tintagael` — ⛔ **not by any test.** ✅ **USER RULING (2026-09-21), and it is the whole specification:** ⚠️ ***"The writer chooses which Card elements to display and in what order they appear. Therefore, if the writer chooses to display an empty card in the stack, then it should be displayed with an appropriate 'No Objects to Display' message. Apple already does this. Linux must as well."*** ⚠️ **REPORTED AS 'Factions is missing'; ⛔ THAT IS THE SYMPTOM, NOT THE DEFECT.** ✅ **Linux CAN render factions** — it derives kinds from `ObjectKindScope`/`kAllStorableKinds` and hardcodes nothing (the standing rule WAS followed). ⛔ **THE DEFECT IS THAT IT NEVER ASKS WHAT THE WRITER CHOSE.** ✅ **`SceneInspector.cpp:551` lists the CORE's kinds and `:577` skips any with no entries in this scene** (*"a kind with nothing in this scene is simply not a row"*) — ⚠️ **so the stack is DERIVED FROM DATA, while Apple's is DECLARED BY THE WRITER.** ⚠️ **APPLE IS THE REFERENCE AND IT IS EXPLICIT:** `ObjectCard.swift:450` renders *"No <kind> in this scene yet."* with the comment ⚠️ ***"§2: empty is a normal state, not an error."*** ⛔ **THE ROOT CAUSE IS A DANGLING READ.** ✅ **`inspector-layout.json` HOLDS the answer — `defaultStacks` and per-scene `scenes` — and [T-0537] just proved Linux ROUND-TRIPS both LOSSLESSLY.** ⚠️ **But `InspectorLayoutStore` exposes ONLY `selectedTab`: ⛔ ZERO reads of `defaultStacks` or `scenes` anywhere in `platforms/linux/`.** ✅ **So Linux has been faithfully PRESERVING a document it does not consult** — ⚠️ **`project_capability_without_surface` in its exact form: a dangling read with no reader.** ⚠️ **CONSEQUENCE BEYOND THE MISSING CARD: ORDER IS ALSO WRONG.** ⛔ **Linux emits kinds in the CORE's order; the writer's chosen order is in `defaultStacks` and is ignored** — ✅ **the user's ruling names order explicitly.** ⚠️ **AND IT BLOCKS THE STACK PICKER ([EP-036] AC4b): a '+' menu that adds a card to a stack nothing reads would change nothing on screen.** ⛔ **That picker had NO Task number; ⚠️ an earlier draft of this row cited `T-0543`, which is now [SP-134]'s — ✅ corrected 2026-09-22.** | **Medium** | ⛔ **UNASSIGNED** — ⚠️ **candidate for [EP-036]; ✅ it is the PREREQUISITE for T-0543, so they should sequence together** | 🔵 **Open** |
| **I-0223** | `[ScriviCore]` ⚠️ **A WORLD CAN BIND TO A DIFFERENT *VERSION* OF ITSELF, SILENTLY — the core detects world IDENTITY but has no notion of world STATE.** ⛔ **REWRITTEN 2026-09-25 BY USER RULING. ⚠️ THE ORIGINAL RECORD WAS SUBSTANTIALLY WRONG AND ITS TEXT IS SUPERSEDED — see 'WHAT THE ORIGINAL CLAIMED' below, kept because the error is instructive.** ---- ✅ **THE USER'S FRAMING, WHICH IS THE SPECIFICATION:** ⚠️ ***"The World's filename and internal ID must match the Project's expectation or else the World Load will be rejected. That will account for every world except a copy of the world or an earlier version of the world. A copy of the world is assumed to be identical... If the mounted file is a mis-matched version (older or newer) of the previous linked world, then an honest reconciliation should take place."*** ---- ✅ **THE IDENTITY CHECK IS REAL, AND IT RUNS ON EVERY CANDIDATE INCLUDING THE RELATIVE ONE.** ✅ **`WorldStore.cpp:428-436`:** ⚠️ ***"IDENTITY CHECK. A package whose worldID differs is NOT this world — resolution stops rather than silently substituting a same-named package. A world's name is a label; its worldID is its identity."*** — ✅ **a mismatch returns `WorldStatus::missing`, NOT a bind.** ✅ **`relinkWorld` enforces the same at `:492-498` with a `worldIDMismatch` detail.** ---- ⚠️ **SO THE SPACE PARTITIONS THREE WAYS, AND ONLY THE THIRD IS A DEFECT:** ✅ **(1) A DIFFERENT WORLD → REJECTED on `worldID`. ⛔ Not a defect; already correct.** ✅ **(2) A COPY of the world → identical `worldID`, identical content, binds and resolves every object reference. ⛔ NOT A DEFECT — a copy is assumed identical and linking to it is FINE (user ruling).** ⛔ **(3) A MIS-MATCHED VERSION of the SAME world (older or newer) → `worldID` MATCHES, so it BINDS SILENTLY, and the core CANNOT NOTICE it is not the version the project last saw. ⚠️ THAT IS THIS ISSUE, ENTIRELY.** ---- ⚠️ **WHY THE CORE IS STRUCTURALLY BLIND TO (3): `worldID` IS IDENTITY, NOT STATE.** ✅ **It answers *"is this the same world?"* and CANNOT answer *"is this the same version of it?"* — ⚠️ **two different questions, and only the first has a mechanism.** ---- ✅ **THE MATERIAL GAP, CONFIRMED IN THE SCHEMAS 2026-09-25:** ⚠️ **`world.json` ALREADY CARRIES `modifiedAt` AND `formatVersion`** (`WorldJson.cpp:25-26`, `WorldRecord` in `worlds/WorldTypes.hpp`) — ⛔ **BUT `WorldBindingRecord` RECORDS NEITHER.** ✅ **It stores only `worldID`, `displayName`, `epochOffsetMs`, `reference` and `cachedIndex`** (`WorldJson.cpp:82-98`). ⚠️ **So the binding has NO RECORD OF WHICH VERSION IT BOUND, and divergence is undetectable by construction — ⛔ not merely unchecked.** ---- ✅ **WHAT THE WORK IS (two capabilities, and the second NEEDS the first):** ✅ **(a) CHANGE-CONTROL DETECTION IN THE WORLD ITSELF** — ⚠️ **something that distinguishes VERSIONS of one `worldID`: a monotonic revision counter or a content hash, written into `world.json` AND recorded in the binding at bind time.** ⛔ **Without this the core cannot detect the case at all, no matter what resolution does.** ⚠️ **`modifiedAt` ALONE IS NOT SUFFICIENT EVIDENCE — a timestamp can go backwards, be preserved by a copy, or be rewritten by sync; ✅ whether it is a usable input is part of the work, not an assumption.** ✅ **(b) HONEST RECONCILIATION** — ⚠️ **when the bound revision and the found revision differ, REPORT IT and say WHICH DIRECTION (older / newer / divergent); ⛔ do not silently pick.** ✅ **This is exactly the two standing rules this code already follows: *"core reports, app decides"* ([SP-141] ruling) and §6a.0's *absence is never deletion*.** ---- ⚠️ **SCOPE WARNING: (a) IS A SCHEMA CHANGE to `world.json` and to the binding, so it needs a MIGRATION story for worlds predating the field** — ✅ **and `WorldRecord::kSupportedFormatVersion` (currently `1`) is the existing mechanism for exactly that.** ⚠️ **THAT is what makes this larger than a resolution patch and why it wants real design, not an incidental fix.** ---- ⚠️ **THE UNMOUNTED-DEVICE VARIANT FOLDS INTO (3), IT IS NOT SEPARATE:** ⛔ **the hazard is NOT resolving to a wrong world (identity blocks that) — ✅ it is resolving to a STALE COPY of the RIGHT world on some other reachable volume while the current one is unmounted.** ⚠️ **Identity passes; nothing flags the divergence.** ---- ⛔ **WHAT THE ORIGINAL RECORD CLAIMED, AND WHY IT WAS WRONG** (⚠️ **kept deliberately**): ⚠️ **it claimed a cross-volume relative path could make the core *"bind the WRONG WORLD and report success"*, severity framed as the [I-0181] class (a wrong answer asserted confidently).** ⛔ **THAT CANNOT HAPPEN — the identity check at `:428` rejects it.** ⚠️ **THE METHOD FAILURE: the 2026-09-22 re-confirmation (*"CONFIRMED STILL LIVE"*) read the CANDIDATE ORDERING at `WorldStore.cpp:305-315` and STOPPED THERE — ✅ it never read the 120 lines further down where the check lives.** ⚠️ **A partial code read produced a confident wrong severity that this record then carried for three days.** ✅ **THE RELATIVE-FIRST ORDERING IS CORRECT AND STAYS** (⚠️ *it is what protects a project and its worlds moved TOGETHER*). ---- ⛔ **THE ORIGINAL ACs ARE VOID:** ⚠️ **old AC2 (*"verify the resolved package IS the bound world"*) IS ALREADY IMPLEMENTED at `:428`; ⚠️ old AC1 (*"skip a meaningless relative candidate"*) addresses a hazard the identity check already closes.** ✅ **[EP-044] MUST BE RE-SPECIFIED around (a) and (b) above.** ---- ⚠️ **SEVERITY RE-REASONED, NOT INHERITED: *"silently binds a STALE VERSION, which is RECONCILABLE"* is a lesser and different thing than *"silently binds the WRONG WORLD"*.** ✅ **It is also arguably NO LONGER the [I-0181] family at all** (⚠️ *that family is about asserting a wrong answer from absent evidence; this is about having no evidence to assert from*). | **Low** — ⚠️ **DROPPED FROM MEDIUM 2026-09-25 (user ruling), re-reasoned NOT inherited:** ⛔ **the Medium came from the FALSE wrong-world claim;** ✅ **"silently binds a STALE VERSION, which is RECONCILABLE" is lesser and different.** ⚠️ **No data is lost and no wrong world is bound — ✅ the writer sees a world that is genuinely theirs, at a state they did not choose.** | ✅ **[EP-044]** `[ScriviCore]` **World Resolution** — ⚠️ **assigned 2026-09-22; ⛔ its ACs MUST BE REWRITTEN (the originals are void, above).** ✅ **REWRITTEN 2026-09-25 in [`../Epics/Epic-EP-044.md`](../Epics/Epic-EP-044.md): AC1 = a world package can be distinguished from another VERSION of itself (schema + binding records the bound version); AC2 = a version mismatch is RECONCILED HONESTLY with its DIRECTION, and a COPY is NOT a mismatch; AC2b = migration for worlds predating the field.** ⛔ **The void originals are kept at the foot of that Epic's AC section** | 🔵 **Open** — ⚠️ **REWRITTEN 2026-09-25 (user ruling); ⛔ NOT SCHEDULED** |
|  **I-0244** | `[Linux]` ⚠️ **THE LINUX APP HAS NONE OF [EP-040]'s EDITOR SHELL, AND NOTHING TRACKS THE GAP.** ⚠️ **RAISED 2026-09-22 from the user's own observation during [SP-135]'s live pass:** ⚠️ ***"I did not toggle the panels on Linux or any of the more recent UI changes because many of them have not been made on Linux yet."*** ✅ **MEASURED THE SAME DAY, not asserted:** ⛔ **Linux has ZERO `QToolBar`/`addToolBar` in `platforms/linux/src/`** · ⛔ **ZERO navigator-visibility state** · ⚠️ **Apple now has a `.toolbar` with 3 `ControlGroup`s and SIX `safeAreaBar`s.** ⚠️ **THIS IS NOT A DEFECT IN EITHER PLATFORM.** ✅ **[EP-040] is scoped `[Apple]` DELIBERATELY** — ⛔ **but four Sprints of shell work ([SP-134] toolbar · [SP-135] bars · [SP-136] inspector column · [SP-137] detail sheet) have no Linux counterpart, and NO Epic, Sprint or Issue records that.** ⚠️ **`feedback_linux_adopts_apple_shape` is the standing rule: *Apple is the reference for architecture; a shape change on Apple must be made the same way on Linux in the SAME work.*** ⛔ **That has not happened, and the divergence is now FOUR SPRINTS DEEP AND GROWING.** ✅ **WHAT THE APPLE WORK ALREADY PROVED, and Linux will re-earn without it:** ⚠️ **[I-0203]'s root cause was that bars were STACK SIBLINGS** — ✅ **Linux's `EditorShell` uses the same layout idiom** — ⚠️ **and the Navigator was unrecoverable because NOTHING OWNED ITS VISIBILITY STATE.** ⛔ **Linux's pane visibility is SESSION-SCOPED ONLY (SP-078/T-0320), which is the same class of gap.** ⚠️ **IT IS NOT [EP-043]'s EITHER:** ✅ **that Epic is the SESSION (multi-window, restore, geometry)** — ⛔ **this is the SHELL (toolbar, bars, panes).** ✅ **They are adjacent and both wait on the same `EditorShell` rework, which is an argument for SEQUENCING them together, not for merging them.** ⚠️ **NOT URGENT AND NOT A REGRESSION** — ✅ **Linux works as it did** — ⛔ **but an untracked divergence is how a port silently falls behind, which is exactly what the Porting Outline exists to prevent.** ---- ⚠️ **A PERFORMANCE EXPECTATION IS INHERITED FROM [I-0213]** (⚠️ *`[Apple]` chapter create froze the app ~2.7 s on a 1,174-scene manuscript; now `~305–400 ms` of work*) **AND IS RECORDED HERE SO LINUX DOES NOT RE-DISCOVER IT AS A DEFECT.** ✅ **[I-0213] was VERIFIED on Apple 2026-09-24 with `createChapter WORK=396.7 ms` on 1,179 scenes, ⚠️ measured DELIBERATELY from a USB mount as a WORST-CASE FLOOR.** ⚠️ **The user named what follows:** *"On Linux it will be even worse because it has to also come through this computer's operating system."* ✅ **SO A LINUX FIGURE WORSE THAN `396.7 ms` ON THE SAME FIXTURE IS ANTICIPATED — ⛔ NOT a regression against [I-0213], and NOT a new Issue on its own.** ⛔ **NOTHING HAS BEEN MEASURED ON LINUX YET; ✅ this is an EXPECTATION TO TEST AGAINST, not a claim about the rig.** ⚠️ **When the Linux editor shell is built, the structural ops MUST be timed with the SAME `[SCRIVI-STRUCT]`-style instrumentation** — ✅ **[I-0213] proved the cost was UNATTRIBUTABLE until each op reported its own number, and that three code-read diagnoses were wrong before it did.** | **Low** | ⛔ **UNASSIGNED — awaiting its OWN Epic.** ✅ **SCOPE RULED 2026-09-26 ([EP-043] [R-Q5], user-approved): it becomes its OWN `[Linux]` Epic, SEQUENCED AFTER [EP-043]** — ⛔ **NOT ACs inside it.** ⚠️ **THIS ROW PREVIOUSLY SAID *"sequenced WITH or after"*; ✅ it is AFTER, and [EP-043]'s record now says the same** — ⛔ **the two records disagreed and no longer do.** ✅ **WHY NOT FOLDED IN: this Issue is FOUR Apple Sprints deep ([SP-134]–[SP-137]), and folding it into a SESSION Epic would repeat the [EP-035] AC1 collapse [I-0178] cites.** ✅ **THE COLLISION IS ALREADY HANDLED: [SP-145] carries a NAMED CARVE-OUT** — ⚠️ **its extraction takes `inspectorVisible_` + `timelineVisible_` onto the session object** (⛔ **per-project state, not per-widget**), ✅ **so this Epic will NOT have to re-open `EditorShell` for pane state.** ⚠️ **Of this Issue's THREE measured gaps, ONLY pane-visibility overlapped, and that is the carve-out** | 🔵 **Open**  |
| **I-0192** | `[ScriviCore]` ⚠️ **A world on a PHYSICALLY REMOVED volume resolved `available`, and a full object read SUCCEEDED through it, until a scene change forced a re-resolve.** ⚠️ **Found by the USER on the REAL RIG during T-0477's S3 physical yank, 2026-09-07.** ⚠️ **RE-SCOPED 2026-09-07 from `[Linux]` to `[ScriviCore]` after reading the code — ⚠️ MY ORIGINAL DIAGNOSIS WAS WRONG.** ⚠️ **I filed this as an app-layer 'cached status with no invalidation' and as a placeholder dialog reusing a name it already held. ✅ **The code says otherwise, and the truth is WORSE:** `EditorShell::onOpenObjectRequested` (`EditorShell.cpp:~1790`) calls `bridge_->openObject(...)`, checks `lastCallFailed()`, and ⚠️ **parses the displayed name out of the `objectJson` THAT CALL RETURNED** — its own comment reads *"the object is genuinely read here."* ⚠️ **There is no app-side status cache to go stale.** ✅ **The read is guarded end-to-end**: `ObjectStore::open` → `findByID` → `kindDirFor` (`ObjectStore.cpp:~250`), which calls `WorldStore::resolve()` and ⚠️ **refuses unless status is `available`.** ✅ **And `resolve` caches NO verdict** — it sets `available` only after actually reading and parsing `world.json` from the candidate path (`WorldStore.cpp:337-342`). ⚠️ **SO THE DEFECT IS NOT STALENESS ANYWHERE — the filesystem itself answered successfully for a volume that was physically gone**, and every layer above correctly trusted a correct answer. ⚠️ **The likely mechanism is the PAGE CACHE / unreaped dentries**, which ✅ **the 2b container pass ALREADY measured in a stronger form**: a held FD survived `umount -l` + `losetup -D` entirely, reading and writing fine while the PATH broke instantly. ⚠️ **THIS IS I-0181's SIBLING, NOT ITS OPPOSITE**: I-0181 is `resolve` inferring absence it cannot prove; ⚠️ **I-0192 is `resolve` inferring PRESENCE it cannot prove** — ✅ **and `read succeeded` is no more proof of a mounted volume than `directory exists` is proof of a deleted one.** ⚠️ **T-0498's `st_dev` primitive is plausibly the SAME fix for both** — ⚠️ **but that must be MEASURED, not assumed.** ⚠️ **PARTIALLY MEASURED 2026-09-07 (over SSH, drive already out): the STEADY STATE is CORRECT** — `scrivi_get_world_status` returns ✅ **`unavailable`** (not `missing`), with `packagePath` empty and `lastKnownPackagePath` preserved, ⚠️ **stable across repeated calls**, and ✅ **`binding.json` stores no status key at all** — ⚠️ **so there is no persisted verdict to go stale.** ⚠️ **Therefore I-0192 is a TRANSIENT WINDOW, not a stuck verdict** — ⚠️ **severity lowered accordingly.** ⚠️ **The decay-vs-persist question across the yank itself is STILL OPEN and needs the sampled run.** | **Low** | ✅ **[EP-044]** `[ScriviCore]` **World Resolution** — ⚠️ **assigned 2026-09-22 by user ruling.** ✅ **AC3.** ⛔ **Its former candidate home, T-0498, is CLOSED and VERIFIED** — ⚠️ **so this had no owner at all; ✅ [EP-044] carries T-0498's unfinished work by design** | 🔵 **Open** |

## I-0285

**Title:** Opening a large project on Linux takes ~3 minutes and freezes the UI for ~55 s — the timeline is built twice on the main thread
**Status:** 🟡 In Progress (SP-173)
**Platform:** `[Linux]`
**Component:** `EditorShell` (project load) · `ScriviBridge` (per-scene story-time calls)
**Severity:** High
**Epic:** [EP-048] (found there; not caused by it) · **Sprint:** 🟡 [SP-173] → [`../Sprints/Sprint-SP-173.md`](../Sprints/Sprint-SP-173.md)
**Date Identified:** 2026-10-09 (SP-166 live pass; user: *"note the time it takes Linux to load this"*)

**Description:** dumas-prose-timelines (1,186 scenes, 1.8 MB), opened on the rig from the network share, takes ~172 s from
launch to an idle editor. For the last ~59 s the UI is frozen: the main thread builds the timeline twice.

**Measured** (build 62, `SCRIVI_LOAD_LOG=1`; run A with the presenter, run B with `SCRIVI_NO_PRESENTER=1`):

| Phase | A | B | Thread |
| ----- | -: | -: | ------ |
| Core `openProject` | 55.6 s | 59.8 s | worker |
| 1,186 scene bodies (`openSceneForBulkLoad`, one call each, ~48 ms) | 57.4 s | 57.3 s | worker (progress bar) |
| `SceneDocument::build` + `setDocument` | 0.04 s | 0.05 s | main |
| `rebuildNavigator()` (its own work is cheap — see root cause) | 28.1 s | 27.9 s | main, UI frozen |
| `reloadTimeline()` at the end of the load | 27.4 s | 27.9 s | main, UI frozen |
| Event loop idle (presenter's deferred first highlight) | 3.4 s | 0.02 s | main |
| **Total** | **172 s** | **173 s** | |

**Root Cause Analysis:** `rebuildNavigator()` ends by calling `reloadTimeline()`. `reloadTimeline()` returns early while
`loading_` is set ("load() calls this once at the end"), but `applyLoadedProject` sets `loading_ = false` **before** calling
`rebuildNavigator()`, so the guard never fires and the timeline is built twice. Each build calls `getSceneStoryTime` once per
scene, and again per scene when a story structure is present (`reloadTimeline`, the dot loop and the band loop): up to
2 × 1,186 core calls per build, every one on the main thread, against the share.

**Expected:** the timeline is built once, its data is fetched off the main thread, and the UI never freezes while a project opens.

**Acceptance Criteria** (user, 2026-10-09):
- [ ] **AC1 — Built once:** the timeline is not rebuilt during the load.
- [ ] **AC2 — EP-048 must not make it worse:** every EP-048 Sprint that touches the Linux load records the `SCRIVI_LOAD_LOG`
  phases on dumas over the share, before and after. The presenter already adds ~3.4 s of main-thread work after the load
  (Qt defers a highlighter's first pass to the event loop); that figure is the baseline to beat, not to grow.
- [ ] **AC3 — Everything we can to minimise load time:** each phase measured and either reduced or ruled irreducible:
  the core open (56 s), the per-scene body reads (57 s, one ABI call per scene), the per-scene story-time calls, the
  navigator, the presenter's first highlight, layout.
- [ ] **AC4 — Off the main thread:** the timeline's data (`getTimeline`, per-scene story time, historical events, story
  structure) is fetched in the load's existing worker; the main thread only builds widgets. If any part cannot move, the
  reason is recorded.
- [ ] **AC5 — On the progress bar:** whatever moves into the worker is counted in the determinate progress bar, as scene
  bodies already are.
- [ ] Apple's shape is checked first (`feedback_linux_adopts_apple_shape`): how Apple loads timeline data at open.

**Related (not duplicates):** [I-0195] (the open froze the UI → moved to a worker), [I-0196] (`scrivi_open_scene` resolved the
whole manuscript per scene), [I-0231]/[I-0232] (slow open, tintagael). The load-phase log and `SCRIVI_NO_PRESENTER` are in
`EditorShell.cpp` (build 62) as measurement only.


✅ **[I-0221] and [I-0222] were filed, fixed AND user-verified 2026-09-17/18**, then archived to
[`Verified/Issue-verified-0221-0230.md`](Verified/Issue-verified-0221-0230.md) **in the same step**
(`feedback_archive_on_close`). ⚠️ **Both came from ONE live pass on a FAT32 volume, and neither was
reachable by the test suite as it stood.**

✅ **I-0191 moved to `Issue-active.md` 2026-09-07** — fixed the same day it was filed; ✅ **user-verified 2026-10-03** → `Verified/Issue-verified-0191-0200.md`.


---

*Last Updated: 2026-10-08 — **I-0284 filed** ([SP-171] AC1 corpus test): a hard break in a paragraph switches off its emphasis rendering.*

*Last Updated: 2026-10-08 — **I-0277 → active** ([SP-171] activated; it left this file).*

*Last Updated: 2026-10-08 — **I-0283 → active** (ruled (a), fixed now; it left this file).*

*Last Updated: 2026-10-08 — **I-0283 filed** (user, reproduced): macOS Smart Quotes rewrite stored text outside the escape layer; ruling owed.*

*Last Updated: 2026-10-07 — **I-0281 filed** ([SP-165] L2): a first line indented 1–3 spaces shifts Apple's emphasis positions on the following lines.*

*Last Updated: 2026-10-05 — **I-0278 filed** (user): Title, Subtitle, Show chapter titles live in `UserDefaults` and do not travel with the project.*

*Last Updated: 2026-10-05 — **I-0277 filed** (user): VoiceOver reads the stored text (escapes, hidden `##`), found in SP-161 step 1.*

*Last Updated: 2026-10-03 — **stale I-0223 row and section DELETED (user)**: they carried the framing from before the 2026-09-25 rewrite (Medium, "resolve to a wrong location"), which contradicted the live record in `Issue-active.md` (Low, rewritten, [EP-044]). ✅ **I-0147 moved here** (accepted limitation). Prior note follows.*

*Last Updated: 2026-09-18 — **[I-0221] and [I-0222] user-VERIFIED and archived** to
`Verified/Issue-verified-0221-0230.md`; ⚠️ **the backlog is empty again.** ✅ **The drive-pull test that
produced I-0222 is complete.** Prior note follows.*

*2026-09-17 — **two Issues filed from ONE live pass** on a FAT32 USB volume.
✅ **[I-0221]** filed AND fixed the same day: AppleDouble `._*` sidecars
aborted project open, ⚠️ **and the sandbox's `com.apple.quarantine` stamp REGENERATED them on every
write**, so the failure recurred after any manual clean. ✅ **Fixed at the `listDirectory` chokepoint**
after the audit found ~20 scan callers, not the 3 first identified. ✅ **[I-0222]** filed AND fixed the same day:
pulling the volume reported `ScriviError 1` instead of naming the world. ✅ **`ScriviError` now conforms
to `LocalizedError`** (~15 call sites had been showing Foundation's type-name fallback) ✅ **and the two
`worldPending:`/`worldUnavailable:` spellings merged to ONE derived constant on the user's ruling.**
⚠️ **The cards emptying was ruled ACCEPTED, not a defect** — an away world's objects genuinely are not
available. ⛔ **Its first diagnosis was WRONG and the correction is kept in the record**: measurement
showed neither call in the card's load path fails at all. ✅ **The underlying pending behaviour was
CORRECT throughout: the manuscript stayed usable with the volume gone and everything restored on
reattach.**
⚠️ **Both Issues were invisible to a green suite** — fixtures build on APFS and hand-construct their
`detail` strings. Prior note follows.*

*Last Updated: 2026-09-07 — **I-0191 opened** on the user's report of an unexplained folder in the
repo root. ⚠️ **Three garbage-named, EMPTY app-support trees** (dated 2026-08-17) were deleted; ⚠️ **the
mechanism that creates them was NOT fixed.** ✅ **Root cause found by reading the code and REPRODUCED**:
`bootstrapAppSupport` validates `appSupportRoot` in no way, `AbsolutePath` is a bare `std::string`, and
the C ABI's `S()` turns NULL into `""`. ✅ **Test-suite audit (user-requested) came back CLEAN** — every
fixture cleans up via RAII and roots at an absolute `temp_directory_path()`, so ⚠️ **the suite is not the
source**; its real gap is that it can only see its OWN temp dir (AC4). ⚠️ **The exact call site that made
these three folders is NOT identified — recorded as an open question, not guessed.** Prior note follows.*

*Last Updated: 2026-08-20 Removed  references to I-0118 which is verified and does not belong here.  

2026-08-17, later same day (*\*I-0017 ✅ Verified and archived; I-0018 partly fixed and
RESCOPED\*\* — both on the user's report while reviewing this backlog. I-0017 had been fixed and confirmed long
ago and was never filed. I-0018's original complaint (no selection shown on load) is **fixed**; the remaining
behaviour — **the manuscript not scrolling to that selection** — is different from what was reported, so the
Issue is retitled and rescoped rather than left implying the whole thing is broken. ⚠️ It is flagged to be
scoped **together with I-0131 and I-0132**, which are the same underlying question — \*what does it mean to
"be at" a scene?\* — across load, click, and quit. **Backlog is now 1.** Prior note follows.)\*





---

## ✅ I-0231 / I-0232 — the `the-stairs-of-tintagael` slow-open pair — **CLOSED 2026-09-20**

✅ **Both VERIFIED on the real rig under `cache=none` and ARCHIVED** →
[`Verified/Issue-verified-0231-0240.md`](Verified/Issue-verified-0231-0240.md).
✅ **[I-0195] verified in the same pass** →
[`Verified/Issue-verified-0191-0200.md`](Verified/Issue-verified-0191-0200.md).

⚠️ **The working narrative that lived here — the `/proc` sampling, the D/S thread
split, the ordering argument — is preserved in
[`../Sprints/Sprint-SP-144.md`](../Sprints/Sprint-SP-144.md)**, ✅ **which is
where a closed investigation belongs.** ⛔ **It is not repeated here: this file
holds OPEN Issues, and a resolved narrative left in it reads as live work.**

✅ **Outcome, measured by the user on 🐧 `Oathkeeper`:** ⚠️ **24.01 s → 13.46 s**,
⚠️ **5,002 → 2,999 syscalls**, ⚠️ **`binding.json` 188 → 2 opens.**
⚠️ **The pair also produced [I-0233], [I-0234] and [I-0235]** — ✅ **all three
verified and archived alongside them.**

## ⛔ NO COUNT IS STATED HERE — ✅ **audit rulings [R-08] / [R-14] / [R-15], 2026-09-15**

⚠️ **THIS FILE CARRIED SIX STACKED *"Currently: N records"* LINES** (twenty-three, twenty-two,
twenty-one, twenty, eighteen, seventeen) — ⛔ **and NONE of them was correct; the actual row count was
25.** ✅ **They were DELETED, not preserved under a dated heading**, ⚠️ **because [R-14] rules that
preserved text stating STATE is as destructive as omitting it: a reader cannot tell which line is
current, so preservation actively misleads.**

✅ **TO COUNT THE OPEN ISSUES: read the rows below.** ⚠️ **Preserving RATIONALE remains correct;
⛔ preserving a COUNT does not.**







✅ **I-0193 VERIFIED 2026-09-10 (user-approved) and ARCHIVED** → [`Verified/Issue-verified-0191-0200.md`](Verified/Issue-verified-0191-0200.md) — ⚠️ **a NEW DECADE FILE.** ⚠️ **The header count also read `thirteen` against TWELVE rows before this edit** — ✅ **corrected to `eleven`, which matches the table and the enumeration.**

⚠️ **I-0193 and I-0194 were filed 2026-09-08 from T-0478's LIVE PASS on the real rig** — ✅ **both against SP-124.** ⚠️ **Neither was findable from the suite**: I-0193 needs a real blocking mount to show its 102 s, and I-0194 appears ONLY on the offline resolution path. ⚠️ **I-0195 was filed the same day from the SAME root architectural gap** — ✅ **no async path for world/project reads.** ⚠️ **I-0193 was the UNREACHABLE case (needed a timeout); I-0195 is the REACHABLE-BUT-SLOW case (needs progress).** ⚠️ **Fixing either without getting the read off the UI thread fixes neither.** ✅ **I-0193 is now VERIFIED and ARCHIVED — its `AsyncCall` machinery (`platforms/linux/src/AsyncCall.hpp`) is the OFF-THE-UI-THREAD PREREQUISITE I-0195 shares**, ⚠️ **so I-0195 should BUILD ON IT rather than re-derive it** — ⚠️ **but I-0195 remains OPEN: a timeout is not progress, and the determinate `files read / files to read` percentage the user specified is still unbuilt.**

✅ **I-0193 IS CLOSED — VERIFIED 2026-09-10 and archived.** ⚠️ **It took TWO passes.** ✅ **Build 34 (2026-09-09) fixed the scene-click path and UNBLOCKED T-0478's DoD item** — the writer-facing string was READ: *"This scene's objects are taking longer than expected to read — a world may be on a disconnected or unreachable volume. Nothing has been lost."* ✅ **It says `may be` and `Nothing has been lost` — the I-0115 discipline held: a wrong-but-confident status is what invites a writer to reach for destructive remedies.** ⚠️ **But that fix left the SECOND call site the Issue had NAMED FROM THE START untouched**, so `Project > Manage Worlds` still froze ~102 s to Force Quit. ✅ **`571fac1` closed it: `WorldsDialog::reload()` is async, and `EditorShell::writerFacingError()` no longer calls the core to name a world.** ✅ **User-verified on the real rig: the dialog OPENS with the share down.**

⚠️ **I-0192 came out of T-0477's S3 physical yank (2026-09-07) — and the INSTRUMENTATION DID NOT FIND IT.** ⚠️ **The probe was run on the WRONG MACHINE because the runbook never said which machine each command belonged to**, so it watched a Linux path on the MacBook and produced noise. ✅ **The user found the defect by PULLING THE DRIVE AND WATCHING THE APP.**

⚠️ **MY FIRST DIAGNOSIS OF I-0192 WAS WRONG AND IS WITHDRAWN (2026-09-07).** ⚠️ **I filed it as an APP-LAYER cached status, and repeated the user's reasonable suggestion that the placeholder dialog was merely reusing a name it already held.** ✅ **Reading the code disproved both**: the double-click performs a genuine `openObject` through the ABI and parses the name from the returned `objectJson`; ⚠️ **there is no app-side status cache**, and ✅ **`WorldStore::resolve` caches no verdict either** — it returns `available` only after reading and parsing `world.json`. ⚠️ **So the FILESYSTEM answered successfully for a volume that was physically gone, and every layer above correctly trusted a correct answer.** ⚠️ **Re-scoped `[Linux]` → `[ScriviCore]`.**

⚠️ **I also claimed "no probe in §4's table would have caught it." ✅ THAT WAS WRONG TOO** — ⚠️ **§4's held-FD probe questions exactly this**, and ✅ **2b had ALREADY measured the stronger form** (a held FD outliving `umount -l` + `losetup -D` entirely). ⚠️ **The finding was reachable by instrumentation; the instrumentation was simply pointed at the wrong machine.** ⚠️ **What is TRUE is narrower and still worth keeping: the OS was honest throughout, so no probe that questions MOUNT STATE would have flagged it** — ✅ **`feedback_live_pass_finds_what_suites_cannot` still applies, but as a claim about which QUESTION was asked, not about instrumentation being useless.**

### ⚠️ I-0192 — ✅ **HOW TO SETTLE IT** (the probe run)

⚠️ **The corrected diagnosis narrows the question; it does NOT answer it.** ⚠️ **Do not fold this into
T-0498 until it is MEASURED** — ⚠️ **fixing `resolve` from a reading of the code is exactly what I-0181's
history warns against** (three narrowings of one block, each made from inference).

✅ **The instrument now exists and builds on the rig**: `scrivi_world_probe`
(`ScriviCore/tools/scrivi_world_probe.cpp`, ⚠️ **Qt-free**, `-DSCRIVI_BUILD_TOOLS=ON` by default).
⚠️ **It did not exist when S3 ran, which is why S3 could not answer this.**

⚠️ **The run — 🐧 `oathkeeper`, one drive pull:**

| Phase | Command | ⚠️ What it settles |
| ----- | ------- | ------------------ |
| **BEFORE** | `scrivi_world_probe <project>` | The healthy baseline envelope |
| ⚠️ **IMMEDIATELY AFTER the yank** | ⚠️ **the same command, REPEATED every ~2 s** | ⚠️ **Does `resolve` still say `available` — and for HOW LONG?** |
| **AFTER a scene change** | same command | ⚠️ **Confirms the flip the user saw, from the ABI rather than from the screen** |

⚠️ **The decisive row is the second, and it must be SAMPLED, not snapshotted** — ✅ **the same reason
`volume-loss-probe.sh` streams:** a single reading cannot distinguish a decaying success from a
persistent one.

| ⚠️ If the probe shows… | ✅ Then |
| ---------------------- | ------- |
| `available` ⚠️ **DECAYING** to `unavailable` on its own | ✅ **The core is CORRECT** — the page cache was answering, and the only defect is that nothing re-asks promptly. ⚠️ **NOT T-0498's** |
| `available` ⚠️ **PERSISTING** indefinitely | ⚠️ **`resolve` is asserting PRESENCE it cannot prove** — ✅ **T-0498's `st_dev` check is then the fix for BOTH directions**, and I-0192 folds into it |

⚠️ **A successful `read` is NOT evidence of a mounted volume**, any more than ✅ **a surviving directory is
evidence of a deleted world** (I-0181). ⚠️ **Same wrong question, opposite sign.**

✅ **I-0191 was filed and fixed on 2026-09-07, the same day the user spotted the folder.** ⚠️ **It is the first Issue in this project found by the user noticing an ARTEFACT rather than a behaviour** — the app never misbehaved visibly; three empty directories simply sat in the repo root for three weeks. ⚠️ **Two of its seven ACs resolved differently than the Issue assumed**: the ⚠️ **test suite was cleared** (audited clean — RAII cleanup, absolute temp roots, no `chdir`), and ⚠️ **AC6's dangling-pointer suspicion did NOT survive the audit** — the `constData()` pattern is safe as written, so it was ✅ **RULED and DOCUMENTED rather than rewritten across 265 lines.** ⚠️ **Compare `feedback_evidence_before_attribution`: the suspected culprit was not the culprit, and checking beat assuming.**

✅ **I-0183, I-0184, I-0185 and I-0186 VERIFIED 2026-09-02 (user-approved) and archived** in the same
step → [`Verified/Issue-verified-0181-0190.md`](Verified/Issue-verified-0181-0190.md), ⚠️ **which OPENS
a new decade file.**

⚠️ **ALL FOUR came from ONE live pass (T-0496, SP-127), and NONE was caught by a suite.** ⚠️ **Two of
the four needed the USER TO CORRECT MY DIAGNOSIS before the real defect came into view** — I-0184
(I blamed a 360 px constant; the cause was the row layout **clipping**) and I-0185 (⚠️ **my first fix
made descending RECOVERABLE when the requirement was that it be IMPOSSIBLE** — and it passed every
test I had written for it). ⚠️ **Compare `feedback_live_pass_finds_what_suites_cannot` and
`feedback_prove_code_is_reached`: a green suite never means usable, and a test written from a wrong
diagnosis certifies the wrong thing.

✅ **I-0184 and I-0186 VERIFIED 2026-09-02 (user-approved) and archived** in the same step →
[`Verified/Issue-verified-0181-0190.md`](Verified/Issue-verified-0181-0190.md), ⚠️ **which OPENS a new
decade file.**

⚠️ **I-0186's root cause was a TESTING BLIND SPOT, not the code alone.** ⚠️ **Qt's no-theme fallback
made every offscreen check — and a screenshot produced as evidence — show a readable path that no real
user ever saw.** ✅ **The user found it by looking at their own screen.** ⚠️ **Compare
`feedback_live_pass_finds_what_suites_cannot`: a green suite never means usable, and this one was
green *because* it was headless.**

⚠️ **I-0183 is the most serious Issue in this file: it is DATA LOSS, and it was found by a LIVE PASS doing exactly what the sprint's own risk table said to test** — ✅ *"Relink accepting the wrong package → the CORE verifies `worldID`"* — ⚠️ **the mitigation was written, implemented, and is INSUFFICIENT, because a copy shares the `worldID`.** ⚠️ **A green suite never showed this** (`feedback_live_pass_finds_what_suites_cannot`).

⚠️ **I-0181 opens the new decade** and is ⚠️ **the first Issue in this project found by INSTRUMENTATION
rather than by use or by a suite.** ✅ **It was found BEFORE the surface that would have shown it was
written** — which is what *instrument-before-implement* is for.

⚠️ **It was RE-SCOPED TWICE in one day, both times by user ruling** — `[Linux]` → `[Cross]` →
⚠️ **`[ScriviCore]`.** ⚠️ **My "macOS is immune" claim did not survive a hand-specified mountpoint**;
⚠️ **then my framing as a REPORTING defect did not survive the observation that Linux has no world
surface to report through at all.** ✅ **"The app won't incorrectly represent the mount point until it
can correctly represent the mount point"** — ⚠️ **so this is a LATENT CORE defect, not a live one**,
and ⚠️ **it is NOT SP-124's to fix.**

⚠️ **NOT fixed**, and ⚠️ **must not be fixed from container evidence**: the container establishes the
CLEAN unmount case, and ⚠️ **the physical-yank case may differ.**

✅ **I-0179 VERIFIED 2026-08-30 and archived** → [`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md), ⚠️ **which CLOSES that decade file.**
⚠️ **The next Issue is I-0187.**

⚠️ **I-0180 is an APPLE defect found by reviewing the LINUX mirror.** ✅ **That is the port paying a
dividend back**: building the same surface a second time exposed a wrong label that had been shipping
on macOS since EP-031 unnoticed. ⚠️ **Worth remembering when the remaining four ports run.**

⚠️ **I-0179 was found by the user in SP-126's live pass**, in a message I had *just* rewritten to be
writer-facing — ✅ **the wording was right and the quoted string was wrong.** ⚠️ **Lesson: a row's
visible text is a PRESENTATION.** Recovering data by parsing it back apart works until the
presentation changes, and here it never worked at all.

⚠️ **All three were found by the USER on the REAL RIG (T-0476, 2026-08-29)** — ⚠️ **the first time the
Linux app had ever run on real hardware**, and ⚠️ **none of them was findable by any suite**: they are
about what survives a QUIT, which no test exercises.

✅ **They are ONE gap with three symptoms, not three bugs.** ⚠️ **I-0178 (multi-project) is the parent** —
Apple solved all three together in **EP-018**, whose per-window `ProjectSession` + `OpenProjectRegistry`
is what "restore what was open" and "restore geometry" both hang from. ⚠️ **A Linux equivalent is a
STRUCTURAL rework of `ScriviWindow`/`EditorShell`, and wants its OWN Epic.**

⚠️ **The user ruled these do NOT block T-0476's verification** — they are gaps in scope never claimed,
not failures of what was built.

✅ **I-0171 VERIFIED 2026-08-29 and archived** → [`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md).
⚠️ **It was fixed by SP-125 but OWNED by SP-122** — verified in the same step SP-125 closed.

✅ **SP-125's three Issues were settled 2026-08-28 in the same step its five Tasks were verified**
(`feedback_archive_on_close`):

- ✅ **I-0173** (elided relationship labels) — **Verified** → [`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md).
  ⚠️ **Found by the LIVE PASS; all 571 ctests and 23 smoke checks were green with it present.**
- ✅ **I-0175** (a synthetic-input driver typed into a real manuscript) — **Verified**, same file.
  ⚠️ **My process defect, not the app's**; repaired byte-for-byte.
- ⚠️ **I-0174 CLOSED as NOT A DEFECT** → [`Closed/Issue-closed-0174.md`](Closed/Issue-closed-0174.md).
  ⚠️ **My diagnosis was wrong and the user corrected it**: the "unexplained" cache write was a second
  project's characters propagating through a **shared world**. ✅ **Opening a project is not a risk.**

✅ **I-0172 was Verified 2026-08-25 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md), which **opens a new
decade file** (the previous closed at I-0170).

⚠️ **I-0172 was verified by COMPILATION plus user approval, not by exercising the popover** — the fork
popover appears only when redoing into a branch point, which SP-122 never hit. ⚠️ **If a sizing
regression appears in that popover, I-0172's change is the first thing to suspect.**

⚠️ **I-0171 was opened 2026-08-25 by SP-122's T-0468** and is the first Issue of the new decade.
⚠️ **It was found by RUNNING the Linux leg, not by reading the `.dockerignore`** — SP-121 added that file
and its own sprint never re-ran a cached container build against a second build directory.

✅ **I-0169 + I-0170 were Verified 2026-08-24 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0161-0170.md`](Verified/Issue-verified-0161-0170.md), which that pair **closes**.
⚠️ **The next Issue is I-0171 and opens a new decade file.**

⚠️ **Both came from SP-120's live click-through; neither from any suite** — which now holds for **22
consecutive Issues** across SP-118, SP-119 and SP-120. ⚠️ **I-0169 was the writer's FIRST instinct**
(the sources card had no route to the Detail Sheet, using a hook that already existed and was never
called); ⚠️ **I-0170 was a surface quietly UNDER-REPORTING the graph** — every field present and
populated, and still not true.

✅ **I-0162 – I-0168 were Verified 2026-08-24 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0161-0170.md`](Verified/Issue-verified-0161-0170.md).

⚠️ **All seven came from SP-119's live click-through. None was found by any suite.** ⚠️ **Six were
data-loss routes into a single surface** — the Object Detail Sheet — reachable by ejecting a drive or
navigating away at six different moments.

**What the table cannot express:**

- ⚠️ **I-0161 took THREE attempts and is the sprint's clearest lesson in diagnosis order.** Attempt 1
  scrolled at click time (wrong: raced the highlight). Attempt 2 fixed that correctly but ⚠️ **was never
  compiled into the macOS build** — the edit reached one of two platform call sites. ⚠️ **Claude spent a
  round explaining the behaviour of code that did not run**, exactly as I-0151 was caused by a comment
  asserting behaviour never checked against the source.
- ⚠️ **THE RULE: prove the new code is REACHED before explaining why it behaves oddly.** One log line, or
  one grep for call sites, would have replaced a whole round of theory. ⚠️ **"It didn't change anything"
  should first be read as "it isn't running", not as "it ran and was wrong."**
- ✅ **Temporary `SCRIVI-DIAG` logging is what settled it** — and was removed once it had. Instrumenting a
  path is cheaper than a third hypothesis.
- ⚠️ **I-0158/I-0159 are one mistake with two faces: I hand-rolled a list.** A `VStack`/`ForEach` meant
  reimplementing selection, the highlight and right-click targeting — each attempt wrong in a new way —
  and ⚠️ **`SceneNavigatorView` was already doing it correctly with `List(selection:)` in the same
  directory.** Switching to `List` fixed selection and broke layout; the answer was to take the selection
  semantics and keep the app's existing scroll structure. ⚠️ **The user's question — "a Swift standard
  List View handles all this automatically… which makes me wonder why it is so hard for you" — is the
  right one**, and the answer is that I built new machinery instead of looking at what the app already had.
- ⚠️ **THREE defects this sprint were "an existing correct pattern the new code did not follow"**: I-0155
  (`ObjectCardModel.rename` re-read before patching), I-0157 (I-0132 ruled selection the source of truth),
  I-0158 (`SceneNavigatorView` already used `List(selection:)`). ⚠️ **All three rules were written down,
  in this repo, before the code that violated them was typed.**
- ⚠️ **I-0155 is the most serious defect of the sprint, and it was reported as a hedge.** The user wrote
  *"It isn't necessarily a defect. More like an unintended consequence… Maybe there is a defect here after
  all."* ⚠️ **It was silent data loss** — a saved note reverting a saved rename. **The uncertainty in a
  report is not a measure of its severity**, which is the same lesson as I-0148 and I-0154, now three
  times in this Epic.
- ⚠️ **I-0155 and I-0157 share a shape: an existing correct pattern that the new surface did not follow.**
  `ObjectCardModel.rename` already re-read before patching; I-0132 already ruled selection the source of
  truth. ⚠️ **Both rules were written down, both were violated by code added days later.** Grepping for
  "how does the app already do this?" would have caught both — the same discipline as the
  derive-never-restate rule, applied to behaviour instead of to lists.
- ⚠️ **I-0151–I-0154 were ALL found by the SP-118 live click-through**, and none by any suite. ⚠️ **The
  green run had asserted edge creation, duplicate rejection, both-endpoint visibility and pending
  presentation** — every one of which held up. **What no test covered was whether a writer could reach any
  of it**, which is `capability_without_surface` for the third time in this Epic.
- ⚠️ **I-0151's cause was a COMMENT ASSERTING A FALSEHOOD.** I wrote *"`openObject` accepts '' and resolves
  it"* next to the line that passed `""`, and never opened `ObjectStore.cpp` to check. ⚠️ **A confident
  comment is not evidence**, and writing one is how an unchecked assumption gets laundered into an
  apparent finding — the same failure as I-0150's misattribution, in a different medium.
- ⚠️ **I-0152 is the one Claude got wrong twice.** Told the writer saw a raw ID, Claude confirmed the empty
  title and concluded *"not a display bug"* — answering **why the data was empty** instead of **what the
  writer was shown**. The user's correction was the point: the Navigator already solved this, so two
  surfaces disagreed about one scene's name and the worse answer won.
- ⚠️ **"The Lantern Foxes" is NOT a defect** — checked and closed. The stored edge is
  `chronicle --appears-in--> scene`, so *"appears in"* from the chronicle's end and *"features"* from the
  scene's end are **the same edge read from opposite endpoints** (Doc 1 §5.2), and `ObjectCard` passes
  `label: edge.label` straight through without recomputing. ✅ **Both displays are correct.**
- ⚠️ **I-0150 was found by the user REFUSING A PLAUSIBLE STORY.** Claude read a timestamp, concluded
  *"you reopened Scrivi"*, and wrote a detailed accounting on that basis. ⚠️ **The user simply said he had
  not** — and the real cause was Claude's own test command. ⚠️ **The failure mode was reaching for the
  explanation that did not implicate my own actions**, and the evidence was in a file I had already been
  told to update (`TEST_HOST` in `project.pbxproj`).
- ⚠️ **I-0150 changes what "safe to test" means on this project.** `xcodebuild test` is **not** a read-only
  operation: it is an app launch with full access to the writer's real projects through saved bookmarks.
  ⚠️ **There is deliberately NO test that flips the guard off to prove the projects reopen** — that
  negative control would re-enable the damaging behaviour on a real machine with real bookmarks. The
  evidence is a before/after checksum of all 220 files, not a reproduction of the harm.
- ⚠️ **I-0149 is the SIXTH EP-034 defect found by use rather than by tests** (I-0137, I-0142, I-0146,
  I-0147, I-0148, I-0149) — ⚠️ **and the first found by a user asking whether the work had actually
  happened.** The suite was green, the binary contained the fix, and the fix did nothing.
- ⚠️ **The lesson is narrower and sharper than "test more".** T-0441 had a drifted fixture, a negative
  control, and a passing assertion that the repair worked. ⚠️ **All of it tested the REPAIR and none of it
  tested the TRIGGER.** A test that calls `load()` to check that `load()` repairs is a tautology wearing a
  fixture; the missing test was *"open a project and touch nothing else."*
- ⚠️ **"On open" is an EVENT, not a function.** The ruling named the event; the implementation picked a
  function that seemed adjacent to it. ⚠️ **When a ruling names a moment, the test must reproduce that
  moment** — not a call that usually accompanies it.
- ⚠️ **A stale test binary nearly hid the fix too.** The Xcode app build reconfigures the shared `build/`
  directory with `SCRIVI_BUILD_TESTS=OFF`, so `cmake --build` silently left a 28-minute-old
  `ScriviCoreTests` in place and the new tests reported *"No tests ran"* — which reads like a filter typo,
  not a stale binary (`project_linux_container_tests_off` is the same class on Linux).

- ⚠️ **I-0147 is a KNOWN LIMITATION, not a defect awaiting a fix** (user ruling, option 1). For up to 60 s
  after an interrupted world write, the world is unwritable and its `.partial` unreclaimable, because the
  dead writer's lock is not yet stale and the sweep only runs after a successful acquire. It **self-heals**
  and loses no data. ⚠️ **A regression test ASSERTS this behaviour** — if someone later makes `acquire`
  break fresh locks, it fails and forces the locking-model conversation rather than letting it happen by
  accident (the lesson of I-0144).
- ⚠️ **The eventual UI must never present the 60 s wait as an error** — it is a retryable state.
- ⚠️ **I-0148 is the FIFTH defect in EP-034 found by use rather than by tests** (I-0137, I-0142, I-0146,
  I-0147, I-0148) — and the first the user reported **without recognising it as a defect**, folded into an
  otherwise positive report. ⚠️ **A satisfied user is not a green suite**: the observation mattered more
  than the verdict attached to it.
- ⚠️ **Three of SP-116's six were found by no suite at all**: I-0143 by reading the code D7 was about to
  modify, I-0144 by looking for a caller to mirror, and **I-0146 by physically pulling a USB drive**.

---

## ✅ SP-115 — all six Issues Verified 2026-08-20

| Issue | Sev | Task | Archive |
| ----- | --- | ---- | ------- |
| **I-0137** | **High** | T-0419 | [`Verified/Issue-verified-0131-0140.md`](Verified/Issue-verified-0131-0140.md) |
| I-0136 | Medium | T-0420 | same |
| I-0139 | Medium | T-0421 | same |
| I-0135 | Low | T-0422 | same |
| I-0138 | Low | T-0423 | same |
| **I-0142** | **High** | T-0425 | [`Verified/Issue-verified-0141-0150.md`](Verified/Issue-verified-0141-0150.md) |

⚠️ **I-0137 was verified on the REAL RIG** with the drive ejected — the check a passing suite genuinely
cannot substitute for.

⚠️ **I-0136 is Verified at the CORE ONLY.** Nothing in Scrivi surfaces `unsupportedWorldFormatVersion`, so
a writer opening a too-new world still sees *"unavailable"* with **no explanation**. The core refuses
correctly; **the writer-facing half does not exist** — `project_capability_without_surface` inside the very
sprint that fixed four other instances. **Owed a surface in a later sprint.**

⚠️ **I-0142 was found by the USER, not a suite** — and its unseen half (**renaming any world object
failed**) was worse than the reported symptom.

---

*Last Updated: 2026-08-24, twenty-sixth pass (**I-0162 – I-0168 ✅ VERIFIED (user-approved) and ARCHIVED**
at SP-119 close → the new `Verified/Issue-verified-0161-0170.md`. Open Issues **7 → 0**; I-0147 remains an
Accepted limitation, not open work. ⚠️ **All seven came from the live click-through; six were data-loss
routes into one surface.** Next available Issue: **I-0169**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-fifth pass (⚠️ **I-0168 FILED — the Scene Inspector bypassed T-0452's
guard.** ⚠️ **The guard was in the wrong PLACE**: the host owns the history and the inspector asks the
host, so the sheet was never consulted. ⚠️ **T-0452 swept the four exits that originate inside the sheet
and could not see the one that originates outside it.** ✅ Fixed by moving the decision to a single owner
rather than adding a fifth check; ✅ **every history mutation swept.** ⚠️ **Sixth data-loss route in this
Epic** — the user has now found all six by ordinary use. Open Issues: **7**. Next available Issue:
**I-0169**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-fourth pass (⚠️ **I-0167 FILED — the ✕ discarded unsaved edits with no
prompt and no way to revert.** ⚠️ **Third route into this Epic's data loss**, and the only one a writer
triggers with an ordinary click. ⚠️ **Back/forward and related-list navigation shared the exposure** and
were fixed in the same pass rather than left for a later report. ✅ **Cancel + Save/Discard prompt**, to the
user's own design; ⚠️ **explicitly NOT undo** per their ruling. Open Issues: **6**. Next available Issue:
**I-0168**; Task: **T-0453**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-third pass (⚠️ **I-0166 FILED — cold-opening an object with its world
away showed a raw error code**, R9 violated in the case R9 exists for. ⚠️ **I-0165's fix covered only the
already-loaded sheet**, and its own comment claimed there was "nothing to show" when history carried the
object's name all along. ✅ Fixed with a `worldUnavailable` accessor mirroring the existing
`isWorldPending` idiom. ⚠️ **Fourth defect in one chain, each found by ejecting the drive at a different
moment.** Open Issues: **5**. Next available Issue: **I-0167**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-second pass (⚠️ **I-0165 FILED — a REGRESSION FROM I-0162'S FIX.**
Ejecting the drive replaced the whole Detail Sheet with a raw ScriviError and ⚠️ **discarded unsaved
edits** — R9 violated outright. The new `worldRevision` reload hit `load()`'s catch branch, which had
always been allowed to blank the sheet because it previously only ran on navigation. ✅ Fixed: a failed
re-read keeps the object and lets the read-only banner explain the outage. ⚠️ **Found by the re-test of
the very fix that caused it.** Open Issues: **4**. Next available Issue: **I-0166**. Prior note follows.)*

*Last Updated: 2026-08-24, twenty-first pass (⚠️ **I-0164 FILED — OPEN, needs a ruling.** An asset already
in a world **cannot be attached** to an object, and the only workaround — re-importing the same file —
⚠️ **silently orphans the first assetID**, because both bytes and sidecar are named after the FILENAME.
✅ **Proven by test**: one asset on disk, new ID listed, first ID unresolvable. ⚠️ **S11 missed it because
it enumerated FIELDS, not OPERATIONS** — `listAssets` was marked "not surfaced" without asking what a
writer would use it for. ✅ **The T-0447 chain itself is PROVEN WORKING** — the Tintagael location's image
imports, links, indexes and displays correctly. Open Issues: **3**. Next available Issue: **I-0165**.
Prior note follows.)*

*Last Updated: 2026-08-24, twentieth pass (⚠️ **I-0163 FILED — an image on disk in a world was invisible
to the app.** ⚠️ **A derived cache written before a field exists never rebuilds itself**, and T-0446's
tests could not see it because they always create their index with the current build. ✅ Fixed with an
index `generation` marker; ⚠️ **bump it when adding an entry field.** ⚠️ **Claude chased a phantom
failure for several rounds — the test had been passing and the binary was stale**
(`feedback_prove_code_is_reached`, third occurrence). Open Issues: **2**. Next available Issue: **I-0164**.
Prior note follows.)*

*Last Updated: 2026-08-24, nineteenth pass (⚠️ **I-0162 FILED — an ejected drive reported the writer's
image as DAMAGED rather than absent**, found by the user's SP-119 step-7 click-through. ⚠️ **Two causes:
the sheet never reloaded on a world-availability change** (`session.worldRevision` already existed and the
inspector cards already watched it — ⚠️ **the fourth "existing pattern not followed" since SP-118**), and
the outage branch was load-time only. ⚠️ **Claude's first two hypotheses were wrong**; the cause was
settled by probing the core (`loadAllVisible` → count=0 for an unavailable world). Open Issues: **1**.
Next available Issue: **I-0163**. Prior note follows.)*

*Last Updated: 2026-08-23, eighteenth pass (**I-0149 – I-0161 ✅ VERIFIED (user-approved) and ARCHIVED in
the same step** at SP-118 close → `Verified/Issue-verified-0141-0150.md` and the new
`Verified/Issue-verified-0151-0160.md`. Open Issues **13 → 0**; I-0147 remains an Accepted limitation, not
open work. ⚠️ **All thirteen came from the live click-through and none from any suite.** ⚠️ **Four were one
failure — an existing correct pattern the new code did not follow.** Next available Issue: **I-0162**.
Prior note follows.)*

*Last Updated: 2026-08-22, seventeenth pass (⚠️ **I-0159: the related list LOOKED like it had lost rows** —
a nested `List` inside the sheet's ScrollView hid 5 of Myton's 8 behind an invisible second scroll;
⚠️ **the USER diagnosed it.** I-0160: ⚠️ **I-0155 had been fixed in one direction only.** I-0161: navigator
reveal for navigation from another surface, ⚠️ **carefully distinguished from the reveal I-0132 removed.**
Open Issues: **12**. Next available Issue: **I-0162**. Prior note follows.)*

*Last Updated: 2026-08-22, sixteenth pass (⚠️ **I-0155 FILED — SILENT DATA LOSS**: a Detail Sheet save
patched a snapshot from sheet-open, reverting a Scene Inspector rename. ⚠️ **Reported by the user as
possibly not a defect at all.** Fixed in three parts, incl. per-field conflict resolution so the fix does
not reverse the loss. I-0156: rows had no selection. I-0157: scene navigation bypassed I-0132's
selection-is-truth ruling. Open Issues: **9**. Next available Issue: **I-0158**. Prior note follows.)*

*Last Updated: 2026-08-22, fifteenth pass (⚠️ **I-0151–I-0154 FILED AND RESOLVED — all four found by the
SP-118 LIVE CLICK-THROUGH, none by any suite.** ⚠️ **I-0151 broke navigation to every world-scoped object**
and was caused by a comment asserting a falsehood I never checked. ⚠️ **I-0152 showed the writer a raw
scene ID** where the Navigator already knew a useful name — ⚠️ **Claude dismissed it once and the user
was right to reject that.** I-0153: scene rows were a dead affordance. I-0154: no right-click highlight.
✅ **"The Lantern Foxes" checked and CLOSED as correct** — opposite endpoints of one edge. Open Issues:
**6**. Next available Issue: **I-0155**. Prior note follows.)*

*Last Updated: 2026-08-22, fourteenth pass (⚠️ **I-0150 FILED AND RESOLVED — `xcodebuild test` launches the
real app and reopened the user's ACTUAL PROJECTS.** ⚠️ **This, not a user launch, is what modified
`the-twisted-remains-of-myself.scrivi`; Claude had misattributed it to the user and was corrected.**
✅ Fixed at the choke point in `restoreOpenProjects()`; ✅ **verified by checksums of 220 files across three
full test runs — byte-identical**. ⚠️ **`pgrep Scrivi` never protected against this.** Open Issues: **2**
(I-0149, I-0150). Next available Issue: **I-0151**. Prior note follows.)*

*Last Updated: 2026-08-22, thirteenth pass (⚠️ **I-0149 FILED AND RESOLVED — found by the USER asking
whether the migration had actually occurred**, after SP-118 reported green. ⚠️ **T-0441 reconciled on READ,
not on OPEN** — the repair lived in `RelationTypeStore::load()`, which a project open never calls; the real
rig opened a drifted project with the fix in the binary and changed nothing. ✅ **Fixed in
`ProjectOpener::open`** as repair pass (e); ✅ **negative control run** (the new test fails against
T-0441-as-shipped); ✅ **verified against a copy of the user's real project**. ⚠️ **T-0441 is NO LONGER
"Implemented"** on its own — it is complete only with I-0149. `ctest` **561/561**. Open Issues: **1**
(I-0149, Resolved - Not Verified). Next available Issue: **I-0150**. Prior note follows.)*

*Last Updated: 2026-08-21, twelfth pass (✅ **I-0148 VERIFIED (user-approved) and ARCHIVED in the same
step.** ⚠️ **It was found by the user's live click-through and reported as an OBSERVATION, not a
complaint** — the fifth defect in EP-034 found by use rather than by tests. **Open Issues: 0**; I-0147
remains an Accepted limitation. Next available Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, eleventh pass (⚠️ **I-0148 FILED AND RESOLVED — found by the user's LIVE
CLICK-THROUGH of SP-117**, and ⚠️ **reported as an observation, not a complaint**: `.disabled()` does not
make a `TextEditor` read-only, so Notes stayed editable beneath a "read only" banner. ✅ **Never a
write-safety bug** — Save is hidden when read-only — ⚠️ **but typing during an outage was silently
discarded on navigation**, since `load()` overwrites the draft. **User ruled: disable it**, for simplicity
and consistency over draft retention. Notes now renders as selectable text when read-only. Next available
Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, tenth pass (✅ **SP-116's SIX ISSUES VERIFIED (user-approved) and ARCHIVED in
the same step** → `Verified/Issue-verified-0141-0150.md` (`feedback_archive_on_close`). ⚠️ **I-0147 remains
here as an ACCEPTED limitation** — deferred to the network-worlds design, with a regression test asserting
it. **Open Issues: 0.** Next available Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, ninth pass (⚠️ **I-0147 FILED AND ACCEPTED as a known limitation** (user ruled
option 1): for up to 60 s after an interrupted world write the world is unwritable and its `.partial`
unreclaimable, because the dead writer's lock is not yet stale and **the sweep only runs after a successful
acquire**. ⚠️ **Found by the tidy end-to-end rig run** — drive pulled, reattached quickly, next write
refused `worldLocked`, **2.9 GB orphan retained**. ✅ **Both halves verified** (fresh lock → refused; past
60 s → acquired **and swept**). ⚠️ **My earlier staged-orphan test passed only because it omitted the
matching fresh lock** — a setup subtly easier than reality; **fourth defect this Epic found only by live
use**. **Deferred to the network-worlds design**, which must revisit "exactly one winner" anyway.
Open Issues: **0** (I-0147 is Accepted, not open). Next available Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, eighth pass (✅ **I-0146 ASSIGNED to SP-116 (T-0433) and RESOLVED** by user
ruling. `WorldLock::sweepAbandonedPartials()` reclaims abandoned `*.partial` files whenever the lock is
acquired. ⚠️ **Swept on EVERY successful acquire, not only after breaking a stale lock** — the rig showed
the lock file and the partial are orphaned TOGETHER, so the next writer acquires cleanly and never reaches
a break path; sweeping only on a break would have missed the exact case this Issue was filed for.
⚠️ **Verified on real hardware**: 459 MB orphan on the USB volume reclaimed by a normal import, 476 MiB →
12 MiB, real assets and `myton.json` untouched. Tests **551/551** (+4), ⚠️ **proven non-vacuous** —
disabling the sweep fails two. **Open Issues: 0.** Next available Issue: **I-0147**. Prior note follows.)*

*Last Updated: 2026-08-21, seventh pass (⚠️ **I-0146 FILED — found by the LIVE RIG PASS, not by a suite.**
Pulling a real USB drive mid-import left a **459 MB `.partial` orphan** inside the shared world: the
cleanup in `copyFileInBlocks` cannot run when the failure IS the volume vanishing. ⚠️ **`list_assets`
cannot see it, so nothing in Scrivi will ever reclaim it.** ✅ **The rest of the abort behaved correctly** —
heartbeat detected the loss, transfer aborted, no destination file, existing assets byte-identical, stale
lock breakable after 60 s. **Fix is the user's own stale-lock sweep**, which SP-116 did not implement.
Open Issues: **1** (I-0146). Next available Issue: **I-0147**. Prior note follows.)*

*Last Updated: 2026-08-21, sixth pass (**I-0144 🟢 Resolved - Not Verified** — every world-package write
path now takes the lock via `WorldWriteGuard`, ⚠️ **inert for project writes so there is no branch to
forget**. ⚠️ **One deliberate exception recorded**: `ObjectIndex::loadWorldIndex`'s rebuild stays unlocked
because `WorldLock` is NOT REENTRANT and `save`/`remove` reach it while holding the lock — a guard there
would fail against itself and skip the rebuild. It is idempotent; the real fix is a reentrant lock, which
belongs with the network-worlds design. **Open Issues: 0.** Next available Issue: **I-0146**. Prior note
follows.)*

*Last Updated: 2026-08-21, fifth pass (**I-0145 FILED — 🟢 Resolved - Not Verified.** ⚠️ **Pre-existing and
shipped**: `AssetStore::remove` deleted the sidecar first and discarded both results, so a half-failed
delete stranded **bytes with no sidecar — invisible to `list` and unfindable by any future `remove`**,
unreclaimable for the life of the package, with `deleted: true` returned regardless. ⚠️ **D6 raises its
severity**, since the junk now lands in a SHARED world. Found by **self-review**; ⚠️ **no test caught it**.
✅ **Fixed in T-0426** (binary deleted first, both failures reported). ⚠️ **A sibling defect was
deliberately NOT filed** — `ObjectKindScope`'s duplicate-key trap was written and fixed inside this sprint
and never shipped. Open Issues: **1** (I-0144). Next available Issue: **I-0146**. Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (✅ **I-0144 ASSIGNED to SP-116** by user ruling → **T-0431**;
⚠️ **it is a High-severity data-loss risk, not an asset defect** — every object write into a shared world
is unserialised. Open Issues: **1**, now assigned. Next available Issue: **I-0145**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**SP-116 IMPLEMENTED — I-0140, I-0141, I-0143 all 🟢 Resolved -
Not Verified.** ⚠️ **I-0140 and I-0143 were each proven non-vacuous by reverting the fix** and watching the
tests fail. ⚠️ **I-0144 FILED (High, unassigned)**: `WorldLock` has **no production caller** — world-package
object writes are unserialised and have been since they shipped, so two projects sharing a world can lose
each other's edits silently. Found while implementing T-0426, looking for a caller to mirror; **no test
would have caught it**, since a missing lock is invisible single-threaded. ⚠️ **Not fixed in SP-116** — it
touches every object write path, not assets. Open Issues: **1** (I-0144). Next available Issue: **I-0145**.
Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**SP-116 ACTIVATED** — all three open Issues are now assigned to
an **active** Sprint, not a planned one; Sprint fields marked 🟡. ⚠️ **None is Resolved** — activation is
not progress, and Claude may never mark an Issue Verified regardless
(`feedback_verification`). Next available Issue: **I-0144**. Prior note follows.)*

*Last Updated: 2026-08-21 (**I-0143 FILED at SP-116 planning** — ⚠️ `scrivi_list_assets` concatenates
its JSON with **no escaping** (`scrivi_c_api.cpp:1330-1341`), while every sibling envelope uses `JsonDoc`.
⚠️ **Found by reading the code D7 modifies, not by a test and not by the design doc** — and D7 is precisely
what makes it reachable, since **T-0427 puts a filesystem path into that array**. ✅ **User ruled: file it
AND fix it in SP-116** (T-0428), keeping T-0424's file-don't-fix-silently precedent while refusing to ship
a corruption path the same sprint could prevent. ⚠️ **The restating summary table below the main table was
REPLACED** with only what the table cannot express (P7). Open Issues 2 → **3**, all SP-116. Next available
Issue: **I-0144**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115's six Issues ✅ VERIFIED by the user and ARCHIVED in the same step** —
I-0135–I-0139 → `Verified/Issue-verified-0131-0140.md`, **I-0142 → a new decade file
`Issue-verified-0141-0150.md`.** Open Issues 8 → 2 (**I-0140, I-0141** — filed for SP-116, unfixed by
design). ⚠️ **I-0137 verified on the real rig, drive ejected.** ⚠️ **I-0136 verified at the CORE ONLY — its
writer-facing surface does not exist and is owed.** Suites: ctest **525/525** · interop **103/103** · app
**BUILD SUCCEEDED**. Next available Issue: **I-0143**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 implemented — all five Issues 🟢 Resolved - Not Verified**, and
⚠️ **I-0140 + I-0141 FILED by T-0424** (restated-kind-list class, occurrence eight → **SP-116**, cured by
D5). Suites: `ctest` **524/524** (was 520) · macOS interop **103/103 in 10 suites** (was 99) · app
**BUILD SUCCEEDED**. ⚠️ **I-0137 still needs the REAL-RIG check** — drive ejected — before it can be
Verified. Open Issues 5 → 7. Next available Issue: **I-0142**. Prior note follows.)*

*Last Updated: 2026-08-20 (**All five open Issues ASSIGNED to SP-115** 🟡 Active under **EP-034** — one
Task each, T-0419–T-0423. ✅ **Two carried rulings recorded**: **D9 = A** for I-0137
(`lastKnownPackagePath`, distinctly named; `packagePath` NOT widened) and **Q-b** for I-0139 (**patch the
control** — the Detail Sheet does **not** replace the inline editor, so it is a real fix). ⚠️ **I-0140 and
I-0141 to be FILED by T-0424.** Next available Issue: **I-0142** after that filing. Prior note follows.)*

*Last Updated: 2026-08-19 (**T-0390 + T-0418 filed five Issues — I-0135…I-0139.** The live pass on the
real USB rig **passed steps 3, 4 and 5**: ⚠️ **AC23's no-intervention clause HELD** — reattaching the drive
restored every card with no click, no menu, no relaunch. Step 1 confirmed **all ten world kinds
round-trip** (the four directories absent since before SP-104 were created on demand); ⚠️ **`source` could
not be created — no UI exists, the known EP-034 gap.** Step 2 was **blocked**: relating from an object card
opens an editor whose exit is labelled "Revert" (I-0139). Findings: **I-0137 (High)** — AC24's refinement
**cannot fire on real hardware**; **I-0138** — disabled-but-unexplained removal; **I-0139** — the editor
exit. Next available: **I-0140**. Prior note follows.)*

*2026-08-18, fourth pass (✅ **I-0132 VERIFIED (user-approved) and re-archived** — both
halves, on an extended live click-through: *"I clicked about a lot and saw no missed clicks or focus
changes."* It took **four** attempts; the first three misdiagnosed it as a first-responder race and
each made the failure rarer rather than fixing it. ⚠️ **The user stopped the fourth before it was
written** — I was about to add an `NSEvent` monitor, reaching further below SwiftUI to win a fight
created by reaching below it in the first place — and redirected to the actual question: *what is the
source of truth, and does it propagate through the View hierarchy?* **The real defect was a one-shot
`navigateToSceneID` trigger**, not responder arbitration: re-selecting the same scene wrote an
unchanged value and SwiftUI coalesced the update away. macOS now uses the selection-as-source-of-truth
shape iOS already had. A **user-prompted loop audit** then replaced a fragile value-equality guard
with explicit echo suppression, plus **two regression tests proven non-vacuous**. Interop **95/95
macOS arm64**. Active count: **2** (I-0133 Resolved-Not-Verified, I-0134 Open). Prior note follows.)*

*2026-08-18, third pass (⚠️ **I-0132 RETURNED FROM VERIFIED — I archived it on a claim
that was not true.** The user verified focus changing **on app launch**, and said so explicitly; I
recorded that as verifying **click-to-focus** as well. Clicking a scene still left focus in the
navigator. **Cause was a responder race, not a missing call:** `takeFocus` ran
`makeFirstResponder` synchronously from inside `onTapGesture`, and the `NSTableView` backing SwiftUI's
`List` reclaimed first responder while finishing its own mouse-down. Launch had no competing responder
change, which is exactly why the two cases diverged — **the evidence I verified against and the
failing case were different code paths.** Second fix defers the transfer one runloop pass. The
**reveal half stays verified** and remains archived. **Lesson recorded:** when a fix has two halves,
verify each half against its own trigger — a verification of one is not evidence for the other.
Active count: 2 → **3**. Prior note follows.)*

*2026-08-18, later same day (**I-0131 + I-0132 ✅ Verified (user-approved) and
archived** to the new `Verified/Issue-verified-0131-0140.md` decade file, and removed from this file
in the same step. **I-0133 ruled and resolved:** the user chose *delete Apple's dead state, leave
Linux alone* — the property, its `loadAll` parameter, the write, the clear and the `ProjectSession`
plumbing are gone, each site commented so the omission reads as deliberate; ⚠️ **the schema field
stays** because Linux consumes it. ⚠️ **Ruling I-0133 surfaced a finding the original report missed,
now filed as I-0134 (🔴 Open):** Linux applies the scroll fraction *after* `centerCursor()`,
deliberately overriding it — so **Apple and Linux now disagree about what "restore where I was"
means.** Deliberately **not** settled inside a dead-code cleanup: it changes shipped, VNC-verified
EP-022 behaviour and belongs to EP-026 parity. **BUILD SUCCEEDED**, interop **93/93 macOS arm64**.
⚠️ **Active count is now 2** — I-0133 (Resolved - Not Verified) and I-0134 (Open). The prior note's
"Active count: 10 → 13" was already stale before this pass: those Issues had been verified and
archived without this line being updated. Prior note follows.)*

*2026-08-18 (**I-0132 both halves now 🟠 Implemented - Not Verified.** ⚠️ **The
reveal-on-selection-change half was REMOVED, not tuned** — the user's re-test found it scrolled the
navigator "a little bit up or down" on **every** click, because `scrollTo` **re-anchors an
already-visible row** rather than no-opping as my comment had claimed. Reveal now fires **`onAppear`
only**, which is the one moment it is needed (restore sets the selection before the view exists).
The **focus half is implemented**: `navigate(to:)` calls `loader.takeFocus()`, so a click or Return
hands the keyboard to the manuscript and the caret is visible. ⚠️ **Accepted trade, user-ruled:**
this ends arrow-key list browsing after the first click — *"Arrow browsing isn't strictly necessary.
Mouse Wheel and Trackpad Scrolling are still available."* **Tab-as-focus-advance is no longer needed
for this Issue.** Also filed **T-0417** (Scene/Chapter boundary navigation) — adopted into SP-102,
shipping as menu items because ⚠️ **no free macOS key combination exists.** **BUILD SUCCEEDED**,
interop **93/93 macOS arm64**. Prior note follows.)*

*2026-08-17, later same day (**I-0114–I-0117 ✅ Verified (user-approved) and archived** to
`Verified/Issue-verified-0111-0120.md` in the same step — verified live during the SP-102 / T-0415
world-availability runs, which exercised those exact surfaces. ✅ **They are now usable as evidence for
SP-100's AC pass**, which the prior note said they were not. **Also filed and fixed the same day:
I-0123–I-0129**, all from the user's live SP-102 runs. Active count: 10 → **13**. Prior note follows.)*

*2026-08-17 (**SP-106 closed — I-0121 and I-0122 ✅ Verified and archived** to the new
`Verified/Issue-verified-0121-0130.md` decade file, and their full entries removed from `Issue-backlog.md` in
the same step. Neither was ever listed in this file — both were tracked in `Issue-backlog.md` and the SP-106
sprint record. **This file is unchanged otherwise: the same 10 `Resolved - Not Verified` Issues remain
active**, including I-0114–I-0117, which are **not** evidence for any EP-031 AC until verified. Prior note
follows.)*

*2026-08-15 (docs cleanup — 48 verified Issues archived to decade files, 4 closed Issues
archived; 6 stale full entries (I-0064, I-0067–I-0071) reconciled against their authoritative table rows.
10 `Resolved - Not Verified` Issues remain active.)*
