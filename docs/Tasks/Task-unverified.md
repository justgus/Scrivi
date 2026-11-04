# Unverified Tasks

*Last Updated: 2026-11-04 (**T-0658, T-0659 → ✅ Verified and archived** (user, 2026-11-04: *"this is a pass. We should be able to close SP-202 and SP-203."*))*

*Last Updated: 2026-11-04 (**T-0617, T-0618 → 🟠** ([SP-189], [SP-190]).)*

*Last Updated: 2026-11-04 (**T-0658, T-0659 → 🟠** (EP-050 S3, S4).)*

*Last Updated: 2026-11-03 (**T-0632–T-0638 → ✅ Verified and archived** (user: the iPad, iPhone and Duo live passes, 2026-11-03).)*

*Last Updated: 2026-11-02 (**T-0400, T-0626, T-0627, T-0629–T-0631, T-0640, T-0641, T-0643–T-0647 → ✅ Verified and archived** (user, 2026-11-02).)*

*Last Updated: 2026-11-02 (**T-0656** filed and implemented → 🟠.)*

*Last Updated: 2026-11-02 (**T-0646 → 🟠** from `Task-active.md`; its rig pass → [T-0653] ([EP-057]).)*

*Last Updated: 2026-11-01 (**T-0647** filed in SP-200 and implemented → 🟠.)*

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0617 — `[Cross]` EP-053 S4: the background sync verification

**Epic:** [EP-053] (S4) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-04) — ⏳ the live passes ([T-0661]; the Mac's section) ·
**Sprint:** [SP-189] → [`../Sprints/Sprint-SP-189.md`](../Sprints/Sprint-SP-189.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** after every structural operation, a background task on both platforms re-reads only the files the delta names and
corrects any difference from the disk silently (design R3, P4, Q3), never overwriting a scene typed into meanwhile (Q2).

**Done (2026-11-04):** `scrivi_range_verify_read` (core); Linux `RangeVerify.hpp` + `EditorShell::verifyAgainstDisk`; Apple
`RangeVerify` + `ViewportSceneLoader.verifyAgainstDisk` + `applyCorrection`. A two-scene paste's verification = 1 listing +
2 reads. Tests on all three; mutations Linux 6/6, Apple 3/3. ✅ [SP-189] Q3 ruled 2026-11-04: a difference a delta cannot fix is logged only.

---

## 🟠 T-0618 — `[Cross]` EP-053 S5: Mac and rig live passes over the share, timed

**Epic:** [EP-053] (S5) · **Status:** 🟠 **Implemented - Not Verified** (2026-11-04) — ⏳ the passes · **Sprint:** [SP-190] →
[`../Sprints/Sprint-SP-190.md`](../Sprints/Sprint-SP-190.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** live passes on dumas-prose-timelines over the share on the Mac and the rig: every structural operation and its undo
and redo, timed by each platform's log, with no freeze; the Mac's timing source per [SP-190] Q1.

**Done (2026-11-04):** the Mac's diagnostic log ([SP-190] Q1: `DiagnosticLog`, Help ▸ Show Log Folder). The steps: the rig's
[T-0661] and the Mac's section, both in `../Sprints/Rig-Live-Pass-Held.md`. ✅ [SP-190] Q2 ruled 2026-11-04: Samba on scrivi-vm (`ScriviOtherLoop`).

---

## 🟠 T-0656 — `[Linux]` Landing: a warm grey background with a little depth

**Epic:** none (a live-pass finding, EP-057) · **Status:** 🟠 Implemented - Not Verified · **Sprint:** none · **Date Requested:**
2026-11-02 (user: *"A slightly warm grey color should be used for the launch panel. maybe with a slight gradient, as if it had
some depth."*)

**Why:** on GNOME Wayland, Qt's Adwaita frame draws no shadow — it reserves a 10 px shadow margin and paints nothing in it
(checked in Qt 6.10.2's `qwaylandadwaitadecoration.cpp`: `ceShadowsWidth = 10`; `paint()` fills the title bar and draws a
1-line border only). Landing and a project window in the same system colour could not be told apart.

**What:** `platforms/linux/qml/Landing.qml` — a vertical gradient behind the `StackView`, light `#ece7e0` → `#d8d1c7`, dark
`#45403b` → `#2e2a27` (from `Application.styleHints.colorScheme`); the landing `Page` and `NewProjectDialog.qml` draw no
background (`background: null`). Linux only: on the Mac the system's window shadow does this.

**Tested:** builds; smokes 38/38; an offscreen launch logs no QML warning. ⏳ The look is the user's call, on the VM.

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

