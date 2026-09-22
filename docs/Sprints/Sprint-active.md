# Active Sprints

⛔ **NO SPRINT IS ACTIVE.** ✅ **[EP-041] CLOSED 2026-09-22 (user-approved)** →
[`../Epics/Closed/Epic-EP-041.md`](../Epics/Closed/Epic-EP-041.md) — ⚠️ **all five of its Sprints,
six Tasks and [I-0197] closed with it.**
⚠️ **ONE ACTIVE EPIC REMAINS: 🟡 [EP-040]** `[Apple]` **The Editor Shell** — ✅ **[SP-134] is
unblocked and LOW risk.**

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

✅ **[SP-142] IS ACTIVE (above).** ✅ **[SP-149] is PLANNED, fully ruled and blocks on nothing.**
⚠️ **[SP-143] queues behind [SP-142]'s LIVE PASS and [SP-149]'s guard** — ⚠️ **rescoped 2026-09-21: it
WITNESSES the guard (red-before / green-after), ⛔ it no longer builds one.**

✅ **[SP-140] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-140.md`](Closed/Sprint-SP-140.md)
— ⚠️ **planned, implemented, verified and closed in ONE DAY.** ✅ **T-0536 and [I-0215] both Verified.**
⚠️ **It also produced [I-0223]**, ✅ **filed unassigned.**

✅ **[SP-130] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-130.md`](Closed/Sprint-SP-130.md).

---

## ⚠️ Two Epics are active

✅ **[EP-041]** — `[Cross]` **The Boundary** — **CLOSED 2026-09-22**; ✅ **all five Sprints closed.**
⚠️ **[SP-142] is PLANNED and UNBLOCKED; [SP-143] queues behind it.**
🟡 **[EP-040]** — `[Apple]` **The Editor Shell** — ✅ **[SP-134] is unblocked and LOW risk.**

✅ **[SP-144] CLOSED 2026-09-20 (user-approved)** → [`Closed/Sprint-SP-144.md`](Closed/Sprint-SP-144.md).
✅ **All 7 ACs met; SIX Issues VERIFIED on the real rig under `cache=none`.**
✅ **[EP-042] CLOSED with it** → [`../Epics/Closed/Epic-EP-042.md`](../Epics/Closed/Epic-EP-042.md).

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

⚠️ **[SP-142] must honour BOTH** — ✅ **its plan already does.**

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

⛔ **[I-0197] IS NOT CLOSED.** ✅ **Class C only.** ⚠️ **Classes A and B continue in Track 2 below.**

---

## 🔵 What is available to activate next

⚠️ **TWO INDEPENDENT TRACKS. ⛔ Claude does not choose between them — the user activates.**

### ✅ Track 1 — the Epic's actual subject (the editor shell)

| Sprint | Step | Title | ⚠️ Risk |
| ------ | ---- | ----- | ------ |
| ✅ **[SP-134]** | **S1+S2** | ✅ **The toolbar** — `NSToolbar` + title/subtitle + existing verbs | ✅ **LOW** |
| **[SP-135]** | **S3** | ✅ **The bars** — `safeAreaBar` conversion; ⚠️ **closes [I-0203]** | ✅ **MEDIUM** |
| **[SP-136]** | **S4** | ⚠️ **The Inspector as a real column** (`.inspector`) | ⚠️ **MED-HIGH** |
| **[SP-137]** | **S6** | ⚠️ **The Object Detail Sheet** — ⚠️ **hosting RULING first, then chrome** | ⚠️ **MED-HIGH** |
| ⛔ **[SP-138]** | **S5** | ⛔ **`NSSplitViewController` rebuild** | ⛔ **HIGH — RECORDED, NOT SCHEDULED** |

✅ **[SP-134] is the natural next Sprint**: ⛔ **it blocks on nothing**, ✅ **it is LOW risk**, and
⚠️ **it is mostly a SURFACING job** — ✅ **the verbs already exist in the menu bar as callable closures**
(`Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`). ⚠️ **S1 and S2 are ONE Sprint deliberately** —
⛔ **splitting them ships an EMPTY TOOLBAR as a milestone.**

### ⚠️ Track 2 — ✅ **NOW [EP-041]** — the [I-0197] bypass chain (boundary work)

| Sprint | Task | Title | ⛔ Blocks on |
| ------ | ---- | ----- | ----------- |
| ✅ **[SP-140]** | **T-0536** | ✅ **[I-0215] CLOSED — Apple's layout round trip is lossless** | ✅ **CLOSED 2026-09-18** |
| ✅ **[SP-141]** | **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** | ✅ **CLOSED 2026-09-21** — [record](Closed/Sprint-SP-141.md) |
| ✅ **[SP-142]** | **T-0537** · **T-0542** | ✅ **CLOSED 2026-09-21** — [record](Closed/Sprint-SP-142.md) | ✅ **done** |
| 🟢 **[SP-149]** | **T-0541** | ✅ **The guard MECHANISM — BUILT; all 7 ACs met** | ✅ **was unblocked** |
| 🔵 **[SP-143]** | **T-0510** | ⚠️ **WITNESS the guard across the retirement — closes [I-0197]** | ⛔ **[SP-142]'s LIVE PASS + [SP-149]** |

✅ **[SP-140] CLOSED 2026-09-18.** ✅ **[SP-141] CLOSED 2026-09-21 (user-approved)** — ✅ **T-0507
Verified by live pass; [EP-041] AC2 MET.** ✅ **[SP-142] IS UNBLOCKED AND PLANNED**;
⚠️ **[SP-143] still queues behind [SP-142]** — ✅ **its guard must see Linux's duplicate PRESENT before
and ABSENT after, so it cannot run first.**

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

✅ **Next available Sprint ID: SP-145 · next available Task: T-0539.**
⚠️ **Corrected 2026-09-18** — ⛔ **this line read `SP-134 · T-0535` while SP-143 and T-0537 already existed.**

⚠️ **WHERE THINGS STAND (2026-09-18):** ✅ **Project open is DONE and measured: `~300 s` → `0.34 s` on
1,174 scenes, and the app is USABLE.** ⚠️ **WHAT REMAINS IS SHAPE, NOT SPEED** — ✅ **the window has no
`NSToolbar`, the bars are `VStack` siblings, and the Inspector hand-rolls a column.**
⚠️ **FOUR DEFECTS ARE CARRIED INTO [EP-040], NOT FIXED: [I-0203]** (chrome vanishes behind the banner),
**[I-0205]** (the banner itself), **[I-0206]** (offset-linear `setSel`, `57–81 ms` near the document end),
**[I-0213]** (chapter create `~305 ms`).
