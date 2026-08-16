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

Currently: **10 Issues awaiting verification** — all `Resolved - Not Verified`.

| ID | Sprint |
| -- | ------ |
| I-0076 | EP-027 |
| I-0077 | EP-027 |
| I-0086 | SP-075 |
| I-0102 | SP-092 |
| I-0105 | SP-093 |
| I-0106 | SP-093 |
| I-0114 | SP-104 |
| I-0115 | SP-104 |
| I-0116 | SP-104 |
| I-0117 | SP-104 |

See: [`Issue-active.md`](Issue-active.md)

---

## Verified Issues

Currently: **93 verified Issues**, archived in decade batches. Counts below are the entries **physically in
each file**, and they sum to 93.

⚠️ **Batches 2 and 3 are misfiled and the counts say so.** `Issue-verified-0011-0020.md` physically contains
**I-0021 – I-0024**, which belong in batch 3; batch 3 carries a pointer stub for them rather than the entries.
So batch 2 holds 10 entries for a range of 10 IDs that should hold 6, and batch 3 holds 6. Nothing is lost —
every Issue is filed exactly once and reachable — but **a decade file does not match its own filename**, and
the previous total of **94 double-counted those four** by adding batch 3's pointer stub to batch 2's real
entries. Left in place rather than re-cut: moving verified archive entries risks more than the tidiness buys.

| Batch | Issues | File | Count |
| ----- | ------ | ---- | ----- |
| 1 | I-0001 – I-0010 | [`Issue-verified-0001-0010.md`](Verified/Issue-verified-0001-0010.md) | 10 |
| 2 | I-0011 – I-0020 | [`Issue-verified-0011-0020.md`](Verified/Issue-verified-0011-0020.md) | 10 ⚠️ |
| 3 | I-0021 – I-0030 | [`Issue-verified-0021-0030.md`](Verified/Issue-verified-0021-0030.md) | 6 ⚠️ |
| 4 | I-0031 – I-0040 | [`Issue-verified-0031-0040.md`](Verified/Issue-verified-0031-0040.md) | 10 |
| 5 | I-0041 – I-0050 | [`Issue-verified-0041-0050.md`](Verified/Issue-verified-0041-0050.md) | 9 |
| 6 | I-0051 – I-0060 | [`Issue-verified-0051-0060.md`](Verified/Issue-verified-0051-0060.md) | 7 |
| 7 | I-0061 – I-0070 | [`Issue-verified-0061-0070.md`](Verified/Issue-verified-0061-0070.md) | 10 |
| 8 | I-0071 – I-0080 | [`Issue-verified-0071-0080.md`](Verified/Issue-verified-0071-0080.md) | 6 |
| 9 | I-0081 – I-0090 | [`Issue-verified-0081-0090.md`](Verified/Issue-verified-0081-0090.md) | 8 |
| 10 | I-0091 – I-0100 | [`Issue-verified-0091-0100.md`](Verified/Issue-verified-0091-0100.md) | 8 |
| 11 | I-0101 – I-0110 | [`Issue-verified-0101-0110.md`](Verified/Issue-verified-0101-0110.md) | 6 |
| 12 | I-0111 – I-0120 | [`Issue-verified-0111-0120.md`](Verified/Issue-verified-0111-0120.md) | 3 |

---

## Closed Issues (not verified)

| File | Issues |
| ---- | ------ |
| [`Issue-closed-0019.md`](Closed/Issue-closed-0019.md) | I-0019 |
| [`Issue-closed-0072-0103.md`](Closed/Issue-closed-0072-0103.md) | I-0072, I-0073, I-0085, I-0103 |

---

*Last Updated: 2026-08-16 (consistency audit — verified total corrected **94 → 93**: batch 3's pointer stub
for I-0021–I-0024 was being counted alongside batch 2's real entries for the same four Issues. Per-batch
counts are now the entries physically present in each file, with the batch 2/3 misfiling flagged in place.
The active list is unchanged at **10 `Resolved - Not Verified`**, and that figure was confirmed against
`Issue-active.md` and the archives — ⚠️ **`Sprint-active.md` and `Epic-active.md` had both claimed
I-0114–I-0119 were verified; only I-0118 and I-0119 are.** Both were corrected. Prior note follows.)*

*2026-08-15 (docs cleanup — 48 verified Issues archived from `Issue-active.md` into decade
batches; 4 closed Issues archived; index rebuilt from the archive files.)*
