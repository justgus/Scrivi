---
sprint: SP-210
epic: EP-058
status: Active
platform: Apple
created: 2026-11-11
---

# SP-210 — `[Apple]` [EP-058] **S2**: Authorship on the Mac and iPad

**Status:** 🟡 **ACTIVE** 2026-11-11 · ✅ built (AC1–AC6) · ⏳ AC7 live passes (user) · (user, 2026-11-11: *"(a), agree with the iPad changes, activate SP-210"*) · ✅ Q1–Q6 ruled.
**Carries:** [T-0671] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-058] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) (AC3, AC4, AC6; AC8's Mac and iPad half).
**Design:** [`../Scrivi_Authorship_Design_v0_2.md`](../Scrivi_Authorship_Design_v0_2.md) §4.2, §4.4.
**Builds on:** [SP-209] (S1, ✅ closed): `scrivi_get_authors`, `scrivi_set_authors`, `scrivi_set_writing_as`,
`scrivi_get_pen_names`, `scrivi_put_pen_names`, and `authors` / `writingAs` / `authorPrompt` on `scrivi_open_project`.
**Size:** L — one model change threaded through the edit paths, three surfaces, two prompts, on two platforms.

---

## What reading the code found (2026-11-11)

- **The author an edit is recorded under is app-global.** `AppEnvironment.bootstrap()` builds ONE `AuthorshipRef` from
  `scrivi_ensure_local_identity` (`AppEnvironment.swift:222–233`; `displayName` = `ProcessInfo.hostName`). It reaches the
  edit paths two ways:
  - `ProjectSession.authorshipRef` — a `let`, copied from the environment when the session is MADE
    (`AppEnvironment.makeSession`, `:398`), before the project is opened; used by the viewport saves (`ProjectSession.swift:494, 559`);
  - **7 sites read `env.authorshipRef` directly**: `EditorView.swift` ×3 (passed on to the Scene Inspector, the Timeline
    strip and the object cards), `ManuscriptTextView.swift` ×3 (structural edits, range cut/paste/relabel, saves),
    `ManuscriptEditing.swift` ×1.
  So S2's model change is: build the ref from the open's `writingAs`, hold it on the session, and move those 7 reads to the
  session. Every write call already takes the ref as a parameter (design §4.2).
- **A new project is credited to the host name.** `AppEnvironment.createProject` (`:652`) passes the app-global ref, so
  `scrivi_create_project` writes a persona named e.g. `flitwick-5.local` — which S1 now reports as `unnamed`, so every new
  project would open straight into the "who wrote this?" prompt → Q3.
- **There is no app-level Settings window.** The Mac has no `Settings { }` scene; ⌘, is **Project Settings…**
  (`ScriviApp.swift:355`). iPad has Project Settings… without ⌘, (iOS reserves it, `:544`) and no app settings at all → Q1.
- **Project Settings** is one `Form` sheet (`ProjectSettingsSheet.swift`: Project · Writing Surface · Typography · Undo
  History), shared by the Mac and iPad — an **Authors** section fits there.
- **Worlds** is one sheet per project (`WorldsView.swift`: the bound worlds, Create… / Add Existing World… / Reconnect,
  per-row Remove and Locate…), shared by the Mac and iPad — a per-world **Authors…** action fits there.
- **`OpenProjectResult` does not decode the new fields** (`ScriviEngine.swift:2320`), and the engine has no wrapper for the
  five endpoints (nor the visionOS stub).
- **Nothing in the app prints an author's name today** (no view reads a persona `displayName`), so the "never show
  unnamed" rule (AC4) is met by the new surfaces alone; the byline on exports is [EP-054] AC9.

## Goal

On the Mac and iPad, a writer names the pen names they write under, credits each project and world to its authors in
order, and every edit in a project is recorded as the right author — and Scrivi asks, once, whenever it cannot know.

## Plan

0. **Engine** (`ScriviEngine.swift` + the visionOS stub): `getAuthors`, `setAuthors`, `setWritingAs`, `getPenNames`,
   `putPenNames`; `OpenProjectResult` gains `authors`, `writingAs`, `authorPrompt` (absent = `"none"`, S1's damaged-file
   rule). Interop tests through the engine.
1. **Per-project author.** `ProjectSession.authorshipRef` becomes a `var` set from `writingAs` at open (and after either
   prompt is answered); the 7 `env.authorshipRef` reads move to the session. Before an answer, per Q4.
2. **New projects** (`NewProjectSheet`, `AppEnvironment.createProject`): an **Author** field, filled with the default pen
   name, passed as the persona's name (Q3).
3. **Settings ▸ Pen Names** — add, rename, remove, reorder, mark the default (Q1): a Mac `Settings` scene (⌘,; Project
   Settings → ⌥⌘,), and on iPad the same view as a sheet from the Launch toolbar and the ••• menu.
4. **Project Settings ▸ Authors:** the ordered list (add from the pen names or type a name, rename, reorder, remove — never
   the last) and **I write as** (this machine's author).
5. **Worlds ▸ Authors…:** the same list for a world (`scrivi_get_authors` with the project's root, so a world without
   authors takes the project's author, S1 Q2), and the world's I write as.
6. **The prompts on open** (Q2): `nameAuthor` — *"Who wrote “Title”?"*, prefilled with the default pen name, renames the
   unnamed author (ID kept); `chooseAuthor` — *"Which of these authors are you?"* with the authors and **Someone else…**.
7. **Tests and checks:** interop tests for every engine call and both prompts' writes; ManuscriptEdit/structural paths
   recorded under the session's author (a sidecar's `modifiedByPersonaID` after an edit); stub parity, boundary and ABI
   guards; Mac and iOS builds.
8. **Live passes** (user), on the Mac and the iPad.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Where the pen-name list lives.** | ✅ **Ruled 2026-11-11 (user's design, amended):** Mac — a real **Settings** window (Scrivi ▸ Settings…, **⌘,**), Pen Names its first tab, room for later app settings; **Project Settings… moves to ⌥⌘,**. iPad — the same tabbed Settings view as an in-app sheet, from a **Settings** button on a new Launch-panel toolbar and from the project's ••• menu (a system Settings bundle cannot edit a list, and would keep it in `UserDefaults`, not `pen-names.json`). Check whether the iPad menu bar's system Settings… item can open it. |
| **Q2** | **The first run** (design §4.4: "the first run after this ships asks once for the writer's pen name"). (a) **Fold it into the prompts:** with an empty menu, *Who wrote…?* and New Project ask for a name and add it to the menu (and make it the default). (b) A separate first-run sheet at launch. | **(a).** One fewer dialog, and it asks at the moment the name is needed; the menu is never empty after the first answer. |
| **Q3** | **A new project's author.** | **An Author field in New Project, filled with the default pen name** (editable; a new name joins the menu). The project's first persona is then a real name, so it never opens into the prompt. |
| **Q4** | **Edits made while a prompt is still unanswered** (the writer dismisses it, or types before answering). | **The prompt is a sheet the writer cannot dismiss without answering** — *Who wrote…?* needs a name; *Which author are you?* lists the authors and Someone else…. So no edit happens as no one. ⚠️ Alternative: let it be dismissed and record edits under the identity's own persona, as today — but then the host name keeps being written into sidecars. |
| **Q5** | **Edits to a world's objects** (made from a project): recorded under the project's author, or the world's? | **The project's writing-as author**, as today (one ref per session). The world's own authors are its byline; per-world attribution of edits would mean a second ref per session for a gain no one has asked for. |
| **Q6** | **Several unnamed authors** (*Who wrote…?* when a project has more than one unnamed author, e.g. two machines both credited by host name). | **Ask for each, in byline order, in one sheet** (one field per unnamed author, the first prefilled with the default pen name). Rare, but never leave a host name in a byline. |

## Acceptance Criteria

- [x] AC1 — The engine wraps the five endpoints and the open's three fields (+ visionOS stub); interop tests through it
- [x] AC2 — Every edit in a project — scenes, structure, objects, images, timeline — is recorded under that project's
      writing-as author, from `writingAs`, not the app-global identity
- [x] AC3 — New Project credits the project to the chosen pen name (Q3)
- [x] AC4 — Pen Names (Q1), Project Settings ▸ Authors with I write as, and Worlds ▸ Authors… on the Mac and iPad; never the
      last author removed; an unnamed author never shown as a name
- [x] AC5 — *Who wrote…?* and *Which of these authors are you?* on open, per Q2, Q4, Q6
- [x] AC6 — Interop and ScriviCore tests green; stub parity, boundary and ABI guards clean; Mac and iOS builds clean
- [ ] AC7 — **Live passes (user), Mac and iPad:** set a pen name; a new project credited to it; a second author added and
      reordered; a world credited; an old project (credited to the host name) asks once and keeps its history; a project made
      on the Mac, opened on the iPad (its own identity), asks *Which of these authors are you?* and writes as the answer

## Progress log

- **2026-11-11** — 🔵 **Created in Planning** (user: *"plan EP-058 S2"*), issued via `next-id.py`; [T-0671] filed. Q1–Q6 to
  rule.
- **2026-11-11** — Q2–Q6 ruled as recommended. Q1: user proposed a Mac Settings window and an iPad Settings bundle; evaluated —
  the bundle cannot hold the list (see Q1).
- **2026-11-11** — ✅ Q1 ruled (Mac Settings window on ⌘,, Project Settings → ⌥⌘,; iPad in-app sheet from the Launch
  toolbar and •••). 🟡 **Activated** (user, 2026-11-11: *"(a), agree with the iPad changes, activate SP-210"*).
- **2026-11-11** — ✅ **Built.** engine wrappers + `Author` / `AuthorsResult` / `AuthorEdit` / `PenNames` and the open's three fields
(`ScriviEngine.swift`, stubs); the window's author from `writingAs` (`ProjectSession.authorshipRef` / `adoptWritingAs`, the 7
`env.authorshipRef` reads moved to the session); New Project's Author (`NewProjectSheet.swift`, `AppEnvironment.createProject`);
`Scrivi/Views/AuthorshipViews.swift` (new) — App Settings ▸ Pen Names, Project Settings ▸ Authors and I write as, Worlds ▸
Authors…, the two prompts; Mac `Settings` scene on ⌘, (Project Settings → ⌥⌘,); iPad Launch toolbar ⚙, the ⋯ menu's Scrivi
Settings, the menu bar's Settings… (⌘,). Evidence: `AuthorshipInteropTests` 7/7; macOS interop 347/347; iOS 19/19; ScriviCore
ctest 834/834; Mac and iOS builds clean (no warnings); stub parity, boundary, ABI guards, TextKit 2, layout and type-source
checks clean. visionOS still fails only on [I-0356] (`EditorView` inspector, predates this).
  T-0671 → 🟠.
