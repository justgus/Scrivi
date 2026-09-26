# Closed Issue (Not Verified) — I-0206

## I-0206: `[Apple]` Manuscript-surface keystroke and `setSelectedRange` costs on a 1.85 MB document

**Status:** ⚪ **CLOSED — Not Verified (2026-09-25, user-approved)**
**Reason for Closure:** ⚠️ **NOT A DEFECT — no performance requirement exists for it to violate.**
**Platform:** macOS
**Component:** `ManuscriptTextView.swift` (AppKit `NSTextView`, TextKit 2)
**Severity at closure:** Medium (⛔ **never re-reasoned against a requirement — see below**)
**Sprint at closure:** was carried under [EP-040]; ✅ **[EP-040] AC11 MET and the Epic CLOSED 2026-09-24**
**Date Identified:** 2026-09-14 · **Date Closed:** 2026-09-25

---

### ✅ THE RULING (user, 2026-09-25)

⚠️ ***"If the setSel becomes a problem we will address it then."***

✅ **CLOSED as an accepted, measured characteristic of the surface — ⛔ not as work completed, and
⛔ not as a limitation.** ⚠️ **Both of those framings were wrong and are corrected here.**

---

### ⛔ WHY IT WAS CLOSED: THERE IS NO REQUIREMENT

⚠️ **`docs/` WAS GREPPED 2026-09-25 FOR ANY KEYSTROKE, TYPING, LATENCY OR FRAME-TIME REQUIREMENT.**
⛔ **THERE IS NONE — no threshold, no budget, no target, in any design document.**

✅ **THAT IS DECISIVE.** ⚠️ **"Limitation" is only meaningful against a requirement something falls
short of.** ⛔ **With no requirement, `~59 ms` per keystroke and `57 ms` `setSel` are MEASUREMENTS,
not failures** — ✅ **and the user's own live passes repeatedly called the surface usable.**

⚠️ **[EP-040] AC11's own wording allowed this all along:** *"addressed **OR** accepted as limitations
WITH a measurement."* ⛔ **AC11 never required the cost to be a defect** — ⚠️ **that word was carried
forward from this record's original framing and never tested.**

---

### ⚠️ WHY IT WAS FILED AT ALL — THE FILING ITSELF WAS THE ERROR

✅ **THIS RECORD WAS DISCOVERED BY SUBTRACTION, NOT BY A FAILED EXPECTATION.** ⚠️ **[I-0204]'s 33 s
`buildClusters` stall MASKED these costs; when that stall was removed, they became VISIBLE and were
filed the same day.** ⛔ **VISIBILITY IS NOT A DEFECT.** ✅ **Nothing was ever measured against a bar,
because no bar existed.**

⚠️ **CONTRAST [I-0213], WHICH WAS CORRECTLY A DEFECT AND WAS FIXED:** ✅ **a `2.7 s` FREEZE on chapter
create is visibly broken against any reasonable expectation.** ⚠️ **This record's numbers never were,
which is exactly why no fix was ever written for it across three Epics.**

---

### ✅ THE MEASUREMENTS, PRESERVED (they are the value of this record)

⚠️ **Measured 2026-09-14 on `dumas-prose`, `tvLen=1823873` in ONE `NSTextView`, TextKit 2 confirmed at
construction AND at every `rebuildStorage`.**

| Measurement | Figure |
| ----------- | ------ |
| ⚠️ **`keyDown` — typing** | **`57.8–88.6 ms`**, ✅ **REMARKABLY CONSTANT across ~70 keystrokes** (⚠️ caps sustained typing ~17 keys/sec) |
| ✅ our own work (`[SCRIVI-KEY] total`) | `0.5–1.3 ms` |
| ✅ our own work (`didChangeText`) | `0.6–1.4 ms` |
| ⚠️ **`setSel` @ offset 123,465** | `5.4 ms` |
| ⚠️ **`setSel` @ offset 513,559** | `16.8 ms` |
| ⚠️ **`setSel` @ offset 1,816,059** | **`57.1 ms`** |
| ✅ `center` (all three offsets) | ~`92 ms` FLAT — ⚠️ **so the scaling is `setSelectedRange` specifically, NOT the centring** |

⚠️ **TWO DISTINCT SHAPES WERE BUNDLED IN ONE RECORD, AND THEY ARE NOT THE SAME THING:**
1. ✅ **TYPING (`~59 ms`) IS CONSTANT** — ⛔ **no scaling pathology.** ⚠️ **Most likely simply what
   AppKit costs on a 1.85 MB `NSTextView`.**
2. ⚠️ **`setSel` IS LINEAR IN DOCUMENT OFFSET** (~10× offset ≈ ~10× cost) — ✅ **that IS an algorithmic
   shape, and it is the only part here that would worsen as manuscripts grow.**

---

### ⚠️ THE RE-OPEN CONDITION — ✅ **THIS IS THE OPERATIVE CLAUSE**

✅ **RE-OPEN IF `setSelectedRange`'s OFFSET-LINEAR COST BECOMES A PROBLEM IN REAL USE** — ⚠️ **the
user's ruling names this explicitly: *"If the setSel becomes a problem we will address it then."***

⚠️ **IF IT IS RE-OPENED, TWO THINGS FROM THIS RECORD MUST BE HONOURED:**
- ⛔ **A TEST MUST EXERCISE THE END OF THE DOCUMENT.** ✅ **This Issue was once wrongly marked RESOLVED
  from a SINGLE sample at offset ~19,000 — ✅ about 1% into a 1.85 MB document — where `setSel` read
  `0.8–1.2 ms`.** ⚠️ **An offset-dependent cost verified at a low offset proves nothing.**
- ✅ **STATE THE REQUIREMENT FIRST.** ⚠️ **The first piece of work is deciding what the bar IS** —
  ⛔ **nothing in the project can currently distinguish acceptable from unacceptable performance.**

---

### ⛔ A CORRECTION THIS RECORD CARRIED FOR ELEVEN DAYS

⚠️ **THE ROW STATED: *"it has NOT been sampled … this one must be `sample`d during sustained typing
before any fix is designed."*** ⛔ **THAT WAS STALE AND WRONG.**
✅ **`sample Scrivi` WAS run repeatedly** — ⚠️ **[T-0532] attributed **4,796 / 6,595** main-thread
samples to `TimelineStripView.buildClusters`; [T-0533] measured **~860** JSON-parse samples per
activation ([SP-133]'s closed record, `:19`, `:21`, `:96`, `:98`).**
⚠️ **The caveat was written 2026-09-14 and NEVER UPDATED as sampling actually happened** — ✅ **and it
was relayed as fact on 2026-09-25 before the user corrected it.**
⚠️ **THE LESSON: a dated caveat in a record is a CLAIM TO RE-CHECK, not a fact to repeat.**

---

### ⚠️ WHAT THIS RECORD IS NOT

- ⛔ **NOT [I-0213]** (⚠️ *chapter create, a `2.7 s` freeze → `~316 ms`*) — ✅ **that was a ONE-OFF on a
  STRUCTURAL op, was a real defect, was fixed by [SP-150] (T-0549 + T-0550), and is VERIFIED
  2026-09-24.** ⛔ **Do not conflate a per-keystroke cost with a structural-op cost.**
- ⛔ **NOT [I-0204]** (⚠️ *the 33 s `buildClusters` stall*) — ✅ **that was the mask, and it is VERIFIED.**
