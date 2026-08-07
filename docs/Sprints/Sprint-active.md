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

## SP-093 (DRAFT): `[Cross]` History capture granularity + presentation

**Status:** 🔵 **Drafted 2026-08-07 — awaiting go-ahead to activate.** Not started.
**Epic:** EP-019 `[Cross]` — Undo/Redo, History & Copy Buffers.
**Goal:** Make the history a faithful record of how the writer actually works — coherent typing sessions
instead of arbitrary fragments, honest labels for whitespace and deletions, and a card that refreshes and
highlights correctly.
**Design:** `Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §4.a (commit triggers), §7 (ABI),
§10 Trade T2.
**Start Date:** TBD.
**Depends on:** SP-092 (the history card ships there; this sprint corrects its data and behaviour).

> ### Origin — the SP-092 live-verify (2026-08-07)
>
> The user verified the "This scene only" filter and orphaned-entry deletion (I-0102/I-0103 work) as
> **working correctly**, then reported six further findings on the same session, on
> `the-stairs-of-tintagael.scrivi` Ch 1 Sc 4 ("Mara's Room"). Diagnosis traced all six to code; three are
> defects (I-0104/I-0105/I-0106) and three are behaviour changes (T-0396/T-0397/T-0398). **None is a
> regression from SP-092** — the capture-granularity behaviour dates to SP-053, and the history card merely
> made it visible for the first time.
>
> **The reference case.** The writer typed one continuous sentence — *"Now is the winter of our discontent
> made glorious summer by this son of york"* — and history recorded it as **three** entries, split at
> `"Now is"` / `" the winter of our discontent made glo"` / `"rious summer by this son of york"`. He did
> **not** move the cursor — it is Shakespeare typed from muscle memory as filler text, in one unbroken run.
> The breaks are the **1 s autosave debounce** calling `flushThenSave()` (corrected diagnosis, 2026-08-07 —
> see T-0396). The `"…made glo"` / `"rious…"` break falls **mid-word**, which no trigger but a wall-clock
> timer can produce.

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| I-0104 | `[ScriviCore]` `externalChange` fires every open — head hash taken over replayed text, not disk bytes | High | 🔵 Open |
| I-0105 | `[Apple]` History card doesn't refresh on commit — new edits appear only after reopen | High | 🔵 Open |
| T-0396 | `[Apple]`+`[ScriviCore]` Typing-session coalescing — continuation-merge + idle timer | High | 🔵 Backlog |
| I-0106 | `[Apple]` Wrong entry bolded — caret-at-boundary + deletions match two rows | Medium | 🔵 Open |
| T-0398 | `[Cross]` Distinguish added vs. deleted text in history rows | Medium | 🔵 Backlog |
| T-0397 | `[Cross]` Whitespace-kind labels instead of "(no text)" | Low | 🔵 Backlog |

**Next free task after this sprint: T-0399.**

### Task detail

**I-0104 — the external-change trigger.** At close, `HistoryStore::persistState` hashes
`service_->headTextForScene(sceneID)` (`HistoryStore.cpp:386`) — the *replayed history head*. At open,
`validateSceneHead` (`HistoryStore.cpp:394-422`) hashes the scene's *on-disk text* and compares. These are
different artifacts and diverge without any external editing, which is why the writer sees the warning on
essentially every relaunch. Hash the bytes actually written at save time; compare disk-to-disk.

> ⚠️ **Do not suppress the barrier.** An `externalChange` barrier is *correct* for a genuine third-party
> edit — undo must not walk past text history never recorded. Only the trigger is wrong. A fix that quiets
> the message without fixing the comparison would hide real external edits.

**This supersedes I-0103**, which framed the same defect as a trailing-newline discrepancy. The newline is
one symptom; the 3 scenes that "did not normalize simply" are explained by the same hash-source mismatch.

**I-0105 — card refresh.** `HistoryCardBody` reloads on scene identity only
(`.task(id: context.sceneID)`, `HistoryCard.swift:54`). A commit mutates state inside
`HistoryCapture`/ScriviCore, which the card cannot observe, so the list goes stale until reopen. Add an
observable revision counter bumped on every successful `flush`/undo/redo/barrier and fold it into the
card's `.task(id:)`.

**T-0396 — typing-session coalescing.** The core behaviour change, and the sprint's real weight.

> ### ⚠️ Diagnosis corrected 2026-08-07 — the cause is the **autosave debounce**, not cursor movement
>
> The first draft of this sprint blamed `flush(trigger: "cursorMove", soft: true)`
> (`ManuscriptTextView.swift:803`) for the three-way split, and proposed retiring that trigger.
> **That was wrong, and the user corrected it:** he did not move the cursor while typing the reference
> sentence — it is Shakespeare he types from muscle memory as filler text, in one unbroken run.
>
> **The actual trigger is the 1-second autosave debounce** (`ManuscriptTextView.swift:753-766`):
>
> ```swift
> saveTask?.cancel()
> saveTask = Task { @MainActor in
>     try? await Task.sleep(nanoseconds: 1_000_000_000)   // fires 1 s after typing stops
>     guard !Task.isCancelled else { return }
>     session.historyCapture?.flushThenSave()             // ← seals a history entry
>     await loader.saveCurrentIfDirty(...)
> }
> ```
>
> The task is cancelled and restarted on every keystroke, so it fires only after **~1 s with no keypress** —
> then commits, to honour the §4.d invariant (disk must never hold text no history node describes).
> **So an idle timer already exists: it is the autosave's, and it is 1 second.** The earlier claim that
> "there is no idle timer at all" was based on reading `HistoryCapture` alone without following
> `flushThenSave` to its caller.
>
> This matches the evidence exactly, including the detail that rules out every other trigger: one break
> falls **mid-word** (`"…made glo"` / `"rious summer…"`). No cursor move, terminator, or structural
> operation fires mid-word — only a wall-clock timer does.

*Triggers that are **kept*** (user ruling, 2026-08-07): **cursor-move, cut/paste, scene switch, and the
sentence terminators all remain valid.** They are intentional writer actions marking a genuine boundary.
**AC2's event model is therefore not being overturned** — see the AC2 note in `Epic-active.md`.

*Target model:* continuous typing at the same insertion point **merges into the previous entry** rather
than opening a new one — *"this may result in one large historical entry especially for a writer that types
really fast, but that would be preferable to breaking sentences up that were effectively typed in one
session."*

Concretely:
- **Decouple the save-time commit from session-sealing.** This is the actual fix. Autosave must still flush
  before writing — the §4.d invariant is sound and stays — but a *save* must not *end a typing session*.
  The pending text is recorded so disk is covered, and the entry **stays open for continuation** rather
  than being sealed.
- On commit, record the scene + resulting end offset. On the next `noteEdit`, if the scene matches and the
  cursor is at that offset (an append-continuation), **extend the open entry** instead of creating a sibling.
- Add a genuine **idle timer at 30–60 s** as the session boundary, separate from the 1 s save cadence.
  Deliberately long: *"sometimes we writers ruminate for longer periods."* A pause ends a session only when
  it is long enough to mean the writer actually stopped, not merely thought mid-sentence.
- **Backspace must not commit.** It does not fire a trigger directly today, but once continuation-merge
  lands this must be explicit: *"a backspace within a line simply means I am thinking of a different word,
  not ending the typing session"* (user, 2026-08-07). Intra-word correction stays inside the open entry.

> **Where this lands — DECIDED (user, 2026-08-07): app-side.** Coalescing extends the pending latch before
> it commits; `HistoryService` stays a pure append-only tree, matching the T-0356/AC6 precedent that kept
> the service pure and ran inverse ops app-side. Do **not** split the logic across both layers.

> ### T-0396 and EP-019 **AC2** — a narrow amendment, not an overturn (revised 2026-08-07)
>
> An earlier draft of this sprint claimed T-0396 **contradicted** AC2 by retiring the
> `cursor-move-with-pending-changes` trigger it names. **With the diagnosis corrected, it does not.**
> Every trigger AC2 lists — sentence terminators, Return, cursor-move-with-pending-changes, paste/cut,
> scene switch, flush — **remains valid and is kept.**
>
> The real gap is that AC2's list is **not exhaustive of what actually commits.** The 1 s autosave debounce
> calls `flushThenSave()`, so a ~1 s typing pause seals an entry — a trigger the AC never mentions and the
> design never specified. AC2 can be verified honestly only once that behaviour is documented.
>
> **Required (T-0217, already in SP-057/SP-094's scope):** amend AC2 and design §4.a to state the
> save-time commit and the idle-session boundary explicitly. This is a **documentation gap being closed**,
> not approved criteria being overturned — a materially smaller change than the earlier draft claimed.

**T-0397 — whitespace-kind labels.** `forkPreview` (`HistoryService.cpp:148-157`) rewrites `\n`, `\r`, `\t`
to spaces; `HistoryCard.label` (`HistoryCard.swift:264-271`) then trims and falls through to `"(no text)"`.
A pure-newline event is therefore indistinguishable from an empty one — the user saw three such rows.
Name the whitespace instead: `⏎ new paragraph`, `⇥ tab`, `␣␣␣ 3 spaces`. The kind must survive from the
diff to the label rather than being flattened at preview time.

**T-0398 — added vs. deleted.** Every row renders `circle.fill` (`HistoryCard.swift:258-262`), so a
deletion is visually identical to an insertion — the user could not tell that the `"is the"` entry was
text he had **removed**. The data exists (`diff.inserted` / `diff.removed`); the tree payload just doesn't
carry it: `t.changeLength = n.diff.inserted.size()` (`HistoryService.cpp:700`) drops the removed length
entirely. Add `removedLength` to the payload, then differentiate glyph/colour and prefix the label.

> **Shared payload change with I-0106.** Both need `removedLength` in `TreeNode`. Do the ABI/serialization
> change **once**, then let I-0106 (caret ranges) and T-0398 (presentation) consume it — sequence I-0106
> and T-0398 together to avoid touching the envelope twice.

**I-0106 — caret highlighting.** Two defects in one range model. (a) A deletion has
`changeLength == 0`, hitting the degenerate `caret == changeOffsetUtf8` branch in
`HistoryTreeNode.contains(caret:)` (`ScriviEngine.swift:2239-2241`) — which matches at the same offset an
adjacent insertion's half-open range `[start, start+len)` also contains, so **two rows bold** (user-verified
with "is the"). (b) The half-open range has no boundary tie-break, so a caret at the start of an entry
bolds its neighbour. Give deletions a real span via `removedLength`; define the most-recent node as the
winner at a shared boundary.

### Exit criteria

- [ ] A scene edited only inside Scrivi produces **no** `externalChange` barrier across repeated
      quit→reopen cycles; a scene genuinely modified by an external editor **still does** (both directions
      covered by ctest).
- [ ] Committing an event refreshes the history card **without** a scene switch or relaunch.
- [ ] The reference sentence — *"Now is the winter of our discontent made glorious summer by this son of
      york"* — typed continuously, records as **one** history entry, not three.
- [ ] **An autosave during typing does not seal the entry** — the scene is written to disk (§4.d holds:
      disk never contains text no history node describes) while the typing session stays open.
- [ ] A deliberate pause mid-sentence **beyond the 30–60 s idle threshold** starts a new entry; a pause of
      1–2 s (which currently splits the entry) does **not**.
- [ ] **Backspace mid-word does not start a new entry** — intra-word correction stays in the open session.
- [ ] **The kept triggers still work**: cursor-move-with-pending-changes, cut/paste, scene switch and
      sentence terminators each still commit as AC2 specifies (regression check — this sprint must not
      quietly weaken them while fixing the save-driven commit).
- [ ] **EP-019 AC2 and design §4.a document the save-time commit and idle boundary** (via T-0217) before
      EP-019 is put forward for close.
- [ ] Caret at the start of an entry bolds **that** entry; a deletion bolds **exactly one** row.
- [ ] A newline/tab/space-run event reads as named whitespace, never `"(no text)"`.
- [ ] Insertions and deletions are visually distinguishable at a glance, and a deletion's label says so.
- [ ] `ctest` + interop suites green; app builds and launches clean.

### Non-negotiables

- **Coalescing is app-side** (user-approved 2026-08-07) — `HistoryService` stays pure. Not to be
  re-litigated mid-implementation, and not split across both layers.
- **Idle threshold 30–60 s** (user ruling) — not silently shortened because a test is slow to run.
- **The §4.d disk invariant is preserved.** Decoupling the save-time commit from session-sealing must not
  let the scene file contain text no history node describes. Record, then keep the entry open — do not
  simply stop flushing before save.
- **Cursor-move, cut/paste, scene switch and sentence terminators stay as commit triggers** (user ruling,
  2026-08-07). This sprint changes *save-driven* commits only.
- **The `externalChange` barrier stays** for genuine external edits — I-0104 fixes the *trigger*, not the
  feature.
- **`removedLength` ships once**, consumed by both I-0106 and T-0398.
- **pbxproj updated in the same step** as every new `.swift` file under `Scrivi/` (CLAUDE.md). **ScriviCore
  `.cpp`/`.hpp` go in CMake, NOT pbxproj.**
- **Build with `-DSCRIVI_BUILD_TESTS=ON`** and confirm the ctest count moves when tests are added (SP-091
  precedent: a stale cached test binary silently replayed).
- **Swift is UI only** (Architecture v0.3).
- Claude may mark tasks **"Implemented — Not Verified"**; only the user marks them Verified.

### Sequencing note

I-0104 and I-0105 are independent and can land first — they are the two that make the card usable enough
to verify everything else against. T-0396 is the largest change and should not be bundled with them.
I-0106 and T-0398 share the `removedLength` payload change and should run together. T-0397 is
self-contained and lowest priority.

### Relationship to EP-019's close — **APPROVED: SP-057 runs after SP-093** (user, 2026-08-07)

SP-092's close note had **EP-019 closing after that sprint** (T-0215 met by T-0366, T-0216 closed OBE,
SP-057 reduced to pure verification). This sprint's findings are all EP-019 behaviour, so **EP-019 does
not close until SP-093 completes** — closing it with the capture granularity in this state would archive
the Epic against behaviour the user has explicitly reported as wrong.

The ordering still holds, for a narrower reason than the first draft claimed. SP-057's job is to live-verify
**AC2**, and while T-0396 does **not** overturn AC2's trigger list (all of it is kept), it does change
*when a typing session ends* — so verifying AC2 before SP-093 would sign off on the fragmentation the user
reported. **T-0217** (doc updates, in SP-057/SP-094's scope) carries the AC2 + design §4.a amendment
documenting the save-time commit and the idle boundary.

---

_Prior: **SP-091** (`[Cross]` EP-030 — writing-tool cards) ✅ **closed 2026-08-05 (Human-approved)** — three real
cards on new additive `scrivi.scene.v1` fields (`tags`/`outline`/`todo`) + four C ABI endpoints; T-0392, T-0393,
T-0363, T-0364 all Verified, plus **I-0101** (unremovable unknown card) found, fixed, and Verified in-sprint.
ctest **381/381**, interop **45/45**. Archived `Closed/Sprint-SP-091.md`._

_**SP-090** (card framework) ✅ closed 2026-08-05 — `Closed/Sprint-SP-090.md`._

_**Then:** **SP-093** (EP-019 history capture granularity + presentation — drafted above, from the SP-092
live-verify) → **SP-094** (combined verification + double Epic close: EP-019 AC2/AC7/AC8 + T-0217 docs,
**and** EP-030 AC1–AC7 + T-0369) → **EP-031** SP-095–SP-100 (T-0370–T-0391), whose ScriviCore sprints have
no EP-030 dependency._

> ### Numbering + the T-0369 question — resolved (user, 2026-08-07)
>
> SP-093 was previously earmarked for **EP-030 verification + Epic close (T-0369)**; this draft claims the
> number for the EP-019 history work.
>
> **T-0369 does not need its own sprint.** The old SP-093 (`Sprint-backlog.md:140-152`) and SP-057
> (`Epic-active.md:235`) are **both pure verification sprints with no build work** — one task each
> (T-0369; AC2/AC7/AC8 + T-0217), both gated on the same live-verify session against the same app build,
> both ending in a user-only Epic close. Running them as two consecutive one-task sprints is ceremony, not
> tracking.
>
> **Merged into a single SP-094: "EP-019 + EP-030 verification & Epic close."** Both Epics' ACs are
> verified in one pass, then closed independently — each still requires its own direct user approval
> (CLAUDE.md), and a failure in one Epic's ACs does **not** block the other's close. The old SP-093 entry
> in `Sprint-backlog.md` should be struck when SP-093 activates.
>
> **If either Epic's verification turns up implementation work**, that work gets its own sprint and the
> affected Epic's close moves behind it — exactly what happened here, when SP-092's verify produced SP-093.

_Next available Sprint **SP-095** (after this draft and SP-094 activate), next Task **T-0399**._
