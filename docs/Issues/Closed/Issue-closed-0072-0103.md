# Closed Issues (Not Verified) — I-0072, I-0073, I-0085, I-0103

These Issues were closed without user verification: two user-directed closures, one
not-a-bug, one superseded. Archived 2026-08-15 during the docs cleanup.

---

## I-0072: [ScriviCore]/[Linux] manuscript.meta.json chapters[] diverges from the chapter sidecars on disk

**Status:** ⚪ **CLOSED — fixed by EP-027 (2026-08-09, user-directed).** Verified against the code and the
test suite at close, not merely inferred from the EP-027 doc trail:

- **Root cause eliminated.** `ChapterCreator` no longer derives the folder slug from the chapter count. It now
  computes an order key between its neighbours — `util::keyBetween(lo, hi)` → `chapterSlug = "chapter-" +
  orderKey` (`ChapterCreator.cpp:77-84`) — so a delete can no longer make a new chapter land on an existing
  directory number. The one surviving `existing.size() + 1` (L134) feeds the human-readable **title**
  (`"Chapter N"`), never a path, so it cannot clobber a sidecar.
- **Damaged projects self-heal.** `keyForRef` resolves an index ref by `chapterID` and falls back to the ref's
  path folder, skipping genuine phantoms; `rebuildIndexIfInconsistent` drops entries whose sidecar disagrees
  (`ChapterIndex.cpp:243-266`). This is the repair path the original entry asked for.
- **Regressions green (2026-08-09).** `ChapterSlugCollisionTests.cpp` covers both halves — *"ChapterCreator
  uses collision-free order-key slugs after a delete (I-0072)"* and *"rebuildIndexIfInconsistent self-heals an
  I-0072-corrupt index"* — plus the *"split orchestration — createChapter after a delete does not collide
  (I-0069/I-0072)"* case. All pass; **full suite 388/388**.

> **The trade study question this entry was blocked on is settled.** The original text says the durable fix
> "should follow that study's decision… awaiting the user's decision." That decision was taken in EP-027 (§8,
> Human-approved 2026-07-17): order-key folder naming + sidecar-owns-identity — i.e. the study's A2/B1
> direction, which is exactly what shipped. Nothing further is pending here.

> ⚠️ **The damaged project itself is not repaired by this close.** "The Twisted Remains of Myself" was corrupt
> on disk before the fix existed; opening it in a current build runs the self-heal, but this entry does not
> assert that specific project has been re-validated. Per memory, the damaged copy on the Desktop should be
> worked from the **"copy" backup**.

**Original entry (2026-07-16) follows.** Discovered while instrumenting the I-0067/I-0068 drag failure. The reorder
rejection `"sourceChapterID not found: chapter_019e9cdd-…"` traced to a **corrupt manuscript index**, not the
drag code.
**Platform:** Data/`[ScriviCore]` (whatever writes `manuscript.meta.json`); surfaced on `[Linux]`.
**Component:** `manuscript/manuscript.meta.json` writer — the chapter-create/reorder/split orchestration that
edits `structure.chapters[]`. On Linux this is `EditorShell::onCreateChapterRequested`
(`create_chapter`→`reorder_chapter`→…) and `ScriviCore` `ChapterCreator`/`ChapterReorderer`. Reader that trips
on it: `SceneReorderer::reorder` (`ScriviCore/src/manuscript/SceneReorderer.cpp:40-51`).
**Severity:** High (a manuscript whose index disagrees with its sidecars makes `reorder_scene` — and any op
keyed on the chapter list — fail or misbehave; latent structural corruption)
**Sprint:** SP-067 (found here; the writer is likely today's split path)
**Epic:** EP-023 `[Linux]`
**Related:** I-0067/I-0068 (the drag failure this explains); I-0064/I-0069/I-0070 (chapter-split — the most
likely corruptor); **I-0065** (the SAME project, "The Twisted Remains of Myself," crashed macOS on open — this
project has been a repeated corruption vector; its state may predate today and mix several defects).

**Description:**
In the project **"The Twisted Remains of Myself"**, `manuscript/manuscript.meta.json`'s `structure.chapters[]`
does not match the chapter sidecars on disk:
- **Wrong id for a real chapter:** the index lists `chapter-004` as `chapter_019e942e-…`, but
  `manuscript/chapter-004/chapter.meta.json` actually contains `chapter_019e9cdd-…`. The id `019e942e` appears
  in **no** sidecar (phantom).
- **Duplicate path:** `manuscript/chapter-015/chapter.meta.json` is listed **twice**, under two different
  chapter ids (`019ed110-…` and `019f6b84-…`).
- **Missing/nonexistent dirs referenced or skipped:** dirs `chapter-011` and `chapter-014` don't exist; the
  index ordering (…, chapter-007, chapter-017, chapter-008, …) is scrambled.

Because the scene being dragged reports `chapterID = 019e9cdd` (from its sidecar, correctly surfaced by
`open_project`), but the manuscript index has `chapter-004` under the phantom `019e942e`, `SceneReorderer`
walks `chapters[]`, never finds `019e9cdd`, and returns `invalidArgument: "sourceChapterID not found"`.

**Expected Behavior:** `manuscript.meta.json`'s `chapters[]` is always a faithful, de-duplicated, correctly
ordered list of the real chapter ids/paths on disk; structural edits keep it consistent (atomic, no phantom or
duplicate entries).

**Actual Behavior:** the index carries phantom ids, a duplicated path under two ids, and references to absent
dirs — diverging from the sidecars.

**Root Cause — CONFIRMED (2026-07-16, forensic disk + timestamp analysis).** The bug is in
**`ScriviCore/src/manuscript/ChapterCreator.cpp`**: the new chapter's on-disk **directory slug is derived from
the chapter *count*** — `newChapterOrdinal = ms.chapters.size() + 1` (L38) → `chapterSlug = "chapter-<ordinal>"`
(L42-44) → `chMetaRelPath` (L46-48). This is **not unique once any chapter has been deleted** (gaps make
`count+1` land on an existing directory number). On such a collision the create path:
1. `atomicWriteTextFile`s the new chapter's `chapter.meta.json` over the **already-existing** colliding
   directory's sidecar (L100) → **destroys the colliding chapter's identity** (its real id now survives ONLY as
   a stale entry in the index = phantom id), and
2. `ms.chapters.push_back({newID, chMetaRelPath})` (L104) **appends a second index entry for that same path**
   → duplicate path; the pre-existing entry now points at an id no sidecar has.

Both observed corruptions are this one bug:
- **Entry 3** (`chapter-004` path, phantom `019e942e`): UUIDv7 timestamps show `019e942e` was a real chapter
  created **Jun 4 19:49**, later clobbered when a `count+1` collision reused the `chapter-004` slug and
  overwrote its sidecar with `019e9cdd` (Jun 6). Index kept the original `019e942e` ref → phantom.
- **Entries 13/14** (`chapter-015` path listed twice): the current `chapter-015` sidecar `019f6b84` was written
  **today Jul 16 15:20** (matches the 11:29 index rewrite and the user's VNC split test), clobbering the
  **Jun 16** chapter `019ed110` that had that slug — leaving `019ed110` as a duplicate/phantom index entry.
  **This is the user's Failure-2 ("a new Chapter 15 was created" at end-of-Ch4/Scene-3), caught by timestamp.**

The user's Failure-1 (end-of-Ch4/Scene-2, follower present → "no new chapter created") is the same collision in
the **with-followers split**: `create_chapter` collides and clobbers the source chapter's sidecar, then a
downstream `reorder_scene` fails on the now-mismatched ids and the orchestration aborts — so the chapter was
"re-identified before the failure stopped creation" (the user's exact hypothesis, confirmed by the code path).

**Provenance:** the corruption is entirely application-generated, triggered by the user's **in-app** split/create
actions during VNC testing (NOT manual file edits, NOT Claude's automated tests). The user did nothing wrong —
they exercised a latent `ChapterCreator` slug-collision defect.

**Architecture study (2026-07-16):** the slug scheme + a second issue the user surfaced (`chapterID` is stored
in BOTH `manuscript.meta.json` and `chapter.meta.json` with no declared source of truth) are analyzed in
**`docs/Scrivi_Chapter_Folder_and_Identity_Trade_Study_v0_1.md`** (folder-naming options A1–A4 + id/order
source-of-truth options B1–B3). The durable fix for I-0072 should follow that study's decision; the minimum
safe close is "A2 monotonic slug + B1 sidecar-owns-id + a validator that prunes index entries whose sidecar id
differs." **Awaiting the user's decision on the trade study before implementation.**

**Immediate impact on SP-067 verification:** drag-reorder (and possibly split) **cannot be trusted-tested on
this project** — its index is already corrupt. Re-run AC4 verification on a **freshly created** project. Also
provide/confirm a repair path (rebuild `manuscript.meta.json` from the sidecars) for damaged projects.

**Fix direction (now scoped — the primary fix is in ChapterCreator):**
1. **`ChapterCreator` slug must be collision-free.** Do NOT derive the directory slug from `chapters.size()+1`.
   Instead pick a slug that can't collide — e.g. `max(existing chapter-NNN dir number)+1`, or a slug derived
   from the unique `chapterID`, or scan-and-skip existing dirs. **Never `atomicWriteTextFile` over a path that
   already exists** for a *new* chapter (a create should fail or pick a fresh dir, never clobber). This is a
   `[ScriviCore]` change affecting all platforms (Apple included) — same collision can occur on macOS after a
   delete; confirm and cover there too.
2. **De-dup / repair the index.** Add an integrity check that rebuilds/repairs `manuscript.meta.json`
   `chapters[]` from the authoritative on-disk sidecars (drop phantom ids whose path's sidecar has a different
   id; collapse duplicate paths) — a load-time self-heal in the spirit of I-0066's history prune and the
   External-Change-Repair matrix. Needed to recover already-damaged projects like this one.
3. `reorder_scene`/consumers already fail loudly ("sourceChapterID not found") — keep that; the durable fix is
   #1 (stop creating the corruption) + #2 (heal existing damage).

**Repro plan (to lock the fix):** fresh project → create several chapters → **delete** one (make a gap) →
create another → assert its dir slug did not collide and `chapters[]` stays 1:1 with the sidecars. Extend to
the split path (I-0064/I-0069/I-0070) once the create is fixed.

**Files:**
- **`ScriviCore/src/manuscript/ChapterCreator.cpp:38,42-48,100,104`** — the slug-collision + clobber + duplicate
  (PRIMARY fix)
- `ScriviCore/src/manuscript/SceneReorderer.cpp:40-56` — where the mismatch is detected (fails correctly)
- `ScriviCore/src/manuscript/ChapterReorderer.cpp` — verified clean (id-keyed, atomic; NOT a corruptor)
- `platforms/linux/src/EditorShell.cpp` `onCreateChapterRequested` — split orchestration that triggers create
- affected data: `…/the-twisted-remains-of-myself.scrivi/manuscript/manuscript.meta.json` (needs repair or
  discard; it's a damaged test project)

---

---

## I-0073: [Linux] Scene drag-drop has a 1–2 s lag before the drop target is realized

**Status:** ⚪ **CLOSED — environmental / not reproduced (2026-08-09, user-directed).** Closed at the user's
instruction without executing triage step 1 (the real-hardware datapoint). The evidence available all points
away from Scrivi code: the symptom appeared **only** through the Docker+Xvfb+x11vnc harness, and the per-move
path does no I/O, no backend call and no notable allocation, so nothing on it can account for ~1 s. A remote
framebuffer protocol batching high-frequency pointer motion during a drag is the far more probable cause.

> ⚠️ **Closed on inference, not measurement.** No native-Linux reading was ever taken, so this is *not* proof
> the code is innocent — it is a judgement that the cost of chasing a VNC-only artifact exceeds the value.
> **If an alpha tester reports drag lag on real hardware, open a new Issue** and start from triage step 2
> (profile `dragMoveEvent`, `indexAt`/`dropIndicatorPosition`, autoscroll timer, repaint volume) rather than
> re-litigating this entry.

**Original entry (2026-07-16) follows.** Observed by the user during I-0067/I-0068 VNC verification (2026-07-16):
the drop target / insertion line takes "a second or two" to catch up while dragging. **Unknown whether this is
environmental (Docker+VNC) or a code issue** — flagged so it isn't lost.
**Platform:** Linux (`platforms/linux/`), observed **over the Docker+VNC harness** (not real hardware).
**Component:** drag-hover path — `NavigatorTree::dragMoveEvent` (`NavigatorTree.cpp:117`); or, more likely, the
Xvfb → x11vnc → VNC-client pointer-motion pipeline.
**Severity:** Low (cosmetic responsiveness; the drop itself resolves correctly — I-0067/I-0068 verified clean).
**Sprint:** — (triage first)
**Epic:** EP-023 `[Linux]`

**Description:** while dragging a scene row, the insertion-line drop indicator lags the cursor by ~1–2 s before
the landing position updates.

**Analysis (code path is cheap — points away from our code):** `dragMoveEvent` does only `QTreeView::
dragMoveEvent` (base indicator/autoscroll) + `indexAt` + `resolveDrop` (a handful of `data()` reads and string
compares) — **no I/O, no backend call, no notable allocation.** Nothing on the per-move path costs ~1 s.
High-frequency pointer motion during a drag is exactly what a remote framebuffer protocol (VNC over
Xvfb-in-Docker) throttles/batches, so the most probable cause is **environmental round-trip + frame-encode
latency**, not compute.

**Triage plan (before treating as a code bug):**
1. **Real-hardware datapoint** — the alpha tester runs the drag on native Ubuntu (no VNC). If smooth there, it's
   the harness → close as environmental / won't-fix (harness-only).
2. If it lags on real hardware too, profile `dragMoveEvent` (is `indexAt` or `dropIndicatorPosition` unexpectedly
   costly on this model?) and check whether the base `QTreeView::dragMoveEvent` autoscroll timer or a repaint is
   the cost; consider throttling our own resolve or reducing repaints.

**Do not act until step 1 gives a real-hardware reading** — optimizing a VNC-only artifact would be wasted work.

---

---

## I-0085

**Status:** ✅ **Closed — Not a Bug (2026-07-21)**: clean relaunch reproduced the working Open dialog; the crash was the environmental duplicate-instance + beta-OS XPC panel timeout, not Scrivi code.
**Severity:** High
**Sprint:** **SP-075** (found)

**Description / Reason for Closure:**
`[Apple]` **App crashes constructing the Open Project panel.** From `LandingView` (or File ▸ Open Project…), clicking Open calls `AppEnvironment.presentOpenProjectPanel()` → `NSOpenPanel()`, which aborts with `*** Assertion failure in -[NSOpenPanel _initBridgeAndStuff], NSSavePanel.m:466 — Advance to configuration phase semaphore timed out.` This fires at panel **construction** (`NSOpenPanel()`), before `runModal`. **Investigation (2026-07-21):** the assertion is the sandbox Open/Save-panel ViewBridge XPC service (`com.apple.appkit.xpc.openAndSavePanelService`) timing out its init handshake. Evidence it is **environmental, not a code defect**: (a) this exact `NSOpenPanel()` code dates to **SP-035** and is unchanged through ~40 sprints of working Open flows; (b) entitlements are correct (`app-sandbox` + `files.user-selected.read-write`) and `ENABLE_APP_SANDBOX = YES`; (c) **a second Scrivi instance was found running** (`…Scrivi -NSDocumentRevisionsDebugMode YES`, an Xcode debug launch) alongside the `open`-launched one — two instances of a sandboxed app contend for the single per-user panel XPC service, a known trigger for this timeout — even though `LSMultipleInstancesProhibited = true`; (d) host is **macOS 27.0 beta (26A5378n)**, where ViewBridge XPC flakiness is common; (e) **not merge-related** — `presentOpenProjectPanel` is untouched by SP-075. **Resolution (2026-07-21):** confirmed **environmental, NOT a code defect.** The user quit all instances and relaunched clean; the Open Project dialog constructed and ran fine, exactly as the investigation predicted (duplicate sandboxed instance + macOS 27.0 beta panel-XPC handshake timeout). No code change made or warranted — `presentOpenProjectPanel`/`NSOpenPanel()` are unchanged and correct. Closed as Not-a-Bug.

> *Archived from the Issue-active.md table row (2026-08-15); no separate full entry existed.*

---

## I-0103

**Status:** ⚪ **Superseded by I-0104** — diagnosis complete (2026-08-07); the root cause is the hash source, not the newline
**Severity:** Low
**Sprint:** —

**Description / Reason for Closure:**
`[Apple]`/`[ScriviCore]` **A trailing-newline discrepancy makes some scenes re-flag as externally changed on every open.** Split out of I-0102 (2026-08-06). On the user's real project 5 of 59 scenes mismatch their persisted head hash at open; **2 match exactly once a trailing `\n` is appended**, i.e. the on-disk file and the text history recorded differ by one byte the writer never typed. Because `validateSceneHead` re-seeds and records an `externalChange` barrier on each mismatch, those scenes generate a fresh barrier **every open, forever** — a permanent source of history noise independent of I-0102's attribution bug. The other 3 mismatches did not normalize simply and may be genuine external edits. **Not yet diagnosed:** whether the manuscript writer appends/strips the newline, or history stores the pre-save text. **Impact after I-0102:** noise only — the barriers are now attributable and filterable, so the card is usable regardless. **SUPERSEDED by I-0104 (2026-08-07):** the trailing newline is one symptom of a broader defect — the head hash is taken over the *replayed in-memory head* rather than the bytes written to disk, so the comparison is head-vs-disk and mismatches routinely without any external edit. The 2 trailing-newline scenes and the 3 that "did not normalize simply" are all explained by it. Fix tracked in **I-0104 / SP-093**; do not work this entry separately.

> *Archived from the Issue-active.md table row (2026-08-15); no separate full entry existed.*
