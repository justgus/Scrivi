---
epic: EP-041
status: CLOSED 2026-09-22 (user-approved)
closed: 2026-09-22
platform: Apple + ScriviCore + Linux
activated: 2026-09-18
---

# Closed Epic — EP-041: `[Cross]` ⚠️ **The Boundary** — ✅ **make ScriviCore the only writer of project state**

**Status:** 🟡 **ACTIVE — activated 2026-09-18 (user-ruled)**
**Sprints:** ✅ [SP-140] · ✅ [SP-141] · 🔵 [SP-142] · 🔵 [SP-149] · 🔵 [SP-143]
**Primary Issue:** [I-0197] (Classes A and B) · **Also:** ✅ [I-0215] (closed) · ⚠️ [I-0223] (unassigned) ·
⚠️ **[I-0241]** (`[Linux]`, filed 2026-09-21 while planning [SP-149]) → ✅ **[SP-142] / T-0542**

✅ **CLOSED 2026-09-22 — USER-APPROVED.** ✅ **All five ACs met, all five Sprints closed, [I-0197]
closed.**
✅ **SP-140 · SP-141 · SP-142 · SP-149 · SP-143 — all closed.** ✅ **T-0536, T-0507, T-0537, T-0542,
T-0541, T-0510 — all Verified.** ✅ **[I-0215], [I-0241], [I-0197] — all closed.**
⚠️ **[I-0242] remains OPEN and is NOT this Epic's** — ✅ **it is [EP-036] AC4a/AC4b.**
⚠️ **[I-0223] was filed unassigned by [SP-140] and is still unassigned.**

⚠️ **Prior status 2026-09-21: ✅ [SP-141] CLOSED (user-approved); T-0507 VERIFIED.** ✅ **AC2 is MET.** ⚠️ **HALF of [I-0197] Class B is closed** — ⛔ **Linux's duplicate
store is untouched until [SP-142], which is now UNBLOCKED.**
✅ **Three Sprints remain: 🟢 [SP-142] (ACTIVE 2026-09-21, two Tasks), 🔵 [SP-149] (the guard MECHANISM — fully ruled, blocks on nothing) and 🔵 [SP-143] (verify + close).**
⚠️ **Q1 was answered BEFORE T-0507 implemented, not after** — ✅ **it surfaced from planning the
then-BLOCKED [SP-142], which is why it cost nothing.** ⚠️ **The pre-implementation status follows:**

⛔ **[SP-141] WAS ACTIVATED BUT NOT STARTED — T-0507 is 🔵 NOT STARTED, no core
endpoint exists (`grep inspector_layout scrivi.h` → nothing), and `InspectorLayoutStore.swift` still
does its own file I/O (:175, :192, :368-370).** ⚠️ **NOTHING HAS BEEN IMPLEMENTED IN THIS EPIC SINCE
[SP-140] CLOSED ON 2026-09-18.** ✅ **[SP-142] was PLANNED 2026-09-21 anyway**
→ [`../Sprints/Sprint-SP-142.md`](../../Sprints/Sprint-SP-142.md) — ⚠️ **it raises Q1 (absence semantics
for a missing/corrupt layout document), ⛔ which [SP-141] must answer BEFORE it implements.**

⚠️ **PRIOR STATUS 2026-09-18: [SP-140] CLOSED — ⛔ the Epic's only unblocked Sprint is done.**
✅ **[SP-141]'s ENDPOINT-SHAPE RULING WAS MADE 2026-09-18 (user): ONE OPAQUE DOCUMENT GET/PUT.**
⚠️ **[SP-141] IS NOW ACTIVE.** ⚠️ **Superseded text follows, kept for its reasoning:**
⛔ **[SP-141] needs an ENDPOINT-SHAPE RULING before it can start, and [SP-142]/[SP-143] queue behind
it** — ⚠️ **so this Epic is currently BLOCKED ON A DECISION, not on work.**

---

## ✅ Why this Epic exists

⚠️ **[I-0197] FOUND 35 PLACES WHERE SWIFT READS AND WRITES PROJECT-PACKAGE FILES DIRECTLY**, bypassing
ScriviCore — ⚠️ **violating the architecture's central rule**, which `CLAUDE.md` states without
qualification:

> *"No backend logic is reimplemented in Swift. Swift is responsible for UI only."*

✅ **Re-measured 2026-09-15: `10` hits in `6` files**, ⚠️ **and they are NOT all equal.** ✅ **Three
classes, and only the first two are violations:**

| class | what | disposition |
| ----- | ---- | ----------- |
| ⚠️ **A** | ✅ **A core endpoint EXISTS and is IGNORED** | ✅ **CLOSED by [SP-129]** (EP-040) |
| ⛔ **B** | ⚠️ **NO core endpoint exists, so Swift had nowhere else to go** | ⛔ **OPEN — this Epic** |
| ✅ **C** | ⚠️ **Existence checks on paths the core ALREADY resolved** | ✅ **CLOSED by ruling, [SP-130]** (EP-040) |

⚠️ **CLASS B IS THE SERIOUS ONE, not the lesser one.** ⚠️ **`InspectorLayoutStore` READS AND WRITES
`inspector-layout.json` INSIDE THE PROJECT PACKAGE** (`:156`, `:173`, `:304` — a `.write(to:atomic)`),
⛔ **and the core has ZERO inspector-layout endpoints (grep: 0).**

✅ **WHY THAT MATTERS BEYOND TIDINESS** — ⚠️ **every bypass is a place the core's guarantees DO NOT
APPLY:**

- ⛔ no atomic-write discipline
- ⛔ no repair path — ⚠️ **the External Change Repair Matrix has NO ROW for this file**
- ⛔ no external-change detection
- ⛔ no `soft`-mount timeout
- ⚠️ **and NO Linux/Windows equivalent** — ⛔ **so each platform invents its own**

⚠️ **IT IS GIT-VISIBLE PROJECT STATE, shared between platforms that cannot read each other's writes.**
⚠️ **[I-0183] is the precedent for what an unvalidated write into a package costs.**

---

## ⚠️ Why this is NOT [EP-040]

⚠️ **[EP-040] IS THE WINDOW AND PANE STRUCTURE** — ✅ **toolbar, `safeAreaBar`, the Inspector as a real
column, the Detail Sheet's chrome.** ⛔ **None of this Epic's work is chrome.**

⚠️ **[SP-129] and [SP-130] WERE PARKED IN EP-040 BY USER RULING 2026-09-15**, ✅ **because they were
HOMELESS: [EP-039] listed the bypasses as in-scope and CLOSED WITHOUT DOING THEM.** ⚠️ **EP-040's own
scope note said plainly that its goal line would not answer for them, and recommended an Epic of their
own.** ✅ **This is that Epic** — ⚠️ **created 2026-09-18 on the user's ruling, before the remaining
four Sprints accumulated the same mismatch.**

⛔ **SP-129 AND SP-130 DO NOT MOVE HERE.** ✅ **User ruling 2026-09-18: a CLOSED Sprint keeps the
provenance of the Epic it actually ran under.** ⚠️ **Their records stay in
[`../Sprints/Closed/`](../../Sprints/Closed/) under [EP-040]** — ✅ **and their outcomes are credited in
this Epic's class table above, so [I-0197]'s story is still readable end to end.**

---

## ✅ Goal

⚠️ **A writer's project state is written by ScriviCore alone, on every platform** — ✅ **so a project
opened on macOS, Linux and Windows round-trips without loss, and the core's repair, validation and
atomic-write guarantees actually cover the files a project contains.**

---

## Acceptance Criteria

- [x] ✅ **AC1 — MET 2026-09-18.** `inspector-layout.json` round-trips losslessly on Apple; ✅ **proven
      by a test demonstrated to FAIL without the fix** (3 assertions across 2 tests).
      ⚠️ **TOP-LEVEL keys only** — ⛔ **a key nested inside a card entry is still lost**, ✅ **deliberately
      deferred to [SP-141]**, which can give entries identity. *(→ [SP-140] / [I-0215] ✅ Verified)*
- [x] **AC2** — ✅ **MET 2026-09-21, USER-VERIFIED.** ✅ **ScriviCore owns `inspector-layout.json`**: core endpoints exist, and ⚠️ **Apple
      calls them instead of `FileManager`.** ⛔ **Zero `.write(to:atomic)` into the package from Swift.**
      ✅ **VERIFIED by the user's live pass 2026-09-21** — ✅ **`scrivi_get_inspector_layout` /
      `scrivi_put_inspector_layout`; `InspectorLayoutStore.swift` has ZERO direct file I/O left;
      `ctest` 621/621, `xcodebuild test` 132/132, and the 8 new ABI tests were PROVEN FAILING against
      an injected [I-0215]-shaped defect.** *(→ [SP-141] / [T-0507])*
- [x] **AC3** — ✅ **MET 2026-09-21, live-pass VERIFIED.** ⛔ **Linux's duplicate
      `InspectorLayoutStore.cpp` is RETIRED**, not left as a second implementation of the same rule.
      ✅ **ZERO `QFile`/`QSaveFile` remain; the Mac cross-check confirmed the layout intact.**
      *(→ [SP-142] / [T-0537])*
- [x] **AC4** — ✅ **MET 2026-09-22.** ✅ **T-0541 VERIFIED by a GREEN GitHub run (Apple CI #1,
      `a25e106`); ✅ WITNESSED by [SP-143]/T-0510 — the guard goes RED against `78a739f~1`.**
      ✅ **`scripts/check-package-boundary.sh` + `.github/workflows/scrivi-apple-ci.yml`** —
      ⚠️ **`Scrivi/`'s FIRST CI of any kind** — ✅ **with `check-textkit2.sh` wired in beside it
      (it had run in NO automation since SP-133), and the Linux half in `scrivi-linux-ci.yml`.**
      ✅ **VERIFIED FAILING FIVE WAYS** (four injected + ⚠️ **the real historical code**).
      ✅ **A regression guard prevents a new bypass from landing.**
      ✅ **[SP-149] CREATED 2026-09-21 TO OWN THE MECHANISM** — ⚠️ **the design half blocks on nothing
      and was at risk of never being done deliberately.** *(→ [SP-149] / [T-0541]; verified by
      [SP-143] / [T-0510])*
      ⚠️ **THREE FINDINGS RESHAPE THIS AC** (measured 2026-09-21): ⛔ **`check-textkit2.sh` — the
      precedent this AC names — IS NOT WIRED INTO ANY CI;** ⛔ **NO workflow covers `Scrivi/` at all,
      so a Swift-only change runs ZERO CI** — ⚠️ **and that is exactly the change that introduces a
      bypass;** ✅ **the surface is 12 hits in 6 files with ZERO package writes left**, ⚠️ **so the
      baseline is clean and this is the cheapest moment to install a guard.**
      ✅ **MECHANISM RULED IN FULL 2026-09-21 (user):** ⚠️ **narrow API pattern** (`.write(to:`,
      `Data/String(contentsOf:)`, `FileHandle`; ⛔ **NOT `fileExists` — Class C is already ruled**) ·
      ✅ **a NEW lint-only `scrivi-apple-ci.yml` on `Scrivi/**`, which is that tree's FIRST CI** ·
      ✅ **`check-textkit2.sh` wired in beside it** · ✅ **ONE script, TWO pattern sets, BOTH workflows** ·
      ✅ **[SP-149] rules AND builds; [SP-143] stays the independent witness.**
      ⚠️ **PLANNING IT FOUND A LIVE BYPASS NOTHING ELSE HAD: [I-0241].**
- [x] **AC5** — ✅ **MET 2026-09-22 — [I-0197] IS CLOSED.** ⚠️ **Classes A and C are already closed** (by [SP-129] and
      [SP-130] under EP-040); ⛔ **AC5 is met only when Class B is too.**
      ⚠️ **STATUS 2026-09-21: Class B's CODE is done on BOTH platforms** — ✅ **`InspectorLayoutStore`
      touches the package on NEITHER** — ⛔ **but [SP-142] is not user-Verified and the guard
      ([SP-149]) is not built, so AC5 stays OPEN.**

⚠️ **AC4 IS THE ONE THAT CAN SLIP QUIETLY.** ✅ **AC1–AC3 are concrete; AC4 is a design problem with no
mechanism yet** — ⚠️ **and an Epic that promises a guard without specifying it will close with a
prose paragraph instead of a check.**

✅ **ACTED ON 2026-09-21: [SP-149] now owns the mechanism**, ⚠️ **split out of [SP-143] precisely
because the DESIGN half blocks on nothing while the VERIFY half blocks on two Sprints** — ⛔ **bundled,
the design would not start until the blockers cleared, which is how it slips to the end.**

---

## Sprints

| Sprint | Task | Title | Status | ⛔ Blocks on |
| ------ | ---- | ----- | ------ | ----------- |
| ✅ **[SP-140]** | **T-0536** | ✅ **[I-0215] CLOSED — the layout round trip is lossless** | ✅ **CLOSED 2026-09-18** — [record](../../Sprints/Closed/Sprint-SP-140.md) | ✅ **was unblocked** |
| ✅ **[SP-141]** | **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** | ✅ **CLOSED 2026-09-21** — [record](../../Sprints/Closed/Sprint-SP-141.md) | ✅ **BOTH rulings made (shape 09-18, absence 09-21)** |
| ✅ **[SP-142]** | **T-0537** · **T-0542** | ⚠️ **Retire Linux's duplicate `InspectorLayoutStore.cpp`** · ⚠️ **[I-0241] historical-event tags off disk** | ✅ **CLOSED 2026-09-21** — [record](../../Sprints/Closed/Sprint-SP-142.md) | ✅ **was unblocked by [SP-141]** |
| ✅ **[SP-149]** | **T-0541** | ✅ **The guard MECHANISM — BUILT and WIRED** | ✅ **CLOSED 2026-09-22** — [record](../../Sprints/Closed/Sprint-SP-149.md) | ✅ **was unblocked** |
| ✅ **[SP-143]** | **T-0510** | ✅ **WITNESSED the guard red-before / green-after; CLOSED [I-0197]** | ✅ **CLOSED 2026-09-22** — [record](../../Sprints/Closed/Sprint-SP-143.md) | ✅ **was unblocked** |

⚠️ **THE CHAIN IS MOSTLY SERIAL, and that is a real schedule risk.** ✅ **Only [SP-140] blocks on
nothing.** ⛔ **[SP-141] cannot start without a ruling that has not been made** — ⚠️ **and SP-142 and
SP-143 both queue behind it, so a delay there stalls three quarters of the Epic.**

✅ **[SP-140] WAS SEQUENCED FIRST DELIBERATELY:** ⚠️ **it is live data loss NOW**, ⛔ **independent of
the ownership refactor**, ✅ **and landing it first means [SP-141] INHERITS a proven lossless behaviour
instead of inventing one.**

---

## ⛔ The ruling [SP-141] needs, stated so it is not discovered late

⚠️ **WHAT SHAPE DO THE `inspector-layout.json` ENDPOINTS TAKE?** ⛔ **Not decided.** ⚠️ **The options
are not equivalent and the choice is not Claude's:**

- ⚠️ **A typed endpoint per property** — ✅ **validates; ⛔ the core then owns the inspector's SCHEMA,
  and every new card kind needs a core change.**
- ⚠️ **One opaque document get/put** — ✅ **the core owns atomicity and repair, the app owns meaning;
  ⛔ the core cannot validate what it is storing.**
- ⚠️ **A hybrid** — ✅ **typed where the core must reason, opaque for the rest.**

---

### ✅ **RULED 2026-09-18 (user): ONE OPAQUE DOCUMENT GET/PUT**

✅ **The core owns atomicity, durability and repair; the APP owns meaning.** ⛔ **The core does NOT
validate the document's interior and does NOT know what a card kind is.** ✅ **This is the option the
[SP-140] lossless rule does not fight** — ⚠️ **unknown keys survive BY CONSTRUCTION, because the core
never interprets them.** ⚠️ **ACCEPTED COST, recorded so it is not later rediscovered as a defect:
the core CANNOT validate what it stores here; a malformed layout is the app's to detect.**

⚠️ **THE LOSSLESS RULE FROM [SP-140] CONSTRAINS ALL THREE** — ✅ **whatever shape is chosen must keep
keys it does not understand**, ⚠️ **which argues against the fully-typed option.**

---

### ✅ **RULED 2026-09-21 (user): ABSENCE SEMANTICS — "CORE REPORTS, APP DECIDES"**

⚠️ **THE SECOND RULING [SP-141] NEEDED.** ✅ **The 2026-09-18 ruling settled the SHAPE; ⛔ it did NOT
settle what a GET returns when the document is MISSING or CORRUPT** — ⚠️ **and both platforms answer
that in app code today, with behaviour their tests already assert.**

✅ **RULED: the GET always succeeds (`ok:true`) and reports a `status` of `ok` | `absent` |
`unreadable`.**

| Case | ✅ Core returns | ✅ App does |
| ---- | -------------- | ---------- |
| present | `status:"ok"` + `document` | use it |
| missing | `status:"absent"` | ⚠️ **its own defaults** |
| corrupt | `status:"unreadable"` + `message` | defaults **+ warn** |

⛔ **THE CORE NEVER INVENTS DEFAULTS** — ✅ **it does not know what a tab is, which is the same
reasoning that produced the opaque shape.** ⛔ **THE CORE NEVER OVERWRITES A CORRUPT FILE on read**;
✅ **a PUT onto one SUCCEEDS, because that is the writer's explicit act.**

✅ **WHY NOT AN ERROR:** ⚠️ **a missing layout is NORMAL — every project created before this file
existed has none.** ⛔ **An error would also collapse `absent` and `unreadable` into one signal**,
⚠️ **so the app could not warn about a RECOVERABLE corrupt file.** ✅ **[I-0222] is precedent: an
unavailable world reporting `ScriviError 1` was itself filed as a defect.**

✅ **WHY NOT AN EMPTY DOCUMENT:** ⚠️ **that is exactly the ambiguity `project_envelope_empty_vs_failed`
records** — ⛔ **and the app would then save `{}` over the writer's damaged layout**, ⚠️ **destroying
evidence the Linux smoke test explicitly protects.**

---

## Issues

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| **[I-0197]** | ⚠️ **Swift reads and writes project-package files directly, bypassing ScriviCore** | **Medium** | ⚠️ **Classes A+C closed; B open** |
| ✅ **[I-0215]** | ✅ **CLOSED — Apple's layout round trip is lossless** | **High** | ✅ **[SP-140], Verified 2026-09-18** |
| ⚠️ **[I-0223]** | ⚠️ **A world on a SEPARATE VOLUME cannot be shared between platforms** — ⛔ **both stored paths encode host layout** | **Medium** | ⛔ **Not Assigned** |

---

## ⚠️ Out of scope, said plainly

⛔ **This Epic does NOT sweep every file Swift touches.** ✅ **It closes the `inspector-layout.json`
bypass and builds the guard that stops the next one.** ⚠️ **If the guard (AC4) finds further Class B
sites, they are NEW work — recorded, not silently absorbed.**

⛔ **It does not cover Windows.** ⚠️ **Windows has no platform layer yet**, ✅ **so AC3's "retire the
duplicate" applies to Linux only** — ⚠️ **and the guard in AC4 is what stops Windows re-earning it,
per `Scrivi_Platform_Porting_Outline_v0_1.md`.**


---

## ✅ Closing summary — 2026-09-22

| | |
| --- | --- |
| **Sprints** | ✅ SP-140 · SP-141 · SP-142 · SP-149 · SP-143 — **all closed** |
| **Tasks** | ✅ T-0536 · T-0507 · T-0537 · T-0542 · T-0541 · T-0510 — **all Verified** |
| **Issues closed** | ✅ [I-0215] · [I-0241] · **[I-0197]** |
| **Issues opened and left open** | ⚠️ [I-0242] (→ [EP-036]) · ⚠️ [I-0223] (unassigned) |

### ⚠️ What this Epic actually cost, and what it was worth

✅ **It planned FIVE sprints and ran FIVE** — ⚠️ **but not the five it planned.** ⛔ **[SP-149] did not
exist at the start:** ✅ **it was split out of [SP-143] on 2026-09-21 because the guard's DESIGN blocked
on nothing while its VERIFICATION blocked on two Sprints** — ⚠️ **and the Epic's own record had already
predicted AC4 would otherwise "close with a prose paragraph instead of a check."**

### ⚠️ THE RECURRING FINDING: capability shipped, no reader

⚠️ **THREE defects of ONE shape surfaced, and NOT ONE was found by a test suite:**

| Issue | Found by | The shape |
| ----- | -------- | --------- |
| **[I-0215]** | a user question about round-tripping | Apple DROPPED keys Linux KEPT |
| **[I-0241]** | ⚠️ **DESIGNING [SP-149]'s guard** | tags written, never readable — so Linux parsed the package itself |
| **[I-0242]** | ⚠️ **[SP-142]'s LIVE PASS** | the card stack is CUSTODIED losslessly and never READ |

✅ **`project_capability_without_surface`, three times in one Epic.** ⚠️ **[I-0242] is the sharpest:
[T-0537] proved Linux round-trips `defaultStacks` and `scenes` PERFECTLY** — ⛔ **and nothing reads
them.** ✅ **The Sprint made Linux a faithful custodian of a document it does not consult.**

### ✅ What the guard is, and what it is not

✅ **`scripts/check-package-boundary.sh` + `scrivi-apple-ci.yml`** — ⚠️ **`Scrivi/`'s FIRST CI of any
kind.** ⛔ **Before it, a Swift-only change ran ZERO checks** — ✅ **exactly the change that introduces a
bypass.** ✅ **It also finally RUNS `check-textkit2.sh`, written and verified in SP-133 and executed by
no automation for nine days.**

⚠️ **[SP-143] EXISTED TO STOP ONE CLAIM BEING MISTAKEN FOR ANOTHER.** ⛔ **A guard written against
already-clean code has only ever seen green.** ✅ **Run against `78a739f~1` — the last commit where BOTH
owners existed — it went RED and named all four sites, AND caught [I-0241]'s disk walk unprompted.**

⛔ **ONE RESIDUAL, NOT CLAIMED AS COVERAGE: CI has never been observed going RED.** ✅ **Red is proven
locally (five ways); ✅ green is proven on GitHub.** ⚠️ **The combination has not been observed.**

### ⚠️ Audit Check findings, ruled at close

⚠️ **[I-0223] WAS REFERENCED IN FIVE DOCUMENTS AND PRESENT IN NONE.** ✅ **Filed by [SP-140] on
2026-09-18, it never reached `Issue-active.md`** — ⛔ **so a measured defect had no tracking row for
four days.** ✅ **RECOVERED into the active file at this close**, ⚠️ **and CONFIRMED STILL LIVE against
the code (`WorldStore.cpp:305-315`).**
⚠️ **It is the same class as [I-0118] in the 2026-08-19 audit** — ✅ **an Issue named as evidence with
no archive entry** — ⛔ **which is precisely why the Audit Check runs FIRST.**

✅ **[SP-149] still showed as 🔵 PLANNED in `Sprint-backlog.md`** — ⚠️ **fixed in the same step**
(`feedback_sprint_backlog_cleanup`).
✅ **Everything else agreed: 5 Sprints archived, 6 Tasks archived, 3 Issues archived, IDs accurate.**
⛔ **No full Audit recommended.**

---

*Closed 2026-09-22 with user approval. Five Sprints, six Tasks, [I-0197] closed.*
