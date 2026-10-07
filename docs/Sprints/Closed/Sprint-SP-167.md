---
sprint: SP-167
epic: EP-047
status: Closed
closed: 2026-10-07
activated: 2026-10-07
platform: Cross
created: 2026-10-07
---

# SP-167 — `[ScriviCore]` + `[Apple]` [EP-047] **S1**: project settings that TRAVEL with the project

**Status:** ✅ **CLOSED 2026-10-07 (user-approved):** *"close SP-167 and begin S2"* — activated 2026-10-07 (user: *"this plan is approed. I am ready."*). ✅ Q1–Q2 ruled 2026-10-07. ✅ All ACs met; T-0596 and I-0278 VERIFIED and archived.
**Tasks:** ✅ [T-0596] → [`../../Tasks/Verified/Task-verified-0596.md`](../../Tasks/Verified/Task-verified-0596.md) · **Issues:** ✅ [I-0278] → [`../../Issues/Verified/Issue-verified-0271-0280.md`](../../Issues/Verified/Issue-verified-0271-0280.md)
**Epic:** [EP-047] `[Apple]` Manuscript Typography & Preferences → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). **EP-047's first Sprint.**
**Size:** M. ✅ Needs no rig: the live pass is on the Mac.

---

## ⚠️ What reading the code found (2026-10-07)

- `ProjectPreferences.swift` holds three settings in `UserDefaults` under `scrivi.project.<id>.preferences`. Readers:
  `ProjectSession.swift:304-309` (creation + `seedTitleFromSchemaIfUnset`), `ProjectWindowManager.swift:129, :282`, `EditorView.swift`
  (title, subtitle, `showChapterTitles` → the manuscript), `SceneNavigatorView.swift`, `ProjectSettingsSheet.swift`.
- ✅ **The pattern to copy exists:** `scrivi_get/put_inspector_layout` (`scrivi_c_api.cpp`) store an OPAQUE app-owned JSON object at the
  package root; `absent` ≠ `unreadable`; never overwritten on read. ⚠️ PUT replaces the whole document, so Apple must READ-MERGE-WRITE
  to keep keys Linux (or a later Scrivi) wrote — [I-0215]'s lesson.
- ✅ **Only `ProjectCreator` writes `project.json`**; `parseProject` → `serializeProject` would DROP unknown fields, so the title is set
  by editing the JSON document in place. The external-change scanner reads only `projectID` from it, so an in-app write is safe.
- "Find stale branches" sits in `ProjectSettingsSheet.swift` as a section with Purge buttons: an ACTION in a settings sheet.

## Goal

✅ **Title, subtitle and "Show chapter titles" live in the project**, so they are the same on every Mac (and readable by Linux). ✅ The
stale-branch purge becomes a menu action.

## EP-047 ACs this Sprint meets

| AC | Criterion (Epic wording, short) |
| -- | ------------------------------- |
| **AC1** | `project-settings.json` via `scrivi_get/put_project_settings`; `absent` ≠ `unreadable`; Linux binds both; the package doc lists it |
| **AC2** | A core endpoint writes `project.json`'s `title`; Project Settings writes it; every display reads it |
| **AC3** | [I-0278] migration once, no renamed title lost; the `UserDefaults` key retired; "Find Stale Branches" re-homed (Q1) |

## ✅ Questions — RULED 2026-10-07

| # | Question | Ruling |
| - | -------- | ------ |
| **Q1** | Where "Find stale branches" goes | ✅ **Project menu**: "Purge Stale History Branches…" opens its own sheet; the "Stale after (days)" threshold stays in Project Settings |
| **Q2** | Migration when this Mac's old values and the package disagree | ✅ **The package wins** for subtitle and "Show chapter titles". **Title:** a rename made on this Mac (old title ≠ `project.json`'s, and the package has no settings yet) is written to `project.json` ONCE; after that `project.json` wins. The old key is deleted after a migration |

## Plan

1. **Core:** `scrivi_get_project_settings` / `scrivi_put_project_settings` (opaque object, the inspector-layout contract; ONE shared helper
   for both files so the two contracts cannot drift) · `scrivi_set_project_title` (edits `project.json`'s `title` in place, atomic;
   rejects an empty title). Catch2 through the ABI.
2. **Linux:** bind the three endpoints in `ScriviBridge`; exercise them in `bridge_parity_smoke`. (Linux UI: ⛔ not this Sprint.)
3. **Apple:** engine methods + visionOS stubs; `ProjectPreferences` backed by the package — whole document kept, its own keys merged in;
   title from `project.json`, renames written through the core; the Q2 migration; `ProjectSession` wiring.
4. **Q1:** "Purge Stale History Branches…" in the Project menu, its own sheet; the section leaves Project Settings.
5. **Docs:** package structure doc lists `project-settings.json` and its keys.
6. **Tests** (interop): round trip; unknown keys survive an Apple write; each Q2 case; the title reaches `project.json`.
7. **Live pass** (user, on the Mac) — steps in the chat reply.

## ✅ Results (2026-10-07)

**Core:** the inspector layout's two endpoint bodies became ONE helper pair (`getOpaqueDocument` / `putOpaqueDocument`, by file name),
now serving `scrivi_get/put_inspector_layout` AND `scrivi_get/put_project_settings` · `scrivi_set_project_title` (edits `project.json`
in place; rejects a blank title) · `ProjectSettingsCApiTests.cpp` (7 cases). **Linux:** `ScriviBridge::getProjectSettings /
putProjectSettings / setProjectTitle`; `bridge_parity_smoke` checks them by reading back. **Apple:** engine methods + stubs (reusing
the inspector layout's fetch/save types by `typealias`); `ProjectPreferences` rewritten on the package (whole document kept; Q2
migration; title through the core); `ProjectSession` wiring; **`StaleBranchesSheet.swift`** (new, ✅ in `project.pbxproj`, all three
targets) + "Purge Stale History Branches…" in the Project menu (macOS and iOS); the section left Project Settings · package doc updated.

| Check | Result |
| ----- | ------ |
| ctest macOS | ✅ 672/672 (+7) |
| ctest Linux (Docker, uid 1000) | ✅ 676/676 |
| Interop (full) | ✅ 215/215 in 25 suites (+7: `ProjectSettingsTravelTests`) |
| Linux smokes (incl. `bridge_parity_smoke` 31 checks) | ✅ 27/27 |
| Mutations — core (3) | ✅ 3/3 killed (settings reading the layout file; a blank title accepted; the title write dropping other fields) |
| Mutations — migration (3) | ✅ 3/3 killed (this Mac always wins; migrating over an unreadable file; a save dropping another platform's keys) |
| Engine stub parity | ✅ clean |

⚠️ **Two test-harness traps, recorded:** (1) zsh does NOT word-split an unquoted `$VAR`, so several `-only-testing` flags in one
variable ran 0 tests and "passed" — use `${=VAR}`; (2) xcodebuild prints an empty "0 tests" run first — read the LAST summary line.

### ⏳ Live pass (user, on the Mac)
1. Open an existing project you have renamed before in Project Settings. The window title shows YOUR title (migrated), not the
   original one; the subtitle and "Show chapter titles" are as you left them.
2. Project ▸ Project Settings… — rename the project, set a subtitle, toggle "Show chapter titles". Close and reopen the project —
   all three are kept. In Finder, the package's `project.json` has the new `title`; `project-settings.json` exists beside it.
3. The "Stale Branches" section is GONE from Project Settings; "Stale after (days)" is still there.
4. Project ▸ Purge Stale History Branches… — the sheet lists stale branches (or "No stale branches."); Rescan and Done work; a Purge
   asks for confirmation.
5. (Optional, the travel test) Copy the package to another location or Mac and open it — title, subtitle and toggle come with it.

## Acceptance Criteria

- [x] AC1: endpoints + Catch2 (absent / unreadable / ok / non-object rejected); Linux bridge + parity smoke; package doc
- [x] AC2: `scrivi_set_project_title`; Project Settings rename reaches `project.json`; the title displays read `ProjectPreferences`, now `project.json`'s
- [x] AC3: migration per Q2 (tests for each case); key retired; the stale-branch purge in the Project menu
- [x] ctest, interop, Linux smokes green
- [x] Live pass (user, Mac) — ✅ 2026-10-07: *"all live tests pass."*
