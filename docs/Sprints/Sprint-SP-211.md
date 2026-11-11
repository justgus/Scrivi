---
sprint: SP-211
epic: EP-058
status: Active
platform: Linux
created: 2026-11-11
---

# SP-211 — `[Linux]` [EP-058] **S3**: Authorship on Linux

**Status:** 🟡 **ACTIVE** 2026-11-11 (user, 2026-11-11: *"Q1–Q3 as recommended, activate and implement SP-211"*) · ✅ Q1–Q3 ruled as recommended · ✅ built (AC1–AC5) · ⏳ CI (AC6) and the live pass (AC7).
**Carries:** [T-0672] → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md).
**Epic:** [EP-058] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) (AC3, AC4 and AC8 on Linux; AC7).
**Design:** [`../Scrivi_Authorship_Design_v0_2.md`](../Scrivi_Authorship_Design_v0_2.md) §4.2, §4.4.
**Shape:** [SP-210] (✅ closed) — Linux adopts the Mac's surfaces, wording and rules (Q2–Q6 there carry over unchanged).
**Size:** M — Linux already keeps one bridge (and so one author) per window; the work is mostly the four surfaces.

---

## What reading the code found (2026-11-11)

- **The author is already per window.** Each `EditorShell` owns its own `ScriviBridge` (`EditorShell.cpp:109`), and the
  bridge threads the identity into every authored call ("a QML caller cannot pass the wrong author because it cannot pass
  one", `ScriviBridge.hpp:523`). Workers copy it (`WorkerBridge`, [I-0309]). So S3's model change is one call: after the
  open, the window's bridge takes the open's `writingAs` as its `personaID` / `displayName`. No call site changes.
- **Every Linux project is credited to "Scrivi".** `EditorShell::load` bootstraps the identity as `"Scrivi"`
  (`EditorShell.cpp:457`), so every existing Linux project will ask *Who wrote…?* once — as the Mac's host-named ones did.
- **The VM is a second machine for free.** Linux keeps its identity in `EncryptedFileSecureStore`, not the Mac's keychain,
  so a Mac project opened on scrivi-vm asks *Which of these authors are you?* — AC8's second-machine step.
- **There is no app Settings on Linux.** Project ▸ Project Settings… has no shortcut (`ScriviWindow.cpp:544`); the menu bar
  is the same `QMainWindow` on the landing page and in a project, so an app-level item is reachable from both → Q1, Q2.
- **The surfaces fit where the Mac's do.** `ProjectSettingsDialog` is `QGroupBox` sections (Project · Writing Surface ·
  Typography · Undo History) — Authors goes after Project. `WorldsDialog` rows have Locate… / Remove — Authors… joins them.
  New Project is QML (`NewProjectDialog.qml`, `bridgeRef.createProject`) — the Author field goes there.
- **The bridge has none of the five endpoints** (`scrivi_get_authors` … `scrivi_put_pen_names`).

## Goal

The Mac and iPad's authorship on Linux, in their shape and wording: pen names, a project's and a world's ordered authors,
every edit recorded as the window's author, and the two questions on open.

## Plan

0. **Bridge** (`ScriviBridge.{hpp,cpp}`): `getAuthors`, `setAuthors`, `setWritingAs`, `getPenNames`, `putPenNames`
   (Q_INVOKABLE, identity threaded by the bridge as everywhere else); `adoptWritingAs(personaID, displayName)`. The open's
   `authors` / `writingAs` / `authorPrompt` already pass through its map.
1. **The window's author** (`EditorShell`): after the open, adopt `writingAs`; with *Which of these authors are you?* pending,
   the question is up before the manuscript takes a keystroke (SP-210 Q4).
2. **New Project** (`NewProjectDialog.qml`): an **Author** field, filled with the default pen name, with the pen names to
   pick from; the name passed to `createProject`, and a new one joins the menu (SP-210 Q2, Q3).
3. **Settings ▸ Pen Names** (Q1): a `Settings` dialog with a Pen Names tab — add, rename, reorder, remove, make default.
4. **Project Settings ▸ Authors**: the ordered list (add from the pen names or type one, rename, move, remove — never the
   last) and **I write as**.
5. **Worlds ▸ Authors…** on each available world, with the project's root (the core seeds a world without authors).
6. **The two questions on open**: *Who wrote “Title”?* (one field per unnamed author, SP-210 Q6) and *Which of these authors
   are you?* (the authors and **Someone else…**) — a modal dialog with no Cancel; Esc and the close button do nothing.
7. **Tests:** an `authorship_smoke` THROUGH the bridge (each call; both questions' writes; a scene saved after adopting is
   recorded as that author, read back from the sidecar); the existing smokes and ctest in Docker; CI.
8. **Live pass** (user) on scrivi-vm.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Where Settings lives on Linux.** The Mac's is in the app menu, which Linux has no equivalent of. (a) **Edit ▸ Settings… (Ctrl+,)**, a dialog with a Pen Names tab; (b) File ▸ Settings…; (c) a Settings menu of its own (KDE's shape). | **(a).** Edit is where Qt and GNOME-era apps put their settings, Ctrl+, is the shortcut Mac users already know, and "Settings" is the Mac's word. It is in the one menu bar, so it works from the landing page too. |
| **Q2** | **Project Settings' shortcut.** The Mac's is ⌥⌘, since SP-210; Linux has none. (a) **Ctrl+Alt+,** (the Mac's, translated); (b) none, as today. | **(a)**, so the two settings sit side by side as on the Mac. It has no Shift, so the VNC path carries it; ⚠️ a clash with a desktop shortcut is checked in the live pass. |
| **Q3** | **The pen names' editor on the landing page.** The iPad gained a gear on its Launch panel; Linux's landing page has the menu bar. (a) **The menu only** (as the Mac's Welcome window); (b) also a Settings button on the landing page. | **(a).** The Mac is the shape Linux follows, and Edit ▸ Settings… is on screen at the landing page already. |

## Acceptance Criteria

- [x] AC1 — The bridge wraps the five endpoints and adopts the open's `writingAs`; `authorship_smoke` through the bridge
- [x] AC2 — Every edit in a window is recorded under that project's writing-as author (a sidecar read back after a save)
- [x] AC3 — New Project credits the project to the chosen pen name
- [x] AC4 — Settings ▸ Pen Names (Q1), Project Settings ▸ Authors with I write as, and Worlds ▸ Authors…, in the Mac's
      wording; never the last author removed; an unnamed author never shown as a name
- [x] AC5 — *Who wrote…?* and *Which of these authors are you?* on open, unanswerable-away as on the Mac
- [ ] AC6 — ctest, every Linux smoke and the Linux CI green; Docker build clean — ✅ ctest 839/839 and smokes 43/43 on scrivi-vm; ⏳ CI after the push (Docker was not running: built on scrivi-vm instead)
- [ ] AC7 — **Live pass (user), scrivi-vm:** set a pen name; a new project credited to it; a second author added and
      reordered; a world credited; an old Linux project (credited to "Scrivi") asks once and keeps its history; a project
      made on the Mac asks *Which of these authors are you?* and writes as the answer

## Progress log

- **2026-11-11** — 🔵 **Created in Planning** (user: *"close SP-210 and plan EP-058 S3"*), issued via `next-id.py`; [T-0672]
  filed. Q1–Q3 to rule.
- **2026-11-11** — ✅ Q1–Q3 ruled as recommended. 🟡 **Activated** (user, 2026-11-11: *"Q1–Q3 as recommended, activate and implement SP-211"*).
- **2026-11-11** — ✅ **Built.** the bridge's five calls, `adoptWritingAs` and `rememberPenName`, and New Project's author through
`createProject` (`ScriviBridge.*`); the window adopts the open's `writingAs` and asks on a first load (`EditorShell::askAuthor`);
`src/AuthorshipDialogs.{hpp,cpp}` (new) — Settings ▸ Pen Names, Project Settings ▸ Authors with I write as, Worlds ▸ Authors…,
the two questions (no Cancel; Esc and the close button do nothing); Edit ▸ Settings… (Ctrl+,), Project Settings… (Ctrl+Alt+,)
(`ScriviWindow.cpp`); the Author field (`NewProjectDialog.qml`). Evidence (scrivi-vm, scratch copy of the working tree):
`authorship_smoke` (new, in CI); ctest 839/839; every Linux smoke 43/43 — three fixtures (`settings`, `indent`, `replace`)
now name their author, as a project credited to "Scrivi" correctly opens *Who wrote…?* over the window they type into.
Boundary and ABI guard checks clean. ⏳ Linux CI after the push.
  T-0672 → 🟠.
