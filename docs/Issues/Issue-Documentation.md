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

Currently: **2 records** — ⚠️ **I-0171 OPEN**, plus **I-0147**, an accepted limitation rather than open
work. ✅ **I-0172 Verified + archived 2026-08-25.**

⚠️ **CORRECTED 2026-08-25 (SP-122 T-0471 Audit Check).** This section had gone **four sprints stale**: it
read *"2 Issues open"* and listed **I-0140/I-0141 as open in SP-116** — ⚠️ **both were fixed, Verified and
archived on 2026-08-21** (`Verified/Issue-verified-0131-0140.md`, `Verified/Issue-verified-0141-0150.md`)
when SP-116 closed. ⚠️ **The index was still describing them as pending work.**

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| **I-0171** | `[Build]` ⚠️ **Both `.dockerignore` files exclude only `build/` while FIVE build dirs exist** — container builds still poisoned by host `CMakeCache.txt`. ⚠️ **Reproduced live**; ⚠️ **SP-121's fix matched the one directory it had seen, not the pattern** | Medium | SP-122 |
| I-0147 | `[ScriviCore]` ⚠️ **Accepted limitation (user-ruled 2026-08-21)** — a world is unwritable for up to 60 s after an interrupted write | Low | ⚠️ Deferred — network-worlds design |

✅ **SP-100's five carried Issues (I-0135–I-0139) were all fixed and Verified by SP-115 on 2026-08-20**,
together with **I-0142**, which the user found *during* that verification. All six are archived.

⚠️ **I-0136 is Verified at the CORE ONLY** — nothing in Scrivi surfaces `unsupportedWorldFormatVersion`, so
a writer opening a too-new world still sees *"unavailable"* with no explanation. **The writer-facing half
does not exist and is owed to no one yet.**

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

Currently: **129 verified Issues**, archived in decade batches (122 before SP-115, **+6** on 2026-08-20,
**+1** on 2026-08-25 — **I-0172**, which opens the new `0171-0180` decade file).

⚠️ **129 is the prior provisional 128 plus one.** ⚠️ **It inherits that figure's uncertainty** — see the
note below; incrementing a provisional number does not make it confirmed.

> ⚠️ **This total is derived from the prior stated total plus this sprint's six — it was NOT confirmed by
> counting the archives.** A mechanical count is unreliable here because the archive files use **two
> different row formats** (older files use `## I-XXXX` headings, newer ones table rows) and because
> cross-references to other Issues inside a file inflate any naive grep. ⚠️ **Treat this number as
> provisional**; an Audit Check with a format-aware counter should confirm or correct it.

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

*Last Updated: 2026-08-20 (**SP-115 ✅ closed — six Issues Verified and archived**: I-0135–I-0139 →
`Verified/Issue-verified-0131-0140.md`, **I-0142 → the new `Issue-verified-0141-0150.md`**. Open Issues
**8 → 2** (I-0140, I-0141 → SP-116); verified **122 → 128**, counts re-derived by counting. ⚠️ **I-0136 is
core-only Verified — its surface is owed.** Next available Issue: **I-0143**. Prior note follows.)*

*Last Updated: 2026-08-20 (**I-0142 filed + fixed — found by the USER during SP-115 verification**, not by
a suite. The object editor never showed an object's own world because `worldID` was gated on `pending`
across **three** layers; ⚠️ **the unseen half was worse — renaming any world-scoped object failed.**
✅ Ruled same day: **a world is a property of the object**, and moving objects between worlds is
**disallowed** — the control is now a label. Next available Issue: **I-0143**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 implemented: I-0135–I-0139 all 🟢 Resolved - Not Verified**;
⚠️ **I-0140 and I-0141 FILED by T-0424 — filed, NOT fixed** (both cured by design-doc **D5**'s kind-scope
endpoint in **SP-116**). ⚠️ **I-0141 is occurrence EIGHT of the restated-kind-list class**, and I-0140 shows
the cause is **structural** — the ABI exposes no kind scope, so Swift has nothing to derive from. Next
available Issue: **I-0142**. Prior note follows.)*

*Last Updated: 2026-08-19 (audit remediation R-01…R-25; then **five Issues filed by SP-100** — I-0135/I-0136 by T-0390 and **I-0137/I-0138/I-0139 by T-0418's live pass**. ⚠️ **I-0137 is High**: AC24's refinement cannot fire on real hardware, which bears on an AC already marked Verified. Prior note: I-0135 + I-0136 by T-0390 — both found by writing repair-matrix §6a against shipped behaviour, both **filed not fixed** per SP-100 ruling R3.)*
