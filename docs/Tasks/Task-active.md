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

## Currently: **no active Tasks** — SP-118's five are ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0441 – T-0445** | SP-118's five — ⚠️ **T-0416's reconciliation (with I-0149)**, the related-objects section, inline creation, push-navigation, and pending far-endpoints | SP-118 | 2026-08-23 → [`Verified/Task-verified-0441-0445.md`](Verified/Task-verified-0441-0445.md) |

⚠️ **Archived in the SAME STEP they were verified** (`feedback_archive_on_close`).

⚠️ **Thirteen Issues (I-0149 – I-0161) were raised against these Tasks after the suites went green**, every
one found by the user's live click-through and none by any suite. All ✅ Verified and archived with them.

⚠️ **Carried out of SP-118, unchanged:** `tags` is deferred to **SP-119**, so **AC2 still cannot close**;
AC9's second half ("a world goes away *while a sheet is open*") is also SP-119's.

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
