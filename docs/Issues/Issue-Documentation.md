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

Currently: **2 open backlogged issues** (+1 Verified, awaiting batch archive)

| Issue | Title | Severity | Status |
| ----- | ----- | -------- | ------ |
| I-0017 | Window maximized state not restored on app relaunch | Medium | 🔴 Open |
| I-0018 | Scene Navigator shows no selection on app load | Low | 🔴 Open |
| I-0112 | `[Apple]` Manuscript renders black-on-dark-gray in macOS Dark Mode (body runs had no `.foregroundColor`) | High | ✅ **Resolved - Verified (2026-08-11)** — Dark Mode confirmed; Light/live-switch/undo unexercised |

> **I-0112 remains in `Issue-backlog.md`** rather than moving to a `Verified/` batch: batch archiving currently
> stops at I-0050, and everything from I-0051 onward (including the Verified I-0104/I-0108/I-0110/I-0111) is
> still awaiting batch archive. It was filed sprintless, so it never passed through `Issue-active.md`.

See: [Issue-backlog.md](Issue-backlog.md)

## Statistics

- **Total Issues:** 68
- **Verified:** I-0001–I-0015, I-0020–I-0058 (except backlog/superseded), I-0061, **I-0062** ✅
- **Active:** 1 (🟢 **I-0086** `[Apple]` build `try?`-unused warnings, Resolved/Not-Verified, SP-075)
- **Verified (2026-07-21):** ✅ **I-0083** chapter-merge scene-loss (SP-074 core + SP-075 app adoption, EP-028); ✅ **I-0084** `[Apple]` caret-jump-after-scene-merge (SP-075)
- **Closed — Not a Bug:** **I-0085** `[Apple]` NSOpenPanel crash — environmental (duplicate instance + macOS 27.0 beta panel-XPC timeout); clean relaunch worked (2026-07-21)
- **Open (backlog):** 4 🔴 (I-0017, I-0018 `[Apple]`; **I-0063** `[Linux]` renumber-on-delete, backlog; **I-0064** `[Linux]` chapter-split, targeted SP-067)
- **Closed (not verified):** 1 ⚪ (I-0019 — superseded by EP-019)
- **Superseded:** 1 ⚪ (I-0016 → I-0018)
- **Next available:** I-0113

> ⚠️ **The Statistics block above is stale** (last re-audited around I-0086 and never updated through the
> I-0087–I-0111 run; the counts and per-issue call-outs predate SP-089 through SP-093). Only **Next
> available** and the **Backlogged Issues** table above it have been kept current. Treat
> `Issue-active.md` as authoritative for issue status until this block is re-audited.

---

*Last Updated: 2026-08-11 (**I-0112 filed, root cause corrected same day** — `[Apple]` Dark Mode. macOS on
"Appearance: Auto" flips to Dark at sunset and the manuscript renders **black text on a dark gray
background**, making the writing surface unusable for roughly half of every day. First hypothesis
(live-appearance-switch staleness) was **wrong** and is retained in the entry as superseded: the user
disproved it by observing the defect on a **cold launch**, hours after the switch. Confirmed cause is
static — the body-text attribute dictionaries omit `.foregroundColor` (`ManuscriptTextView.swift:517` and
`:296-298`) and `textColor` is never set, so AppKit renders body runs as literal `NSColor.black` while the
background follows the appearance. **Manuscript-only**, as the sole AppKit text surface; every SwiftUI view
uses `.foregroundStyle` and renders correctly (user-confirmed). Sprintless/unassigned; full entry in
`Issue-backlog.md`. **Fix implemented same day (🟡 Resolved - Not Verified):** `.foregroundColor:
NSColor.textColor` added to both body-text attribute dictionaries (initial build *and* the history-apply path,
so undo/redo cannot re-blacken text), a `textView.textColor` backstop in `makeNSView`, and appearance-scoped
drawing for the divider attachment cell. `ManuscriptTextView.swift` only — no new file, so no pbxproj change;
no core/ABI change. BUILD SUCCEEDED, interop **56/56** — but those tests do not exercise AppKit rendering, so
**acceptance rests on user verification**. **✅ Verified same day (user-confirmed):** *"Manuscript View appears
now in the live demo with light text on a dark background."* ⚠️ **Verified in Dark Mode only** — Light Mode,
live appearance switch, and undo/redo-in-Dark were not exercised (all share the confirmed fix's one-line
mechanism; re-open I-0112 rather than filing anew if one misbehaves). The all-views Dark Mode sweep also
remains outstanding. Next available **I-0113**. Prior note follows.)*

*2026-07-19 (**I-0080/I-0081/I-0082 filed + Resolved-Not-Verified**, all SP-073. I-0080
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
