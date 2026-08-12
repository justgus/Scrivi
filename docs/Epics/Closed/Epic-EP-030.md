# Closed Epic: EP-030

## EP-030: [Apple] Scene Inspector Card Framework (base cards)

**Codebase:** `[Apple]` — the framework + writing-tool cards are SwiftUI; the ScriviCore work was minimal
(scene-notes fields for the writing cards, plus `scrivi_history_get_tree` for the history card).

**Status:** ✅ **CLOSED (Human-approved 2026-08-11).** All acceptance criteria **AC1–AC7 Verified** across
4 sprints (SP-090, SP-091, SP-092, SP-101). ctest **413/413** · macOS interop **56/56** · TEST SUCCEEDED.

**Goal:** Replace the placeholder `SceneInspectorView.swift` (57 lines: one tab case, a stub body, fixed
280pt) with the approved card framework — bottom tabs (`Writing | Worldbuilding | Properties`), per-scene card
stacks persisted in `inspector-layout.json`, and the writing-tool cards. Folded in EP-019's history panel
(T-0215) as the `history` card, **which is what let EP-019 close without waiting for the object model.**

**Design:** `docs/Scrivi_Scene_Inspector_Card_Framework_v0_1.md` ✅ Approved 2026-08-05 (C1=C, C2=B, C3=A
within tabs, C4=A, C5=C + in-stack creation, C6=B with sort per-stack); **§7.1 amended 2026-08-11** (see
below).

**Depends on:** nothing unbuilt. Deliberately sequenced *before* EP-031 so EP-019 could close first.

**Date Created:** 2026-08-05
**Actual Close Date:** 2026-08-11

---

## Acceptance Criteria — all Verified

| AC | Criterion | Verified |
| -- | --------- | -------- |
| AC1 | Bottom tabs in order `Writing \| Worldbuilding \| Properties`; each stack tab a scrolling column of collapsible cards; one-card-per-tab layout gone | ✅ 2026-08-11 |
| AC2 | Tab selection persists **at project level**, unchanged by scene switches; tab view hides/shows from a menu item | ✅ 2026-08-11 |
| AC3 | Card stacks per-scene in `inspector-layout.json`, with "apply to all scenes"; Worldbuilding ships empty, Writing ships with empty `tags`/`outline`/`todo` | ✅ 2026-08-11 |
| AC4 | Sort per-**stack**; unknown `typeID` skipped with a notice; **a card that fails to load its content never blocks the stack** *(AC12 rescoped — see below)* | ✅ 2026-08-11 |
| AC5 | The `history` card delivers T-0215's behavior (windowed tree, branch selection, stale badges, purge) | ✅ 2026-08-11 |
| AC6 | Properties tab renders author/timestamps/metrics, is **not** a card stack, and is **field-driven** | ✅ 2026-08-11 |
| AC7 | No regression: editor, scene navigation, timeline, undo/redo, external-change scan; suites green | ✅ 2026-08-11 |

**AC4 evidence note:** AC10 (per-stack sort) and AC11 (unknown `typeID`) were verified **live**. AC12
(soft-failure isolation) was verified **on test evidence, user-accepted 2026-08-11** — there is no UI path to
make a card fail, so a live-verify record for it does not and cannot exist.

**AC5 amendment:** I-0108 (SP-093) moved stale-branch **purge** out of the card and into Project Settings —
one irreversible operation, one entry point. The card shows a **badge only**. AC5 was verified against that
amended behaviour, and branch selection was exercised for the first time in the closing pass.

---

## ⚠️ AC12 was rescoped to soft failures — the original was not achievable

**Original wording:** *"one card's failure never blocks the stack."*

**Two problems surfaced in the closing verification pass (SP-094):**

1. **The framework guarantee was never implemented.** T-0368 (SP-092) was to promote failure isolation from a
   per-card courtesy to a framework guarantee. Only the per-card half existed, and it **predated the task**
   (`CardErrorView`, SP-091). `CardBodyBoundary` applied a frame and nothing else — no error handling, no
   fallback — so a card that did not self-report had no backstop. The comment above its call site asserted
   the guarantee as delivered; **it was false**.

2. **The AC was unachievable as written.** SwiftUI **cannot catch a trapping view body** — there is no
   `try`/`catch` equivalent around a `View`, and a runtime trap terminates the process. No parent view,
   boundary, or wrapper can contain it.

**Rescoped (user-approved 2026-08-11), Doc 2 §7.1 rewritten:** a card that **fails to load or produce its
content** must not prevent the rest of the stack rendering, and shows an inline warning in place of its
content. **Hard failures are explicitly out of scope** — a trapping card is a defect in that card, caught by
tests and review, never absorbed at runtime.

**Implemented as T-0399 (SP-101):** throwing `makeContent` on the card protocol (default forwards to `body`,
so existing cards are unaffected), a real `CardBodyBoundary` rendering `CardFailureView` in place of failed
content, and three test fixtures. **T-0368 is closed as delivered by T-0399** — the specified work landed,
under a later number and against the narrowed AC; only the unachievable hard-failure clause was dropped.

**Layering kept deliberately:** cards that report their own load errors still do — that message is specific
and better. The framework fallback is the **backstop** for cards that don't, so isolation never depends on a
card author remembering.

---

## Sprints

| Sprint | Title | Closed |
| ------ | ----- | ------ |
| SP-090 | Framework: card protocol, registry, `inspector-layout.json`, tabs | ✅ 2026-08-05 |
| SP-091 | Writing-tool cards: tags, todo, outline (`sources` deferred → EP-031) | ✅ 2026-08-05 |
| SP-092 | `[Cross]` `history` card (folds in T-0215) + Properties tab + `scrivi_history_get_tree` | ✅ 2026-08-11 |
| SP-094 | EP-019 + EP-030 verification & Epic close (merged) | ✅ 2026-08-11 |
| SP-101 | `[Apple]` AC12 soft-failure isolation (T-0399) | ✅ 2026-08-11 |

---

## The lesson worth carrying forward

**A criterion the app's UI structurally cannot exercise needs its test fixture written *with* the feature —
never deferred to a verification sprint.**

AC12 sat unchecked through two sprints and was marked "Implemented" in a third, because live verification was
the only plan for it and live verification could not reach it. It surfaced only when the user reported the
criterion as unverifiable and the code was read to find out why. The false "FRAMEWORK guarantee" comment had
been sitting above a no-op the entire time — a comment asserting a guarantee is not evidence the guarantee
exists.

**Corollary:** when an AC turns out to be unachievable on the platform (here: catching a trapping SwiftUI
view body), say so and rescope it explicitly. Leaving it in place would have meant either shipping a knowingly
unmet criterion or building a "boundary" that could never work.
