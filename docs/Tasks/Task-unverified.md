# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

| ID | Title | Sprint | Epic | Implemented |
| -- | ----- | ------ | ---- | ----------- |

_No Tasks awaiting verification._

> ⚠️ **T-0365 must not be Verified on the card alone.** It renders correctly and shows *"No sources cited
> by this scene's objects"* — which is indistinguishable from working, because there is no way to create
> a source to test it with. Its write half is owed to [EP-034](../Epics/Epic-backlog.md), and §3.1.1's
> object-card entry point to `CitationPopover` is built but unwired.

> T-0389 is also listed in [`Task-active.md`](Task-active.md) because **SP-102 is still active** — it
> is implemented but its Sprint has not closed. It leaves both files on verification.
> ⚠️ Its AC23 live-verification half was deliberately **split out as T-0415**, because only a live
> ejectable-volume run can establish "restores with no writer intervention" — a fixture cannot, and
> burying that inside an implementation task is how it gets reported done on a fixture.

---

*Last Updated: 2026-08-18 (**Documentation audit.** ⚠️ **This file read "No Tasks awaiting
verification" while `Task-backlog.md` carried seven 🟠 Implemented — Not Verified rows** — T-0394,
T-0395, T-0366, T-0367, T-0368, T-0396, T-0397, T-0399. Six were in fact already Verified in their
Sprint archives and have been archived to `Verified/`; T-0394/T-0395 were recorded Verified
2026-08-11 with SP-092 on user ruling, their omission from SP-092's task table being clerical.
T-0389 is the one genuinely unverified Task and is now listed. Prior note follows.)*

*2026-07-06 (T-0184/T-0189/T-0190 verified & archived — EP-017 Spotlight end-to-end verified on a
signed build; iOS/visionOS deferred (T-0197).)*
