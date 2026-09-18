# Active Sprints

⛔ **NO SPRINT IS ACTIVE.**

✅ **[SP-140] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-140.md`](Closed/Sprint-SP-140.md)
— ⚠️ **planned, implemented, verified and closed in ONE DAY.** ✅ **T-0536 and [I-0215] both Verified.**
⚠️ **It also produced [I-0223]**, ✅ **filed unassigned.**

✅ **[SP-130] CLOSED 2026-09-18 (user-approved)** → [`Closed/Sprint-SP-130.md`](Closed/Sprint-SP-130.md).

---

## ⚠️ Two Epics are now active

🟡 **[EP-041]** — `[Cross]` **The Boundary** — ⛔ **its only unblocked Sprint is DONE.**
⚠️ **[SP-141] CANNOT be activated without an endpoint-shape ruling; SP-142/143 queue behind it.**
🟡 **[EP-040]** — `[Apple]` **The Editor Shell** — ✅ **[SP-134] is unblocked and LOW risk.**

⛔ **SO THE DOCKET HAS EXACTLY ONE ACTIVATABLE SPRINT ([SP-134]) AND ONE PENDING RULING ([SP-141]).**

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
| **[SP-141]** | **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** | ⛔ **an ENDPOINT-SHAPE ruling** |
| **[SP-142]** | **T-0537** | ⚠️ **Retire Linux's duplicate `InspectorLayoutStore.cpp`** | ⛔ **SP-141** |
| **[SP-143]** | **T-0510** | ⚠️ **The regression guard — closes [I-0197]** | ⛔ **SP-141 AND SP-142** |

✅ **[SP-140] CLOSED 2026-09-18** — ⚠️ **it was this track's only unblocked Sprint.**
⛔ **[SP-141] CANNOT BE ACTIVATED AS-IS: it needs an ENDPOINT-SHAPE RULING first** — ✅ **the three
options and the constraint SP-140 puts on them are written up in
[`../Epics/Epic-EP-041.md`](../Epics/Epic-EP-041.md).** ⚠️ **SP-142 and SP-143 both queue behind it.**

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

✅ **Next available Sprint ID: SP-134 · next available Task: T-0535.**

⚠️ **WHERE THINGS STAND (2026-09-18):** ✅ **Project open is DONE and measured: `~300 s` → `0.34 s` on
1,174 scenes, and the app is USABLE.** ⚠️ **WHAT REMAINS IS SHAPE, NOT SPEED** — ✅ **the window has no
`NSToolbar`, the bars are `VStack` siblings, and the Inspector hand-rolls a column.**
⚠️ **FOUR DEFECTS ARE CARRIED INTO [EP-040], NOT FIXED: [I-0203]** (chrome vanishes behind the banner),
**[I-0205]** (the banner itself), **[I-0206]** (offset-linear `setSel`, `57–81 ms` near the document end),
**[I-0213]** (chapter create `~305 ms`).
