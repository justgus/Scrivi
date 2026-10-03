---
sprint: SP-152
epic: none
status: Closed
closed: 2026-10-03
platform: Cross
created: 2026-10-02
activated: 2026-10-02
---

# SP-152 — `[Cross]` Carried defect from SP-151 — [I-0268]

**Status:** ✅ **CLOSED 2026-10-03 (user-approved):** *"close SP-152"*. (Activated 2026-10-02; complete 2026-10-03.)
**Epic:** none.
**Size:** ✅ **SMALL** — one defect.

---

✅ **Created at [SP-151]'s close by user ruling:** *"close SP-151 and carry the Issues into the next Sprint."*
⚪ **[I-0267] REMOVED 2026-10-02** — a duplicate of [I-0206]'s accepted keystroke cost (user: *"I thought I ruled on that as not a problem earlier"*) → `../Issues/Closed/Issue-closed-0267.md`.
**Epic:** none. ✅ **ACTIVATED 2026-10-02 by the user:** *"activate SP-152."*

| ID | Title | Severity | Note |
| -- | ----- | -------- | ---- |
| **[I-0268]** | `[ScriviCore]` History fails to open on `the-stairs-of-tintagael` — `unknown node` | High | ⚠️ Real writing work: forensics on a COPY of the history log only. Same message as [I-0110] (fixed SP-093) |

## Acceptance Criteria
- [x] **AC1** — [I-0268] root cause shown on a COPY of the log; history opens; proven through `scrivi_*`. ✅ 2026-10-03 — ✅ **USER-VERIFIED on the real project:** *"I verify that undo works and displays the external change notice."*
- [x] **AC-build** — macOS + iOS build; `ctest` green on macOS and Linux (if the core changes). ✅ 2026-10-03 — macOS + iOS + visionOS BUILD SUCCEEDED; `ctest` 646/646 macOS, 650/650 Linux (GCC, non-root).

---

## Progress log

### 🟡 2026-10-02 — Sprint ACTIVATED

⚠️ **[I-0268] not yet investigated.** ⚠️ It concerns REAL writing work (`the-stairs-of-tintagael` on the
`Scrivi-Worlds` USB volume): ✅ the history log is COPIED to the scratchpad and examined there — ⛔ never edited
or opened by a tool in place.

### 🟢 2026-10-03 — [I-0268] root-caused, fixed, Resolved - Not Verified

✅ **Cause PROVEN, not read:** the log jumps seq 3862 → 3869, and the last event's parent is in no record.
✅ The system log shows the project's USB drive **removed without eject three times** while it was open
(15:37:13, 15:37:58, 15:41:40 EDT, 2026-09-30). The lost parent was minted at 15:42:04, inside the
third outage.

⛔ **Two core defects, both fixed:** (A) `appendLine` discarded a failed append → now retried ahead of
the next record and at `checkpoint()`; (B) the open threw on an orphan → `finalizeLoad` drops orphan
subtrees and falls back to the last surviving pointer.

✅ Two `[I-0268]` boundary tests, **red against the un-fixed core**. ✅ The **copy** of the real log opens
through `scrivi_history_open` (un-fixed: the reported error). ✅ ctest 646/646 macOS, 650/650 Linux.
✅ macOS / iOS / visionOS builds succeeded. ⚠️ **The drive was not touched**: only copies in the scratchpad
were opened. Details: [I-0268] row in `../Issues/Issue-active.md`.

⚠️ **Verification owed:** open `the-stairs-of-tintagael` with this build; history should open and undo
should work. Expect ONE `externalChange` repair, on `scene_019faeb3…`, whose newest edit was lost.

### ✅ 2026-10-03 — [I-0268] VERIFIED; Sprint COMPLETE

✅ **User:** *"I verify that undo works and displays the external change notice."* ✅ I-0268 archived →
`../Issues/Verified/Issue-verified-0261-0270.md` in the same step. ✅ **Both ACs met. ⚠️ Awaiting user
approval to CLOSE.**

### ✅ 2026-10-03 — Sprint CLOSED (user-approved)

✅ *"close SP-152"*. ✅ [I-0268] was already verified and archived. ✅ Nothing carried forward.
