# Active Sprint

## SP-092: `[Cross]` EP-030 — history card + Properties tab

**Status:** 🟡 **Active — activated 2026-08-06.**
**Epic:** EP-030 `[Apple]` — Scene Inspector Card Framework (3rd of 4 sprints).
**Goal:** The `history` card — which **absorbs EP-019's T-0215 and unblocks that Epic's close** — plus the
Properties tab and per-card failure isolation.
**Design:** `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §4.8/§7.1, AC12–AC15 ·
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §7 (ABI), §10 Trade T2 option B (panel).
**Start Date:** 2026-08-06
**Target Close Date:** TBD.
**Depends on:** SP-090 ✅ (framework), SP-091 ✅ (card precedent).

> ### ⚠️ Scope correction found at planning (2026-08-05) — second sprint running
>
> **`scrivi_history_get_tree` does not exist.** `scrivi.h:334` says so explicitly: *"The remaining deferred
> function is scrivi_history_get_tree (history panel, SP-057)."* It was deferred when EP-019 deferred its panel,
> and the panel then moved here. The card cannot render a tree without it, so **SP-092 is `[Cross]`, not
> `[Apple]`** — exactly the shape SP-091 turned out to have.
>
> **The good news: the engine work is already done.** `HistoryService` exposes `nodes()`, `rootID()`, and
> `currentNodeID()` (`HistoryService.hpp:280-294`), and `EventNode` already carries everything a panel needs —
> `parentID`, `primaryChildID`, `childIDs`, `kind`, `sceneID`, `timestamp`, `sessionID`, `bufferID`,
> `barrierKind`/`barrierNote`. `get_tree` is therefore a **serialization + windowing layer over existing
> state**, not new history logic. The sibling endpoints (`select_branch`, `list_stale_branches`,
> `purge_branch`) already ship (`scrivi_c_api.cpp:1904/1928/1956`).
>
> **Swift has no tree wrapper either** — `HistoryCapture` has `selectBranch` and `listStaleBranches`
> (`HistoryCapture.swift:276/291`) but nothing for the tree.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0394 | `[ScriviCore]` `scrivi_history_get_tree(projectRootPath, paramsJSON)` — windowed `{aroundNodeID?, maxNodes?}` + ctest | High | 🟠 **Implemented — Not Verified** |
| T-0395 | `[Apple]` `ScriviEngine` / `HistoryCapture` tree wrapper + interop test | High | 🟠 **Implemented — Not Verified** |
| T-0366 | `[Apple]` `history` card — windowed tree, branch selection, stale badges, purge (**supersedes T-0215**) | High | 🟠 **Implemented — Not Verified** |
| T-0367 | `[Apple]` Properties tab — **field-driven** view (author, timestamps, metrics) | Medium | 🟠 **Implemented — Not Verified** |
| T-0368 | `[Apple]` Card failure isolation + inline warning presentation | Medium | 🟠 **Implemented — Not Verified** |

**Next free task after this sprint: T-0396.**

### Task detail

**T-0394 — `scrivi_history_get_tree`.** Signature is already specified in the approved EP-019 design (§7):

```c
const char* scrivi_history_get_tree(const char* projectRootPath, const char* paramsJSON);
    /* windowed: {aroundNodeID?, maxNodes?} */
```

Returns the node set plus the pointers a panel needs — at minimum `rootID`, `currentNodeID`, and per node:
`eventID`, `parentID`, `childIDs`, `primaryChildID`, `kind`, `sceneID`, `timestamp`, `sessionID`, a short
`preview`, and barrier fields where present. **Windowing is the point:** a 100k-event history must not serialize
whole. Default `maxNodes` conservatively and window around `aroundNodeID ?? currentNodeID`.

> **Empty-array omission (I-0094).** The C API omits empty arrays, so a leaf node ships no `childIDs` key.
> T-0395's Swift types **must** decode absent arrays as empty — this is the exact bug that sank EP-029's
> cross-boundary paste, and it recurred as a latent risk in SP-091.

**T-0395 — Swift wrapper.** `getHistoryTree(projectRootPath:aroundNodeID:maxNodes:)` on `ScriviEngine`, surfaced
through `HistoryCapture` beside the existing `selectBranch` / `listStaleBranches`. visionOS stubs. Interop test.

**T-0366 — the `history` card.** Delivers what T-0215 specified, as a dismissible card in the **Writing** stack:
the windowed tree (primary spine + fork glyphs), click-to-select a node, stale-branch badges, and
user-confirmed purge. Reuses the shipped `select_branch` / `list_stale_branches` / `purge_branch` endpoints.

> ⚠️ **Purge is destructive and irreversible** — it erases a branch subtree and writes a `ctl:purge` record so it
> cannot resurrect on reload. The confirmation must name what is being discarded, not ask a generic "are you
> sure?". Precedent: Project Settings' existing Stale Branches section (SP-055).

**T-0367 — Properties tab.** Author, created/modified timestamps, word/character counts, story-time position.
**Not a card stack.** Built **field-driven** — each field declares its own editability — so a future editable
property is a field-level addition rather than a rewrite (§4.8.1). Fields read-only *because derived* (metrics,
timestamps) must be marked distinctly from fields that merely have no editor yet.

> Properties is the **one legitimate exception to §2's manual-surfacing principle** (§4.8): these are facts
> *about* the scene, not world content the writer chose to associate. Worth keeping stated so it is not later
> read as a violation.

**T-0368 — card failure isolation.** One card's failure must never block the stack; the failed card shows a
warning in place of its content. SP-091's cards already do this individually (`CardErrorView` in
`WritingToolCards.swift`); this task makes it a **framework guarantee** rather than a per-card courtesy.

### Exit criteria

- [ ] `scrivi_history_get_tree` returns a windowed tree; `maxNodes` is honoured and a large history does not
      serialize whole (ctest).
- [ ] Swift decodes the tree with **absent arrays tolerated as empty** (I-0094 class).
- [ ] The `history` card renders the windowed tree with the primary spine and fork glyphs; clicking a node
      selects it.
- [ ] Stale branches are badged; purge is available **with a confirmation naming what is discarded**, and a
      purged branch does not return after relaunch.
- [ ] The card is **dismissible** and its position/collapsed state persist (§1) — parity with SP-091's cards.
- [ ] The **Properties** tab renders author, timestamps, and metrics, and is **not** a card stack (AC14).
- [ ] Properties is **field-driven**, each field declaring editability; derived-read-only is distinguished from
      no-editor-yet (AC15).
- [ ] One card failing to load does not prevent others rendering; the failed card shows a warning in place of
      its content (AC12).
- [ ] `ctest` + interop suites green; app builds and launches clean.

### Non-negotiables

- **pbxproj updated in the same step** as every new `.swift` file under `Scrivi/` (CLAUDE.md). **ScriviCore
  `.cpp`/`.hpp` go in CMake, NOT pbxproj.**
- **Build with `-DSCRIVI_BUILD_TESTS=ON`.** SP-091 found `build/` configured without it, silently replaying a
  cached test binary — new cases appeared to pass without being compiled. **Confirm the ctest count moves when
  tests are added.**
- **Any card retirement or `typeID` rename must confirm the unknown-type path is _actionable_, not merely
  visible** (I-0101). SP-092 does not retire a card, but EP-031 will.
- **Swift is UI only** (Architecture v0.3).
- Claude may mark tasks **"Implemented — Not Verified"**; only the user marks them Verified.

### What this unblocks

**EP-019 closes after this sprint.** Its history-panel requirement (T-0215) is met by T-0366, and T-0216 was
closed OBE, leaving **SP-057 as pure verification** — AC2/AC7 were audited as already implemented and both
suites are green, so what remains is live user verification of AC2/AC7/AC8 plus T-0217's doc updates.

---

_Prior: **SP-091** (`[Cross]` EP-030 — writing-tool cards) ✅ **closed 2026-08-05 (Human-approved)** — three real
cards on new additive `scrivi.scene.v1` fields (`tags`/`outline`/`todo`) + four C ABI endpoints; T-0392, T-0393,
T-0363, T-0364 all Verified, plus **I-0101** (unremovable unknown card) found, fixed, and Verified in-sprint.
ctest **381/381**, interop **45/45**. Archived `Closed/Sprint-SP-091.md`._

_**SP-090** (card framework) ✅ closed 2026-08-05 — `Closed/Sprint-SP-090.md`._

_**Then:** **EP-019 SP-057** (pure verification + Epic close) → **SP-093** (EP-030 verification + Epic close) →
**EP-031** SP-094–SP-099 (T-0370–T-0391), whose ScriviCore sprints have no EP-030 dependency._

_Next available Sprint **SP-093**, next Task **T-0396**._
