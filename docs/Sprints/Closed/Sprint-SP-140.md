---
sprint: SP-140
epic: EP-041
status: CLOSED 2026-09-18 (user-approved)
platform: Apple
activated: 2026-09-18
---

# Closed Sprint — SP-140 — `[Apple]` Stop discarding unknown `inspector-layout.json` keys ([I-0215])

**Epic:** [EP-041](../../Epics/Epic-EP-041.md) — `[Cross]` **The Boundary**
**Activated:** 2026-09-18 · **Closed:** 2026-09-18 · **Status:** ✅ **CLOSED — user-approved**
**Task:** **T-0536** ✅ **VERIFIED** · **Issue:** [I-0215] ✅ **VERIFIED**
**Blocks on:** ⛔ **NOTHING.** ✅ **It is EP-041's opener and its only unblocked Sprint.**

⚠️ **MOVED FROM [EP-040] 2026-09-18** (user ruling) — ✅ **EP-040 is the editor shell; this is boundary
work.** ⚠️ **[SP-130] is CLOSED**, so the earlier "runs in parallel with SP-130" note is historical.

## ⚠️ This is a LIVE defect, not preparation for the refactor

⚠️ **Apple's `InspectorLayoutDocument` is a fixed 6-property `Codable` struct**
(`InspectorLayoutStore.swift:63`). ⚠️ **Any key it does not know is DROPPED on the next save.**
✅ **Linux deliberately does the opposite** (`InspectorLayoutStore.cpp:97-100`):

> ⚠️ *"THE WHOLE DOCUMENT is kept, including every key this build does not understand … This is what
> makes the round trip lossless."*

⚠️ **CONSEQUENCE TODAY:** ⚠️ **open a project in a newer Scrivi (or on Linux, with a key Apple's build
predates), then open it on Apple** — ⛔ **Apple's next save DESTROYS the unknown keys.**
⚠️ **The user runs BOTH platforms against the same projects.**

✅ **WHY IT IS ITS OWN SPRINT:** ⚠️ **it is live data loss NOW.** ⛔ **Burying it inside [SP-141]'s
ownership refactor would delay a real fix behind a design ruling** — ⚠️ **and would make SP-141's
diff impossible to review, because a behavioural fix and an ownership move would be entangled.**

## Task

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0536** | ⚠️ **Make Apple's `inspector-layout.json` round trip LOSSLESS** — ✅ **preserve keys the build does not understand, as Linux already does** | **High** | ✅ **VERIFIED 2026-09-18** |

## Definition of Done

- [ ] ⚠️ **A key Apple does not know SURVIVES a load→mutate→save cycle.**
- [ ] ⚠️ **PROVEN BY A TEST that FAILS without the fix** — ⚠️ **not merely passes with it**
      (✅ **the [I-0214] discipline: a guarded check over absent data passes vacuously**).
- [ ] ⚠️ **Apple and Linux agree on the round trip** — ✅ **same file, both platforms, no loss.**
- [ ] ⚠️ **`xcodebuild` green for macOS, iOS AND visionOS.**

## ✅ The two sites, measured

⚠️ **The whole defect lives in two lines**, and the surrounding code is already correct:

| site | what it does | ⚠️ why it loses keys |
| ---- | ------------ | -------------------- |
| `InspectorLayoutStore.swift:174` | `JSONDecoder().decode(InspectorLayoutDocument.self, …)` | ⛔ **Decodes into a fixed 6-property struct — every other key is DISCARDED at this line** |
| `InspectorLayoutStore.swift:298` | `encoder.encode(document)` | ⛔ **Re-encodes only those 6 properties, so the discarded keys never come back** |

✅ **Everything AROUND them is already right and must not be disturbed:**
✅ **the save is genuinely atomic** (temp sibling + `replaceItemAt`, `:304-306`);
✅ **a corrupt file is NOT overwritten on load** (`:176-181` — *"the writer's layout may be recoverable
by hand, and clobbering it on open would destroy that chance"*);
✅ **the six typed properties and their ruled defaults are correct.**
⚠️ **This Sprint changes WHAT IS CARRIED, not how it is written.**

## ✅ Approach — ⚠️ mirror Linux, do not invent

⛔ **DO NOT DESIGN A NEW MECHANISM.** ✅ **Linux already solved this and its solution is in-tree**
(`platforms/linux/src/InspectorLayoutStore.cpp:97-100`): ⚠️ **it keeps the WHOLE parsed object and
reads typed values out of it**, rather than decoding into a fixed shape.

> ⚠️ *"THE WHOLE DOCUMENT is kept, including every key this build does not understand … This is what
> makes the round trip lossless."*

✅ **The Swift equivalent: keep the raw JSON object alongside the typed view.**

1. ✅ **On load, parse to a raw dictionary FIRST** (`JSONSerialization` or a `[String: AnyCodable]`),
   ⚠️ **keep it**, then decode the typed `InspectorLayoutDocument` from the same data as today.
2. ✅ **On save, re-encode the typed properties INTO the retained raw object** — ⚠️ **overwriting the
   six keys this build owns and leaving every other key untouched** — then serialise that.
3. ⚠️ **An absent file still yields `makeDefault()`**, ✅ **and a corrupt one still leaves the file
   alone** — ⛔ **neither behaviour changes.**

⚠️ **THE ORDERING RULE THAT MATTERS:** ⛔ **the raw object is the SOURCE for unknown keys ONLY.**
✅ **For the six known keys the typed struct always wins** — ⚠️ **otherwise a stale raw value could
resurrect a setting the writer just changed.**

⚠️ **`JSONEncoder`'s `.sortedKeys` must be preserved** (`:298`) — ✅ **the file is Git-visible project
state, and unstable key order would churn diffs on every save.**

## ⚠️ The test, and why the obvious one is not enough

⛔ **A test that writes a known key and reads it back proves NOTHING** — ✅ **it passes today.**

✅ **THE TEST MUST USE A KEY THE BUILD DOES NOT KNOW:**

1. Write an `inspector-layout.json` containing the six known keys **plus** an invented one
   (e.g. `"futureCardOrder": {"scene_x": ["a","b"]}`).
2. Load it, **mutate a known property** (flip `inspectorHidden`, say), save.
3. ✅ **Re-read the file as raw JSON and assert the invented key SURVIVED, with its value intact.**
4. ✅ **Assert the mutated known property also took effect** — ⚠️ **so the test cannot pass by simply
   not writing at all.**

⚠️ **AND IT MUST BE PROVEN TO FAIL WITHOUT THE FIX.** ✅ **The [I-0214]/[I-0221] discipline: stub the
change out, watch the test go red, restore it.** ⛔ **A regression test that passes both ways is not a
regression test.**

⚠️ **Step 4 is not padding.** ⛔ **A "fix" that preserved unknown keys by never re-encoding anything
would pass steps 1–3** — ✅ **and would silently stop saving the writer's layout.**

## ✅ Definition of Done — ⚠️ AC-linked

- [ ] ⚠️ **A key Apple does not know SURVIVES a load→mutate→save cycle.** *(→ [EP-041] AC1)*
- [ ] ⛔ **PROVEN BY A TEST THAT FAILS WITHOUT THE FIX**, demonstrated, not asserted.
- [ ] ✅ **A known property mutated in the same cycle still persists** — ⛔ **the test cannot pass
      vacuously.**
- [ ] ✅ **Key order stays sorted; the diff of an unchanged save is EMPTY.**
- [ ] ✅ **Corrupt-file behaviour is UNCHANGED** — ⚠️ **the file is still left untouched and reported.**
- [ ] ⚠️ **Apple and Linux agree on the round trip** — ✅ **same file through both, no loss either way.**
- [ ] ✅ **`xcodebuild` green for macOS, iOS AND visionOS.**

⛔ **NOT IN SCOPE:** ⚠️ **moving this file's ownership to ScriviCore is [SP-141]'s** — ✅ **this Sprint
does not add a core endpoint, and does not touch `platforms/linux`.**

## ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **`Codable` cannot express "keep the rest"** | ⚠️ **A fixed struct structurally cannot.** ✅ **Expect to keep the raw object alongside the typed view — which is what Linux does** |
| ⚠️ **The fix is written, then [SP-141] moves the file to the core anyway** | ✅ **The core will need the SAME lossless rule.** ⚠️ **Landing it here first means SP-141 INHERITS a proven behaviour instead of inventing one** |


---

## ✅ IMPLEMENTED AND VERIFIED 2026-09-18

✅ **T-0536 ✅ USER-VERIFIED 2026-09-18** and archived to
[`../Tasks/Verified/Task-verified-0536.md`](../../Tasks/Verified/Task-verified-0536.md).
✅ **[I-0215] ✅ VERIFIED** with it.
⚠️ **SP-140 HAS NO REMAINING WORK AND IS READY TO CLOSE** — ⛔ **only the user may close a Sprint.**

⚠️ **IT ALSO PRODUCED A NEW ISSUE IT DID NOT PLAN FOR: [I-0223]** — ✅ **the user asked whether a
relative path can work when the world is on a genuinely DIFFERENT VOLUME.** ⛔ **It cannot**, ✅ **and
the failure is worse than a miss: it resolves to a plausible WRONG location.** ⚠️ **Filed, unassigned;
✅ it fits [EP-041] but is NOT scheduled.**

### ✅ What changed — `Scrivi/App/InspectorLayoutStore.swift`, three edits

| # | change | why |
| - | ------ | --- |
| 1 | ✅ **`private var rawDocument: [String: Any]?`** added | ⚠️ holds the file **as read**, including keys the build cannot name |
| 2 | ✅ **Load captures it** (after the typed decode, non-fatally) | ⛔ a document that decoded cleanly must still load if this parse yields nothing |
| 3 | ✅ **`mergedForSave()` writes the typed document OVER the retained raw object** | ⚠️ direction matters: known keys always take the writer's current value |

✅ **`save()` also REFRESHES `rawDocument` after a successful write** — ⚠️ **without it a SECOND save in
one session would merge against load-time state and could resurrect a top-level key the first save had
legitimately removed.**

✅ **Everything around the change is untouched, as planned**: the atomic temp+replace, the
do-not-clobber-a-corrupt-file rule, the six typed properties and their ruled defaults.

### ⚠️ A LIMIT THAT IS DELIBERATE, NOT AN OVERSIGHT

⛔ **Preservation is TOP-LEVEL ONLY.** ⚠️ **An unknown key nested INSIDE a card entry is still lost** —
`InspectorCardEntry` is its own fixed 3-key struct.

✅ **WHY IT WAS NOT DEEP-MERGED:** ⚠️ **a merge deep enough to reach inside `scenes` → stack → entries
would have to reconcile ARRAYS the writer may have reordered, inserted into, or deleted from** —
⛔ **and with no identity on an entry, that reconciliation can silently reattach a stale value to the
WRONG CARD.** ⚠️ **Losing an unknown key inside a card is bad; putting one on the wrong card is worse.**
✅ **The nested case belongs with [SP-141]**, which owns the schema and can give entries identity.
⚠️ **RECORDED HERE so it is a known boundary, not a surprise.**

### ✅ Verification

| check | result |
| ----- | ------ |
| ⚠️ **Unknown key survives load→mutate→save** | ✅ **`futureCardOrder` → `["a","b"]` intact** |
| ⛔ **Same, with the fix DISABLED** | ⛔ **`futureCardOrder` LOST** — ✅ **the defect reproduced** |
| ✅ **Mutated known property still written** | ✅ **`inspectorHidden: true`** |
| ✅ **Keys stay sorted** | ✅ **`defaultStacks` first** |
| ✅ **`ctest`** | ✅ **604/604** |
| ✅ **`xcodebuild test` (interop)** | ✅ **132/132** — ⚠️ **run 2026-09-18 once Scrivi was closed** |
| ⛔ **Interop suite with the fix DISABLED** | ⛔ **3 assertions fail across BOTH new tests** |
| ✅ **`xcodebuild` macOS / iOS / visionOS** | ✅ **all three BUILD SUCCEEDED** |

⚠️ **THE DISABLED RUN IS THE ONE THAT MATTERS** — ✅ **the merge was stubbed to ignore the retained
object and the unknown key vanished**, ⛔ **while the "known property written" assertion passed in BOTH
runs.** ✅ **That second assertion is not redundant: it guards the OPPOSITE failure** — a "fix" that
preserved unknown keys by never re-encoding would satisfy the first check while silently ending all
layout saves.

✅ **`xcodebuild test` RAN 2026-09-18 once the user closed Scrivi: 132/132 pass**, and ⛔ **with the
merge stubbed out, THREE assertions fail across both new tests** — ✅ **`futureCardOrder` and
`linuxOnlyPreference` both vanish, exactly as [I-0215] describes.**

### ⛔ A TEST THAT PASSED WITHOUT RUNNING — recorded because it nearly shipped

⚠️ **The first attempt at the cross-platform test took the project path from an ENVIRONMENT VARIABLE
and `guard`ed on it being non-empty.** ⛔ **The variable never reached the test runner, so the test
returned early and REPORTED SUCCESS having executed nothing.**

✅ **It was caught only by reading `inspector-layout.json` afterwards and noticing `inspectorHidden`
was still `false`** — ⚠️ **i.e. by checking the side effect, not the test result.**

⚠️ **THE LESSON, and it is the same one [I-0214] and [I-0221] taught:** ⛔ **"the suite is green" is
not evidence that the code under test ran.** ✅ **The replacement stages its own fixture from bytes it
controls, so there is no path by which it can pass vacuously** — ✅ **and it asserts the store WROTE
(`inspectorHidden == true`, `selectedTab == "writing"`) alongside the preservation checks, so a
never-executed store fails it.**

### ⚠️ What the 2026-09-18 two-machine pass DID and DID NOT prove

✅ **The user ran a genuine two-machine pass** — ⚠️ **the same project open on the Linux rig and on
macOS simultaneously, both with the world resolved, a character (`Verya`) added on macOS and seen on
Linux after a scene switch, and Linux surviving a quit/relaunch with objects intact.**

⛔ **IT DID NOT EXERCISE THIS SPRINT.** ✅ **Measured afterwards: the project's `inspector-layout.json`
was UNTOUCHED (mtime 2026-08-28) and contained ONLY the six keys Apple already knows** — ⚠️ **so there
were no unknown keys to preserve and the file was never rewritten.**

✅ **What it DID prove is worth recording anyway** — ⚠️ **it belongs to the world/relationship layer,
not to T-0536:** ✅ **two platforms co-edit one project correctly; `relationships.jsonl` round-trips
between them; both bindings resolve the same world through the RELATIVE path
(`../../worlds/Eskandar.scrivworld`) while each stores its own machine-specific absolute path as a
hint.** ⚠️ **That relative path is what makes cross-machine sharing work and must not be "tidied".**

⛔ **VERIFYING T-0536 THROUGH THE UI IS NOT POSSIBLE** — ⚠️ **the Inspector never displays layout key
values, and asking the user to hand-edit JSON is not a UI action.** ✅ **So this Sprint's verification
is the interop suite plus the disabled-fix run above**, ⚠️ **not a rig pass.**


---

## ✅ CLOSE — 2026-09-18, user-approved

✅ **T-0536 ✅ VERIFIED** → [`../../Tasks/Verified/Task-verified-0536.md`](../../Tasks/Verified/Task-verified-0536.md).
✅ **[I-0215] ✅ VERIFIED** — ⛔ **live data loss, closed the day the Sprint opened.**

⚠️ **PLANNED AND DELIVERED IN ONE DAY**, ✅ **which is what "blocks on nothing" was supposed to mean and
for once did.** ⚠️ **It was the only unblocked Sprint in [EP-041]; ⛔ the remaining three are serial.**

### ⚠️ Two things this Sprint taught that outlive it

⛔ **1. A TEST PASSED WITHOUT RUNNING.** ⚠️ **The first cross-platform test read its path from an
environment variable that never reached the runner; it returned early and reported success having
executed nothing.** ✅ **Caught only by checking the file's side effect afterwards.** ⚠️ **Same lesson
as [I-0214] and [I-0221]: "green" is not evidence the code under test ran.**

⛔ **2. THE UI CANNOT VERIFY THIS.** ⚠️ **The Inspector never displays layout key values, and asking the
user to hand-edit JSON is not a UI action** — ✅ **the user said so plainly and was right.**
⚠️ **Verification was the interop suite plus a disabled-fix run, and the Sprint record says so instead
of implying a rig pass happened.**

### ✅ It produced an Issue it did not plan for

⚠️ **[I-0223]** — ✅ **the user asked, from first principles, whether a relative path can work when the
world is on a genuinely DIFFERENT VOLUME.** ⛔ **It cannot**, ⚠️ **and the failure is worse than a miss:
the stored relative path resolves to a plausible WRONG location inside the user's home directory.**
✅ **Measured and filed the same day; unassigned.** ⚠️ **It fits this Epic but is NOT scheduled.**

### ⛔ What this Sprint did NOT close

⚠️ **[I-0197] REMAINS OPEN** — ✅ **Class C closed by [SP-130]; ⛔ Classes A and B are not.**
⚠️ **The chain continues: [SP-141] → [SP-142] → [SP-143].**
⛔ **[SP-141] CANNOT START without an endpoint-shape ruling.**

⚠️ **AND THE NESTED-KEY LIMIT IS CARRIED, NOT FIXED:** ⛔ **an unknown key INSIDE a card entry is still
lost.** ✅ **Deliberate — deep-merging arrays with no entry identity could reattach a stale value to the
WRONG card, which is worse than losing it.** ⚠️ **It belongs with [SP-141], which can give entries
identity.**
