# Active Tasks

Tasks currently being worked by an **active Sprint**. Each row names the Sprint it belongs to; the
Sprint's own documentation lives in [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md) and is
**not** duplicated here.

A Task leaves this file when it is Verified (→ `Verified/Task-verified-XXXX.md`) or when its Sprint
closes. Tasks that are implemented but awaiting user verification move to
[`Task-unverified.md`](Task-unverified.md).

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ **Active Tasks live here, never in `Task-backlog.md`.** A Sprint may be **deactivated without
deactivating its Tasks** — the Sprint's status changes on its own layer while its Tasks stay in this
file. The backlog is for unstarted, unassigned work only.

---

## Currently: **no active Tasks** — SP-120's six are ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0453 – T-0458** | SP-120's six — the `attributes` map, source creation, ⚠️ **cite-an-existing** (the I-0164 mirror), ⚠️ **T-0365's second entry point**, citation fields, and ⚠️ **S11 widened to OPERATIONS** | SP-120 | 2026-08-24 → [`Verified/Task-verified-0453-0458.md`](Verified/Task-verified-0453-0458.md) |

⚠️ **Archived in the SAME STEP they were verified** (`feedback_archive_on_close`).

⚠️ **Two Issues (I-0169, I-0170) were raised against these Tasks after the suites went green** — both from
the user's live click-through, neither from any suite. Both ✅ Verified and archived with them.
⚠️ **That is 22 consecutive Issues across SP-118, SP-119 and SP-120 with none found by a test.**

## Previously: SP-119's seven Tasks — ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0446 – T-0452** | SP-119's seven — the index image field, AC3's image UI, D8 thumbnails, tags (**closing AC2**), the at-risk warning (**AC9**), attach-existing, and the guarded exit | SP-119 | 2026-08-24 → [`Verified/Task-verified-0446-0452.md`](Verified/Task-verified-0446-0452.md) |

⚠️ **Archived in the SAME STEP they were verified** (`feedback_archive_on_close`).

⚠️ **Seven Issues (I-0162 – I-0168) were raised against these Tasks after the suites went green** — every
one found by the user's live click-through, none by any suite. ⚠️ **Six were data-loss routes into a single
surface.** All ✅ Verified and archived with them.

## Previously: SP-117's seven Tasks — ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0434 – T-0440** | SP-117's seven — the Detail Sheet pane, history, ⚠️ **the app's first typed object model**, patch-based save, both entry points, pending read-only, and ⚠️ **T-0420's surface owed since SP-115** | SP-117 | 2026-08-21 → [`Verified/Task-verified-0434-0440.md`](Verified/Task-verified-0434-0440.md) |

## Previously: SP-116's eight Tasks — ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0426 – T-0433** | SP-116's eight — D5/D6/D7 and six Issues | SP-116 | 2026-08-21 → [`Verified/Task-verified-0426-0433.md`](Verified/Task-verified-0426-0433.md) |

## Previously: SP-115's seven Tasks — ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0419 – T-0425** | SP-115's seven Tasks — the five carried EP-031 Issues, the two filings, and ⚠️ **T-0425 (I-0142, user-found)** | SP-115 | 2026-08-20 → [`Verified/Task-verified-0419-0425.md`](Verified/Task-verified-0419-0425.md) |

⚠️ **Archived in the SAME STEP they were verified** (`feedback_archive_on_close`) — deferring this rotted
four files once before.

⚠️ **One carried consequence:** **T-0420 is Verified at the CORE ONLY.** Nothing in Scrivi surfaces
`unsupportedWorldFormatVersion`, so a writer opening a too-new world sees *"unavailable"* with no
explanation. **The writer-facing surface is owed** — recorded so it is not mistaken for finished work.

✅ **SP-100 closed 2026-08-19 (user-approved)**, closing **EP-031**. Its three Tasks are ✅ Verified →
[`Verified/Task-verified-0390-0418-0391.md`](Verified/Task-verified-0390-0418-0391.md).

---

*Last Updated: 2026-08-24, fourteenth pass (**SP-120's six Tasks ✅ VERIFIED (user-approved) and ARCHIVED
in the same step** → `Verified/Task-verified-0453-0458.md`. Active Tasks 6 → **0**. ✅ **AC8 CLOSES**, and
⚠️ **T-0365's write half is paid after four sprints** — the `sources` card rendered CONTENT for the first
time since SP-102. ⚠️ **S11's widening to OPERATIONS earned its place immediately**: it found T-0455
before a writer did, and caught `CitationPopover` being unable to SHOW the fields the sprint was adding.
Next available Task: **T-0459**; Issue **I-0171**. Prior note follows.)*

*Last Updated: 2026-08-24, thirteenth pass (**all six SP-120 Tasks 🟠 IMPLEMENTED — Not Verified.**
`ctest` **567/567** — ⚠️ **unchanged, confirming SP-120 touched no core code**; app **BUILD SUCCEEDED**;
T-0453's logic **8/8 standalone incl. a negative control proving the `{"k","v"}` trap is real**.
✅ **Interop 127/127** (up from 122 — the five T-0453 tests, each confirmed BY NAME), run after the user
quit Scrivi. ⚠️ **ONE THING REMAINS OWED:** the **live click-through**, which is the only thing that has
ever found this Epic's defects (20 Issues across SP-118+SP-119, **none from a suite**).
⚠️ **AC8 is NOT closed.** Prior note follows.)*

*Last Updated: 2026-08-24, twelfth pass (**SP-120 ACTIVATED — T-0453–T-0458 created directly as Active**
in the same step. Active Tasks 0 → **6**. ⚠️ **They were never in `Task-backlog.md`** — they are new work
defined at activation, and the backlog is for unstarted *unassigned* work. ⚠️ **T-0455 and T-0456 exist
because of planning findings, not the AC**: AC8 names neither *attach an existing source* nor T-0365's
owed second entry point. Next available Task: **T-0459**. Prior note follows.)*

*Last Updated: 2026-08-24, eleventh pass (**SP-119's seven Tasks ✅ VERIFIED (user-approved) and ARCHIVED
in the same step** → `Verified/Task-verified-0446-0452.md`. Active Tasks 7 → **0**. ✅ **AC2, AC3, AC4 and
AC9's second half close.** ⚠️ **T-0449 disproved SP-117's own deferral reason** — the chip editor had
existed since T-0363. Next available Task: **T-0453**. Prior note follows.)*

*Last Updated: 2026-08-23, tenth pass (**SP-119 ACTIVATED — T-0446–T-0450 moved here from the backlog** in
the same step. Active Tasks 0 → **5**. ⚠️ **T-0446 begins with the `scanDir()` unification** so the image
field is added in exactly one place. Next available Task: **T-0451**. Prior note follows.)*

*Last Updated: 2026-08-23, ninth pass (**SP-118's five Tasks ✅ VERIFIED (user-approved) and ARCHIVED in
the same step** → `Verified/Task-verified-0441-0445.md`. Active Tasks 5 → **0**. ⚠️ **T-0443's deviation
from the plan was RULED IN FAVOUR by the user** — the second picker stays. ⚠️ **T-0441 is complete only
with I-0149**, which is why "reconcile on open" needed a second fix. Next available Task: **T-0446**.
Prior note follows.)*

*Last Updated: 2026-08-22, eighth pass (**SP-118 ACTIVATED — T-0441–T-0445 moved here from the backlog** in
the same step, and all five ✅ **Implemented — Not Verified** the same day. Active Tasks 0 → **5**.
⚠️ **T-0441 is the one that is genuinely proven** — negative control run, drifted fixture, both halves fail
without the fix. ⚠️ **T-0442–T-0445 have not been seen by a writer**, and five EP-034 defects have been
found by exactly that pass. ⚠️ **AC5/AC6 NOT closed** — they close on S9. ⚠️ **T-0443's deviation needs a
ruling.** ctest **559/559** arm64 · x86-64 · ASan; Linux **563/563** non-root; interop **119/119**; app
**BUILD SUCCEEDED**. Next available Task: **T-0446**. Prior note follows.)*

*Last Updated: 2026-08-21, seventh pass (**SP-117's seven Tasks ✅ VERIFIED (user-approved) and ARCHIVED in
the same step** → `Verified/Task-verified-0434-0440.md`. Active Tasks 7 → **0**. ⚠️ **AC2 still cannot close
— `tags` deferred to SP-119.** Next available Task: **T-0441**. Prior note follows.)*

*Last Updated: 2026-08-21, sixth pass (**all seven Tasks IMPLEMENTED — 🟠 Not Verified.** ctest
**554/554**, Linux **558/9332**, interop **115/115**, app **BUILD SUCCEEDED**. ⚠️ **T-0440 forced a
ScriviCore change** — the reason a world is unavailable was **discarded by `resolve()` and carried by no
envelope**, so the app could not have explained it however it was written. ⚠️ **S9 (live click-through) is
still owed** — only the user can supply it. Prior note follows.)*

*Last Updated: 2026-08-21, fifth pass (**SP-117 ACTIVATED — T-0434–T-0440 moved here**. Active Tasks 0 →
**7**. ⚠️ **T-0437 is the highest-risk item** (patch-don't-reconstruct); ⚠️ **T-0440 finally owns T-0420's
surface.** Next available Task: **T-0441**. Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (**SP-116's eight Tasks ✅ VERIFIED (user-approved) and ARCHIVED in
the same step** → `Verified/Task-verified-0426-0433.md`. Active Tasks 8 → **0**. ⚠️ **T-0420's surface is
still owed and still unowned.** Next available Task: **T-0434**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**T-0431 + T-0432 ADDED by user ruling.** ⚠️ **T-0432 supersedes
the `destExisted` rollback as the real answer to partial writes** — a rollback cannot run after a crash or a
yanked drive; a lock that stops being kicked can. Active Tasks 5 → **7** (5 🟠 Implemented, 2 🔵 new).
⚠️ **Network-hosted worlds are NOT in SP-116** — they need a locking model that does not rest on
`createFileExclusive`. Next available Task: **T-0433**. Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**all five Tasks IMPLEMENTED — 🟠 Not Verified.** ctest **536/536**
(arm64 · x86-64 · ASan/UBSan), interop **107/107**, app **BUILD SUCCEEDED**; ⚠️ **the Linux container leg
could not run — Docker unavailable.** ⚠️ **Only the user may mark these Verified**
(`feedback_verification`), and ⚠️ **no UI shipped**, so there is nothing a writer can click to verify —
these are core/boundary changes that SP-117 will surface. Prior note follows.)*

*Last Updated: 2026-08-21 (**SP-116 ACTIVATED — T-0426–T-0430 moved here from the backlog** in the same
step. Active Tasks 0 → **5**. ⚠️ **Detail lives in `../Sprints/Sprint-active.md`, not here** (P7).
⚠️ **Ordering constraint recorded**: T-0428 lands before or with T-0427, because T-0427 is what makes
I-0143 reachable. ⚠️ **T-0420's owed surface is still UNOWNED** and is **not** in SP-116 — this sprint
ships no UI. Next available Task: **T-0431**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115's seven Tasks ✅ VERIFIED by the user and ARCHIVED in the same step**
→ `Verified/Task-verified-0419-0425.md`. Active Tasks 7 → 0. ⚠️ **T-0420 is core-only Verified — its
surface is owed.** Next available Task: **T-0426**. Prior note follows.)*

*Last Updated: 2026-08-20 (**T-0425 added — I-0142, found by the USER during SP-115 verification.** The
object editor never showed an object's own world, and ⚠️ **renaming a world object was broken outright**
as a result. ✅ **User ruling: moving an object between worlds is DISALLOWED** — the picker becomes a
label. `ctest` **525/525**; app **BUILD SUCCEEDED**. Active Tasks 6 → 7; next available **T-0426**.
Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 IMPLEMENTED — all six Tasks 🟢 Implemented - Not Verified.**
`ctest` **524/524** · interop **103/103** · app **BUILD SUCCEEDED**. ⚠️ **T-0419 (I-0137) still needs the
REAL-RIG check** — drive ejected — before it can be Verified; a fixture cannot close it. **Only the user
marks a Task Verified.** Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 🟡 ACTIVATED for EP-034 — six Tasks T-0419–T-0424 now Active.**
⚠️ Scope is the five open Issues I-0135–I-0139 **and nothing else**; T-0424 **files** two further findings
rather than fixing them. Active Tasks 0 → 6. Next available Task: **T-0426**. Prior note follows.)*

*2026-08-19 (SP-100 closed; EP-031 closed; no Sprint active.)*
