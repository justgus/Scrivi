# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0626 — `[Cross]` EP-052 S2a: every scene save through the queue, Mac and Linux

**Epic:** [EP-052] (AC4, AC5, AC6, AC7) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-26) — ⏳ the rig and Mac live passes ([SP-195] AC6, AC7) · **Sprint:** [SP-195] (🟡 Active) →
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

