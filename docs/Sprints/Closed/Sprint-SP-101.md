# SP-101: `[Apple]` EP-030 — AC12 soft-failure isolation (T-0399) ✅ CLOSED

**Status:** ✅ **Closed 2026-08-11 (Human-approved)** — planned, implemented, and closed same day.
**Epic:** EP-030 `[Apple]` — Scene Inspector Card Framework. **The last item before EP-030 closed.**
**Goal:** Make card failure isolation a real **framework guarantee** for soft failures, and give AC12 a
verification mechanism that doesn't depend on the UI — because the UI structurally cannot reach it.
**Design:** `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §7.1 (rewritten 2026-08-11, user-approved).
**Dates:** 2026-08-11.
**Suites at close:** ctest **413/413** · macOS interop **56/56** (from 53) · **TEST SUCCEEDED**, 0 warnings.

> *(SP-095–SP-100 belong to EP-031; this sprint took the next free number rather than colliding.)*

### Why this sprint existed

The user reported EP-030 **AC12** — *"one card's failure never blocks the stack"* — as **unverifiable live**
during SP-094's pass. That was correct, and reading the code to find out why surfaced two problems:

1. **The framework guarantee was never implemented.** `CardBodyBoundary` applied a frame and nothing else.
   The per-card half (`CardErrorView`) works but **predates T-0368**, the task meant to deliver the guarantee.
   The comment above the call site asserted the guarantee as delivered — it was false.
2. **The AC was unachievable as written.** SwiftUI cannot catch a trapping view body; a trapping card
   terminates the process regardless of any wrapper.

### AC12 as rescoped (user-approved 2026-08-11)

A card that **fails to load or produce its content** must not prevent the rest of the stack rendering, and
must show an inline warning in place of its content. **Hard failures are explicitly out of scope** — caught
by tests and code review, never absorbed at runtime.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0399 | `[Apple]` Card soft-failure isolation as a framework guarantee + failing-card test fixture | ✅ **Verified 2026-08-11** (user-accepted on test evidence) |

### What shipped

- **`InspectorCard.makeContent(context:) throws`** (`InspectorCard.swift`) — opt-in throwing variant with a
  default forwarding to `body(context:)`, so every existing card is untouched.
- **`AnyInspectorCard.body` now throws**, carrying a soft failure to the framework boundary.
- **`CardBodyBoundary` is a real boundary** (`InspectorCardStackView.swift`) — a throwing card renders
  `CardFailureView` **in place of its content**; the rest of the stack renders normally. The false comment
  was replaced with an accurate one naming the scope limit.
- **Three test fixtures** (`ScriviInteropTests.swift`) — `FailingCard` / `HealthyCard`, plus a three-card
  stack asserting 2 built / 1 failed.

**Layering kept deliberately:** cards that report their own load errors still do — that message is specific
and better than a generic one. The framework fallback is the **backstop** for cards that don't, so isolation
never depends on a card author remembering.

### pbxproj change — worth knowing about

The test target was compiling `ScriviEngine.swift` and `ScriviError.swift` **directly** *and* using
`@testable import ScriviApp`. That gave the target its own duplicate `ScriviEngine` type, distinct from the
app's — so a test-constructed engine could not be passed to any app-side API (`CardContext` among them).
**This is what made the fixture unwritable**, and it would have blocked any future test touching an app type
that takes an engine.

Both files were removed from the target's Sources phase; it now relies on the `@testable import` it already
used for every other app type. All 53 pre-existing tests still pass, so nothing depended on the duplicates.
**No new source files**, so no pbxproj additions were required.

### Exit criteria

- [x] A card that throws while building content renders a warning **in place of its content**, not a blank
      body, and does not prevent other cards rendering.
- [x] A card that never adopted the throwing variant is completely unaffected.
- [x] The behaviour is covered by a **test-only failing-card fixture** — the only mechanism that can reach it.
- [x] `ctest` + interop suites green; app builds and launches clean. **413/413 · 56/56 · TEST SUCCEEDED.**
- [x] **EP-030 AC4 re-verified** and the Epic put forward for close. ✅ **EP-030 closed 2026-08-11.**

### Verification note

⚠️ **AC12 is not live-verifiable** — there is no UI path to make a card fail. Verification is the test
fixture, and the user **explicitly accepted that as the evidence** (2026-08-11). Do not re-open this
expecting a live-verify record; none can exist.

### Lesson

**A criterion the app's UI structurally cannot exercise needs its test fixture written *with* the feature,
never deferred to a verification sprint.** AC12 sat unchecked through SP-092 and SP-094 and was marked
"Implemented" in the former, because live verification was the only plan for it. A comment asserting a
guarantee is not evidence the guarantee exists.
