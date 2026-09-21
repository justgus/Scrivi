# Verified Task: T-0507

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0507** | ⚠️ **Core endpoints for `inspector-layout.json` + Apple adoption** — ✅ **[I-0197] Class B closed on the Apple side** | [SP-141] | [EP-041] | **2026-09-21** |

---

## What it was

⚠️ **TWO APP-SIDE IMPLEMENTATIONS OWNED ONE SCHEMA, AND THEY HAD ALREADY DRIFTED.** Swift's
`InspectorLayoutStore.swift` and Qt's `InspectorLayoutStore.cpp` each read and wrote
`inspector-layout.json` directly, with no core to arbitrate — ⛔ **a direct violation of the
architecture's central rule** (*"No backend logic is reimplemented in Swift"*).

✅ **The drift was not hypothetical: [I-0215] — Linux PRESERVED keys it did not understand, Apple
DROPPED them.** ⚠️ **Same file, same schema, two behaviours** — ✅ **and Linux was the one that had it
right.**

T-0507 built the core endpoints and converted Apple to them.

---

## ⚠️ It needed a SECOND ruling, and asking early is what made it cheap

✅ **The 2026-09-18 ruling settled the endpoint SHAPE** (one opaque document GET/PUT: the core owns
atomicity, durability and repair; the app owns meaning). ⛔ **It did NOT settle what a GET returns when
the document is MISSING or CORRUPT** — ⚠️ **and both platforms answered that themselves, in code, with
behaviour their tests already asserted.**

✅ **RULED 2026-09-21 (user): "CORE REPORTS, APP DECIDES."**

| Case | ✅ Core returns | ✅ App does |
| ---- | -------------- | ---------- |
| present | `status:"ok"` + `document` | use it |
| missing | `status:"absent"` | ⚠️ **its own defaults** |
| corrupt | `status:"unreadable"` + `message` | defaults **+ warn** |

⛔ **The core never invents defaults** (✅ it does not know what a tab is — the same reasoning that
produced the opaque shape) ⛔ **and never overwrites a corrupt file on read.** ✅ **A PUT over one
SUCCEEDS** — ⚠️ **the read path leaves damage alone; a write is the writer's explicit act.**

⚠️ **`absent` and `unreadable` are DELIBERATELY DISTINCT.** ✅ **The app defaults for both but can only
WARN about the second** — ⛔ **collapsing them loses that**, ⚠️ **and a missing layout is NORMAL: every
project created before this file existed has none.** ✅ **[I-0222] is precedent — an unavailable world
reporting an error was itself filed as a defect.**

✅ **THE QUESTION WAS RAISED BEFORE IMPLEMENTATION, FROM [SP-142]'s PLAN.** ⚠️ **Planning the BLOCKED
sprint is what surfaced it** — ✅ **asking then cost nothing; asking after would have meant revising an
endpoint two platforms call.**

---

## What shipped

**ScriviCore**
- ✅ **`scrivi_get_inspector_layout(projectRootPath)`** and
  **`scrivi_put_inspector_layout(projectRootPath, documentJson)`** — `scrivi.h`, `scrivi_c_api.cpp`.
- ⚠️ **The document is OPAQUE** — parsed only far enough to prove it is a JSON object, then returned or
  stored verbatim. ✅ **Unknown keys survive BY CONSTRUCTION, because the core never interprets them** —
  ⛔ **not by a preservation mechanism someone must remember to maintain**, ⚠️ **which is precisely the
  mechanism Apple did not have in [I-0215].**
- ✅ **`JsonDoc::isObject()`** added (`util/Json.hpp/.cpp`) — ⚠️ **the ONE thing a PUT validates.**
  ⛔ **An array or number parses as valid JSON and would make the file unreadable to every future GET**,
  ✅ **so storing one would manufacture the corruption the read path exists to report.**
  ✅ **An empty object `{}` is LEGAL** — a writer may clear their layout.
- ✅ **Atomic write (temp + rename)** — the discipline the Swift store used to implement itself.

**Apple**
- ✅ **`ScriviEngine.getInspectorLayout` / `.putInspectorLayout`**, plus `InspectorLayoutFetch`,
  `InspectorLayoutSave`, `InspectorLayoutStatus` and a **`JSONValue`** any-JSON carrier.
  ⚠️ **The document crosses as RAW JSON** — ⛔ **decoding it into a fixed struct at the boundary IS the
  [I-0215] defect**; ✅ **the typed decode stays one layer up, where the raw copy sits beside it.**
- ✅ **`InspectorLayoutStore.swift` converted.** ⛔ **ZERO `FileManager`, `Data(contentsOf:)` or
  `write(to:)` remain** — ✅ **that is [I-0197] Class B closed on the Apple side.**
- ⚠️ **The engine is INJECTED, matching `BufferService`** — ✅ **`feedback_look_for_existing_pattern_first`.**
  ⛔ **`ScriviEngine.shared` was written first from habit and does not exist in this app**; ⚠️ **inventing
  one would have created a SECOND route to the core.**
- ✅ **The non-macOS stubs were extended in the same step** — ⚠️ **that file records that they DRIFTED
  once already, and a lagging stub breaks only visionOS, long after the change that caused it.**

---

## How it was verified

✅ **USER LIVE PASS, 2026-09-21 — all checks pass.** ⚠️ **This is the half no suite can report on:**
✅ **what survives a quit** (`feedback_live_pass_finds_what_suites_cannot`).

**Automated evidence:**
- ✅ **`ctest` 621/621 on macOS** — ⚠️ **was 613; the 8 new ABI tests are the difference.**
- ✅ **`xcodebuild test` 132/132 in 12 suites** — ⚠️ **including BOTH [I-0215] lossless tests, which now
  exercise the core path rather than Swift's own file I/O.**
- ✅ **macOS app builds and codesigns clean, no warnings.**

⚠️ **THE NEW TESTS WERE PROVEN ABLE TO FAIL.** ✅ **An [I-0215]-shaped defect was injected into the core
(a typed round trip that drops unknown keys) and 3 of the 8 FAILED; the fix restored, all 8 pass.**
⛔ **A green test that cannot fail is not evidence** — ⚠️ **this Epic has paid for that twice ([I-0214],
[I-0215]).**

✅ **The tests go through `scrivi_*`, never the facade** — `feedback_boundary_tests_not_facade`.
⚠️ **A facade test cannot see a boundary gap by construction; that is how I-0113 shipped green.**

---

## ⚠️ What this did NOT close

⛔ **[I-0197] Class B is only HALF closed.** ⚠️ **Linux's duplicate `InspectorLayoutStore.cpp` is
UNTOUCHED** — ✅ **that is [SP-142] / T-0537, deliberately a separate sprint**: ⚠️ **bundling it would
let a Linux slip block a finished, verified Apple half ([SP-132]'s exact failure mode)**, ⛔ **and would
erase [SP-143]'s witness, which needs the duplicate PRESENT before and ABSENT after.**

✅ **[EP-041] AC2 is MET. ⛔ AC3, AC4 and AC5 remain open.**

---

## Incidental

⚠️ **The file predicted the defect it then caught.** ✅ **A C++ helper added inside `scrivi_c_api.cpp`'s
`extern "C"` span triggered `-Wreturn-type-c-linkage`** — ⚠️ **the file already documents this trap
TWICE (≈lines 1193 and 3341) and asks that any future helper in that span carry an `extern "C++"`
wrapper.** ✅ **Hit exactly as written; the documented fix applied.**

---

*Verified by the user 2026-09-21 after a live pass. Implemented under [SP-141] / [EP-041].*
