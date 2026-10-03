# Active Sprints

## No Sprint is active (SP-152 closed 2026-10-03)

✅ **[SP-152] CLOSED 2026-10-03 (user-approved)** `[Cross]` → [`Closed/Sprint-SP-152.md`](Closed/Sprint-SP-152.md) — ✅ [I-0268] tintagael history fails to open — **fixed and USER-VERIFIED 2026-10-03, archived** (⚪ [I-0267] closed 2026-10-02: duplicate of [I-0206]'s ruling).
✅ **[SP-151] CLOSED 2026-10-02 (user-approved)** `[Apple]` → [`Closed/Sprint-SP-151.md`](Closed/Sprint-SP-151.md) — ✅ 18 items verified (incl. [I-0270] data-corruption fix on both platforms, edit-group undo in the core); ⚪ AC2/[I-0202] closed WITHOUT a resolution (user ruling) → Issue backlog; ➡️ [I-0268] carried to [SP-152]; ⚪ [I-0267] closed as a duplicate of [I-0206].
✅ **[SP-148] CLOSED 2026-09-30 (user-approved)** `[Linux]` → [`Closed/Sprint-SP-148.md`](Closed/Sprint-SP-148.md) — ✅ [EP-043] S4 complete; ✅ **[EP-043] CLOSED 2026-09-30 (user-approved).**
✅ **[SP-147] CLOSED 2026-09-30 (user-approved)** `[Linux]` → [`Closed/Sprint-SP-147.md`](Closed/Sprint-SP-147.md) — ✅ [EP-043] S3 complete; ➡️ **[SP-148] next**.

---

✅ **[SP-146] CLOSED 2026-09-29 (user-approved)** → [`Closed/Sprint-SP-146.md`](Closed/Sprint-SP-146.md).
✅ **ALL ELEVEN ACs MET; ⚠️ LIVE PASS PASSED on the rig** — ✅ ***"I tested it on the rig and it
passed."*** ✅ **T-0558 · T-0559 · T-0560 · T-0561 · T-0562 VERIFIED and archived** →
[`../Tasks/Verified/Task-verified-0558-0562.md`](../Tasks/Verified/Task-verified-0558-0562.md).
✅ **[I-0257] VERIFIED.** ✅ **[EP-043] S2 of 4 COMPLETE.**
⚠️ **HEADLINE: Linux opens TWO projects in TWO windows** — ⛔ **[I-0178], the defect this Epic exists
for.** ⚠️ **The user's FIRST rig pass confirmed AC9 and FOUND [I-0257]** (⛔ a quit that left windows
open non-deterministically) — ✅ **fixed in [T-0562] and re-verified.**



✅ **[SP-145] CLOSED 2026-09-29 (user-approved)** → [`Closed/Sprint-SP-145.md`](Closed/Sprint-SP-145.md).
✅ **ALL EIGHT ACs MET; ⚠️ LIVE PASS PASSED on the rig** — ✅ ***"the live test passed. hide inspector
survived a restart."*** ✅ **T-0551, T-0552 and T-0553 VERIFIED and archived** →
[`../Tasks/Verified/Task-verified-0551-0553.md`](../Tasks/Verified/Task-verified-0551-0553.md).
✅ **[I-0251] VERIFIED.** ✅ **[EP-043] S1 of 4 COMPLETE.**
⚠️ **THE PASS PRODUCED FOUR ITEMS, ⛔ NONE a regression:** ✅ **[I-0255]** (⛔ timeline visibility does
not persist — ⚠️ **RULED 2026-09-29 that it SHOULD, ⛔ making it a `[Cross]` defect on BOTH platforms**)
· ✅ **[I-0256]** (⛔ no Navigator control on Linux — ⚠️ **RULED: collapse the splitter to 0, ✅ which
already works; `setCollapsible(false)` is applied to the inspector only**) · ✅ **[T-0556]**
(`View ▸ Hide All` / `Restore All`, ⚠️ **RULED to PERSIST across a restart**) · ✅ **[T-0557]**
(⚠️ buffers palette per-session — ⛔ **excluded from this Sprint: the palette is macOS-only and this
Sprint is `[Linux]`**).

✅ **[SP-146] IS NOW ACTIVE (above), created and activated 2026-09-29.**

---

✅ **[SP-134] CLOSED 2026-09-22 (user-approved)** → [`Closed/Sprint-SP-134.md`](Closed/Sprint-SP-134.md).
✅ **T-0543 VERIFIED by live pass.** ✅ **[EP-040] AC2 and AC3 MET.**
⚠️ **It produced [I-0243]** — ⛔ **the project title renders THREE times, from three independent
sources** — ✅ **filed for [SP-135], which already owns the title area.**
✅ **User ruled OPTION A for the tab bar: windows only (`tabbingMode = .disallowed`).**

✅ **[SP-135] CLOSED 2026-09-22 (user-approved)** → [`Closed/Sprint-SP-135.md`](Closed/Sprint-SP-135.md).
✅ **All EIGHT ACs met; T-0545 VERIFIED.** ✅ **[EP-040] AC4, AC5 and AC9 MET.**
✅ **[I-0203] STRUCTURALLY FIXED** — ⚠️ **bars were `VStack` siblings, which is why ONE mistake read as
FOUR bugs.** ✅ **[I-0205] FULLY RESOLVED, both halves.**

✅ **[SP-137] CLOSED 2026-09-23 (user-approved)** → [`Closed/Sprint-SP-137.md`](Closed/Sprint-SP-137.md).
✅ **ALL EIGHT ACs met; AC2 and AC8 USER-VERIFIED.** ✅ **T-0547 and T-0548 VERIFIED and archived.**
⚠️ **IT PRODUCED THREE ISSUES, ALL FROM ONE TASK** — ✅ **[I-0248]** (a `.toolbar` renders NOTHING on a
macOS sheet) · ✅ **[I-0250]** (`.cancelAction` on a toolbar item never receives Esc — ⛔ silent data
loss) · ✅ **[I-0249]** (min-width contention between adjacent views; ⛔ the runtime guard REMOVED by
user ruling) — ⚠️ **all archived with the close.**
⛔ **T-0547 TOOK FOUR IMPLEMENTATIONS, each building clean and passing every suite** — ✅ **the lesson
is recorded in [`../Tasks/Verified/Task-verified-0547-0548.md`](../Tasks/Verified/Task-verified-0547-0548.md).**

✅ **[SP-150] CLOSED 2026-09-24 (user-approved)** → [`Closed/Sprint-SP-150.md`](Closed/Sprint-SP-150.md).
✅ **T-0549 and T-0550 VERIFIED and archived.** ✅ **[EP-040] AC11, AC12 and AC13 MET.**
✅ **`reloadSceneDots` ~235 ms → `1.4 ms` on 1,178 scenes (~170x), measured on the user's USB rig.**
⛔ **NO SPRINT IS ACTIVE.**
⚠️ **[SP-137] was [EP-040]'s last PLANNED Sprint** — ✅ **the Epic's
remaining ACs can now be swept.** ⚠️ **AN AUDIT CHECK IS OWED BEFORE THE EPIC CLOSES** (⛔ **it is NOT
an Audit — the lightweight mechanical sweep**).
⚠️ **[EP-040] AC11** (⚠️ *the carried performance clause: the manuscript surface's remaining
O(DOCUMENT) costs are addressed OR accepted as limitations WITH a measurement*) **carried [I-0206]**
(⚠️ *offset-linear `setSelectedRange`, `57–81 ms` near the end of a 1.85 MB document*) **and [I-0213]**
(⚠️ *chapter create froze the app ~2.7 s on 1,174 scenes*) **from [EP-039]** (⚠️ *Project Load
Performance — it replaced a ~300 s frozen open*) — ⛔ **neither is this Sprint's.**
✅ **[I-0213] IS SETTLED AND ITS REMAINDER IS NOW FIXED** — ✅ **[SP-150] (T-0549 + T-0550) closed
2026-09-24; `reloadSceneDots` ~235 ms → `1.4 ms` on the USB rig.**
✅ **[I-0206] WAS RULED 2026-09-25, AFTER the Epic close — ⛔ and NEITHER branch of AC11 is what it
took.** ⚠️ **It is CLOSED AS NOT-A-DEFECT** → [`../Issues/Closed/Issue-closed-0206.md`](../Issues/Closed/Issue-closed-0206.md):
⛔ **`docs/` states NO keystroke or latency requirement for it to violate**, ✅ **so its figures are
MEASUREMENTS, not a limitation.** ⚠️ **Re-open condition: if `setSelectedRange`'s offset-linear cost
becomes a problem in real use.**

✅ **[SP-136] CLOSED 2026-09-23 (user-approved)** → [`Closed/Sprint-SP-136.md`](Closed/Sprint-SP-136.md).
⚠️ **All 9 ACs VERIFIED by the user's live pass; T-0546 VERIFIED and archived.** ⚠️ **THREE Issues came
out of that pass and NONE from any suite:** ✅ **[I-0245]** (the Detail Sheet's `HStack` shape made the
window's widths unsatisfiable and CRASHED the app — now a real `.sheet`) and ✅ **[I-0246]** (a status
banner, since the sheet no longer closes on Save) are **VERIFIED**; ⛔ **[I-0247]** (D1-E's NON-MODAL
intent is superseded) is **OPEN for [SP-137] to rule.**

⛔ **PRIOR STATE:** ✅ **[EP-041] CLOSED 2026-09-22 (user-approved)** →
[`../Epics/Closed/Epic-EP-041.md`](../Epics/Closed/Epic-EP-041.md) — ⚠️ **all five of its Sprints,
six Tasks and [I-0197] closed with it.**
✅ **[EP-040] CLOSED 2026-09-24.** ✅ **[EP-043] `[Linux]` The Session ACTIVATED 2026-09-25 (above).**

✅ **[SP-143] CLOSED 2026-09-22 (user-approved)** → [`Closed/Sprint-SP-143.md`](Closed/Sprint-SP-143.md)
— ⚠️ **the Epic's LAST.** ✅ **T-0510 VERIFIED; ✅ [I-0197] CLOSED.**
⚠️ **D1 held:** ✅ **the guard went RED against `78a739f~1`** — ⛔ **the last commit where both owners
of `inspector-layout.json` existed** — ✅ **and caught [I-0241]'s disk walk as a bonus.**

✅ **[SP-149] CLOSED 2026-09-22 (user-approved)** → [`Closed/Sprint-SP-149.md`](Closed/Sprint-SP-149.md).
✅ **T-0541 VERIFIED by a GREEN GitHub run** (Apple CI #1, `a25e106`) — ⚠️ **the evidence local
testing could not produce.** ✅ **`Scrivi/` has CI for the first time.**

✅ **[SP-142] CLOSED 2026-09-21 (user-approved)** → [`Closed/Sprint-SP-142.md`](Closed/Sprint-SP-142.md).
✅ **All TEN ACs met; live pass PASSED on BOTH Apple and Ubuntu.** ✅ **T-0537, T-0542 and [I-0241]
VERIFIED and archived.** ✅ **[EP-041] AC3 MET — [I-0197] Class B closed on both platforms.**
⚠️ **The pass also produced [I-0242]** — ⛔ **Linux never reads the writer's card stack** — ✅ **filed
unassigned; [EP-036] gained AC4a/AC4b.**

✅ **[SP-141] CLOSED 2026-09-21 (user-approved)** → [`Closed/Sprint-SP-141.md`](Closed/Sprint-SP-141.md)
under ✅ **[EP-041]** (now CLOSED). ✅ **T-0507 VERIFIED by live pass; [EP-041] AC2 MET.**
⚠️ **ITS BODY IS NOT KEPT HERE** — ✅ **the closed record supersedes it** (`Epic-GUIDELINES.md`:
*"strip the active-file entry down to a pointer"*).

✅ **[SP-140] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-140.md`](Closed/Sprint-SP-140.md)
— ⚠️ **planned, implemented, verified and closed in ONE DAY.** ✅ **T-0536 and [I-0215] both Verified.**
⚠️ **It also produced [I-0223]**, ✅ **filed unassigned.**

✅ **[SP-130] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-130.md`](Closed/Sprint-SP-130.md).

---
---

## 🟡 **[EP-043]** `[Linux]` **The Session** IS ACTIVE — ✅ **activated 2026-09-25**
✅ **Record:** → [`../Epics/Epic-EP-043.md`](../Epics/Closed/Epic-EP-043.md).

## ✅ **[SP-145]** `[Linux]` **The Session Split** — ✅ **CLOSED 2026-09-29 (user-approved)**

✅ **Record:** → [`Closed/Sprint-SP-145.md`](Closed/Sprint-SP-145.md). ✅ **ALL EIGHT ACs MET; ✅ LIVE
PASS PASSED.** ✅ **T-0551 · T-0552 · T-0553 VERIFIED and archived** →
[`../Tasks/Verified/Task-verified-0551-0553.md`](../Tasks/Verified/Task-verified-0551-0553.md).
⚠️ **The detail lives in the closed record** (`Epic-GUIDELINES.md`: *"strip the active-file entry down
to a pointer"*) — ✅ **what follows is kept HERE only because [SP-146] needs it.**

⛔ **THE ONE FINDING [SP-146] MUST NOT LOSE:** ⚠️ **`registry_` sits on `EditorShell` and must be
LIFTED to the app/window level as [SP-146]'s FIRST step** — ✅ **a per-shell registry cannot answer R3
across windows, which is the whole reason to have one.**

⚠️ **ID NOTE, still in force:** ✅ **`next-id.py` offered SP-151; the reservation in five documents said
SP-145.** ✅ **User ruled 2026-09-27: honour the reservation.** ⚠️ **The counter is a HIGH-WATER MARK,
reset to 146 with a `_reserved` note so it is not "corrected" upward again.**

---

⚠️ **PRIOR STATE (rulings, 2026-09-26):** ⚠️ **all FIVE owed rulings were answered
2026-09-26 (user-approved);** ✅ **full text in that record's §Rulings.** ⚠️ **What [SP-145] now carries
that it did not before:** ✅ **the registry is keyed by `projectID` ([R-Q2])** · ✅ **the extraction takes
`inspectorVisible_` + `timelineVisible_` onto the session object ([R-Q5]'s carve-out, so the shell Epic
does not re-open `EditorShell`)** · ⚠️ **plus [I-0251] as a SEPARATE, NAMED Task** — ⛔ **it is a real
behaviour CHANGE inside a behaviour-preserving Sprint and must not be folded into the extraction.**
⚠️ **Its chain is SERIAL: [SP-145] → [SP-146] → [SP-147] → [SP-148]**, ⛔ **so a stall in [SP-145]
stalls the Epic.**

### ⛔ Previously closed Epics

✅ **[EP-040]** `[Apple]` **The Editor Shell** — **CLOSED 2026-09-24** → [`../Epics/Closed/Epic-EP-040.md`](../Epics/Closed/Epic-EP-040.md).
✅ **[EP-041]** `[Cross]` **The Boundary** — **CLOSED 2026-09-22** → [`../Epics/Closed/Epic-EP-041.md`](../Epics/Closed/Epic-EP-041.md).
✅ **[EP-042]** `[Cross]` **Project Open Cost** — **CLOSED 2026-09-20** → [`../Epics/Closed/Epic-EP-042.md`](../Epics/Closed/Epic-EP-042.md),
⚠️ **closed with [SP-144]** → [`Closed/Sprint-SP-144.md`](Closed/Sprint-SP-144.md) (✅ **all 7 ACs met;
SIX Issues VERIFIED on the real rig under `cache=none`**).

---

## ✅ THE TWO [SP-141] RULINGS — ⚠️ **moved to the closed record 2026-09-21**

⚠️ **Both rulings governed a Sprint that is now CLOSED, so their text lives with it** —
✅ [`Closed/Sprint-SP-141.md`](Closed/Sprint-SP-141.md) — ⛔ **not here, where it would be a second
source of truth going stale.** ✅ **They remain BINDING on [SP-142] and [SP-143]:**

1. ✅ **ENDPOINT SHAPE (2026-09-18): ONE OPAQUE DOCUMENT GET/PUT.** ✅ **The core owns atomicity,
   durability and repair; the APP owns meaning.** ⚠️ **ACCEPTED COST: the core cannot validate what it
   stores there.**
2. ✅ **ABSENCE SEMANTICS (2026-09-21): "core reports, app decides"** — `status` = `ok` | `absent` |
   `unreadable`. ⛔ **The core never invents defaults and never overwrites a corrupt file on read.**

⚠️ **[SP-142] and [SP-143] honoured BOTH; ✅ both are CLOSED.** ⚠️ **The rulings REMAIN BINDING on any
future work against that endpoint pair.**

---


## ✅ What SP-130 delivered (closed 2026-09-18)

✅ **T-0508 ✅ VERIFIED** by the user's live pass → [`../Tasks/Verified/Task-verified-0508.md`](../Tasks/Verified/Task-verified-0508.md).
⛔ **T-0509 STRUCK 2026-09-15** — ✅ **re-measurement proved its remainder is ZERO; not deferred.**

⚠️ **ITS PREMISE DID NOT HOLD, and that is the result worth carrying forward:** ⚠️ **it was planned as
"a RULING to record, not a defect to fix"** — ⛔ **and `ExistingAssetPicker.swift:130` was a real
main-actor block**, ✅ **found precisely BY writing the ruling, which split *"degrades gracefully"*
(architecture) from *"blocks the main actor"* (cost).** ⚠️ **A re-measurement that asks only one of two
questions will clear a site that fails the other.**

⚠️ **Its verification pass produced TWO Issues the Sprint did not plan for** — ⛔ **the drive-pull test
had been deferred as "informational" and was not:**
⛔ **[I-0221]** (**Critical** — a project on ANY non-APFS volume was repeatedly unopenable) and
✅ **[I-0222]** (an unavailable world reported `ScriviError 1`). ✅ **Both fixed, Verified, and archived**
→ [`../Issues/Verified/Issue-verified-0221-0230.md`](../Issues/Verified/Issue-verified-0221-0230.md).
✅ **It also CONFIRMED the pending-world architecture works** — ⚠️ **the more important half**: the
manuscript stayed usable with the volume gone and everything restored on reattach.

⛔ **[I-0197] WAS NOT CLOSED BY [SP-130]** — ✅ **Class C only.** ⚠️ **Classes A and B continued under
[EP-041]** — ✅ **and [I-0197] CLOSED with [SP-143] on 2026-09-22.**

---

## ✅ Sprints that were queued here — ⚠️ **ALL CLOSED**

⛔ **NOTHING IS AVAILABLE TO ACTIVATE FROM THIS FILE.** ✅ **Both tracks finished:**
⚠️ **Track 1 (the editor shell) closed with [EP-040]** → [`../Epics/Closed/Epic-EP-040.md`](../Epics/Closed/Epic-EP-040.md)
— ✅ **[SP-134] · [SP-135] · [SP-136] · [SP-137] · [SP-150].** ⛔ **[SP-138]** (`NSSplitViewController`
rebuild, **S5**) was **RECORDED, NEVER SCHEDULED, and NEVER CREATED** — ⚠️ **it exists ONLY as a row in
[`../Epics/Closed/Epic-EP-040.md`](../Epics/Closed/Epic-EP-040.md) (`:160`, `:278`); ⛔ it is NOT in
[`Sprint-backlog.md`](Sprint-backlog.md).** ✅ **Reviving S5 means planning it fresh.**
⚠️ **Track 2 (the [I-0197] bypass chain) closed with [EP-041]** → [`../Epics/Closed/Epic-EP-041.md`](../Epics/Closed/Epic-EP-041.md)
— ✅ **[SP-140] · [SP-141] · [SP-142] · [SP-143] · [SP-149]; [I-0197] CLOSED.**
✅ **The per-Sprint records in [`Closed/`](Closed/) are the source of truth for both.**

---

## ⚠️ Two rulings raised by SP-130 and deliberately NOT taken

⚠️ **Recorded so they are not lost; ⛔ neither is scheduled.**

1. ⛔ **`SceneIndex` aborts an ENTIRE project open on any unparseable file**, where its two sibling
   scanners `continue` past one. ✅ **No AppleDouble sidecar reaches the parser after [I-0221]'s fix** —
   ⚠️ **but whether one malformed REAL scene should deny access to a whole manuscript is a product
   question, not an oversight to patch.**
2. ⛔ **The core's parse errors name no file.** ⚠️ **`[json.exception.parse_error.101] … empty input`
   with no path is undiagnosable from the UI.** ✅ **Worth doing properly rather than incidentally.**

---

✅ **CLOSED 2026-09-15 (all user-approved):**
✅ **[SP-129]** → [`Closed/Sprint-SP-129.md`](Closed/Sprint-SP-129.md) — the unbuilt surfaces + the `loadImportedTimelines` bypass.
✅ **[SP-131]** → [`Closed/Sprint-SP-131.md`](Closed/Sprint-SP-131.md) — the indexes; `~300 s` → `1.06 s`.
✅ **[SP-132]** → [`Closed/Sprint-SP-132.md`](Closed/Sprint-SP-132.md) — ⚠️ **PARTIAL; T-0519/T-0520 reverted.**
✅ **[SP-133]** → [`Closed/Sprint-SP-133.md`](Closed/Sprint-SP-133.md) — TextKit 2; all tasks verified.

⚠️ **NEXT-AVAILABLE IDs ARE NO LONGER RECORDED IN ANY TRACKING DOCUMENT (user ruling 2026-09-24).**
✅ **They live in [`../tools/next-ids.json`](../tools/next-ids.json), allocated by
[`../tools/next-id.py`](../tools/next-id.py)** — ⚠️ **`python3 docs/tools/next-id.py --peek` to look,
`… next-id.py task` to allocate.**
⛔ **THE FIGURE WAS RESTATED AND KEPT GOING STALE:** ⚠️ **this line read `SP-145 · T-0539` while SP-149
and T-0548 already existed (corrected 2026-09-23), and `SP-134 · T-0535` while SP-143 and T-0537 did
(corrected 2026-09-18).** ✅ **A counter the reader does not need, and the writer kept getting wrong.**

⚠️ **WHERE THINGS STAND (2026-09-18):** ✅ **Project open is DONE and measured: `~300 s` → `0.34 s` on
1,174 scenes, and the app is USABLE.** ⚠️ **WHAT REMAINS IS SHAPE, NOT SPEED** — ✅ **the window has no
`NSToolbar`, the bars are `VStack` siblings, and the Inspector hand-rolls a column.**
⚠️ **FOUR DEFECTS WERE CARRIED INTO [EP-040], NOT FIXED: [I-0203]** (chrome vanishes behind the banner),
**[I-0205]** (the banner itself), **[I-0206]** (offset-linear `setSel`, `57–81 ms` near the document end
— ✅ **CLOSED 2026-09-25 as NOT-A-DEFECT**),
**[I-0213]** (⚠️ *chapter create, originally a 2.7 s freeze, now `~305–400 ms` of work*).
✅ **[I-0213] IS NOW VERIFIED (2026-09-24) and ARCHIVED** →
[`../Issues/Verified/Issue-verified-0211-0220.md`](../Issues/Verified/Issue-verified-0211-0220.md);
✅ **[I-0203] and [I-0205] are VERIFIED (2026-09-24) and ARCHIVED; ✅ [I-0206] is CLOSED (2026-09-25).**
⛔ **NONE OF THE FOUR REMAIN OPEN.**
