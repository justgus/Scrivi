# Active Sprint

**No Sprint is currently active.**

**SP-119 was ✅ CLOSED 2026-08-24 (user-approved)** → [`Closed/Sprint-SP-119.md`](Closed/Sprint-SP-119.md).
**EP-034 remains 🟡 Active** — SP-120 is next.

---

## What SP-119 delivered

EP-034's **fifth** sprint. **All seven Tasks and seven Issues ✅ Verified.**

| Task | Delivered |
| ---- | --------- |
| T-0446 | `image` in the object index — ⚠️ **exposed THREE duplications where one was predicted** |
| T-0447 | **AC3** — import / display / replace / remove an object image; ✅ **S11 written first** |
| T-0448 | **AC4 / D8** — card thumbnails, async, never blocking |
| T-0449 | **AC2** — tags; ⚠️ **SP-117's deferral reason was wrong — the chip editor already existed** |
| T-0450 | **AC9's second half** — unsaved edits at risk, warned |
| T-0451 | **I-0164** — attach an asset already in the world |
| T-0452 | **I-0167 + I-0168** — Cancel, and a guarded exit on all six routes |

**Closes AC2, AC3, AC4 and AC9's second half.**

**Suites at close:** `ctest` **567/567** · interop **122/122** · app **BUILD SUCCEEDED**.

---

## ⚠️ Seven Issues, none found by any suite

SP-118 raised thirteen; SP-119 raised seven. ⚠️ **In both sprints, not one came from a test.** The suites
were green throughout and were not wrong — they assert what the core does. **What no test covered was
whether a writer could reach it, and what happened when the world moved under her.**

### ⚠️ Six data-loss routes into ONE surface

Stale image path (I-0162) · sheet destroyed on eject (I-0165) · every reload clobbered drafts, live since
SP-117 (I-0165b) · cold open showed a raw error code (I-0166) · ✕/back/forward/related-list discarded
edits (I-0167) · Scene Inspector bypassed the guard (I-0168).

⚠️ **Claude's failure was consistent — fix the trigger in front of it, treat the class as closed.**
✅ **What worked was moving the DECISION to one owner**, not adding another check.

---

## ⚠️ Two rulings that outlive this sprint

**Core-before-surface is the CORRECT sequence, not a defect.** ScriviCore is the capability layer; the app
is one surface over it. ⚠️ **The real risk is the design phase** — designing to the lowest common
denominator rather than to the capability that exists. ✅ **S11 is the guard**, and it works;
⚠️ **it must enumerate OPERATIONS as well as fields** (the gap that let I-0164 through).

**An unlinked asset is a library entry, not debris.** An orphaned image may remain and be linked to other
world objects as needed. **Asset deletion is unbuilt scope.**

---

## ⚠️ Carried out of SP-119

| Item | Owner |
| ---- | ----- |
| **AC8** — source creation + footnote text (R6) | **SP-120** |
| **AC11** — `[Linux]` parity | **SP-121** |
| **AC12** — full suite verification + Epic close prep | **SP-122** |
| ⚠️ **S11 must enumerate OPERATIONS as well as fields** | **SP-120** |
| ⚠️ **Thumbnail generation** — `thumbnailAssetID` is read but never written | Unscheduled |
| ⚠️ **Asset deletion** — ✅ ruled acceptable to omit | Unscheduled |
| **I-0147** — the 60 s post-crash lock window; ✅ **Accepted** | **Network-worlds design** |

---

## Next

⚠️ **No Sprint is planned.** SP-120 has not been defined — ⚠️ **Claude does not plan or activate one
without the user's word.** Its subject is **AC8**: source creation from the documented object, closing
T-0365's write half and unblocking EP-032.

⚠️ **SP-107–SP-114 remain RESERVED to EP-032** and are not available.

**Next available:** Sprint **SP-120** · Task **T-0453** · Issue **I-0169**.

---

*Last Updated: 2026-08-24 (**SP-119 ✅ CLOSED, user-approved.** Seven Tasks + seven Issues Verified and
archived in the same step. ⚠️ **Seven Issues, none from any suite; six were data-loss routes into one
surface.** Active Sprints 1 → 0; EP-034 stays 🟡 Active, **5 of 8 sprints closed**.)*
