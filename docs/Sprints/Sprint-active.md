# Active Sprints

🟠 **[SP-141] COMPLETE 2026-09-21 — ⛔ AWAITING USER APPROVAL TO CLOSE** (activated 2026-09-18) under
🟡 **[EP-041]**. ⚠️ **NO SPRINT IS CURRENTLY ACTIVE.**
✅ **Its ONLY Task is Verified and archived** → [`../Tasks/Verified/Task-verified-0507.md`](../Tasks/Verified/Task-verified-0507.md).
⚠️ **Claude cannot close a Sprint** — ✅ **this one is ready when you are.**
✅ **[EP-041] AC2 is MET. ✅ [SP-142] is UNBLOCKED.**

⚠️ **Prior status line, kept for the record:** 🟢 **IMPLEMENTED 2026-09-21.** ✅ **Both endpoints ship; Apple is converted;
`InspectorLayoutStore.swift` has ZERO direct file I/O left.** ✅ **`ctest` 621/621 (was 613 — 8 new ABI
tests), `xcodebuild test` 132/132 in 12 suites.** ⚠️ **The 8 new tests were PROVEN TO FAIL against an
injected [I-0215]-shaped defect** — ✅ **a green test that cannot fail is not evidence.**
✅ **A SECOND RULING WAS MADE (user, 2026-09-21): ABSENCE SEMANTICS — "core reports, app decides"**
(`status` = `ok` | `absent` | `unreadable`). ✅ **[SP-142] IS NOW UNBLOCKED.**
⛔ **A LIVE PASS IS STILL OWED** — ⚠️ **switch tabs, quit, relaunch, confirm the layout returns.**

⚠️ **PRIOR STATUS, kept because it is what made the plan for [SP-142] worth writing:**
⛔ **NOT STARTED AS OF 2026-09-21 (morning).** ✅ **Verified three ways:** **T-0507 is 🔵 NOT STARTED**
(`../Tasks/Task-active.md:52`); ⛔ **no `inspector_layout` endpoint exists in `scrivi.h`**; and
⚠️ **`Scrivi/App/InspectorLayoutStore.swift` STILL does its own file I/O** (`:175`, `:192`,
`:368-370`) — ✅ **which is the [I-0197] bypass T-0507 exists to remove.**
⚠️ **Nothing has been implemented in [EP-041] since [SP-140] closed on 2026-09-18.**
✅ **[SP-142] was PLANNED 2026-09-21** → [`Sprint-SP-142.md`](Sprint-SP-142.md); ⚠️ **it owes [SP-141]
a question (Q1: what the core returns for a MISSING or CORRUPT layout document), ⛔ which must be
answered BEFORE T-0507 implements, not after.**
✅ **ITS BLOCKING RULING WAS MADE THE SAME DAY (user, 2026-09-18): the `inspector-layout.json`
endpoints take the OPAQUE DOCUMENT GET/PUT shape.** ⚠️ **See the ruling record below.**

✅ **[SP-140] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-140.md`](Closed/Sprint-SP-140.md)
— ⚠️ **planned, implemented, verified and closed in ONE DAY.** ✅ **T-0536 and [I-0215] both Verified.**
⚠️ **It also produced [I-0223]**, ✅ **filed unassigned.**

✅ **[SP-130] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-130.md`](Closed/Sprint-SP-130.md).

---

## ⚠️ Two Epics are now active

🟡 **[EP-041]** — `[Cross]` **The Boundary** — ⛔ **its only unblocked Sprint is DONE.**
⚠️ **[SP-141] CANNOT be activated without an endpoint-shape ruling; SP-142/143 queue behind it.**
🟡 **[EP-040]** — `[Apple]` **The Editor Shell** — ✅ **[SP-134] is unblocked and LOW risk.**

✅ **THE [SP-141] RULING IS MADE AND [SP-141] IS NOW ACTIVE.** ✅ **[SP-134] remains activatable.**
✅ **[SP-144] CLOSED 2026-09-20 (user-approved)** → [`Closed/Sprint-SP-144.md`](Closed/Sprint-SP-144.md).
✅ **All 7 ACs met; SIX Issues VERIFIED on the real rig under `cache=none`.**
✅ **[EP-042] CLOSED with it** → [`../Epics/Closed/Epic-EP-042.md`](../Epics/Closed/Epic-EP-042.md).

---

## ✅ THE [SP-141] ENDPOINT-SHAPE RULING — user, 2026-09-18

✅ **RULED: ONE OPAQUE DOCUMENT GET/PUT.** ⛔ **NOT typed-per-property; ⛔ NOT the hybrid.**

⚠️ **WHAT THIS DECIDES:** ✅ **the CORE owns atomicity, durability and repair of
`inspector-layout.json`; ✅ the APP owns its MEANING.** ⛔ **The core does NOT validate the document's
interior and does NOT know what a card kind is.**

✅ **WHY THIS SHAPE** — ⚠️ **it is the one the [SP-140] lossless rule does not fight.** ✅ **Keys the
core does not understand survive BY CONSTRUCTION**, ⚠️ **because the core never interprets them at
all** — ⛔ **whereas typed-per-property would put the inspector's SCHEMA in the core and make every new
card kind a core change.** ⚠️ **THE ACCEPTED COST, stated so it is not rediscovered as a defect: the
core CANNOT validate what it stores here.** ✅ **A malformed layout is the app's to detect.**

⚠️ **[SP-142] and [SP-143] are UNBLOCKED by this ruling** (they queued on [SP-141] only).

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
| 🟠 **[SP-141]** | **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** | 🟠 **COMPLETE 2026-09-21 — ✅ T-0507 VERIFIED; ⛔ awaiting close approval** |
| 🔵 **[SP-142]** | **T-0537** | ⚠️ **Retire Linux's duplicate `InspectorLayoutStore.cpp`** | ✅ **UNBLOCKED 2026-09-21** — [plan](Sprint-SP-142.md) |
| **[SP-143]** | **T-0510** | ⚠️ **The regression guard — closes [I-0197]** | ⛔ **SP-141 AND SP-142** |

✅ **[SP-140] CLOSED 2026-09-18.** 🟠 **[SP-141] IS COMPLETE 2026-09-21 — ✅ T-0507 user-verified by
live pass; ⛔ awaiting close approval.** ✅ **[SP-142] IS UNBLOCKED AND PLANNED**;
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
