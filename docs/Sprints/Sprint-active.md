# Active Sprint

**No Sprint is currently active.**

**SP-117 was ✅ CLOSED 2026-08-21 (user-approved)** → [`Closed/Sprint-SP-117.md`](Closed/Sprint-SP-117.md).
**EP-034 remains 🟡 Active** — SP-118 is next.

---

## What SP-117 delivered

EP-034's **third** sprint, and ⚠️ **the first to ship a writer-facing surface**. Two sprints had closed
before it with their criteria met at the ABI and **nothing reachable in the product** — the very defect the
Epic exists to cure.

**All seven Tasks and I-0148 ✅ Verified.**

| Task | Delivered |
| ---- | --------- |
| T-0434 | **D1-E** — the Detail Sheet as an editor-level, **non-modal** pane |
| T-0435 | **D2-B** — explicit **back/forward** history |
| T-0436 | ⚠️ **The app's FIRST typed object model** — `subtitle`/`notes` had been unreachable since SP-095 |
| T-0437 | **D3-A** — fields + ⚠️ **patch-based save** |
| T-0438 | **R7** — double-click **and** right-click → "View Detail" |
| T-0439 | **R9** — pending objects read-only **and explained** |
| T-0440 | ⚠️ **T-0420's surface, owed since SP-115** — finally paid |

**Suites at close:** `ctest` **554/554** (arm64 · x86-64 · ASan) · **Linux 558/9332** (GCC 13) · interop
**115/115** · app **BUILD SUCCEEDED**.

---

## ⚠️ The two things this sprint proved

### 1. A trade can be ruled correctly and still not be understood until it exists

> **The user, who chose D1-E:** *"I was expecting a popup. But I love the navigation push. I know it's what
> I asked for, but I wasn't able to grasp the full interaction before I saw it."*

⚠️ **The strongest argument yet against another core-only sprint.**

### 2. `capability_without_surface` can be caused by a missing FIELD, not a missing view

**T-0440 could not be done in Swift at all.** `WorldStore::resolve` **discarded** the reason a world was
unavailable — it returns a *status*, not an error, so `unsupportedWorldFormatVersion` died in the core and
**no envelope carried it**. ⚠️ **The app could not have explained it however it was written.**

⚠️ **The fix for I-0136 was Verified at the core in SP-115 and stayed invisible in the product for two
sprints**, while the facade test asserting it passed the whole time — because it stops at `resolve()`.

---

## ⚠️ Found by the LIVE CLICK-THROUGH — the fifth such defect in EP-034

**I-0148**, reported by the user as an *observation* inside an otherwise glowing report:

> *"When the disk is unmounted, the Notes field is still editable."*

⚠️ **`.disabled()` does not make a `TextEditor` read-only.** ✅ Never a write-safety bug — Save is hidden
when read-only — ⚠️ **but typing during an outage was silently discarded on navigation.**

⚠️ **Claude's first assessment ("R9 violated") was too strong and the user corrected it**; the user's own
framing ("read only, in a sense") was also incomplete. **Neither party had named the real cost until the
code was read.**

> ⚠️ **A satisfied user is not a green suite.** Five defects in this Epic have now been found by use
> (I-0137, I-0142, I-0146, I-0147, I-0148) — and this is the first reported without being recognised as one.

---

## ⚠️ Carried out of SP-117 — do not read as delivered

| Item | Owner |
| ---- | ----- |
| ⚠️ **`tags` deferred** (user ruling) — **R2 not fully met, AC2 CANNOT CLOSE** | **SP-119** |
| ⚠️ **AC9 is HALF met.** ✅ Pending objects read-only/explained, verified by use. ⚠️ **A world going away *while a sheet is open* is NOT demonstrated** — a different trigger from the one tested (`feedback_verify_each_half_separately`) | **SP-119** |
| **AC3–AC8** — images, relationships, sources | **SP-118 – SP-120** |
| **I-0147** — the 60 s post-crash lock window; ✅ **Accepted**, regression-tested | **Network-worlds design** |
| ⚠️ **`ObjectIndex::loadWorldIndex`'s rebuild is still UNLOCKED** — `WorldLock` is not reentrant | **Network-worlds design** |

---

## Next

**SP-118** — `[Apple]` Related objects + relationship creation (**D4**), closing **AC5** and **AC6**.

> ⚠️ **T-0416 may surface as a LIVE BLOCKER here.** Seeded relation types never reach existing projects, so
> a writer relating objects in an existing project (e.g. `tintagael`) may see only that project's
> vocabulary. **Worth confirming on the real rig before SP-118 plans.**

⚠️ **SP-107–SP-114 remain RESERVED to EP-032** and are not available.

**Next available:** Sprint **SP-118** · Task **T-0441** · Issue **I-0149**.

---

*Last Updated: 2026-08-21 (**SP-117 ✅ CLOSED, user-approved.** Seven Tasks + I-0148 Verified and archived in
the same step. ⚠️ **The first EP-034 sprint a writer could use** — and the live pass both validated the D1-E
ruling and found I-0148. Active Sprints 1 → 0; EP-034 stays 🟡 Active, **3 of 8 sprints closed**.)*
