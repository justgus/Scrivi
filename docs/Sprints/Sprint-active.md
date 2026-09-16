# Active Sprints

## SP-130 — `[Apple]` Rule the asset presence-checks (Class C of [I-0197])

🟡 **ACTIVE — activated 2026-09-15.** ✅ **Record: [`Sprint-SP-130.md`](Sprint-SP-130.md). Task: T-0508.**
✅ **T-0508 IMPLEMENTED 2026-09-16 — ⛔ NOT VERIFIED.** ⚠️ **The Sprint CANNOT close until the user
verifies it** (`feedback_verification`). ✅ **Ruling: [`../Scrivi_Asset_Presence_Check_Ruling_v0_1.md`](../Scrivi_Asset_Presence_Check_Ruling_v0_1.md).**
⚠️ **ITS OWN PREMISE DID NOT HOLD:** ⚠️ **this was planned as "a RULING to record, not a defect to
fix"** — ⛔ **and one of the five sites (`ExistingAssetPicker.swift:130`) was a REAL blocking-stat
defect**, ✅ **found precisely BY writing the ruling, which split "degrades gracefully" (architecture)
from "blocks the main actor" (cost).** ⚠️ **The 2026-09-15 re-measurement cleared it on the first
question alone.** ✅ **`xcodebuild` green ×3 schemes; 127 interop tests pass** — ⛔ **neither exercises
the defect; ✅ a LIVE rig pass is what verifies it.**
⚠️ **RESTRUCTURED 2026-09-15 (user-ruled): SP-130 was ONE sprint of four tasks; ⚠️ it is now FIVE.**
✅ **Rationale: [`Sprint-SP-130-RESTRUCTURE.md`](Sprint-SP-130-RESTRUCTURE.md).**
⛔ **T-0509 STRUCK — its remainder is provably ZERO** (10 hits partition exactly into T-0507's 3,
T-0508's 5, and 2 recorded non-violations).

✅ **THE OTHER FOUR ARE PLANNED, NOT ACTIVE:**
🔵 **[SP-140]** — [I-0215] unknown-key drop (**T-0536**) — ⛔ **blocks on NOTHING; ✅ can run parallel to SP-130.**
🔵 **[SP-141]** — core endpoints + Apple adoption (**T-0507**) — ⛔ **BLOCKED ON AN ENDPOINT-SHAPE RULING.**
🔵 **[SP-142]** — retire Linux's duplicate (**T-0537**) — ⛔ **blocks on SP-141.**
🔵 **[SP-143]** — the regression guard (**T-0510**) — ⛔ **blocks on SP-141 AND SP-142.**

🟡 **[EP-040] is the ACTIVE Epic** — `[Apple]` **The Editor Shell** (activated 2026-09-15).
✅ **Its next Sprints, per its own Sprint table:** ⚠️ **[SP-129]** (the four unbuilt Apple surfaces +
the `loadImportedTimelines` bypass) — ✅ **user-ruled NEXT UP** — then ⚠️ **[SP-130]** (close the
ScriviCore bypasses, [I-0197]), then ✅ **SP-134** (S1+S2, the toolbar).
⚠️ **SP-129/SP-130 need ACTIVATING; ✅ their planning detail is in [`Sprint-backlog.md`](Sprint-backlog.md).**

✅ **CLOSED 2026-09-15 (all user-approved):**
✅ **[SP-131]** → [`Closed/Sprint-SP-131.md`](Closed/Sprint-SP-131.md) — the indexes; `~300 s` → `1.06 s`.
✅ **[SP-132]** → [`Closed/Sprint-SP-132.md`](Closed/Sprint-SP-132.md) — ⚠️ **PARTIAL; T-0519/T-0520 reverted.**
✅ **[SP-133]** → [`Closed/Sprint-SP-133.md`](Closed/Sprint-SP-133.md) — TextKit 2; all tasks verified.

✅ **Next available Sprint ID: SP-134 · next available Task: T-0535.**

⚠️ **WHERE THINGS STAND (2026-09-15):** ✅ **Project open is DONE and measured: `~300 s` → `0.34 s` on
1,174 scenes, and the app is USABLE.** ⚠️ **WHAT REMAINS IS SHAPE, NOT SPEED** — ✅ **the window has no
`NSToolbar`, the bars are `VStack` siblings, and the Inspector hand-rolls a column.**
⚠️ **FOUR DEFECTS ARE CARRIED INTO [EP-040], NOT FIXED: [I-0203]** (chrome vanishes behind the banner),
**[I-0205]** (the banner itself), **[I-0206]** (offset-linear `setSel`, `57–81 ms` near the document end),
**[I-0213]** (chapter create `~305 ms`).
