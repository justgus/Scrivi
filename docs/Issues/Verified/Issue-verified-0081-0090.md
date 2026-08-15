# Verified Issues — I-0081 to I-0090


---

## I-0081

**Status:** ✅ Resolved - **Verified (2026-07-19, VNC round 2)** — a drag-reordered scene renames immediately, no restart needed
**Severity:** High
**Sprint:** **SP-073**

**Description / Resolution:**
`[ScriviCore]`+`[Linux]` **Scene rename (and save) fails after a drag-reorder — the segment's paths are stale.** Found in the SP-073 VNC walkthrough: a scene moved to another chapter can't be renamed until quit→reopen (fresh paths), while newly-created scenes rename fine. **Root cause:** under EP-027 §8, `scrivi_reorder_scene` **renames the scene's files** to their new order-key stem — and a cross-chapter move **relocates them** into the target chapter's folder — but its envelope reported no paths, and `EditorShell::onSceneDropped` (SP-067 code, written pre-§8 when a reorder was an index shuffle) kept the pre-move `metadataPath`/`contentPath`/`chapterMetadataPath` in the segment. A later `rename_scene`/`save_scene` through them targets vanished files (the I-0074/I-0079/SP-073-chapter stale-path class — this closes the last member: scene drag). **Fix:** `ReorderSceneResult` + the `reorder_scene` envelope now report the post-move `metadataPath`/`contentPath`/`chapterMetadataPath` (payload-only, `scrivi.h` untouched); `SceneDocument::refreshScenePaths` applies them in `onSceneDropped`. Core regression `reorderScene - reports the scene's post-move paths (I-0081)` + smoke coverage (rename+save through refreshed paths in `scene_reorder_smoke` Case C).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0082

**Status:** ✅ Resolved - **Verified (2026-07-19, VNC round 2)** — chapter drag works (teal selection, container move, persists across quit→reopen)
**Severity:** High
**Sprint:** **SP-073**

**Description / Resolution:**
`[Linux]` **Chapter drag never starts — the heading can't be picked up; the drag rubber-band-selects the nearest scene instead.** Found in the SP-073 VNC walkthrough (chapter selection box shows pale blue = non-selectable; dragging a heading just moves the scene selection up/down). **Root cause:** `QAbstractItemView::mouseMoveEvent` enters `DraggingState` (and calls `startDrag`) only when the pressed row is among the **selected** draggable indexes — and chapter rows were `setSelectable(false)` (SP-061 "chapters group; scenes select"), so a chapter row could never be in the selection and the view fell into rubber-band drag-selection instead. The SP-073 manual-`QDrag` path in `NavigatorTree::startDrag` was correct but unreachable. **Fix:** chapter rows are now selectable (`rebuildNavigator`); clicking a heading is harmless (`onNavigatorActivated` ignores rows without a sceneID). Headless smokes can't drive a real mouse drag — VNC-only class, caught exactly where expected.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0083

**Status:** ✅ Resolved - **Verified (2026-07-21)** — core `scrivi_merge_chapter` (T-0300) relocates every scene file into the predecessor before removing the emptied chapter; macOS app adoption (SP-075 T-0303) points `handleMergeChapter` at it, retiring the lossy `deleteChapter` composition. User confirmed in the GUI that a chapter-merge survives quit→reopen with every scene present and in order.
**Severity:** High
**Sprint:** **SP-074/SP-075**

**Description / Resolution:**
`[Apple]`+`[ScriviCore]` **Chapter-merge loses scenes on reopen.** The macOS `⇧⌘-Backspace` chapter-merge (`ManuscriptTextView.handleMergeChapter`) reassigns the merged chapter's scenes to the predecessor chapter **in memory only** (`ViewportSceneLoader.mergeChapterIntoPredecessor`), then calls `scrivi_delete_chapter(currentChapterID)`. Under EP-027 the scene `.md`/`.meta.json` files physically live in the chapter's own `chapter-<orderKey>/` folder, so `deleteChapter` removes the folder **and every scene file the in-memory model believed it had preserved** → on quit/reopen those scenes are gone. **Fix (EP-028/SP-074):** a first-class atomic `scrivi_merge_chapter` that **relocates** the scene files into the predecessor folder (minting order keys after the predecessor's last scene) before removing the emptied chapter; the app calls it instead of composing merge from delete.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0084

**Status:** ✅ Resolved - **Verified (2026-07-21)** — fix in `ManuscriptTextView.rebuildStorage`; user confirmed in the GUI that the caret holds its seam position after a scene merge (no jump to the next scene/chapter start).
**Severity:** Medium
**Sprint:** **SP-075**

**Description / Resolution:**
`[Apple]` **Caret jumps to the next scene/chapter start after a scene merge.** After `⌘-Backspace` scene-merge (SP-075 T-0303), the cursor lost its intended seam position and landed at what looked like the start of the following chapter/scene. **Root cause:** `handleMergeScene` mutates `loader.segments` (`@Observable`), which schedules a `updateNSView` pass; that pass rebuilt storage a SECOND time (its `segIDs != lastSegmentIDs` guard was still true because the coordinator's manual `rebuildStorage` never updated `lastSegmentIDs`), and the redundant `setAttributedString` dropped the selection to 0 — stomping the caret that `rebuildStorageAndPlaceCursor` had just placed. Latent in the original merge code; SP-075's added `await` exposed it. **Fix:** `rebuildStorage` now stamps `lastSegmentIDs`/`lastShowChapterTitles` at the end, so the follow-up `updateNSView` sees no change and skips the redundant rebuild — the caret placement survives. Benefits every manual-rebuild handler (merge/split/create).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0087: [Linux] Timeline dots crowd to one edge when a scene is a far time-outlier (no zoom to spread them)

**Status:** ✅ **Verified (2026-07-23, VNC)** — found in SP-081 T-0332 VNC verification (2026-07-22); fixed by
bringing **SP-083 (zoom/pan)** forward. User confirmed the zoom buttons work and zoom + pan spread the crowded
cluster so the far-outlier flashback and present-day dots are individually workable.
**Platform:** Linux (`platforms/linux/`).
**Component:** `platforms/linux/src/TimelinePanel.cpp` — the linear story-time → x mapping (`xForOffset`, the
`[minMs_, maxMs_]` window set in `setTimeline`). Not a defect in that code; it's the *design* (linear axis, no
zoom) meeting a real manuscript.
**Severity:** High (the timeline is unusable for any project with a large story-time span; band assignment by
drag is impossible).
**Sprint:** SP-081 (found) → **SP-083** (fix).
**Epic:** EP-025 `[Linux]`.

**Description:**
In a real project, one scene mid-manuscript is a flashback set ~2 years before the others. The timeline maps
story-time **linearly** across the strip, so the window spans ~730 days. Every present-day scene (spanning
days/weeks) collapses into a tiny cluster at the far edge, dots overlapping and un-grabbable; the flashback
sits alone at the opposite edge with a vast empty gap between. The writer cannot spread the cluster, so:
- moving a present-day scene is very hard (dots overlap), and
- **T-0332 band assignment (drag a dot UP onto a band label) is impossible** — the tiny horizontal room means
  any drag is read as a horizontal story-time move and opens the Time Delta Picker instead.

**Expected:** the writer can zoom into the present-day cluster to separate the dots and interact with them
(assign to bands, reposition), then pan across the timeline.

**Actual:** no zoom or pan exists yet (SP-083), so an outlier scene renders the rest of the timeline
un-interactable.

**Root cause:** the **linear time-axis with no zoom** — an intrinsic property of a faithful duration axis (an
outlier dominates the scale). This is the same class Apple hit (EP-016 I-0046 "year-spanning timelines: max
zoom too low to resolve a 24-hour cluster" + the T-0174 clustering redesign). It is NOT a Linux layout bug and
NOT the story-structure code (T-0329/0330/0331 all Verified independently).

**Decision (user, 2026-07-22):** **keep the linear time-axis** (Apple parity — the user built the Apple side
that way and wants Linux to match) — do **not** substitute even/ordinal spacing. Fix via **zoom + pan**, and
**bring SP-083 forward now** rather than deferring T-0332 verification. Gesture set (all VNC-safe or with a
VNC-safe fallback):
- **Zoom = `Ctrl`+mouse-wheel** — the universal X11/Linux zoom idiom (GIMP/Inkscape/browsers/VS Code/LibreOffice).
  Zoom about the pointer (center if the pointer is outside the strip). Carries over VNC via the wheel modifier,
  though a Mac Magic Mouse/trackpad may not emit a discrete wheel x11vnc forwards — hence the buttons below.
- **`+`/`−` control** (user spec): a tiny horizontal control at the timeline's **bottom-right**, below the
  scrollbars — `+` left, `−` right. Click zooms about the current pointer (center if away). A plain click always
  survives VNC → the guaranteed path for testing + non-power-users. Ship BOTH.
- **Pan = click-drag on the empty area above/below the dots** (background drag was reserved for this in SP-080;
  the dot itself is the story-time drag). Optionally `Shift`+wheel.

**Fix:** delivered by **SP-083** (activated 2026-07-22, brought forward from its planned position). Once zoom
exists, re-run the T-0332 band-assignment verify zoomed into the cluster.

**Files (fix lands in SP-083):**
- `platforms/linux/src/TimelinePanel.cpp/.hpp` — a zoom factor + pan offset over `xForOffset`/`offsetForX`;
  `wheelEvent` (Ctrl+wheel, zoom-about-pointer); a `+`/`−` widget; background-drag pan.

---

---

## I-0088: [Linux] A scene before Story Open (a flashback) can't be placed — negative story-time is floored to 0

**Status:** ✅ **Verified (2026-07-23, VNC)** — found in the SP-083 VNC verify on
`the-twisted-remains-of-myself`; user confirmed the flashback places correctly before Story Open and the later
scenes no longer follow it into the negative region.

**Symptoms (user report).** On the timeline: the "Story Open" label clings to the left edge regardless of
scroll. Setting the Flashback scene to "2 years before previous" moves it to sit directly under Chapter 1
Scene 1 (offset 0) instead of two years earlier. Scenes after the flashback (Ch 8 Scene 1) then show "2 hours
after Story Open" rather than shifting to well before Story Open. Overlapping dots and a mis-highlight on
select (Flashback → highlights Ch 3 Scene 1) followed from multiple scenes collapsing onto the same clamped
offset.

**Root cause — Linux shell only.** The backend (`ScriviCore`) stores `storyTime.offsetMs`/`gapMs` as signed
`int64` with **no clamping** (`SceneMetaJson.cpp`, `scrivi_c_api.cpp`), so it already supports scenes before
the epoch. The Linux Qt shell threw the negative away in three places and mislabeled the origin in two:

1. `TimeDeltaPicker::commitSetOffset` — `resultOffsetMs_ = std::max<qint64>(0, spinnerOffsetMs())` deleted the
   negative the instant "before" was chosen. **This is the primary defect.**
2. `EditorShell::showTimeDeltaPicker` chain propagation then computed `offset[i] = prevEnd + gapMs` from the
   clamped (=0) flashback, so every later scene inherited a wrong `prevEnd` → "2 hours after Story Open".
3. `TimelinePanel::offsetForX` also floored at 0, so a dot couldn't be *dragged* left of the epoch either.
4. `TimelinePanel::paintEvent` drew "Story Open" at a fixed `kSideInset` (left edge), not at the epoch's real
   position — so it never tracked scroll/zoom and, once negatives exist, sat nowhere near offset 0.
5. `TimelinePanel::humanStoryTime` measured `rel = offsetMs - minMs_` (relative to the earliest dot), so
   tooltips read relative to the flashback, not Story Open.

**Fix.**
- **Picker** (`TimeDeltaPicker.cpp/.hpp`): removed the zero-floor (`resultOffsetMs_ = spinnerOffsetMs()`, may be
  negative). Added an **anchor combo** — the delta can be measured from the previous scene's end *or* from
  **Story Open** (offset 0), so a flashback is naturally "N years before Story Open". Seeds the epoch anchor
  when the raw offset is ≤ 0. Direction relabeled "after"/"before".
- **Panel** (`TimelinePanel.cpp/.hpp`): `offsetForX` no longer floors at 0; `humanStoryTime` measures from the
  epoch (offset 0) and phrases "… before/after Story Open"; the epoch label is now **anchored to
  `xForOffset(0)`** with a dashed **origin tick**, and pins to the nearer edge with a ‹/› hint when the origin
  scrolls off-screen.

**Files:** `platforms/linux/src/TimeDeltaPicker.cpp`, `TimeDeltaPicker.hpp`,
`platforms/linux/src/TimelinePanel.cpp`, `TimelinePanel.hpp`. No `scrivi.h`/backend change. pbxproj N/A
(Linux/Qt only).

**Verify (pending, user over VNC):** open `the-twisted-remains-of-myself`; set the Flashback to "2 years before
Story Open" — it lands two years left of the origin tick, and every later scene shifts with it (no "2 hours
after Story Open"); the "Story Open" label + tick sit at the true origin and track zoom/pan; the tooltip on the
flashback reads "2 years before Story Open". Container build + existing Linux smokes green.

---

## I-0089: [Linux] Scene→band drag-up assignment (T-0332) never triggers — drag mode latches to horizontal

**Status:** ✅ **Verified (2026-07-23, VNC)** — found re-verifying T-0332 after SP-083 zoom fixed the crowding
(I-0087); the dots spread correctly but the drag-up assignment still failed until this fix. User confirmed the
drag-up assignment now works (target-band cue + ring, survives reopen).

**Symptom.** With a story structure applied, dragging a scene dot **up onto a band label** to assign it does
nothing — a small move just opens the Time Delta Picker (the horizontal story-time drag). This is what blocked
SP-081 T-0332 from being verified; SP-083 was brought forward assuming crowding was the only cause, but the
gesture stayed broken once the dots were spread.

**Root cause — a mode-latching bug, distinct from I-0087 (crowding).** `TimelinePanel::mouseMoveEvent` chose
between `DotToBand` and `DotHorizontal` **exactly once**, on the first mouse-move past the 4px drag threshold,
and the `DotToBand` branch required `p.y() < bandLabelRowHeight() + 6.0` — i.e. the pointer already inside the
~22px label row at the **top** of the strip. But scene dots are drawn at the strip's **vertical centre**
(`cy = height()/2 ≈ 60px` in the ~120px strip). On the first 4px move the pointer is still ~55px below the label
row, so `intoLabelRow` was false → the mode latched to `DotHorizontal` and never re-evaluated, no matter how far
up the drag continued. The assignment path was unreachable.

**Fix.** Classify the dot drag by **dominant direction, re-evaluated while the mode is still ambiguous** (not a
one-shot check): an **upward** drag (`dy < 0` and `|dy| > |dx|`) with a structure present →
`DragMode::DotToBand`; a clearly **sideways** drag (`|dx| ≥ |dy|`) → `DragMode::DotHorizontal`; a small diagonal
stays unresolved until one axis wins. Once `DotHorizontal` is chosen it holds (the picker preview is live). The
drop still resolves by `bandIndexAtX(release x)` (which already snaps an out-of-region flashback dot to the
nearer band). Added a visual **assignment cue** — the target band is outlined in the highlight colour and a
dashed leader line runs from the dragged dot up to the label row — so the writer can see they are in assignment
mode and which act they will drop into.

**Files:** `platforms/linux/src/TimelinePanel.cpp` (mode classification in `mouseMoveEvent`, cue in
`paintEvent`, reset in `mouseReleaseEvent`), `TimelinePanel.hpp` (`dragPos_`, `dragBandTarget_`). No backend /
`scrivi.h` change; no pbxproj (Linux-only).

**Verify (pending, user over VNC):** with a structure applied, drag a scene dot upward → the target band
outlines + a leader line appears; release → the dot gets that band's colored ring and the assignment survives
quit→reopen. A horizontal drag still opens the Time Delta Picker. This unblocks **SP-081 T-0332**. Container
build + Linux smokes green.

---

## I-0090

**Status:** ✅ **Verified (2026-07-24, VNC)** — user confirmed the second imported row ("Bureau of Identity") is fully visible above the scrollbar; container build green (211/211), smokes + app-launch PASS.
**Severity:** Medium
**Sprint:** **SP-082**

**Description / Resolution:**
`[Linux]` **The lowest imported-timeline row is obscured by the zoom scrollbar.** Found in the SP-082 VNC verify (2026-07-24, T-0342 on `the-twisted-remains-of-myself` with two imports — the second, "Bureau of Identity", was almost entirely hidden). **Root cause:** the imported rows stacked upward from `height()`, but the +/- zoom buttons + horizontal scrollbar (`layoutControls`) also occupy the bottom ~24px — so the lowest row centred directly under the scrollbar. **Fix:** reserve a `kBottomControlsHeight` (24px) band at the strip bottom — `importedRowY` now stacks up from `height() − kBottomControlsHeight`, `projectRowY` includes it in its reserved space, and the panel min-height already accounts for it.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*
