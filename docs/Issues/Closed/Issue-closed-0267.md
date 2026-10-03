# Closed Issue (Not Verified) — I-0267

## I-0267: `[Apple]` Every ordinary keystroke on dumas takes 60–95 ms

**Status:** ⚪ **CLOSED — Not Verified (2026-10-02, user-directed)**
**Reason for Closure:** ⛔ **ALREADY RULED — a DUPLICATE of [I-0206]'s accepted characteristic. It should never have been filed.**
**Date Identified:** 2026-10-01 · **Date Closed:** 2026-10-02

---

### ✅ THE RULING

✅ **User, 2026-10-02:** *"With respect to the slow keystrokes. I thought I ruled on that as not a problem earlier."*
✅ **They had — [I-0206], closed 2026-09-25:** *"If the setSel becomes a problem we will address it then."* ✅ I-0206
measured **~59 ms per keystroke** and **57 ms `setSel`** on the same 1.85 MB manuscript and closed them as
**NOT A DEFECT: no keystroke, latency or frame-time requirement exists in `docs/`** → [`Issue-closed-0206.md`](Issue-closed-0206.md).

### ⛔ WHY IT WAS FILED ANYWAY — the error, recorded

⚠️ I compared the 2026-10-01 console against [I-0213]'s *"1.7–8.6 ms for every ordinary keystroke"* and called
the difference a regression. ⛔ **I never searched `Closed/`**, where I-0206 had already measured and accepted the
higher figure. ⚠️ The two prior records disagree with each other (I-0213 vs I-0206) — ✅ the RULING is I-0206's.

### What the record contained (kept, not acted on)

| **I-0267** | `[Apple]` ⚠️ **EVERY ORDINARY KEYSTROKE ON dumas TAKES 60–95 ms — it was 1.7–8.6 ms on 2026-09-14.** ✅ **MEASURED in the user's console 2026-10-01** (`dumas-prose-timelines`, 1,179 scenes, 1.85 M chars): `[SCRIVI-EDIT] keyDown(…)=61.5–94.3 ms` across ~120 keystrokes, ⚠️ **while `didChangeText` stays 2.4–3.5 ms.** ✅ The [I-0213] record gives the baseline on the same project: *"`1.7–8.6 ms` for every ordinary keystroke."* ⚠️ Per the T-0531 instrumentation comment, **slow `keyDown` with fast delegate work puts the cost in AppKit's own edit/layout/display**, not in Scrivi's delegate. ⛔ **NOT ATTRIBUTED.** ⚠️ [T-0569] (navigator search) landed the same day and must be ruled IN or OUT by measurement, not by reading: its navigator does not observe `segments` and titles update on a debounce, ⛔ but that is a code read, and code reads have been wrong on this class three times ([I-0213]). ✅ **Next step: A/B the same typing on the build before T-0569.** ---- ✅ **2026-10-01 SECOND RUN — the slowness is NOT CONSTANT, and [T-0569] is RULED OUT as its cause.** ✅ Same project, a build that still CONTAINS T-0569's navigator code (plus T-0571): ⚠️ **arrow keys `keyDown(code=125/126)` = `0.5–5.0 ms`**, ⛔ **against `77.6` / `85.4 ms` for the SAME keys in the first run.** ✅ Code present in both runs and fast in one cannot be a per-keystroke constant. ⚠️ **WHAT DIFFERED BETWEEN THE RUNS (the lead, NOT a diagnosis):** ⛔ **the slow run opened `the-stairs-of-tintagael` FIRST (whose history open FAILED, [I-0268]), CLOSED that window, then opened dumas IN THE SAME PROCESS**; ✅ the fast run launched straight into dumas. ⚠️ Apple never calls `scrivi_close_project` ([I-0233]), so state from a closed window outliving it is plausible — ⛔ **unproven.** ⚠️ Caveat: the fast run typed only ONE character, so the comparison rests on ARROW keys, which were slow in the first run too. ✅ **Next step: reproduce the slow run's ORDER — launch, open another project, close its window, open dumas, then type and arrow.** | High | SP-152 (carried from SP-151 at its close, 2026-10-02) | 🔴 **Open — not attributed; T-0569 ruled out** |

✅ **Its one finding of lasting use:** [T-0569] (navigator search) was RULED OUT as a per-keystroke cost by the
second run (arrows 0.5–5.0 ms on a build containing it).
⚠️ **Removed from [SP-152]** (Planning), which now carries [I-0268] alone.
