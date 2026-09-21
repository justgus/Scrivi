---
sprint: SP-141
epic: EP-041
status: CLOSED 2026-09-21 (user-approved)
platform: Apple + ScriviCore
activated: 2026-09-18
closed: 2026-09-21
task: T-0507
---

# Closed Sprint — SP-141 — `[Apple]`+`[ScriviCore]` Core-owned `inspector-layout.json`

**Epic:** [EP-041](../../Epics/Epic-EP-041.md) — `[Cross]` **The Boundary**
**Activated:** 2026-09-18 · **Closed:** 2026-09-21 · **Status:** ✅ **CLOSED — user-approved**
**Task:** **T-0507** ✅ **VERIFIED** → [`../../Tasks/Verified/Task-verified-0507.md`](../../Tasks/Verified/Task-verified-0507.md)
**Issue:** [I-0197] (Class B) — ⚠️ **HALF closed; see "What it did NOT close"**
**Serves:** ✅ **[EP-041] AC2 — MET**

---

## ✅ What it delivered

✅ **ScriviCore now owns `inspector-layout.json` on the Apple side.**

- ✅ **`scrivi_get_inspector_layout` / `scrivi_put_inspector_layout`** (`scrivi.h`, `scrivi_c_api.cpp`).
  ⚠️ **The document is OPAQUE** — parsed only far enough to prove it is a JSON object, then returned
  or stored verbatim.
- ✅ **`JsonDoc::isObject()`** (`util/Json.hpp/.cpp`) — ⚠️ **the ONE thing a PUT validates.**
- ✅ **`ScriviEngine.getInspectorLayout` / `.putInspectorLayout`** + `InspectorLayoutFetch`,
  `InspectorLayoutSave`, `InspectorLayoutStatus`, and a **`JSONValue`** any-JSON carrier.
- ✅ **`InspectorLayoutStore.swift` converted** — ⛔ **ZERO `FileManager`, `Data(contentsOf:)` or
  `write(to:)` remain.**

---

## ⚠️ It needed a SECOND ruling — and the way that was caught is the lesson

✅ **The 2026-09-18 ruling settled the endpoint SHAPE** (one opaque document GET/PUT).
⛔ **It did NOT settle what a GET returns for a MISSING or CORRUPT document** — ⚠️ **and both platforms
answered that themselves, in code, with behaviour their tests already asserted.**

✅ **RULED 2026-09-21 (user): "CORE REPORTS, APP DECIDES"** — `status` = `ok` | `absent` | `unreadable`.
⛔ **The core never invents defaults and never overwrites a corrupt file on read;** ✅ **a PUT over one
SUCCEEDS.** ⚠️ **`absent` and `unreadable` stay DISTINCT so the app can WARN about a recoverable
corrupt file while defaulting silently for a normal missing one.**

⚠️ **THE QUESTION SURFACED FROM PLANNING [SP-142] — THE SPRINT THAT WAS BLOCKED ON THIS ONE.**
✅ **Asking it before T-0507 implemented cost nothing.** ⛔ **Asking it after would have meant revising
an endpoint two platforms call.** ⚠️ **Worth carrying forward: planning a blocked sprint is not
busywork — it is how the blocker's own unstated questions get found.**

---

## ⚠️ The plan asked a question the ruling dissolved

⚠️ **The original [SP-141] plan asked: "`InspectorLayoutStore` has FOURTEEN mutators — which get
endpoints?"** ✅ **The opaque ruling made the question vanish: TWO endpoints serve all fourteen**,
⚠️ **because the core never interprets the document and therefore needs no entry point per property.**
✅ **That is the clearest evidence the shape ruling was the right one** — ⚠️ **the typed alternative
would have put the inspector's schema in the core and made every new card kind a core change.**

---

## ✅ Evidence

- ✅ **`ctest` 621/621 on macOS** — ⚠️ **was 613; the 8 new ABI tests are the difference.**
- ✅ **`xcodebuild test` 132/132 in 12 suites** — ⚠️ **including BOTH [I-0215] lossless tests, which
  now exercise the core path rather than Swift's own file I/O.**
- ⚠️ **THE NEW TESTS WERE PROVEN ABLE TO FAIL** — ✅ **an [I-0215]-shaped defect (a typed round trip
  dropping unknown keys) was injected into the core and 3 of 8 FAILED; fix restored, all 8 pass.**
  ⛔ **A green test that cannot fail is not evidence** (⚠️ **this Epic has paid for that twice:
  [I-0214], [I-0215]**).
- ✅ **Tests go through `scrivi_*`, never the facade** — `feedback_boundary_tests_not_facade`.
- ✅ **macOS app builds and codesigns clean, no warnings.**
- ✅ **USER LIVE PASS 2026-09-21 — all checks pass.** ⚠️ **The half no suite can report on.**

---

## ⛔ What it did NOT close

⚠️ **[I-0197] CLASS B IS HALF CLOSED.** ✅ **Apple is converted; ⛔ Linux's duplicate
`InspectorLayoutStore.cpp` is UNTOUCHED** — ⚠️ **so the schema still has two app-side owners, just no
longer two undisciplined ones.** ✅ **[SP-142] / T-0537 retires it, and is now UNBLOCKED.**

⛔ **[EP-041] AC3, AC4 and AC5 remain OPEN.**

---

## ⚠️ Audit-check findings, ruled as part of this close

⚠️ **TWO STALE RECORDS were found while archiving T-0507 — ✅ both fixed in the same step:**

1. ⛔ **`Sprint-SP-141.md` carried `epic: EP-040`** — ⚠️ **stale since the 2026-09-18 split** — ✅ **and
   a status of "BLOCKED ON A RULING" that was by then made twice. ✅ **Corrected; its plan is kept for
   the reasoning, under an outcome header.**
2. ⛔ **`Sprint-active.md`'s FIRST LINE still read "[SP-141] ACTIVE"** — ⚠️ **the first thing any
   reader sees.** ✅ **Corrected.**

✅ **Neither is systemic; ⛔ no full Audit is recommended.** ⚠️ **Both are the same shape as the index
drift found on 2026-09-20 and 2026-09-21, though: a status restated in a second place goes stale
there first.**

---

## Incidental

⚠️ **The codebase predicted a defect this Sprint then hit.** ✅ **A C++ helper added inside
`scrivi_c_api.cpp`'s `extern "C"` span triggered `-Wreturn-type-c-linkage`** — ⚠️ **the file documents
that trap TWICE (≈lines 1193, 3341) and asks that any future helper in that span carry an
`extern "C++"` wrapper.** ✅ **Hit exactly as written; the documented fix applied.**

---

*Closed 2026-09-21 with user approval. T-0507 Verified by live pass; [EP-041] AC2 met.*
