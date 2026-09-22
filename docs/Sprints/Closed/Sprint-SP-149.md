---
sprint: SP-149
epic: EP-041
status: CLOSED 2026-09-22 (user-approved)
activated: 2026-09-21
closed: 2026-09-22
task: T-0541
planned: 2026-09-21
platform: Apple + Linux (tooling)
---

# Closed Sprint — SP-149 — ⚠️ **The guard MECHANISM**

**Status:** ✅ **CLOSED 2026-09-22 — user-approved.** ✅ **All SEVEN ACs met; T-0541 VERIFIED** →
[`../../Tasks/Verified/Task-verified-0541.md`](../../Tasks/Verified/Task-verified-0541.md).
✅ **THE PROOF ARRIVED THE WAY THIS PLAN SAID IT WOULD HAVE TO: a GREEN GitHub Actions run**
("Scrivi Apple CI #1", `a25e106`, 2026-09-21 18:19 EDT). ⚠️ **The plan recorded that local evidence
could not settle it** — ✅ **and the first push touching `Scrivi/` settled it.**
✅ **AC1 was already MET at activation: all four mechanism questions were ruled by the user
2026-09-21, BEFORE the Sprint started** — ⛔ **which is the whole reason it was split out of [SP-143].**
**Epic:** 🟡 [EP-041] `[Cross]` **The Boundary** → [`../Epics/Epic-EP-041.md`](../../Epics/Epic-EP-041.md)
**Task:** **T-0541** — ⚠️ **ID reserved, not issued** (Tasks issue at activation).
**Serves:** **[EP-041] AC4** — ⚠️ **the only AC with no mechanism.**
**Blocks on:** ⛔ **NOTHING.** ✅ **It is design work and can run in parallel with [SP-142].**

⚠️ **ID NOTE: this was drafted as SP-145 and RENUMBERED to SP-149 before activation** — ✅ **SP-145–SP-148
are RESERVED for [EP-043] `[Linux]` The Session** (`Sprint-backlog.md`), ⛔ **exactly the collision that
reservation block exists to prevent.**
⚠️ **But it must LAND after [SP-142]** — see *"Why this is not simply SP-143"*.

---

## ⚠️ Why this Sprint exists

✅ **[EP-041]'s own record names AC4 as the one that can slip:**

> ⚠️ *"AC1–AC3 are concrete; AC4 is a design problem with no mechanism yet — and an Epic that promises
> a guard without specifying it will close with a prose paragraph instead of a check."*

⚠️ **[SP-143] is currently carrying BOTH the design and the build of that guard**, ✅ **and the design
half has no dependency at all** — ⛔ **while the build half is blocked behind two Sprints.**
⚠️ **Bundling them means the design does not start until the blockers clear**, ✅ **which is exactly
how AC4 slips to the end and closes as prose.**

✅ **THIS SPRINT SPLITS THE DESIGN OUT SO IT CAN START NOW.**

---

## ⚠️ THREE FINDINGS FROM THE CODE — ✅ **measured 2026-09-21, and they reshape the job**

### ⛔ 1. `check-textkit2.sh` IS NOT WIRED INTO ANY CI

⚠️ **[EP-041] AC4 says the guard should work *"the way [T-0527] guards the TextKit downgrade"*.**
✅ **T-0527's script is real, good, and VERIFIED FAILING** (`scripts/check-textkit2.sh`).
⛔ **But nothing runs it.** ✅ **Measured: no workflow in `.github/workflows/` references it, and
`scripts/verify-macos.sh` does not either.**

⚠️ **SO THE PRECEDENT IS A SCRIPT A HUMAN MUST REMEMBER TO RUN.** ⛔ **Copying it faithfully would
produce a second such script** — ✅ **and a guard nobody runs is the prose paragraph AC4 is trying to
avoid, just in executable form.**

### ⛔ 2. NO CI WORKFLOW COVERS `Scrivi/` AT ALL

✅ **Both workflows are path-filtered:**

| Workflow | Triggers on |
| -------- | ----------- |
| `scrivi-core-ci.yml` | `ScriviCore/**`, `CMakeLists.txt`, itself |
| `scrivi-linux-ci.yml` | `platforms/linux/**`, `ScriviCore/**`, `CMakeLists.txt`, itself |

⚠️ **A change touching ONLY `Scrivi/` runs ZERO CI.** ⛔ **That is precisely the change that introduces
an [I-0197] bypass** — ✅ **T-0507's own conversion was Swift-only in its Apple half.**

⚠️ **THIS IS THE SPRINT'S REAL PROBLEM.** ✅ **The grep is easy; ⛔ the place to run it does not exist.**

### ✅ 3. THE SURFACE IS SMALL AND ALREADY ENUMERATED — **12 hits, 6 files**

✅ **`FileManager.default` / `Data(contentsOf:)` / `.write(to:` across `Scrivi/` excluding tests:**

| File | Hits | ⚠️ Disposition |
| ---- | ---- | ------------- |
| `App/WorldVolumeStatus.swift` | 3 | ✅ **VOLUME/mount checks — not package I/O.** ⚠️ Ruled Class A |
| `App/AppEnvironment.swift` | 1 | ✅ **App-support dir lookup — outside any package** |
| `Views/Inspector/ObjectCard.swift` | 2 | ✅ **Asset presence — [I-0197] Class C, RULED 2026-09-18 (T-0508)** |
| `Views/Detail/ObjectImageSection.swift` | 2 | ✅ **Class C, ruled — UNREACHABLE case (core resolved the path)** |
| `Views/Detail/ExistingAssetPicker.swift` | 1 | ✅ **Class C, ruled — the one that WAS a real defect, fixed** |
| `Views/TimelineStripView.swift` | 3 | ⚠️ **2 are COMMENTS; 1 is `NSSavePanel` export to a USER-CHOSEN url** |

⛔ **ZERO package writes remain.** ✅ **So the guard starts from a CLEAN baseline** — ⚠️ **which is the
best possible moment to install one, and also the moment it is easiest to forget to.**

⚠️ **NOTE THE EXPORT CASE (`TimelineStripView.swift:2004`).** ✅ **`result.timelineJSON.write(to: url)`
where `url` comes from `NSSavePanel`** — ⛔ **writing OUT of the app to a place the writer picked is
NOT a bypass.** ⚠️ **A naive pattern flags it**, ✅ **and mis-flagging it is how an allow-list starts
accumulating unexamined entries.**

---

## Acceptance Criteria

- [x] **AC1** — ✅ **MET 2026-09-21, BEFORE ACTIVATION.** ⚠️ **THE MECHANISM IS RULED, IN WRITING,
      BEFORE ANY SCRIPT IS WRITTEN.** ✅ **All four questions answered by the user** (see *The four
      rulings*). ⛔ **This AC is the Sprint's whole point: [EP-041] AC4 exists because a guard was
      promised without a mechanism** — ✅ **and this Sprint was split out of [SP-143] precisely so the
      ruling could happen while the build half was still blocked.**
- [x] **AC2** — ✅ **MET.** ✅ **The guard EXISTS and RUNS AUTOMATICALLY.** ⚠️ **"Runs" means on the change that
      would introduce a bypass** — ⛔ **NOT "a script is present in `scripts/`", which is the state
      `check-textkit2.sh` is in today.**
      ✅ **Ruled Q2-A: a NEW lint-only `.github/workflows/scrivi-apple-ci.yml` on `Scrivi/**`.**
      ⚠️ **This is `Scrivi/`'s FIRST CI coverage of any kind.**
- [x] **AC3** — ✅ **MET — VERIFIED FAILING FOUR WAYS** (Apple bypass · Linux bypass · a bypass INSIDE an allow-listed file · a reasonless marker). ⚠️ **IT IS VERIFIED FAILING.** ✅ **Reintroduce a package read/write in Swift → the
      guard exits non-zero and names the line; remove it → exits zero.**
      ⛔ **A guard that has never failed is not known to work** (`feedback_fix_red_tests_dont_label_them`,
      and T-0527's own record makes the same point).
- [x] **AC4** — ✅ **MET.** ⚠️ **ENFORCED, not merely required: a `boundary-ok:` marker with no reason FAILS THE GUARD.** ✅ **Every allow-list entry carries a REASON**, ⚠️ **and every hit the ruled pattern
      produces is either allowed-with-reason or excluded.** ⛔ **No entry may read "pre-existing"**
      (`feedback_fix_red_tests_dont_label_them`). ✅ **Under the ruled pattern the Apple list starts at
      TWO entries** — ⚠️ **the `NSOpenPanel` import (`:1977`) and `NSSavePanel` export (`:2004`), both
      user-chosen paths.** ⛔ **Compare ELEVEN under the wide pattern.**
- [x] **AC5** — ✅ **MET.** ✅ **IT COVERS BOTH APP LAYERS** (ruled Q3): ⚠️ **ONE script, TWO pattern sets, wired
      into BOTH workflows.** ✅ **[EP-041]'s scope note says the guard "is what stops Windows
      re-earning it"** — ⚠️ **so a single rule statement is the point, not a convenience.**
- [x] **AC6** — ✅ **MET** (Apple + Linux stores, and [I-0241]'s site). ✅ **A tombstone/pointer at the guarded sites** so the next porter finds the rule where
      the temptation is, ⚠️ **in the form [SP-129] used at `EditorShell.cpp:2571`.**

- [x] **AC7** — ✅ **MET BY BECOMING UNNECESSARY.** ⛔ **[I-0241] was FIXED by [SP-142]/T-0542 before this guard existed, so its site is GONE and needs no entry** — ✅ **exactly the better outcome this AC anticipated.** ⚠️ **Original text: [I-0241] IS ON THE LINUX ALLOW-LIST, WITH ITS ISSUE NUMBER AND REASON.**
      ✅ **Found 2026-09-21 while designing this guard** (`EditorShell.cpp:2469` reads
      `objects/historical-events/*.json` directly). ⛔ **It is NOT fixed here** — ✅ **the fix needs a
      core change, since `scrivi_list_historical_events` drops `tags`.**
      ⚠️ **An allow-list entry naming an OPEN Issue is a DEBT, not an exception.**
      ✅ **[I-0241] IS ASSIGNED — [SP-142] / T-0542** (user ruling 2026-09-21), ⚠️ **so the debt has an
      OWNER and a date, not just a number.** ⛔ **The entry must be removed when T-0542 lands.**
      ✅ **IF [SP-142] RUNS FIRST, THIS ENTRY IS NEVER WRITTEN AT ALL** — ⚠️ **the guard would simply
      find nothing there, which is the better outcome.**
---

## ✅ THE FOUR RULINGS — user, 2026-09-21

⚠️ **ALL FOUR ARE MADE. ✅ AC1 IS DISCHARGED BEFORE ACTIVATION** — ⛔ **which is the whole reason this
Sprint was split out of [SP-143].**

---

### ✅ Q1 — RULED: **NARROW THE API PATTERN** (option C)

✅ **The guard greps for the reads and writes that MATTER, not for `FileManager.default`.**

```
PATTERN:  \.write\(to:|Data\(contentsOf:|String\(contentsOf:|FileHandle
```

⛔ **`fileExists` is DELIBERATELY EXCLUDED.** ✅ **It is [I-0197] Class C and was RULED on 2026-09-18
(T-0508)** — ⚠️ **including it would re-litigate a closed decision and inflate the allow-list from
about one entry to eleven**, ✅ **and T-0527's own script warns that *"an unexamined exception is how
this returns."***

⚠️ **`String(contentsOf:)` and `FileHandle` were ADDED to the pattern at ruling time** — ✅ **they are
the spellings a read-only bypass would otherwise use**, ⚠️ **which the bare narrow option would have
let through.** ⛔ **This is the accepted cost of C, paid down rather than left implicit.**

✅ **THE ADDITION IMMEDIATELY EARNED ITS PLACE.** ⚠️ **Re-measuring with `String(contentsOf:)` in the
pattern surfaced a FOURTH site the three-term version missed** (`TimelineStripView.swift:1977`) —
✅ **benign, but invisible to the pattern as first drafted.**

✅ **MEASURED BASELINE 2026-09-21 — FOUR hits in `Scrivi/`, ALL dismissible:**

| Site | ⚠️ Disposition |
| ---- | ------------- |
| `TimelineStripView.swift:156` | ✅ **COMMENT** — excluded by the comment filter |
| `TimelineStripView.swift:572` | ✅ **COMMENT** (a [SP-129] tombstone) — excluded |
| `TimelineStripView.swift:1977` | ✅ **`NSOpenPanel` import from a USER-CHOSEN url** — ⚠️ **ALLOW, with reason** |
| `TimelineStripView.swift:2004` | ✅ **`NSSavePanel` export to a USER-CHOSEN url** — ⚠️ **ALLOW, with reason** |

⛔ **ZERO package reads or writes remain in `Scrivi/`.** ✅ **The Apple allow-list starts at TWO
entries, both the same shape: the writer picked the path, so it is not package I/O.**
⚠️ **Compare the wide pattern's ELEVEN** — ✅ **which is the whole case for C.**

---

### ✅ Q2 — RULED: **A NEW LINT-ONLY `Scrivi/` WORKFLOW** (option A)

⚠️ **THIS IS THE RULING THAT MAKES THE SPRINT INFRASTRUCTURE WORK, NOT LINT.** ✅ **Ruled deliberately,
with that understood.**

✅ **A new `.github/workflows/scrivi-apple-ci.yml`, path-filtered to `Scrivi/**`, LINT ONLY.**
⛔ **NO `xcodebuild`** — ⚠️ **adding a build to CI is its own decision with its own cost and is NOT
smuggled in here** (see *Out of scope*). ✅ **A grep-only job is seconds on any runner, so the
macOS-runner cost objection does not apply.**

⚠️ **WHY IT IS NEEDED AT ALL — the two measured facts:**
1. ⛔ **`scripts/check-textkit2.sh` IS WIRED INTO NOTHING.** ✅ **No workflow references it; neither
   does `scripts/verify-macos.sh`.** ⚠️ **The precedent [EP-041] AC4 names is a script a human must
   remember to run.**
2. ⛔ **NO WORKFLOW COVERS `Scrivi/` AT ALL.** ✅ **`scrivi-core-ci.yml` filters to `ScriviCore/**`;
   `scrivi-linux-ci.yml` to `platforms/linux/**`.** ⚠️ **A Swift-only change runs ZERO CI** —
   ✅ **and T-0507's own Apple half was Swift-only.**

✅ **`check-textkit2.sh` IS WIRED INTO THE SAME WORKFLOW IN THIS SPRINT.** ⛔ **Adding a second unrun
guard beside an existing unrun guard would be absurd** — ⚠️ **and it makes `Scrivi/`'s first CI carry
both guards from day one.**

---

### ✅ Q3 — RULED: **YES, COVER LINUX — and YES, FILE THE FINDING**

✅ **ONE script, TWO pattern sets, wired into BOTH workflows.** ⚠️ **One place to state the rule** —
✅ **the standing derive-don't-restate principle applied to tooling.**

⚠️ **THE QUESTION STOPPED BEING HYPOTHETICAL DURING PLANNING.** ✅ **[I-0241] FILED 2026-09-21:**
⛔ **`readHistoricalEventTagsFromDisk()` (`platforms/linux/src/EditorShell.cpp:2469`) reads
`objects/historical-events/*.json` straight off disk**, ⚠️ **scanning EVERY file to find one event.**

⚠️ **ITS OWN COMMENT NAMES ITS SOURCE:** *"the same read-the-file pattern Apple uses for
imported-timeline events"* — ⛔ **and APPLE RETIRED THAT PATTERN in [SP-129]/[T-0502].**
✅ **So Linux is carrying a bypass Apple has already removed** (`feedback_linux_adopts_apple_shape`).
⚠️ **Apple's tombstone even predicted it:** *"which is exactly why Linux reads the files too."*

✅ **THE FINDING VALIDATES THE SPRINT:** ⚠️ **the guard would have caught this, and nothing else did** —
⛔ **not a test, not a review, not any tracking layer in ten months.**

⚠️ **[I-0241] IS NOT FIXED HERE.** ✅ **[SP-149] builds the guard; ✅ [SP-142] / T-0542 fixes it**
(user ruling 2026-09-21) — ⚠️ **because the fix needs a CORE change**
(`scrivi_list_historical_events` drops `tags`). ⛔ **The two Sprints do not block each other.**
⚠️ **So the Linux pattern set MUST start with [I-0241] on the allow-list, WITH ITS REASON AND ITS
ISSUE NUMBER** — ✅ **an allow-list entry that names an open Issue is a debt, not an exception.**

---

### ✅ Q4 — RULED: **THIS SPRINT RULES *AND* BUILDS** (option A)

✅ **[SP-149] designs the mechanism AND ships the working guard.**
✅ **[SP-143] keeps its real job: VERIFY it turns RED before [SP-142] and GREEN after, then close
[I-0197].**

⚠️ **The restructure record's objection — *"a guard written by the same change it polices has no
independent witness"* — DOES NOT APPLY HERE.** ✅ **[SP-149] does not touch Linux's
`InspectorLayoutStore`**, ⚠️ **so the guard is written by a change that does not police itself**, and
✅ **[SP-143] remains the independent witness.**

---


## Plan of work

| Step | Work | ⚠️ Note |
| ---- | ---- | ------ |
| ~~S1~~ | ✅ **DONE 2026-09-21 — all four ruled before activation** | ✅ **AC1 MET** |
| **S2** | ✅ Write the guard script from the ruling | ⚠️ **Model on `check-textkit2.sh`: reason-per-entry, actionable failure output** |
| **S3** | ✅ Classify every hit — allow-with-reason or out-of-pattern; ⚠️ **[I-0241] listed with its Issue number** | ⚠️ **AC4/AC7** |
| **S4** | ✅ **Create `scrivi-apple-ci.yml` (lint-only, `Scrivi/**`)**; wire BOTH guards into it, and the Linux patterns into `scrivi-linux-ci.yml` | ⚠️ **AC2/AC5 — the part that is not a grep** |
| **S5** | ⚠️ **VERIFY IT FAILING** — reintroduce a package write, confirm non-zero + named line | ⚠️ **AC3** |
| **S6** | ✅ Tombstones at the guarded sites | ⚠️ **AC6** |

---

## ⚠️ Why this is not simply [SP-143]

✅ **[SP-143] REMAINS, and its job is unchanged: verify the guard turns RED before [SP-142] and GREEN
after, then close [I-0197].** ⚠️ **That verification is what cannot be done early** — ⛔ **its
allow-list must contain Linux's `InspectorLayoutStore` before [SP-142] and not after**, ✅ **which is
the restructure record's original reason for a separate Sprint, and it still holds.**

⚠️ **WHAT MOVES HERE IS THE DESIGN**, ✅ **which blocks on nothing and is the half that was at risk of
never being done deliberately.**

---

## ⛔ Out of scope

- ⛔ **Retiring Linux's duplicate store** — ✅ **[SP-142] / T-0537.**
- ⛔ **Closing [I-0197]** — ✅ **[SP-143] / T-0510, and only after Class B is fully done.**
- ⛔ **Re-litigating Class A or Class C.** ⚠️ **Both are RULED** (T-0508, 2026-09-18) — ✅ **the guard
  must not flag what a ruling already permitted.**
- ⛔ **Broad CI expansion.** ✅ **Q2 RULED a new Apple workflow, and it is LINT-ONLY** — ⚠️ **adding
  `xcodebuild` to CI is its own decision with its own cost and is NOT smuggled in here.**
  ⛔ **The new workflow runs greps and nothing else.**
- ⛔ **FIXING [I-0241].** ✅ **This Sprint FINDS and ALLOW-LISTS it; ⚠️ the fix needs a CORE change**
  (`scrivi_list_historical_events` drops `tags`) — ⛔ **and writing a better Qt parser instead would
  repeat the very mistake [SP-129] corrected on Apple.**

---

## ⚠️ Risks

- ⛔ **THE BIGGEST: building a guard nothing runs.** ✅ **That is the state `check-textkit2.sh` is in
  TODAY** — ⚠️ **and it would be easy to reproduce while believing the precedent was followed.**
  ✅ **AC2 is worded to make "a script exists" insufficient.**
- ⚠️ **[I-0241]'s allow-list entry becoming permanent.** ⛔ **An entry naming an open Issue is a
  DEBT** — ✅ **it must be removed when [I-0241] closes**, ⚠️ **and nothing in a grep script reminds
  anyone to do that.**
- ⚠️ **An allow-list that grows unexamined.** ✅ **T-0527's script says it outright: *"An unexamined
  exception is how this returns."*** ⚠️ **Starting at ~11 entries (Q1 Option A, wide pattern) is
  already near the edge** — ⛔ **hence the recommendation to narrow the pattern instead.**
- ⚠️ **Flagging the legitimate export** (`TimelineStripView.swift:2004`, `NSSavePanel`).
  ⛔ **Mis-flagging teaches people to add entries without thinking.**
- ⚠️ **A guard that passes vacuously** — ✅ **AC3 is the only defence, and it is non-negotiable.**

---

## ⚠️ Estimate

✅ **SHORT — the code is a grep and a workflow file.** ⚠️ **The cost is S1 (a ruling) and S4 (the CI
gap), not the script.** ⛔ **If Q2 rules "new workflow", that is a first for `Scrivi/`** — ✅ **worth
doing on its own merits, ⚠️ but it makes this Sprint infrastructure work, not just lint.**


---

## ✅ Outcome

✅ **`Scrivi/` HAS CI FOR THE FIRST TIME.** ⚠️ **Before this Sprint, a Swift-only change ran ZERO
checks** — ⛔ **and that is exactly the change that introduces an [I-0197] bypass.**
✅ **`check-textkit2.sh` — written, verified-failing, and then run by NOTHING since SP-133 — now
actually executes somewhere.**

⚠️ **AC7 WAS MET BY BECOMING UNNECESSARY.** ✅ **It required [I-0241] on the allow-list as a tracked
debt; ⛔ [SP-142]/T-0542 FIXED it first, so the site is gone and the allow-list is smaller.**

⚠️ **THE SPRINT FOUND A DEFECT BEFORE ITS OWN GUARD EXISTED.** ✅ **[I-0241] surfaced while DESIGNING
the guard — the exercise of asking "what would this catch?" caught it.** ⛔ **No test, review or
tracking layer had it in the months it lived.**

## ⚠️ Audit-check findings, ruled as part of this close

⚠️ **ONE THING REMAINS UNPROVEN AND IS NOT CLAIMED HERE:** ⛔ **that CI goes RED on a violation.**
✅ **Proven LOCALLY four ways; ⛔ never through GitHub.** ⚠️ **A guard written when the code was already
clean has only ever seen green** — ✅ **which is precisely [SP-143]'s D1, and why that Sprint was kept
separate rather than folded in.**

✅ **Every layer agreed at close: T-0541 archived, no stale plan file, nothing left in the backlog.**

---

*Closed 2026-09-22 with user approval. All seven ACs met; T-0541 verified by a green CI run.*
