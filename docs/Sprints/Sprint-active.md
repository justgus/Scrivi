# Active Sprint

**No Sprint is currently active.**

**SP-115 was ✅ CLOSED 2026-08-20 (user-approved)** → [`Closed/Sprint-SP-115.md`](Closed/Sprint-SP-115.md).
**EP-034 remains 🟡 Active** — SP-116 is next.

---

## What SP-115 delivered

EP-034's **first** sprint — planned, activated, implemented, verified and closed on 2026-08-20.

**All seven Tasks and all six Issues ✅ Verified.** Scope was *"the five carried EP-031 Issues and nothing
else"*; it grew by exactly one Task (**T-0425**) for a defect found **during verification**.

| Issue | Sev | Task | Outcome |
| ----- | --- | ---- | ------- |
| **I-0137** | **High** | T-0419 | ✅ ⚠️ **verified on the real rig, drive ejected** |
| I-0136 | Medium | T-0420 | ✅ ⚠️ **at the CORE ONLY — surface owed** |
| I-0139 | Medium | T-0421 | ✅ Verified |
| I-0135 | Low | T-0422 | ✅ Verified |
| I-0138 | Low | T-0423 | ✅ Verified |
| **I-0142** | **High** | T-0425 | ✅ ⚠️ **found by the USER, not a suite** |

**Suites at close:** `ctest` **525/525** (was 520) · macOS interop **103/103 in 10 suites** (was 99) · app
**BUILD SUCCEEDED**.

---

## ⚠️ The lesson this sprint proved twice

> **User, at close:** *"ctest, and unit tests, and integration tests are designed to test specific things.
> But the true user experience can only be tested live, in app."*

1. ⚠️ **I-0137 — a suite named after the acceptance criterion PASSED while the feature could not fire.**
   `refine` was correct, unit-tested and correctly wired; the datum it needed never arrived, because
   `packagePath` was populated only for *available* worlds — never for the case the refinement exists for.
   **Only ejecting a real drive proved it.**
2. ⚠️ **I-0142 — the user found in five minutes what 628 automated tests did not.** The visible symptom
   (an empty world picker) was the lesser half: **renaming any world-scoped object was failing silently.**

> ⚠️ **A passing test suite named after an acceptance criterion is not evidence the criterion is met in
> the product.**

---

## ⚠️ Carried out of SP-115 — do not read as delivered

| Item | Owner |
| ---- | ----- |
| **T-0420 has no writer-facing surface** — a writer opening a too-new world sees *"unavailable"* with **no explanation**. ⚠️ `capability_without_surface`, shipped by the very sprint that fixed four instances of it | ⚠️ **unassigned — needs an owner** |
| **I-0140 + I-0141** — filed by T-0424, fixed by neither, **by design** | **SP-116** (design-doc **D5**) |

---

## Next

**SP-116** — `[ScriviCore]` world assets + `assetPath` + the kind-scope endpoint (**D6, D7, D5**).
⚠️ **D5 also retires I-0140 and I-0141.**

⚠️ **SP-107–SP-114 remain RESERVED to EP-032** and are not available.

**Next available:** Sprint **SP-116** · Task **T-0426** · Issue **I-0143**.

---

*Last Updated: 2026-08-20 (**SP-115 ✅ CLOSED, user-approved.** Seven Tasks + six Issues Verified and
archived in the same step. ⚠️ **Two items carried out**: T-0420's missing surface (**unowned**) and
I-0140/I-0141 (**SP-116**). Active Sprints 1 → 0; EP-034 stays 🟡 Active.)*
