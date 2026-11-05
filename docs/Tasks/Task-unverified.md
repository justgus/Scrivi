# Unverified Tasks

*Last Updated: 2026-11-05 (**T-0662 → 🟠** ([SP-204], EP-026 S3).)*

*Last Updated: 2026-11-04 (**T-0616, T-0617, T-0618 → ✅ Verified and archived** (user, 2026-11-04: *"Steps 1-8 on the Mac pass."* — the Mac on a local disk, build 1.0.349 `ef79dca`))*

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

## 🟠 T-0662 — `[Linux]` EP-026 S3: branching — chooser, margin mark, History section, stale purge, Undo History settings

**Epic:** [EP-026] (S3) · **Status:** 🟠 **Implemented - Not Verified** · **Sprint:** [SP-204] →
[`../Sprints/Sprint-SP-204.md`](../Sprints/Sprint-SP-204.md) · **Date Requested:** 2026-11-05 (user: *"plan EP-026 S3"*)

**What:** bind the six branching endpoints; Ctrl+Shift+Z at a branch point opens the branch chooser ([I-0320]); the
branch-point margin mark ([T-0620]'s Linux half); a History section in the Writing tab; Project ▸ Purge Stale History
Branches…; Project Settings ▸ Undo History. `[Apple]` too ([SP-204] Q3): `HistoryCard.swift:207`'s stale-badge text names
Project ▸ Purge Stale History Branches…, not Project Settings. Tested by a new `branch_smoke`, mutation-checked.

**Done (2026-11-05):** built as [SP-204]'s plan; AC1–AC7 met on Docker + macOS evidence (all 40 Linux smoke steps, `branch_smoke`
5/5 mutations caught, ctest macOS 806 + Linux 810, interop 337). Files: `ScriviBridge`, `HistoryCapture`, `EditorShell`,
`ManuscriptEditor`, `HistorySection` (new), `ProjectSettingsDialog`, `ProjectPreferences`, `SceneInspector`, `ScriviWindow`,
`tests/branch_smoke.*`, CMake, Linux CI; `[Apple]` `Scrivi/Views/Inspector/HistoryCard.swift:207`. ⏳ The Mac look at the badge
string (Apple half) · the Linux live pass (AC8 → [EP-057] at close).

---

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

