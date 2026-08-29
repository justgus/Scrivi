# I-0174 — CLOSED, not a defect

**Title:** `[Linux]` / `[ScriviCore]` "Opening a project writes to it"
**Severity:** Low (lowered from Medium) · **Sprint:** SP-125 (EP-035)
**Closed:** 2026-08-28 — **user ruling: *"opening a project does not write to it… it is not a risk."***

---

## What was reported

I fingerprinted the real rig drive (274 files) before and after a **single open** of
`the-stairs-of-tintagael.scrivi` in the Linux app, and two files changed with no user edit:

- `objects/index.json` — gained `"generation": 2`
- `worlds/<worldID>/binding.json` — `cachedIndex` gained three characters
  (*Pet*, *Myton at 23*, *Administrator Rusk*)

⚠️ **I presented the three names as an unexplained side effect of opening.**

## ⚠️ Why that was WRONG

✅ **User correction 2026-08-28:** those three characters were **authored in a SECOND project**,
`the-lone-golem.scrivi`, which is bound to the **SAME Eskandar world**
(`world_character_01a000fb-539a-7402-802e-0d97eeb1e594`).

**Verified:** Lone Golem's own binding caches all **35** world entries, including those three;
Tintagael's cached **34** and was simply **stale**, predating that work.

✅ **So the reconcile is not a mystery write — it is exactly how a SHARED WORLD propagates between
projects.** ⚠️ **Suppressing it would stop a project ever seeing objects another project added to a
world they share**, which is the whole point of a shared world.

## Disposition

**CLOSED as not a defect.** No prose is ever touched; the changes are additive and are the system
working as designed.

⚠️ **Do NOT "fix" this by suppressing the cache refresh.**

## ⚠️ The lesson worth keeping

⚠️ **A surprising write into a SHARED resource should send you looking for the OTHER SHARER before it is
called a defect.** ⚠️ **I had the evidence to find this myself** — the second project was on the same
machine, bound to the same world — **and instead filed an Issue against behaviour that was correct.**

⚠️ **Secondary caution, still true and still not a defect:** an open is not a byte-for-byte no-op, so a
session that must prove it changed nothing should fingerprint before/after rather than assume.

---

*Closed 2026-08-28 by user ruling, in the same step I-0173 and I-0175 were verified and archived.*
