# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0629 — `[ScriviCore]` EP-052 S3: one Inspector read per scene, and the per-scene searchable text

**Epic:** [EP-052] (AC8, AC9) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ⏳ the Mac and rig live passes ([SP-197] AC5) · **Sprint:** [SP-197] (🟡 Active) → [`../Sprints/Sprint-SP-197.md`](../Sprints/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** (a) one endpoint returning everything the Inspector shows for a scene: its notes, its edges with each far
object's name, kind, image path and pending state, the sources' citations, and the bound worlds' status, with the object
index built once (v0.2 R3b; [SP-197] Q1). It replaces the 32 calls the Mac makes today (`ObjectCard.swift:157,179,614`,
`SourcesCard.swift:89,107`, `WritingToolCards.swift:37`, `ScenePropertiesView.swift:133`) and Linux's
(`SceneInspector.cpp:404–409,856`). An unreachable world keeps today's behaviour ([I-0222], D8): its edges still list by
cached name. (b) `scrivi_extract_searchable_text` for a list of scene IDs (`scrivi.h:684` takes a project only).
**Tests:** boundary tests through `scrivi_*`; the file-operation count for one scene asserted (CountingFileSystem); an
unreachable world; Engine stub parity (`check-engine-stub-parity.sh`) and the Linux bridge.

**Done (2026-10-27):** `scrivi_load_scene_inspector` (`scrivi_c_api.cpp:3477`): notes, edges, the far objects (the
object index built once, narrowed to the scene's far endpoints), worlds and, when asked, each reachable far object's own
edges, under ONE project lock, each part its standalone endpoint's builder (`EndpointPart`, shared with
`scrivi_load_timeline`), a failing part as `<part>Error`. An empty part is `{}`, never `null`. `scrivi_extract_searchable_scenes`
(`:3588`): the named scenes' records only (`ExtractSearchableTextRequest::scenesOnly`). Bound in Swift (`loadSceneInspector`,
`extractSearchableScenes`) and on Linux (`ScriviBridge::loadSceneInspector`). Each envelope's trace now names its project, so a
test can count one project's main-thread calls apart from the others running beside it. **Evidence:** ctest 786/786 (5 new
`[SP-197]` boundary cases: parts equal the standalone endpoints, one revision; a world away; nothing related; a missing root;
the per-scene extraction). File operations on the fixture: the composite **67**, against **353+** for the per-card calls it
replaces. ⚠️ One unidentified ctest failure in the first full run after a rebuild; 18 full runs since are clean.

---

## 🟠 T-0630 — `[Cross]` EP-052 S3: the Inspector and the timeline read off the main thread, revision-checked

**Epic:** [EP-052] (AC8) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ⏳ the Mac and rig live passes ([SP-197] AC5) · **Sprint:** [SP-197] (🟡 Active) → [`../Sprints/Sprint-SP-197.md`](../Sprints/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** Mac and Linux in the same work (`feedback_linux_adopts_apple_shape`). The Inspector shows the new scene's cards
at once ([SP-197] Q2), runs [T-0629]'s read as ONE job off the main thread, and fills the cards when it lands if
`scrivi_get_project_revision` has not moved (D4); otherwise it reads again. The timeline's reloads do the same: the open's
main-thread fallback (`ProjectSession.swift:328`) and Linux's `reloadTimeline()` (`EditorShell.cpp:3351`). Linux's
object-card worker (`SceneInspector.cpp:399`) is the shape; its notes read (`:856`) joins it.
**Tests:** the inventory probes (`MainThreadInventoryTests`; `SCRIVI_INVENTORY=1 structural_baseline_smoke`) assert 0 file
operations on the main thread for click, select, scroll, navigator and the Inspector following; a result whose revision
moved is dropped.

**Done (2026-10-27):** Mac: `SceneInspectorReader` (`InspectorCard.swift:199`), owned by `SceneInspectorView`, makes ONE read
per scene off the main actor; every card (tags, outline, todo, the ten object cards, Sources) and the Properties tab are built
from it (`CardContext.scenePart`); a result whose revision moved is read again; while the read is out the cards keep their last
contents dimmed and take no edits, "Loading…" after 400 ms (Q2, `sceneReadGate`); a card's own write re-reads the scene. The
open's timeline fallback is re-read off the main actor (`ProjectSession.swift:560`). Linux: the Inspector's worker makes the one
call, the notes included (`SceneInspector.cpp:374`), revision-checked, the last contents dimmed, "Loading…" after 400 ms; the
load's stale timeline is re-read on a worker (`EditorShell.cpp:3355`). **Evidence:** interop 323/323, new
`MainThreadNoFileOpsTests` (0 main-thread file ops for click, select, scroll, navigator and the Inspector following; a card's
write re-reads); Linux smokes 38/38, `structural_baseline_smoke`'s new SP-197 gate (0 GUI-thread file ops for click, select,
scroll) — a mutation in a copy (the notes read back on the GUI thread) fails it on all four gestures.

---

## 🟠 T-0631 — `[Apple]` EP-052 S3: Spotlight on one serial background queue, per-scene, behind the saves

**Epic:** [EP-052] (AC9) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ⏳ the Mac and rig live passes ([SP-197] AC5) · **Sprint:** [SP-197] (🟡 Active) → [`../Sprints/Sprint-SP-197.md`](../Sprints/Sprint-SP-197.md) · **Date Requested:** 2026-10-27 (user: *"plan S3"*)

**What:** v0.2 §5 S1–S4. Every `CSSearchableIndex` call (donate, re-index, delete on close) runs on one serial queue
(Apple: one thread at a time). The open's donation and the app-switch re-index (`ProjectSession.swift:541,550`) leave the
main thread; the extraction is a read behind the queue's saves; the app switch re-indexes only the scenes typed in
([T-0629] (b)); a close cancels a pending job. Only `spotlightDomainIdentifier` is set on the main actor, when the job lands.
**Tests:** the inventory's open and app-switch rows read 0 file operations on the main thread; a close during a donation.

**Done (2026-10-27):** every Spotlight call runs on one serial background queue (`SpotlightDonor.enqueue`,
`SpotlightDonor.swift:40`): the open's donation and the app-switch re-index extract there, behind the queue's saves; the app
switch re-indexes only the scenes typed in (`extractSearchableScenes`); a close cancels a pending job and deletes the domain
behind any donation already started (by the projectID, so it is known before the first donation lands). **Evidence:** interop
323/323; build clean. ⚠️ No test drives a close during a donation.

---

## 🟠 T-0627 — `[Cross]` EP-052 S2b: structural edits through the queue, and the gesture cues

**Epic:** [EP-052] (AC4, AC5, AC10) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-27) — ✅ Mac live pass; ⏳ the rig live pass ([SP-196] AC4) · **Sprint:** [SP-196] (🟡 Active) →
[`../Sprints/Sprint-SP-196.md`](../Sprints/Sprint-SP-196.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** on both platforms a structural edit is `scrivi_enqueue_range_op` behind the queued saves; the delta comes back in
its outcome and is applied on the main thread; its history node is recorded from it. Each platform's structural worker is
removed (W7h). The v0.2 Q5 gesture cues.

**Done (2026-10-27):** each structural edit — paste, split, chapter split, merge, chapter merge, cut / Backspace / Delete over
scenes, and every undo and redo of them — is queued (`enqueueRangePaste` / `Cut` / `Relabel`, `ScriviEngine.swift:1334`,
`ScriviBridge.cpp:397`) behind the unsaved scenes' saves (W7b), OFF the main thread; its outcome comes through the ONE write
waiter (`ViewportSceneLoader.swift` `apply(_ wait:)`, `EditorShell.cpp:1689`), and its history node and story times are made
off the main thread before the delta is applied. The synchronous structural call and its worker are gone (W7h)
(`ViewportSceneLoader.swift:551`, `ManuscriptTextView.swift:1372`, `EditorShell.cpp:2037`). Gesture cues (v0.2 Q5): the
cue at once, a repeat ignored, and ⌘Z / Ctrl+Z pressed while an edit is in flight WAITS ITS TURN
(`ManuscriptTextView.swift:390`, `EditorShell.cpp:2502`). A structural edit behind a held queue is refused at once; one that
is itself held keeps the editor read-only until it lands, and a close asks rather than waits. **Evidence:** interop 321/321
(2 new: `ScriviInteropTests.swift:7799`, `:7828`); Linux smokes 38/38 (`background_wait_smoke` §2b new, caught by a mutation
in a copy; `structural_baseline_smoke` Q5 updated). Details: [SP-196]'s log.

---

## 🟠 T-0626 — `[Cross]` EP-052 S2a: every scene save through the queue, Mac and Linux

**Epic:** [EP-052] (AC4, AC5, AC6, AC7) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-26) — ✅ Mac live pass ([SP-195] AC7); ⏳ the rig live pass ([SP-195] AC6) · **Sprint:** [SP-195] (🟡 Active) →
[`../Sprints/Sprint-SP-195.md`](../Sprints/Sprint-SP-195.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** on both platforms the autosave, app switch, undo and redo writes, Replace All, the drive-returned flush, quit, and the
resume stamp go through `scrivi_enqueue_save_scenes`; a background waiter applies each outcome on the main thread (written →
clean; failed → dirty). The platforms' `noteScenePersisted` save-path calls and W6's ordering code are removed. Replace All
and its undo stop locking the manuscript; Linux Replace All shows a real count ([SP-185] Q3). Close and quit wait for the
queue and ask when it is held ([T-0570] on the Mac; on Linux, [I-0322] folded in per [SP-195] Q3: the Mac's banner and question).

**Done (2026-10-26):** Mac (`ViewportSceneLoader.swift` `enqueueSaves` / `startWaiting` / `apply`) and Linux (`EditorShell.cpp`
`enqueueSaves` / `startWaiting` / `applyWriteOutcomes`): every scene save queued, outcomes on the main thread, W6 removed,
Replace All and undo never lock the manuscript, a close and quit wait for the queue and ask when it is held (Linux [I-0322]:
the banner and the question), Linux Replace All counts *"Replacing N of M"*. **Evidence:** interop 319/319; smokes 38/38;
main-thread file operations for the saves 0 on both (Mac autosave / ⌘Z / ⇧⌘Z 5 → 0; Linux 6 / 8 / 7 → 0); ctest Mac 781,
Linux 784. Details: [SP-195]'s log.

---

## 🟠 T-0616 — `[Cross]` EP-053 S3: structural cut, Backspace / Delete over scenes, and their undo

**Epic:** [EP-053] (S3) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-22) — ✅ rig over the share, ✅ Mac on a local disk; ⏳ the Mac over a share → [SP-190] · **Sprint:** [SP-188] (✅ Closed 2026-10-26) →
[`../Sprints/Closed/Sprint-SP-188.md`](../Sprints/Closed/Sprint-SP-188.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** on the Mac and on Linux, a cut, buffer cut, or Backspace / Delete across scenes removes the structure (design R4,
§4.3, Q2, Q4) through one `scrivi_range_cut` off the main thread, the delta applied, no reload; undo is a range paste
(`rangeCut` node), redo the cut. The core balances the cut's join and returns the markers it added, so the undo is exact.
The clipboard (or slot) holds the cut's returned Range with its sidecars; a selection crossing a divider takes that scene
whole; Backspace over one scene divider merges; paste over a cross-scene selection is the cut, then the paste. Old
`structuredCut` nodes and [I-0270]'s grouped deletes still undo. Retires both platforms' `deleteAcrossScenes`.

**Done (2026-10-22):** ScriviCore (`RangeJoin`, the balanced join in `cut()`, applied and checked in `paste()`); the Mac
(`selectionRun`, `cutAcrossScenes(_:keeping:then:)`) and Linux (`ManuscriptEditor::selectionRun`,
`EditorShell::cutAcrossScenes`) — every cross-scene cut, Backspace / Delete and paste-over one range call off the main
thread, the delta applied, undone in one step; `deleteAcrossScenes` retired on both. ctest 763 (Mac) / 767 (Linux),
interop 311, Linux smokes 38; mutations core 6, Mac 8, Linux 7, all caught.

---

*Last Updated: 2026-10-26 (**T-0616**: [SP-188] closed; its last check, the Mac pass over a share, → [SP-190].)*

*Last Updated: 2026-10-25 (**T-0622 → ✅ Verified** (user) and archived → `Verified/Task-verified-0622.md`, with [SP-193]'s close.)*

*Last Updated: 2026-10-25 (**T-0622**: ✅ Mac half passed (user: About shows the version and hash); ⏳ the rig pass.)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** ([SP-193]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-24 (**T-0620 → ✅ Verified** (user) and archived → `Verified/Task-verified-0620.md`, with [SP-192]'s close.)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** ([SP-192]); ⏳ Mac live pass.)*

*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** ([SP-188]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-22 (**T-0612 → ✅ Verified** (user) and archived → `Verified/Task-verified-0612.md`, with [SP-185]'s close.)*

*Last Updated: 2026-10-22 (**T-0619 → ✅ Verified** (user) and archived → `Verified/Task-verified-0619.md`, with [SP-191]'s close.)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** ([SP-191]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-21 (**T-0605, T-0607, T-0608, T-0609, T-0613, T-0615 → ✅ Verified** (user: Mac + rig live passes, build 75) and archived → `Verified/Task-verified-XXXX.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** ([SP-187]); ⏳ live passes.)*

*Last Updated: 2026-10-20 (**T-0614 → ✅ Verified** (user) and archived → `Verified/Task-verified-0614.md`, with SP-186's close.)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** ([SP-182]); ⏳ rig live pass.)*

