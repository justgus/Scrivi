# Active Epics

## 🟡 **[EP-043]** — `[Linux]` **The Session** — ✅ **ACTIVATED 2026-09-25 (user-approved)**

✅ **Full record:** → [`Epic-EP-043.md`](Epic-EP-043.md) (⚠️ **R1–R8, four Sprints**; ✅ **all five owed
rulings ANSWERED 2026-09-26**).
**Goal:** ⚠️ **many projects, each in its own window, restored where the writer left it.**
**Closes:** ✅ **[I-0178]** (multi-project) · ✅ **[I-0176]** (reopen at launch) · ✅ **[I-0177]** (window
+ splitter geometry) — ⚠️ **all three found by the USER on the REAL RIG 2026-08-29, the first day the
Linux app ran on real hardware; ⛔ NONE by a test suite.**
**Codebase:** ⚠️ **`platforms/linux/` ONLY.** ✅ **No ScriviCore change expected.**
**Apple precedent:** ✅ **[EP-018]** → [`Closed/Epic-EP-018.md`](Closed/Epic-EP-018.md), R1–R5
user-verified 2026-06-25 in **3 Sprints**. ⚠️ **This is scoped at 4** — ✅ **the extra is [SP-145]'s
extraction.**

✅ **[SP-145] CLOSED 2026-09-29 (user-approved) — ⚠️ S1 of 4 COMPLETE.** ✅ **All eight ACs MET; the
LIVE PASS PASSED on the rig** (*"the live test passed. hide inspector survived a restart"*).
✅ **T-0551/T-0552/T-0553 VERIFIED and archived** →
[`../Tasks/Verified/Task-verified-0551-0553.md`](../Tasks/Verified/Task-verified-0551-0553.md);
✅ **[I-0251] VERIFIED.**
⚠️ **The pass produced [I-0255], [I-0256], [T-0556] and [T-0557] — ⛔ none a regression.**
✅ **TWO WERE RULED THE SAME DAY:** ⛔ **timeline visibility SHOULD persist ([I-0255], now a `[Cross]`
defect on BOTH platforms)** · ⚠️ **the Navigator is hidden by collapsing its splitter to 0, ⛔ not by a
new control ([I-0256]) — ✅ and index 0 already collapses, so no widget change is needed.**

✅ **[SP-146] CLOSED 2026-09-29 (user-approved) — ⚠️ S2 of 4 COMPLETE** →
[`../Sprints/Closed/Sprint-SP-146.md`](../Sprints/Closed/Sprint-SP-146.md). ✅ **ALL ELEVEN ACs MET;
⚠️ LIVE PASS PASSED on the rig.** ✅ **T-0558–T-0562 VERIFIED and archived** →
[`../Tasks/Verified/Task-verified-0558-0562.md`](../Tasks/Verified/Task-verified-0558-0562.md);
✅ **[I-0257] VERIFIED.**
⚠️ **HEADLINE: ✅ Linux opens TWO projects in TWO windows — ⛔ [I-0178], the defect this Epic exists
for.** ⚠️ **The user's FIRST rig pass confirmed AC9 and FOUND [I-0257]** (⛔ a quit that left windows
open non-deterministically, ⚠️ a regression from [T-0561]) — ✅ **reproduced, fixed, re-verified.**

✅ **[SP-147] CLOSED 2026-09-30 (user-approved) — ⚠️ S3 of 4 COMPLETE** →
[`../Sprints/Closed/Sprint-SP-147.md`](../Sprints/Closed/Sprint-SP-147.md). ✅ Projects reopen where the writer left them
(size, maximized, splitters); ⚠️ window POSITION ruled out on Wayland ([I-0264]). ➡️ **[SP-148] (S4) is NEXT.**
⚠️ **[I-0256] remains a fold-in candidate** (⛔ still not built).

⚠️ **PRIOR STATE — ✅ ALL FIVE OWED RULINGS WERE ANSWERED 2026-09-26 (user-approved).** ✅ **Full text in the Epic record's §Rulings.** The five, in one line each:
✅ **[R-Q1]** session state → **`<appSupportRoot>/session.ini`** via `QSettings`, mirroring the existing
`timeline-view.ini` · ✅ **[R-Q2]** key by **`projectID`**, with `path` as a stored ATTRIBUTE (⚠️ *a moved
project is skipped for one launch but KEEPS its geometry*) · ✅ **[R-Q3]** a **SEPARATE Landing window**;
project windows are editor-only; `File ▸ New`/`Open` RAISE Landing; ⛔ **closing the last project shows
Landing and never quits** · ✅ **[R-Q4]** inspector visibility persists **THROUGH THE CORE** (Apple
parity), timeline stays session-scoped · ✅ **[R-Q5]** **[I-0244]** becomes a **SIBLING Epic, AFTER this
one**, with a named carve-out in [SP-145].

⚠️ **THREE OF THE FIVE QUESTIONS WERE FRAMED ON A PREMISE THE CODE CONTRADICTED**, and in each case the
correction changed the ANSWER: ⛔ *"Apple used `UserDefaults`"* (✅ **it SPLITS — per-project document
state goes through the core, which is what [R-Q4] needed**) · ⛔ *"projectID needs a resolver Linux does
not have"* (✅ **`EditorShell.cpp:2087` already keys persisted state by `projectID`**) · ⛔ *"should Linux
reverse SP-078?"* (✅ **not a reversal — Apple already persists inspector; Linux is simply out of
parity**). ✅ **The superseded text is kept in §Rulings because the error is the instructive part.**
✅ **[R-Q4] FILED [I-0251]** (⚠️ *Linux reads `inspectorHidden`'s default and never writes it back*)
**against [SP-145].**

✅ **WHY THIS EPIC AND NOT [EP-035]** (⚠️ **the other ready `[Linux]` Epic**): ⛔ **a FILE COLLISION,
measured not assumed.** ⚠️ **[SP-145] extracts per-project state out of `EditorShell.cpp` (2,783
lines), and [EP-035]'s open **AC4** (object CRUD) attaches to that SAME owner** — ✅
**`EditorShell::onOpenObjectRequested` (`EditorShell.cpp:2214`, wired from `SceneInspector` at `:131`).**
⛔ **[EP-035] first would write CRUD into `EditorShell` and have [SP-145] relocate it immediately.**
✅ **[EP-035]'s AC5 (thumbnails) does NOT collide** (⚠️ `SceneInspector.cpp`) — ✅ **it could run
alongside if object work is wanted sooner.**

---

✅ **EP-040** — `[Apple]` **The Editor Shell** — **CLOSED 2026-09-24 (user-approved)**
→ [`Closed/Epic-EP-040.md`](Closed/Epic-EP-040.md). ✅ **Five Sprints: SP-134 · SP-135 · SP-136 ·
SP-137 · SP-150.** ⚠️ **Twelve ACs MET; ✅ AC10 RULED OBE.** ✅ **An Audit Check ran before the close**
(`../Audits/Audit-Check-20260924.md`) — ⚠️ **three findings, all ruled and remediated.**
✅ **EP-041** — `[Cross]` **The Boundary** — **CLOSED 2026-09-22 (user-approved)**
→ [`Closed/Epic-EP-041.md`](Closed/Epic-EP-041.md).
✅ **[EP-042] CLOSED 2026-09-20** (below).

⛔ **[I-0244] REMAINS OPEN and was NOT closed by [EP-040]** — ⚠️ **it is the Linux shell gap.**
✅ **RULED 2026-09-26 ([EP-043] [R-Q5]): it becomes its OWN `[Linux]` Epic, sequenced AFTER [EP-043]** —
⛔ **NOT ACs inside it** (⚠️ *it is four Apple Sprints deep; folding it in would repeat the [EP-035] AC1
collapse*). ✅ **[SP-145] carries a named carve-out so the extraction is paid ONCE.**
✅ **[I-0206] IS NOW CLOSED 2026-09-25 (user-approved)** → [`../Issues/Closed/Issue-closed-0206.md`](../Issues/Closed/Issue-closed-0206.md)
— ⛔ **NOT as a limitation, and NOT as work done: ⚠️ CLOSED AS NOT-A-DEFECT, because `docs/` states NO
keystroke or latency REQUIREMENT for it to violate.** ✅ **Its measurements are preserved in the closed
record, with a re-open condition on `setSelectedRange`'s offset-linear cost.**
⛔ **[EP-039] `Project Load Performance` was NOT reopened** (⚠️ **user: *"I'm not going backwards"***);
✅ **a closed Epic keeps its record.** ⚠️ **EP-039 measured on LOCAL DISK, where the page cache hides
the amplification [EP-042] exists to remove.** ✅ **EP-041 was split OUT of EP-040** because
[I-0197]'s bypass chain ([SP-140]–[SP-143]) is BOUNDARY work, not chrome — ⚠️ **EP-040's own scope note
had already said its goal line would not answer for it.** ⛔ **[SP-129] and [SP-130] did NOT move:**
✅ **a CLOSED Sprint keeps the provenance of the Epic it ran under** (user ruling 2026-09-18);
✅ **their outcomes are credited in EP-041's class table.**

✅ **EP-041 CLOSED 2026-09-22 (user-approved)** → [`Closed/Epic-EP-041.md`](Closed/Epic-EP-041.md).
✅ **Five Sprints, six Tasks, [I-0197] closed.** ⚠️ **THE FINDING WORTH CARRYING: THREE defects of ONE
shape — capability shipped with no reader** ([I-0215], [I-0241], [I-0242]) — ⛔ **and NOT ONE was found
by a test suite.** ✅ **[I-0242] is now [EP-036]'s (AC4a/AC4b); ⚠️ [I-0223] is unassigned.**
⚠️ **Its Audit Check RECOVERED [I-0223]**, ✅ **which had been referenced in five documents and present
in none since 2026-09-18** — ⛔ **the same class as [I-0118] in the 2026-08-19 audit.**

✅ **EP-042** `[Cross]` Project Open Cost — **CLOSED 2026-09-20 (user-approved)**
→ [`Closed/Epic-EP-042.md`](Closed/Epic-EP-042.md).
✅ **Created 2026-09-18 from a user report that a real project opened "empty" on Linux** — ⚠️ **it did
not; it took 6m11s.** ✅ **Goal MET and proven on the real rig at `cache=none`: 24.01 s → 13.46 s,
5,002 → 2,999 syscalls, `binding.json` 188 → 2 opens.** ⚠️ **It opened with TWO Issues and closed with
SIX** — ✅ **[I-0234], the per-scene write that made SMALL projects slow, was found only because the
user reported that recent projects were NOT large, which falsified the working theory.**

✅ **EP-039** `[Cross]` Project Load Performance — **CLOSED 2026-09-15**
→ [`Closed/Epic-EP-039.md`](Closed/Epic-EP-039.md) (⚠️ **moved 2026-09-15, audit ruling [R-07]**).
⚠️ **[I-0206]** (⚠️ *every keystroke costs ~59 ms inside AppKit and `setSelectedRange` is LINEAR IN DOCUMENT OFFSET*) **and [I-0213]** (⚠️ *chapter create froze the app ~2.7 s on 1,174 scenes*) **were CARRIED from it into EP-040;** ✅ **[I-0213] is now VERIFIED (2026-09-24); ✅ [I-0206] is CLOSED 2026-09-25 as NOT-A-DEFECT** (⚠️ **no
requirement exists for it to violate**) → [`../Issues/Closed/Issue-closed-0206.md`](../Issues/Closed/Issue-closed-0206.md).

⚠️ **Every other Epic is in [`Epic-backlog.md`](Epic-backlog.md).**
⛔ **No count is stated here** — ✅ **audit ruling [R-15]; read the rows.**

---

