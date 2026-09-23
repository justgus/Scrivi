---
sprint: SP-135
epic: EP-040
status: CLOSED 2026-09-22 (user-approved)
activated: 2026-09-22
closed: 2026-09-22
task: T-0545
planned: 2026-09-22
platform: Apple
---

# Closed Sprint — SP-135 — ✅ **The bars** — `safeAreaBar`, and the end of [I-0203]

**Status:** ✅ **CLOSED 2026-09-22 — user-approved.** ✅ **All EIGHT ACs met, live-pass verified.**
✅ **T-0545 VERIFIED** → [`../../Tasks/Verified/Task-verified-0545.md`](../../Tasks/Verified/Task-verified-0545.md).
✅ **[I-0203] and [I-0205] both RESOLVED; ✅ [EP-040] AC4 and AC5 MET.**
✅ **Q1–Q3 ruled before activation; ⛔ the [I-0205] gate is DISCHARGED.**
**Epic:** 🟡 [EP-040](../../Epics/Epic-active.md) — `[Apple]` **The Editor Shell**
**Task:** **T-0545** — ✅ **ISSUED 2026-09-22 at activation.**
**Step:** **S3** of the app-shape study §4.3 · **conformance F3**
**Serves:** **[EP-040] AC4, AC5, AC9** · closes **[I-0203]** · answers **[I-0205]**
**Blocks on:** ✅ **NOTHING — the [I-0205] gate was discharged by the Q1 ruling 2026-09-22.**
**Design:** ✅ [`../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`](../../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md) §4.3 ·
[`../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md`](../../Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md) F3

---

## ✅ Goal

⚠️ **A bar appearing must not displace anything else.** ✅ **The world-warning banner, the Timeline
strip and the Scene Inspector's tab bar stop being `VStack` siblings of the content and become
`safeAreaBar`s** — ⚠️ **so showing one INSETS the manuscript rather than SHOVING the window's chrome
out of the way.**

✅ **THIS IS [I-0203]'s STRUCTURAL FIX.** ⚠️ **[SP-134] gave the writer an affordance to bring a lost
pane back; ⛔ it did not stop the banner from taking the chrome away in the first place.**

---

## ⚠️ What is actually there — measured 2026-09-22

| Bar | Where | Shape today |
| --- | ----- | ----------- |
| **World warning** | `EditorView.swift:311` | ⛔ **a `VStack` sibling of `ManuscriptTextView`** |
| **Timeline strip** | `EditorView.swift:321`, `:353` | ⛔ **same `VStack`** |
| **Inspector tab bar** | `SceneInspectorView.swift:50` | ⛔ **a `VStack` sibling inside the inspector** |

✅ **`safeAreaBar` EXISTS AND COMPILES on the macOS 27 SDK** — ⚠️ **verified by `swiftc`, not assumed
from documentation** (⛔ **the discipline the [SP-134] blank-symbol defect should have had**).

⚠️ **THE VSTACK IS WHY [I-0203] LOOKS LIKE FOUR BUGS.** ✅ **When the banner appears it takes vertical
space from a stack the manuscript, the timeline and the chrome all share** — ⛔ **so four things move
at once and it reads as four defects rather than one structural mistake.**

---

## ⛔ [I-0205] GATES THIS SPRINT — ⚠️ **and the evidence is now in**

⚠️ **[EP-040] AC9 requires [I-0205] ANSWERED BEFORE THE BANNER IS RESTYLED**, ✅ **or a false banner
gets entrenched as a design.** ⚠️ **The Issue says plainly: *"NOT DIAGNOSED… no code has been read for
this record and none should be cited until it has."***

✅ **THE CODE HAS NOW BEEN READ (2026-09-22), and it answers (a) vs (b):**

```
rows = worlds
    .filter { $0.worldStatus.isUnavailable }      // WorldWarningView.swift:128
```

✅ **THE BANNER IS NOT FALSE.** ⚠️ **It reports EVERY unavailable BOUND world, and that is
DELIBERATE** — the code says so in its own comment:

> ⚠️ *"Report every unavailable BOUND world, not merely those with pending edges. A world that is away
> with nothing yet linked is still something the writer needs to know about — she cannot add to it,
> and silence would read as 'everything is fine'."*

⛔ **SO [I-0205](a) — "correct but ugly" — IS THE ANSWER, NOT (b).** ✅ **No availability check is
wrong; ⚠️ the data-facing defect the Issue feared does not exist.**

⚠️ **BUT THAT DOES NOT SETTLE WHETHER THE BANNER SHOULD SHOW.** ✅ **`project_unlinked_world_objects_are_normal`
records that an unavailable world is a NORMAL condition — a shared world on an unmounted volume is
expected, not broken.** ⚠️ **The code's justification (*"silence would read as 'everything is fine'"*)
is a real argument; ⛔ so is the user's (*"a writer is being punished with a broken window for a normal
condition"*).** ✅ **That is a PRODUCT decision and it is Q1.**

---

## Acceptance Criteria

- [x] **AC1** — ✅ **MET, live-pass VERIFIED 2026-09-22.** ✅ **The three bars are `safeAreaBar`s** (or split-item accessories), ⛔ **not `VStack`
      siblings.** ✅ **Closes conformance F3; ✅ [EP-040] AC4.**
- [x] **AC2** — ✅ **MET, live-pass VERIFIED 2026-09-22 — BOTH HALVES, CHECKED SEPARATELY.**
      ✅ **APPEARING:** *"The banners all appeared in the correct places and did not deform the UI."*
      ✅ **DISMISSING:** *"I closed the banner and only the timeline adjusted its size."*
      ⚠️ **THAT DETAIL IS THE PROOF, not a caveat.** ✅ **The Timeline is the INNER bar and the banner
      the OUTER one, so the Timeline reclaiming the freed inset is exactly correct** — ⛔ **and NOTHING
      above it moved: Navigator, Inspector tab bar and toolbar all held.**
      ⛔ **Under the old `VStack` that was impossible** — ⚠️ **every sibling shared one vertical budget,
      so removing the banner pushed on all of them.** ✅ **That is [I-0203], and it is gone.**
      ✅ **It also validates the Q2 ruling: the transient bar outermost means its removal disturbs ONE
      piece of furniture instead of several.**
      ⚠️ **Original text:** ⚠️ **SHOWING OR DISMISSING THE BANNER DISTURBS NOTHING ELSE.** ✅ **This is [I-0203]'s
      acceptance test and [EP-040] AC5** — ⚠️ **and it is the Sprint's headline outcome.**
      ✅ **PROVE IT BY DOING IT: with the banner up, the Navigator, the Inspector tab bar, the Timeline
      and the toolbar are ALL still present and unmoved.**
- [x] **AC3** — ✅ **MET 2026-09-22 — [I-0205]'s row now carries the diagnosis, the evidence and the ruling.** ✅ **[I-0205] IS ANSWERED IN THE RECORD** (⚠️ **not merely in this plan**), ✅ **and the
      banner's behaviour matches the Q1 ruling.** ⚠️ **[EP-040] AC9.**
- [x] **AC4** — ✅ **MET — user toggled the Timeline with the banner up; ⛔ no coupling.** ⛔ **THE TIMELINE'S OWN VISIBILITY IS UNCHANGED.** ⚠️ **SP-102 R1 ruled the warning
      strip has its OWN toggle, deliberately NOT tied to the Timeline** — ✅ **converting both to bars
      must not quietly couple them.**
- [x] **AC5** — ✅ **MET, live-pass VERIFIED 2026-09-22:** *"I verified the Inspector's tabs all select and the selection persists across launch."* ⚠️ **The PERSISTENCE half matters most — the tab bar moved into a `safeAreaBar`, and its selection round-trips through `inspector-layout.json` via [SP-141]'s core endpoints.** ✅ **The Inspector's tab bar still selects tabs, and its selection still persists at
      PROJECT level** (⚠️ **not per scene — the [SP-141] ruling**).
- [x] **AC6** — ✅ **MET — verified from the DIFF, not asserted:** ⛔ **`ManuscriptTextView` appears ZERO times in the implementation commit (`8586582`)**, ✅ **and `SceneInspectorView` still opens with `HStack(spacing: 0)`.** ⛔ **NOTHING ELSE MOVES.** ⚠️ **The Inspector is still an `HStack` member until
      [SP-136]; ⛔ `ManuscriptTextView`'s internals are untouched** (app-shape §4.3).
- [x] **AC7** — ✅ **MET — build clean, `TEST SUCCEEDED` 132/132, `ctest` 626/626.** ✅ **`xcodebuild` build + test clean on macOS.** ⚠️ **iOS/visionOS remain DEFERRED**
      (user ruling 2026-09-22 — ⛔ **the app cannot RUN there**), ✅ **but this Sprint touches
      `EditorView`'s shared body and must not break their compile if it is ever restored.**
- [x] **AC8** — ✅ **MET 2026-09-22 — the user forced a world unavailable and exercised both halves.** ⚠️ **A LIVE PASS.** ✅ **Force a world unavailable (unmount the test volume), confirm
      the banner appears, confirm NOTHING else moves, dismiss it, confirm nothing moves back.**
      ⚠️ **`project_test_rig_tintagael_eskandar` is the rig; ⛔ back up first — it is real work.**

---

## ✅ THE THREE RULINGS — user, 2026-09-22

---

### ✅ Q1 — RULED: **KEEP THE BANNER.** ⛔ **Fix the layout, not the signal.**

✅ **[I-0205](a) CONFIRMED FROM CODE: the banner is CORRECT, not false.** ⚠️ **No availability check is
wrong** — ⛔ **the data-facing defect the Issue feared does not exist.**

✅ **It reports every unavailable BOUND world, deliberately** (`WorldWarningView.swift:128`), ⚠️ **and
the code states why: *"A world that is away with nothing yet linked is still something the writer
needs to know about — she cannot add to it, and silence would read as 'everything is fine'."***

⛔ **NOT demoted to a badge; ⛔ NOT limited to worlds with pending edges.** ✅ **Once the bar stops
displacing chrome, the complaint that remains is layout, and AC2 fixes that.**
⚠️ **The trade, recorded: an unavailable world IS a normal condition**
(`project_unlinked_world_objects_are_normal`) — ✅ **so this can be revisited from the fixed layout.**
⛔ **A warning that was never shown cannot be.**

---

### ✅ Q2 — RULED: **THE BANNER SITS BELOW THE TIMELINE**

⚠️ **THIS IS A CHANGE, NOT A TRANSCRIPTION OF TODAY'S ORDER.** ⛔ **Today the banner is ABOVE the
timeline** — *"Sits directly under the manuscript and ABOVE the Timeline"* (`EditorView.swift:306-308`).
✅ **Ruled: the banner goes to the OUTERMOST bottom edge, below the Timeline.**

✅ **WHY IT FOLLOWS: a warning is transient and the Timeline is furniture.** ⚠️ **Putting the transient
thing outermost means its appearance moves the FEWEST stable surfaces** — ✅ **which is AC2's whole
subject.**
⚠️ **THE OLD COMMENT MUST BE REWRITTEN, not left** — ⛔ **it asserts the opposite order and its
reasoning (visible whether or not the Timeline shows) is satisfied by either arrangement.**

---

### ✅ Q3 — RULED: **CONVERT ALL THREE BARS**

✅ **The Inspector's tab bar converts HERE, not in [SP-136].**
⚠️ **Conformance F3 names all three**, ✅ **and AC2's *"nothing else moves"* is only provable if no bar
is still a `VStack` sibling** — ⛔ **one unconverted bar leaves the defect class alive and the
acceptance test weaker than it reads.**

⚠️ **ACCEPTED COST: [SP-136] rebuilds the Inspector as a real `.inspector` column and may touch this
again.** ✅ **Judged worth it** — ⛔ **a partial conversion is the thing that makes AC2 unfalsifiable.**

---


## Plan of work

| Step | Work | ⚠️ Note |
| ---- | ---- | ------ |
| ~~S0~~ | ✅ **DONE 2026-09-22 — all three ruled before activation** | ✅ **Gate discharged** |
| **S1** | ✅ Convert the world-warning banner to `safeAreaBar` | ⚠️ **AC1** |
| **S2** | ✅ Convert the Timeline strip — ⚠️ **banner goes BELOW it (Q2)** | ⚠️ **AC1/AC4** |
| **S3** | ✅ Convert the Inspector tab bar (per Q3) | ⚠️ **AC1/AC5** |
| **S4** | ✅ Record [I-0205]'s answer in the Issue | ⚠️ **AC3** |
| **S5** | ✅ `xcodebuild` build + test | ⚠️ **AC7** |
| **S6** | ⚠️ **Live pass with a real unmount** | ⚠️ **AC2/AC8 — the only thing that proves it** |

---

## ⚠️ Risks

- ⛔ **AC2 DECLARED GREEN WITHOUT FORCING THE CONDITION.** ⚠️ **The banner only appears when a world is
  genuinely unavailable** — ✅ **so the test REQUIRES an unmount.** ⛔ **A pass that never saw the banner
  proves nothing**, ⚠️ **and [I-0203] is precisely about what happens while it is up.**
- ⚠️ **Coupling the banner and the Timeline.** ✅ **SP-102 R1 gave them separate toggles ON PURPOSE**
  — ⛔ **two bars on the same edge invite a shared container.** ⚠️ **AC4 exists to catch that.**
- ⚠️ **`safeAreaBar` changing SCROLL behaviour.** ✅ **A safe-area inset is not a layout sibling** —
  ⚠️ **the manuscript's scroll extent and the [I-0235] scroll-fraction restore both interact with
  insets.** ⛔ **Confirm a restored scroll position still lands correctly with a bar up.**
- ⚠️ **Doing [SP-136]'s work early.** ⛔ **The Inspector stays an `HStack` member here** — ✅ **AC6.**
- ⚠️ **Answering [I-0205] in this plan and never in the Issue.** ⛔ **AC3 requires the RECORD updated**
  — ✅ **a diagnosis that lives only in a Sprint plan is lost when the Sprint closes.**

---

## ⛔ Out of scope

- ⛔ **The Inspector as a real `.inspector` column** — ✅ **[SP-136] / S4.**
- ⛔ **The Object Detail Sheet's hand-built chrome** — ✅ **[SP-137] / S6.**
- ⛔ **`NSSplitViewController`** — ✅ **[SP-138], recorded NOT scheduled.**
- ⛔ **Export and the text-size lens.** ⚠️ **Slots only; [SP-134] declared them.**
- ⛔ **Changing what makes a world unavailable.** ✅ **That is `[ScriviCore]`** — ⚠️ **[EP-044].**
- ⛔ **iOS / visionOS layout.** ✅ **Deferred to a future `[Apple]` Epic** (user ruling 2026-09-22).

---

## ⚠️ Estimate

✅ **MEDIUM — the study's own rating.** ⚠️ **Three conversions are mechanical; ⛔ the cost is Q1 (a
product ruling) and AC8 (a live pass that needs a REAL UNMOUNT).**
⚠️ **AC2 is the Sprint** — ✅ **everything else is in service of "showing a bar disturbs nothing".**


---

## ✅ Outcome

✅ **[I-0203] IS STRUCTURALLY FIXED.** ⚠️ **Its root cause — bars as `VStack` siblings — was read in the
hierarchy, exactly as that record demanded before anyone fixed it from the description.**
⛔ **One structural mistake had read as four bugs.**

✅ **[I-0205] IS FULLY RESOLVED, BOTH HALVES.** ⚠️ **It had sat UNDIAGNOSED for eight days with its own
record forbidding citation of unread code.** ✅ **Reading it settled (a) vs (b) — the banner is CORRECT,
not false — ✅ the user ruled to keep it, and after the layout fix: *"The banner's look is perfect."***

## ⚠️ Audit-check findings, ruled as part of this close

⚠️ **[I-0203] WAS STILL MARKED 🔵 OPEN at close time**, ✅ **with a caveat saying the root cause had not
been read in the code.** ⛔ **That caveat had been answered by this Sprint and the row never updated.**
✅ **Corrected in the same step** — ⚠️ **the same class as [I-0223], which was referenced in five
documents and present in none.**

✅ **THE PLAN'S OWN RISK LIST PROVED ACCURATE ON ONE POINT WORTH KEEPING:** ⚠️ **AC2 could not have
been verified without forcing the condition.** ⛔ **A pass that never saw the banner proves nothing**,
✅ **and the user's dismissal observation — *only the Timeline resized* — is the single most
informative sentence in the Sprint.**

⚠️ **ONE THING THIS SPRINT DELIBERATELY DID NOT DO:** ⛔ **no Linux counterpart.** ✅ **Now tracked as
[I-0244]** — ⚠️ **four Sprints of Apple shell work with no `[Linux]` record until 2026-09-22.**

---

*Closed 2026-09-22 with user approval. Eight ACs met; T-0545 Verified; [I-0203] and [I-0205] resolved.*
