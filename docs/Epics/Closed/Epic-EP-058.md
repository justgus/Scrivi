## EP-058: `[Cross]` **Authorship** — pen names, several authors per project, authors for worlds, and writing as

**Status:** ✅ **CLOSED 2026-11-11** (user, 2026-11-11: *"close SP-211 and EP-058, and match the Mac menu"*) — ✅ the Goal met; Sprints [SP-209]–[SP-211] closed; ✅ Audit Check run at the close (below). — activated 2026-11-10 (user: *"activate EP-058 and plan its first sprint"*). Created (user, 2026-11-10: *"We need to surface the authorship issue immediately… finalize the Authorship design and create the Epic that will implement it. And it must be in place before we can finish the current Epic."*) Design: [`../Scrivi_Authorship_Design_v0_2.md`](../../Scrivi_Authorship_Design_v0_2.md)
(✅ APPROVED 2026-11-10 — Q1–Q9 ruled as recommended; S1 can be planned).
**Codebase:** `[ScriviCore]` first (schemas read, world identity files, endpoints), then `[Apple]` and `[Linux]` surfaces; one
Epic with a platform AC each, as [EP-054] Q1 ruled (design Q8).
**Goal:** a writer sets the pen name each project and world is credited to, credits several authors in order, and writes each
project as the right author on every machine; the name on an export is that byline, never a host name.
**Blocks:** [EP-054] AC9 (every export carries the byline) — ⚠️ **[EP-054] cannot complete before this Epic's S1** (R4).

**What the code has today (read 2026-11-10):** the package already holds a credit list (`identities/project-personas.json`)
and a member list (`identities/project-members.json`), both written once at creation and never read; every write carries an
`AuthorshipRef` with a `personaID`. But no endpoint names, adds or chooses a persona; the app builds ONE `AuthorshipRef` at
launch for every project; the persona's name is the Mac's host name (Apple) or "Scrivi" (Linux); worlds have no authorship.

**Acceptance Criteria:**
- [x] AC1 — The core reads and writes the ordered authors of a project AND of a world (`scrivi_get_authors` /
      `scrivi_set_authors`): IDs permanent, a rename keeps the ID, a removed author is retired never deleted, never zero authors
- [x] AC2 — The app-level pen-name menu, one list per identity (`scrivi_get_pen_names` / `scrivi_put_pen_names`), outside every package
- [x] AC3 — ✅ core + Mac/iPad ([SP-210]); ✅ Linux ([SP-211]) — Writing as: `scrivi_open_project` returns this identity's author; every write in that project (scenes, objects,
      history) is recorded under it; an identity that is not a member is asked once which author it is (design Q3)
- [x] AC4 — ✅ core + Mac/iPad ([SP-210]); ✅ Linux ([SP-211]) — No host name and no "Scrivi" is ever shown or exported as a pen name; an existing package credited to one is
      asked once and that author renamed, its ID kept (design Q4); a world without authors gets them on first open (Q5)
- [x] AC5 — Boundary tests through `scrivi_*` for AC1–AC4, on macOS and Linux
- [x] AC6 — Settings ▸ Pen Names, Project Settings ▸ Authors (with "I write as") and World ▸ Authors on the Mac and iPad
- [x] AC7 — The same on Linux, in the Mac's shape and wording — ✅ [SP-211] (live pass on scrivi-vm, 2026-11-11)
- [x] AC8 — ✅ Mac + iPad ([SP-210], 2026-11-11); ✅ Linux ([SP-211]) — Live passes on each platform: a pen name set, a second author added and reordered, a world credited, and a second
      machine (scrivi-vm) answering "which author are you?" and writing as the co-author

**Sprints:** **S1 = [SP-209]** core ([T-0670]) ✅ Closed 2026-11-11 · **S2 = [SP-210]** Apple ([T-0671]) ✅ Closed 2026-11-11 · **S3 = [SP-211]** Linux ([T-0672]) ✅ Closed 2026-11-11. Then [EP-054] AC9 (the byline in Markdown, HTML and `.docx`).
**Not in scope:** moving an identity between machines (v0.1 option A), signatures, roles beyond author (design Q1).

### ✅ Audit Check at the close (2026-11-11)

Mechanical sweep (Audit-Guidelines § The Audit Check), run before the close. **Clean** — the only disagreements were the ones
the close itself resolves: AC3, AC4, AC7, AC8 unticked here while met by [SP-211], and [SP-211] shown 🟡 Active in this file and
`Epic-Documentation.md` after its close.
1. AC status — `Epic-Documentation.md` states no per-AC status for EP-058; nothing to disagree.
2. Evidence — [T-0670], [T-0671], [T-0672] each archived in `Tasks/Verified/` with a ✅ Verified row in `Task-Documentation.md`;
   none left in `Task-active.md`, `Task-backlog.md` or `Task-unverified.md`.
3. Sprint status — [SP-209]–[SP-211]: `Closed/` records, `status: Closed`, ✅ CLOSED rows; none in `Sprint-active.md` or `Sprint-backlog.md`.
4. Counts — the Epic and Sprint indexes restate no counts (P6/P7).
5. Parity — three Task rows, three archive files.
6. Orphans — none (`Rig-Live-Pass-*.md` in `Sprints/` are named by `Sprint-GUIDELINES.md`).
7. IDs — SP-209–SP-211 and T-0670–T-0672 all accounted for; `next-ids.json` at SP-212 / T-0673.
⚠️ Noted, fixed at the close: EP-058's row in `Epic-Documentation.md` carried its Sprints in the Start column.

