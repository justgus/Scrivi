# Active Sprint

**No Sprint is currently active.**

**SP-116 was ✅ CLOSED 2026-08-21 (user-approved)** → [`Closed/Sprint-SP-116.md`](Closed/Sprint-SP-116.md).
**EP-034 remains 🟡 Active** — SP-117 is next.

---

## What SP-116 delivered

EP-034's **second** sprint: three design trades (**D5, D6, D7**), **four Issues fixed**, and ⚠️ **two more
found while fixing them**.

**All eight Tasks and all six Issues ✅ Verified.** Scope grew from five Tasks to eight — **every addition
came from a defect found during the work**, none from re-planning.

| Task | Delivered |
| ---- | --------- |
| T-0426 | **D6** — world assets live in the `.scrivworld` package and **travel with the world** |
| T-0427 | **D7** — `assetPath` from `list_assets` |
| T-0428 | **I-0143** — the assets array routes through `JsonDoc` |
| T-0429 | **D5** — `scrivi_list_object_kinds`, **derived**, and adopted in Swift |
| T-0430 | **I-0141** — `scrivi.h` states the rule by reference |
| T-0431 | ⚠️ **I-0144** — `WorldWriteGuard` locks **every** world-package write path |
| T-0432 | ⚠️ **Block transfer + per-block watchdog** (user ruling) |
| T-0433 | ⚠️ **I-0146** — stale-lock sweep of abandoned `*.partial` files |

**Suites at close:** `ctest` **552/552** macOS arm64 · x86-64 · ASan/UBSan (**was 525**) · **Linux 556
cases / 9300 assertions** (GCC 13) · interop **107/107** · app **BUILD SUCCEEDED**.

---

## ⚠️ The lesson this sprint proved — twice more

**Four defects in EP-034 have now been found by USE, not by tests** (I-0137, I-0142, I-0146, I-0147). Two
were in this sprint.

1. ⚠️ **I-0144 — a lock that shipped complete, correct, unit-tested, and was NEVER CALLED.** Every object
   write into a shared world was unserialised for three sprints. **A green suite for a capability says
   nothing about whether anything invokes it.**
2. ⚠️ **I-0146 — a USB drive physically pulled mid-import left a 459 MB `.partial`** that no Scrivi
   operation could ever reclaim. **The lab test asserting "no partial remains" passed throughout** — it can
   only test failures the writing process *survives*.

> ⚠️ **And three times my own test setup was easier than reality** — the staged orphan omitted the matching
> fresh lock (twice), and the competing-writer rig used a clock whose heartbeat read as stale.
>
> **Staging the AFTERMATH of a failure is not staging the FAILURE.** It silently omits whatever else the
> failure leaves behind — here, the very thing that blocked the fix.

---

## ⚠️ Carried out of SP-116 — do not read as delivered

| Item | Owner |
| ---- | ----- |
| ⚠️ **No UI shipped**, by design. S1–S14 are core-and-boundary criteria — **AC3 and AC9 cannot close** without the Detail Sheet | **SP-117** |
| **T-0420's missing surface** — a writer opening a too-new world still sees *"unavailable"* with no explanation. ⚠️ Carried from **SP-115**, still unowned | ⚠️ **SP-117 is the first sprint that could take it** |
| **I-0147** — the 60 s window after an interrupted world write where the world is unwritable and its orphan unreclaimable. ✅ **Accepted** (user-ruled); a regression test asserts it | **Network-worlds design** |
| ⚠️ **`ObjectIndex::loadWorldIndex`'s rebuild is still UNLOCKED** — `WorldLock` is not reentrant and `save`/`remove` reach it while holding the lock | **Network-worlds design** (needs a reentrant lock) |

---

## Next

**SP-117** — `[Apple]` Detail Sheet shell: pane, navigation, fields, save (**D1, D2, D3**).

⚠️ **It is the first sprint of this Epic to ship a writer-facing surface**, which makes it the first that
can close **AC3/AC9** — and the first that could take **T-0420's owed explanation**.

⚠️ **SP-107–SP-114 remain RESERVED to EP-032** and are not available.

**Next available:** Sprint **SP-117** · Task **T-0434** · Issue **I-0148**.

---

*Last Updated: 2026-08-21 (**SP-116 ✅ CLOSED, user-approved.** Eight Tasks + six Issues Verified and
archived in the same step. ⚠️ **Two defects found by live use** (I-0144, I-0146) and **one accepted
limitation** (I-0147). Active Sprints 1 → 0; EP-034 stays 🟡 Active.)*
