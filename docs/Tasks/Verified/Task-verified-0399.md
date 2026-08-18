# T-0399 — `[Apple]` Card Soft-Failure Isolation (EP-030, SP-101)

**Status:** ✅ **Verified 2026-08-11** (user-accepted on test evidence)
**Epic:** EP-030 — Scene Inspector Card Framework · ✅ **Closed** →
[`Epic-EP-030.md`](../../Epics/Closed/Epic-EP-030.md)
**Sprint:** SP-101 — EP-030 AC12 soft-failure isolation · **Record of truth:**
[`Sprint-SP-101.md`](../../Sprints/Closed/Sprint-SP-101.md)

---

## What shipped

Card soft-failure isolation as a **framework guarantee** rather than a per-card courtesy:

- a framework backstop in `CardBodyBoundary`,
- a **throwing `makeContent`** (the default implementation forwards, so existing cards are unchanged),
- a failing-card **test fixture**.

One card's failure never blocks the stack; the failed card shows an inline warning in place of its
content.

## Scope ruling — AC12 rescoped to soft failures

⚠️ **EP-030 AC12 was rescoped 2026-08-11 (user-approved): SwiftUI cannot catch a trapping view body,
so hard failures are out of scope.** The guarantee covers *soft* failures — a card that throws while
building its content — which is what the framework can actually honour.

## Verification note

**Not live-verifiable:** no UI path makes a card fail, so this was **user-accepted on test evidence**
(the failing-card fixture) rather than by observation in the running app.

## Relationship to T-0368

**T-0399 is T-0368's completion record.** T-0368 (SP-092) was to deliver failure isolation, but only
its per-card half existed at SP-092's close; the missing framework half was found in SP-094 and built
here. T-0368 is ✅ **Closed 2026-08-11 — delivered by T-0399**. Anyone auditing T-0368 should read
this file rather than look for missing work.

---

*Archived 2026-08-18 during the tracking-documentation audit. `Task-backlog.md` had carried this row
as 🟠 Implemented — Not Verified after SP-101 closed.*
