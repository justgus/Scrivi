# Issues (I) — Index

The main index for all Scrivi Issues. Issues track bugs and unintended system behavior.

> For planned improvements and new features, see [Tasks (T)](../Tasks/Task-Documentation.md).

## Organization

| File | Holds |
| ---- | ----- |
| [`Issue-active.md`](Issue-active.md) | Issues awaiting **user verification** |
| [`Issue-backlog.md`](Issue-backlog.md) | Unresolved Issues not assigned to a Sprint |
| [`Verified/`](Verified/) | Resolved **and user-verified**, archived in batches of ten |
| [`Closed/`](Closed/) | Closed without verification (superseded, not-a-bug, user-directed) |

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

---

## Active Issues

Currently: **5 Issues open**, all filed 2026-08-19 by **SP-100** and all awaiting **triage**, not
verification.

| ID | Title | Severity | Found by |
| -- | ----- | -------- | -------- |
| I-0135 | A corrupt/unparseable `world.json` has no test coverage | Low | T-0390 |
| I-0136 | ⚠️ `world.json`'s `formatVersion` is read but **never compared** — a newer package parses as current | Medium | T-0390 |
| **I-0137** | ⚠️ **AC24's `unmounted`/`offline` refinement can NEVER FIRE** — `packagePath` is empty for exactly the worlds it must diagnose | **High** | T-0418 |
| I-0138 | "Remove from scene" disabled for a pending object but **not explained** | Low | T-0418 |
| I-0139 | Clicking an object title opens the editor with no evident way back to viewing | Medium | T-0418 |

⚠️ **Next available Issue ID: I-0140.**

See: [`Issue-active.md`](Issue-active.md)

---

## Backlog Issues (open, no Sprint)

Currently: **0.** The Issue backlog is empty.

✅ **I-0018 was the last entry**, archived 2026-08-19 as ✅ Verified (audit ruling **R-02**) → batch 2.
Its rescoped behaviour was delivered by I-0131's restore centring, verified 2026-08-18.
⚠️ **Recorded with it: I-0018 should never have been rescoped** — a retargeted ID destroys the record of
the defect it originally named (**P2**).

⚠️ **A Verified Issue is archived in the SAME STEP it is verified**, from whichever file it lives in —
`Issue-active.md` **or** `Issue-backlog.md`. Deferring it is how I-0018 (F-02) and I-0118 (F-03) went
unarchived, one of them with no record written at all.

See: [`Issue-backlog.md`](Issue-backlog.md)

---

## Verified Issues

Currently: **122 verified Issues**, archived in decade batches.

⚠️ **Counts below are DERIVED, never adjusted (P6).** Each is `grep -c '^## I-0'` on the file itself, run
2026-08-19 after all archiving completed. **A batch file's table rows must equal its entry count (P4)** —
and **a new batch file gets its index row in the same edit that creates it.** Batch 14 was created
2026-08-18 and went un-indexed for a day; that is what this rule prevents.

⚠️ **A decade file is named for its ID RANGE, not its contents.** Files are created before their range
fills, so **the last ID of a range is frequently never assigned** (I-0050, I-0060, I-0100, I-0120 were
never used). **This is not a filing defect** — do not re-open it as one.

| Batch | Issues | File | Count |
| ----- | ------ | ---- | ----- |
| 1 | I-0001 – I-0010 | [`Issue-verified-0001-0010.md`](Verified/Issue-verified-0001-0010.md) | 10 |
| 2 | I-0011 – I-0020 | [`Issue-verified-0011-0020.md`](Verified/Issue-verified-0011-0020.md) | 12 ⚠️ |
| 3 | I-0021 – I-0030 | [`Issue-verified-0021-0030.md`](Verified/Issue-verified-0021-0030.md) | 7 ⚠️ |
| 4 | I-0031 – I-0040 | [`Issue-verified-0031-0040.md`](Verified/Issue-verified-0031-0040.md) | 10 |
| 5 | I-0041 – I-0050 | [`Issue-verified-0041-0050.md`](Verified/Issue-verified-0041-0050.md) | 9 |
| 6 | I-0051 – I-0060 | [`Issue-verified-0051-0060.md`](Verified/Issue-verified-0051-0060.md) | 8 |
| 7 | I-0061 – I-0070 | [`Issue-verified-0061-0070.md`](Verified/Issue-verified-0061-0070.md) | 10 |
| 8 | I-0071 – I-0080 | [`Issue-verified-0071-0080.md`](Verified/Issue-verified-0071-0080.md) | 8 |
| 9 | I-0081 – I-0090 | [`Issue-verified-0081-0090.md`](Verified/Issue-verified-0081-0090.md) | 9 |
| 10 | I-0091 – I-0100 | [`Issue-verified-0091-0100.md`](Verified/Issue-verified-0091-0100.md) | 8 |
| 11 | I-0101 – I-0110 | [`Issue-verified-0101-0110.md`](Verified/Issue-verified-0101-0110.md) | 9 |
| 12 | I-0111 – I-0120 | [`Issue-verified-0111-0120.md`](Verified/Issue-verified-0111-0120.md) | 9 |
| 13 | I-0121 – I-0130 | [`Issue-verified-0121-0130.md`](Verified/Issue-verified-0121-0130.md) | 10 |
| **14** | I-0131 – I-0140 | [`Issue-verified-0131-0140.md`](Verified/Issue-verified-0131-0140.md) | **3** |

**⚠️ Batches 2 and 3 — a known, deliberate irregularity. Do not "fix" it.**
`Issue-verified-0011-0020.md` physically contains **I-0021 – I-0024**, which belong to batch 3; batch 3
carries a **pointer stub** for them instead of the entries. Nothing is lost — every Issue is filed exactly
once and reachable.

> ⚠️ **Batch 3's count of 7 includes that pointer stub, which is a heading but NOT an Issue entry** (6 real
> entries + 1 stub). It is counted as a heading so the **P4 check stays purely mechanical** — a check with
> a hand-maintained exception is not a check.

**Left in place rather than re-cut** (ruled 2026-08-16, reaffirmed 2026-08-19): *moving verified archive
entries risks more than the tidiness buys.*

### ID accounting

⚠️ **Every ID from I-0001 to the highest issued is accounted for.** Recorded so that an unassigned ID and a
lost record can be told apart — the Issue layer previously had no such line, which made them
indistinguishable.

| ID | Disposition | Evidence |
| -- | ----------- | -------- |
| **I-0059** | ⚪ **Never assigned** | Appears in git only as *"Next available: I-0059"* and in *"pending I-0059/I-0060"* — never attached to a defect |
| **I-0099** | ⚪ **Never assigned** | **Zero commits** touch it in any form |
| **I-0016** | ⚪ **Superseded → I-0018** | `8e64bfe` (SP-033), 2026-06-08: *"I-0016 \| Navigator selection on load \| ⚪ Superseded by I-0018"* |
| I-0050, I-0060, I-0100, I-0120 | ⚪ **Never assigned** | End-of-range IDs; appear only in filenames and range labels |

---

## Closed Issues (not verified)

| File | Issues |
| ---- | ------ |
| [`Issue-closed-0019.md`](Closed/Issue-closed-0019.md) | I-0019 |
| [`Issue-closed-0072-0103.md`](Closed/Issue-closed-0072-0103.md) | I-0072, I-0073, I-0085, I-0103 |
| [`Issue-closed-0134.md`](Closed/Issue-closed-0134.md) | I-0134 — ⚠️ **non-issue** (erroneous parity premise; Apple authoritative) |

---

*Last Updated: 2026-08-19 (audit remediation R-01…R-25; then **five Issues filed by SP-100** — I-0135/I-0136 by T-0390 and **I-0137/I-0138/I-0139 by T-0418's live pass**. ⚠️ **I-0137 is High**: AC24's refinement cannot fire on real hardware, which bears on an AC already marked Verified. Prior note: I-0135 + I-0136 by T-0390 — both found by writing repair-matrix §6a against shipped behaviour, both **filed not fixed** per SP-100 ruling R3.)*
