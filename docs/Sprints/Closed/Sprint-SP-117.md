# SP-117 — `[Cross]` Detail Sheet shell: pane, navigation, fields, save

✅ **CLOSED 2026-08-21 (user-approved).** EP-034's **third** sprint, and ⚠️ **the first to ship anything a
writer can click.**

**All seven Tasks ✅ Verified** → [`../Tasks/Verified/Task-verified-0434-0440.md`](../Tasks/Verified/Task-verified-0434-0440.md).
**I-0148 ✅ Verified** → [`../Issues/Verified/Issue-verified-0141-0150.md`](../Issues/Verified/Issue-verified-0141-0150.md).

**Suites at close:** `ctest` **554/554** macOS arm64 · x86-64 · ASan/UBSan · **Linux 558 cases / 9332
assertions** (GCC 13) · interop **115/115 in 12 suites** · app **BUILD SUCCEEDED**.

⚠️ **Verified BY USE on the real rig** — see the live click-through section below.

---

## SP-117: `[Apple]` Detail Sheet shell — pane, navigation, fields, save

**Status:** ✅ **CLOSED 2026-08-21 (user-approved)**
**Epic:** [EP-034: `[Cross]` Object Detail & Media](../Epics/Epic-active.md) — its **third** sprint
**Goal:** Give the writer a **first-class, non-modal surface** where she can see and edit what an object
actually is — and **reach it**. ⚠️ **This is the first sprint of EP-034 that ships anything a writer can
click.**
**Start Date:** 2026-08-21
**End Date:** 2026-08-21
**Trades implemented:** ✅ **D1-E** (non-modal editor-level pane) · ✅ **D2-B** (explicit back/forward
history) · ✅ **D3-A** (plain `TextEditor` for `notes`)

---

### ⚠️ Why this sprint matters more than its size suggests

**EP-034 exists because of `project_capability_without_surface`** — the entire object-editing capability
shipped and was Verified while the app exposed a single `TextField("Name")`. **Two sprints have now closed
without a writer being able to reach any of it.**

> ⚠️ **SP-115 and SP-116 were both core-and-boundary sprints.** Their success criteria were met at the ABI,
> and **not one of them proved a writer could do anything.** That was correct for those sprints and is
> stated in both archives — but it means ⚠️ **SP-117 is where EP-034 starts paying out**, and where its
> ACs first become verifiable *by use* rather than by envelope.

⚠️ **Consequence for verification:** SP-115 and SP-116 each produced defects that only live use found
(I-0137, I-0142, I-0146, I-0147 — **four across the Epic**). This sprint's surface is the first that can be
click-tested, so ⚠️ **a live click-through is a success criterion here, not a nicety.**

---

### ⚠️ The constraint that shapes every file this sprint creates

> **D1-E ruling:** *"it must be built as a **self-contained, navigable component that does not depend on
> its host**. That is what keeps B (a window) available later without a rewrite."*

⚠️ **This is easy to honour on day one and easy to lose by day three.** A view that reaches for
`focusedSession`, the editor's environment, or the inspector's 280pt geometry is host-*dependent* the moment
it does so — and the cost is not visible until someone tries to put it in a window.

✅ **It is therefore a success criterion (S8), not a note.**

⚠️ **There is no auxiliary window in Scrivi today** — grep shows `.sheet` and `.popover` only. **D1-E was
ruled specifically to avoid buying a second window type inside an Epic already carrying nine trades**, and
EP-018's registry, dead-window cache, `.onOpenURL` unreliability and terminate-on-last-window override are
the receipts for what that costs.

---

### Assigned Tasks

| ID | Title | Codebase | Priority | Status |
| -- | ----- | -------- | -------- | ------ |
| **T-0434** | **D1-E** — the Detail Sheet **pane**: editor-level, non-modal, ⚠️ **host-independent** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0435** | **D2-B** — explicit **back/forward** history object | `[Apple]` | **High** | ✅ **Verified** |
| **T-0436** | ⚠️ **A typed object model in Swift** — `openObject` returns raw JSON today | `[Apple]` | **High** | ✅ **Verified** |
| **T-0437** | **D3-A** — view + edit `displayName`, `subtitle`, `notes`; ⚠️ **save by PATCH, never reconstruct** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0438** | **R7** — entry points: **double-click** *and* **right-click → "View Detail"** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0439** | **R9** — a **pending** object opens **read-only, explained, never written** | `[Apple]` | **Medium** | ✅ **Verified** |
| **T-0440** | ⚠️ **T-0420's owed surface** — explain `unsupportedWorldFormatVersion` to the writer | `[Apple]` | Medium | ✅ **Verified** |

### Assigned Issues

**None open.** ⚠️ **I-0147 remains an Accepted limitation** (network-worlds design) and is **not** SP-117's.

---

### Task detail

#### T-0434 — the pane (D1-E)

Editor-level, **non-modal**, sized as a document sub-view. ⚠️ **NOT the 280pt inspector strip**
(`SceneInspectorView.swift:7,46`) — that width is the reason D1-C was rejected, and it is wrong for Mara's
backstory. The manuscript is hidden or narrowed while the writer is cultivating; that is §1's mode switch
made visible.

⚠️ **Host-independence is the whole point of the ruling.** The pane takes what it needs as parameters —
project root, object identity, an engine — and reads **nothing** from its host's environment.

#### T-0435 — back/forward (D2-B)

An explicit history object: a stack plus a cursor, **back and forward**. ⚠️ **`NavigationStack` gives back
but not forward** (D2-A), and forward needs exactly this machinery anyway, so it is built explicitly.

✅ **D1-agnostic by construction** — the same object serves a pane now and a window later.

#### T-0436 — ⚠️ a typed object model, because there isn't one

`OpenObjectResult` carries **`objectJson: String`** (`ScriviEngine.swift:1713-1716`) — the app has **no
typed object model at all**. Every field the sheet shows has to come from somewhere, and hand-parsing JSON
in a View is how fields get dropped.

⚠️ **`subtitle`, `notes` and `tags` exist in the core** (`ObjectTypes.hpp:209,222,223`) **and are surfaced
nowhere in Swift** — grep confirms. **This is the largest instance of `capability_without_surface` in the
Epic**, and T-0436 is what ends it.

#### T-0437 — the fields (D3-A), and ⚠️ THE HIGHEST-RISK CODE IN THIS SPRINT

`displayName`, `subtitle`, `notes` — three plain strings. `notes` gets a plain `TextEditor`
(D3-A). ⚠️ **D3-C was RULED OUT, not deferred**: reusing the manuscript editor would drag EP-019's
sentence-granular history engine into object editing.

> ⚠️ **SAVE BY PATCHING THE OPENED JSON. NEVER RECONSTRUCT THE OBJECT.**
>
> `rename()` already does this and says why (`ObjectCard.swift:286-293`): *"Patch displayName in the
> object's own JSON rather than reconstructing it, so every field this build does not know about survives
> the round trip."*
>
> ⚠️ **A sheet that reconstructs would silently destroy `image`, `thumbnailAssetID` and every field added
> after this build** — and the loss would be invisible until a writer noticed her portrait gone. **S4 tests
> exactly this.**

⚠️ **`tags` is DEFERRED to SP-119** (user ruling at planning): it is a string **array** with no UI
precedent anywhere in the app, and a chip/token editor carries its own design question. **R2 is therefore
NOT fully met by SP-117** — recorded so the gap is deliberate rather than discovered.

#### T-0438 — entry points (R7)

**Double-click** *and* **right-click → "View Detail"** on Kind Card list items (user ruling at planning:
both, per R7).

⚠️ **A SINGLE CLICK MUST KEEP DOING WHAT IT DOES TODAY** — `.onTapGesture { if !entry.pending { onEdit() } }`
(`ObjectCard.swift:906`) opens the inline editor, and **Q-b makes the Detail Sheet ADDITIVE, replacing
nothing.**

⚠️ **Layering double-click over an existing single-click tap is the I-0132 class of bug** — that Issue took
**four attempts** and three misdiagnoses, and the lesson recorded was *what is the source of truth, and does
it propagate?* rather than reaching below SwiftUI. **Do not win this with an `NSEvent` monitor.**

#### T-0439 — pending objects (R9)

A world-unavailable object opens **read-only**, ⚠️ **says why**, and is **never written**. Doc 3: *absence
is never deletion*. ⚠️ **I-0138's rule is disabled-AND-EXPLAINED** — a greyed control that does not say why
is the defect, not the fix.

#### T-0440 — ⚠️ the surface SP-115 owed and SP-116 could not take

**T-0420 shipped `unsupportedWorldFormatVersion` at the core and Verified it there. Nothing surfaces it**,
so a writer opening a too-new world sees *"unavailable"* with **no explanation**.

⚠️ **It has been unowned across two sprints** because neither shipped UI. **SP-117 is the first sprint that
can take it, so it does.** ⚠️ **Leaving it a third time would make it precisely the defect this Epic was
created to cure.**

---

---

### ⚠️ Implementation status — 2026-08-21

**All seven Tasks implemented.**

| Leg | Result |
| --- | ------ |
| `ctest` macOS **arm64** | ✅ **554/554** (was 552) |
| `ctest` macOS **x86-64** | ✅ **554/554** |
| `ctest` **ASan + UBSan** | ✅ **554/554** |
| **Linux** under GCC 13 | ✅ **558 cases / 9332 assertions** |
| macOS **interop** | ✅ **115/115 in 12 suites** (was 107 — **+8**) |
| macOS **app** | ✅ **BUILD SUCCEEDED** |

⚠️ **S4 proven NON-VACUOUS** — replacing the patch with a reconstruction fails it exactly where it should
(`image` gone). **S8 verified mechanically**: `grep` for `@Environment`/`focusedSession`/`env.` across all
three new files returns only the comment describing the rule.

#### ⚠️ SCOPE CHANGE — SP-117 is `[Cross]`, not `[Apple]`

**T-0440 could not be done in Swift at all**, and finding out why is the sprint's most useful discovery.

⚠️ **`WorldStore::resolve` DISCARDED the reason.** T-0420 (I-0136) made a too-new package report
`unavailable` rather than the forbidden `missing`, and its comment says the reason *"reaches the writer
through the parse error's `unsupportedWorldFormatVersion` detail"* — but **`resolve()` returns a STATUS, not
an error**, so that detail died at `WorldStore.cpp:313` and **never crossed the ABI**. No envelope carried
it. **The app could not have explained it however it was written.**

> ⚠️ **This is `capability_without_surface` caused by a MISSING FIELD rather than a missing view** — a new
> shape of the Epic's signature defect. The fix was Verified at the core in SP-115 while remaining
> invisible in the product for **two sprints**, and the facade test asserting it **passed the whole time**,
> because it stops at `resolve()`.

✅ **User ruled: add `statusReason` to the ABI.** Additive — empty means "no further detail", so every
existing caller is unaffected. Carried on `WorldResolution` and `WorldSummary`, emitted from both world
envelopes, decoded in `WorldEntry`, and turned into a sentence **once** by
`unavailabilityExplanation` — ⚠️ **not by string comparisons at call sites**, which would be the
restated-list defect in another costume.

⚠️ **Boundary-tested, not facade-tested** (`feedback_boundary_tests_not_facade`).

#### What shipped

- **T-0434** `ObjectDetailSheet` — editor-level, non-modal, hosted in `EditorView` beside the manuscript.
  ⚠️ **Not in the 280pt inspector** (the width D1-C was rejected over) and **not a window** (D1-B's cost is
  documented in EP-018).
- **T-0435** `ObjectDetailHistory` — stack + cursor, back **and** forward, with the browser truncation rule.
  ⚠️ **Re-visiting the current object is a NO-OP** — without that, opening the same sheet twice stacks
  duplicates and "back" appears dead, the shape of I-0132.
- **T-0436** `ObjectDetail` — ⚠️ **the app's first typed object model.** `subtitle` and `notes` have existed
  in the core since SP-095 and were surfaced **nowhere**.
- **T-0437** Fields + ⚠️ **patch-based save**.
- **T-0438** Double-click **and** right-click → "View Detail", routed through `CardContext` as a **request**
  — a card inside a 280pt strip cannot present an editor-level pane.
- **T-0439** Pending → read-only **and explained**, per I-0138's disabled-AND-explained rule.
- **T-0440** The debt from SP-115, finally paid.

#### ⚠️ Two defects in my own work, caught by the compiler

1. ⚠️ **The Detail Sheet's state landed inside an `#if os(iOS)` block**, so it did not exist on **macOS —
   the platform this sprint ships on.** The build caught it; a less strict language would have given a
   runtime surprise.
2. **`CardContext` argument order** — a mis-ordered call site. Trivial, but it is the second time this
   sprint that "it compiles" was the only thing standing between me and a wrong answer.

---

### ✅ THE LIVE CLICK-THROUGH — 2026-08-21 (S9), and the defect it found

**The user ran it. Everything below is verified BY USE, not by a suite.**

| Checked | Result |
| ------- | ------ |
| Entry + navigation | ✅ *"I love the interaction… I love the navigation push."* |
| Fields presented | ✅ |
| Persistence | ✅ *"Added data is saved and persists."* |
| Back / forward | ✅ |
| Save button state | ✅ enables on change, disables after save |
| ⚠️ Drive removed | ✅ *"Eskandar is unmounted, so this object is read only until it is available again"* — **named the world, said why, in words** (I-0138's disabled-AND-explained rule) |
| ⚠️ Drive returns | ✅ Save button returns to its correct state |
| Quit with the sheet open | ✅ Does not reappear on relaunch — ⚠️ **user-ruled acceptable: the Detail Sheet is TRANSIENT** and restoration is not required |

> ⚠️ **The design ruling was validated by seeing it, not by reasoning about it.** The user, who chose D1-E:
> *"I was expecting a popup. But I love the navigation push. I know it's what I asked for, but I wasn't
> able to grasp the full interaction before I saw it."*
>
> ⚠️ **That is an argument for shipping a surface early**, and against another core-only sprint: a trade can
> be ruled correctly on paper and still not be *understood* until it exists.

#### ⚠️ I-0148 — reported as an observation, not a defect

> *"When the disk is unmounted, the Notes field is still editable."*

That sentence sat inside an otherwise glowing report, ⚠️ **and it was a real defect.**
`.disabled()` **does not make a `TextEditor` read-only** — the two `TextField`s honoured the identical
modifier and the `TextEditor` ignored it. ⚠️ **This is the app's first disabled `TextEditor`**; the only
other one (`WritingToolCards.swift:209`) is never disabled, so nothing would have exposed it.

✅ **My first assessment — "R9 violated" — was too strong, and the user corrected it.** `Save` is hidden
entirely when read-only, so **nothing could ever reach disk** and R9's *"never written"* guarantee held.

⚠️ **But the real cost was LOST TYPING**, which neither of us had named: `load()` overwrites `draftNotes`
from disk, so a paragraph typed during an outage vanished **silently** on navigating away and back — no
prompt. That is precisely the class the inline editor already guards (I-0126).

✅ **User ruled: disable it**, for simplicity and consistency over inventing draft-retention rules. Notes now
renders as **selectable text** when read-only, so the state is visible rather than merely enforced.

⚠️ **FIFTH defect in EP-034 found by use rather than tests** (I-0137, I-0142, I-0146, I-0147, I-0148) — and
the first reported without being recognised as one. **A satisfied user is not a green suite; the
observation mattered more than the verdict attached to it.**

### Success criteria

Written to be checkable **by use** — this is the first sprint of EP-034 where that is possible.

| # | Criterion |
| - | --------- |
| **S1** | From a Kind Card list item, **double-click** opens the Detail Sheet, **and** right-click → **"View Detail"** opens it. ⚠️ **A single click still opens the inline editor**, unchanged. |
| **S2** | The sheet **displays** `displayName`, `subtitle` and `notes` for an existing object — ⚠️ including objects whose `subtitle`/`notes` were written by SP-095's core and **have never been visible in the app**. |
| **S3** | Editing any of the three and saving **persists across an app restart**. |
| **S4** | ⚠️ **A save preserves fields the sheet does not display.** Set an object's `image.assetID` on disk, edit `notes` in the sheet, save → **`image.assetID` survives**. ⚠️ **This is the reconstruct-vs-patch defect, and it is the one most likely to ship silently.** |
| **S5** | **Back and forward** move through the navigation history; ⚠️ **forward is reachable after going back**, and both disable correctly at the ends. |
| **S6** | ⚠️ A **pending** object opens **read-only and EXPLAINED** — the reason is words on screen, not a greyed control — and ⚠️ **no write reaches disk** (verify the file's mtime/hash is unchanged). |
| **S7** | ⚠️ A world that is **too new** (`unsupportedWorldFormatVersion`) is **explained to the writer**, not reported as a bare "unavailable" (T-0440). |
| **S8** | ⚠️ **The pane is HOST-INDEPENDENT**: it reads nothing from `focusedSession`, the editor environment, or inspector geometry, and takes everything it needs as parameters. **Demonstrated by instantiating it in a test/preview with no host.** |
| **S9** | ✅ **DONE 2026-08-21 — and it found I-0148.** ⚠️ **A LIVE CLICK-THROUGH on the real rig** (`tintagael` + `Eskandar` on USB): open Myton's sheet, read his fields, edit `notes`, restart, confirm. ⚠️ **Four defects in this Epic were found only by live use** — this is not optional. |
| **S10** | `ctest` green (arm64 · x86-64 · sanitizers) · Linux green · interop green · app **BUILD SUCCEEDED**. |

⚠️ **S2 and S4 are the two that would most easily pass vacuously.** S2 needs an object that *actually has* a
subtitle or notes on disk — a blank field proves nothing. S4 needs a real `image.assetID` present *before*
the edit. **Fixtures must be prepared deliberately, not assumed.**

---

### ⚠️ Testing — where this sprint's evidence has to come from

⚠️ **The interop suite cannot test a SwiftUI pane.** SP-116's boundary lesson does not transfer here; this
sprint's risk is in the app layer, and its evidence is:

1. **Interop tests** for T-0436's typed model and ⚠️ **especially T-0437's patch-preservation (S4)** — that
   one is pure data handling and *is* testable without a UI. **Write it as a test, not a click.**
2. ⚠️ **A live click-through (S9)**, because the rest is not.

⚠️ **Do not claim a criterion is met because a suite named after it passes** — SP-115's I-0137 did exactly
that, and the feature could not fire.

---

### Sprint Notes

- ⚠️ **R2 is NOT fully met** — `tags` deferred to SP-119 by user ruling. **AC2 therefore cannot close in
  SP-117**; it closes when tags ship.
- ⚠️ **T-0416 may surface as a live blocker in SP-118**, not here — seeded relation types never reach
  existing projects. **Worth confirming on the rig while it is attached for S9.**
- **AC1, AC9 and the T-0420 debt are the ACs in play**; **AC3–AC8 belong to SP-118–SP-120.**
- ⚠️ **Every new `.swift` file MUST be added to `Scrivi.xcodeproj/project.pbxproj` in the same step**
  (CLAUDE.md, non-negotiable). ⚠️ **SP-116 added one file across 3 build-file entries + a file ref + a group
  + 3 Sources phases** — the shape to copy.
- ⚠️ **`pgrep Scrivi` before `xcodebuild test`** — a running instance blocks the runner
  (`feedback_check_scrivi_running_before_tests`).

### Retrospective

*(Filled in at close.)*

---

## Previously: SP-116 ✅ CLOSED 2026-08-21 (user-approved)

### What it delivered

EP-034's **second** sprint: three design trades (**D5, D6, D7**), **four Issues fixed**, and ⚠️ **two more
found while fixing them**.

**All eight Tasks and all six Issues ✅ Verified.** Scope grew from five Tasks to eight — **every addition
came from a defect found during the work**, none from re-planning.

| Task | Delivered |
| ---- | --------- |
| T-0426 | **D6** — world assets live in the `.scrivworld` package and **travel with the world** |
| T-0427 | **D7** — `assetPath` from `list_assets` |
| T-0428 | **I-0143** — the assets array routes through `JsonDoc` |
| T-0429 | **D5** — `scrivi_list_object_kinds`, **derived**, and adopted in Swift |
| T-0430 | **I-0141** — `scrivi.h` states the rule by reference |
| T-0431 | ⚠️ **I-0144** — `WorldWriteGuard` locks **every** world-package write path |
| T-0432 | ⚠️ **Block transfer + per-block watchdog** (user ruling) |
| T-0433 | ⚠️ **I-0146** — stale-lock sweep of abandoned `*.partial` files |

**Suites at close:** `ctest` **552/552** macOS arm64 · x86-64 · ASan/UBSan (**was 525**) · **Linux 556
cases / 9300 assertions** (GCC 13) · interop **107/107** · app **BUILD SUCCEEDED**.

---

## ⚠️ The lesson this sprint proved — twice more

**Four defects in EP-034 have now been found by USE, not by tests** (I-0137, I-0142, I-0146, I-0147). Two
were in this sprint.

1. ⚠️ **I-0144 — a lock that shipped complete, correct, unit-tested, and was NEVER CALLED.** Every object
   write into a shared world was unserialised for three sprints. **A green suite for a capability says
   nothing about whether anything invokes it.**
2. ⚠️ **I-0146 — a USB drive physically pulled mid-import left a 459 MB `.partial`** that no Scrivi
   operation could ever reclaim. **The lab test asserting "no partial remains" passed throughout** — it can
   only test failures the writing process *survives*.

> ⚠️ **And three times my own test setup was easier than reality** — the staged orphan omitted the matching
> fresh lock (twice), and the competing-writer rig used a clock whose heartbeat read as stale.
>
> **Staging the AFTERMATH of a failure is not staging the FAILURE.** It silently omits whatever else the
> failure leaves behind — here, the very thing that blocked the fix.

---

## ⚠️ Carried out of SP-116 — do not read as delivered

| Item | Owner |
| ---- | ----- |
| ⚠️ **No UI shipped**, by design. S1–S14 are core-and-boundary criteria — **AC3 and AC9 cannot close** without the Detail Sheet | **SP-117** |
| **T-0420's missing surface** — a writer opening a too-new world still sees *"unavailable"* with no explanation. ⚠️ Carried from **SP-115**, still unowned | ⚠️ **SP-117 is the first sprint that could take it** |
| **I-0147** — the 60 s window after an interrupted world write where the world is unwritable and its orphan unreclaimable. ✅ **Accepted** (user-ruled); a regression test asserts it | **Network-worlds design** |
| ⚠️ **`ObjectIndex::loadWorldIndex`'s rebuild is still UNLOCKED** — `WorldLock` is not reentrant and `save`/`remove` reach it while holding the lock | **Network-worlds design** (needs a reentrant lock) |

---

---

*Last Updated: 2026-08-21, fourth pass (✅ **ALL SEVEN TASKS VERIFIED (user-approved)** and archived →
`../Tasks/Verified/Task-verified-0434-0440.md`; ✅ **I-0148 Verified and archived** →
`../Issues/Verified/Issue-verified-0141-0150.md`. **SP-117 → 🟠 REVIEW** — ⚠️ **Claude cannot close a
Sprint**, so it awaits user approval. ✅ **AC1 met by use; AC10 already met (SP-115); AC9 HALF met** —
⚠️ **the "world goes away while a sheet is open" half is NOT demonstrated** and belongs with SP-119.
⚠️ **AC2 cannot close — `tags` deferred.** Prior note follows.)*

*Last Updated: 2026-08-21, third pass (✅ **S9 — THE LIVE CLICK-THROUGH IS DONE**, by the user, on the real
rig. Entry points, fields, persistence, back/forward, Save-button state, drive-removal messaging and
drive-return recovery **all verified by use**. ⚠️ **It found I-0148** — `.disabled()` does not make a
`TextEditor` read-only — ⚠️ **reported as an observation inside a positive report**, not as a complaint.
✅ **My "R9 violated" call was too strong and the user corrected it**: Save is hidden when read-only, so
nothing reached disk; the real cost was **silently lost typing** on navigation. Fixed, user-ruled. Interop
**115/115**. ⚠️ **The Detail Sheet is TRANSIENT** — user ruled no session restoration required. Prior note
follows.)*

*Last Updated: 2026-08-21, second pass (**ALL SEVEN TASKS IMPLEMENTED — 🟠 Not Verified.** ctest
**554/554** (arm64 · x86-64 · ASan) · **Linux 558/9332** · interop **115/115** (+8) · app **BUILD
SUCCEEDED**. ⚠️ **SP-117 became `[Cross]`**: T-0440 was impossible in Swift because `WorldStore::resolve`
discarded the reason and **no envelope carried it** — `capability_without_surface` caused by a **missing
field**. ✅ **User ruled `statusReason` into the ABI**; boundary-tested. ⚠️ **S4 proven non-vacuous**;
**S8 verified by grep**. ⚠️ **S9 (live click-through on the rig) NOT YET RUN** — that is the one criterion
no suite can supply. Prior note follows.)*

*Last Updated: 2026-08-21 (**SP-116 ✅ CLOSED, user-approved.** Eight Tasks + six Issues Verified and
archived in the same step. ⚠️ **Two defects found by live use** (I-0144, I-0146) and **one accepted
limitation** (I-0147). Active Sprints 1 → 0; EP-034 stays 🟡 Active.)*

*Last Updated: 2026-08-21 (**SP-117 🟡 ACTIVATED** — EP-034's third sprint and ⚠️ **the first to ship a
writer-facing surface**; seven Tasks T-0434–T-0440, no open Issues. ⚠️ **`tags` deferred to SP-119**, so
R2/AC2 cannot close here. ⚠️ **T-0440 takes T-0420's surface, unowned across two sprints.** Next available:
Sprint **SP-118** · Task **T-0441** · Issue **I-0148**.)*
