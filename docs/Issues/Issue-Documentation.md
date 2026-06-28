# Issues (I) - Index

This is the main index for all Scrivi Issues. Issues track bugs and unintended system behavior.

> **Note:** For planned improvements and new features, see [Tasks (T)][1]
> 
## Organization

- **I-active** - Active, unresolved issues assigned to a Sprint
- **I-backlog** - Inactive, unresolved issues currently not assigned to a Sprint
- **I-closed** - Inactive, unresolved issues that will not be worked.
- **I-verified-XXXX-YYYY.md** - Resolved and verified issues in batches of 10

## Active Issues

Currently: **3 active issues**

| Issue | Title | Severity | Sprint |
| ----- | ----- | -------- | ------ |
| I-0051 | Restored project windows don't remember per-window size/position | Medium | SP-046 (🟢 Resolved - Not Verified) |
| I-0053 | iOS `ScriviEngine` stubbed — ScriviCore not built/linked for iOS | High | SP-046 (🟢 Resolved - Not Verified) |
| I-0054 | iPad has no button bar and no iOS menu bar — Project Settings / Close Project unreachable | High | SP-046 (🔴 Open) |

See: [Issue-active.md][2]

## Verified Issues

Currently: **40 verified issues**

| Batch | DRs              | File                              | Status      |
| ----- | ---------------- | --------------------------------- | ----------- |
| 1     | I-0001 – I-0010  | [Issue-verified-0001-0010.md][3]  | ✅ Complete |
| 2     | I-0011 – I-0020  | [Issue-verified-0011-0020.md][4]  | ✅ Complete (10/10) |
| 3     | I-0021 – I-0030  | [Issue-verified-0021-0030.md](Verified/Issue-verified-0021-0030.md) | ✅ Complete (6/10) |
| 4     | I-0031 – I-0040  | [Issue-verified-0031-0040.md](Verified/Issue-verified-0031-0040.md) | ✅ Complete (10/10) |
| 5     | I-0041 – I-0050  | [Issue-verified-0041-0050.md](Verified/Issue-verified-0041-0050.md) | 🟡 In Progress (9/10) |

## Closed Issues (Not Verified)

Currently: **0 closed issues**

| Batch | DRs | File | Reason |
| ----- | --- | ---- | ------ |
|       |     |      |        |

## Backlogged Issues

Currently: **3 backlogged issues**

| Issue | Title | Severity |
| ----- | ----- | -------- |
| I-0017 | Window maximized state not restored on app relaunch | Medium |
| I-0018 | Scene Navigator shows no selection on app load | Low |
| I-0019 | Undo and Redo have no effect in the manuscript editor | High |

See: [Issue-backlog.md](Issue-backlog.md)

## Statistics

- **Total Issues:** 54
- **Verified:** 47 ✅ (I-0001–I-0015, I-0020–I-0050, I-0052; I-0016 superseded; I-0017–I-0019 backlog)
- **Active:** 3 ⚠️ (I-0051, I-0053 — SP-046, Resolved - Not Verified; I-0054 — Open)
- **Open (backlog):** 3 🔴 (I-0017, I-0018, I-0019)
- **Superseded:** 1 ⚪ (I-0016 → I-0018)
- **Next available:** I-0055

---

*Last Updated: 2026-06-28 (I-0054 filed — iPad has no button bar and no iOS menu bar, so Project Settings / Close Project are unreachable on iPad; surfaced during T-0123 verification. Active table corrected: I-0052 is Verified (was stale), I-0053 added.)*


[1]:	./ER-Documentation.md
[2]:	Issue-active.md
[3]:	Verified/Issue-verified-0001-0010.md
[4]:	Verified/Issue-verified-0011-0020.md
