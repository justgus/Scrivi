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

Currently: **2 Issues awaiting verification** — I-0131 and I-0132, both from the SP-102 live runs.

⚠️ **2026-08-17 was a large clear-down: 18 Issues verified and archived in one day.** I-0114–I-0117 (batch
12), I-0121/I-0122 and I-0123–I-0130 (batch 13), I-0076/I-0077 (batch 8), I-0086 (batch 9), and
I-0102/I-0105/I-0106 (batch 11) — plus **I-0017** out of the backlog (batch 2). Several had been sitting
`Resolved - Not Verified` for weeks; the user cleared them while reviewing the backlog.

| ID | Sprint |
| -- | ------ |
| I-0131 | SP-102 |
| I-0132 | unassigned |

See: [`Issue-active.md`](Issue-active.md)

---

## Backlog Issues (open, no Sprint)

Currently: **1**.

| ID | Title | Severity | Status | Epic |
| -- | ----- | -------- | ------ | ---- |
| I-0018 | **Manuscript does not scroll to the restored scene on app load** (rescoped) | Low | 🟠 Partly fixed | — |

⚠️ **I-0018 was rescoped 2026-08-17, not closed.** Its original complaint — no selection shown in the
Navigator on load — is **fixed**. What remains is a different behaviour: the manuscript is not scrolled to
that selection. Flagged to be scoped **with I-0131 and I-0132**, which ask the same underlying question —
*what does it mean to "be at" a scene?* — across load, click and quit.

✅ **I-0121 and I-0122 archived 2026-08-17, in the same step SP-106 closed** — both ✅ Verified
(user-approved) → [`Verified/Issue-verified-0121-0130.md`](Verified/Issue-verified-0121-0130.md), a new
batch 13. Their full entries left `Issue-backlog.md` at the same time. ⚠️ **This is the discipline the note
below says was missed twice** (I-0058, I-0112): archive in the step the Issue is verified, don't leave it in
the backlog.

⚠️ **I-0058 and I-0112 were archived out of this backlog on 2026-08-16** (to batches 6 and 12). Both had been
Verified — 2026-07-09 and 2026-08-11 — and left in the backlog for weeks. **A Verified Issue must be archived
in the same step it is verified**, from *whichever* file it lives in; the 2026-08-15 cleanup applied that to
`Issue-active.md` only.

See: [`Issue-backlog.md`](Issue-backlog.md)

---

## Verified Issues

Currently: **116 verified Issues**, archived in decade batches. Counts below are the entries **physically in
each file**, and they sum to 116. (**+4 on 2026-08-17** — I-0114–I-0117 → batch 12, verified live during the
SP-102 / T-0415 world runs.) (**+2 on 2026-08-17** — I-0121 and I-0122 → the new batch 13, archived **at
the SP-106 close** rather than late. **+2 on 2026-08-16** — I-0058 → batch 6 and I-0112 → batch 12, both
archived late out of `Issue-backlog.md`.)

⚠️ **Batches 2 and 3 are misfiled and the counts say so.** `Issue-verified-0011-0020.md` physically contains
**I-0021 – I-0024**, which belong in batch 3; batch 3 carries a pointer stub for them rather than the entries.
So batch 2 holds 10 entries for a range of 10 IDs that should hold 6, and batch 3 holds 6. Nothing is lost —
every Issue is filed exactly once and reachable — but **a decade file does not match its own filename**, and
the previous total of **94 double-counted those four** by adding batch 3's pointer stub to batch 2's real
entries. Left in place rather than re-cut: moving verified archive entries risks more than the tidiness buys.

| Batch | Issues | File | Count |
| ----- | ------ | ---- | ----- |
| 1 | I-0001 – I-0010 | [`Issue-verified-0001-0010.md`](Verified/Issue-verified-0001-0010.md) | 10 |
| 2 | I-0011 – I-0020 | [`Issue-verified-0011-0020.md`](Verified/Issue-verified-0011-0020.md) | 11 ⚠️ |
| 3 | I-0021 – I-0030 | [`Issue-verified-0021-0030.md`](Verified/Issue-verified-0021-0030.md) | 6 ⚠️ |
| 4 | I-0031 – I-0040 | [`Issue-verified-0031-0040.md`](Verified/Issue-verified-0031-0040.md) | 10 |
| 5 | I-0041 – I-0050 | [`Issue-verified-0041-0050.md`](Verified/Issue-verified-0041-0050.md) | 9 |
| 6 | I-0051 – I-0060 | [`Issue-verified-0051-0060.md`](Verified/Issue-verified-0051-0060.md) | 8 |
| 7 | I-0061 – I-0070 | [`Issue-verified-0061-0070.md`](Verified/Issue-verified-0061-0070.md) | 10 |
| 8 | I-0071 – I-0080 | [`Issue-verified-0071-0080.md`](Verified/Issue-verified-0071-0080.md) | 8 |
| 9 | I-0081 – I-0090 | [`Issue-verified-0081-0090.md`](Verified/Issue-verified-0081-0090.md) | 9 |
| 10 | I-0091 – I-0100 | [`Issue-verified-0091-0100.md`](Verified/Issue-verified-0091-0100.md) | 8 |
| 11 | I-0101 – I-0110 | [`Issue-verified-0101-0110.md`](Verified/Issue-verified-0101-0110.md) | 9 |
| 12 | I-0111 – I-0120 | [`Issue-verified-0111-0120.md`](Verified/Issue-verified-0111-0120.md) | 8 |
| 13 | I-0121 – I-0130 | [`Issue-verified-0121-0130.md`](Verified/Issue-verified-0121-0130.md) | 10 |

---

## Closed Issues (not verified)

| File | Issues |
| ---- | ------ |
| [`Issue-closed-0019.md`](Closed/Issue-closed-0019.md) | I-0019 |
| [`Issue-closed-0072-0103.md`](Closed/Issue-closed-0072-0103.md) | I-0072, I-0073, I-0085, I-0103 |

---

*Last Updated: 2026-08-17, end of day (**18 Issues verified and archived — the largest clear-down to date.**
The user reviewed the backlog and confirmed: **I-0017** fixed long ago and never filed (batch 2);
**I-0076/I-0077** (batch 8), **I-0086** (batch 9), **I-0102/I-0105/I-0106** (batch 11) — all long-standing
`Resolved - Not Verified` entries; and **I-0123–I-0129** (batch 13), the seven filed and fixed the same day
during the SP-102 live runs. With I-0114–I-0117 and I-0121/I-0122/I-0130 earlier, that is 18 in one day.
**Active is now 2** (I-0131, I-0132) and the **backlog is 1** (I-0018, rescoped).
⚠️ **Batch counts were RECOUNTED from the files rather than derived by arithmetic**, which is how the
batch-2/3 misfiling stayed visible: batch 3's first heading is the pointer stub *"I-0021 through I-0024"*, so
it holds **6 real entries + 1 stub** while those four live in batch 2 (11). Verified total **116**, not the
117 a naive heading count gives. Prior note follows.)*

*2026-08-17, later same day (**I-0114–I-0117 ✅ Verified (user-approved) and archived** to
batch 12 in the same step — verified live during the SP-102 / T-0415 world-availability runs, which exercised
those exact surfaces (object creation into a world, world status reporting, the sandbox grant surviving
relaunch, Manage Worlds repair). ✅ **They are now usable as evidence for SP-100's AC pass.** Verified
97 → **101**; batch 12 holds 8. Prior note follows.)*

*2026-08-17 (**SP-106 closed — I-0121 and I-0122 ✅ Verified (user-approved) and archived** to
the new **batch 13**, `Verified/Issue-verified-0121-0130.md`, with their full entries removed from
`Issue-backlog.md` in the same step. Counts: verified 95 → **97**; backlog 4 → **2** (I-0017 and I-0018, both
🔴 Open and unassigned). The 10 `Resolved - Not Verified` Issues in `Issue-active.md` are unchanged.
Prior note follows.)*

*2026-08-16 (**I-0121 opened** — `rebalancedKeys(1)` divides by zero, the cause of ScriviCore
CI failing on every commit since 2026-07-30; assigned to EP-031, to be scoped at the next sprint's planning.
A **Backlog Issues** section was added to this index, which previously listed only active and archived
Issues — the backlog was unrepresented, and ⚠️ **two Issues filed there (I-0058, I-0112) are Verified but
never archived.** Earlier the same day: consistency audit — verified total corrected **94 → 93**: batch 3's pointer stub
for I-0021–I-0024 was being counted alongside batch 2's real entries for the same four Issues. Per-batch
counts are now the entries physically present in each file, with the batch 2/3 misfiling flagged in place.
The active list is unchanged at **10 `Resolved - Not Verified`**, and that figure was confirmed against
`Issue-active.md` and the archives — ⚠️ **`Sprint-active.md` and `Epic-active.md` had both claimed
I-0114–I-0119 were verified; only I-0118 and I-0119 are.** Both were corrected. Prior note follows.)*

*2026-08-15 (docs cleanup — 48 verified Issues archived from `Issue-active.md` into decade
batches; 4 closed Issues archived; index rebuilt from the archive files.)*
