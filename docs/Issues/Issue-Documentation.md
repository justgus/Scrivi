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

✅ **The rows live in [`Issue-backlog.md`](Issue-backlog.md)** — [I-0202], [I-0147], [I-0275] (filed 2026-10-04), [I-0277] (filed 2026-10-05). ([I-0281] fixed and verified in [SP-169], archived.) ([I-0278] fixed and verified in [SP-167], archived.) ⛔ This line read *"Currently: 0 — the backlog is empty"* while two Issues sat there; corrected 2026-10-04.

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
| 15 | I-0141 – I-0150 | [`Issue-verified-0141-0150.md`](Verified/Issue-verified-0141-0150.md) | 7 |
| 16 | I-0151 – I-0160 | [`Issue-verified-0151-0160.md`](Verified/Issue-verified-0151-0160.md) | 11 |
| 17 | I-0161 – I-0170 | [`Issue-verified-0161-0170.md`](Verified/Issue-verified-0161-0170.md) | 9 |
| 18 | I-0171 – I-0180 | [`Issue-verified-0171-0180.md`](Verified/Issue-verified-0171-0180.md) | 8 |
| 19 | I-0181 – I-0190 | [`Issue-verified-0181-0190.md`](Verified/Issue-verified-0181-0190.md) | 6 |
| 20 | I-0191 – I-0200 | [`Issue-verified-0191-0200.md`](Verified/Issue-verified-0191-0200.md) | 9 |
| 21 | I-0201 – I-0210 | [`Issue-verified-0201-0210.md`](Verified/Issue-verified-0201-0210.md) | 7 |
| 22 | I-0211 – I-0220 | [`Issue-verified-0211-0220.md`](Verified/Issue-verified-0211-0220.md) | 8 |
| 23 | I-0221 – I-0230 | [`Issue-verified-0221-0230.md`](Verified/Issue-verified-0221-0230.md) | 2 |
| 24 | I-0231 – I-0240 | [`Issue-verified-0231-0240.md`](Verified/Issue-verified-0231-0240.md) | 5 |
| 25 | I-0241 – I-0250 | [`Issue-verified-0241-0250.md`](Verified/Issue-verified-0241-0250.md) | 8 |
| 26 | I-0251 – I-0260 | [`Issue-verified-0251-0260.md`](Verified/Issue-verified-0251-0260.md) | 10 |
| 27 | I-0261 – I-0270 | [`Issue-verified-0261-0270.md`](Verified/Issue-verified-0261-0270.md) | 9 |
| 28 | I-0271 – I-0280 | [`Issue-verified-0271-0280.md`](Verified/Issue-verified-0271-0280.md) | 6 |
| 29 | I-0281 – I-0290 | [`Issue-verified-0281-0290.md`](Verified/Issue-verified-0281-0290.md) | 2 |

⚠️ **Rows 15 onward added 2026-09-30 ([SP-148] Audit Check A6)** — the table had stopped at batch 14. ✅ Each count is re-derived from the file (distinct IDs in its table rows ∪ `## I-0` sections), ⛔ never copied.

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
| **I-0181** | ⚪ **Never assigned** — ✅ **added 2026-09-15, audit ruling [R-04]** | ⚠️ **NOT in `Issue-verified-0181-0190.md` despite that filename**, ⛔ **and in no other file.** ⚠️ **NOT reconstructed: the user ruled the information is not in git** |
| **I-0187** | ⚪ **Never assigned** — ✅ **[R-04]** | ⚠️ **CITED AS REAL by `Closed/Sprint-SP-127.md:81`** (*"Issue **I-0187**"*), ⚠️ **but no Issue record exists anywhere.** ✅ **The citation is preserved here so a reader finds the reference rather than concluding the sprint cited nothing** |
| **I-0188, I-0189** | ⚪ **Never assigned** — ✅ **[R-04]** | ⛔ **Appear NOWHERE in `docs/` in any form** — ⚠️ **not even as a range label** |
| **I-0190** | ⚪ **Never assigned** — ✅ **[R-04]** | ⚠️ **Referenced as a decade boundary by `Issue-verified-0191-0200.md:3`** (*"The previous decade closed at **I-0190**"*), ⚠️ **but no record exists** |

⚠️ **ON `Issue-verified-0181-0190.md`: ITS NAME DOES NOT MATCH ITS CONTENTS, AND THAT IS DELIBERATE.**
✅ **The file holds only I-0182–I-0186.** ⛔ **Audit ruling [R-04] PROHIBITS renaming it:** ⚠️ **a
filename that admits the gap SURFACES the loss; one that matches its contents HIDES it.** ✅ **The name
stands as a marker.**

---

## Closed Issues (not verified)

| File | Issues |
| ---- | ------ |
| [`Issue-closed-0019.md`](Closed/Issue-closed-0019.md) | I-0019 |
| [`Issue-closed-0072-0103.md`](Closed/Issue-closed-0072-0103.md) | I-0072, I-0073, I-0085, I-0103 |
| [`Issue-closed-0134.md`](Closed/Issue-closed-0134.md) | I-0134 — ⚠️ **non-issue** (erroneous parity premise; Apple authoritative) |
| [`Issue-closed-0174.md`](Closed/Issue-closed-0174.md) | I-0174 — ⚠️ **not a defect** ("opening a project writes to it" — the write is a shared world propagating another project's objects; closed 2026-08-28) |
| [`Issue-closed-0206.md`](Closed/Issue-closed-0206.md) | I-0206 — ⚠️ **not a defect** (keystroke / `setSel` cost on a 1.85 MB manuscript; no latency requirement exists; closed 2026-09-25) |
| [`Issue-closed-0180.md`](Closed/Issue-closed-0180.md) | I-0180 — ⚪ **not a defect** (per-row label is the scene's projection; Linux reverted in T-0576; closed 2026-10-03) |
| [`Issue-closed-0201.md`](Closed/Issue-closed-0201.md) | I-0201 — ⚪ **OBE** (`[Apple]` launch arguments no longer used; closed 2026-10-03) |
| [`Issue-closed-0267.md`](Closed/Issue-closed-0267.md) | I-0267 — ⚠️ **duplicate of [I-0206]'s ruling** (keystroke cost accepted 2026-09-25; filed 2026-10-01 without checking `Closed/`) |
| [`Issue-closed-0274.md`](Closed/Issue-closed-0274.md) | I-0274 — ⚪ **not a defect** (Control-Return mis-measured by a direct-`keyDown` harness; in the app it opens the context menu; closed 2026-10-04) |

---

*Last Updated: 2026-10-08 (**I-0281 archived** → `Issue-verified-0281-0290.md` (2), with the SP-169 close.)*

*Last Updated: 2026-10-08 (**I-0281 ✅ VERIFIED** (SP-169 live pass) — archive with the SP-169 close.)*

*Last Updated: 2026-10-07 (**I-0281 → active** with SP-169 (EP-047 S3).)*

*Last Updated: 2026-10-07 (**I-0282 archived** → new batch `Issue-verified-0281-0290.md` (1), with the SP-168 close.)*

*Last Updated: 2026-10-07 (**I-0282 ✅ VERIFIED** (user, live re-check 4) — archive with the SP-168 close.)*

*Last Updated: 2026-10-07 (**I-0282 filed + Resolved - Not Verified** (SP-168 live pass) — a typeface or size change lost the writer's place; Medium.)*

*Last Updated: 2026-10-07 (**I-0278 archived** → `Issue-verified-0271-0280.md` (6), with the SP-167 close.)*

*Last Updated: 2026-10-07 (**I-0278 ✅ VERIFIED** (SP-167 live pass, user) — archive with the SP-167 close.)*

*Last Updated: 2026-10-07 (**I-0278 → active** with SP-167 (EP-047 S1).)*

*Last Updated: 2026-10-07 (**I-0280 filed + Resolved - Not Verified** (SP-165) — a fenced code block recursed forever in the block analyzer; High. **I-0281 filed** to the backlog — Apple emphasis positions shift after an indented first line; Medium.)*

*Last Updated: 2026-10-06 (**I-0279 archived** → `Issue-verified-0271-0280.md` (5, re-counted), with the SP-163 close.)*

*Last Updated: 2026-10-06 (**I-0279 ✅ Verified** (live re-check) — archived with the SP-163 close.)*

*Last Updated: 2026-10-05 (**I-0279 filed + Resolved - Not Verified** (SP-163) — after one cross-scene copy every ⌘V pasted it; High.)*

*Last Updated: 2026-10-05 (**I-0278 filed** to the backlog — three Project Settings stored in `UserDefaults` do not travel with the project; Medium.)*

*Last Updated: 2026-10-05 (**I-0277 filed** to the backlog — VoiceOver reads the stored manuscript (escapes, hidden `##`); Medium; found in SP-161.)*

*Last Updated: 2026-10-04 (**I-0276 ✅ VERIFIED** on the rig (build 57) → `Issue-verified-0271-0280.md` (4).)*

*Last Updated: 2026-10-04 (**I-0276 filed + Resolved - Not Verified** — Linux launch window opened a project on a single click; now select/double-click. SP-160.)*

*Last Updated: 2026-10-04 (**I-0255 ✅ VERIFIED** — Linux half passed on the rig, build 54 → `Issue-verified-0251-0260.md` (10).)*

*Last Updated: 2026-10-04 (Audit Check F-5: `Issue-backlog.md`'s restated count line removed — R-15.)*

*Last Updated: 2026-10-04 (**I-0275**: sync-vs-async background added at the user's request.)*

*Last Updated: 2026-10-04 (**I-0275 filed** to the backlog — keystroke cost grows with manuscript position (AppKit); user: "kind of concerned", not ruled as biting.)*

*Last Updated: 2026-10-04 (**I-0274 CLOSED — not a defect** (user-directed) → `Closed/Issue-closed-0274.md`; removed from the backlog.)*

*Last Updated: 2026-10-04 (**I-0274 filed** to the backlog — Control-Return writes U+2028; found by SP-156's key measurement. Backlog section corrected: it claimed empty while I-0202 and I-0147 were there.)*

*Last Updated: 2026-10-03, close of day (**I-0255** — Apple half fully VERIFIED (timeline + Scene Navigator); Linux half awaits the rig.)*

*Last Updated: 2026-10-03, end of day (**I-0255** — Apple Navigator + Linux timeline/navigator implemented, not verified; Linux awaits the rig.)*

*Last Updated: 2026-10-03, latest (**I-0254 ✅ VERIFIED** → `Issue-verified-0251-0260.md` (9); **I-0255** Apple timeline half verified, Scene Navigator persistence ADDED to its scope.)*

*Last Updated: 2026-10-03, later (**I-0180 CLOSED — not a defect** → `Closed/Issue-closed-0180.md`; Linux reverted in T-0576. **I-0254** Resolved - Not Verified. **I-0255** retagged `[Cross]`; Apple half Resolved - Not Verified, Linux half open.)*

*Last Updated: 2026-10-03 (**User verifications archived** — I-0191, I-0194, I-0200 → `Issue-verified-0191-0200.md`; I-0208, I-0209, I-0210 → `Issue-verified-0201-0210.md`; I-0211, I-0212 → `Issue-verified-0211-0220.md`; I-0253 → `Issue-verified-0251-0260.md`; I-0268 → `Issue-verified-0261-0270.md` (counts re-derived). ⚠️ I-0200, I-0208, I-0211 had been user-verified 2026-09-14 and never archived. **I-0201 CLOSED — OBE** → `Closed/Issue-closed-0201.md`. **I-0147 → backlog** (accepted limitation).)*

*Last Updated: 2026-10-02 (closed-issues index: rows added for **I-0174** and **I-0206**, which existed in `Closed/` without rows — user-directed.)*

*Last Updated: 2026-10-02 (**I-0267 CLOSED — duplicate of I-0206's 2026-09-25 ruling** → `Closed/Issue-closed-0267.md`; removed from SP-152.)*

*Last Updated: 2026-10-02 (**SP-151 closed** — I-0266, I-0269, I-0270 → `Issue-verified-0261-0270.md` (8, re-counted); I-0271–I-0273 → new `Issue-verified-0271-0280.md` (3, re-counted); I-0202 → backlog (AC2 closed without a resolution); I-0267 + I-0268 → SP-152. ⚠️ I-0266–I-0273 had been issued without `next-id.py`; registry advanced at close — no collision.)*

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
