---
sprint: SP-143
epic: EP-041
status: Planned
task: T-0510
platform: Cross
---

# Sprint SP-143 — `[Cross]` The regression guard ([I-0197] closure)

**Epic:** [EP-041](../Epics/Epic-EP-041.md) — `[Cross]` **The Boundary** · **Status:** 🔵 **PLANNED**
**Task:** **T-0510** · **Blocks on:** ⛔ **[SP-142]'s VERIFICATION and [SP-149]'s guard.**

⚠️ **FRONT-MATTER CORRECTED 2026-09-21: this read `epic: EP-040`** — ✅ **stale since the 2026-09-18
split that moved the [I-0197] chain into [EP-041].** ⚠️ **[SP-141] and [SP-142] carried the same
error; ✅ all three are now fixed.**

---

## ⚠️ SCOPE NARROWED 2026-09-21 — ✅ **[SP-149] now owns the guard's DESIGN AND BUILD**

⚠️ **THIS SPRINT NO LONGER BUILDS THE GUARD.** ✅ **[SP-149] was split out on 2026-09-21 (user ruling
Q4) because the guard's DESIGN blocked on nothing while its VERIFICATION blocked on two Sprints** —
⛔ **bundled, the design would not have started until the blockers cleared, which is how [EP-041]'s own
record predicted AC4 would slip.**

✅ **WHAT MOVED TO [SP-149]:** building the script, wiring it into CI (⚠️ **including creating
`scrivi-apple-ci.yml` — `Scrivi/`'s FIRST CI**), the allow-list with reasons, and proving it fails.

✅ **WHAT REMAINS HERE — and it is a REAL job, not a leftover:**
⚠️ **[SP-149] proves the guard fails on a defect IT introduces.** ⛔ **That is not the same as proving
it policed the RETIREMENT.** ✅ **This Sprint is the INDEPENDENT WITNESS: it confirms the guard is
RED before [SP-142]'s change and GREEN after** — ⚠️ **the check a guard written by its own author
cannot make for itself** (`feedback_boundary_tests_not_facade`'s sibling logic, and the reason the
[SP-130] restructure kept these apart).

⚠️ **STATUS OF THE DEPENDENCY, MEASURED 2026-09-21** — ✅ **the allow-list transition this Sprint was
waiting for HAS ALREADY HAPPENED:**

| Owner | Package I/O in `InspectorLayoutStore` |
| ----- | ------------------------------------ |
| **Apple** | ✅ **ZERO** ([SP-141] / T-0507, user-Verified) |
| **Linux** | ✅ **ZERO** ([SP-142] / T-0537 — ⚠️ **Implemented, NOT user-Verified**) |

⛔ **SO THE REMAINING BLOCKER IS [SP-142]'s LIVE PASS, NOT ITS CODE.** ⚠️ **Running the witness against
an unverified retirement means witness and subject are both unconfirmed** — ✅ **and if the live pass
forces another change, a guard asserted against today's code would have been asserted against code
that then moved.**

## ⚠️ WHY THIS GOES LAST — ⚠️ **HISTORICAL, and its premise is now SPENT**

⚠️ **KEPT FOR THE REASONING, NOT AS CURRENT STATE (2026-09-21).** ✅ **The "before" phase it describes
is over: `InspectorLayoutStore` touches the package on NEITHER platform.** ⛔ **So the allow-list
transition below is no longer a thing to wait for — it is a thing to CONFIRM, which is this Sprint's
narrowed job.** ✅ **The argument is still worth reading: it is why the guard was not written early.**

### (original text)

⚠️ **The guard must FAIL when Swift touches a project-package path.** ⚠️ **But TODAY, Swift touching
`inspector-layout.json` is LEGITIMATE** — ✅ **it is how the feature works until [SP-141] lands.**

| phase | ⚠️ allow-list must contain |
| ----- | ------------------------- |
| **Before [SP-141]** | ⚠️ **`InspectorLayoutStore` (3 sites)** + the 2 permanent non-violations + T-0508's ruled sites |
| **After [SP-141]/[SP-142]** | ⛔ **`InspectorLayoutStore` entries REMOVED** |

⛔ **A GUARD WRITTEN EARLY WOULD PASS GREEN ACROSS THE VERY CHANGE IT EXISTS TO POLICE.**
⚠️ **That is not a guard; it is a rubber stamp.** ✅ **Written last, it FAILS until BOTH owners are
gone** — ⚠️ **which is what makes splitting [SP-141]/[SP-142] safe: the drift cannot quietly survive.**

## Task

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0510** | ⚠️ **WITNESS [SP-149]'s guard across the retirement — RED before [SP-142], GREEN after — and CLOSE [I-0197]** | **Medium** | 🔵 Not started |

⚠️ **T-0510 WAS RESCOPED 2026-09-21.** ⛔ **It formerly read *"a test or CI grep that FAILS when Swift
touches a project-package path directly, with an explicit allow-list"*** — ✅ **that is now [SP-149] /
T-0541.** ⚠️ **Leaving the old wording would have had two Tasks claiming to build one guard.**

## Definition of Done — ⚠️ **NARROWED 2026-09-21**

- [ ] **D1** — ⚠️ **THE GUARD IS RED AGAINST THE PRE-RETIREMENT CODE.** ✅ **Check out (or reconstruct)
      Linux's `InspectorLayoutStore` as it stood BEFORE [SP-142] and confirm [SP-149]'s guard FAILS
      on it, naming the site.** ⛔ **This is the assertion [SP-149] structurally cannot make** — ⚠️ **its
      guard was written when the code was already clean, so it has only ever seen green here.**
- [ ] **D2** — ✅ **THE GUARD IS GREEN against the retired state**, ⚠️ **with `InspectorLayoutStore`
      ABSENT from the allow-list** — ⛔ **not merely passing because it was excused.**
- [ ] **D3** — ⚠️ **[I-0241]'s allow-list entry is GONE if T-0542 has landed** — ✅ **an entry naming a
      closed Issue is stale by definition** (⚠️ [SP-149] AC7 records it as a DEBT with an owner).
- [ ] **D4** — ✅ **[I-0197] is CLOSED.** ⚠️ **Classes A and C closed under [SP-129]/[SP-130]; Class B
      closes when BOTH owners are gone AND the guard witnesses it** — ⛔ **which is D1+D2.**

⛔ **MOVED TO [SP-149] — ⚠️ do not re-do them here:** building the script · wiring it into CI ·
the allow-list and its reasons · proving it fails on an introduced defect.
✅ **Their status lives in [`Sprint-SP-149.md`](Sprint-SP-149.md)**, ⚠️ **not restated here** (P7).

## ⚠️ Risk

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The allow-list becomes a dumping ground** | ⚠️ **An entry without a reason is a silent exception.** ✅ **[SP-149] AC4 requires each carry one; ⚠️ D3 here removes the one that is a DEBT** |
| ⚠️ **A grep-based guard is brittle** | ✅ **Accepted: it is a TRIPWIRE, not a type system.** ⚠️ **Its job is to make the class LOUD when it returns, not to prove absence** |
| ⛔ **D1 quietly skipped as "obviously fine"** | ⚠️ **IT IS THE WHOLE SPRINT.** ✅ **A guard that has only ever seen clean code is untested against the defect it exists for** — ⛔ **and reconstructing the pre-[SP-142] store takes minutes (`git show`)** |
| ⚠️ **Running before [SP-142] is user-Verified** | ⛔ **The subject would be unconfirmed.** ✅ **If the live pass forces another change, D1/D2 were asserted against code that then moved** |
