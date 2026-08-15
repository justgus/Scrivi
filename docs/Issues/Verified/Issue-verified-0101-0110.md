# Verified Issues — I-0101 to I-0110


---

## I-0101

**Status:** ✅ **Resolved - Verified (2026-08-05, user-confirmed)** — the writer removed the stranded `placeholder` card from Chapter 7 Scene 6 via the new per-card ✕, and confirmed it **stays removed across a relaunch** (the layout write persisted). BUILD SUCCEEDED; ctest 381/381.
**Severity:** Medium
**Sprint:** **SP-091**

**Description / Resolution:**
`[Apple]` **An unavailable ("unknown-typeID") inspector card cannot be removed — no UI affordance.** Found in the SP-091 live-verify (2026-08-05): Chapter 7 Scene 6 of `the-stairs-of-tintagael.scrivi` still carried a `placeholder` card in its Writing stack — the SP-090 scaffolding card that SP-091 retired — and the writer had **no way to clear it**. **Root cause:** `InspectorCardStackView.unknownCardsNotice(_:)` rendered unresolved typeIDs as a passive, display-only summary row ("N cards aren't available yet" + a comma-joined list) with no per-card action. The store's `removeCard(typeID:sceneID:stack:)` already handled unknown types correctly (it operates on `rawEntries`, which deliberately retains unresolved entries so an edit cannot silently drop them) — **only the UI was missing**. The SP-090 close-out note claiming such cards were "removable by the writer" was wrong; AC11 was verified for *reporting* but never for *removal*. **Fix:** the notice becomes one **removable row per unavailable card**, each with its own ✕ calling the existing `removeCard`, styled with a dashed border to read as a placeholder rather than a real card. `InspectorCardStackView.swift` only; no core/ABI/pbxproj change (edited file already tracked).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0104

**Status:** ✅ **Resolved - Verified (2026-08-11, user-confirmed)** — root-caused by an **instrumented probe on a copy** of the writer's project after four failed hypotheses. **The repair was never broken:** `validateSceneHead` recorded a barrier and re-seeded the floor from disk correctly every time — forensics on `chapter-I/E-scene.md` showed the floor had been right for **77 consecutive repairs** while `state.json` stayed frozen on **floor #2 of 79** (seq 351, len 911, vs. the correct 910). **The app never CLOSED the history on quit**, so `scrivi_history_close` → `checkpoint()` never ran and every repair died with the process. Proof: the probe (open → validate every scene → **close**) healed **all 14 in one cycle** — 59 matching, 0 mismatching. **Fix:** `beginTermination()` now saves every dirty scene synchronously *and* closes each session's history. Also fixed en route: nothing saved scenes on quit at all (`saveAllDirty` ran only on `willResignActive`), which was the user's *"not saving/restoring the selected scene"*; and `stampWritingSurface` wrote scene files without reporting the bytes. ⚠️ **Two earlier attempts were wrong and were reverted** — hashing `pendingText` at close (made it worse: 7→8 barriers on one relaunch, because T-0396 defers the commit so that text may never have reached disk) and a `checkpoint()` precedence change (broke 3 existing tests). **User-verified over two launches (2026-08-11):** *"The externals are properly repaired and no new false external edit notices are being generated."* ctest 411/411, interop 53/53.
**Severity:** Medium
**Sprint:** **SP-093**

**Description / Resolution:**
`[ScriviCore]` **`externalChange` barriers fire on every open for normally-edited scenes — the head hash is taken over replayed in-memory text, not the bytes written to disk.** Reported 2026-08-07 in the SP-092 live-verify: the writer sees "This scene was changed outside Scrivi…" on essentially every relaunch despite never editing outside the app. **Supersedes/generalises I-0103**, which framed this as a trailing-newline discrepancy affecting 2 scenes; the defect is the comparison itself, and the trailing newline is one symptom of it. **Root cause:** at close, `HistoryStore::persistState` writes `sceneHeads[sceneID].sha256 = sha256Hex(service_->headTextForScene(sceneID))` (`HistoryStore.cpp:386`) — the **replayed history head**, i.e. the concatenation of recorded diffs. At open, `validateSceneHead` (`HistoryStore.cpp:394-422`) re-hashes the scene's **on-disk text** and compares. These two are different artifacts: the manuscript save path (autosave, `flushThenSave`) writes the editor's text with its own normalization, and any save that lands after the last recorded history event — or any newline normalization the writer applies — diverges the head from the file **without any external editing at all**. The mismatch then re-seeds the floor and records a barrier, which is why the noise recurs indefinitely rather than settling. **Fix:** hash the bytes actually persisted to the scene file at save time and compare disk-to-disk, so the check detects genuine third-party modification only. **Do not** simply suppress the barrier — the barrier is correct behaviour for a real external edit; only its trigger is wrong.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0107

**Status:** ✅ **Resolved - Verified (2026-08-10, user-confirmed)** — user rebuilt and re-verified live on `the-stairs-of-tintagael.scrivi` Ch 2 Sc 6: *"It behaves… the bolded changes seem to highlight in the right places."* Verification was explicitly conditioned on the interop suite, which then passed **52/52** (app instance closed to unblock the test host). ctest **404/404**. ⚠️ **First fix made it worse** (single running total shifted every node by all later edits regardless of position — user: *"highlights only when the cursor is six lines below"*); corrected to per-edit operational transform: an edit strictly **after** a node does not move it, a removal spanning it collapses it to the cut point, otherwise net delta. The 2 tests added for that case **fail against the first implementation**. ⚠️ **Not exhaustively tested live** (user: *"I cannot test all conditions live"*) — spot-checked, not swept.
**Severity:** High
**Sprint:** **SP-093**

**Description / Resolution:**
`[ScriviCore]`/`[Apple]` **The history card highlights the wrong entry because change offsets are never rebased — every row's hit zone drifts as later text is inserted, and zero-span rows are unreachable.** Reported by the user 2026-08-10 in a live verify on `the-stairs-of-tintagael.scrivi` Ch 2 Sc 6 ("The Safe Way Home"). Symptoms: positioning the caret just before the "N" of "Now is the winter…" bolded a *different* entry (a new-paragraph row); advancing one character bolded the *second* new-paragraph row; only at the **third** character did the expected entry bold. The same 2-character lag appeared at every sentence boundary in the paragraph. Two later paragraphs could **never** be bolded at any caret position, and the whitespace rows (`⏎ 2 new paragraphs`, `␣ 2 spaces`) could not be bolded either. **Root cause (one defect, three symptoms):** `TreeNode.changeOffsetUtf8` is `diff.offsetUtf8` — where the edit landed **when it was recorded**. Every subsequent insertion/deletion in that scene moves the text, but the stored offset never moves, so `HistoryTreeNode.contains(caret:)` compares a **current** caret position against **historical** coordinates. The writer's two leading newlines (2 bytes) displaced every earlier entry by exactly 2 — matching the reported lag precisely — and entries whose stale spans drifted beyond any reachable caret position became permanently unhittable. **Secondary defect:** an event that inserted nothing had `changeLength == 0`, so `contains(caret:)` matched only a single exact byte, making whitespace-only rows effectively impossible to land on. **Fix:** (a) `buildTree` walks the primary spine newest→oldest accumulating each node's net length change **per scene**, and reports each node's **rebased** (current) offset; side branches keep their stored offset since their text is not in the document. (b) `contains(caret:)` uses `max(changeLength, 1)` so **every recorded row is reachable by the caret** — a rule the user asked for explicitly. **Ruling (user, 2026-08-10):** exact-hit only, no nearest-entry fallback.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0108

**Status:** ✅ **Resolved - Verified (2026-08-11, user-confirmed)** — the writer confirmed the project-level badge. The card shows a count + "Across the whole project — manage in Project Settings…"; list, `Discard…` buttons, purge confirmation and `pendingPurge` removed. ⚠️ **Amends T-0366** ("stale badges + user-confirmed purge" in the card): purge is now Project-Settings-only, one entry point for one irreversible operation.
**Severity:** Low
**Sprint:** **SP-093**

**Description / Resolution:**
`[Apple]` **The history card listed abandoned branches project-wide, so they kept showing with "This scene only" checked.** Reported 2026-08-11 in the SP-093 live verify: *"the history panel shows abandoned branches for the entire manuscript, even when 'This Scene Only' is checked. That is confusing."* **Root cause:** stale branches ARE project-wide, and `StaleBranch` (`HistoryService.hpp:196-202`) carries `branchRootEventID`/`forkNodeID`/`preview`/`tipTimestamp`/`nodeCount` but **no `sceneID`** — so the card had nothing to filter on. Rendering the list under a visible per-scene toggle read as a filter bug. It was also a **duplicate surface**: Project Settings has shipped a full scan/list/purge flow plus the staleness threshold since T-0212/SP-055. **Fix (user-ruled 2026-08-11):** the card keeps a **badge only** — count + "Across the whole project — manage in Project Settings…" — and the list, the `Discard…` buttons, the purge confirmation and `pendingPurge` are removed. Passive discoverability is retained where the writer works; management stays in one place, so one irreversible operation no longer has two entry points. ⚠️ **Amends T-0366**, which specified "stale badges + user-confirmed purge" in the card; purge is now Project-Settings-only. Rejected alternative: adding `sceneID` through the C ABI to filter per-scene — more work to show something Project Settings already shows better, and a branch spanning scenes would filter by its root scene, which is arbitrary.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0109

**Status:** ✅ **Resolved - Verified (2026-08-11, user-confirmed)** — arrow keys + Return select a scene and update **both** the manuscript and the history card. Arrows remain browse-only (highlight without navigating), which the writer asked to keep; `.onKeyPress(.return)` commits the selection, macOS-only.
**Severity:** Low
**Sprint:** **SP-093**

**Description / Resolution:**
`[Apple]` **Return does nothing in the Scene Navigator — a scene reached with the arrow keys cannot be opened from the keyboard.** Reported 2026-08-11: arrow-key selection moves the highlight without updating the manuscript or the history card (**which the user wants KEPT** — *"that is actually ok, since it allows me to rapidly move to another scene"*), but pressing Return on the landed scene also did nothing. **Root cause:** navigation is bound **solely to `onTapGesture`** (`SceneNavigatorView.swift:187`); no key path reaches `navigate(to:)`. The macOS binding is highlight-only by design — `listSelection` returns `$highlightedRowID` when the parent passes no `selection` (`:60`), documented at `:57` as *"on macOS it tracks viewportSceneID for highlight only (navigation is via tap there)"*. So this is the code doing exactly what it says, not a regression. **Fix:** an `.onKeyPress(.return)` handler on the list navigates to the highlighted scene. **macOS-only** (`#if os(macOS)`): on iOS the `selection` binding already drives Master/Detail, so moving the highlight *is* navigation and a Return handler would double-trigger. Arrows remain browse-only.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0110

**Status:** ✅ **Resolved - Verified (2026-08-11, user-confirmed)** — *"it opens!"* The real project now opens cleanly; `applyLoadedEviction` walks `currentNodeID_` back to the nearest surviving ancestor after erasing a purged subtree, matching the guard `pruneInconsistentNodes` already had. Regression test replays the exact log shape and is **RED without the guard**; a probe on a COPY of the project opened cleanly across 3 cycles. ctest 412/412, interop 53/53. ⚠️ **Open follow-up:** the offending record had an out-of-order `seq 1` at the end of a 3,607-record log — replay now survives it, but nothing explains how a purge was written with a stale sequence counter. See [[I-0111]].
**Severity:** High
**Sprint:** **SP-093**

**Description / Resolution:**
`[ScriviCore]` **History fails to OPEN — `unhandled exception: HistoryService: unknown node <id>` — so the project loses undo/redo entirely.** Reported 2026-08-11: `[Scrivi] historyOpen failed: ScriviError(code: 13, message: "unhandled exception: HistoryService: unknown node evt_019ff122-637c-7024-824b-2dbaabdb79e3")`. **Root cause:** replaying a `ctl:purge` whose `branchRootEventID` is an **ancestor of the loaded `currentNodeID`** erases that node along with the subtree (`applyLoadedEviction` → `eraseSubtree`), but nothing walked the pointer back. The next `nodeRef()` threw `std::logic_error` (`HistoryService.cpp:213`), which escaped `scrivi_history_open` as an unhandled exception — a **hard open failure**, not a degraded history. **Forensics on the real log:** 3,607 records; the failing node (seq 3592, kind=barrier) IS defined and never referenced; the log's **last** `ctl` record is a purge with an out-of-order `seq 1` whose branch root is one of the node's 1,242 ancestors. Every open replayed it and failed identically. **Fix:** `applyLoadedEviction` now walks `currentNodeID_` back to the nearest surviving ancestor (root at worst) after erasing subtrees — the guard `pruneInconsistentNodes` has always had (`:1084`), which the eviction path simply never got. The two paths now agree. **Verified against the real data:** a probe run on a COPY of the project opens cleanly across 3 cycles (0 flagged, 0 exceptions). Regression test replays the exact shape and is **RED without the guard**. ctest **412/412**, interop **53/53**.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*
