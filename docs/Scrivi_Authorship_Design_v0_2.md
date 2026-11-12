# Scrivi — Authorship: Pen Names, Authors and Writing As — Design v0.2

**Status:** ✅ **APPROVED 2026-11-10** — Q1–Q9 (§7) ruled as recommended (user: *"Q1–Q9 as recommended"*).
**Date:** 2026-11-10
**Codebase:** `[Cross]` — ScriviCore (schemas, endpoints) first, then `[Apple]`, then `[Linux]`.
**Supersedes:** the authorship half of [`Scrivi_Identity_Authorship_and_Settings_Design_v0_1.md`](Scrivi_Identity_Authorship_and_Settings_Design_v0_1.md)
(2026-09-28). Its settings half has shipped (EP-047: `scrivi_get_project_settings` / `scrivi_put_project_settings`), and its
cross-device identity half (§2, options A/B/C) stays open there; this document takes only what authorship needs from it (§4.4).
**Epic:** [EP-058] → [`Epics/Epic-active.md`](Epics/Epic-active.md).
**Occasioned by:** the user, 2026-11-10, after SP-208 planning wrongly said Scrivi had no author field:

> *"We need to surface the authorship issue immediately. We must be able to set the pen name of the author per project. We
> must also be allowed to establish multiple authors per project. Worlds also require settable authorship. Basically we need
> to finalize the Authorship design and create the Epic that will implement it. And it must be in place before we can finish
> the current Epic."*

and earlier rulings that still stand (2026-09-28): *"identity remains app-level, and the configured pen names are configured
app level, but the actual name on the manuscript is set up on a per-project basis"*; *"identity is managed app side and
authorship is handled project side."*

---

## 1. Requirements

| # | Requirement | Source |
| - | ----------- | ------ |
| **R1** | The writer sets the **pen name** a project is credited to — per project | user, 2026-11-10 |
| **R2** | A project can have **several authors** | user, 2026-11-10 |
| **R3** | A **world** has its own settable authorship | user, 2026-11-10 |
| **R4** | It is in place **before [EP-054] can complete** — the exports carry it | user, 2026-11-10 |
| **R5** | Identity (who is at the keyboard) stays app-level and private; authorship (whose name is on the work) lives in the package and is public | user, 2026-09-28 |
| **R6** | Pen names are kept app-level as the writer's own list, and chosen per project | user, 2026-09-28 |

## 2. What the code has today (read 2026-11-10)

| Piece | Today | File |
| ----- | ----- | ---- |
| **Identity** | ✅ minted once per machine and **persisted** on both platforms ([I-0216] fixed in [SP-151]: the Keychain on Apple, `EncryptedFileSecureStore` on Linux). Holds `identityID`, `deviceID`, one `personaID` and a `displayName` | `IdentityService.cpp:61–113` |
| **The identity's persona name** | ⛔ the Mac's **host name** on Apple (`ProcessInfo.hostName`), the literal **"Scrivi"** on Linux | `AppEnvironment.swift:222`, `EditorShell.cpp:457` |
| **Project personas** | `identities/project-personas.json` (`scrivi.projectPersonas.v1`: `personaID`, `displayName`, `personaKind`, `status`, `controlledBy[]`) — written once at creation with the identity's ONE persona, ⛔ **never read**. The Tintagael package reads `"displayName": "flitwick-5.local"` | `ProjectCreator.cpp`, `ProjectPersonasJson.cpp` |
| **Project members** | `identities/project-members.json` (`scrivi.projectMembers.v1`: `identityID`, `role`, `status`, `defaultPersonaID`, `joinedAt`) — written once, ⛔ **never read** | `ProjectMembersJson.cpp` |
| **Attribution of edits** | `AuthorshipRef {identityID, personaID, displayName}` is passed on every write (15+ `personaID` parameters in `scrivi.h`) and recorded in scene sidecars, objects (`createdBy` / `modifiedBy`, with `displayNameAtCreation`), history | `Types.hpp:41`, `ObjectJson.cpp:17–25` |
| **…but it is app-global** | ⛔ the app builds ONE `AuthorshipRef` at launch and uses it for every project (`AppEnvironment.authorshipRef`) — so a project cannot be written "as" its own pen name | `AppEnvironment.swift:21, 229` |
| **Worlds** | ⛔ **no authorship at all**: `world.json` has `worldID`, `displayName`, epoch, dates | `WorldJson.cpp:20–25` |
| **Persona endpoints** | ⛔ none | `scrivi.h` |
| **Exports** | Markdown, HTML ([SP-207]) and `.docx` ([SP-208]) title pages carry the title and subtitle; ✅ and the byline since [SP-212] (EP-054 AC9): "by A, B and C", HTML `<meta name="author">`, `.docx` Author style + `dc:creator` | `ManuscriptExport.cpp` |

So the shape the user asked for in September is already in the package — a credit list (personas) and a member list
(identities) — but nothing lets the writer name a persona, add one, or choose which one they write as, and worlds have
neither file.

## 3. The model

Four ideas, kept apart because each answers a different question:

| Idea | Answers | Where it lives | Public? |
| ---- | ------- | -------------- | ------- |
| **Identity** | *Is this the same human?* | app (SecureStore) | ⛔ private |
| **Pen name** | *What names does this human write under?* — the menu | app (`appSupport`) | the writer's own list |
| **Author** | *Whose names are on this work, in what order?* | the package (`identities/…-personas.json`) | ✅ public, travels with the work |
| **Writing as** | *Which author is this human, here?* — the name their edits are recorded under | the package (`identities/…-members.json`, the member's `defaultPersonaID`) | ✅ travels |

```
APP  (per machine)                       PROJECT or WORLD package
─────────────────                        ───────────────────────────────────────────
Identity  identityID ──────────────┐     members.json   identityID → writingAs (personaID)
Pen names "J. R. Gustafson"        │                    identityID → writingAs …
          "Kit Marlowe"   ── chosen ──►  personas.json  [1] "J. R. Gustafson"   controlledBy: [me]
                                   │                    [2] "A. N. Other"       controlledBy: []   ← co-author not on Scrivi
                                   └──►  every write:   AuthorshipRef {identityID, personaID = writingAs, displayName}
```

- **An author is a persona in the package.** Its `personaID` is permanent; a rename changes `displayName` only; removing an
  author sets `status: "retired"` and never deletes the entry, because sidecars, objects and history refer to it by ID.
- **The byline is the ordered list of active authors.** The order is the order in the file (the writer reorders it).
- **A co-author need not use Scrivi:** a persona with no `controlledBy` is still an author (R2).
- **Writing as is per identity, per project** — the member's `defaultPersonaID` — so each co-author on their own machine
  writes as themself, and one writer with two pen names writes each project as the right one.
- **Pen names in the app are only a menu.** Choosing one copies it into the package as a new persona (or reuses the
  package's persona with that name), so the work stays self-describing on any machine (v0.1 Q2, recommended "both").

## 4. What changes

### 4.1 Core (ScriviCore)

| Change | Notes |
| ------ | ----- |
| **`scrivi.projectPersonas.v1` read at last, and its ORDER kept** | No new fields are needed: the array order is the byline order, `status` active/retired, `controlledBy` the identities that may write as it. The same schema serves a world |
| **Worlds gain `identities/world-personas.json` and `identities/world-members.json`** | Same two schemas inside the `.scrivworld` package; created on world creation, and on first open of an existing world (§4.4) |
| **`scrivi_get_authors(packageRoot)`** | → `{authors:[{personaID, displayName, controlledBy}], writingAs?:{personaID, displayName}}` for a project OR a world root; `writingAs` for this machine's identity |
| **`scrivi_set_authors(packageRoot, authorsJSON)`** | Replaces the ordered active list. Entries with a `personaID` keep it (rename = new `displayName`); entries without one get a new persona; an active persona left out becomes `retired`. Refused with zero authors |
| **`scrivi_set_writing_as(packageRoot, identityID, personaID)`** | Records the member (joining if new) and its `defaultPersonaID`; adds the identity to that persona's `controlledBy` |
| **`scrivi_get_pen_names(appSupportRoot)` / `scrivi_put_pen_names(appSupportRoot, json)`** | The app-level menu (R6): `pen-names.json` beside the identity in `appSupport`, one list per identity. Not secret, so not in SecureStore; not in any package |
| **`scrivi_open_project` returns `writingAs`** | So the app builds the project's `AuthorshipRef` from the package, not from launch |
| **Exports read the authors** | Title page and document metadata in Markdown, HTML and `.docx` (and PDF/ePub when [EP-054] S5 builds them) — Q6 |
| **Boundary tests through `scrivi_*`** | Never the facade ([I-0113]) |

### 4.2 Apple

- `AuthorshipRef` becomes **per project session** (from `writingAs`), not app-global. Every write site already takes it as a
  parameter; what changes is where it is built.
- **Settings ▸ Pen Names** — the app-level menu (add, rename, remove; one marked as the default for new projects and worlds).
- **Project Settings ▸ Authors** — the ordered author list (add from the menu or type a new name, rename, reorder, remove),
  and **"I write as"** for this machine.
- **World ▸ Authors** — the same, for a world (in the Worlds surface).
- The first-run prompt and the existing-package prompt (§4.4).

### 4.3 Linux

The same three surfaces and prompts, in the Mac's shape and wording (`feedback_linux_adopts_apple_shape`). The bridge
stops passing "Scrivi" as a name anyone sees.

### 4.4 Existing projects and worlds, and the first run

- **No host name and no "Scrivi" is ever shown or exported as a pen name.** A persona whose `displayName` equals this
  machine's host name or "Scrivi" — the only names the code has ever written — counts as **unnamed**.
- **The first run after this ships** asks once for the writer's pen name (Settings ▸ Pen Names), so the menu is never empty.
- **A project or world whose authors are all unnamed** asks once, on open, who wrote it (prefilled with the default pen name);
  the answer renames the existing persona, so its ID — and every sidecar that refers to it — is kept (Q4).
- **An identity that is not a member of the package** (a co-author's machine, or the same writer on a new machine before
  v0.1's identity export exists) is asked once on open: *"Which of these authors are you?"* — the authors listed, or a new
  name (Q3). This is v0.1's option B, and the least that makes "writing as" true for several authors.
- **A world with no `identities/` folder** gets one on first open, its author set from the opening project's writing-as
  persona (Q5).

## 5. What this does NOT do

- ⛔ **It is not security.** Choosing "I am this author" proves intent, not identity (v0.1 §2.3). Signatures need key
  distribution and are out of scope.
- ⛔ **It does not move an identity between machines** (v0.1 option A). Without that, the same writer on a second machine is a
  second member who answers the §4.4 prompt once — correct, just not seamless.
- ⛔ **It does not rewrite history.** Recorded `displayNameAtCreation` values stay as they were; a renamed author shows the new
  name everywhere the name is looked up by `personaID`.
- ⛔ **No roles beyond "author"** unless Q1 rules otherwise.

## 6. Sequencing

1. **[EP-058] S1 — core:** schemas read, world identity files, the five endpoints, `writingAs` on open, the unnamed rule,
   boundary tests.
2. **[EP-058] S2 — Apple:** per-project `AuthorshipRef`, the three surfaces, the prompts.
3. **[EP-058] S3 — Linux:** the same.
4. **[EP-054] — the byline in every export** (a new EP-054 AC), after S1. [EP-054] cannot complete before it (R4).

## 7. Questions for the user

| # | Question | Ruling |
| - | -------- | ------ |
| **Q1** | **Roles.** Only *author*, or a role per credit (author, co-author, editor, translator, illustrator…)? | ✅ **RULED as recommended (user, 2026-11-10).** **Author only, ordered.** It meets R2; a `role` can be added to the persona entry later without breaking the file. |
| **Q2** | **A co-author who does not use Scrivi.** May an author be just a name, with no identity behind it? | ✅ **RULED as recommended (user, 2026-11-10).** **Yes** (`controlledBy` empty). Credits often include people who never open the project. |
| **Q3** | **Writing as on a machine that is not yet a member.** | ✅ **RULED as recommended (user, 2026-11-10).** **Ask once on open** — *"Which of these authors are you?"* with the authors and "Someone else…" — and record the answer as membership. Never guess. |
| **Q4** | **Existing packages, credited to the host name or "Scrivi".** | ✅ **RULED as recommended (user, 2026-11-10).** **Ask once on open, prefilled with the default pen name, and rename that persona** (its ID kept). Until answered, the author is unnamed: never shown, and an export asks first. |
| **Q5** | **A world's authors.** Independent of the projects that use it? | ✅ **RULED as recommended (user, 2026-11-10).** **Independent.** A new world starts with the creating project's writing-as author; after that it has its own list (a world is often shared by projects with different bylines). |
| **Q6** | **The byline on exports.** | ✅ **RULED as recommended (user, 2026-11-10).** Title page: a line **"by A"**, **"by A and B"**, **"by A, B and C"** under the title and subtitle (`.docx`: an **Author** style); metadata: HTML `<meta name="author">`, `.docx` `dc:creator` ("A; B"), Markdown title page line. Copy as HTML (body only) carries none, as it carries no title. This re-opens SP-208 Q7 ("title only") — it was ruled that way only because the name was not settable. |
| **Q7** | **Where the pen-name menu lives.** | ✅ **RULED as recommended (user, 2026-11-10).** **`appSupport`, one list per identity, not synced.** It is the writer's own menu, not a secret; moving it between machines is v0.1's identity export, not this Epic. |
| **Q8** | **One `[Cross]` Epic with a platform AC each** (the [EP-054] Q1 precedent), or one Epic per platform? | ✅ **RULED as recommended (user, 2026-11-10).** **One Epic**, as EP-054 Q1 ruled: the model and endpoints are one design, and each platform's surface is an AC. |
| **Q9** | **How [EP-054] waits for it.** | ✅ **RULED as recommended (user, 2026-11-10).** [SP-208] closes on its own ACs (ruled: no byline). [EP-054] gains **AC9 — every export carries the byline**, delivered by an EP-054 Sprint after [EP-058] S1; EP-054 cannot complete until AC9 is met. |

`personaKind` (v0.1 Q4) stays as written (`individual`), unread, unless a ruling gives it a meaning — a shared pen name such
as a writing duo is just one author controlled by two identities, which the model already allows.

## 8. What was read

`IdentityService.cpp:47–115` · `Types.hpp:41–60` · `ProjectPersonasJson.cpp` · `ProjectMembersJson.cpp` ·
`ProjectCreator.cpp` · `ObjectJson.cpp:17–25` · `WorldJson.cpp:20–40` · `scrivi.h` (no persona endpoints) ·
`AppEnvironment.swift:21, 215–235` · `platforms/linux/src/ScriviBridge.cpp:140–180`, `EditorShell.cpp:457` · the Tintagael
package's `identities/project-personas.json`. ⛔ **Not read:** how each history record stores its author (it is passed the
`AuthorshipRef`; whether it stores `displayName` is S1's first check), and the Worlds surfaces' layout on either platform.
