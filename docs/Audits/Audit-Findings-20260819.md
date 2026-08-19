# Documentation Audit — 2026-08-19

**Scope:** Every tracking file in `docs/Issues/`, `docs/Tasks/`, `docs/Sprints/`, `docs/Epics/` —
guidelines, archives, and summary files — in the order defined by
[`Audit-Guidelines.md`](Audit-Guidelines.md).

**Method:** Read-only. Counts were re-derived from the filesystem (`grep -c '^## I-0'` per batch file,
`ls` per archive folder, ID-enumeration scripts), **never taken from a stated figure**. Every finding
cites `file:line` or the command that established it.

**Result: 31 findings — 9 Critical, 14 Moderate, 8 Minor.**

⚠️ **Nothing was changed by this audit.** Seven files carry uncommitted edits from *before* the audit
began — see **§0**, which you must rule on first, because several findings below describe those files.

---

## 0. ⚠️ Uncommitted state — read this first

Before you stopped me, I had already edited seven files in this session (SP-100 planning + the start of an
I-0018/I-0118/I-0133/I-0134 fix). `git status` at audit time:

```
 M docs/Epics/Epic-active.md
 M docs/Issues/Verified/Issue-verified-0131-0140.md
 M docs/Sprints/Sprint-Documentation.md
 M docs/Sprints/Sprint-active.md
 M docs/Sprints/Sprint-backlog.md
 M docs/Tasks/Task-Documentation.md
 M docs/Tasks/Task-backlog.md
```

Two of these are **half-finished** and are themselves defects:

- **`Issue-verified-0131-0140.md`** — I added table rows for **I-0133 and I-0134** but was stopped before
  writing their full entries. The file now lists four Issues in its table and holds **two** entries.
  ⚠️ **This is exactly the batch-file inconsistency that F-03 and F-04 describe.**
- **`Task-Documentation.md`** — T-0418 was inserted and statistics rewritten for a Sprint (SP-100) that is
  planned but **not activated**.

**Options:** (a) `git checkout` all seven and re-apply from approved findings; (b) keep the SP-100 planning
edits (they were the requested work) and revert only the two half-finished ones; (c) keep all and fix
forward. **I recommend (b).** No option is applied without your ruling.

---

## Summary table

| # | Layer | File | Kind | Sev | Finding |
| - | ----- | ---- | ---- | --- | ------- |
| F-01 | Issues | Issue-active.md | Stale | **Critical** | I-0133 + I-0134 shown Resolved/Open; you state both are **Verified** |
| F-02 | Issues | Issue-backlog.md | Stale | **Critical** | I-0018 shown 🟠 partly-fixed; you state it is **Verified** |
| F-03 | Issues | Verified/0111-0120 | Missing | **Critical** | **I-0118 has no archive entry anywhere** despite being cited as Verified evidence |
| F-04 | Issues | Verified/ (all) | Missing | **Critical** | **I-0059 and I-0099 exist in no file in `docs/`** |
| F-05 | Issues | Issue-active.md | Misplaced | **Critical** | I-0118 "design ruling" block parked in the active file; you asked it be closed |
| F-06 | Tasks | Task-Documentation.md | Contradiction | **Critical** | T-0185–T-0188 are ✅ Verified here, ⚪ Descoped in Task-backlog.md |
| F-07 | Sprints | Sprints/ | Missing | **Critical** | **SP-039 has no archive** — only a stale 🔵 Planning draft in the wrong folder |
| F-08 | Epics | Epic-Documentation.md | Missing | **Critical** | **EP-034 is absent** from the index; totals still say 33 / next EP-034 |
| F-09 | Issues | Verified/0131-0140 | Contradiction | **Critical** | Table lists 4 Issues, file holds 2 entries (from §0) |
| F-10 | Issues | Issue-Documentation.md | Stale | Moderate | "Active: 2 (I-0131, I-0132)" — both archived 2026-08-18 |
| F-11 | Issues | Issue-Documentation.md | Stale | Moderate | Verified total says 116; actual is **119**; batch 14 missing from table |
| F-12 | Issues | Issue-Documentation.md | Stale | Moderate | Batch 3 count says 6; file holds **7** |
| F-13 | Issues | Verified/ | Misplaced | Moderate | 4 batch files titled for IDs they do not contain |
| F-14 | Issues | Issue-GUIDELINES.md | Guidelines | Moderate | Paths wrong (`/Scrivi/Documentation/Issues/`), no such tree |
| F-15 | Issues | Issue-GUIDELINES.md | Guidelines | Moderate | Batch policy names `DR-verified-*`; status list omits 🟠 |
| F-16 | Tasks | Task-unverified.md | Contradiction | Moderate | Says T-0389 "is now listed"; table is empty |
| F-17 | Tasks | Task-unverified.md | Stale | Moderate | "SP-102 is still active" — closed 2026-08-18 |
| F-18 | Tasks | Task-Documentation.md | Contradiction | Moderate | Stats say 9 Implemented-Not-Verified; Task-unverified.md holds 0 |
| F-19 | Tasks | Task-Guidelines.md | Guidelines | Moderate | Wrong paths; "ER"/"Taskss" leftovers; no Closed/ in lifecycle |
| F-20 | Sprints | Sprints/ | Misplaced | Moderate | **12 loose Sprint files** duplicating archives |
| F-21 | Sprints | Sprint-Documentation.md | Stale | Moderate | All-Sprints table + Statistics showed SP-102 Active (pre-existing) |
| F-22 | Sprints | Sprint-GUIDELINES.md | Guidelines | Moderate | No rule covering loose planning files — the cause of F-20 |
| F-23 | Epics | Epic-Documentation.md | Stale | Moderate | "9 of 11 sprints", SP-102 Active — two days stale |
| F-24 | Epics | Epic-active.md | Stale | Moderate | AC9 note still says "🟡 In progress — SP-102 active" |
| F-25 | Issues | Issue-Documentation.md | Guidelines | Minor | Index carries activity logs its own guidelines forbid |
| F-26 | Tasks | Task-Documentation.md | Minor | Minor | 180 unfiled rows — known, user-ruled, restated here for completeness |
| F-27 | Tasks | Task-0118.md | Misplaced | Minor | Only per-Task detail file outside `Verified/` |
| F-28 | Sprints | Sprint-backlog.md | Minor | Minor | 200+ lines of closed-sprint history in a *backlog* file |
| F-29 | Epics | Epic-GUIDELINES.md | Guidelines | Minor | No Audit layer referenced |
| F-30 | All | 4× *-GUIDELINES.md | Guidelines | Minor | Inconsistent naming: `GUIDELINES` vs `Guidelines` |
| F-31 | All | CLAUDE.md | Missing | Minor | Agile Tracking section has no Audits layer |

---

## Findings

### F-01 — I-0133 and I-0134 are shown unverified; you state both are Verified

**Layer:** Issues · **File:** `Issue-active.md:11-12` · **Kind:** Stale · **Severity: Critical**

**What is there:** I-0133 → `🟠 Resolved - Not Verified (2026-08-18)`; I-0134 → `🔴 Open (2026-08-18)`,
*"parity ruling needed; no code changed."* Line 114 states *"Active count: 2."*

**What is true:** You stated both are resolved and verified. The file has not been updated since.

**Why it matters:** ⚠️ **This produced a wrong statement in my SP-100 plan.** I wrote that I-0134 was
"Open and needs a ruling" and deliberately excluded both from SP-100 — reasoning taken straight from these
rows. A stale row did not just sit there; it propagated into a planning document and into what I told you.

**Options:** (a) Archive both to `Verified/Issue-verified-0131-0140.md` with full entries, remove from
active, correct the SP-100 note; (b) same, but also record I-0134's parity ruling, since "Verified" for a
cross-platform disagreement implies a decision was made about *which* behaviour is right — that decision
is not written down anywhere I can find.

⚠️ **(b) matters:** I-0134's entry says Linux overrides `centerCursor()` deliberately and that changing it
means re-verifying EP-022 over VNC. If the resolution was "Apple is right, Linux changes," that is EP-026
work that must be recorded, not closed silently.

---

### F-02 — I-0018 is shown partly-fixed; you state it is Verified

**Layer:** Issues · **File:** `Issue-backlog.md:5,11` · **Kind:** Stale · **Severity: Critical**

**What is there:** *"Currently: 1. Only I-0018 remains, 🟠 partly fixed and rescoped"*, with
*"Resolution: TBD — pending the keyboard/focus/caret model ruling that I-0132 also needs."*

**What is true:** You state I-0018 should be Verified and archived. I-0132's ruling has landed
(Verified 2026-08-18), which is the dependency this row was waiting on.

**Why it matters:** It is the **only** row in the Issue backlog. Once archived, `Issue-backlog.md` becomes
empty — which is a state the file has no text for, and which the guidelines do not describe.

**Options:** (a) Archive to `Verified/Issue-verified-0011-0020.md` (its home decade) and leave the backlog
with an explicit *"Currently: 0"*; (b) archive to batch 14 alongside the Issues it was scoped with
(I-0131–I-0134), keeping the "same question across load/click/quit" group together — ⚠️ but that breaks
the decade-batch rule.

**I recommend (a)** — the decade rule is mechanical and should stay mechanical; the grouping belongs in
cross-references, which I-0018's entry already has.

---

### F-03 — ⚠️ I-0118 has no archive entry, yet is cited as Verified evidence

**Layer:** Issues · **File:** `Verified/Issue-verified-0111-0120.md` · **Kind:** Missing ·
**Severity: Critical**

**What is there:** `Issue-verified-0111-0120.md` contains entries for I-0111, I-0112, I-0113, I-0114,
I-0115, I-0116, I-0117, I-0119 — **eight**. There is **no `## I-0118` entry.** Established by
`grep -n '^## I-0' docs/Issues/Verified/Issue-verified-0111-0120.md`.

**What is true:** I-0118 is asserted Verified in at least three places — `Epic-active.md:34`
(*"I-0118 and I-0119 were verified 2026-08-14/15"*), `Issue-active.md:26`, and
`Issue-Documentation.md`. Its *ruling text* lives in `Issue-active.md:24-85`. **Its Issue record does
not exist.**

**Why it matters:** ⚠️ **I-0118 is load-bearing for EP-031.** It is the world-search-indexing Issue whose
Q1–Q4 rulings spawned **EP-033**, and `Epic-active.md:36` names it as *"available as evidence for SP-100's
AC pass."* **SP-100 is about to rest an Epic close on an Issue with no record.** The four rulings are
preserved (in `Issue-active.md`), but there is no statement of what was broken, what was fixed, or what
was verified.

**Options:** (a) Write the I-0118 entry into batch 12 from the SP-105 sprint archive and the ruling text,
then close it as you asked; (b) close it citing the SP-105 archive as record-of-truth, accepting it as one
more "unfiled" record like the 180 Tasks in F-26.

⚠️ **(b) is cheaper but weaker**, and this is the specific Issue SP-100 needs to lean on. **I recommend
(a).**

---

### F-04 — ⚠️ I-0059 and I-0099 exist in no file in `docs/` at all

**Layer:** Issues · **File:** *(none)* · **Kind:** Missing · **Severity: Critical**

**What is there:** Nothing. `grep -rl "I-0059" docs/` and `grep -rl "I-0099" docs/` both return **zero
files**. Not in Verified, Closed, active, backlog, index, or any Sprint/Epic archive.

**What is true:** Unknown, and that is the finding. Either the IDs were never assigned (and no file says
so), or two Issue records were lost.

**Why it matters:** Every other tracking layer explicitly records its gaps — *"T-0278 never assigned"*,
*"SP-068 skipped"*. The Issue layer records nothing, so **an unassigned ID and a lost record look
identical.** The verified total cannot be reconciled against the ID range while this is unresolved.

**Related:** **I-0016** appears only in `Sprint-SP-033.md` and `Epic-EP-011.md` — it has no Issue-layer
record either, though at least it is traceable.

**Options:** (a) Check git history for these IDs and restore or annotate; (b) record them in
`Issue-Documentation.md` as *"never assigned"* if you recall that they were; (c) leave, and add a
"Never assigned" line to the Issue index so future audits stop re-finding them.

⚠️ **Do not skip (c) whatever else is chosen** — an unexplained gap will be re-discovered every audit.

---

### F-05 — The I-0118 "design ruling" block is parked in `Issue-active.md`

**Layer:** Issues · **File:** `Issue-active.md:24-85` · **Kind:** Misplaced · **Severity: Critical**

**What is there:** 62 lines under *"## I-0118 — design ruling (user, 2026-08-14)"*, introduced as
*"Retained here as an active design ruling, not an open defect."*

**What is true:** You said: *"Motherfucking close I-0118. Neither you nor I need to keep this here as a
'Design Ruling'."*

**Why it matters:** `Issue-active.md` is defined as *"Issues awaiting user verification"*. A 62-line
design essay in it is a category error, and it is the largest block in the file — anyone opening it to
check active Issues wades through it first. Its substance is **already captured**: the Q1–Q4 rulings are
in `Epic-backlog.md`'s EP-033 entry, and the implementation shipped as SP-105.

**Options:** (a) Delete outright — content is duplicated in EP-033 and SP-105; (b) move the ruling text
into I-0118's archive entry (see F-03), so the Issue record and its ruling live together, and delete from
active. **I recommend (b)** — it solves F-03 and F-05 in one move.

---

### F-06 — T-0185–T-0188: Verified in the index, Descoped in the backlog

**Layer:** Tasks · **Files:** `Task-Documentation.md:438` vs `Task-backlog.md:39` · **Kind:**
Contradiction · **Severity: Critical**

**What is there:**
- `Task-Documentation.md:438` — `| T-0185 | New Spotlight importer app-extension target | SP-046 | EP-017 | ✅ Verified | ⚠️ unfiled |`
- `Task-backlog.md:39` — `| T-0185–T-0188 | ⚪ Descoped — I-0057, CSImportExtension non-functional on macOS |`

**What is true:** Cannot be determined from the tracking files — they assert opposites. `Sprint-SP-046.md`
is named as record of truth by the index and must be read to settle it.

**Why it matters:** "Verified" and "Descoped" are opposite claims about whether work exists. Four IDs are
affected, and the index counts them in its **376 Verified** figure — so if Descoped is right, the headline
Task statistic is wrong by four.

**Options:** (a) Read `Sprint-SP-046.md` and correct whichever file disagrees; (b) if the sprint archive
is ambiguous, mark all four ⚪ Descoped (the backlog claim is more specific — it names the blocking Issue,
I-0057) and decrement Verified to 372.

---

### F-07 — ⚠️ SP-039 has no archive; its only record is a stale planning draft in the wrong folder

**Layer:** Sprints · **File:** `docs/Sprints/Sprint-SP-039.md` · **Kind:** Missing ·
**Severity: Critical**

**What is there:** `Sprint-SP-039.md` sits loose in `docs/Sprints/` and reads **`**Status:** 🔵 Planning`**.

**What is true:** `Sprint-Documentation.md:305` records SP-039 as **✅ Closed, 2026-06-11**. There is **no
`Closed/Sprint-SP-039.md`** — established by enumerating `Closed/` (101 archives; missing SP-034, SP-039,
SP-057, SP-068, SP-100). SP-034 is cancelled, SP-057 superseded, SP-068 skipped, SP-100 not yet run —
**all four are legitimately absent. SP-039 is not.**

**Why it matters:** SP-039 delivered the **entire timeline C ABI** — `Epic-Documentation.md:160` calls it
out as the reason EP-025 could be a pure UI Epic. That work's only sprint-level record says the sprint was
never started.

**Options:** (a) Reconstruct `Closed/Sprint-SP-039.md` from `Epic-EP-016.md` + the planning draft, then
delete the loose file; (b) move the draft to `Closed/Sprint-SP-039.md`, correct its status, and mark it
reconstructed (precedent: `Sprint-SP-056.md` is explicitly *"reconstructed"*).

**I recommend (b)** — the precedent exists and the draft has the task list.

---

### F-08 — EP-034 is missing from the Epic index entirely

**Layer:** Epics · **File:** `Epic-Documentation.md:57,93-99` · **Kind:** Missing ·
**Severity: Critical**

**What is there:** `Epic-Documentation.md:57` — *"Currently: **33 Epics** | Next available: **EP-034**"*.
The All-Epics table ends at EP-033. Statistics say *"Total Epics: 33"*, *"Proposed (backlog): 2"*.
**EP-034 appears exactly once in the file — as the "next available" number.**

**What is true:** **EP-034 exists.** `Epic-backlog.md:111` — *"## EP-034: [Cross] Object Detail & Media"*,
opened 2026-08-18, and `Epic-backlog.md:188` records *"Backlog Epics 3 → 4"*.

**Why it matters:** EP-034 is the Epic that **owes source creation** — the reason T-0365 closed as a
partial delivery, and a carried item in SP-100's plan. It is invisible in the index, and *"next available:
EP-034"* means **the next Epic opened will collide with it.**

**Options:** (a) Add the EP-034 row, totals → 34, proposed → 3, next available → EP-035.

---

### F-09 — Batch 14's table lists four Issues; the file holds two entries

**Layer:** Issues · **File:** `Verified/Issue-verified-0131-0140.md:8-11` · **Kind:** Contradiction ·
**Severity: Critical**

**What is there:** Summary table rows for I-0131, I-0132, **I-0133, I-0134**. Full entries: **I-0131 and
I-0132 only.**

**What is true:** ⚠️ **I caused this during this session** (§0) — I added the rows and was stopped before
writing the entries.

**Why it matters:** It reproduces the exact defect class this audit exists to find, and if committed it
would look identical to F-03 (I-0118) — a table claiming an archive that is not there.

**Options:** As per §0. If you choose to keep it, the two entries must be written in the same pass.

---

### F-10 — Issue index says Active is I-0131 + I-0132; both were archived

**Layer:** Issues · **File:** `Issue-Documentation.md:22,29-32,113` · **Kind:** Stale · **Severity:
Moderate**

**What is there:** *"Currently: **2 Issues awaiting verification** — I-0131 and I-0132"*, with a table
listing them, and *"Active is now 2 (I-0131, I-0132)"*.

**What is true:** Both were Verified and archived to batch 14 on 2026-08-18. `Issue-active.md` has since
held I-0133 and I-0134 — **which this index never learned about.** Neither ID appears anywhere in it.

**Why it matters:** The index is two days and two Issues behind. Combined with F-01, **no file in the
Issues layer currently states the true active set** (which, per your correction, is now zero).

---

### F-11 — Verified total is 116; actual is 119, and batch 14 is missing from the table

**Layer:** Issues · **File:** `Issue-Documentation.md:68,81-95` · **Kind:** Stale · **Severity:
Moderate**

**What is there:** *"Currently: **116 verified Issues**… Counts below are the entries physically in each
file, and they sum to 116."* The batch table runs batch 1 → batch 13.

**What is true:** Counted with `grep -c '^## I-0'` per file: 10, 11, **7**, 10, 9, 8, 10, 8, 9, 8, 9, 8,
10, **2** = **119**. **Batch 14 (`Issue-verified-0131-0140.md`) has no row in the table**, and batch 3 is
miscounted (F-12).

**Why it matters:** The file claims its counts were physically re-derived — the discipline is right, but
it was not repeated when batch 14 was created. ⚠️ **A count that advertises its own method is more
dangerous when stale**, because it invites trust.

---

### F-12 — Batch 3 is recorded as 6 entries; the file holds 7

**Layer:** Issues · **File:** `Issue-Documentation.md:85` · **Kind:** Stale · **Severity: Moderate**

**What is there:** `| 3 | I-0021 – I-0030 | ... | 6 ⚠️ |`

**What is true:** `grep -c '^## I-0' Issue-verified-0021-0030.md` → **7**.

**Why it matters:** Small, but it is inside the batch-2/3 misfiling note — the one place the file
explicitly claims to have recounted from disk. Either an entry was added later, or the recount was wrong.

---

### F-13 — Four batch files are titled for IDs they do not contain

**Layer:** Issues · **Files:** `Verified/` · **Kind:** Misplaced · **Severity: Moderate**

**What is there:**
| File | Titled | Actually contains |
| ---- | ------ | ----------------- |
| `-0011-0020.md` | I-0011–I-0020 | 11 entries incl. **I-0021–I-0024** (belong to batch 3) |
| `-0041-0050.md` | I-0041–I-0050 | I-0041–I-0049 — **no I-0050** |
| `-0051-0060.md` | I-0051–I-0060 | **no I-0059, no I-0060** |
| `-0091-0100.md` | I-0091–I-0100 | **no I-0099, no I-0100** |
| `-0111-0120.md` | I-0111–I-0120 | **no I-0118, no I-0120** |

**What is true:** The batch-2/3 overlap is **known and documented** (`Issue-Documentation.md:74-79`) with
a ruling to leave it. The others are not documented anywhere.

**Why it matters:** I-0050, I-0060, I-0100, I-0120 are each the *last ID of a decade* — a suspicious
pattern suggesting an off-by-one at batch-rollover, not four coincidences. ⚠️ **Worth checking whether
those four Issues exist at all** (F-04 shows two nearby IDs do not).

**Options:** (a) Determine for each whether the ID was used; (b) annotate each batch file's header with
which IDs it actually holds — cheap, and makes the next audit trivial.

---

### F-14 — Issue guidelines reference a directory tree that does not exist

**Layer:** Issues · **File:** `Issue-GUIDELINES.md:31,171,177,180` · **Kind:** Guidelines ·
**Severity: Moderate**

**What is there:** *"File: `/Scrivi/Documentation/Issues/Issue-Documentation.md`"*, *"Main DR-Reports
directory (`Documentation/Issues/`)"*, *"`Documentation/Issues/Verified/`"*.

**What is true:** The tree is `docs/Issues/`. There is no `Documentation/` directory in the repo.

**Why it matters:** Every path in the authoritative process document is wrong. The same defect is in
`Task-Guidelines.md` (F-19).

---

### F-15 — Issue guidelines: wrong file-naming scheme and an incomplete status list

**Layer:** Issues · **File:** `Issue-GUIDELINES.md:106-111,133-141,175` · **Kind:** Guidelines ·
**Severity: Moderate**

**What is there:**
1. Batch policy names files **`Verified/DR-verified-0001-0010.md`** — leftover "DR" naming; actual files
   are `Issue-verified-*`.
2. Status list (l.106-109) defines 🔴 Open, ⚠️ In Progress, 🟡 Resolved-Not-Verified, ✅ Verified — but
   **every file in practice uses 🟠 for Resolved-Not-Verified**, and 🟠 is not in the list.
3. l.175 names **`Issue-backlog-XXXX-YYYY.md`** (batched); the real file is a single `Issue-backlog.md`.
4. Checklists (l.253, l.260) say *"moved from Issue-unverified.md"* — **that file does not exist** in the
   Issues layer. (Tasks have `Task-unverified.md`; Issues do not.)
5. The template's closing fence is malformed (l.85 `````` vs l.102 escaped ```` \`\`\` ````).

**Why it matters:** Item 4 is why Issues get archived straight out of `Issue-active.md` or
`Issue-backlog.md` with no rule covering it — and F-02/F-03 are instances.

---

### F-16 — `Task-unverified.md` says T-0389 "is now listed"; the table is empty

**Layer:** Tasks · **File:** `Task-unverified.md:13-16,23-27,36` · **Kind:** Contradiction ·
**Severity: Moderate**

**What is there:** An empty table, `_No Tasks awaiting verification._`, then a note *"T-0389 is also
listed in Task-active.md because SP-102 is still active"*, and a Last-Updated saying *"T-0389 is the one
genuinely unverified Task and is now listed."*

**What is true:** T-0389 was Verified 2026-08-17 and archived to `Task-verified-0389-0415.md`. The table
is correct; **the prose around it is stale.**

---

### F-17 — `Task-unverified.md` states SP-102 is still active

**Layer:** Tasks · **File:** `Task-unverified.md:23` · **Kind:** Stale · **Severity: Moderate**

**What is there:** *"because **SP-102 is still active** — it is implemented but its Sprint has not
closed."*

**What is true:** SP-102 closed 2026-08-18 (user-approved).

---

### F-18 — Task statistics claim 9 unverified Tasks; the unverified file holds 0

**Layer:** Tasks · **Files:** `Task-Documentation.md:47` vs `Task-unverified.md:16` · **Kind:**
Contradiction · **Severity: Moderate**

**What is there:** Stats: *"🟠 Implemented - Not Verified: **9**"*. `Task-unverified.md`: *"No Tasks
awaiting verification."*

**What is true:** They cannot both hold. ⚠️ Note the stats **sum to exactly 418** (376+24+0+9+3+2+2+1+1),
so if the 9 is wrong, at least one other line absorbs the error and is also wrong — the total was
balanced, not derived.

**Options:** (a) Enumerate all 418 IDs and re-derive every line from the files.

---

### F-19 — Task guidelines: wrong paths, leftover "ER" naming, no Closed state in the lifecycle

**Layer:** Tasks · **File:** `Task-Guidelines.md` · **Kind:** Guidelines · **Severity: Moderate**

1. **Paths wrong** — `Documentation/Tasks/` throughout (l.126, 177, 184, 187); actual is `docs/Tasks/`.
2. **"ER" leftovers** — l.163 *"Verified ERs"*, l.254-256 *"ER moved from…"*, l.178 *"all ERs"*.
3. **Typos** — *"Taskss"* (l.133, 160-162), *"Sprint Asigned"* (l.74), *"aproval"* (l.119).
4. **Lifecycle diagram (l.9-14) is mangled** and omits `Closed/` although a Closed state is defined at
   l.22 and `Tasks/Closed/` exists with two files.
5. **Status emoji collision:** 🟡 is used for *both* Active and Implemented-Not-Verified (l.11, 26, 29).
   Practice uses 🟠 for the latter. **This is the same defect as F-15 item 2** — worth fixing once, in
   both files, with a shared convention.
6. **Version footer says 2026-01-10** — seven months stale.

---

### F-20 — Twelve loose Sprint files duplicate their archives

**Layer:** Sprints · **Files:** `docs/Sprints/Sprint-SP-*.md` · **Kind:** Misplaced · **Severity:
Moderate**

**What is there:** 13 loose files: `Sprint-SP-003/004/005/006/007/008/026-planning.md` and
`Sprint-SP-027/028/029/031/032/039.md`.

**What is true:** 12 have a counterpart in `Closed/`; **all 12 differ from it** (checked with `diff` on
SP-003/027/031/032 — 4-13 line differences each), consistent with pre-close drafts superseded by the
archive. **SP-039 is the exception and is F-07.**

**Why it matters:** `Sprint-GUIDELINES.md:72-82` defines exactly four file types for this folder; these
match none. They are pre-close drafts that read as current documents — `Sprint-SP-039.md` still says
"🔵 Planning" for a sprint closed in June.

**Options:** (a) Delete the 12 duplicates (archives supersede them), handle SP-039 per F-07; (b) move all
13 to `Sprints/Drafts/`; (c) delete 12, and for SP-039 promote the draft. **I recommend (a)+F-07(b).**

⚠️ **Before deleting, spot-check two or three archives** to confirm the draft carries nothing the archive
lacks — the diffs show the drafts are *shorter*, which is consistent with supersession but not proof.

---

### F-21 — Sprint index: All-Sprints table and Statistics showed SP-102 Active

**Layer:** Sprints · **File:** `Sprint-Documentation.md:348,356-358,222` · **Kind:** Stale ·
**Severity: Moderate**

**What is there (before my session edits):** the All-Sprints row `| SP-102 | … | 2026-08-17 | — | 🟡
**Active** |`; Statistics *"Active: 1 🟡 — SP-102"*; and l.222 *"100 closed, 1 🟡 Active (SP-102)"*.

**What is true:** SP-102 closed 2026-08-18. The file's **own header (l.14) already said so.**

**Why it matters:** ⚠️ **Third occurrence of the same pattern** — this file's lower tables lag its header.
Its own l.193-197 note documents the previous two. **A defect documented twice and repeated a third time
is a process problem, not a clerical one** (→ Systemic §S1).

**Note:** I edited these lines during this session; the finding records the pre-existing state.

---

### F-22 — Sprint guidelines have no rule for planning drafts

**Layer:** Sprints · **File:** `Sprint-GUIDELINES.md:70-87` · **Kind:** Guidelines · **Severity:
Moderate**

**What is there:** A four-file structure: `Sprint-GUIDELINES.md`, `Sprint-Documentation.md`,
`Sprint-active.md`, `Sprint-backlog.md`, `Closed/`.

**What is true:** In practice sprints get a planning document, and 13 of them are loose in the folder
(F-20). The guidelines neither permit nor forbid this, so nothing says when they should be removed.

**Also missing:** no rule that a Sprint may be *planned in full while still 🔵 Planning* — which is
exactly what SP-100 is now, with its plan in `Sprint-active.md` behind a banner. That works, but it is
undocumented and a future reader will reasonably assume `Sprint-active.md` means active.

---

### F-23 — Epic index is two days stale on EP-031

**Layer:** Epics · **File:** `Epic-Documentation.md:15-19,102-103` · **Kind:** Stale · **Severity:
Moderate**

**What is there:** *"**9 closed** (SP-095, SP-096, SP-097, SP-098, SP-099, SP-103, SP-104, SP-105,
SP-106). Two remain: **SP-102** 🟡 **Active** … then **SP-100** 🔵 Planning"*; Statistics repeat
*"9 of 11 sprints closed; SP-102 🟡 Active"*.

**What is true:** SP-102 closed 2026-08-18 → **10 of 11**, and only SP-100 remains. `Epic-active.md`
states this correctly at l.6.

**Also:** l.24 says *"AC9 and AC10 are also outstanding"* — **AC9 was met 2026-08-18.**

---

### F-24 — `Epic-active.md`'s AC9 note still describes SP-102 as in progress

**Layer:** Epics · **File:** `Epic-active.md:199-204` · **Kind:** Stale · **Severity: Moderate**

**What is there:** Under AC9, after the ✅ MET line: *"🟡 ~~In progress — SP-102 active (2026-08-17).~~"*
followed by *"**AC9 ticks when SP-102 lands**."*

**What is true:** AC9 is met; the note is struck-through but the surrounding prose still reads forward-looking.

**Why it matters:** Minor in isolation, but `Epic-active.md` is **582 lines** where each AC carries layers
of superseded commentary. It is the file SP-100 must read to verify ten ACs. ⚠️ **The signal-to-history
ratio is now poor enough to be a risk to the close itself** (→ Systemic §S3).

---

### F-25 — The Issue index carries activity logs its own guidelines forbid

**Layer:** Issues · **File:** `Issue-Documentation.md:108-143` · **Kind:** Guidelines · **Severity:
Minor**

**What is there:** 36 lines of dated activity narrative.

**What is true:** `Issue-GUIDELINES.md:167` — *"**CRITICAL:** Do NOT add activity logs, summaries, or
per-Issue detail tables to Issue-Documentation.md… The index must stay compact."*

**Why it matters:** The same rule exists for Tasks (`Task-Guidelines.md:173`) and is also broken —
`Task-Documentation.md` is **700+ lines**. Either the rule is wrong or the practice is. **Decide once.**

---

### F-26 — 180 Verified Tasks have no archive file (known, user-ruled)

**Layer:** Tasks · **File:** `Task-Documentation.md:57-64` · **Kind:** Missing · **Severity: Minor**

**What is there:** A *"Verified but unfiled"* section listing **180** Tasks (row count confirmed = 180,
matching the stated figure — ✅ this one is accurate), each pointing at a Sprint archive.

**What is true:** Confirmed independently: 171 archive files cover **263** IDs; **155 IDs in T-0001–T-0419
have no archive filename**, concentrated in T-0258–T-0357 (EP-023/024/025/028/029).

**Why it matters:** Recorded as **already ruled** (2026-08-15: index as-is, flag, do not back-fill), and
l.17-20 flags it as needing a dedicated pass. **No action requested — listed for completeness**, since an
audit that omitted the single largest gap would be misleading.

---

### F-27 — `Task-0118.md` is the only per-Task detail file outside `Verified/`

**Layer:** Tasks · **File:** `docs/Tasks/Task-0118.md` · **Kind:** Misplaced · **Severity: Minor**

**What is there:** A standalone detail file for backlog Task T-0118, linked from `Task-backlog.md:53`.
Dated June 8.

**What is true:** No other backlog Task has one; the guidelines describe per-Task files only under
`Verified/`.

**Options:** (a) Fold into the backlog row and delete; (b) keep and add a guidelines rule permitting
detail files for backlog Tasks. Harmless either way — flagged so it is not mistaken for an orphan.

---

### F-28 — `Sprint-backlog.md` is 260 lines, mostly closed-sprint history

**Layer:** Sprints · **File:** `Sprint-backlog.md` · **Kind:** Misplaced · **Severity: Minor**

**What is there:** One backlog row (SP-100), then ~230 lines of closure records for SP-052 through SP-106.

**What is true:** The standing rule (correctly followed) is that a Sprint's Planning block is removed when
it activates — but the **replacement closure note is never removed**, so the file accretes permanently.

**Why it matters:** The one file that should answer *"what is planned?"* buries a single row under a year
of history. Everything in those 230 lines is duplicated in `Closed/`.

---

### F-29 — Epic guidelines do not reference the Audit layer

**Layer:** Epics · **File:** `Epic-GUIDELINES.md` · **Kind:** Guidelines · **Severity: Minor**

Now that `docs/Audits/` exists, the four GUIDELINES files should cross-reference it — particularly that
**an Epic close should be preceded by an audit**, which is the case that produced this one.

---

### F-30 — Guidelines filenames are inconsistently cased

**Layer:** All · **Kind:** Guidelines · **Severity: Minor**

`Issue-GUIDELINES.md`, `Sprint-GUIDELINES.md`, `Epic-GUIDELINES.md` — but `Task-Guidelines.md`. I created
`Audit-Guidelines.md` matching the Task form; **you wrote "Audi-Guidelines"** in your instruction, which I
read as a typo. **Confirm the spelling and case you want**, and whether to normalise the other four.

---

### F-31 — CLAUDE.md's Agile Tracking section has no Audits layer

**Layer:** All · **File:** `CLAUDE.md` · **Kind:** Missing · **Severity: Minor**

As you instructed. The section documents Issues/Tasks/Sprints/Epics with file paths and authorization
rules; Audits needs the same treatment, including ⚠️ **the prime rule that an audit changes nothing**.

**Not yet applied** — this audit changes nothing, including CLAUDE.md. Draft text is ready on your word.

---

## Guidelines recommendations

| # | Change | Files |
| - | ------ | ----- |
| G-1 | Fix all directory paths (`Documentation/` → `docs/`) | Issue-GUIDELINES, Task-Guidelines |
| G-2 | **Adopt one status-emoji convention** and apply it — 🟠 for Resolved/Implemented-Not-Verified, since that is universal in practice; remove the 🟡 collision | Issue-GUIDELINES, Task-Guidelines |
| G-3 | Remove `DR-`/`ER-` leftovers | Issue-GUIDELINES, Task-Guidelines |
| G-4 | Fix checklists naming `Issue-unverified.md` — Issues archive from active **or backlog**; state that | Issue-GUIDELINES |
| G-5 | Add a rule for Sprint planning drafts: where they live, when deleted | Sprint-GUIDELINES |
| G-6 | Document that a Sprint may be **fully planned while 🔵 Planning**, and where that plan lives | Sprint-GUIDELINES |
| G-7 | Resolve the index-compactness contradiction — either enforce "no activity logs" or drop the rule | All four |
| G-8 | Add a rule: **statistics blocks must be re-derived from files, never adjusted arithmetically** | All four |
| G-9 | Add a rule: **the last decade/batch file must get an index row when created** | Issue-GUIDELINES |
| G-10 | Cross-reference the Audit layer + "audit before an Epic close" | All four |
| G-11 | Record intentionally-unused IDs explicitly (Issues has no such line; Tasks/Sprints do) | Issue-GUIDELINES |

---

## Systemic observations

**S1 — Long index files go stale from the bottom up.** `Sprint-Documentation.md` has now had its lower
tables contradict its own header **three times** (F-21; its own l.193-197 documents the first two).
`Epic-Documentation.md` l.119-127 records the same pattern and reaches the same conclusion: *"every one of
these was a count or status that drifted while the table beneath it stayed correct — which is the argument
for deriving these lines from the table rather than restating them."* **That conclusion was written, and
then not acted on.** The next occurrence should change the file's structure, not its contents.

**S2 — The same fact is restated in five places, and restatements are where drift lives.** A Sprint's
status appears in `Sprint-active.md`, `Sprint-backlog.md`, `Sprint-Documentation.md` (×2), and
`Epic-active.md`. Every finding in F-10, F-16, F-17, F-21, F-23, F-24 is a restatement that fell behind
its source. ⚠️ **This is the same defect class as the CLAUDE.md standing rule about derived kind lists** —
*"a list that restates rather than derives is a defect on sight, even when currently correct."* **The
tracking docs have the identical disease and no equivalent rule.**

**S3 — Superseded commentary is never removed, only struck through.** `Epic-active.md` is 582 lines for
one Epic; `Task-Documentation.md` 700+; `Sprint-backlog.md` 260 for one backlog row. The convention
("Prior note follows") preserves history well and makes current state hard to find. ⚠️ **SP-100 must read
`Epic-active.md` to verify ten ACs, and that file's signal-to-history ratio is now itself a risk to the
close.**

**S4 — Verification and archival have drifted apart.** F-02 (I-0018), F-03 (I-0118), F-01 (I-0133/0134)
are all the same shape: **verified in conversation, never archived.** The Issue layer has no
`Issue-unverified.md`, so there is no single queue where "verified but unfiled" is visible — which is why
this keeps happening and why you keep finding it.

**S5 — What this audit could not check.** Each layer's archive was checked for *existence* and *ID
coverage*, not *content correctness*. F-06 (T-0185–T-0188) needs `Sprint-SP-046.md` read in full to
settle, and F-26's 180 unfiled Tasks rest on sprint archives no audit has verified say what the index
claims. **A content-level audit of the archives is a separate, larger pass.**

---

## Open questions for the user

1. **§0** — which option for the seven uncommitted files?
2. **F-04** — do you recall I-0016 / I-0059 / I-0099 being assigned, or should they be recorded as never
   used?
3. **F-06** — are T-0185–T-0188 Verified or Descoped? (`Sprint-SP-046.md` should settle it; you may know
   outright.)
4. **F-01** — I-0134 is a cross-platform parity disagreement. **What was the ruling** — Apple's centring,
   Linux's fraction, or deferred? "Verified" alone does not say, and EP-026 will need it.
5. **F-13** — should I investigate whether I-0050 / I-0060 / I-0100 / I-0120 exist? (The end-of-decade
   pattern suggests a systematic rollover error.)
6. **F-20** — delete the 12 duplicate planning files, or relocate them?
7. **G-7** — enforce index compactness, or drop the rule? It is currently broken in all four layers.
8. **F-30** — `Audit-Guidelines.md` as created, or a different spelling/case?

---

*Audit performed 2026-08-19 · read-only · 31 findings · no files changed by this audit.*
*Next audit should re-check F-21 and F-24 specifically — both are recurrences.*
