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

Currently: **0 active issues** (I-0051/I-0052/I-0053/I-0054/I-0055/I-0056/I-0057 ✅ Verified, awaiting batch archive).

_No Issues are currently open or awaiting verification._

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

Currently: **1 closed issue**

| Issue | File | Reason |
| ----- | ---- | ------ |
| I-0019 | [Issue-closed-0019.md](Closed/Issue-closed-0019.md) | Superseded / Design Decision — OBE; requirement carried by EP-019 AC1 (SP-053). Re-open if EP-019 is cancelled/descoped. |

## Backlogged Issues

Currently: **2 backlogged issues**

| Issue | Title | Severity | Status |
| ----- | ----- | -------- | ------ |
| I-0017 | Window maximized state not restored on app relaunch | Medium | 🔴 Open |
| I-0018 | Scene Navigator shows no selection on app load | Low | 🔴 Open |

See: [Issue-backlog.md](Issue-backlog.md)

## Statistics

- **Total Issues:** 64
- **Verified:** I-0001–I-0015, I-0020–I-0058 (except backlog/superseded), I-0061, **I-0062** ✅
- **Active:** 0
- **Open (backlog):** 4 🔴 (I-0017, I-0018 `[Apple]`; **I-0063** `[Linux]` renumber-on-delete, backlog; **I-0064** `[Linux]` chapter-split, targeted SP-067)
- **Closed (not verified):** 1 ⚪ (I-0019 — superseded by EP-019)
- **Superseded:** 1 ⚪ (I-0016 → I-0018)
- **Next available:** I-0083

---

*Last Updated: 2026-07-19 (**I-0080/I-0081/I-0082 filed + Resolved-Not-Verified**, all SP-073. I-0080
`[ScriviCore]`: open-time chapter migration undid a legitimate chapter reorder (no legacy gate; stale
index array hijacked as order authority) — digits-only legacy gate + eager cache rebuild. I-0081
`[ScriviCore]`+`[Linux]`: scene rename/save fails after a drag-reorder — §8 reorder renames/relocates the
scene files but the envelope reported no paths; envelope now reports post-move paths +
`refreshScenePaths` in the app (closes the I-0074/I-0079 stale-path class for scene drag). I-0082
`[Linux]`: chapter drag never started — non-selectable heading rows can never satisfy Qt's
selected-draggable gate; rows now selectable. I-0081/I-0082 found by the user's SP-073 VNC walkthrough.
Full entries in `Issue-active.md`. Statistics above predate a re-audit; next available **I-0083**. Prior
note follows.)*

*2026-07-15 (SP-066 `[Linux]` rename — **I-0062 ✅ Resolved-Verified** (new-chapter heading derives its "Chapter N" ordinal from order immediately, macOS parity). Two new `[Linux]` Issues filed during SP-066 VNC verify: **I-0064** (Ctrl+Shift+Return appends a chapter at the end instead of splitting at the caret — targeted SP-067) and **I-0063** (created chapters don't renumber on delete — backlog). Next available I-0065. Earlier: 2026-07-09 I-0058 Verified.)*


[1]:	./ER-Documentation.md
[2]:	Issue-active.md
[3]:	Verified/Issue-verified-0001-0010.md
[4]:	Verified/Issue-verified-0011-0020.md
