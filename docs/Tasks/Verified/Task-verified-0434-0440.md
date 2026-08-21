# Verified Tasks: T-0434 – T-0440

**SP-117** — `[Cross]` Detail Sheet shell: pane, navigation, fields, save (EP-034).
✅ **All seven Verified 2026-08-21 (user-approved)** and archived in the same step.

⚠️ **The first sprint of EP-034 that shipped anything a writer can click.** Two sprints had closed before it
with their criteria met at the ABI and **nothing reachable in the product** — the very defect the Epic
exists to cure.

| ID | Title | Sprint | Priority | Status |
| -- | ----- | ------ | -------- | ------ |
| **T-0434** | **D1-E** — Detail Sheet pane; ⚠️ **host-independent** | SP-117 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0435** | **D2-B** — back/forward history | SP-117 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0436** | ⚠️ **Typed Swift object model** — none exists today | SP-117 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0437** | **D3-A** — fields; ⚠️ **save by PATCH, never reconstruct** | SP-117 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0438** | **R7** — double-click **and** right-click → "View Detail" | SP-117 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0439** | **R9** — pending: read-only, **explained**, never written | SP-117 | Medium | ✅ **Verified (2026-08-21)** |
| **T-0440** | ⚠️ **T-0420's owed surface** — explain `unsupportedWorldFormatVersion` | SP-117 | Medium | ✅ **Verified (2026-08-21)** |

---

## What SP-117 delivered

| Task | Outcome |
| ---- | ------- |
| T-0434 | `ObjectDetailSheet` — **D1-E**: editor-level, non-modal, hosted in `EditorView` beside the manuscript. ⚠️ Not the 280pt inspector (the width D1-C was rejected over); not a window (EP-018 documents that cost) |
| T-0435 | `ObjectDetailHistory` — **D2-B**: stack + cursor, back **and** forward, browser truncation. ⚠️ Re-visiting the current object is a NO-OP, or "back" appears dead (the I-0132 shape) |
| T-0436 | `ObjectDetail` — ⚠️ **the app's FIRST typed object model.** `subtitle`/`notes` had existed in the core since SP-095, surfaced **nowhere** |
| T-0437 | Fields + ⚠️ **patch-based save** — never reconstruct |
| T-0438 | **R7** — double-click **and** right-click → "View Detail", routed through `CardContext` as a **request** |
| T-0439 | **R9** — pending objects read-only **and explained** |
| T-0440 | ⚠️ **T-0420's surface, owed since SP-115** — finally paid |

### ⚠️ T-0440 forced a ScriviCore change, and finding out why was the sprint's most useful discovery

**It could not be done in Swift at all.** `WorldStore::resolve` **discarded** the reason: T-0420's own
comment said the reason *"reaches the writer through the parse error's `unsupportedWorldFormatVersion`
detail"* — but **`resolve()` returns a STATUS, not an error**, so the detail died at `WorldStore.cpp:313`
and **no envelope carried it**. ⚠️ **The app could not have explained it however it was written.**

> ⚠️ **`capability_without_surface` caused by a MISSING FIELD rather than a missing view** — a new shape of
> the Epic's signature defect. Verified at the core in SP-115, invisible in the product for **two sprints**,
> and the facade test asserting it **passed the whole time**, because it stops at `resolve()`.

✅ **User ruled `statusReason` into the ABI** — additive, empty means "no further detail". Explained **once**
by `WorldEntry.unavailabilityExplanation`, ⚠️ **never by string comparisons at call sites**. Boundary-tested.

### ⚠️ What the LIVE CLICK-THROUGH found — and what it validated

**The user ran S9 on the real rig.** Entry points, fields, persistence, back/forward, Save-button state,
drive-removal messaging and drive-return recovery: **all verified by use.**

⚠️ **It also found I-0148**, reported as an *observation* inside an otherwise glowing report:
*"When the disk is unmounted, the Notes field is still editable."*
`.disabled()` does not make a `TextEditor` read-only. ✅ **Never a write-safety bug** — Save is hidden when
read-only — ⚠️ **but typing during an outage was silently discarded on navigation**, since `load()`
overwrites the draft.

> ⚠️ **My first assessment ("R9 violated") was too strong and the user corrected it.** The guarantee held;
> the cost was lost typing, which neither of us had named at first.

✅ **And the design ruling was validated by SEEING it:** *"I was expecting a popup… I wasn't able to grasp
the full interaction before I saw it."* ⚠️ **A trade can be ruled correctly on paper and still not be
understood until it exists** — the strongest argument yet against another core-only sprint.

### Verification

**S4 proven NON-VACUOUS** — swapping the patch for a reconstruction fails it exactly where it should
(`image` gone). **S8 verified mechanically** — `grep` for `@Environment`/`focusedSession`/`env.` across all
three new files returns only the comment describing the rule, so **D1-B's window successor stays available
without a rewrite**.

**Suites:** `ctest` **554/554** macOS arm64 · x86-64 · ASan/UBSan · **Linux 558 cases / 9332 assertions**
(GCC 13) · interop **115/115 in 12 suites** (was 107) · app **BUILD SUCCEEDED**.

⚠️ **Two of my own defects were caught only by the compiler**: the sheet's state landed inside an
`#if os(iOS)` block — so it did not exist on **macOS, the platform it ships on** — and a mis-ordered
`CardContext` argument.

### ⚠️ Carried out of SP-117

| Item | Owner |
| ---- | ----- |
| ⚠️ **`tags` deferred** (user ruling) — so **R2 is not fully met and AC2 CANNOT CLOSE** | **SP-119** |
| **AC3–AC8** — images, relationships, sources | **SP-118 – SP-120** |
| **AC9** — a world going away *while a sheet is open* is handled; ⚠️ **the read-only half is now verified**, the rest belongs with images | **SP-119** |

---

*Last Updated: 2026-08-21 (**T-0434–T-0440 ✅ Verified (user-approved) and archived in the same step**.)*
