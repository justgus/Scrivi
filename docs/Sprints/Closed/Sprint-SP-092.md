# SP-092: `[Cross]` EP-030 — history card + Properties tab ✅ CLOSED

**Status:** ✅ **Closed 2026-08-11 (Human-approved)** — verified via SP-094's live pass.
**Epic:** EP-030 `[Apple]` — Scene Inspector Card Framework (3rd of 4 sprints).
**Goal:** The `history` card — which **absorbs EP-019's T-0215 and unblocked that Epic's close** — plus the
Properties tab and per-card failure isolation.
**Design:** `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §4.8/§7.1, AC12–AC15 ·
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §7 (ABI), §10 Trade T2 option B.
**Dates:** 2026-08-06 – 2026-08-11.
**Suites at close:** ctest **413/413** · interop **56/56** · macOS BUILD SUCCEEDED.

### Scope correction found at planning (2026-08-05)

**`scrivi_history_get_tree` did not exist** — deferred when EP-019 deferred its panel, and the panel then
moved here. The card cannot render a tree without it, so **SP-092 was `[Cross]`, not `[Apple]`**. The engine
work was already done (`HistoryService::nodes()/rootID()/currentNodeID()`; `EventNode` already carried
`parentID`, `primaryChildID`, `childIDs`, `kind`, `sceneID`, `timestamp`, `sessionID`, `bufferID`,
`barrierKind`/`barrierNote`), so `get_tree` was a **serialization + windowing layer over existing state**, not
new history logic.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0366 | `[Apple]` `history` card — windowed tree, branch selection, stale badges, purge (**supersedes T-0215**) | ✅ **Verified 2026-08-11** (EP-030 AC5) |
| T-0367 | `[Apple]` Properties tab — **field-driven** view (author, timestamps, metrics) | ✅ **Verified 2026-08-11** (EP-030 AC6) |
| T-0368 | `[Apple]` Card failure isolation + inline warning presentation | ✅ **Closed 2026-08-11 — delivered by [[T-0399]]** (see below) |

### Exit criteria at close

- [x] `scrivi_history_get_tree` returns a windowed tree; `maxNodes` honoured (ctest).
- [x] Swift decodes the tree with **absent arrays tolerated as empty** (I-0094 class).
- [x] The `history` card renders the windowed tree with primary spine and fork glyphs; clicking selects.
- [x] Stale branches badged; purge available with confirmation; purge survives relaunch.
      ⚠️ **Amended in-sprint by I-0108** — the card shows a **badge only**; purge moved to Project Settings
      (one irreversible operation, one entry point). Verified against the amended behaviour.
- [x] The card is **dismissible** and its position/collapsed state persist.
- [x] The **Properties** tab renders author, timestamps, and metrics, and is **not** a card stack (AC14).
- [x] Properties is **field-driven**; derived-read-only distinguished from no-editor-yet (AC15).
- [ ] ~~One card failing to load does not prevent others rendering (AC12).~~ **Carried to T-0399** (SP-101).
- [x] `ctest` + interop suites green; app builds and launches clean.

### ⚠️ T-0368 did not deliver its framework half — found in SP-094

T-0368 was to promote failure isolation from a **per-card courtesy to a framework guarantee**. Only the
per-card half existed, and it **predated the task** (`CardErrorView`, SP-091). `CardBodyBoundary` applied a
frame and nothing else — no error handling, no fallback — so a card that did not self-report had no backstop.
The comment above its call site claimed the guarantee as delivered; it was false.

This was invisible to live verification (**there is no UI path to make a card fail**), which is why it
survived this sprint's close criteria and SP-094's AC pass. It surfaced only when the user reported AC12 as
unverifiable and the code was read to find out why.

**Resolution — T-0368 ✅ closed 2026-08-11, delivered by T-0399.** AC12 was rescoped to soft failures
(user-approved 2026-08-11 — SwiftUI cannot catch a trapping view body) and T-0368's actual deliverable, the
framework guarantee, was implemented under **T-0399** in SP-101 with a test-only failing-card fixture.

> **This is not a waiver.** The specified work landed — a real backstop in `CardBodyBoundary`, verified by
> fixture — it simply landed under a later task number, and against a narrowed AC. The only part *dropped* is
> hard-failure containment, which was never achievable in SwiftUI and should not have been written into the
> AC. Anyone auditing T-0368 should read T-0399 as its completion record, not look for missing work.

### What this unblocked

**EP-019's close.** Its history-panel requirement (T-0215) is met by T-0366 and T-0216 was closed OBE, which
reduced SP-057 to pure verification — later merged into SP-094, where EP-019 reached AC-complete.

### Process note

A criterion that **cannot be exercised by the app's UI** needs a test fixture written *with* the feature, not
deferred to a verification sprint. AC12 sat unchecked through two sprints because live verification was the
only plan for it, and live verification structurally could not reach it.
