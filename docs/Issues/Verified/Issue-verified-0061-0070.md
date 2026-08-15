# Verified Issues — I-0061 to I-0070


---

## I-0061: [Linux] Landing Quit button does nothing after the SP-061 shell flip

**Status:** ✅ Resolved - Verified (2026-07-14 — user confirmed the Quit button now quits over VNC, and the quit-path save fired; headless `quit_smoke` guards the regression in CI)
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/main.cpp` (app bootstrap); `qml/Landing.qml` Quit button (`Qt.quit()`)
**Severity:** Medium (shipped UI action is dead; the app can't be quit from the landing — but the window-X still works, and edits aren't lost)
**Sprint:** SP-062 (regression introduced by SP-061)
**Epic:** EP-022 `[Linux]`
**Related:** SP-061 shell flip (T-0234); SP-062 T-0239 (the quit-path auto-save that depended on this quit working)
**Date Identified:** 2026-07-14 (user VNC verification of SP-062)
**Date Resolved:** 2026-07-14

**Description:**
The SP-061 shell flip moved the landing QML from a top-level `QQmlApplicationEngine` into a
`QQuickWidget`. `QQmlApplicationEngine` auto-connects `QQmlEngine::quit()` (the signal QML's `Qt.quit()`
emits) to `QCoreApplication::quit()`; a `QQuickWidget`'s engine does **not**. So the landing's **Quit**
button emitted into the void.

**Expected Behavior:** Clicking **Quit** on the landing exits the app (and, in the Docker/VNC harness,
tears the container down since the app is the foreground process).

**Actual Behavior:** Nothing happened. Console: `Signal QQmlEngine::quit() emitted, but no receivers
connected to handle it.`

**Root Cause:** Missing signal→slot wiring after the integration direction inverted (QML now lives inside
Widgets). No auto-connect happens for a `QQuickWidget` engine.

**Fix:** `main.cpp` now connects the landing `QQuickWidget`'s engine explicitly —
`QQmlEngine::quit → QApplication::quit` (plus the `exit(int)` variant). This also restores the T-0239
quit-path auto-save chain (Quit → `QApplication::quit` → `aboutToQuit` → `ScriviWindow::flushEditor`).

**Verification:** New headless **`quit_smoke`** wires the connection the way `main.cpp` does, emits
`QQmlEngine::quit()`, and asserts the app reaches `exit(0)` (fail-safe timer fails loudly if unconnected).
Green in Docker + wired into CI. **Awaiting user VNC confirmation that the Quit button now quits.**

---

---

## I-0062: [Linux] New chapter heading shows "Chapter" (not "Chapter N") until reload

**Status:** ✅ Resolved - Verified (2026-07-15, user-confirmed over VNC — a newly-created chapter's heading shows
its ordinal "Chapter N" immediately, no reload). **Resolution (SP-066 / T-0256):** the Linux app now **derives**
the chapter heading ordinal from segment order (`SceneDocument::chapterHeadingText` — custom title wins, else
"Chapter N" by position), matching macOS `ManuscriptTextView`. `insertSceneAfter` reflows the new (untitled)
chapter's heading to its derived ordinal on splice, so the live heading is correct without a round-trip. (This
supersedes the earlier "Option B openProject re-fetch" plan — order-based derivation is cleaner and is the
macOS mechanism.) Verified by `scene_create_smoke` (asserts the created chapter derives "Chapter 2" live) +
VNC. Note: this covers **untitled** chapters; renumbering **created** (stored-"Chapter N") chapters on a later
delete is the separate I-0063.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`) →
`SceneDocument::insertSceneAfter` (chapter-heading text); the correct label comes from
`scrivi_open_project`'s `chapterTitle` on reload.
**Severity:** Low (purely a live-vs-reloaded label mismatch; the chapter is created correctly, persists
correctly, and renders its real title on the next open)
**Sprint:** SP-062 (observed during T-0242 verification); **fix scheduled SP-066 / T-0256**
**Epic:** EP-022 `[Linux]`
**Related:** T-0241 (in-editor create chapter)
**Date Identified:** 2026-07-14 (user VNC verification)

**Description:**
When a chapter is created in-editor (`Ctrl+Shift+Return`), its heading in the live document reads the
generic **"Chapter"** rather than **"Chapter 2"** (etc.). After quitting and reopening the project, the
heading renders correctly as "Chapter 2".

**Root Cause:** `scrivi_create_chapter` returns `{chapterID, chapterMetadataPath, firstSceneID,
firstSceneMetadataPath, firstSceneContentPath}` — **no display title**. So `insertSceneAfter` falls back to
the `"Chapter"` label for the freshly-inserted heading. On reopen, `scrivi_open_project` returns the real
`chapterTitle` (the ordinal-derived "Chapter 2"), so it displays correctly.

**Expected Behavior:** The new chapter's heading shows its real ordinal title immediately, matching what a
reload shows.

**Options (not yet chosen):**
- **A.** Have `scrivi_create_chapter` return the derived `chapterTitle` (a `[ScriviCore]` additive change),
  and use it in `insertSceneAfter`. Keeps label derivation in the backend (preferred — no UI-side ordinal
  logic). Additive to `scrivi.h`.
- **B.** Re-fetch just the new chapter's title via a light `open_project` read after create and patch the
  heading. Avoids a core change but adds a round-trip.
- **C.** Accept the fallback until EP-023 (chapter rename/structure editing), where headings become
  first-class and titled explicitly.

**Deferred:** cosmetic and self-correcting on reload; chapter *naming/structure editing* is **EP-023**
scope, not EP-022. Revisit when EP-023 is planned (or promote to a Task if the live label matters sooner).

---

---

## I-0063: [Linux] Deleting/inserting a chapter doesn't renumber later created chapters

**Status:** ✅ Resolved - **Verified (2026-07-16, user-confirmed over VNC** — deleting an earlier chapter
renumbered the later created "Chapter N" chapters to their correct ordinals, and a custom-titled chapter was
left untouched; case 4 of the SP-067 VNC walkthrough). Implemented per **Option A** below. New
`EditorShell::renumberCreatedChapters()` walks chapters in manuscript order and, for each whose STORED title
matches the anchored auto pattern `^Chapter \d+$`, calls `bridge_->renameChapter(chapterMetadataPath, "Chapter
<ordinal>")` to rewrite the sidecar to its new position; custom titles (which don't match the anchored
pattern) are untouched, and untitled chapters already renumber for free via `chapterHeadingText`. Wired into
`deleteChapterByID` (after `removeChapter`) and the T-0261 chapter-split path (after the reload), each followed
by `applyDerivedLabels()` + `rebuildNavigator()` so the live navigator/heading match disk. Also fixed a latent
gap: `EditorShell::load` now copies `chapterMetadataPath` from `open_project`'s scene entries into the segment
map (the renumber + chapter-rename on a freshly-loaded chapter both need it). Verified headless by
`scene_reorder_smoke` Case D's I-0063 assertions (created "Chapter N" chapters carry their correct ordinal on
reopen after a split); **VNC user-verification pending.**
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`deleteChapterByID` and any future chapter-insert path);
`SceneDocument::chapterHeadingText` / `reflowAllChapterHeadings` (the renumber machinery already exists).
**Severity:** Low (display-only ordinal drift; scene/chapter data and order are correct on disk)
**Sprint:** — (backlog; discovered during SP-066)
**Epic:** EP-023 `[Linux]`
**Related:** SP-065 (delete), SP-066 (rename — added `chapterHeadingText` ordinal derivation + the unused
`reflowAllChapterHeadings` helper this fix would call); macOS parity: `ViewportSceneLoader.renumberChapterTitlesFrom`.

**Description:**
When a chapter is created, **ScriviCore stamps a stored title `"Chapter N"`** into the chapter sidecar
(`ChapterCreator.cpp:90`), and `scrivi_delete_chapter` does **not** renumber the remaining chapters. So after
deleting an earlier chapter (or inserting one between), later **created** chapters keep their now-stale stored
ordinal — e.g. delete "Chapter 1" and the old "Chapter 3" still reads "Chapter 3" instead of "Chapter 2".

Untitled chapters (empty stored title) are unaffected — the Linux app derives their heading from order via
`SceneDocument::chapterHeadingText` (added in SP-066), so those renumber for free. The gap is specifically
chapters carrying a stored `"Chapter N"` string.

**Expected Behavior (macOS parity):**
After any chapter structural change (delete/insert/reorder), every subsequent chapter that is **not** custom-
titled shows its correct ordinal — matching macOS `ViewportSceneLoader.renumberChapterTitlesFrom`, which
rewrites the in-memory `chapterTitle` to `"Chapter N"` for affected chapters (and the engine persists the
ordinals to disk).

**Root Cause:**
Two facts combine: (a) ScriviCore stores `"Chapter N"` as a real title at creation (not empty), so the app's
order-based derivation treats it as a custom title and won't recompute it; (b) `ChapterDeleter` doesn't
renumber survivors on disk. So neither layer renumbers created chapters after a delete.

**Options (not yet chosen):**
- **A.** App-side active renumber (macOS parity): after a chapter delete/insert, walk later chapters and, for
  each whose stored title matches the auto pattern `"Chapter <n>"`, `renameChapter` it to its new ordinal
  (rewrites disk). Uses the existing `reflowAllChapterHeadings` for the live document. Fragile edge: a user
  who deliberately typed "Chapter 5" would be renumbered.
- **B.** ScriviCore stores chapters **untitled** (empty title) and derives `"Chapter N"` only for display —
  then the app's order-based derivation handles everything with no disk rewrite. Cleanest, but a `[ScriviCore]`
  behavior change affecting Apple too (needs a cross-platform decision).
- **C.** Accept the drift for created chapters until a dedicated structure-editing pass (reorder, SP-067/068),
  where renumbering is revisited holistically.

**Deferred:** display-only; created-chapter ordinals self-correct if the user renames them, and untitled
chapters already renumber. Revisit alongside reorder (SP-067/068) or when the cross-platform numbering
policy (Option B) is decided.

---

---

## I-0064: [Linux] Ctrl+Shift+Return appends a chapter at the end instead of splitting at the caret

**Status:** ✅ Resolved - **Verified (2026-07-18, VNC, B3)** — split inserts the chapter at the caret (P6 model, see I-0074/SP-071).
caret mid-sentence → head stays, tail becomes the new chapter's first scene, new chapter inserted after
the current one). The **end-of-scene** paths do **not**: end-of-scene *with followers* renumbers but shows
no new chapter/scenes (**I-0069**), and end-of-scene *with no followers* still appends the new chapter at
the manuscript end (**I-0070** — the exact original I-0064 symptom, surviving for that branch). So the fix
landed for the mid-scene case only; the two end-of-scene branches are split out as I-0069/I-0070 and I-0064
stays open until those close. (Prior "Resolved - Not Verified" note below retained for history.)

**Prior status (2026-07-15, superseded by the VNC result above):** ✅ Resolved - Not Verified (SP-067 / T-0261). `onCreateChapterRequested` now SPLITS
the current chapter at the caret instead of appending an empty chapter at the manuscript end. Disk-correct
orchestration (unlike macOS, which splices only in-memory): `create_chapter` (append K + blank K0) →
`reorder_chapter(K, afterChapterID = C)` → for each scene that followed the caret's scene S within C,
`reorder_scene(scene, C, K, afterSceneID)` in order → **mid-scene:** `save_scene` head into S, tail into K0;
**end-of-scene with followers:** followers become K's scenes and the redundant blank K0 is dropped;
**end-of-scene, no followers:** K0 stays as a genuinely-new empty chapter after C → full reload (the split
touches multiple scenes + chapters, so re-reading disk is the safest source-of-truth path) →
`renumberCreatedChapters()` (I-0063) → caret lands at the start of the caret scene (K0/tail for mid-scene,
first follower for end-of-scene). A confirmation dialog fires first when ≥1 subsequent chapter will renumber
(macOS `ManuscriptTextView` parity). Verified headless by `scene_reorder_smoke` Cases D (end-of-scene) + E
(mid-scene head/tail); **VNC user-verification pending.**
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`) →
`SceneDocument::insertSceneAfter`. The Linux create-chapter path has appended-at-end since EP-022/SP-062.
**Severity:** Medium (the ⌘⇧↩ chapter gesture does not do what a writer expects; data is not corrupted, but
the manuscript structure produced is wrong — a stray end-of-manuscript chapter instead of a split).
**Sprint:** discovered SP-066; **fix targeted SP-067**
**Epic:** EP-023 `[Linux]`
**Related:** I-0063 (chapter renumbering); SP-067/068 (drag-reorder — same reorder primitives);
macOS spec: `ManuscriptTextView.swift` (⌘⇧↩ handler, ~L690–795) + `ViewportSceneLoader`
(`splitScene`, `splitChapter`, `insertChapterFirstScene`, `renumberChapterTitlesFrom`).

**Description:**
Positioning the caret inside/at the end of a scene and pressing **Ctrl+Shift+Return** should **split the
current chapter at the caret** (macOS parity), not append a new empty chapter at the end of the manuscript.

Reproduced (user, 2026-07-15): three chapters with multiple scenes; Chapter 2 has three scenes; caret at the
**end of Scene 2 of Chapter 2**. Expected: Chapter 2 splits — Scene 3 (and any following scenes of Chapter 2)
becomes the first scene(s) of a **new Chapter 3** inserted right there, and the old Chapters 3+ renumber down
the line. Actual: a brand-new empty **Chapter 4** with a single blank scene was **appended to the end** of the
document; no split, no scene reassignment, no renumber.

**Expected Behavior (macOS parity — `ManuscriptTextView` ⌘⇧↩):**
- **Caret at end of a scene:** insert the new chapter **immediately after the current scene**; the scenes that
  followed the caret's scene **within the current chapter** are reassigned into the new chapter; subsequent
  chapters renumber. (macOS `insertChapterFirstScene(result, after: segIdx)` + `renumberChapterTitlesFrom`.)
- **Caret mid-scene:** **split the scene** at the caret — the head stays in the current scene/chapter; the tail
  becomes the **first scene of the new chapter**; following scenes of the old chapter move into the new
  chapter; renumber. (macOS `splitScene` + `splitChapter` + `renumberChapterTitlesFrom`.)
- A confirmation is shown when the split will renumber ≥1 subsequent chapter.

**Actual Behavior:**
`onCreateChapterRequested` calls `scrivi_create_chapter` (which is **append-only** — `ChapterCreator.cpp:104`
`ms.chapters.push_back`, always a fresh chapter + blank scene at the manuscript end) and then
`SceneDocument::insertSceneAfter(lastIdx, …)`, so the chapter always lands at the end with no positional
insert, no scene reassignment, and no renumber. The caret position is ignored entirely.

**Root Cause:**
`scrivi_create_chapter` is an append-only primitive with no position argument and no scene-move capability.
macOS builds the split behavior **on top of** it by orchestrating additional calls — `create_chapter` (append),
`save_scene` (head/tail split), `reorder_chapter` (move the new chapter into position), `reorder_scene`
(reassign the following scenes into the new chapter), then in-memory renumber. The Linux path never added that
orchestration; it stops at the raw append.

**Fix direction (SP-067 — no ScriviCore change needed):**
The reorder endpoints already exist (`scrivi_reorder_scene(sceneID, sourceChapterID, targetChapterID,
afterSceneID)`, `scrivi_reorder_chapter(chapterID, afterChapterID)` — `scrivi.h:199/206`). Orchestrate:
1. `create_chapter` (appends new chapter C with blank first scene S).
2. `reorder_chapter(C, afterChapterID = current chapter)` to move C into position.
3. For each scene after the caret's scene in the current chapter: `reorder_scene` into C (in order).
4. Mid-scene case: `save_scene` head into the current scene, tail into S (or the first reassigned scene).
5. Renumber untitled/auto chapters (ties into I-0063) + rebuild the `SceneDocument` splice + navigator.
6. Confirmation dialog when subsequent chapters will renumber.

Do this in SP-067 alongside drag-reorder, which delivers/【exercises the same `reorder_*` orchestration and
the `SceneDocument` move-splice peers.

**Workaround (until fixed):** create chapters at the end and reorder later (once drag-reorder lands), or keep
scenes you want split as separate scenes.

---

> ⚠️ **Entry/table conflict corrected at archive time (2026-08-15).** This entry's body still
> described the issue as open/root-caused; the Issue-active.md table row recorded it Verified with
> dated VNC evidence. Per user ruling the table is authoritative. Body status line updated; the
> original diagnostic narrative below is retained as written.

---

## I-0065: [ScriviCore]/[Apple] A mismatched history diff crashes the macOS app on project open

**Status:** ✅ Resolved - Verified (2026-07-15, user-confirmed on macOS — "The Twisted Remains of Myself" opens
on a rebuilt app instead of crashing). Fix landed in ScriviCore; ctest reproduces the crash and is green
(268/268 with the I-0066 prune tests).
**Platform:** macOS (any Apple platform — the fault is in shared ScriviCore + its C ABI). Not Linux.
**Component:** `ScriviCore/src/history/HistoryService.cpp` (`applyForward`/`applyReverse`,
`rebuildHeadCache`/`finalizeLoad` replay-on-load); `ScriviCore/src/public_api/scrivi_c_api.cpp`
(`scrivi_history_open`/`_undo`/`_redo` boundary). Swift caller: `Scrivi/App/HistoryCapture.swift`.
**Severity:** High (hard crash — `libc++abi: terminating due to uncaught exception`; the project never opens).
**Sprint:** — (out-of-band fix during EP-023; not a Linux sprint)
**Epic:** EP-019 `[Apple]` (Custom Undo/Redo History) — the history subsystem that shipped the replay-on-load path.
**Related:** **I-0066** (the durable root-cause fix — a navigator scene delete left the orphaned diff that
mismatched here; that Issue adds the delete-time barrier + load-time prune). EP-019 SP-054 (HistoryStore
load/replay); `HistoryCapture.swift` best-effort `open()` try/catch (which could not catch this because the
exception `std::terminate`d *inside* the C ABI, before returning).
**Date Identified:** 2026-07-15 (user — macOS app crash opening a real project)
**Date Resolved:** 2026-07-15

**Description:**
Opening the project **"The Twisted Remains of Myself"** on the macOS app crashed **before the project opened**,
with:
```
libc++abi: terminating due to uncaught exception of type std::length_error: basic_string
```
The crash fired during history **replay-on-load**: `scrivi_history_open` → `HistoryStore` replays the persisted
JSONL log → `HistoryService::finalizeLoad()` → `rebuildHeadCache()`, which walks the root→current path and
applies each node's diff forward with `applyForward`. When a persisted node's diff no longer matches the scene's
baseline text — the shape produced when **a diff is replayed against a deleted or externally-changed scene**
(the user's hypothesis: "applying a historical diff to the wrong Scene") — `applyForward` computed
`reserve(oldText.size() - removed.size() + inserted.size())`. With `removed.size() > oldText.size()` that
subtraction **underflowed** (both are `std::size_t`, unsigned) to a near-`SIZE_MAX` value, and
`std::string::reserve` threw `std::length_error`. A sibling `append(oldText, offset+removed, npos)` could throw
`std::out_of_range` on the same mismatch.

Because that exception was thrown **inside** an unguarded C ABI function (`scrivi_history_open`), it crossed the
`extern "C"` boundary — which is a hard `std::terminate` (a C++ exception may never unwind through a C frame).
The Swift side (`HistoryCapture.open()`) already wraps `engine.historyOpen` in a best-effort try/catch ("a
failure here must never block editing"), but it never got the chance: the process was already dead.

**Expected Behavior:**
A corrupt, stale, or mismatched history log degrades **history** to best-effort (no undo, or a truncated tree)
and the **project still opens and is editable**. No crash, ever, from replaying a bad diff.

**Actual Behavior:**
The whole app terminated at project-open before the editor appeared.

**Root Cause:**
Two independent latent faults, both required for the crash:
1. **`applyForward`/`applyReverse` assumed the diff matched `oldText`.** The `reserve()` size arithmetic
   underflowed on `removed.size() > oldText.size()`, throwing `std::length_error`; the tail `append` could throw
   `std::out_of_range`. A mismatched/stale/corrupt persisted diff triggers both.
2. **The C ABI history entrypoints were unguarded.** Any C++ exception escaping `scrivi_history_open` (or
   `_undo`/`_redo`) unwound into a C frame → `std::terminate`. The boundary contract (`scrivi.h` is a pure C ABI;
   exceptions must never cross it) was violated by omission for these three functions.

**Fix (2026-07-15):**
- **Clamp-safe apply (defense in depth #1)** — `applyForward`/`applyReverse` now clamp the offset and the
  removed/inserted span into the actual text:
  `off = min(offsetUtf8, text.size())`, `cut = min(off + span.size(), text.size())`, and build the result from
  `[0,off) + patch + [cut, end)`. No unsigned underflow, no out-of-range; a mismatched diff yields a best-effort
  string instead of throwing. (`HistoryService.cpp` ~L96–119.)
- **Guarded C ABI (defense in depth #2)** — a `guarded(fn)` helper wraps the history entrypoints in
  `try { … } catch (const std::exception& e) { → errorEnvelope(internalError, "unhandled exception: …") } catch (...) { … }`,
  so **no** exception can ever cross the boundary again; a failure returns a `{"error":…}` envelope Swift already
  handles. Applied to `scrivi_history_open`, `scrivi_history_undo`, `scrivi_history_redo`.
  (`scrivi_c_api.cpp` ~L175, L1498/L1645/L1680.)

Both layers are deliberate: the clamp fixes *this* bug at the source; the guard ensures *any* future history-path
exception becomes an error envelope (best-effort history) rather than a process kill — matching what
`HistoryCapture.open()` was already written to expect.

**Verification (2026-07-15 — ctest green; user run pending):**
- New regression **`replay-on-load survives a diff whose removed span exceeds the floor`**
  (`ScriviCore/tests/unit/HistoryServiceTests.cpp`, tag `[History][load][I-0065]`) rehydrates a service via
  `addLoadedFloor`/`addLoadedNode`/`setPointers` with a persisted node whose `diff.removed` (20 bytes) exceeds
  the scene's floor ("hi"), then asserts `finalizeLoad()` **does not throw**, yields a well-formed best-effort
  head, and that `undo()`/`redo()` on the mismatched node also don't throw. **Before the fix this reproduced the
  `std::length_error`.**
- `ctest --test-dir build --output-on-failure` → **268/268 pass** (the I-0065 replay test + 3 I-0066 prune tests).
- **USER-CONFIRMED (2026-07-15):** on a rebuilt macOS app, **"The Twisted Remains of Myself"** opens (history for
  the affected scene truncated/best-effort) instead of crashing.

**Files Affected:**
- `ScriviCore/src/history/HistoryService.cpp` — clamp-safe `applyForward`/`applyReverse`
- `ScriviCore/src/public_api/scrivi_c_api.cpp` — `guarded()` helper + wrapped history entrypoints
- `ScriviCore/tests/unit/HistoryServiceTests.cpp` — I-0065 regression test

---

---

## I-0066: [ScriviCore]/[Apple] A deleted scene/chapter leaves orphaned history in the log

**Status:** ✅ Resolved - Verified (2026-07-15, user-confirmed on macOS — the previously-crashing project opens;
its history log self-heals after one open; navigator scene/chapter deletes record a `sceneDelete` barrier).
Load-time prune + delete-time barrier landed; ctest green (268/268).
**Platform:** macOS/Apple (shared ScriviCore history + the Apple navigator delete path). Not Linux.
**Component:** `Scrivi/Views/SceneNavigatorView.swift` (`performDeleteScene`/`performDeleteChapter` — the missing
barrier); `ScriviCore/src/history/HistoryService.cpp` (`pruneInconsistentNodes`, `diffMatches`);
`ScriviCore/src/history/HistoryStore.cpp` (`openOrCreate` prune + `ctl:purge` persist).
**Severity:** Medium (root cause of the I-0065 crash; on its own it produces silently-degraded/orphaned undo
history for deleted scenes, and — before I-0065's clamp — the crash).
**Sprint:** — (out-of-band fix during EP-023)
**Epic:** EP-019 `[Apple]` (Custom Undo/Redo History)
**Related:** **I-0065** (the crash this feeds — clamp + C ABI guard was the immediate stop; this is the durable
fix). macOS already records a `sceneMerge`/`sceneSplit` barrier on the *editor* delete/split path
(`ManuscriptTextView.swift`); the **navigator** delete path never did.
**Date Identified:** 2026-07-15 (diagnosing I-0065 — the mismatched diff traced back to a navigator scene delete)
**Date Resolved:** 2026-07-15

**Description:**
When a scene or chapter is deleted **from the Scene Navigator** (`performDeleteScene`/`performDeleteChapter`),
the app called `deleteScene`/`deleteChapter` but recorded **no history barrier** and did **nothing** to that
scene's history in `history/log-000001.jsonl`. The scene's `rec:"floor"` and `rec:"event"` records stayed in the
log. On the next open, replay faithfully reconstructed the deleted scene's history; because the scene (and its
baseline) were gone or changed, a diff no longer matched — an **orphaned/inconsistent diff**. That is exactly the
mismatched node that crashed the app in **I-0065**. (The editor-driven delete/merge/split paths in
`ManuscriptTextView` *do* record barriers; only the navigator delete path was missing one.)

**Expected Behavior:**
Deleting a scene/chapter records a structural **`sceneDelete` barrier** (undo can't cross into the removed
content), and any history the deletion orphaned is cleaned up so a later open never replays a diff against a
missing scene — no crash, no silent head corruption, no perpetual re-degrade.

**Actual Behavior:**
Orphaned floor+event records accumulated in the log with no barrier; on reload they mismatched (crash pre-I-0065;
silently-clamped best-effort head post-I-0065, re-degraded on every open).

**Root Cause:**
Two gaps: (a) the navigator delete path recorded no barrier (unlike the editor paths); (b) `scrivi_delete_scene`
/`scrivi_delete_chapter` have no history-side effect, and the load path had no integrity check — a persisted diff
was trusted to match its scene.

**Fix (2026-07-15):**
1. **Delete-time barrier (Apple)** — `performDeleteScene`/`performDeleteChapter` now call
   `session.historyCapture?.recordBarrier(kind: "sceneDelete", …)` **before** the delete, matching the editor
   paths. Undo stops at the barrier instead of walking into the removed scene.
2. **Load-time self-heal (ScriviCore)** — `HistoryService::pruneInconsistentNodes()` runs in
   `HistoryStore::openOrCreate` after `finalizeLoad()`/eviction. It DFS-walks the tree carrying each scene's
   replayed text and, via the new `diffMatches(oldText, diff)` predicate, drops any node whose diff can't have
   come from its scene (offset past end, or removed bytes absent) **together with its subtree**; the current
   pointer is walked back to a surviving ancestor. It returns the detached subtree roots, and the store persists
   a **`ctl:purge`** per root — so the log is clean on the *next* open (replay honors `ctl:purge`). The bad
   history **self-heals** on first open instead of degrading forever.

Together with I-0065's clamp (never crash) and C ABI guard (never cross the boundary), history is now: never
crashes, never silently mangles a head, and repairs an already-corrupt log on load.

**Verification (2026-07-15 — ctest green; user run pending):**
- New tests (`HistoryServiceTests.cpp`, tag `[History][load][I-0066]`): (a) an inconsistent node is dropped while
  a consistent sibling scene's history stays intact and undoable; (b) the current pointer is moved out of a
  dropped subtree (whole bad chain → one purge root); (c) a fully-consistent tree prunes nothing (no false
  positives). **15 assertions, 3 cases, all pass.**
- Full suite: `./ScriviCoreTests` → **268 cases / 1568 assertions pass**; `ctest` → **268/268**.
- **USER-CONFIRMED (2026-07-15):** on a rebuilt macOS app the previously-crashing project opens; after that open
  its history log no longer carries the orphaned nodes (self-healed via the persisted `ctl:purge`); deleting a
  scene/chapter from the navigator now records a `sceneDelete` barrier (undo stops there).

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift` — `sceneDelete` barrier before navigator scene/chapter delete
- `ScriviCore/src/history/HistoryService.{hpp,cpp}` — `pruneInconsistentNodes()`, `diffMatches()`
- `ScriviCore/src/history/HistoryStore.cpp` — prune-on-load + `ctl:purge` persistence
- `ScriviCore/tests/unit/HistoryServiceTests.cpp` — I-0066 prune tests

---

---

## I-0067: [Linux] Scene drag-reorder does not persist across quit→relaunch (AC4)

**Status:** ✅ Resolved - **Verified (2026-07-16, user "It's clean" over VNC on a fresh project)**
hypotheses (wrong/empty dragged id; success-but-no-op reorder) were BOTH falsified by runtime evidence. The
`startDrag` latch (kept — it's still more correct) had no effect because id-resolution was never the failing
step. **Confirmed cause (unified with I-0068 — one event, both symptoms, as the user predicted):** on a drag,
`NavigatorTree` uses `setDragDropMode(DragDrop)` + `setDefaultDropAction(Qt::MoveAction)` and `dropEvent`
calls `event->acceptProposedAction()` **before** the backend move is known to succeed. `onSceneDropped` then
calls `scrivi_reorder_scene`, which **failed** for this project (see below), so the handler correctly did
NOTHING (early-returned on the empty envelope) — **hence no persist**. But because the drop was accepted as a
`MoveAction`, **Qt's own `QAbstractItemView::startDrag` removed the source row from the model** after
`dropEvent` returned — **hence the vanish (I-0068)**. The removal (Qt) and the persist (ScriviCore) are
decoupled and the former isn't gated on the latter.

**Why `reorder_scene` failed here:** instrumentation captured the raw envelope —
`{"ok":false,"error":{"code":1,"message":"sourceChapterID not found: chapter_019e9cdd-…"}}`. The test project
**"The Twisted Remains of Myself"** has a **corrupt `manuscript.meta.json`** whose `chapters[]` list disagrees
with the on-disk chapter sidecars (e.g. it lists `chapter-004` as `019e942e` — a phantom id in no sidecar —
while `chapter-004/chapter.meta.json` actually holds `019e9cdd`; also lists `chapter-015` twice and references
nonexistent `chapter-011`/`014`). So the scene's real chapter (`019e9cdd`) isn't in the manuscript index →
ScriviCore can't find the source chapter → rejects. **That data corruption is filed separately as I-0072**
(likely fallout of today's I-0064/I-0069/I-0070 split path — same project that also crashed macOS in I-0065).

**Fix direction (app layer — this Issue):** the navigator must change ONLY via our own `rebuildNavigator`, never
via Qt's drag auto-remove.

**FIX IMPLEMENTED (2026-07-16, SP-067 T-0260):** `NavigatorTree` now runs the drag as **`Qt::CopyAction`, never
`MoveAction`** — `startDrag` forces `QTreeView::startDrag(Qt::CopyAction)`, the ctor sets
`setDefaultDropAction(Qt::CopyAction)`, and `dragMoveEvent`/`dropEvent` `setDropAction(Qt::CopyAction)` before
accept. With a Copy drop the base class never removes the source row, so the navigator changes *only* via
`sceneDropRequested → EditorShell::onSceneDropped → rebuildNavigator`. A backend-rejected/failed reorder now
leaves the tree untouched (no vanish); a successful one reflects solely through our rebuild. TEMP drag
instrumentation removed. Container build + 275 ctest smokes green; **needs VNC re-verification on a FRESH
(non-corrupt) project** — the old test project's I-0072 index corruption would still make `reorder_scene`
legitimately fail (that's EP-027's job).

**Prior finding (2026-07-16):** VNC cases 2a/2b: dragging a scene appeared to move it live (the row left its
position), but after Quit→relaunch the chapter returned to its original order. Now explained: the disk move
never happened; only Qt's row-removal did.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onSceneDropped`, ~L528–582) →
`ScriviBridge::reorderScene`; possibly `NavigatorTree::draggedSceneID`/`resolveDrop`
(`NavigatorTree.cpp`).
**Severity:** High (AC4 — the headline SP-067 deliverable — does not work end-to-end; the move is not durable)
**Sprint:** SP-067 (regression against T-0260's exit criterion "the new order persists across quit→reopen")
**Epic:** EP-023 `[Linux]`
**Related:** I-0068 (the sibling live symptom — scene vanishes); T-0260/T-0263. **Note:** the headless
`scene_reorder_smoke` **Case C passes** — it calls `ScriviBridge::reorderScene` directly and confirms the
reopened on-disk order changed. So the bridge + ScriviCore endpoint persist correctly *when called with the
right arguments*; the fault is in the **live app path** that computes and issues that call, which the smoke
never exercises.

**Description:**
A scene dragged in the navigator does not keep its new position after the project is closed and reopened —
the manuscript reverts to the pre-drag order on disk.

**Expected Behavior (AC4):** the drop calls `scrivi_reorder_scene`, the continuous viewport + map re-splice
to the new order, and **the new order persists across quit→reopen**.

**Actual Behavior:** live view changes momentarily (see I-0068), but disk is unchanged; quit→relaunch shows
the original order.

**Root Cause (hypothesis — needs a live-path debug run to confirm):**
The persistence primitive is proven good by `scene_reorder_smoke` Case C, so the break is in `onSceneDropped`
issuing the bridge call. The most probable causes, in order:
1. **`reorderScene` returns empty and the early `return` at `EditorShell.cpp:557` fires**, so `moveScene`
   never runs on disk — but then the row wouldn't "disappear" live, so this alone doesn't fit I-0068. More
   likely the call is made with **wrong/empty arguments** (`draggedSceneID`, `sourceChapterID`,
   `targetChapterID`, or `afterSceneID`), so ScriviCore rejects or no-ops it while the *in-memory* `moveScene`
   still splices (producing the visual change that reverts on reload).
2. **`draggedSceneID()` resolves from `currentIndex()`/selection** (`NavigatorTree.cpp:22–28`), not from the
   drag's actual source row. During a Qt drag the current index can differ from the grabbed row, so the wrong
   (or empty) sceneID is sent to `reorderScene`. This also explains I-0068 (the *visually* removed row is the
   selected one, not necessarily the one reordered on disk).
3. **`sourceChapterID` mismatch:** `onSceneDropped` reads `moved.chapterID` from the segment at `fromIdx`; if
   `fromIdx` was resolved from a stale/again-selected scene, the source chapter passed to `reorder_scene` is
   wrong and ScriviCore's move is a no-op.

**5-Whys:**
1. *Why did the reorder not persist?* — Because `scrivi_reorder_scene` did not change the on-disk index for
   the intended scene.
2. *Why didn't it change disk?* — Because the live `onSceneDropped` call either returned empty (early-return
   before disk write) or issued the call with arguments that don't identify the intended move.
3. *Why were the arguments wrong / the call empty?* — Because `draggedSceneID`/`sourceChapterID` are derived
   from the tree's **current selection** at drop time rather than from the drag's committed source row, and a
   Qt drag can leave the selection pointing elsewhere.
4. *Why was selection used instead of the drag source?* — Because `NavigatorTree` never captured the source
   sceneID at `startDrag`/drag-start; it re-reads `currentIndex()` in both `dragMoveEvent` and `dropEvent`.
5. *Why wasn't this caught before VNC?* — Because `scene_reorder_smoke` validates the **bridge + SceneDocument
   primitives directly** (Case C reorders and reopens successfully) but **never drives `onSceneDropped` or a
   real `QDropEvent`**, so the argument-derivation path had zero automated coverage. **Root cause: the live
   drag→drop→persist path has no test; only its primitives do.**

**Fix direction (proposed, not yet implemented):**
- Capture the dragged sceneID at **drag start** (override `startDrag`, stash the source row's `kSceneIDRole`
  and its parent chapterID) instead of reading `currentIndex()` at drop time; pass both into
  `sceneDropRequested`.
- In `onSceneDropped`, when `reorderScene` returns empty, do **not** run the in-memory `moveScene` (keep disk
  and view in lock-step — no optimistic splice that can't be persisted).
- Add a **live-path smoke** that constructs a real `QDropEvent` (or calls `onSceneDropped` with resolved
  args) and asserts the reopened on-disk order — closing the coverage gap named in why #5.

---

> ⚠️ **Entry/table conflict corrected at archive time (2026-08-15).** This entry's body still
> described the issue as open/root-caused; the Issue-active.md table row recorded it Verified with
> dated VNC evidence. Per user ruling the table is authoritative. Body status line updated; the
> original diagnostic narrative below is retained as written.

---

## I-0068: [Linux] Scene disappears from the navigator on drop (Qt MoveAction auto-removes the source row)

**Status:** ✅ Resolved - **Verified (2026-07-16, user "It's clean" over VNC on a fresh project)**
I-0067** (see it for the full evidence). The vanish is **Qt removing the dragged source row itself**: with
`DragDrop` mode + `defaultDropAction = MoveAction`, `dropEvent`'s `acceptProposedAction()` tells the drag loop
the move succeeded, so `QAbstractItemView::startDrag` calls its remove-source-rows path — deleting the row
from the `QStandardItemModel` — **regardless of whether `onSceneDropped` did anything.** In this project
`onSceneDropped` early-returned (backend rejected the move, I-0072), so the row was removed by Qt with no
compensating rebuild → it disappears until reload. The instrumentation proved our handler never touched the
tree (envelope empty → early return before `moveScene`).

**Fix direction:** shared with I-0067 — stop accepting a `MoveAction` in `dropEvent` so Qt never auto-removes;
let `rebuildNavigator` be the only thing that changes the model. The "outline box, no insertion line" in the
original 2a was a separate cosmetic (drop indicator gating), re-check after the fix.

**Prior finding (2026-07-16):** VNC 2a/2b: on drop the dragged scene disappeared from the navigator entirely
rather than reappearing at the drop position.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onSceneDropped` → `SceneDocument::moveScene` +
`rebuildNavigator`, ~L560–580); `NavigatorTree::dragMoveEvent` (indicator draw, `NavigatorTree.cpp:89–109`).
**Severity:** High (the reorder UI visibly loses the scene from the tree — alarming; the scene is not gone on
disk, but the navigator no longer shows it until reload)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0067 (same drop path; the persistence failure). Likely the same root cause (wrong
dragged/source id).

**Description:**
Dropping a dragged scene removes its row from the navigator instead of re-placing it at the new position. The
tree ends up missing the scene until the project is reloaded.

**Expected Behavior:** after the drop the scene appears at its new position in the navigator (and the
viewport reflows), caret preserved.

**Actual Behavior:** the row disappears; no re-inserted row is shown at the target.

**Root Cause (hypothesis — confirm with a live run):**
`onSceneDropped` runs the in-memory `SceneDocument::moveScene` (which lifts the scene out via `removeScene`
then re-inserts) and then `rebuildNavigator()` (which projects the tree from `segments()`). A disappeared row
means **either** (a) `moveScene` removed the scene but re-inserted it into a target the navigator then
doesn't render (e.g. wrong `targetChapterID`, so it's spliced under a non-existent/hidden chapter node),
**or** (b) `moveScene` returned `newIdx < 0` and the fallback full `load()` at `EditorShell.cpp:570` ran with
a bad title/argument and dropped the row, **or** (c) the dragged id and the reordered id diverged (I-0067
cause #2) so `removeScene` took out one row while nothing was re-inserted for it. The "outline box, no
insertion line" in 2a further suggests `dragMoveEvent` `ignore()`d the position (no accepted drop target), yet
`dropEvent` still emitted — i.e. the drop fired at a position the gate had rejected.

**5-Whys:**
1. *Why did the scene disappear?* — `rebuildNavigator` projected a segment list in which the dragged scene's
   row was removed but not re-inserted at a rendered position.
2. *Why was it removed-but-not-re-inserted?* — `moveScene` re-inserted it under a `targetChapterID` the
   navigator didn't render, or the dragged id ≠ reordered id so the removed row had no matching re-insert.
3. *Why did the ids diverge / the target come out wrong?* — Same origin as I-0067: `draggedSceneID`/target
   resolved from `currentIndex()` + drop-indicator at drop time rather than from the committed drag source.
4. *Why did the drop fire at a rejected position (2a)?* — `dropEvent` re-resolves `resolveDrop` independently
   of `dragMoveEvent`; if the indicator position at drop differs from the last accepted move position, the
   drop can proceed on a target the move gate would have vetoed (no shared "is this drop currently legal?"
   state).
5. *Why wasn't this caught before VNC?* — No test drives the real `QDropEvent`/navigator rebuild; the smoke
   asserts `moveScene` on a `SceneDocument` in isolation (Cases A/B), which always re-inserts correctly
   because it's handed correct arguments. **Root cause: the navigator-rebuild-after-drop path is untested with
   real drop-resolved arguments.**

**Fix direction (proposed):** shares I-0067's fix (capture drag source at drag-start; don't splice in-memory
unless the disk reorder succeeded). Additionally, have `dropEvent` reuse the **last `dragMoveEvent`-accepted**
resolution (store it) instead of re-resolving, so a drop can never land where the move gate said no.

---

> ⚠️ **Entry/table conflict corrected at archive time (2026-08-15).** This entry's body still
> described the issue as open/root-caused; the Issue-active.md table row recorded it Verified with
> dated VNC evidence. Per user ruling the table is authoritative. Body status line updated; the
> original diagnostic narrative below is retained as written.

---

## I-0069: [Linux] Ctrl+Shift+Return at end-of-scene WITH followers renumbers but creates no visible chapter/scenes

**Status:** ✅ Resolved - **Verified (2026-07-18, VNC, B4)** — P6 model (see I-0074/SP-071).
fired and, on confirm, the later chapters **renumbered** — but **no new chapter or scenes appeared**. The
followers were not visibly moved into a new chapter.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`, end-of-scene-with-followers
branch, ~L1126–1136 → reload L1146 → `renumberCreatedChapters` L1151).
**Severity:** Medium (the ⌘⇧↩ split gesture appears to do nothing structural for this case, though it renumbers)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0064 (parent), I-0070 (the no-followers branch), I-0067/I-0068 (share the `reorder_scene`
plumbing — a shared reorder defect could explain all four). macOS parity: `ManuscriptTextView` ⌘⇧↩.

**Description:**
With the caret at the **end of a scene that has following scenes in its chapter**, Ctrl+Shift+Return should
insert a new chapter right after the current one and **move the followers into it**. Instead the followers
stayed put and no new chapter/scenes showed; only the (unrelated) renumber visibly happened.

**Expected Behavior:** a new chapter K appears immediately after the current chapter C, C's post-caret scenes
become K's scenes, subsequent chapters renumber, the blank K0 is dropped.

**Actual Behavior:** chapters renumbered but no new chapter/scenes are visible; followers not reassigned in
the UI.

**Root Cause (hypothesis — confirm with a live run):**
The branch at `EditorShell.cpp:1126–1136` does: for each follower `reorderScene(f, chapterC, newChapterID,
afterID)`, then `deleteScene(firstSceneID)` (drop blank K0), then a full `load()` at L1146. If **`reorderScene`
here fails/no-ops** (the same suspected argument defect as I-0067 — e.g. a wrong `chapterC`/`newChapterID`, or
`afterID` handling), the followers never join K; then `deleteScene(K0)` removes K's *only* scene, leaving K
**empty**, and an empty chapter has no segments so the post-`load()` navigator **doesn't render it at all** —
hence "no new chapter/scenes," while `renumberCreatedChapters()` still walked the chapters and renamed
sidecars (the visible renumber). The mid-scene branch (3b) passes because it **saves the tail into K0** (K is
never empty) and doesn't depend on follower reassignment.

**5-Whys:**
1. *Why did no new chapter/scenes appear?* — The new chapter K ended up **empty** (no scenes), and the
   navigator doesn't render an empty chapter.
2. *Why was K empty?* — The followers were not reassigned into K, and its only born scene (blank K0) was then
   deleted.
3. *Why were the followers not reassigned?* — The per-follower `reorderScene` call no-op'd/failed — the same
   `reorder_scene` argument/plumbing fault suspected in I-0067.
4. *Why did K0 get deleted anyway?* — The code unconditionally deletes K0 in the with-followers branch,
   assuming the followers already populated K; it doesn't verify K is non-empty before dropping K0.
5. *Why wasn't this caught before VNC?* — `scene_reorder_smoke` Case D replays these bridge steps **directly
   with correct ids** and passes, so it proves the *orchestration recipe* but not the *app's execution* of it
   (caret→segment resolution, follower collection, argument passing). **Root cause: the smoke tests the recipe,
   not `onCreateChapterRequested`; and the with-followers branch deletes K0 without a "K is non-empty" guard.**

**Fix direction (proposed):** fix the shared `reorder_scene` argument path (I-0067); guard the K0 delete on
"K now has ≥1 scene" (never leave/observe an empty chapter — consistent with the I-0071 no-empty-chapter
policy); add a live-path test that drives `onCreateChapterRequested` end-of-scene-with-followers and asserts
the reopened structure.

---

> ⚠️ **Entry/table conflict corrected at archive time (2026-08-15).** This entry's body still
> described the issue as open/root-caused; the Issue-active.md table row recorded it Verified with
> dated VNC evidence. Per user ruling the table is authoritative. Body status line updated; the
> original diagnostic narrative below is retained as written.

---

## I-0070: [Linux] Ctrl+Shift+Return at end-of-scene with NO followers appends the new chapter at the manuscript end

**Status:** ✅ Resolved - **Verified (2026-07-18, VNC, B4)** — P6 model (see I-0074/SP-071).
chapter's **last** scene (no followers) **added a new chapter at the very end of the document** instead of
inserting it right after the current chapter. This is the **original I-0064 symptom** surviving for this
branch.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`, no-followers path — falls
through to the "else" at ~L1137 after `reorderChapter` at L1102).
**Severity:** Medium (a stray end-of-manuscript chapter — the exact wrong structure I-0064 was meant to fix)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0064 (parent — this is the residual), I-0069 (sibling end-of-scene branch). macOS parity:
`ManuscriptTextView` ⌘⇧↩ + `insertChapterFirstScene`.

**Description:**
Caret at the **end of the last (or only) scene of a chapter**; Ctrl+Shift+Return creates the new empty chapter
but it lands at the **end of the manuscript**, not immediately after the current chapter.

**Expected Behavior:** a new empty chapter is inserted **directly after the current chapter C**.

**Actual Behavior:** the new chapter is appended at the manuscript end (no repositioning).

**Root Cause (hypothesis — confirm with a live run):**
Step 2, `reorderChapter(projectPath_, newChapterID, chapterC)` at `EditorShell.cpp:1102`, is supposed to move
the freshly-appended K to sit right after C. For the no-followers branch nothing else moves K, so if
`reorderChapter` **no-ops or fails**, K stays where `create_chapter` put it — appended at the end. Candidates:
(a) `scrivi_reorder_chapter`'s `afterChapterID = chapterC` isn't honored when C is the last chapter (moving
after the last chapter = staying last is *correct* only if C really is last — but here C is **not** meant to be
last; check whether `chapterC` is the right id and whether reorder_chapter treats "after the last chapter" as a
no-op); (b) `chapterC` is resolved wrong (stale caret→segment). Note the smoke's Case D calls
`reorderChapter(kCh, ch1)` where ch1 is **not** last and it passes — so the failing condition is specifically
**C being the current last chapter** (the natural no-followers case), which the smoke never exercises.

**5-Whys:**
1. *Why did the new chapter land at the end?* — K was never repositioned after C.
2. *Why wasn't it repositioned?* — `reorderChapter(K, afterChapterID=C)` didn't move K.
3. *Why didn't reorderChapter move it?* — Suspected: when C is the manuscript's **last** chapter, "insert K
   after C" collapses to "K stays last" — but K was appended **after** the also-appended nothing… i.e. the
   after-anchor/ordering for the last-chapter case isn't handled, or `chapterC` was resolved incorrectly.
4. *Why is the last-chapter case unhandled?* — The orchestration assumes C has chapters after it (the general
   split case); the no-followers-at-end case (C is last, K should still be a distinct new last chapter placed
   immediately after C) wasn't distinguished.
5. *Why wasn't it caught before VNC?* — `scene_reorder_smoke` Case D positions K after a **non-last** chapter
   (ch1, with ch2 after it), so `reorder_chapter`'s last-chapter behavior is never tested; and no test drives
   `onCreateChapterRequested` with the caret at the end of the last scene. **Root cause: the end-of-last-chapter
   reposition path is untested, and the app path isn't covered.**

**Fix direction (proposed):** verify `chapterC` resolution; confirm/handle `reorder_chapter`'s
"after the current last chapter" semantics so K is placed as the new chapter immediately following C; add a
live-path + smoke case with C as the last chapter.

---

> ⚠️ **Entry/table conflict corrected at archive time (2026-08-15).** This entry's body still
> described the issue as open/root-caused; the Issue-active.md table row recorded it Verified with
> dated VNC evidence. Per user ruling the table is authoritative. Body status line updated; the
> original diagnostic narrative below is retained as written.
