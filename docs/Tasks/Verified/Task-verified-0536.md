# Verified Task: T-0536

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0536** | ⛔ **Make Apple's `inspector-layout.json` round trip LOSSLESS** ([I-0215]) | [SP-140] | [EP-041] | **2026-09-18** |

---

## What it was

⛔ **LIVE DATA LOSS.** ⚠️ **`InspectorLayoutDocument` was a fixed 6-property `Codable` struct**
(`InspectorLayoutStore.swift:63`), ⛔ **so any key Apple did not declare was discarded on decode and
never written back** — ⚠️ **while Linux deliberately keeps the whole document**
(`InspectorLayoutStore.cpp:97-100`: *"THE WHOLE DOCUMENT is kept … This is what makes the round trip
lossless"*). ⚠️ **The same projects are opened on both, so Apple silently deleted what Linux preserved.**

## The fix — three edits plus one the plan did not call for

| # | change |
| - | ------ |
| 1 | ✅ **`private var rawDocument: [String: Any]?`** — the file **as read**, unknown keys included |
| 2 | ✅ **Load captures it** after the typed decode, non-fatally |
| 3 | ✅ **`mergedForSave()`** writes the typed document **OVER** the retained raw object |
| 4 | ⚠️ **`save()` REFRESHES `rawDocument`** after a successful write |

⚠️ **(4) WAS NOT IN THE PLAN and is the subtle one:** ⛔ **without it a SECOND save in one session would
merge against load-time state and could resurrect a top-level key the first save legitimately removed.**

✅ **Direction matters:** the typed document is encoded first and written over the raw object, ⚠️ **so
every key the build owns takes the writer's current value** — ⛔ **reading a known value back out of the
raw object would resurrect a setting just changed.**

✅ **Everything around it untouched:** the atomic temp+replace, the do-not-clobber-a-corrupt-file rule,
the six typed properties and their ruled defaults, and `.sortedKeys` (⚠️ **Git-visible state — unstable
key order would churn the diff on every save**).

## ⚠️ A DELIBERATE LIMIT, recorded so it is not mistaken for an oversight

⛔ **Preservation is TOP-LEVEL ONLY.** ⚠️ **An unknown key nested INSIDE a card entry is still lost** —
`InspectorCardEntry` is its own fixed 3-key struct.

✅ **WHY IT WAS NOT DEEP-MERGED:** ⚠️ **reaching inside `scenes` → stack → entries means reconciling
ARRAYS the writer may have reordered, inserted into, or deleted from** — ⛔ **and with no identity on an
entry, that reconciliation can silently reattach a stale value to the WRONG CARD.** ⚠️ **Losing an
unknown key inside a card is bad; putting one on the wrong card is worse.** ✅ **The nested case belongs
with [SP-141]**, which owns the schema and can give entries identity.

## Verification

| check | result |
| ----- | ------ |
| ✅ **`xcodebuild test` (interop)** | ✅ **132/132** |
| ⛔ **Same suite, merge STUBBED OUT** | ⛔ **3 assertions fail across BOTH new tests** |
| ✅ **`ctest` (core)** | ✅ **604/604** |
| ✅ **macOS / iOS / visionOS** | ✅ **all three BUILD SUCCEEDED** |

⛔ **VERIFYING THIS THROUGH THE UI IS NOT POSSIBLE** — ⚠️ **the Inspector never displays layout key
values, and hand-editing JSON is not a UI action.** ✅ **The user said so plainly, and was right:**
verification here is the suite plus the disabled-fix run, ⛔ **not a rig pass.**

## ⛔ A TEST THAT PASSED WITHOUT RUNNING — the lesson worth keeping

⚠️ **The first cross-platform test read its project path from an ENVIRONMENT VARIABLE and `guard`ed on
it.** ⛔ **The variable never reached the test runner, so the test returned early and REPORTED SUCCESS
having executed nothing.** ✅ **Caught only by reading `inspector-layout.json` afterwards and noticing
`inspectorHidden` was still `false`** — ⚠️ **i.e. by checking the side effect, not the test result.**

✅ **The replacement stages its own fixture from bytes it controls and asserts THE STORE WROTE
(`inspectorHidden == true`, `selectedTab == "writing"`) alongside the preservation checks** — ⚠️ **so a
never-executed store fails it.**

⚠️ **Same lesson as [I-0214] and [I-0221]:** ⛔ **"the suite is green" is not evidence that the code
under test ran.**

## ⚠️ What the 2026-09-18 two-machine pass did and did not prove

✅ **The user ran a genuine two-machine pass** — same project on the Linux rig and macOS at once, both
worlds resolved, a character (`Verya`) added on macOS and seen on Linux after a scene switch, Linux
surviving a quit/relaunch with objects intact.

⛔ **IT DID NOT EXERCISE THIS TASK.** ✅ **Measured afterwards: that project's `inspector-layout.json`
was UNTOUCHED (mtime 2026-08-28) and held only the six keys Apple already knows.**

✅ **What it DID prove belongs to the world/relationship layer:** two platforms co-edit one project
correctly, `relationships.jsonl` round-trips, and both bindings resolve the same world through the
RELATIVE path while each stores its own machine-specific absolute path as a hint.
⚠️ **That relative path is what makes cross-machine sharing work and must not be "tidied".**

⚠️ **AND IT LED DIRECTLY TO [I-0223]:** ✅ **the user asked whether a relative path could work when the
world is on a genuinely DIFFERENT VOLUME.** ⛔ **It cannot** — ✅ **measured and filed the same day.**
