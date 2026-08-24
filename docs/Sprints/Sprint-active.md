# Active Sprint

**No Sprint is currently active.**

**SP-120 was ✅ CLOSED 2026-08-24 (user-approved)** → [`Closed/Sprint-SP-120.md`](Closed/Sprint-SP-120.md).
**EP-034 remains 🟡 Active** — SP-121 is next.

---

## What SP-120 delivered

EP-034's **sixth** sprint. **All six Tasks and both Issues ✅ Verified.**

| Task | Delivered |
| ---- | --------- |
| T-0453 | `attributes` read/write — the `[{"k","v"}]` wire form; ⚠️ **round-trip test written FIRST** |
| T-0454 | **AC8** — create a `source` from the documented object + attach `cites` |
| T-0455 | ⚠️ **Cite an EXISTING source** — the I-0164 mirror, **found by S11 before a writer hit it** |
| T-0456 | ⚠️ **T-0365's second entry point** — owed since SP-102, owned by **no AC and no sprint** |
| T-0457 | Citation fields + footnote text; ⚠️ **the popup learned to SHOW them** |
| T-0458 | ⚠️ **S11 written FIRST**, widened to **OPERATIONS** |

✅ **Closes AC8.** ✅ **T-0365 is COMPLETE** — no longer a partial delivery.

**Suites at close:** `ctest` **567/567** · interop **127/127** · app **BUILD SUCCEEDED**.

---

## ⚠️ The sprint changed NO core code, and that is the evidence

Every backend piece of AC8 already shipped — the `source` kind (SP-098), `cites` unconstrained on both
ends (SP-096), `create_object`, `create_edge`, and a round-tripping `attributes` map ⚠️ **read by nothing
since SP-095.**

⚠️ **`ctest` holding at 567/567 is the proof the `[Apple]` scoping was right**, not an absence of proof.
This was `capability_without_surface` for the **third** time inside EP-034.

⚠️ **The `sources` card rendered CONTENT for the first time since SP-102.** For four sprints it could only
draw its empty state — indistinguishable from working correctly.

---

## ⚠️ S11 earned its place three times

Widened from **fields** to **OPERATIONS** because ⚠️ **fields-only is exactly how I-0164 slipped through
SP-119.**

1. It found **T-0455** — an *operation* no field table could surface.
2. It caught **`CitationPopover` unable to SHOW** the new fields — ⚠️ **the trap reappearing inside the
   sprint curing it.**
3. It **shortened the work**: `notes` already *was* AC8's footnote text.

⚠️ **But S11 also flagged `page` as being on the wrong object and it shipped anyway** — and the limit was
hit **one sprint later** by real citation work (**T-0459**). ✅ Recording it made that a five-minute
diagnosis; ⚠️ **recording a known-wrong model is not the same as not shipping one.**

---

## ⚠️ Two Issues, neither from a suite

| Issue | What |
| ----- | ---- |
| **I-0169** | The `sources` card had **no route to the Detail Sheet** — ⚠️ **the writer's FIRST instinct.** The hook already existed and was never called. |
| **I-0170** | A twice-cited source **named only ONE citing object** — ⚠️ **quietly under-reporting the writer's own graph**, every field present and populated. |

⚠️ **22 consecutive Issues across SP-118, SP-119 and SP-120 came from clicking. None from a suite.**
The suites are not wrong — **they assert what the code does, not that a writer can reach it or that what
she reads is true.**

---

## ⚠️ Carried out of SP-120

| Item | Owner |
| ---- | ----- |
| **AC11** — `[Linux]` parity | **SP-121** |
| **AC12** — full suite verification + Epic close prep | **SP-122** |
| ⚠️ **T-0459** — per-citation locators on the `cites` **EDGE**; ⚠️ **`scrivi_update_edge` DOES NOT EXIST** | **EP-032** (user-ruled) |
| ⚠️ **Thumbnail generation** — `thumbnailAssetID` read, never written | Unscheduled |
| **I-0147** — the 60 s post-crash lock window; ✅ Accepted | **Network-worlds design** |

---

## Next

⚠️ **No Sprint is planned.** SP-121 has not been defined — ⚠️ **Claude does not plan or activate one
without the user's word.** Its subject is **AC11**: `[Linux]` parity for AC1–AC9 in Docker+VNC,
⚠️ **with no gesture-only affordances.**

⚠️ **SP-107–SP-114 remain RESERVED to EP-032** and are not available.

**Next available:** Sprint **SP-121** · Task **T-0460** · Issue **I-0171** (⚠️ **opens a new decade file**
— I-0170 closed the current one).

---

*Last Updated: 2026-08-24 (**SP-120 ✅ CLOSED, user-approved.** Six Tasks + two Issues Verified and
archived in the same step. ✅ **AC8 closes**; ✅ **T-0365 COMPLETE after four sprints**; ✅ **EP-032
unblocked.** ⚠️ **T-0459 filed to EP-032** — SP-120's own S11 predicted the `page`-on-source limit and real
use hit it one sprint later. Active Sprints 1 → 0; EP-034 stays 🟡 Active, **6 of 8 sprints closed**.)*
