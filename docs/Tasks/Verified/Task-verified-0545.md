# Verified Task: T-0545

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0545** | ✅ **The bars** — `safeAreaBar` conversion (S3) | [SP-135] | [EP-040] | **2026-09-22** |

✅ **VERIFIED by the user's live pass 2026-09-22.** ✅ **All EIGHT of [SP-135]'s ACs met.**

---

## ⚠️ The defect, and why it looked like four

⛔ **THE WORLD-WARNING BANNER, THE TIMELINE AND THE INSPECTOR'S TAB BAR WERE ALL `VStack` SIBLINGS.**
⚠️ **A sibling TAKES vertical space from the stack it joins** — ✅ **so the banner appearing pushed on
everything else in that stack simultaneously.**

⚠️ **THAT IS WHY [I-0203] READ AS FOUR BUGS** (Nav Bar, Inspector tab bar, Timeline and a stray title
panel all vanishing at once) ✅ **rather than one structural mistake.**

✅ **A `safeAreaBar` INSETS the content instead of displacing its neighbours.**

## ✅ What shipped

| Bar | Placement |
| --- | --------- |
| **World warning** | ⚠️ **OUTERMOST bottom bar** |
| **Timeline strip** | inner bottom bar, nearer the content |
| **Inspector tab bar** | bottom bar within the inspector |

⚠️ **THE BANNER BELOW THE TIMELINE IS A CHANGE, NOT A TRANSCRIPTION** (user ruling Q2). ⛔ **The old
code read *"Sits directly under the manuscript and ABOVE the Timeline"*** — ✅ **and that comment was
REWRITTEN, not left to rot.** ⚠️ **A warning is TRANSIENT and the Timeline is FURNITURE: the transient
bar outermost means its appearance disturbs the FEWEST stable surfaces.**

✅ **ALL THREE CONVERTED (user ruling Q3), including the Inspector's** — ⚠️ **even though [SP-136]
rebuilds that pane and may touch it again.** ⛔ **A partial conversion would have made AC2
unfalsifiable: *"nothing else moves"* cannot be proven while any bar is still a sibling.**

## ✅ How it was verified — ⚠️ **both halves, checked separately**

✅ **APPEARING:** *"The banners all appeared in the correct places and did not deform the UI."*
✅ **DISMISSING:** *"I closed the banner and only the timeline adjusted its size."*

⚠️ **THE SECOND RESULT IS THE PROOF, NOT A CAVEAT.** ✅ **The Timeline is the INNER bar, so it
reclaiming the freed inset is exactly correct** — ⛔ **and NOTHING above it moved: Navigator, Inspector
tab bar and toolbar all held.** ✅ **Under the old `VStack` that was impossible.**
✅ **It also validates the Q2 ruling: outermost-transient disturbs ONE piece of furniture, not several.**

⚠️ **CHECKED SEPARATELY ON PURPOSE** — ✅ **adding an inset and removing one are different code paths**
(`feedback_verify_each_half_separately`, which [I-0132] paid for).

**Also verified:** ✅ **AC4** (Timeline toggled with the banner up — ⛔ no coupling; SP-102 R1's
deliberate independence survives) · ✅ **AC5** (tabs select AND the selection persists across launch —
⚠️ **the persistence half round-trips through `inspector-layout.json` via [SP-141]'s core endpoints**) ·
✅ **AC6** (verified from the DIFF: `ManuscriptTextView` appears ZERO times in commit `8586582`).

✅ **Build clean · `TEST SUCCEEDED` 132/132 · `ctest` 626/626 · boundary guard clean.**

## ✅ It closed a two-year-old ambiguity as a side effect

⚠️ **[I-0205] had sat UNDIAGNOSED since 2026-09-14**, ✅ **with its own record forbidding anyone from
citing code they had not read.** ✅ **Reading it answered (a) vs (b): the banner is CORRECT, not false**
(`WorldWarningView.swift:128`) — ⛔ **the DATA-facing defect that record feared does not exist.**
✅ **User ruled KEEP IT; ✅ and after the layout fix: *"The banner's look is perfect."***
⚠️ **The diagnosis was written into the ISSUE, not just this Sprint** — ⛔ **a finding that lives only
in a Sprint record is lost when the Sprint closes.**

---

*Verified 2026-09-22 by the user's live pass. [I-0203] and [I-0205] both resolved; [EP-040] AC4/AC5 met.*
