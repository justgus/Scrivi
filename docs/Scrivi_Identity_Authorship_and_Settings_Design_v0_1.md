# Scrivi — Identity, Authorship, Pen Names and Project Settings: Design v0.1

**Status:** 🟡 **DRAFT — for ruling.** ⚠️ **NOT an approved design document.**
**Date:** 2026-09-28
**Codebase:** `[Cross]` — ⚠️ **ScriviCore (schemas + ABI) FIRST, then `[Apple]`, then `[Linux]`.**
**Occasioned by:** ✅ **user rulings 2026-09-28**, given while ruling the scene-divider question:

> ⚠️ *"Use the opaque document pattern, author name is app-level, that stays true, but perhaps we can
> configure a number of 'pen names' that the author uses. So identity remains app-level, and the
> configured pen names are configured app level, but the actual name on the manuscript is set up on a
> per-project basis."*
>
> ⚠️ *"we may need to start to consider how we're going to handle identity for a project that is worked
> on by the same person on different devices… Scrivi will need to be assured that the same actual person
> is modifying the document there. So, perhaps identity is managed app side and authorship is handled
> project side."*

---

## 0. ⚠️ THE HEADLINE — most of this is ALREADY DESIGNED INTO THE CORE

⛔ **This design is mostly NOT new construction. ✅ It is FINISHING something half-built.**

⚠️ **Every claim below is read in the code, with `file:line`.**

| The user's proposal | ✅ Already in ScriviCore | ⛔ Missing |
| ------------------- | ----------------------- | --------- |
| **identity is app-level** | ✅ `IdentityID`, minted once per machine, held in `SecureStore` (`IdentityService.cpp:61`) | — |
| **pen names ("personas")** | ✅ **`scrivi.projectPersonas.v1` EXISTS** — `personaID`, `displayName`, `personaKind`, `status`, `controlledByIdentityID` (`ProjectPersonasJson.cpp:8-19`) | ⛔ **no create/list/edit anywhere; ⛔ not on the ABI** |
| **authorship is project-side** | ✅ **`scrivi.projectMembers.v1` EXISTS** — `identityID`, `role`, `status`, `defaultPersonaID`, `joinedAt` (`ProjectMembersJson.cpp:8-16`) | ⛔ **WRITTEN ONCE at project creation and NEVER READ AGAIN** |
| **the name on THIS manuscript** | ✅ `AuthorshipRef { identityID, personaID, displayName }` is threaded through **every** authorship-bearing ABI call (`Types.hpp:41`; 15 `personaID` parameters in `scrivi.h`) | ⛔ **the app always passes the ONE default persona** |
| **cross-device** | ✅ `WorkspaceState` already carries `deviceID` **and** `identityID` **and** `activePersonaID` (`Types.hpp:54-60`) | ⛔ **`deviceID` is RANDOM PER MACHINE and nothing compares them** |

⚠️ **`ProjectCreator.cpp:101-102` creates BOTH files on every new project** — ✅ `identities/project-members.json`
and `identities/project-personas.json`. ⛔ **Nothing has ever read either.**
⚠️ **This is the `project_capability_without_surface` class** — ✅ **and a large one: two schemas, a
package directory, and fifteen ABI parameters, all built and none reachable.**

---

## 1. ✅ The model, in the user's own terms

```
APP LEVEL  (per machine, per install)          PROJECT LEVEL (in the .scrivi package)
─────────────────────────────────────          ──────────────────────────────────────
Identity                                       project-members.json
  identityID   ← who this human is               identityID   ← which humans may write here
  deviceID     ← which machine                   role/status
  secretMaterial (SecureStore)                   defaultPersonaID

Pen names (the MENU of choices)                project-personas.json
  "Justin Gustafson"                             personaID + displayName
  "J. R. Gustafson"                              controlledByIdentityID
  "Kit Marlowe"                                  status
                                                 ⚠️ THE NAME ON *THIS* MANUSCRIPT
```

✅ **The user's split is exactly the one the core already models:**
⚠️ **IDENTITY answers *"is this the same human?"*** — ⛔ **it is a secret, it is app-level, and it never
belongs in the package.**
⚠️ **AUTHORSHIP answers *"whose name goes on this?"*** — ✅ **it is per-project, and it is public.**

⚠️ **ONE CORRECTION TO THE USER'S FRAMING, AND IT IS SMALL:** ⚠️ *"the configured pen names are
configured app level"* — ⛔ **the core puts `project-personas.json` INSIDE the package.** ✅ **Both are
defensible and §5 Q2 asks which is wanted**, ⚠️ **but note the package copy is what makes a pen name
survive a move to another machine.** ✅ **A sensible answer is BOTH: an app-level menu of the writer's
usual names, COPIED into the project when chosen** — ⚠️ **so the project remains self-describing.**

---

## 2. ⚠️ Cross-device identity — the hard part, and what it actually requires

⚠️ **The user's scenario:** *"If I write some of the-stairs-of-tintagael on my mac, and then transfer
over to the ubuntu box, and continue on it, Scrivi will need to be assured that the same actual person
is modifying the document there."*

### 2.1 ⛔ WHAT HAPPENS TODAY

✅ **`IdentityService::ensureLocalIdentity` (`IdentityService.cpp:61`) mints a FRESH `identityID`
(UUIDv7) and a FRESH random `deviceID` the first time an app runs on a machine**, ⛔ **and stores them
in that machine's `SecureStore`.**

⛔ **SO THE SAME HUMAN ON TWO MACHINES IS TWO DIFFERENT IDENTITIES, AND NOTHING NOTICES.**
⚠️ **The Ubuntu box writes its own `identityID` into the scene sidecars it touches.** ✅ **Nothing
breaks — ⛔ but the project's own record of *who wrote this* is now wrong, and `project-members.json`
(which would have caught it) is never read.**

⚠️ **AND [I-0216] MAKES IT WORSE ON APPLE:** ✅ **`makeSecureStore()` is gated to Linux, so macOS falls
back to the IN-MEMORY `PrototypeSecureStore` and ⛔ RE-MINTS THE IDENTITY EVERY LAUNCH.**
⚠️ **So on Apple today the same human on the SAME machine is a new identity every time.** ⛔ **Any
cross-device design is unbuildable until that is fixed** — ✅ **it is already filed and unruled.**

### 2.2 ⚠️ The three ways to answer *"is this the same person?"*

| Option | How | ⚠️ Cost / risk |
| ------ | --- | -------------- |
| **A — Identity EXPORT / IMPORT** (recommended first step) | ✅ The writer exports an identity bundle from machine 1 (it already exists as a JSON blob in `SecureStore`: `identityID`, `personaID`, `displayName`, `deviceID`, `secretMaterial`) and imports it on machine 2 | ✅ **Small, honest, offline, no account.** ⚠️ The bundle contains `secretMaterial` — ⛔ **it is a CREDENTIAL and must be treated as one** (never in the project, never in Git) |
| **B — The project VOUCHES** | ⚠️ `project-members.json` lists known `identityID`s; a NEW identity writing to the project is flagged and the writer confirms *"yes, that is also me"* | ✅ **Uses the schema that already exists** and makes it a READER at last. ⚠️ **Relies on the writer telling the truth** — ✅ which is fine for a single-author tool, ⛔ not for real multi-author trust |
| **C — Account / key sync** | a Scrivi account, or CloudKit | ⛔ **Enormous.** ⚠️ CloudKit is Apple-only, and this is a cross-platform app by design. ⛔ **Out of scope** |

✅ **RECOMMENDED: A, THEN B.** ⚠️ **A gives the writer a way to BE the same person; ✅ B gives the
project a way to NOTICE when they are not.** ⛔ **Neither needs a server.**
⚠️ **A DEVICE LIST FALLS OUT OF B FOR FREE:** ✅ **`deviceID` is already in `WorkspaceState`, so
"this project has been edited from 2 devices" is readable once anything reads it.**

### 2.3 ⛔ WHAT THIS DESIGN DOES **NOT** CLAIM

⛔ **None of this is SECURITY.** ⚠️ **A local JSON file and a writer's own confirmation prove
*continuity of intent*, not *authentication*.** ✅ **For a single author moving between their own
machines that is the right amount of mechanism.** ⛔ **It must not later be described as proving who
somebody is** — ⚠️ **that claim needs signatures, and signatures need key distribution.**

---

## 3. ✅ Project Settings — the opaque document, as ruled

⚠️ **User: *"Use the opaque document pattern."*** ✅ **That is [SP-141]'s endpoint shape, already built
and already proven on `inspector-layout.json`:**

- ✅ **ONE opaque GET/PUT pair per document; ⚠️ the CORE owns atomicity, durability and repair; ✅ the
  APP owns meaning.**
- ✅ **Absence semantics are ruled: `ok` | `absent` | `unreadable`** — ⛔ **the core never invents
  defaults and never overwrites a corrupt file on read.**
- ⚠️ **ACCEPTED COST, restated: the core cannot validate what it stores there.**

✅ **PROPOSED: `scrivi.projectSettings.v1`, one opaque document per project**
(`scrivi_get_project_settings` / `scrivi_put_project_settings`).

| Setting | ⚠️ Level | Notes |
| ------- | -------- | ----- |
| **scene-break glyph** | project | ✅ `ruled` \| `dashes` \| `asterisks` — ⚠️ **the MENU is app-level, the CHOICE is per-project** |
| **display font + size** | project | ⚠️ **F1 in the rendering study — a PREFERENCE, not a format change.** ✅ The `.md` is untouched |
| **byline (pen name)** | project | ✅ **the chosen `personaID`, resolved against the persona list for a display name** |

⛔ **WHAT DOES NOT GO IN IT:** ⚠️ **`identityID`, `deviceID`, `secretMaterial`** — ✅ **app-level,
`SecureStore`, never in the package.**

⚠️ **MIGRATION NOTE:** ✅ **`ProjectPreferences` (Apple) currently holds `showChapterTitles`,
`projectTitle`, `projectSubtitle` in **`UserDefaults`, keyed by projectID** (`ProjectPreferences.swift:3`).**
⛔ **Those are per-project settings living app-side, which is the same mistake in the other direction:**
⚠️ **they do not travel with the project, so the Ubuntu box cannot see them.** ✅ **They should migrate
into this document** — ⚠️ **and that is a real migration, not a rename; it needs its own AC.**

### 3.1 ⚠️ The scene-break glyph — and why the user's choice of symbols is right

⚠️ **User: *"a ruled line similar to html `<HR>`, a `(- - -)`, or `(* * *)` (I use these because I want
to distinguish them from the Markdown horizontal rules `---` and `***`)."***

✅ **THE DISTINCTION IS EXACTLY RIGHT AND IT IS LOAD-BEARING.** ⚠️ **`---` and `***` ARE Markdown
thematic breaks; `- - -` and `* * *` are ALSO valid CommonMark thematic breaks** (⚠️ **internal spaces
are permitted**) — ⛔ **so on disk they are the same construct.**
✅ **BUT THE SCENE BREAK IS NOT IN THE TEXT AT ALL:** ⚠️ **scenes are SEPARATE FILES, and the divider is
inserted by the VIEW between them** (`ManuscriptTextView.swift:614`). ⛔ **So the glyph is a RENDERING
choice with nothing on disk to collide with** — ✅ **which is what makes this safe, and cheap.**
⚠️ **[Q4 in the rendering study] asks whether it should ever become real text; ✅ this design assumes NO.**

✅ **AND IT SIDESTEPS THE BROKEN DRAWING CODE ENTIRELY** — ⚠️ **[I-0252]: the divider is an
`NSTextAttachment` whose `image(for:)` may never be called under TextKit 2.** ✅ **A glyph is TEXT,
rendered by the same path as every other character.** ⛔ **The attachment can go.**

---

### 3.2 ✅ **RULED 2026-09-28 — the `ProjectPreferences` migration**

⚠️ **User: *"Yes, migrate the ProjectPreferences settings into the new document."*** ✅ **Accepted.**
⛔ **BUT IT IS NOT A THREE-FIELD COPY.** ⚠️ **The three settings turn out to have THREE different
dispositions, and one of them is a live ambiguity ([I-0093]) that a naive move would entrench.**

| Setting | ✅ Disposition | ⚠️ Why |
| ------- | ------------- | ----- |
| **`showChapterTitles`** | ✅ **MOVE — clean** | ⚠️ A pure per-project display preference with no other owner. ✅ It belongs in the document and should travel with the project |
| **`projectSubtitle`** | ✅ **MOVE — clean** | ⛔ **`project.json` has NO subtitle field** (`ProjectJson.cpp:32` writes `title` only). ✅ So `UserDefaults` is its ONLY home today, and it is lost the moment the writer opens the project anywhere else |
| ⚠️ **`projectTitle`** | ⛔ **DO NOT MOVE AS-IS — RESOLVE IT** | ⚠️ **It is NOT a setting. It is a DISPLAY OVERRIDE of `project.json`'s title** |

#### ⛔ `projectTitle` is the one that needs a decision, not a move

✅ **READ — [I-0093] (Verified 2026-07-28) built the current behaviour deliberately:**
⚠️ **`scrivi_open_project` emits `projectTitle` from `project.json`; `ProjectSession` SEEDS
`ProjectPreferences.projectTitle` from it ONLY when `UserDefaults` has no stored value**
(`ProjectPreferences.seedTitleFromSchemaIfUnset`, `:49-55`), ⚠️ **and its comment states the rule:
*"an explicit later rename in Project Settings (persisted to UserDefaults) always wins."***

⛔ **SO TODAY A PROJECT CAN HAVE TWO TITLES: the one in `project.json` (the source of truth per
`CLAUDE.md`) and a per-machine display override that silently beats it.**
⚠️ **MIGRATING THAT OVERRIDE INTO THE PACKAGE WOULD MAKE IT PERMANENT AND PORTABLE** — ✅ **i.e. it
would promote a local quirk into the project's own record, where it would then disagree with
`project.json` on every machine instead of one.**

✅ **RECOMMENDED RESOLUTION (⚠️ needs the user's word — new Q6):**
⛔ **Do not carry `projectTitle` into `scrivi.projectSettings.v1` at all.** ✅ **Renaming a project should
WRITE `project.json`**, ⚠️ **which is the source of truth and already travels with the package.**
✅ **The migration then reads: an existing `UserDefaults` title that DIFFERS from `project.json` is
offered to the writer once — *"this machine shows this project as X; the project says Y. Which is
right?"*** — ⛔ **rather than being silently adopted or silently discarded.**

#### ⚠️ What the migration must actually do

1. ✅ **On open, if `scrivi.projectSettings.v1` is `absent` and a `UserDefaults` blob exists for this
   `projectID`, import `showChapterTitles` + `projectSubtitle` into the document and write it once.**
2. ⛔ **Do NOT delete the `UserDefaults` blob in the same release.** ⚠️ **It is the only copy of the
   writer's real settings until the document is proven to round-trip** — ✅ **and this project has
   already been bitten by a store that dropped keys it did not understand ([I-0215]).**
3. ⚠️ **`projectTitle` per the ruling above.**
4. ✅ **Migration runs ONCE per project and is idempotent** — ⚠️ **a second run must not re-import a
   value the writer has since changed in the document.**

⚠️ **AND A CROSS-PLATFORM NOTE:** ⛔ **Linux has NO `ProjectPreferences` equivalent and therefore
nothing to migrate** — ✅ **it simply starts reading the document.** ⚠️ **So Linux GAINS
`showChapterTitles` for the first time, which is a behaviour change on that platform, not just a
port** (`feedback_linux_adopts_apple_shape`).

---

## 4. ⚠️ Sequencing — and what blocks what

| # | Work | ⚠️ Blocks / blocked by |
| - | ---- | --------------------- |
| **0** | ⚠️ **[I-0254] — fix the Project Settings layout** | ⛔ **PREREQUISITE.** ✅ The sheet already clips its labels; ⚠️ **adding three more rows to a sheet that cannot lay out its current ones makes it worse** |
| **1** | ✅ **`scrivi.projectSettings.v1` + GET/PUT (opaque)** | ✅ **ScriviCore. Blocks everything below** |
| **2** | ✅ **Scene-break glyph** (setting + render) | ⛔ **Closes [I-0252] by REPLACING the attachment** |
| **3** | ✅ **Display font + size** (F1) | ⚠️ Independent of the rendering Epics |
| **4** | ✅ **Expose personas: create / list / choose** | ⚠️ **ABI work — the schema exists, the endpoints do not** |
| **5** | ⚠️ **Read `project-members.json`** (option B) | ⛔ **Blocked by [I-0216]** — ✅ identity must PERSIST on Apple before membership means anything |
| **6** | ⚠️ **Identity export/import** (option A) | ⛔ **Blocked by [I-0216]** |

⛔ **[I-0216] IS THE GATE ON THE WHOLE CROSS-DEVICE HALF**, ✅ **and it is already filed and unruled:**
⚠️ **`makeSecureStore()` is gated to Linux, so Apple re-mints identity every launch.**

⚠️ **SIZE: this is more than one Epic.** ✅ **Items 1–3 are a coherent `[Cross]` settings Epic; ⚠️ items
4–6 are an identity Epic that cannot start until [I-0216] is ruled.**

---

## 5. ⚠️ Questions for the user

| # | Question | ⚠️ Why it is yours |
| - | -------- | ------------------ |
| ~~**Q1**~~ | ✅ **RULED 2026-09-28 (user): YES, MIGRATE.** ⚠️ **See §3.2 — the three settings have THREE DIFFERENT dispositions, and that had to be worked out before the ruling could be executed** | ✅ **CLOSED** |
| **Q2** | ⚠️ **Pen names: app-level menu, per-project copy, or BOTH?** ✅ **Recommended: both** — an app-level menu, copied into the project when chosen | ⚠️ The core already puts `project-personas.json` IN the package; ✅ that is what makes a project self-describing on another machine |
| **Q3** | ⚠️ **Cross-device: export/import (A) then project-vouches (B)?** ✅ **Recommended** | ⛔ Option C (accounts/CloudKit) is out of scope and Apple-only |
| **Q4** | ⚠️ **Is `personaKind` meaningful to you?** ✅ The schema already has the field | ⛔ Unknown what values were intended — ⚠️ it may be vestigial |
| **Q5** | ⚠️ **Should the byline appear anywhere yet** (export, title page, nowhere)? | ⛔ **[EP-032] Q5 already records that manuscript EXPORT has no existing path.** ⚠️ A byline with nowhere to print is the *capability-without-surface* class again |
| **Q6** | ⚠️ **`projectTitle` (§3.2): does renaming a project WRITE `project.json`, ✅ or does a per-project display override survive in the settings document?** ✅ **Recommended: write `project.json`; ⛔ do not migrate the override** | ⚠️ **Raised BY the Q1 ruling.** ⛔ Today a project can carry two disagreeing titles and the local one silently wins ([I-0093]) — ✅ migrating it would make that portable instead of local |

---

## 6. ✅ What was read, and what was not

✅ **READ:** `ScriviCore/include/scrivi/Types.hpp:41-60` · `ProjectPersonasJson.cpp` ·
`ProjectMembersJson.cpp` · `IdentityService.cpp:47-115` · `SystemUUIDProvider.cpp:71-77` ·
`ProjectCreator.cpp:95-115` · `scrivi.h` (15 `personaID` parameters; ⛔ **no persona endpoints**) ·
`Scrivi/App/AppEnvironment.swift:196-215` · `Scrivi/App/ProjectPreferences.swift` ·
`Scrivi/Views/ProjectSettingsSheet.swift`.

⛔ **NOT READ / NOT DONE:**
- ⛔ **[I-0254] was NOT reproduced in the running app.** ⚠️ **The clipping cause in its record is READ
  FROM CODE, not measured** — ✅ and on 2026-09-28 two carefully-measured divider diagnoses each
  addressed the wrong layer, so this one is explicitly marked a hypothesis.
- ⛔ **No Linux investigation.** ⚠️ **Linux has its own identity bootstrap and its own settings surface;
  ✅ `feedback_linux_adopts_apple_shape` applies and this design does not yet cover it.**
- ⛔ **`personaKind`'s intended values were not determined** — ⚠️ **the field is written and never read,
  so the code cannot say what it meant** (Q4).
- ⛔ **No assessment of what happens to EXISTING projects' `project-members.json`** — ⚠️ **they all have
  one, written at creation, listing a single identity that on Apple is now STALE (re-minted every
  launch, [I-0216]).** ✅ **A migration/repair question this design does not answer.**
