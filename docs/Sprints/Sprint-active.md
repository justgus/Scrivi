# Active Sprint

**No Sprint is currently active.**

**SP-100 was ✅ CLOSED 2026-08-19 (user-approved)** → [`Closed/Sprint-SP-100.md`](Closed/Sprint-SP-100.md).
⚠️ **Its close closed EP-031** → [`../Epics/Closed/Epic-EP-031.md`](../Epics/Closed/Epic-EP-031.md).

---

## What SP-100 delivered

EP-031's **eleventh and last** sprint — planned, activated and closed on 2026-08-19.

| Task | Outcome |
| ---- | ------- |
| **T-0390** | Repair-matrix world conditions — **§6a, 578 → 826 lines, world mentions 0 → 72** | ✅ Verified |
| **T-0418** | ⚠️ Live-use pass on the real USB rig | ✅ Verified |
| **T-0391** | AC1–AC10 verification + Epic close prep | ✅ Verified |

**All ten ACs verified. AC1 and AC10 were AMENDED first** (rulings R1/R2) — each carried a clause the
Epic's own §3.0 no-migration ruling had superseded, so verifying them as written would have asserted
behaviour EP-031 deliberately chose not to build. ⚠️ **AC10's amended form is *stronger***, requiring both
architectures and sanitizers.

**Suites at close:** `ctest` **520/520 macOS arm64** · x86-64 + sanitizers ✅ (CI 2×2; ScriviCore verified
byte-identical to that run) · macOS interop **99/99 in 10 suites** · app **BUILD SUCCEEDED**.

---

## ⚠️ Five Issues filed — none fixed, by design

Rulings **R3** and **R4** required findings to be **filed, not fixed**: a verification sprint that writes
fixes stops being one.

| Issue | Severity | Found by |
| ----- | -------- | -------- |
| **I-0137** | **High** | T-0418 — ⚠️ **AC24's refinement cannot fire on real hardware** |
| I-0136 | Medium | T-0390 — `formatVersion` read but never compared |
| I-0139 | Medium | T-0418 — editor exit discoverability |
| I-0135 | Low | T-0390 — no coverage for a corrupt `world.json` |
| I-0138 | Low | T-0418 — disabled-but-unexplained removal |

⚠️ **All five need triage. None blocked the Epic's close.**

---

## The finding that outlives the Epic

**I-0137.** `WorldVolumeStatus.refine` is correct, unit-tested and correctly wired — and guards on
`packagePath.isEmpty`, while `WorldStore::listWorlds` supplies `packagePath` **only when the world is
`available`**. ⚠️ **The one input it needs is guaranteed absent in the only case it exists for.**

⚠️ **The interop suite contains a suite named *"World volume status refinement (EP-031 AC24)"* — and it
passes.** That is not a contradiction; it *is* the defect. **What no unit test can see is that the caller
never supplies the input.** *The tests pass and the feature does not reach the writer.*

> ⚠️ **A passing test suite named after an acceptance criterion is not evidence the criterion is met in
> the product.** This is the cleanest example the project has produced — and it was found by a writer
> ejecting a drive, not by CI.

---

## Next

**No Epic is active.** Candidates in [`../Epics/Epic-backlog.md`](../Epics/Epic-backlog.md):
**EP-034** `[Cross]` Object Detail & Media *(owes source creation — EP-031's largest carried gap)* ·
**EP-032** · **EP-033** · **EP-026** `[Linux]`.

**Next available:** Sprint **SP-107** · Task **T-0419** · Issue **I-0140** · Epic **EP-035**.

---

*Last Updated: 2026-08-19 (SP-100 closed; EP-031 closed; no Sprint active.)*
