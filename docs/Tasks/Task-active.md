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

## Currently: **no active Tasks** — SP-116's eight are ✅ Verified and archived

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| ✅ **T-0426 – T-0433** | SP-116's eight Tasks — D5/D6/D7, four Issues, and ⚠️ **two more found while fixing them** (I-0144's uncalled lock, I-0146's orphaned partial) | SP-116 | 2026-08-21 → [`Verified/Task-verified-0426-0433.md`](Verified/Task-verified-0426-0433.md) |

⚠️ **Archived in the SAME STEP they were verified** (`feedback_archive_on_close`).

⚠️ **Two consequences carried out of SP-116:**

- **No UI shipped**, by design — so **AC3 and AC9 remain open** and belong to **SP-117**.
- **T-0420's missing surface is STILL UNOWNED** (carried from SP-115). ⚠️ **SP-117 is the first sprint that
  could take it**, since it is the first to ship a writer-facing surface at all.

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
