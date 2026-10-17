---
sprint: SP-181
epic: EP-048
status: Active
activated: 2026-10-15
platform: Linux
created: 2026-10-15
---

# SP-181 — `[Linux]` [EP-048] **S4b**: Replace and Replace All over the presented text

**Status:** 🟠 **IMPLEMENTED 2026-10-15** — ⏳ AC4's GNOME check and AC6, the rig live pass · 🟡 ACTIVATED 2026-10-15 (user: *"activate SP-181 and implement it."*) — created 2026-10-15 (user: *"plan EP-048 fully."*). ✅ Q1–Q3 ruled 2026-10-15 (Q3 against the
recommendation, and widened: only the scenes with a match are written, on both platforms).
**Carries:** [T-0608] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md) · [I-0303] `[Apple]`, fixed first (plan
step 0), so Linux mirrors the corrected shape ([I-0289]'s pattern).
· [I-0304] `[Linux]` and [I-0305] `[Apple]` `[Linux]`, filed from the findings and ruled in (2026-10-15).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). **L8's Replace
half.** Split from [SP-176] (Find) by the user, 2026-10-11, because Replace All needed Linux undo (SP-176 Q3).
**Builds on:** [SP-178] (EP-026 S2, Linux undo) — its grouped edit (one undo step across scenes) passed the rig on 2026-10-13
(steps 4–12). ⚠️ SP-178 is not closed: its AC6/AC8 wait for the rig, for [I-0297] and [I-0298]. Neither touches the grouped
edit path this Sprint uses.
**Authority:** EP-048 L8 · [SP-164] Q2 (Replace All = ONE undo step, across scenes) · [SP-164] Q3 (a replacement takes the
style of the match's first character, the type-over rule) · [SP-176] Q2 (Find and Replace… = Ctrl+Alt+F) · design
[`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §7.
**Shape:** ✅ Apple's (`feedback_linux_adopts_apple_shape`): `replaceFound` (`Scrivi/Views/ManuscriptTextView.swift:2887`),
`applyReplacements` (`:2894`), the coordinator's `replaceAll` (`:2484`).
**Size:** S–M. ⚠️ Needs a **rig live pass**.

---

## What reading the code found (2026-10-15)

- ✅ **The bar was built for this.** `FindBar` has no Replace controls; it was built so S4b adds a second row
  (`platforms/linux/src/FindBar.hpp:8`).
- ✅ **The write path exists.** `ManuscriptEscapes::escape` escapes typed text, and `ManuscriptEditor::replaceRange`
  (`ManuscriptEditor.cpp:567`) balances through `scrivi_markdown_balanced_edit` ([SP-175]).
- ✅ **One undo step across scenes exists.** `groupedEditBegins` / `groupedEditEnded` (`EditorShell.cpp:367–387`) report
  every part as ONE `HistoryCapture::groupedEdit`. `deleteAcrossScenes` (`ManuscriptEditor.cpp:1143`) uses it, and it passed
  on the rig.
- **Apple, one Replace:** `replaceFound` is `insertText`, the typing path. Nothing flushes or labels it, so the core's capture
  rules decide its step. ✅ Linux matches by reporting it as an ordinary edit (`replaceRange` → `contentsChange` →
  `noteEdit`). No new rule.
- **Apple, Replace All:** one editing pass, back to front; each replacement escaped, snapped (`snapSelection`) and balanced
  as a single Replace. History is flushed (`trigger: "replace"`), the pass runs with capture suppressed, and then ONE
  `recordGroupedEdit(kind: "replace")`. Touched scenes are saved asynchronously. Measured on dumas: **1,173 edits in 83 ms**
  ([SP-164]). Its first, per-edit version took about **a minute**.
- ⚠️ **Linux saves on the UI thread, one scene at a time** (`saveDirtyScenes`, `EditorShell.cpp:1794`). A Replace All on
  dumas touches about 1,171 scenes, so about 1,171 synchronous saves. Over the rig's share that is [I-0297]'s class (40 s
  accepted there, for the structural undo). → **Q3**.
- ⚠️ **Each `replaceRange` is its own document edit.** Every one fires `contentsChange` → `SceneDocument` reconcile, a
  grouped-part lookup (a linear scan of `groupParts_`, `EditorShell.cpp:1486–1494`) and a re-highlight. That is the shape of
  Apple's slow first version. **Measure before building** (plan step 0).
- After a Replace All, Find's presented text rebuilds every touched scene. The whole build measured **292 ms** on dumas
  ([SP-176]). Acceptable, once.

## Goal

✅ **The Linux writer replaces the text they SEE**, with Apple's results: escaped, balanced, and in the match's style.
Replace All is one undo step, and it does not freeze the window.

## Plan

0. **`[Apple]` [I-0303] first** (user, 2026-10-15: *"apply the rule to both platforms. File it in [Apple] and put it in
   SP-181 before linux."*). The Mac's Replace All does its scene writes off the main thread, behind progress, with the
   manuscript locked until it lands; only the matched scenes are written; [I-0259]'s failed-write rule kept. An interop test
   fails first. Find how the Mac already shows a long operation before writing a new one.
0b. **Prove, on BOTH platforms, that a Replace All writes only the scenes holding a match** (Q3 ruling). A test through the
   real paths on a project where the query occurs in two scenes of many counts the scene files (and sidecars) written by
   the Replace All and by the next autosave, and what goes into the history step: exactly those two. Apple through
   `replaceAll` (interop test); Linux through the bar (`find_smoke`). ⚠️ By reading, both already hold (Apple saves only
   `touched`, `ManuscriptTextView.swift:2487, 2517`; Linux marks dirty only a scene a `contentsChange` landed in,
   `EditorShell.cpp:1473`), but that is a reading, not proof. Where a test fails, the fix is in this Sprint; an `[Apple]`
   failure is filed as its own Issue first ([I-0289]'s pattern).
1. **The Replace row** (Q1): a second row of the bar holding the replacement field, **Replace** and **Replace All**.
   Edit ▸ Find ▸ Find and Replace… (**Ctrl+Alt+F**, ruled in SP-176 Q2) opens the bar with the row; Find… (Ctrl+F) opens it
   without. Return in the replacement field does Replace.
2. **Replace:** when the editor's selection IS the current match, replace it: escaped, through `replaceRange` (balanced; the
   first character's style), then Find Next. When it is not a match, select the next match and replace nothing.
3. **Replace All:** every match, back to front, as ONE grouped edit of kind `replace` (one undo step, SP-164 Q2), each
   escaped, snapped and balanced as a single Replace. The count shows how many were replaced. The caret stays where it was,
   clamped (Apple's).
4. **Replace All runs off the GUI thread** (Q3 ruling): the replacements are computed and the touched scenes written on a
   worker, behind the progress row, with the editor read-only until it lands (`runStructural`'s pattern, [I-0297]). The
   editor then takes the new texts and the history records ONE grouped step. Only the scenes with a match are written.
5. **Menu:** Edit ▸ Find ▸ Find and Replace… Ctrl+Alt+F, in `addFindMenu`. ⚠️ Measure it against GNOME on the rig before
   relying on it (the SP-175 Ctrl+Alt+T lesson).
6. **Tests** (`find_smoke` extended, already in CI):
   - Replace writes escaped bytes (`d*g` → `d\*g`, Apple's `FindReplaceTests` case) and stays balanced across a span edge.
   - It takes the first character's style.
   - Replace All is one undo step: Ctrl+Z restores every scene in the editor and on disk, and Ctrl+Shift+Z re-applies it.
   - Ctrl+Alt+F through the real key dispatch.
   - The dumas timing, opt-in as [SP-180]'s §8.
   - **Mutations:** no escape · one step per scene instead of one group · no balance · Replace with a non-match selection
     replaces anyway.
7. **Live pass** (user, rig). Steps in the chat reply; Scrivi autosaves.

## Questions

### Q1 — What the Replace row holds, and what its buttons are called

**The situation.** On the Mac, ⌥⌘F opens AppKit's own find bar with a second row: a replacement field and two buttons,
labelled *Replace* and *All*. Scrivi did not choose those labels; `NSTextFinder` draws them. Linux's bar is Scrivi's own
(`FindBar`), so Linux must choose. GNOME's text editors label the second button *Replace All*.

**Options.**
- (a) A replacement field, *Replace* and *Replace All*.
- (b) A replacement field, *Replace* and *All*, Apple's words exactly.

✅ **Recommendation: (a).** ✅ **RULED 2026-10-15 (user): (a).** The buttons do exactly what Apple's do, so behaviour is unchanged. Only the label differs: *All*
reads as unclear on its own next to a text field, and *Replace All* is what a Linux writer expects to see.

### Q2 — What Replace does when the selection is not a match

**The situation.** Replace is meant to act on the current match: Find selects it, and Replace overwrites it. But the writer
can click in the text between the two, so when Replace is pressed the selection may be a caret, or text that is not a match.
Apple's bar handles this inside AppKit; Linux's bar has to decide.

**Options.**
- (a) Select the next match from the caret and replace nothing. The writer sees it and presses Replace again to replace it.
- (b) Replace the next match at once, without showing it first.

✅ **Recommendation: (a).** ✅ **RULED 2026-10-15** (user: *"agreed. find first, then replace."*). A Replace then only ever changes text the writer could see selected, which matters in a manuscript
where the next match may be pages away. It costs one extra press only in this case; in the normal Find → Replace → Replace
sequence nothing changes.

### Q3 — How the scenes a Replace All touches are saved

**The situation.** A Replace All of a common word on dumas changes about 1,171 scenes at once. The text change itself should
be fast (Apple: 83 ms). But Linux saves each scene synchronously on the UI thread (`saveDirtyScenes`,
`EditorShell.cpp:1794`), one file at a time. On the rig the project lives on a network share, where [I-0297] showed the
same kind of bulk write freezing the window for 90 s to 2 minutes (40 s after its fix, accepted for now). 1,171 saves in a
row could do the same: the replace looks done, then the window stops responding.

**Options.**
- (a) Measure first (plan step 0). If the saves block the window for more than about 1 s over the share, run them on a
  worker thread behind the existing progress row, with the editor read-only until they finish. This is the pattern
  `runStructural` already uses for [I-0297].
- (b) Keep synchronous saves, as every other Linux save is today, and accept whatever freeze it produces.
- (c) Always save on a worker, without measuring.

✅ **Recommendation: (a).** The numbers decide whether new code is needed. If it is, the code reuses a pattern already
proven on the rig rather than inventing one; (c) adds that code even if the saves turn out fast, and (b) risks shipping a
freeze the writer would hit on the first real Replace All.

✅ **RULED 2026-10-15: (c), and wider.** User: *"don't measure it. It will be horrible. Replace All must not run on the gui
thread. always use the worker (it will be needed). Also we need to not automatically rewrite every frigging scene if it
isn't necessary. If replace all would only replace one or two occurences, then it must only write to those occurences. We
will need to check both Apple and Linux to determine if that is true and take steps to make it true if it turns out it
isn't."* → plan steps 0b and 4.

⛔ **NOT in this Sprint:** regular expressions (Apple's bar offers none) · replace across projects · the navigator's search
(SP-176 Q5) · any change to Find's matching.

## Acceptance Criteria

- [x] AC0a — `[Apple]` [I-0303]: the Mac's Replace All writes no scene on the main thread, shows progress and locks the
      manuscript until it lands; interop test red first — `replaceAllWritesOffTheMainThread` (red, then green; 2 mutations
      caught)
- [x] AC0 — Plan step 0b proven on Apple AND Linux: a Replace All writes only the scenes holding a match — Mac: the same
      interop test; Linux: `replace_smoke` §3 (`write-all` mutation caught on both). ⚠️ That the HISTORY step holds only those
      scenes is by construction (it is built from the touched scenes), not asserted separately. Nothing failed, so no fix
- [x] AC1 — **L8 (Replace half):** Replace writes escaped, balanced text in the match's first character's style, then finds
      the next; with a non-match selection it only selects (Q2) — `replace_smoke` §2 (`replace-any` caught)
- [x] AC2 — Replace All replaces every match in one pass, as ONE undo step: Ctrl+Z restores every scene's text and file, and
      Ctrl+Shift+Z re-applies it — `replace_smoke` §3–§4 (`no-escape`, `no-group` caught). ⛔ Not timed: Q3 ruled *"don't
      measure it"*
- [x] AC3 — Replace All runs on a worker, never the GUI thread (Q3); every scene with a match is on disk afterwards, and no
      other scene file is written — `replace_smoke` §3 (`on-gui` caught, after the check was sharpened)
- [x] AC4 — Edit ▸ Find ▸ Find and Replace… Ctrl+Alt+F through the real dispatch ✅ (`replace_smoke` §1); ✅ not bound by GNOME
      (rig live pass 2026-10-15, step 11: the bar opens with the Replace row)
- [x] AC5 — a new `replace_smoke` (in CI) rather than `find_smoke` extended, since Replace All needs a real project, history
      and disk; mutation-checked (5 of 5); ctest macOS 724/724, Linux 727/727 (uid 1000); Linux smokes 33/33; interop 292/292
- [x] AC7 — `[Linux]` [I-0304]: a scene whose save fails stays dirty and is retried by the next save — `background_wait_smoke`
      §1–§2 (2 mutations caught)
- [x] AC8 — `[Apple]` `[Linux]` [I-0305]: closing a project window or quitting while background writes run waits for them,
      behind a dialog with a spinner (*"Please wait — saving updates…"*), then closes as usual — Mac
      `closeWaitsForBackgroundWrites`, Linux `background_wait_smoke` §3–§4 (2 mutations caught on each). ⏳ Live: both
      platforms, in the AC6 pass
- [ ] AC6 — ✅ **Rig live pass (user)**

## Progress log

### 🔵 2026-10-15 — Sprint CREATED in Planning

User: *"plan EP-048 fully."* Issued with [SP-182]–[SP-184] (EP-048 S5a–S5c) via `next-id.py`. [T-0608] filed. Q1–Q3 await
rulings.

### 🔵 2026-10-15 — Q1–Q3 RULED

User: *"Q1: Replace and \"Replace All\". Q2: agreed. find first, then replace. Q3: don't measure it. It will be horrible.
Replace All must not run on the gui thread. always use the worker …"* Q3 also widened the Sprint: a Replace All writes only
the scenes holding a match, proven on both platforms (plan step 0b). ⚠️ Found while answering: Apple's own touched-scene
saves are NOT off the main thread (`ViewportSceneLoader.saveScene` is `async` but calls `saveSceneBlocking`, and the Tasks
run `@MainActor`). Asked whether the worker rule applies to the Mac.

### 🔵 2026-10-15 — [I-0303] `[Apple]` filed and ruled in, before Linux

User: *"yes. apply the rule to both platforms. File it in [Apple] and put it in SP-181 before linux."* → plan step 0, AC0a.

### 🟡 2026-10-15 — Sprint ACTIVATED (user-approved)

User: *"activate SP-181 and implement it."* [T-0608] → active.

### 🟠 2026-10-15 — IMPLEMENTED ([T-0608] Implemented - Not Verified, [I-0303] Resolved - Not Verified); ⏳ rig live pass

**Step 0, `[Apple]` [I-0303]:**
- `ViewportSceneLoader.saveScenesOffMain` takes the matched dirty scenes' texts on the main actor and writes them in a
  detached task. When they land it marks each clean (unless changed meanwhile, then saves it again), tells history the
  bytes, and runs `whenLanded` in the same step that lowers `scenesWritingOffMain`. A failed write stays dirty ([I-0259]).
- ⚠️ A main-actor save of a scene still in flight is deferred until the flight lands, so an older write can never land over
  a newer one. The quit path waits for the flight (`DispatchGroup`, 30 s), then writes the still-dirty scenes itself.
- `replaceAll` (`ManuscriptTextView.swift`) uses it: the manuscript is not editable, and Undo / Redo are off, until the
  writes land. `EditorView` shows *"Saving N scenes…"* as an overlay (the [I-0263] reason: no layout change).
- **Test:** `replaceAllWritesOffTheMainThread`, red first (the writes ran on the main actor, after `replaceAll` returned).
  Mutations caught: writing every dirty scene; writing on the main actor.
- ⚠️ **Found by the full run:** the unlock first ran in a separate task, after the count reached zero, so the editor read as
  locked for a moment. Now the same step.

**Step 0b:** both platforms write only the scenes holding a match (the tests above and below).

**Linux:**
- `FindBar`: the Replace row (Q1), Find and Replace… Ctrl+Alt+F, Replace (Q2: the current match only, typed over through
  the new `ManuscriptEditor::typeOver`, then the next) and Replace All, handed to the shell; *"N replaced"* in the count.
- `EditorShell::replaceAll` (Q3), three steps:
  1. The worker computes each matched scene's balanced text (`scrivi_markdown_balanced_edit` on plain strings), every match
     snapped and widened as a Replace would type over it.
  2. The GUI thread applies the finished texts (only the span that differs, `replaceSceneSpan`) as ONE grouped `replace`
     step. Qt's document is GUI-only, so this cannot move.
  3. The worker writes those scenes (`saveScenesOffMain`, Apple's shape).
  The editor is read-only with the progress row (*"Replacing…"*, then *"Saving N scenes…"*) throughout. The grouped-edit
  lambdas became `beginGroupedEdit` / `endGroupedEdit`, and the lock `lockForWork` / `unlockAfterWork`.
- **Tests:** a new `replace_smoke` (CMake, wrapper, CI step); 5 mutations caught (`no-escape`, `write-all`, `no-group`,
  `replace-any`, `on-gui`).
  - ⚠️ `on-gui` survived at first: the saves are async either way, so "locked on return" did not catch a synchronous compute.
    Now it also checks the text is unchanged when Replace All returns.
  - Two test mistakes fixed: moving the caret to another scene flushes every dirty scene (the T-0239 cadence), so the
    unsaved-typing step now comes after the search; and a SHOWN shell destroyed segfaulted (a focus-out on a document
    already gone), so the smoke hides it first, as the app and `structured_paste_smoke` do.

**Runs:** interop 292/292; ctest macOS 724/724, Linux 727/727 (uid 1000); Linux smokes 33/33 (Docker, Release, Qt 6.4.2);
no compiler warnings in the Linux build or the changed Swift files.

**Found, not changed (for rulings):**
- ⚠️ **Undoing a Replace All writes its scenes on the GUI thread, on both platforms** (the ordinary undo apply path). Q3's
  rule was about Replace All; whether it covers its undo and redo is the user's call.
- ⚠️ **Linux drops a scene from the dirty set when its save FAILS** (`EditorShell::saveScene`: *"A retry policy is out of
  scope here"*); the Mac keeps it dirty ([I-0259]). The new off-main save mirrors Linux's existing rule.
- ⚠️ **Closing a Linux project window while the writes are in flight:** the scenes are already out of the dirty set, so the
  close does not write them again; the worker finishes them. The same holds for `runStructural` ([I-0297]). Not tested.

### 🟠 2026-10-15 — Rulings on the three findings

User:
1. *"I think you believe it should be undo in the background in this one instance on both platforms … Perhaps we should
   consider doing all undos in a background thread (at least the ScriviCore part). Perhaps we can create the beginnings of
   a persistent thread pool (if the OS doesn't already do that) or … a dedicated thread for performing undo/history update
   operations."* ⏳ Thinking aloud, not yet a ruling. ✅ Design note written 2026-10-15 (user: *"write the design not[e]"*): [`../Scrivi_Background_Writes_Design_v0_1.md`](../Scrivi_Background_Writes_Design_v0_1.md), ✅ Q1–Q5 ruled 2026-10-15 → a new Epic, [EP-052] `[ScriviCore]` Background Writes (Draft). Not this Sprint's work.
2. *"We'll have to remark it as needing saving when the save fails."* ✅ Linux keeps a scene dirty after a failed save, as
   the Mac does ([I-0259]).
3. *"In both cases (Apple and Linux), if we attempt to close the application while a background update is in progress the
   close must WAIT until the background process is completed. I recommend showing a dialog with a spinner"* (*"Please
   Wait, saving updates"*). ✅ Ruled for both platforms.

✅ Filed and fixed here (user, 2026-10-15: *"yes, file them into SP-181 and fix them now"*): ruling 2 → [I-0304] `[Linux]`;
ruling 3 → [I-0305] `[Apple]` `[Linux]`. Ruling 1 (a per-project serial queue for history and scene writes) stays a design
recommendation for later, not this Sprint.

### 🟠 2026-10-15 — [I-0304] and [I-0305] implemented

- **[I-0304] `[Linux]`:** a scene leaves the dirty set only after a write that succeeded: the ordinary save, the teardown
  save, and the off-main save (a failed or timed-out write is put back). Retries come on the existing cadence.
- **[I-0305] `[Apple]` `[Linux]`:** one "background work done" signal per project, and a close waits on it:
  - Apple: `ViewportSceneLoader.whenOffMainWritesLand`; `SavingUpdatesWait` shows the spinner as a sheet (window close,
    Close Project) or a panel (Quit, `.terminateLater`).
  - Linux: `EditorShell` counts its off-main saves, Replace All and structural workers; `ScriviWindow` waits in
    `closeEvent` and Close Project; Quit waits for every busy window behind one dialog before closing any.
- **Tests:** Apple `closeWaitsForBackgroundWrites` (interop); Linux `background_wait_smoke` (new, in CI, non-root). 6
  mutations caught: 2 Mac, 4 Linux.
- **Runs:** interop 293/293; Linux smokes 34/34 (Docker, uid 1000); no compiler warnings in either build. ScriviCore
  unchanged, so ctest was not re-run.

**Found, not changed:**
- ⚠️ **`[Apple]` File ▸ Close Project (⌘W) skips [T-0570]'s "Some changes can't be saved" question.** It calls
  `NSWindow.close()`, which never asks `windowShouldClose`. The comment there claimed it covers ⌘W. The [I-0305] wait is
  in both places, so this Sprint's fix holds. ✅ Filed as [I-0306] `[Apple]`, 2026-10-15 (user: *"file the ⌘W gap"*), unassigned.
- The window-close wiring itself (`windowShouldClose`, `close()`, `applicationShouldTerminate`, Linux `closeEvent`) is
  thin; the Mac's is covered by the live pass, not by a test.


### 🟠 2026-10-15 — Mac live check of [I-0305]: the sheet cut its text off

User: *"the sheet displayed is too small to display all the text in the title. we should put an identical label above the
spinner."* ✅ The label *"Please wait — saving updates…"* now sits ABOVE the spinner at its full width, and the sheet takes
the view's size (`NSHostingController.sizingOptions = .preferredContentSize`). `closeWaitsForBackgroundWrites` still passes;
⏳ the look is for the user to re-check.

✅ **Re-check (user, 2026-10-15):** *"the sheet looks right now."* The Mac half of [I-0305]'s live check passes; Linux waits for
the rig.

### 🔵 2026-10-15 — Rig live pass, step 11

✅ AC4: Ctrl+Alt+F opens the find bar with the Replace row on the rig. ⛔ Typing in the bar then crashed the app → [I-0308]
`[Linux]` (filed, unassigned). The pass stopped there for the day; AC6 is not yet run.

