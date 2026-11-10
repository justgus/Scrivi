---
sprint: SP-209
epic: EP-058
status: Planning
platform: ScriviCore
created: 2026-11-10
---

# SP-209 — `[ScriviCore]` [EP-058] **S1**: Authorship in the core

**Status:** 🔵 **PLANNING** 2026-11-10 (user, 2026-11-10: *"activate EP-058 and plan its first sprint"*) · Q1–Q5 to rule.
**Carries:** [T-0670] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-058] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Design:** [`../Scrivi_Authorship_Design_v0_2.md`](../Scrivi_Authorship_Design_v0_2.md) (✅ approved 2026-11-10) §4.1, §4.4, §6 step 1.
**Unblocks:** [EP-054] AC9 (the byline on every export), and [EP-058] S2 (Apple) / S3 (Linux).
**Size:** M — two schemas read at last, two world files, five endpoints, two fields on open. Core only: no surface, and no
existing caller changes.

---

## What reading the code found (2026-11-10)

- **The two package files exist and are never read.** `identities/project-personas.json` and `project-members.json` are
  written once by `ProjectCreator.cpp:207–232`; their parsers (`ProjectPersonasJson.cpp`, `ProjectMembersJson.cpp`) have no
  caller.
- ⚠️ **`controlledBy` is stored as an array but held as ONE string.** On disk it is `[{"value": "<identityID>"}]`
  (`ProjectPersonasJson.cpp:21`); the parser keeps only item 0 and `PersonaEntry` has a single `controlledByIdentityID`. A
  duo pen name (one author, two identities — design §7 note) needs the whole array → Q5.
- **The identity's name is the host name, recorded at mint.** `IdentityService::ensureLocalIdentity` stores the
  `requestedDisplayName` the app passed (Apple `ProcessInfo.hostName`, Linux "Scrivi") in the SecureStore, and every
  package's first persona copies it. So the names the code has ever written are "Scrivi" and **the identity's recorded name**
  — not necessarily today's host name, if the Mac was renamed since → Q1.
- ⚠️ **Reading that name means reading the SecureStore** — the real login Keychain on Apple, which is why the Keychain tests
  are hidden (`[.keychain]`). `scrivi_open_project` does not read the identity today → Q1.
- **History records store no author.** Nothing in `src/history/` records a `personaID` or `displayName` (the design's §8
  open check). Scene sidecars, chapters, objects and comments record `personaID` + a display name at the time; that stays
  (design §5: history is not rewritten).
- **Worlds have no identity files** (`WorldStore::createWorld`, `WorldStore.cpp:82`), and `scrivi_create_world` takes no
  identity — it has `projectRootPath`, `packagePath`, `displayName`, `epochLabel` → Q2.
- **`scrivi_open_project` already takes `identityID`** (`scrivi.h:45`), so it can answer "writing as" without a signature
  change.
- `scrivi.h` has **144** endpoints today (re-counted); this Sprint adds 5.

## Goal

The core knows, for any project or world, **who its authors are, in order**, and **which of them this identity writes as**;
it keeps the writer's **pen-name menu**; and it tells the app on open when it must ask — so S2 and S3 only draw dialogs, and
[EP-054] can print a byline.

## Plan

0. **Schemas.** `PersonaEntry.controlledBy` becomes a list (Q5); parse/serialize round-trip tests. A
   `scrivi.penNames.v1` schema for the menu (Q4).
1. **`AuthorStore`** (`src/identity/AuthorStore.{hpp,cpp}`), for a package root of either kind (project:
   `identities/project-*.json`; world: `identities/world-*.json`):
   - `authors()` — active personas in file order, each with `unnamed` (Q1);
   - `setAuthors(list)` — the design's §4.1 rules: an entry with a `personaID` keeps it (rename = new `displayName`), one
     without gets a new persona; an active persona left out → `status: "retired"`, never deleted; refused with zero active
     authors, an empty name, or an unknown `personaID`; order = list order (retired entries after the active ones);
   - `writingAs(identityID)` — the member's `defaultPersonaID` when it names an active author, else none;
   - `setWritingAs(identityID, personaID)` — joins the identity as a member if new, sets `defaultPersonaID`, adds the
     identity to that persona's `controlledBy`; refused for a retired or unknown persona.
   Atomic writes, under the package's existing write path (never a second writer — [EP-052]).
2. **Endpoints** (`scrivi.h`, `scrivi_c_api.cpp`), each taking a project OR world root:
   - `scrivi_get_authors(packageRoot, appSupportRoot, identityID, projectRootPath)` →
     `{authors:[{personaID, displayName, unnamed}], writingAs?, authorPrompt}`; `projectRootPath` only for a world (Q2);
   - `scrivi_set_authors(packageRoot, authorsJSON)`;
   - `scrivi_set_writing_as(packageRoot, identityID, personaID)`;
   - `scrivi_get_pen_names(appSupportRoot)` / `scrivi_put_pen_names(appSupportRoot, penNamesJSON)`.
3. **`scrivi_open_project` adds `authors`, `writingAs` and `authorPrompt`** (Q3), read from the package it already opens.
4. **Worlds:** a world without `identities/` gets both files when Q2 says so; `scrivi_create_world` unchanged unless Q2 rules
   (b).
5. **Boundary tests through `scrivi_*`** ([I-0113]): every rule in step 1 for a project AND a world; a co-author with no
   identity (design Q2); a duo author; the unnamed cases; an old package (host-name persona, one member) opens with
   `authorPrompt: "nameAuthor"` and is renamed with its ID kept; the pen-name round trip; refusals. macOS and Linux ctest.
6. **ABI gap audit row** for the five endpoints (Apple and Linux: "S2" / "S3"), so the gap is recorded, not forgotten.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **How does the core know an author is "unnamed"?** The names ever written are "Scrivi" and the identity's recorded name, which lives in the SecureStore — the real Keychain on Apple, so reading it on every open would put the Keychain under every ABI open test. (a) **The core remembers the identity's name from `scrivi_ensure_local_identity`**, which both apps call at launch before any open; unnamed = "Scrivi" or that name. (b) The core reads the SecureStore on each open / get. (c) The app passes the name it wants treated as unnamed. | **(a).** No Keychain on open, no ABI change, and the rule stays in the core. Tests cover "Scrivi" through the ABI on both platforms, and the recorded-name case through `ensure_local_identity` on Linux (its file store) plus one hidden `[keychain]` test on Apple. ⚠️ A co-author's host-name persona made on THEIR machine shows as a name on mine — acceptable: their own machine asks them (design §4.4 "this machine's host name"). |
| **Q2** | **Seeding a world's authors** (design Q5: the creating project's writing-as author). (a) **`scrivi_get_authors` on a world with no `identities/` seeds it** from `projectRootPath`'s writing-as for this identity — one rule for new AND existing worlds. (b) `scrivi_create_world` gains `identityID` and seeds at creation; existing worlds still need (a). | **(a).** Covers both cases with one rule, and leaves `scrivi_create_world` and its two callers alone. |
| **Q3** | **What tells the app to ask?** | **`authorPrompt` on open and on `scrivi_get_authors`:** `"none"`; `"nameAuthor"` when every active author is unnamed (design Q4 — rename it, ID kept); `"chooseAuthor"` when this identity is not a member, or its writing-as author was retired (design Q3). `nameAuthor` wins when both apply. The core decides; the apps only draw. |
| **Q4** | **The pen-name file.** | **`<appSupport>/pen-names.json`**, `scrivi.penNames.v1`: `{"penNames": ["…"], "default": "…"}` — a plain ordered menu of names, the default for new projects and worlds. One machine has one identity, so one list (design Q7). Names only: choosing one copies it into the package as a persona, which has the ID. |
| **Q5** | **`controlledBy` on disk.** | **Keep `[{"value": "<identityID>"}]`** — every existing package has it — and read every item, not item 0. |

## Acceptance Criteria

- [ ] AC1 — `scrivi_get_authors` / `scrivi_set_authors` read and write the ordered authors of a project AND a world per the
      design §4.1 rules: IDs permanent, rename keeps the ID, removed = retired, never zero active authors
- [ ] AC2 — `scrivi_set_writing_as` records membership and writing-as; `writingAs` is returned for this identity, or absent
- [ ] AC3 — `scrivi_open_project` returns `authors`, `writingAs` and `authorPrompt` (Q3); an old package credited to "Scrivi"
      or the identity's name opens with `nameAuthor`, and naming it keeps its `personaID`
- [ ] AC4 — A world gets its author files and first author per Q2
- [ ] AC5 — `scrivi_get_pen_names` / `scrivi_put_pen_names` round-trip the menu outside every package
- [ ] AC6 — Boundary tests through `scrivi_*` for AC1–AC5; macOS and Linux ctest green; the CI sanitizer jobs green
- [ ] AC7 — No existing caller changes: Apple and Linux build and pass unchanged; the gap audit lists the five endpoints

No live pass: nothing a writer can see changes until S2.

## Progress log

- **2026-11-10** — 🔵 **Created in Planning** (user: *"activate EP-058 and plan its first sprint"*), issued via `next-id.py`;
  [T-0670] filed. Q1–Q5 to rule.
