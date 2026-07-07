# Scrivi Undo/Redo History and Copy Buffers Design v0.1

**Status:** ✅ Approved baseline — user design sign-off 2026-07-06 (all six trades ruled; T2 approved with interaction refinements, incorporated in §10 T2)
**Project:** Scrivi
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Task:** T-0198 (this document)
**Supersedes:** I-0019 (Undo and Redo have no effect in the manuscript editor — ⚪ Closed 2026-07-06 as OBE/superseded by EP-019, user-approved; the requirement is carried by EP-019 AC1)
**Platforms:** macOS first; iOS/iPadOS follow when the iOS editor exists (the engine is platform-neutral by construction)
**Related docs:**
- `Scrivi_Architecture_v0_3.md` — pass-by-value principle, JSON-over-string boundary
- `Scrivi_Project_Package_Structure_v0_1.md` — `.scrivi` package layout (this design adds `history/`)
- `Scrivi_External_Change_Repair_Matrix_v0_2.md` — gains one row (history corrupt/missing)
- `docs/Issues/Closed/Issue-closed-0019.md` — I-0019, the defect this Epic supersedes (closed 2026-07-06)

---

## 1. Why

Undo and Redo do nothing in the manuscript editor (I-0019, High severity, identified 2026-06-09; closed 2026-07-06 as superseded by this Epic — the defect itself remains until EP-019 AC1 ships). The root cause is now understood: the editor's authoritative text state is `ViewportSceneLoader.segments[].text`, and the single `NSTextStorage` is rebuilt and re-extracted around `NSTextView`, so `NSUndoManager` never accumulates a usable action stack. Patching native undo would fight the editor's architecture.

Instead, Scrivi implements undo/redo from scratch, and turns a defect fix into a differentiating feature for professional writers:

- **Sentence-granular history** — each committed sentence (or cursor reposition after typing) is one undo step, which matches how writers think about revision, not per-keystroke noise.
- **Persistent, per-project history** — history lives on disk inside the project; yesterday's edits remain undoable after relaunch. A session boundary warning prevents accidental time travel.
- **Tree-structured history** — undoing and then typing forks a branch instead of destroying the redo chain. Abandoned drafts remain recoverable; the writer can return to a fork and re-adopt the original text.
- **Multiple copy buffers** — vim/emacs-register-style named buffers for repetitive replacement work (concept of operations in §9.1).

## 2. Scope (proposed)

**In scope (v1):**
1. History events for manuscript **text** edits (typing, deletion, replacement, paste, cut) in the macOS editor.
2. Tree history with branching, primary-line selection, capacity eviction, stale-branch detection and purge.
3. On-disk persistence in the `.scrivi` package; cross-session undo with session-boundary warning.
4. Configurable history capacity (Trade T1).
5. ⌘Z / ⇧⌘Z and Edit-menu routing to the new system; native `NSUndoManager` content undo disabled.
6. Multiple copy buffers: per-project, persistent, load/paste UI, history integration (Trades T3, T4).

**Out of scope for v1 (see §13):** structural undo (scene/chapter create/delete/merge/reorder are history *barriers*, §4.5), find-and-replace, iOS capture layer, collaborative/multi-author history semantics.

## 3. Architecture Decision

Fixed by the approved architecture (`Scrivi_Architecture_v0_3.md`): **all history logic and persistence live in ScriviCore (C++23)**; Swift is capture-and-apply UI only.

- New backend module `ScriviCore/src/history/` (`HistoryService`) plus a buffers store, both behind new `scrivi_history_*` / `scrivi_buffers_*` functions in the pure C ABI (`ScriviCore/include/scrivi/scrivi.h`), returning the standard `{"ok":true,"result":{…}}` JSON envelopes freed via `scrivi_free` (§7).
- Swift gains one new component, `HistoryCapture` (owned by `ProjectSession`), which detects commit triggers in the editor and applies undo/redo results back to the text view (§8). No history state machine, no diffing, no tree logic in Swift.
- The history store is **not** a second source of truth for the manuscript: scene `.md` files remain canonical. History records how the text got there and how to walk it back.

### 3.a Key design decision — snapshot-diff in C++, not keystroke tracking

Swift does **not** accumulate per-keystroke deltas. At each commit trigger, Swift sends the affected scene's **full current text** across the boundary (the coordinator already extracts exactly this string on every `textDidChange`). `HistoryService` keeps the text-at-current-node per touched scene and computes a minimal diff (common-prefix/common-suffix trim on Unicode scalar boundaries), storing only the delta.

Why: it is robust against every mutation `NSTextView` performs without a keystroke — autocorrect, dictation, spell-fix, drag-and-drop, IME commit — because an event is defined by *state difference*, not intercepted input. It also eliminates cross-language offset math entirely: offsets exist only inside C++ (UTF-8) and are never round-tripped, because undo/redo return full scene text (§7). Cost: one scene-sized string per sentence across the ABI — trivial at sentence frequency (§11).

### 3.b Key design decision — undo/redo return full scene text

`scrivi_history_undo`/`_redo` return `{sceneID, newText, cursorAfter}` per affected scene. This matches the existing whole-scene `saveScene` granularity, and Swift applies it with a single ranged `replaceCharacters` inside the scene's boundary (dividers and protected chapter headings untouched), then persists via the normal `engine.saveScene` path. C++ never writes scene files from the history path — scene files keep exactly one write path.

## 4. Event Model

### 4.a Definition of a history event

A history event is committed when **text has been typed or deleted since the last event** AND one of the triggers below fires. Cursor movement (or any number of cursor movements/newlines) without an intervening text change produces **no** event.

Commit triggers (detected by `HistoryCapture` via the existing editor hooks in `ManuscriptTextView.swift`):

| # | Trigger | Detection point |
|---|---------|-----------------|
| 1 | Sentence terminator typed: `.` `!` `?` (approved 2026-07-06) or Return/Enter | `shouldChangeText` (character identified) → commit after it lands on the next `textDidChange` |
| 2 | Cursor moved with pending text changes | `textViewDidChangeSelection`, disambiguated from edit-driven selection changes by an "edit in flight" flag set by any `textDidChange` and cleared per run-loop turn |
| 3 | Paste (system pasteboard or copy buffer) | `paste(_:)` override / buffer-paste action — commits pending first, then commits the insertion as its own `paste` event |
| 4 | Cut (incl. cut-into-buffer) | as above; `kind:"cut"`, tagged with `bufferID` when applicable |
| 5 | Cursor crosses a scene boundary | existing `lastCursorSegmentIndex` tracking — a special case of #2 committed against the *previous* scene's retained text |
| 6 | Auto-save flush | committed **before** `saveCurrentIfDirty` runs in the existing 1 s debounce (invariant, §4.d) |
| 7 | Structural op / app resign / project close | commit pending, then (structural) record a barrier (§4.5) |

No commit occurs while IME marked text is active (`hasMarkedText()`); rebuilds (`isRebuilding`) never set the pending flag.

### 4.b Event record (`scrivi.history.v1` node)

```json
{
  "eventID": "evt_<uuid>",
  "parentID": "evt_<uuid> | null",
  "kind": "typing | delete | replace | paste | cut | barrier",
  "sceneID": "scene_…",
  "diff": { "offsetUtf8": 1234, "removed": "teh ", "inserted": "the " },
  "cursorBefore": 1230,
  "cursorAfter": 1238,
  "timestamp": "2026-07-06T14:03:22Z",
  "sessionID": "ses_<uuid>",
  "groupID": null,
  "authorship": { "identityID": "…", "personaID": "…" },
  "meta": { "trigger": "sentence|cursorMove|paste|cut|sceneSwitch|flush", "bufferID": null }
}
```

- Offsets and cursor positions are **scene-local UTF-8 byte offsets, C++-internal only** (never interpreted by Swift; Swift converts the returned scene-local cursor to storage coordinates through its existing boundary maps).
- `groupID` is **reserved from day one** so a future multi-scene operation (find-and-replace, cross-divider deletion) can be undone atomically as one step. v1 never produces groups.

### 4.c History shape: manuscript-global tree, scene-scoped events

One history tree per project. Each event targets exactly one scene. Rationale: the writer's mental model is a single manuscript timeline ("undo my last N sentences," walking back across scene switches), and the persistence unit (whole-scene `.md` + `saveScene`) is scene-scoped, so events map 1:1 onto the existing save path. Per-scene trees would multiply session/branch/capacity bookkeeping by scene count and make a single ⌘Z timeline impossible.

### 4.d Invariant: history commits before disk writes

The scene file on disk must always equal the text at some recorded history node (normally the head). Therefore every path that writes a scene file (debounced auto-save, save-on-scene-exit, save-on-resign, save-after-undo) first flushes any pending history commit. Enforced in one place: `HistoryCapture.flushThenSave`.

### 4.5 (4.e) Structural operations are barriers in v1

Scene/chapter create, delete, merge, split, and reorder record a **barrier node** (`kind:"barrier"`, `barrierKind: sceneSplit|sceneDelete|sceneMerge|chapterSplit|chapterMerge|reorder|externalChange`). Undo stops at a barrier with a clear notice ("Can't undo past a scene merge"). Rationale: inverting structural ops means resurrecting scene files, IDs, metadata, and timeline band assignments — a large separable feature; the app already tells users chapter splits cannot be undone, so barriers are honest and strictly better than today. Structural undo is the documented future extension riding on `groupID` + inverse operations.

## 5. History Tree Model

- **Node/edges:** every event node carries `parentID`; children are derived at load. The root is the oldest retained state.
- **Primary line:** each node with children has a `primaryChildID`. Redo follows the primary chain. The primary line is "the" history; other subtrees are branches.
- **Current pointer:** `currentNodeID` — the node whose cumulative state matches the editor. Undo moves to the parent (returning the inverse of the departed node's diff as full scene text); redo moves to the primary child.
- **Branching (per user spec):** recording a new event while `currentNodeID` already has children creates a **sibling** and sets it as the fork node's `primaryChildID` — the new work becomes the primary history, the old chain remains as a branch. Selecting an old branch (`scrivi_history_select_branch`) re-assigns `primaryChildID` — the restored branch becomes primary again. **Branch relegation (approved refinement, 2026-07-06):** when the writer undoes past an existing fork point and then types, the entire undone chain — including that embedded fork point — is relegated as one non-primary branch under the new fork; nested forks inside a branch remain navigable if the branch is later re-selected.
- **Sessions:** a `sessionID` is minted per session (definition: Trade T5). An undo that would cross into a node from an earlier session returns `crossedSessionBoundary: true` plus the boundary timestamp; the UI warns once per crossing ("You are about to undo changes from a previous session — yesterday 21:42").
- **Capacity eviction:** when node count exceeds capacity (Trade T1; default 20,000), the root is evicted repeatedly: non-primary subtrees hanging off the root are auto-purged (the user-specified rule — a branch dies when its branch point falls off the history limit), then the primary child becomes the new root and its state is folded into the per-scene floor snapshots. **Never evict a node on the root→current path**; if the current pointer sits inside a subtree that would be purged, other branches are purged first and eviction defers.
- **Stale-branch detection:** a branch is any non-primary subtree. Stale = tip older than a threshold (default 7 days; setting) or branch point more than K nodes behind head. The app surfaces the count and offers purge; purge is user-confirmed except for the automatic eviction rule above.

## 6. Persistence Design (`scrivi.history.v1`)

### 6.a Files (inside the package — Trade T6)

```
MyProject.scrivi/
  history/
    state.json          ← atomic checkpoint: schemaVersion, settings, rootID, currentNodeID,
                          primaryChild overrides, sessionID, lastEventSeq, per-scene head-text hashes
    log-000001.jsonl    ← append-only: event nodes AND control records, one JSON object per line
    buffers.json        ← copy buffers (scrivi.buffers.v1, §9)
```

- **Control records** (`{"op":"undo"}`, `"redo"`, `"setPrimary"`, `"purge"`, `"session"`, `"evict"`) are appended alongside event records; the log alone fully reconstructs the tree and pointers.
- `state.json` is written via `scrivi::util::atomicWriteTextFile` on project close, on pointer-heavy operations (branch select, purge), and every ~200 records. It is a **load accelerator, not a source of truth** — on mismatch, the log tail is replayed from `lastEventSeq`.

### 6.b Crash safety and recovery

- Log appends are write+flush; a torn final line is detected at load (parse failure) and truncated — worst case loses the last sentence, never the tree.
- **Head-hash validation:** `state.json` stores each touched scene's head-text hash. `scrivi_history_open` validates hashes against the scene files. A mismatch (external edit, or crash between undo-apply and save) appends an `externalChange` **barrier** and re-seeds that scene's cached text from disk — history is never silently wrong, and repair **never modifies the manuscript**.
- **Compaction:** on segment rotation (~10k records), eviction, or purge, the live tree is serialized to `log-(N+1).jsonl` via temp+rename and old segments are deleted. (Rewriting a single monolithic file per event and per-node files were both rejected: tens-of-MB rewrites per sentence, and 100k tiny files respectively.)

### 6.c Interaction with Git snapshots and external-change scan

- `history/` is added to the `.gitignore` that `SnapshotService::writeGitignore` emits — undo state is app-local churn, not canonical content. A small migration appends the entry for projects with an existing `.gitignore`.
- The external-change scanner's ignore set gains `history/`; the External Change Repair Matrix gains one row: **history corrupt/missing → reset history, warn the user, never touch the manuscript.**

## 7. Boundary API (C ABI)

New functions in `scrivi.h`, implemented in `scrivi_c_api.cpp` (standard envelope/`scrivi_free`/per-project registry conventions):

```c
const char* scrivi_history_open(const char* projectRootPath, const char* paramsJSON);
    /* loads + validates history, mints sessionID; returns {canUndo, canRedo, sessionID,
       settings:{capacityEvents, staleBranchDays}, staleBranchCount} */
const char* scrivi_history_record_event(const char* projectRootPath, const char* sceneID,
                                        const char* newSceneTextUtf8, const char* paramsJSON);
    /* params: {trigger, kind, cursorBefore, cursorAfter, bufferID?, groupID?, authorshipRef}
       returns {eventID, createdBranch, evictedCount} */
const char* scrivi_history_record_barrier(const char* projectRootPath, const char* paramsJSON);
const char* scrivi_history_undo(const char* projectRootPath);
const char* scrivi_history_redo(const char* projectRootPath);
    /* both return {changes:[{sceneID, newText, cursorAfter}], nodeID, canUndo, canRedo,
       crossedSessionBoundary, boundaryTimestamp?, stoppedAtBarrier:{kind,note}?,
       forkAhead:{nodeID, children:[{eventID, preview, timestamp, isPrimary}]}?} */
const char* scrivi_history_get_tree(const char* projectRootPath, const char* paramsJSON);
    /* windowed: {aroundNodeID?, maxNodes?} — for the history panel (T2) */
const char* scrivi_history_select_branch(const char* projectRootPath, const char* forkNodeID,
                                         const char* childEventID);
const char* scrivi_history_list_stale_branches(const char* projectRootPath);
const char* scrivi_history_purge_branch(const char* projectRootPath, const char* branchRootEventID);
const char* scrivi_history_get_settings(const char* projectRootPath);
const char* scrivi_history_set_settings(const char* projectRootPath, const char* settingsJSON);
const char* scrivi_history_close(const char* projectRootPath);   /* checkpoint + flush */

const char* scrivi_buffers_list(const char* projectRootPath);
const char* scrivi_buffers_set(const char* projectRootPath, const char* bufferID, const char* textUtf8);
const char* scrivi_buffers_clear(const char* projectRootPath, const char* bufferID);
```

**Write policy:** the engine is in-memory with per-event log append (I/O ≈ once per sentence — negligible) and checkpoints on pointer operations and close.

## 8. Swift Layer

- **`HistoryCapture`** (new, `@MainActor`, owned by `ProjectSession`): pending-flag state machine, trigger detection, engine calls, session-warning popup and fork-popover state. The `Coordinator` in `ManuscriptTextView.swift` only *notifies* it (edit occurred in scene X with text T; cursor moved; paste/cut occurred). `ViewportSceneLoader` stays history-agnostic apart from applying returned texts via the existing `updateText`.
- **Apply path (undo/redo):** ranged `replaceCharacters(in: sceneBoundaries[segIdx], with: newText)` on the storage under the `isRebuilding` guard (bypassing `shouldChangeText`; dividers and `scriviHeading` runs untouched), then `recomputeBoundaries`, cursor placement from `cursorAfter` (scene-local → storage offset via existing maps), `loader.updateText`, and an **immediate** (non-debounced) `engine.saveScene`.
- **⌘Z / Edit-menu routing (T-0199 spike, round 1 result — 2026-07-06):** the originally proposed
  `UndoManager` proxy (override `undoManager` on the text view) is **REJECTED on empirical
  evidence**. Live run showed (a) the Edit menu never consulted the proxy — no `canUndo` /
  `undoMenuItemTitle` queries; items stayed disabled with default titles — and (b) even with
  `allowsUndo = false`, NSTextView's internal typing coalescing
  (`NSTextViewSharedData coalesceInTextView:` → `_NSUndoStack _setGroupIdentifier:`) reached into
  the proxy's private stack state and raised `NSInternalInconsistencyException` ("must begin a
  group before registering undo") on every keystroke. AppKit requires a real, internally-consistent
  `NSUndoManager`; a subclass that swallows grouping calls corrupts it (risk §12.6 confirmed).
  **Adopted mechanism (round 2 of the spike — ✅ CONFIRMED live, 2026-07-06):** no `undoManager`
  override anywhere; `allowsUndo = false`; implement `undo(_:)`/`redo(_:)` **action methods** on
  `ManuscriptNSTextView` — the Edit ▸ Undo/Redo items send `undo:`/`redo:` down the responder
  chain and the first responder's implementation intercepts them — plus
  `validateUserInterfaceItem` for enable state (and optional titles). Observed: validation
  callbacks fired (`validate undo:/redo: queried`); **menu clicks AND ⌘Z/⇧⌘Z both delivered to
  the action methods** (sender `SwiftUIMenuItem` — the key equivalents route through the SwiftUI
  menu item, so a `keyDown` backstop is unnecessary on the healthy path but harmless as defense);
  typing produced **no exceptions and no undo-manager interaction**. T-0205 implements this
  mechanism for real, delegating to `HistoryCapture`; menu-title customization ("Undo Typing"-style)
  can be exercised there via the same validation callback.

## 9. Multiple Copy Buffers

### 9.a Concept of operations (user-supplied)

A manuscript uses an invented language with singular "Kazd'ul" and plural "Kazda'la". The writer loads corrected spellings into buffers A and B, then scans the manuscript; at each occurrence she pastes from the matching buffer. Buffers make repetitive multi-string replacement fast without round-tripping through the single system pasteboard.

### 9.b Model

- Named/numbered slots (v1: 9 slots, IDs `1`–`9`; optional user labels later).
- **Per-project and persistent**, stored by ScriviCore in `history/buffers.json` (`scrivi.buffers.v1`) — persistence lives in C++ per the architecture; Swift holds an in-memory mirror for display.
- The system pasteboard is **never clobbered** — buffers are a parallel mechanism; ordinary ⌘C/⌘V behavior is unchanged.
- **History integration:** paste-from-buffer is an ordinary insertion → recorded as a `paste` event (undo works with zero special cases). Copy-into-buffer vs history: Trade T3. Cut-into-buffer is a text mutation → always an event (`kind:"cut"`, `bufferID` tagged).

### 9.c Presentation

Per Trade T4: keyboard-first HUD as the fast path plus a small toggleable palette for loading/inspecting slots, with Edit-menu items for discoverability.

## 10. Trade Studies

House format, abbreviated per trade: options, criteria, scoring (1–5, higher is better), decision. **All six decisions approved by the user 2026-07-06** (T1–T5 ruled individually; T6 via the overall design approval). T2 was approved with interaction refinements, recorded in that section.

### T1 — Where does the history-capacity setting live?

| Option | Description |
|--------|-------------|
| A | App settings (Swift `UserDefaults`) |
| B | **Project settings (`project.json`, C++ `ProjectJsonData`)** |
| C | Hybrid: app-level default for new projects + per-project override |

| Criterion | A | B | C |
|-----------|--:|--:|--:|
| Architectural fit (C++ owns eviction & `project.json`) | 1 | 5 | 4 |
| Portability (capacity governs per-project on-disk data; travels with project) | 2 | 5 | 5 |
| UI simplicity / one obvious knob | 4 | 4 | 2 |
| Multi-machine consistency (no two machines fighting over eviction) | 1 | 5 | 4 |

**Decision (✅ Approved 2026-07-06): B.** Add a `historySettings` block to `ProjectJsonData` (`capacityEvents`, `staleBranchDays`) and a row in `ProjectSettingsSheet`. A fails architecturally — the engine would need the value shuttled on every call, and two machines with different app settings would silently fight over eviction of the same on-disk history. C is a cheap later addition if a global default knob is wanted.

### T2 — Branch-point identification & branch selection

| Option | Description |
|--------|-------------|
| A | **Inline fork popover:** when undo lands on (or redo reaches) a fork, a transient popover at the caret lists the children with preview text + timestamp; arrows/digits + Return selects (and sets primary), Esc keeps the primary |
| B | **History panel:** persistent management surface rendering the windowed tree (primary spine + fork glyphs), click-to-select, stale-branch badges + purge |
| C | Edit ▸ "Undo History…" modal browser only |

| Criterion | A | B | C |
|-----------|--:|--:|--:|
| Flow disruption (writer stays in the text) | 5 | 3 | 1 |
| Discoverability | 3 | 4 | 4 |
| Implementation cost | 4 | 2 | 2 |
| Covers stale-branch/purge management | 1 | 5 | 3 |

**Decision (✅ Approved 2026-07-06, with refinements): A as the core interaction now; B added late in the Epic (SP-057) as the management surface** (purge, browsing yesterday's work). C rejected — heavyweight for the common case. The `forkAhead` field in the undo/redo envelope (§7) exists to drive A.

**Approved interaction refinements (user, 2026-07-06):**
1. **Undo landing on a fork shows the popover.** When an undo step arrives at a node with multiple children — typically some time after the fork was created — the writer is likely searching for that fork, so the popover appears, offering the branches.
2. **Undoing past the fork suppresses it.** If the writer issues a further ⌘Z from the fork, the popover dismisses and undo continues — the writer is looking for something else, and the popover must never obstruct continued undoing. It is transient and non-modal at all times.
3. **Branch relegation.** If the writer undoes past a fork point and then begins typing, the new event forks at the current node per §5: the entire undone chain — *including the fork point embedded within it* — is relegated to a single non-primary branch, and the new work becomes primary. (No special mechanics needed; this falls out of the tree model, but it is the specified expectation.)
4. **Redo shows the popover immediately.** When a redo step brings the writer to a fork, the popover appears right away — redo is a deliberate forward search, so the choice is offered at once.
5. **Redoing past the choice takes the primary.** If the writer redoes again without selecting from the popover, redo proceeds along the primary child and the popover dismisses.

### T3 — Is copying *into* a buffer a history event?

| Option | Description |
|--------|-------------|
| A | Yes, always |
| B | No, never |
| C | **Only when the operation mutates text (cut-into-buffer yes; copy-into-buffer no)** |

**Decision (✅ Approved 2026-07-06): C.** The history tree is a *text-state* tree; a plain copy changes no text, so an event would be a no-op node costing the writer an extra ⌘Z — violating the spirit of "no text change ⇒ no event." Nothing is lost: buffer loads persist in `buffers.json` regardless. Cut is a deletion and is always an event.

### T4 — Copy-buffer presentation UX

| Option | Description |
|--------|-------------|
| A | Edit ▸ Copy To Buffer / Paste From Buffer submenus (⌥⌘1–9) |
| B | Floating buffers palette (utility panel): slots with content previews, click-to-paste |
| C | Inspector section alongside the Scene Inspector |
| D | Pure keyboard: ⌥⌘C → digit HUD to load; ⌥⌘V → digit/arrow HUD to paste (vim-register style) |

| Criterion | A | B | C | D |
|-----------|--:|--:|--:|--:|
| Repetitive-paste speed (eyes on text, hands on keyboard) | 3 | 3 | 2 | 5 |
| Buffer-content visibility while loading | 1 | 5 | 4 | 2 |
| Screen-estate cost | 5 | 3 | 2 | 5 |
| Discoverability | 4 | 4 | 3 | 1 |

**Decision (✅ Approved 2026-07-06): B + D combined, with A's menu items added for discoverability.** Keyboard HUD is the fast path for the scan-and-replace workflow; the palette (toggleable, non-modal) serves loading and inspection; menus are cheap and give the shortcuts a home. C rejected — the inspector is scene-scoped, buffers are project-scoped.

### T5 — What defines a "session"?

| Option | Description |
|--------|-------------|
| A | App launch |
| B | Calendar day |
| C | **Project-open span, with idle rollover (> 8 h without events mints a new session)** |

**Decision (✅ Approved 2026-07-06): C.** Windows are per-project with a single writer per project in-process, so project-open is well-defined. A misfires for multi-project users (opening another project shouldn't roll your session); B misfires for midnight writers and adds clock-edge cases. The idle rollover covers the laptop-left-open-for-days case. The warning popup shows the boundary's wall-clock time.

### T6 — Where does history live on disk?

| Option | Description |
|--------|-------------|
| A | **Inside the package: `MyProject.scrivi/history/`, gitignored** |
| B | App support root keyed by projectID (like workspace state) |

**Decision (✅ Approved 2026-07-06, via the overall design approval): A.** History travels with the project — yesterday's edits stay undoable after moving machines or cloud sync, matching the requirement's spirit — and history stays adjacent to the content it describes. Costs are manageable: Git-snapshot noise is solved by `.gitignore` (§6.c); cloud-sync churn is ~300 B appends per sentence. B silently desyncs the moment the package is copied and orphans history when projects are deleted.

## 11. Non-Functional Requirements / Performance Envelope

- Commit cost: one scene-sized string across the ABI + one JSONL append per sentence — negligible at writing speed. Worst-case single-scene size to test: 500 KB (pathological one-scene novel; integration fixture required, T-0216).
- 100k events ≈ 30–40 MB on disk (~300–400 B/line). Cold full-log replay ~0.5–2 s, reduced to near-zero by the checkpoint (only the tail replays). In-memory tree at 100k events with resident payloads ~50–100 MB → **default capacity 20,000 events** (≈ a novel's worth of sentences), max 100,000. Lazy payload loading (offsets into the log) is a documented future optimization, not v1.
- Undo/redo apply: single ranged storage replace + immediate scene save — same cost as today's save path.

## 12. Risks and Open Questions

1. **UndoManager proxy behavior — ✅ RESOLVED by spike T-0199 (2026-07-06).** Risk confirmed real: AppKit's typing coalescer corrupted a proxy `UndoManager`'s private state and threw per keystroke, and the menu never consulted it. The proxy is rejected; the confirmed mechanism is first-responder `undo(_:)`/`redo(_:)` action methods + `validateUserInterfaceItem` (§8).
2. **Auto-save ordering.** The §4.d invariant must be enforced at every save entry point (debounce, scene-exit, resign, undo-apply) through the single `flushThenSave` choke point.
3. **IME / marked text / non-keystroke mutations.** Never commit during marked text; the "edit in flight" flag must be set by *any* `textDidChange` so autocorrect isn't misclassified as a pure cursor move.
4. **Unicode discipline.** Prefix/suffix trimming must respect scalar boundaries (no torn emoji/combining sequences). All offsets C++-internal UTF-8.
5. **Protected runs during undo apply.** Ranged replace inside `sceneBoundaries[segIdx]` under `isRebuilding` must never touch divider attachments or heading runs; verify with the heading-adjacent-edit fixture.
6. **Cross-scene selection deletes.** Rare (partially blocked by existing separator guards); v1 records a barrier. Open question: silently barrier, or block the edit?
7. **Barrier honesty.** Writers will hit "can't undo past a scene merge." UI copy must be clear; structural undo is the roadmap answer.
8. **External-change scanner.** Confirm the scanner ignores unknown top-level dirs; add `history/` to its ignore set explicitly and the repair-matrix row (§6.c).
9. **Find-and-replace (future)** must route through `scrivi_history_record_event` with a `groupID` — noted in the API contract now so the schema doesn't change later.
10. **Multi-window.** One window per project and a single in-process writer per project (EP-018 model) — no concurrent-writer handling needed in v1.

## 13. Out of Scope for v1

- Structural undo (inverse create/delete/merge/reorder) — barriers instead; future extension on `groupID` + inverse ops.
- Find-and-replace (will be a grouped history client when built).
- iOS/iPadOS capture layer (editor is a stub; engine + ABI are platform-neutral, so only a UIKit `HistoryCapture` twin is needed later). visionOS with it.
- Buffer labels/reordering beyond 9 numbered slots; cross-project buffers.
- Lazy payload loading for very large histories.
- Any change to Git snapshots (they remain the coarse, optional, whole-project mechanism).

## 14. Relationship to Existing Documents

| Document | Relationship |
|----------|--------------|
| `Scrivi_Architecture_v0_3.md` | This design conforms: backend logic/persistence in C++, JSON-over-string boundary, no Swift reimplementation |
| `Scrivi_Project_Package_Structure_v0_1.md` | Adds `history/` (state.json, log-*.jsonl, buffers.json); doc gains the new directory when this design is approved |
| `Scrivi_External_Change_Repair_Matrix_v0_2.md` | Gains one row: history corrupt/missing → reset history, warn, never touch manuscript |
| `Scrivi_Backend_Behavior_Spec_v0_2.md` | New history/buffers behavior is additive; no existing behavior changes |
| Git snapshots (EP-001/T-0010) | Complementary, unchanged; `history/` excluded from snapshots |

## 15. Success Criteria (Epic-level acceptance criteria, EP-019)

1. Typing then pressing ⌘Z removes the most recent history event's text; repeated ⌘Z walks back event by event; ⇧⌘Z re-applies — in the running macOS app (**delivers the fix formerly tracked as I-0019**, closed–superseded 2026-07-06).
2. Events commit exactly per §4.a (sentence terminators `.` `!` `?`, Return, cursor-move-with-pending-changes, paste/cut, scene switch, flush); pure cursor moves/newlines without text changes produce no event.
3. Quit and relaunch: prior-session edits remain undoable; undoing past the session boundary shows the warning (once per crossing) and proceeds only on confirmation.
4. Undo N × then type ⇒ a branch is created; the new line is primary; the old branch is selectable at the fork (T2 UI) and becomes primary when selected; the abandoned text is fully restorable.
5. Capacity is configurable (T1 location), oldest events fall off when exceeded, and a branch is auto-purged when its branch point ages off; stale branches are detectable and purgeable with user confirmation.
6. Copy buffers: load ≥ 2 buffers, paste from each at multiple manuscript locations (CONOPS §9.a); each paste is one undo step; system pasteboard unaffected; buffers persist across relaunch.
7. Structural operations record barriers; undo stops at a barrier with a clear notice; no crash, no text corruption.
8. No regression: auto-save, scene navigation, scene structure ops, external-change scan, and Git snapshots behave as before; backend `ctest` and interop suites green.

---

## Appendix A — `scrivi.history.v1` / `scrivi.buffers.v1` Field Specification *(T-0200, 2026-07-06)*

Authoritative field-level spec for the `history/` store (§6). Conventions follow the house rules:
camelCase keys, `*ID` identity keys (never `id`), ISO-8601 timestamps, IDs are `<kind>_<uuid>`.
Companion doc updates: `Scrivi_Project_Package_Structure_v0_1.md` §16a/§17;
`Scrivi_External_Change_Repair_Matrix_v0_2.md` §6.21.

### A.1 Log records (`history/log-NNNNNN.jsonl`)

One JSON object per line; discriminated by `rec`; every record carries a monotonically increasing
`seq` (global across segments — `state.json` tracks `lastSeq`). Three record kinds:

**A.1.a `rec:"floor"` — scene baseline.** Appended when a scene first enters history (seed from
its `.md` file) and rewritten at the head of a fresh segment during compaction/eviction. Replay
must never read scene files (they may have changed since); floors make replay self-contained.

```json
{ "rec": "floor", "seq": 101, "sceneID": "scene_…", "reason": "seed | compaction | eviction",
  "text": "…full scene text…", "sha256": "…", "timestamp": "2026-07-06T14:00:00Z" }
```

**A.1.b `rec:"event"` — history node** (§4.b fields, flattened):

```json
{ "rec": "event", "seq": 102,
  "eventID": "evt_…", "parentID": "evt_… | null",
  "kind": "typing | delete | replace | paste | cut | barrier",
  "sceneID": "scene_…",
  "diff": { "offsetUtf8": 1234, "removed": "…", "inserted": "…" },
  "cursorBefore": 1230, "cursorAfter": 1238,
  "timestamp": "…", "sessionID": "ses_…", "groupID": null,
  "authorship": { "identityID": "identity_…", "personaID": "persona_…" },
  "meta": { "trigger": "sentence | cursorMove | paste | cut | sceneSwitch | flush", "bufferID": null } }
```

For `kind:"barrier"`, `diff`/`cursorBefore`/`cursorAfter` are omitted and a `barrier` object is
present instead: `{ "barrierKind": "sceneSplit | sceneDelete | sceneMerge | chapterSplit |
chapterMerge | reorder | externalChange", "note": "…" }`.

**A.1.c `rec:"ctl"` — pointer/maintenance operations** (log is the source of truth; the
checkpoint is derived):

```json
{ "rec": "ctl", "seq": 103, "op": "undo | redo | setPrimary | purge | evict | session",
  "timestamp": "…",
  "nodeID": "evt_…",              // undo/redo: the resulting currentNodeID
  "forkNodeID": "evt_…",          // setPrimary
  "childEventID": "evt_…",        // setPrimary
  "branchRootEventID": "evt_…",   // purge
  "newRootID": "evt_…",           // evict
  "sessionID": "ses_…", "reason": "projectOpen | idleRollover" }   // session
```

Only the fields relevant to each `op` are present.

### A.2 Checkpoint (`history/state.json`, atomic write)

Load accelerator only — on any mismatch, replay the log tail from `lastSeq`.

```json
{
  "schema": "scrivi.history.v1",
  "settings": { "capacityEvents": 20000, "staleBranchDays": 7, "idleRolloverHours": 8 },
  "rootID": "evt_… | null",
  "currentNodeID": "evt_… | null",
  "sessionID": "ses_…",
  "sessionStartedAt": "2026-07-06T14:00:00Z",
  "lastSeq": 12345,
  "activeLogSegment": "log-000003.jsonl",
  "primaryChildren": { "evt_fork…": "evt_child…" },
  "sceneHeads": { "scene_…": { "sha256": "…" } }
}
```

- `primaryChildren` lists **forks only** (nodes with ≥ 2 children); a single child is implicitly
  primary.
- `sceneHeads` holds the head-text hash per touched scene — validated against the scene files at
  `scrivi_history_open` (§6.b; mismatch ⇒ `externalChange` barrier).
- `settings` here mirrors the canonical values in `project.json` `historySettings` (Trade T1);
  `project.json` wins on disagreement.

### A.3 Buffers (`history/buffers.json`, atomic write)

```json
{
  "schema": "scrivi.buffers.v1",
  "buffers": [
    { "bufferID": "1", "label": null, "text": "Kazd'ul", "updatedAt": "2026-07-06T14:00:00Z" }
  ]
}
```

- `bufferID` is `"1"`–`"9"` in v1 (string, to allow named registers later); `label` reserved,
  null in v1. Empty slots are omitted.

### A.4 `project.json` addition (Trade T1)

```json
"historySettings": { "capacityEvents": 20000, "staleBranchDays": 7, "idleRolloverHours": 8 }
```

Optional block (absent in older projects ⇒ defaults above), read/written by
`ScriviCore/src/schemas/ProjectJson`.

---

*Document Status: ✅ Approved baseline — v0.1 created and approved 2026-07-06 (EP-019 planning session; I-0019 root cause confirmed). All six trades ruled by the user 2026-07-06: T1=B, T2=A-with-refinements (§10 T2), T3=C, T4=B+D+A, T5=C, T6=A. The T-0198 design sign-off gate is met. Appendix A (T-0200 schema spec) added 2026-07-06; SP-051's remaining scope is the T-0199 spike.*
