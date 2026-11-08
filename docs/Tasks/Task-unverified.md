# Unverified Tasks

*Last Updated: 2026-11-07 (**T-0665:** two export defects from the live pass fixed in the core.)*

*Last Updated: 2026-11-06 (**T-0665 → 🟠 Implemented - Not Verified** ([SP-206]); the live passes (AC7) are the user's.)*

*Last Updated: 2026-11-06 (**T-0664 → ✅ Verified** (user, 2026-11-06: *"verify T-0664"*) and archived → `Verified/Task-verified-0664.md`.)*

*Last Updated: 2026-11-06 (**T-0664**: [SP-205] closed; stays 🟠 — the user verifies it, at the latest in [SP-206]'s live passes.)*

*Last Updated: 2026-11-06 (**T-0664 → 🟠 Implemented - Not Verified** ([SP-205]); core only — the live passes are [SP-206]'s.)*

*Last Updated: 2026-11-06 (**T-0611 → ✅ Verified and archived** → `Verified/Task-verified-0611.md` ([SP-184] closed).)*

*Last Updated: 2026-11-05 (**T-0611 → 🟠** ([SP-184], EP-048 S5c).)*

*Last Updated: 2026-11-05 (**T-0610 → ✅ Verified and archived** → `Verified/Task-verified-0610.md` ([SP-183] closed).)*

*Last Updated: 2026-11-05 (**T-0610 → ✅ Verified** (user, 2026-11-05: *"all three re-test steps passed."* · *"C1, C2, C3, and C6 all passed."*); archived when [SP-183] closes.)*

*Last Updated: 2026-11-05 (**T-0610 → 🟠** ([SP-183], EP-048 S5b).)*

*Last Updated: 2026-11-05 (**T-0662 → ✅ Verified and archived** → `Verified/Task-verified-0662.md` ([SP-204] closed).)*

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

## 🟠 T-0665 — `[Cross]` EP-054 S2: Export Archive… / Import Archive… on the Mac, Linux and iPad

**Epic:** [EP-054] (S2) · **Status:** 🟠 Implemented - Not Verified · **Sprint:** [SP-206] (🟡 Active) →
[`../Sprints/Sprint-SP-206.md`](../Sprints/Sprint-SP-206.md) · **Date Requested:** 2026-11-06 (user: *"You may complete the planning for EP-054."*)

**What:** the two archive endpoints on each platform's own file surfaces — File menu on the Mac and Linux, the share sheet
and Files on iPad/iPhone — off the main thread with progress, asking whether to include the world (Q3) and where an
unknown world goes (Q8). Live-passed across machines: Mac → Linux, Linux → Mac, Mac → iPad.

**Implemented (2026-11-06, [SP-206]):** `Scrivi/App/ProjectArchiveFlow.swift` (Mac + iOS: Export / Import, the world question,
Cancel / Rename / Replace, Q1, Q3), `ScriviEngine.exportArchive` / `importArchive` (`ScriviEngine.swift`), File menu + toolbar
Export slot + Landing's Import button, `Info.plist` (`com.caposoft.scrivi.archive`); Linux `EditorShell::exportArchive`,
`ShellController::importArchive` (`ScriviWindow.cpp`), `ScriviBridge::exportArchive` / `importArchive`, Landing.qml. Evidence:
interop `archiveRoundTripThroughEngine` ✅, `bridge_parity_smoke` 70/70 ✅ (Docker). ⏳ AC7 live passes (user).
**Fixed (2026-11-07, live pass):** the export refused a trailing-slash project root (Mac: *"invalid filename"*) and reported
a removed reference's leftover `worlds/` folder as an unreachable world (Linux) — `ProjectArchive.cpp` (`folderPath`,
`listWorlds`); `ArchiveCApiTests` covers both.

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

