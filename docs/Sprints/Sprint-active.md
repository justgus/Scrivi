# Active Sprint

**No active sprint.** EP-019 and EP-030 both closed 2026-08-11; **EP-031** is next and its first sprint
(**SP-095** — object kinds + fields + object index) is staged in `Sprint-backlog.md`, awaiting activation.

---

## Recently closed (2026-08-11) — double Epic close

| Sprint | Title | Epic | Archive |
| ------ | ----- | ---- | ------- |
| SP-101 | `[Apple]` AC12 soft-failure isolation (T-0399) | EP-030 | `Closed/Sprint-SP-101.md` |
| SP-094 | EP-019 + EP-030 verification & Epic close (merged) | EP-019 + EP-030 | `Closed/Sprint-SP-094.md` |
| SP-093 | `[Cross]` EP-019 history capture granularity + presentation (11 items) | EP-019 | `Closed/Sprint-SP-093.md` |
| SP-092 | `[Cross]` EP-030 history card + Properties tab | EP-030 | `Closed/Sprint-SP-092.md` |

**✅ EP-019 CLOSED** (Human-approved) — AC1–AC8 all Verified across 7 sprints. AC2 amended and design §4.d
relaxed, both user-approved as a design change. → `../Epics/Closed/Epic-EP-019.md`

**✅ EP-030 CLOSED** (Human-approved) — AC1–AC7 all Verified across 4 sprints. AC12 rescoped to soft failures
and delivered by T-0399. → `../Epics/Closed/Epic-EP-030.md`

**Suites at close:** ctest **413/413** · macOS interop **56/56** · TEST SUCCEEDED.

---

## Carried forward — deliberately, not forgotten

1. **T-0400 — history log-segment rotation.** 🟢 Nice-to-have (user ruling 2026-08-11). `activeSegment_` is
   hard-fixed to `log-000001.jsonl`; capacity/eviction bounds the tree, not the log, so a segment grows
   unbounded (a real project is already ~3.4 MB). Additive when done — the reader already honours
   `activeLogSegment`. No sprint assigned.

2. **EP-019 ships with no large-scale performance coverage.** T-0216's fixtures (100k events, 500 KB scene)
   were closed OBE. If history performance is ever suspect at scale, open a **new** task.

3. **`HistoryCapture` is not in the test target**, so T-0396's timing logic (45 s idle boundary, deferred
   save, backspace-does-not-split) has **no automated coverage**. EP-019 AC2 items 5–8 rest on live
   verification alone; a regression there would not be caught by CI.
