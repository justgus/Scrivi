---
epic: EP-041
status: Active
platform: Apple + ScriviCore + Linux
activated: 2026-09-18
---

# EP-041: `[Cross]` ⚠️ **The Boundary** — ✅ **make ScriviCore the only writer of project state**

**Status:** 🟡 **ACTIVE — activated 2026-09-18 (user-ruled)**
**Sprints:** [SP-140] · [SP-141] · [SP-142] · [SP-143]
**Primary Issue:** [I-0197] (Classes A and B) · **Also:** ✅ [I-0215] (closed) · ⚠️ [I-0223] (unassigned)

⚠️ **STATUS 2026-09-18: [SP-140] CLOSED — ⛔ the Epic's only unblocked Sprint is done.**
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
[`../Sprints/Closed/`](../Sprints/Closed/) under [EP-040]** — ✅ **and their outcomes are credited in
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
- [ ] **AC2** — ✅ **ScriviCore owns `inspector-layout.json`**: core endpoints exist, and ⚠️ **Apple
      calls them instead of `FileManager`.** ⛔ **Zero `.write(to:atomic)` into the package from Swift.**
      *(→ [SP-141] / [T-0507])*
- [ ] **AC3** — ⛔ **Linux's duplicate `InspectorLayoutStore.cpp` is RETIRED**, not left as a second
      implementation of the same rule. *(→ [SP-142] / [T-0537])*
- [ ] **AC4** — ✅ **A regression guard prevents a new bypass from landing.** ⚠️ **Mechanism NOT yet
      designed** — ⛔ **it must be SPECIFIED before it is promised**, the way [T-0527] guards the
      TextKit downgrade. *(→ [SP-143] / [T-0510])*
- [ ] **AC5** — ✅ **[I-0197] CLOSES.** ⚠️ **Classes A and C are already closed** (by [SP-129] and
      [SP-130] under EP-040); ⛔ **AC5 is met only when Class B is too.**

⚠️ **AC4 IS THE ONE THAT CAN SLIP QUIETLY.** ✅ **AC1–AC3 are concrete; AC4 is a design problem with no
mechanism yet** — ⚠️ **and an Epic that promises a guard without specifying it will close with a
prose paragraph instead of a check.**

---

## Sprints

| Sprint | Task | Title | Status | ⛔ Blocks on |
| ------ | ---- | ----- | ------ | ----------- |
| ✅ **[SP-140]** | **T-0536** | ✅ **[I-0215] CLOSED — the layout round trip is lossless** | ✅ **CLOSED 2026-09-18** — [record](../Sprints/Closed/Sprint-SP-140.md) | ✅ **was unblocked** |
| 🔵 **[SP-141]** | **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** | 🔵 **Planned** | ⛔ **an ENDPOINT-SHAPE RULING** |
| 🔵 **[SP-142]** | **T-0537** | ⚠️ **Retire Linux's duplicate `InspectorLayoutStore.cpp`** | 🔵 **Planned** | ⛔ **[SP-141]** |
| 🔵 **[SP-143]** | **T-0510** | ⚠️ **The regression guard — closes [I-0197]** | 🔵 **Planned** | ⛔ **[SP-141] AND [SP-142]** |

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

⚠️ **THE LOSSLESS RULE FROM [SP-140] CONSTRAINS ALL THREE** — ✅ **whatever shape is chosen must keep
keys it does not understand**, ⚠️ **which argues against the fully-typed option.**

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
