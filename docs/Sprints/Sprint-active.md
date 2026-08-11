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

**Status:** 🟢 **COMPLETE — all 11 items ✅ Verified (2026-08-11).** Awaiting **user approval to close**
(CLAUDE.md: only the user closes a Sprint). EC1–EC12 all pass. The sprint grew from 6 planned items to 11:
**I-0107** and **I-0108–I-0111** were all found by live verification, not by CI.
ctest **413/413** · interop **53/53** · macOS BUILD SUCCEEDED.
**Epic:** EP-019 `[Cross]` — Undo/Redo, History & Copy Buffers.

> ### Progress 2026-08-09 — I-0104 + I-0105 landed (the sequencing note's "land first" pair)
>
> **I-0104** `[ScriviCore]`+`[Apple]`. Root cause was exactly as diagnosed: `persistState` hashed
> `service_->headTextForScene()` (`HistoryStore.cpp:386`) while `validateSceneHead` hashed the on-disk text
> (`:400`) — two different artifacts. Fixed by teaching history what the save path actually wrote:
> new `scrivi_history_note_scene_persisted` C ABI → `HistoryStore::noteScenePersisted` stores the sha256 of
> those bytes, and `persistState` prefers it over a head-derived hash (falling back to the loaded baseline for
> a scene never saved this session — a head-derived fallback would have reintroduced the bug). Called from the
> single save choke point, `ViewportSceneLoader.saveScene`, which every save path funnels through.
> **The barrier itself is untouched** — a genuine external edit still raises it, per the non-negotiable.
>
> **I-0105** `[Apple]`. `HistoryCapture.revision` bumped on each tree mutation; threaded through
> `CardContext.historyRevision`; `HistoryCardBody` keys `.task(id:)` on `"sceneID#revision"`. Caret offset
> excluded by design — bolding is pure render, so re-fetching per arrow key would be gratuitous.
>
> **Evidence.** 3 new ctest cases, each **verified RED without the fix** (tests 347/349 fail pre-fix; 348, the
> genuine-external-edit direction, correctly passes either way). **ctest 391/391** — the count moved from 388,
> which satisfies the sprint's stale-cached-binary non-negotiable. macOS **BUILD SUCCEEDED**, no new warnings.
> No new `.swift`/`.cpp` files, so **no pbxproj change was required**.
>
> ✅ **Interop suite: 46/46 passing (2026-08-10).** Yesterday's *"Could not launch ScriviInteropTests"* was a
> **toolchain-selection** problem, not a project defect: `xcode-select -p` points at the Xcode **26.6**
> release, whose max supported deployment target is 26.5.99, so LaunchServices refused a test host built for
> `MACOSX_DEPLOYMENT_TARGET = 27.0`. Xcode **27.0 beta** (`27A5228h`) is installed and runs it fine.
>
> **Procedure** (no `xcode-select` change needed — this leaves the global toolchain alone):
>
> ```bash
> DEVELOPER_DIR=/Applications/Xcode-beta.app/Contents/Developer \
> /Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild \
>   -scheme ScriviApp -destination 'platform=macOS' test
> ```
>
> ⚠️ **Read the right line.** The run prints `Executed 0 tests, with 0 failures` — that is the **legacy
> XCTest** counter and is misleading. The real result is the Swift Testing line:
> `Suite ScriviInteropTests passed after 0.104 seconds` / `Test run with 46 tests in 1 suite passed`.
> Do not treat `** TEST SUCCEEDED **` alone as evidence; a suite that launched and ran nothing also prints it.
>
> This retires the "no automated coverage for `historyNoteScenePersisted`" caveat recorded 2026-08-09.

> ### Progress 2026-08-10 — T-0396 landed, and **§4.d was relaxed** (user-approved)
>
> **⚠️ The approach changed at implementation, with a user ruling.** The sprint plan said "record the pending
> text so disk is covered, and the entry **stays open for continuation**". That turned out not to be
> implementable as written: `HistoryService::record` **always appends a node** and diffs against the current
> head (`HistoryService.cpp:204-206`), and there is **no amend/extend path**. So a save that records anything
> necessarily creates a node — and since undo walks **one node per step**, re-arming an app-side latch would
> still have left the writer with one visible row but three ⌘Z stops.
>
> **Ruled (user, 2026-08-10): defer the commit.** An autosave mid-session now writes the file and records
> **nothing**; the entry commits once at a real boundary (idle, or any kept AC2 trigger, or close).
> One node, one undo step, one row.
>
> **Consequence — §4.d is relaxed, deliberately.** The invariant "disk never contains text no history node
> describes" no longer holds strictly: **disk may lead history by at most one save's worth of typing while a
> session is open.** This is bounded — `close()` commits pending text, so a clean quit loses nothing, and the
> only exposed window is a hard crash mid-session, where I-0104's head-hash check raises an `externalChange`
> barrier so undo stops at the last node it can honestly describe. Failure mode is *"undo stops early"*, never
> *"undo corrupts the manuscript"*. **T-0217 must document this** alongside the AC2 amendment — it is a change
> to a stated invariant, not just an omission being filled in.
>
> **Implementation.** `flushThenSave()` no longer flushes mid-session (commits only if already idle); a
> **45 s** `idleTask` (within the ruled 30–60 s band) seals the session on its own, so a writer who stops
> mid-sentence and walks away still gets their entry; `noteEdit` commits first if the gap already exceeded the
> threshold, then folds the keystroke into a fresh entry. **Backspace does not commit** — it is an ordinary
> edit into the open latch, so intra-word correction stays in-session. **All kept triggers are untouched**:
> `cursorMove`/`sentence`/`paste`/`cut`/`sceneSwitch` still call `flush` directly at their own call sites
> (`ManuscriptTextView.swift:638-806`).
>
> **Evidence.** ctest **391/391**; interop **48/48** (was 46) on Xcode 27 beta. The two new interop cases
> assert the engine-level property the fix rests on — one record per session = one node = one undo step,
> versus the pre-fix per-save shape that yields three of each.
>
> ⚠️ **Coverage gap, stated plainly.** `HistoryCapture` is **not compiled into the test target** (which builds
> only `ScriviEngine.swift` + `ScriviError.swift`), so the coalescing decision itself has **no automated
> test** — I first wrote tests against `HistoryCapture` and they failed to compile for exactly this reason.
> The new tests cover the engine invariant beneath it, not the app-side timing. **The 45 s idle boundary, the
> deferred save, and backspace-does-not-split are therefore live-verification items**, not proven by CI.

> ### Progress 2026-08-10 — I-0106 + T-0398 landed (`removedLength` shipped once, as sequenced)
>
> **The shared payload.** `TreeNode.removedLength = diff.removed.size()` added in `buildTree`, serialized by
> the C ABI, decoded in `HistoryTreeNode` (absent-key tolerant per I-0094), and the `scrivi.h` doc block —
> which had never documented the change fields at all — now specifies all three. **One envelope change, two
> consumers**, exactly as the sequencing note required.
>
> **I-0106 (a) — deletions get a real span.** `contains(caret:)` gives a pure deletion a 1-byte span at the
> seam where text was removed, instead of the degenerate `caret == changeOffsetUtf8` branch that matched at
> the same offset an adjacent insertion's half-open range also contained.
>
> **I-0106 (b) — a boundary tie-break exists now.** The winner is resolved **once across the shown set**
> (`caretNodeID(in:)`) rather than evaluated per row, so at most one row can ever bold. `orderedNodes` is
> spine-newest-first, so `first(where:)` implements the ruled winner: **the most recent node claims a shared
> boundary** — the caret where the writer just typed belongs to what they just typed.
>
> **T-0398 — deletions look like deletions.** `minus.circle[.fill]` instead of `circle.fill`, an orange glyph
> (insertions keep the existing `.secondary` — this adds a distinction, it does not restyle every row), a
> `"Deleted …"` label prefix, and a byte-count label for a deletion with an empty preview. `cut` counts as a
> deletion too, since it removes text from the scene.
>
> **Evidence.** ctest **394/394** (3 new cases: deletion, pure insertion, replacement — **verified RED**:
> reverting the payload line fails 136 + 138, while the insertion case correctly stays green). Interop
> **50/50** (2 new). macOS BUILD SUCCEEDED. No new files → no pbxproj change.
>
> ### ⚠️ Build-hygiene finding — the stale-test-binary trap fired for real
>
> Mid-task, `ctest` reported **391/391 with the new tests apparently passing — but they had never been
> compiled.** `build/CMakeCache.txt` had `SCRIVI_BUILD_TESTS:BOOL=OFF`: the **Xcode build reconfigured the
> CMake cache**, silently turning tests off, and `ctest` replayed the previously-built binary. This is exactly
> the failure this sprint's non-negotiables warn about (SP-091 precedent), and it means **the 391/391 recorded
> for T-0396 on 2026-08-10 was a cached replay, not a real run** (re-verified since: genuinely green).
>
> **Practical rule:** after any `xcodebuild` run, re-run `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON` before
> trusting `ctest`, and **confirm the test count moved** when tests were added (391 → 394 here).
>
> _Confirmed again during T-0397: the cache had flipped to OFF a second time, right after the interop run.
> This is reproducible, not a one-off._

> ### Progress 2026-08-10 — T-0397 landed; **all six SP-093 items now Implemented**
>
> **The design choice worth recording: `preview` was left alone.** The obvious fix — stop rewriting `\n`/`\r`/
> `\t` to spaces in `forkPreview` — would have changed the string **four** consumers render (fork popover,
> stale-branch list, purge confirmation, tree rows), turning a low-priority label fix into a cross-surface
> behaviour change. Instead the classification travels as its **own field**, exactly as the sprint specified
> ("the kind must survive from the diff to the label rather than being flattened at preview time").
>
> **Implementation.** New `TreeNode.whitespaceKind`, set by a `whitespaceKind()` classifier that returns
> `""` the moment it sees a non-whitespace byte — so the common case costs one early exit and no allocation.
> Wire format is `"<kind>:<count>"` (`"newline:2"`, `"tab:1"`, `"space:3"`); mixed runs report the dominant
> kind, ties resolving **newline > tab > space** (the coarser boundary is the more meaningful thing to name).
> `\r` is not counted, so CRLF reports as one newline. Swift parses it once in
> `HistoryTreeNode.whitespaceLabel` — `"⏎ new paragraph"` / `"⇥ tab"` / `"␣ 3 spaces"`, pluralised — so every
> consumer gets identical wording rather than each view inventing its own. The card checks it **before**
> `preview`, since that is precisely the path that trimmed to empty and produced `"(no text)"`.
>
> **Deletions are named too.** The classifier falls back to `diff.removed`, so a *deleted* paragraph break
> reads `"Deleted ⏎ new paragraph"` — composing correctly with T-0398's prefix rather than fighting it.
>
> **Evidence.** ctest **398/398** (4 new cases, **verified RED**: reverting the one assignment line fails
> 139/140/142, while the "empty for real text" case correctly stays green — it asserts an absence).
> Interop **52/52** (2 new, including an assertion that `preview` really does trim to empty, pinning the
> original failure mode). macOS BUILD SUCCEEDED. No new files → no pbxproj change.

---

## ✅ SP-093 — all tasks Implemented (2026-08-10); **awaiting live verification**

| ID | Title | Status |
| -- | ----- | ------ |
| I-0104 | `externalChange` fires every open | ✅ **Verified (2026-08-11, user-confirmed)** — EC1 passes; two launches, no new false notices |
| I-0105 | History card doesn't refresh on commit | ✅ **Verified (2026-08-10, user-confirmed)** — EC6: the card refreshes on commit without a scene switch or relaunch. |
| T-0396 | Typing-session coalescing | ✅ **Verified (2026-08-10, user-confirmed)** — EC2–EC5: the reference sentence records as ONE entry; an autosave does not seal it; a pause beyond the idle threshold starts a new entry; backspace mid-word does not split. ⚠️ §4.d relaxed — T-0217 must document it. |
| I-0106 | Wrong entry bolded | ✅ **Verified (2026-08-10, user-confirmed)** — EC7 + EC9 (with [[I-0107]], which fixed the offset drift this alone did not). |
| T-0398 | Added vs. deleted text | ✅ **Verified (2026-08-10, user-confirmed)** — EC10: insertions and deletions are distinguishable at a glance. |
| T-0397 | Whitespace-kind labels | ✅ **Verified (2026-08-10, user-confirmed)** — EC11: whitespace events read as named whitespace, never "(no text)". |
| I-0107 | Caret highlight uses stale offsets (found in live verify) | ✅ **Verified (2026-08-10)** |
| I-0108 | `[Apple]` Stale branches listed project-wide under "This scene only" — card now shows a **badge only**; purge lives in Project Settings | Low | ✅ **Verified (2026-08-11, user-confirmed)** |
| I-0109 | `[Apple]` Navigator: Return does not open the arrow-key-selected scene (arrows stay browse-only) | Low | ✅ **Verified (2026-08-11, user-confirmed)** |
| I-0110 | `[ScriviCore]` **History fails to open** — `unknown node` exception when a replayed purge contains the current node; project loses undo/redo | High | ✅ **Verified (2026-08-11, user-confirmed)** |
| I-0111 | `[ScriviCore]` Prune-driven purges written with `seq 1` — log sequence numbers regressed (diagnostic only) | Low | ✅ **Verified (2026-08-11, user-approved)** — diagnostic-only; approved on test evidence |

**Suites:** ctest **404/404** · interop **52/52** · macOS BUILD SUCCEEDED. Both re-run 2026-08-10 after the
I-0107 correction, with the app instance closed to unblock the test host.

> ### I-0107 — found by the user's live verify (2026-08-10), fixed in-sprint
>
> **The first live verification of SP-093 found a defect my I-0106 fix did not address.** Placing the caret
> before "Now is the winter…" bolded a *different* row; two later paragraphs could never be bolded at all;
> the whitespace rows were unreachable. The user's own read — *"the initial two new paragraphs are somehow
> capturing the highlight position"* — was essentially right, with one refinement: those newlines were not
> capturing the highlight, they were **displacing every earlier entry by their 2 bytes** while never becoming
> hittable themselves.
>
> **One root cause, three symptoms.** `changeOffsetUtf8` is where an edit landed *when recorded*; the caret is
> a position in the text *as it is now*. Nothing rebased the two, so every entry's hit zone drifted by the
> number of bytes inserted before it since — exactly the 2-character lag reported, and enough to push some
> spans permanently out of reach.
>
> **I-0106 was a real but partial fix.** It corrected the *tie-break* (two rows bolding at once) and the
> deletion span; it did not address offset drift, because a stored offset in an append-only log looks stable
> until you ask what it means in the current document. Both fixes are needed; neither subsumes the other.
>
> **Fix.** `buildTree` rebases each spine node's offset onto the current text; off-spine nodes keep their
> stored offset — their text is not in the document, so no caret corresponds to them. `contains(caret:)` uses
> `max(changeLength, 1)`, so **every recorded row is reachable**, which the user asked for as a general rule:
> *"any entry row you make to the history panel should be highlightable via the position of the cursor."*
> **Ruled: exact-hit only**, no nearest-entry fallback — if nothing is under the caret, nothing bolds.
>
> ### ⚠️ The first I-0107 fix made it WORSE — rejected in live verify, corrected 2026-08-10
>
> The first implementation accumulated **one running total** of every later edit's net length and applied it
> to each older node. That is wrong: **a later edit only moves an older one if it happened at or before that
> node's position.** Text appended *after* an entry leaves it exactly where it was. So an entry followed by
> seven typed sentences got displaced by all seven — the user reported *"Now is the winter…" highlights only
> when the cursor is six lines below the line it refers to*, and almost nothing else would highlight at all.
>
> **Why the tests passed anyway — the important lesson.** Every original rebase test inserted at **offset 0**,
> which is always "before" the node being checked. The naive running total is *accidentally correct* for that
> shape, so four green tests certified a fundamentally broken rule. Coverage of one shape is not coverage of
> the rule.
>
> **Corrected implementation.** Each node is transformed through every subsequent same-scene edit
> individually, oldest-first, the way operational transforms compose:
> - an edit strictly **after** the node → no effect;
> - a removal **spanning** the node's position → collapses it to the cut point;
> - otherwise → apply the net `inserted − removed`.
>
> Two new tests pin exactly this: *"a later APPEND does not move an earlier node"* (the reported case, seven
> appended sentences) and *"a later insertion moves only nodes after it"*. **Both fail against the first
> implementation**, which is the evidence that matters — they would have caught the regression before it
> reached the user.
>
> ### ✅ I-0107 Verified (2026-08-10, user-confirmed)
>
> The user rebuilt and re-verified live on the reported scene: *"It behaves… the bolded changes seem to
> highlight in the right places."* Verification was explicitly conditioned on the interop suite, re-run after
> the correction with the app closed: **52/52** (all six SP-093 cases confirmed individually in the run log),
> ctest **404/404**.
>
> ⚠️ **Scope of this verification, stated honestly.** The user was explicit: *"I cannot test all conditions
> live."* This is a **spot-check of the reported scene**, not a sweep of the rebase rule. Deletions spanning
> an entry, multi-scene interleaving, and rebasing across branch switches are covered by ctest but have **not**
> been exercised by hand. Treat a future report in this area as plausible rather than surprising.
>
> ### Live verification round 2 (2026-08-10) — **EC1 failed, EC2–EC12 all passed**

User ran the full exit-criteria list against a new chapter of `the-stairs-of-tintagael.scrivi`:

| Criterion | Result |
| --------- | ------ |
| **EC1** — no `externalChange` on quit→reopen | ❌ **FAILED** — 3 restarts produced 3 barriers |
| EC2–EC12 | ✅ **All passed** (EC12 "apparently") |

**This means the substance of SP-093 is confirmed.** The reference sentence records as one entry, the idle
boundary behaves, backspace does not split, the kept triggers still commit, the card refreshes on commit, and
the caret/label/deletion work all verified live. Only the head-hash trigger remained broken.

> #### EC1 root cause — a **second** I-0104 defect, surfaced by T-0396
>
> The first fix was correct as far as it went: the save path records the bytes it writes. But
> `HistoryCapture.close()` **flushes** — advancing the history head — and **nothing saves after it**, because
> `ViewportSceneLoader.saveScene` early-returns once `isDirty` is false and the last autosave already cleared
> it. The head therefore moved past the last hash `noteScenePersisted` recorded, and `persistState` fell back
> to hashing the replayed head. Next open compared head-to-disk: the original bug, reached by a new route.
>
> **T-0396 is what exposed it.** Before deferring the save-time commit, the closing flush had nothing pending
> to commit, so the head never advanced after the final save. The two changes are individually sound and
> interact badly — worth noting, because neither task's own tests could have caught it.
>
> **Fix:** `close()` captures the pending scene + text *before* flushing, then records it via
> `noteScenePersisted`. That text is already on disk — autosave wrote the file; T-0396 deferred only the
> history event, never the write — so the persisted hash stays disk-accurate.
>
> **Regression coverage at both layers**, each replaying the reported three-cycle shape: ctest **405/405**,
> interop **53/53**.

### ✅ EC1 resolved 2026-08-11 — root-caused by instrumentation, not by another hypothesis

**The repair was never broken. Nothing was persisting it.** `beginTermination()` saved the window manifest
but never called `historyCapture.close()`, so `scrivi_history_close` → `checkpoint()` never ran on quit and
every `externalChange` repair died with the process. Each launch: detect the stale baseline → record a
barrier → re-seed the floor **correctly** → discard it.

**How it was found.** After four wrong hypotheses, an instrumented probe drove the real C ABI against a
**copy** of the project. Two results settled it:

- Given the actual bytes of the actual files, the engine flagged **zero** scenes — so ScriviCore was innocent.
- The probe (open → validate every scene → **close**) healed **all 14 in one cycle**: 59 matching, 0
  mismatching. The only thing it did that the app did not was close the history.

Forensics on `chapter-I/E-scene.md` confirmed the shape: 79 floor records, `state.json` frozen on **floor #2**
(seq 351, len 911), floors #3–#79 all byte-identical to the file (len 910). The repair had been right **77
consecutive times**.

**Why four earlier attempts failed.** Every test wrote called `scrivi_history_close`, because that is the
obvious way to end a session — and that call *was* the fix. The tests could not fail. I was testing the
contract while the app violated it, and no amount of test-writing could reveal that. **Driving the real
project was the only thing that could.** Two of those attempts shipped and were reverted: hashing
`pendingText` at close made it strictly worse (7→8 barriers on one relaunch), and a `checkpoint()` precedence
change broke three existing tests.

**Also fixed en route:** nothing saved scenes on quit at all (`saveAllDirty` ran only on `willResignActive`) —
the user's *"not saving/restoring the selected scene"*; and `stampWritingSurface` wrote scene files without
reporting the bytes to history.

**User-verified over two launches:** *"The externals are properly repaired and no new false external edit
notices are being generated."*

### Two findings from the same session — I-0108 / I-0109 (both `[Apple]`, both fixed)

**I-0108 — stale branches ignored "This scene only".** They are genuinely project-wide, and `StaleBranch`
carries no `sceneID`, so the card had nothing to filter on; showing the list under a per-scene toggle read as
a filter bug. It was also a **duplicate** of Project Settings' scan/list/purge surface (T-0212/SP-055).
**User-ruled:** card keeps a **badge only** ("Across the whole project — manage in Project Settings…"); the
list, `Discard…` buttons and purge confirmation are removed.

> ⚠️ **This amends T-0366**, which specified "stale badges + user-confirmed purge" *in the card*. Purge is now
> Project-Settings-only — one irreversible operation, one entry point. Recorded here rather than done quietly,
> since T-0366 was already user-Verified. **Rejected alternative:** threading `sceneID` through the C ABI to
> filter per-scene — more work to show what Project Settings shows better, and a branch spanning scenes would
> filter by its root scene, which is arbitrary.

**I-0109 — Return did nothing in the Navigator.** Navigation was bound solely to `onTapGesture`; the macOS
selection is highlight-only *by design* (`SceneNavigatorView.swift:57`), so no key path reached
`navigate(to:)`. Added `.onKeyPress(.return)`, macOS-only — on iOS the `selection` binding already drives
Master/Detail, so a Return handler would double-trigger. **Arrow keys stay browse-only**, which the user
explicitly asked to keep: *"it allows me to rapidly move to another scene."*

### 🟢 SP-093 complete (2026-08-11) — 11 items, all Verified

| # | ID | Verified |
| - | -- | -------- |
| 1 | I-0104 `externalChange` fires every open | 2026-08-11 (EC1, two launches) |
| 2 | I-0105 card doesn't refresh on commit | 2026-08-10 (EC6) |
| 3 | T-0396 typing-session coalescing | 2026-08-10 (EC2–EC5) |
| 4 | I-0106 wrong entry bolded | 2026-08-10 (EC7/EC9) |
| 5 | T-0398 added vs. deleted | 2026-08-10 (EC10) |
| 6 | T-0397 whitespace-kind labels | 2026-08-10 (EC11) |
| 7 | I-0107 caret offsets never rebased | 2026-08-10 |
| 8 | I-0108 stale branches ignored the scene filter | 2026-08-11 |
| 9 | I-0109 Navigator Return did nothing | 2026-08-11 |
| 10 | I-0110 history failed to OPEN (`unknown node`) | 2026-08-11 |
| 11 | I-0111 purge records written with `seq 1` | 2026-08-11 (approved on test evidence) |

**Suites at close:** ctest **413/413** (from 388 at sprint start) · interop **53/53** (from 46) · macOS
BUILD SUCCEEDED. No new source files, so **no pbxproj change** was required at any point.

> ### The finding that matters for how the next sprint is run
>
> **Five of eleven items — I-0107 and I-0108 through I-0111 — were found by live verification, not by CI.**
> Two were severe: I-0110 stopped history opening at all (the project ran with no undo/redo), and I-0104's
> real cause survived four wrong hypotheses and two shipped-then-reverted fixes, one of which made the
> symptom measurably worse for the writer.
>
> **Why the tests kept agreeing with me.** Three separate times a test passed while testing nothing:
> the I-0107 rebase tests only ever inserted at offset 0 (always "before", the one shape the naive rule gets
> right); the I-0104 tests all called `scrivi_history_close`, which *was* the missing fix, so they could not
> fail; the first I-0111 test left its bogus node unparented, so the prune never reached it. Each suite
> exercised the shape I already had in mind.
>
> **What actually resolved it:** an instrumented probe driving the real C ABI against a **copy** of the
> writer's project. It answered in one run what four rounds of reasoning had not — the engine flagged zero
> scenes, so the defect had to be in what the app did around it. **Reach for real-data instrumentation
> earlier**, especially when a hypothesis needs a test that cannot fail.

### Lesson recorded — why this took three attempts
>
> I-0106 fixed the tie-break but not offset drift. The first I-0107 fix addressed drift but with a rule that
> was wrong for the most common editing shape (appending). Both passed their tests. The through-line: **each
> test suite only exercised the shape its author had in mind** — offset-0 insertions in the first case — so
> green tests certified a broken rule twice. When a fix turns on a variable (here, *where* a later edit
> happened), the tests must vary that variable, not hold it constant.

### What live verification must cover — and what CI does *not* prove

Every exit criterion below the line was checked by automated tests; these were **not**, and cannot be:

1. **The reference sentence** — *"Now is the winter of our discontent made glorious summer by this son of
   york"* typed continuously must record as **one** entry, not three. This is the headline case.
2. **A 1–2 s pause must not split** the entry, and a pause **beyond ~45 s** must start a new one.
3. **Backspace mid-word must not split** the entry.
4. **The kept triggers still commit** — cursor-move, sentence terminator, paste/cut, scene switch.
5. **Quit → reopen raises no `externalChange`** for a scene edited only inside Scrivi; a scene genuinely
   edited by an external editor **still does**.
6. **The card refreshes on commit** without a scene switch or relaunch.
7. **Caret at an entry's start bolds that entry**; a deletion bolds **exactly one** row.
8. **A deletion looks like a deletion**, and a newline event reads as `⏎ new paragraph`, never `(no text)`.
9. **(I-0107 re-verify, on the reported scene — Ch 2 Sc 6 "The Safe Way Home")** — the caret placed **just
   before** the "N" of "Now is the winter…" bolds **that** entry, with no 2-character lag; the two later
   paragraphs that could never be bolded now can be; and the `⏎ 2 new paragraphs` / `␣ 2 spaces` rows bold
   when the caret sits on them. **Every row in the panel should be reachable** — that is the rule to test
   against, not just these examples.

⚠️ **The largest gap is T-0396's timing logic.** `HistoryCapture` is not compiled into the test target, so the
45 s idle boundary, the deferred save, and backspace-does-not-split have **no automated coverage at all** —
items 1–4 rest entirely on live verification.

⚠️ **§4.d was relaxed** (see the T-0396 note). That is a changed invariant, not a bug fix, and **T-0217 must
document it** before EP-019 is put forward for close.
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
| I-0104 | `[ScriviCore]` `externalChange` fires every open — head hash taken over replayed text, not disk bytes | High | ✅ **Verified (2026-08-11, user-confirmed)** — EC1 passes; two launches, no new false notices |
| I-0105 | `[Apple]` History card doesn't refresh on commit — new edits appear only after reopen | High | ✅ **Verified (2026-08-10, user-confirmed)** — EC6: the card refreshes on commit without a scene switch or relaunch. |
| T-0396 | `[Apple]` Typing-session coalescing — deferred save-commit + 45 s idle timer | High | ✅ **Verified (2026-08-10, user-confirmed)** — EC2–EC5: the reference sentence records as ONE entry; an autosave does not seal it; a pause beyond the idle threshold starts a new entry; backspace mid-word does not split. ⚠️ §4.d relaxed — T-0217 must document it. |
| I-0106 | `[Apple]` Wrong entry bolded — caret-at-boundary + deletions match two rows | Medium | ✅ **Verified (2026-08-10, user-confirmed)** — EC7 + EC9 (with [[I-0107]], which fixed the offset drift this alone did not). |
| T-0398 | `[Cross]` Distinguish added vs. deleted text in history rows | Medium | ✅ **Verified (2026-08-10, user-confirmed)** — EC10: insertions and deletions are distinguishable at a glance. |
| T-0397 | `[Cross]` Whitespace-kind labels instead of "(no text)" | Low | ✅ **Verified (2026-08-10, user-confirmed)** — EC11: whitespace events read as named whitespace, never "(no text)". |
| I-0107 | `[Cross]` Caret highlight uses **stale offsets** — hit zones drift; zero-span rows unreachable | High | ✅ **Verified (2026-08-10, user-confirmed)** |
| I-0108 | `[Apple]` Stale branches listed project-wide under "This scene only" — card now shows a **badge only**; purge lives in Project Settings | Low | ✅ **Verified (2026-08-11, user-confirmed)** |
| I-0109 | `[Apple]` Navigator: Return does not open the arrow-key-selected scene (arrows stay browse-only) | Low | ✅ **Verified (2026-08-11, user-confirmed)** |
| I-0110 | `[ScriviCore]` **History fails to open** — `unknown node` exception when a replayed purge contains the current node; project loses undo/redo | High | ✅ **Verified (2026-08-11, user-confirmed)** |
| I-0111 | `[ScriviCore]` Prune-driven purges written with `seq 1` — log sequence numbers regressed (diagnostic only) | Low | ✅ **Verified (2026-08-11, user-approved)** — diagnostic-only; approved on test evidence |

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
> **T-0369 does not need its own sprint.** The old SP-093 (the struck-through *"SP-093: `[Apple]` EP-030 —
> verification & Epic close"* entry in `Sprint-backlog.md`) and SP-057
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
>
> ✅ **Propagated 2026-08-09.** The backlog files have been realigned: `Sprint-backlog.md` now carries SP-093
> (EP-019 history) and SP-094 (merged verification) as real entries with the old EP-030-close entry struck,
> SP-057 marked ⚪ superseded, and **EP-031 renumbered SP-094–SP-099 → SP-095–SP-100** to clear the collision
> with SP-094; `Task-backlog.md` has T-0369 reassigned to SP-094 and T-0396's stale "retire cursor-move"
> description replaced with the corrected diagnosis.

_Next available Sprint **SP-095** (after this draft and SP-094 activate), next Task **T-0399**._
