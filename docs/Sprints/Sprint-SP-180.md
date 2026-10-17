---
sprint: SP-180
epic: EP-026
status: Active
platform: Linux
created: 2026-10-13
---

# SP-180 — `[Linux]` [EP-026] **S4**: structured cut / copy / paste across scenes

**Status:** 🟡 **ACTIVE 2026-10-13** (user: *"yes, as step 0 for both platforms."*) — created 2026-10-13 (user: *"start planning EP-026 S4"*). ✅ Q1–Q4 ruled. ✅ AC0–AC8 met 2026-10-13 ([I-0293]–[I-0296] Verified 2026-10-15); ⏳ AC9 rig live pass.
**Carries:** [T-0607] → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md) (🟠 Implemented - Not Verified) · [I-0293] `[ScriviCore]` · [I-0294] `[Apple]` · [I-0295] `[Apple]` · [I-0296] `[ScriviCore]`.
**Epic:** [EP-026] `[Linux]` Undo/Redo History, Copy Buffers & Structured Cut/Copy/Paste →
[`../Epics/Epic-active.md`](../Epics/Epic-active.md). EP-026 Q2 (ruled 2026-10-11): EP-029's Linux follow-on is in this Epic.
**Builds on:**
- [SP-178] (S2): undo. A structured paste is a reversible structural history node.
- [SP-179] (S5): the copy buffers. A cross-scene slot holds a fragment.
- ⚠️ Neither has had its rig live pass. A defect either pass finds in undo or the buffers may touch this Sprint.

**Authority:**
- [`../Scrivi_Structured_CutCopyPaste_Design_v0_1.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_1.md): §3 (fragment), §4.2 (paste-splice), §5 (history) and T1–T4 (all A).
- [I-0270] ruling (2026-10-02): a cross-scene cut KEEPS the scenes. It replaced the collapsing cut in §4.3/§4.4.
- [I-0279]: a held fragment expires once anything else is copied.
- [T-0591]: each piece is balanced.

**Shape:** ✅ Apple's, as it is today (`feedback_linux_adopts_apple_shape`). In `ManuscriptTextView.swift`:
- `structuredCopyIfCrossBoundary` (`:2411`) and `balancedFragment` (`:2433`).
- `pasteStructuredFragment` (`:2593`).
- `applyStructuralInverse` (`:598`).
- The `copy` / `paste` / `cut` overrides (`:3183–3210`).
- The buffer paths (`:840`, `:873`, `:908`).
- `StructuredClipboard` (`:3534`).

**Size:** L. ⚠️ Needs a **rig live pass**.

---

## What reading the code found (2026-10-13)

✅ **The core is done.** `scrivi_fragment_extract`, `_paste` and `_uncut_paste` (`scrivi.h:916–1009`) are built and tested since EP-029.
- Linux bridges all four `scrivi_fragment_*` endpoints (`ScriviBridge.cpp:1180–1238`) and calls none.
- The undo engine already steps across a structural node and returns `structuralInverse` (`HistoryService.cpp:375`).

| Need | Apple | Linux today |
| ---- | ----- | ----------- |
| Cross-scene copy keeps the structure | `fragmentExtract` → balanced pieces. The fragment is held internally; the pasteboard gets the presented pieces joined by a blank line | ⛔ `balancedCopyOf` (`ManuscriptEditor.cpp:779`) copies the gap between scenes **as it is**, chapter heading included, into one flat string → **finding 2** |
| Where the fragment is held | `StructuredClipboard`, one per window, valid while the pasteboard's change count is the copy's ([I-0279]) | ⛔ nothing → **Q1** |
| Paste rebuilds the scenes | `pasteStructuredFragment`: commit pending typing → `balancePastePieces` against the caret's style → `fragmentPaste` → a `structuredPaste` structural node → reload from disk, caret at the split | ⛔ `insertFromMimeData` (`:832`) refuses a target that touches a boundary, then pastes the flat string into ONE scene |
| Cross-scene cut | the cross-scene COPY + `deleteAcrossScenes` (keeps the scenes, [I-0270]) | ✅ the delete half is done (`cutSelection`, `:923` → `deleteAcrossScenes`, `:1010`); only the copy half changes |
| Undo / redo of a structural node | `applyStructuralInverse`: two disk ops cover four directions (re-paste ↔ uncut), with a session cache of what a re-paste created | ⛔ [SP-178] Q2's stopgap (`EditorShell.cpp:2453`): it steps back over the node and says *"Undo it on the Mac for now"* |
| Copy buffers | a cross-scene ⌘N / ⌥N stores `{text, fragment}`; ⌃⌘N on a structured slot runs `pasteStructuredFragment` | ⛔ [SP-179] Q1's stopgap: flat text in, flat text out, and a once-per-project notice for a slot written on the Mac (`EditorShell.cpp:2419`) |
| Paste balancing | `scrivi_markdown_style_at_caret` + `scrivi_markdown_balance_paste_pieces` | ⛔ not bridged (Linux binds `balanced_edit` and `balanced_copy` only) |
| Reload after a structural op | `reloadManuscriptFromDisk(caretSceneID:caretByteOffset:)` | ✅ the split path already reloads (`EditorShell::load`, `EditorShell.cpp:1942`), and a structural reload keeps the open history |

⚠️ **Findings from reading the code:**
1. **A gap of the [I-0287] kind may exist after a structured paste.** Read, not run, so it is not filed yet.
   - Since [I-0287], a HARD barrier resets the core's cached text for the scenes it rewrote (`sceneTexts`).
   - A STRUCTURAL node (`structuredPaste`) does not: `HistoryService.cpp:303` applies `sceneTexts` only when `kind == Barrier`, and neither platform passes them.
   - Consequence, if the target scene had an edit earlier in the session (so it is already seeded): after the paste, the next edit there is diffed against the pre-paste text. Undoing that edit would put the pre-paste text back while the created scenes still exist, so the text after the caret would appear twice. A later undo of the paste itself would then fail, because `uncut_paste` checks the target body.
   - It would affect Apple too, since both platforms call the same core path. → **Q3**
2. **Linux's cross-scene clipboard text holds the chapter heading.**
   - Selecting from Chapter 1 into Chapter 2 and copying puts *"…end of scene\n\nChapter 2\nStart of…"* on the clipboard, and an in-app paste drops that heading in as prose.
   - Apple puts the pieces joined by a blank line, with no heading (`balancedFragment`).
   - Once structured paste exists, the flat text is what other apps get, and what Scrivi falls back to once the fragment has expired. → **Q4**
3. **Apple's structured paste ignores a selection.** `pasteStructuredFragment` takes `selectedRange().location` and never removes the selected text, so the paste lands BEFORE the selection and the selection stays. A flat ⌘V replaces it. Read, not run. → **Q2**
4. **Old `structuredCut` nodes.** A history made on the Mac before 2026-10-02 can hold the collapsing cut. Apple still replays them (undo = re-paste the fragment; redo = uncut what that re-paste created). The same code path covers it on Linux, so it is in scope, not a question.

## Goal

✅ **Apple's structured cut/copy/paste on Linux:**
- A selection across scenes or chapters copies (Ctrl+C) or cuts (Ctrl+X) as a fragment.
- Ctrl+V rebuilds those scenes and chapters at the caret, with their titles, as ONE undo step that Ctrl+Z reverses and Ctrl+Shift+Z re-applies.
- The copy buffers hold fragments, and a slot filled on the Mac pastes with its scene breaks.
- A structural step recorded on either platform undoes on the other.

## Plan

0. **Q3 first, if ruled in.** Prove finding 1 through `scrivi_*` with a failing test before any fix (as [I-0287] was proven). File it, fix it in the core, and re-point both platforms. If the test does not fail, record that and drop it.
1. **Bridge** `scrivi_markdown_style_at_caret` and `scrivi_markdown_balance_paste_pieces`. They are static, like S3b's markdown calls. Add them to `bridge_parity_smoke`.
2. **The fragment model on Linux.** This is a small value type in `platforms/linux/src/`: pieces (`opensWith`, `chapterTitle`, `sceneTitle`, `text`, `partial`) and `plainText`, with JSON in and out. It keeps every field the core returns, because a fragment written on the Mac is read here.
   - The **spans** come from `SceneDocument`'s segments: scene-local UTF-16 offsets are converted to UTF-8 bytes, and a zero-length span is added where the selection crosses a scene start (Apple's `fragmentSpans`).
   - Each piece is balanced with `markdownBalancedCopy`, as `balancedFragment` does.
3. **Cross-scene Ctrl+C** (`createMimeDataFromSelection`): extract and balance the fragment, write the flat text per Q4, and hold the fragment per Q1. A copy inside one scene is unchanged (AC5 of EP-029: the fast path stays).
4. **Ctrl+V** (`insertFromMimeData`): while a held fragment is valid, paste it structurally; otherwise the flat path runs as today. The structured paste:
   - commits pending typing (`HistoryCapture::flush`) and handles a selection per Q2;
   - balances the pieces against the caret's style;
   - calls `fragmentPaste` at (target scene, UTF-8 caret);
   - records a `structuredPaste` structural node with **Apple's payload, field for field** (`HistoryStructuralPayload`: `op`, `fragmentJSON`, `caretSceneID`, `caretByte`, `createdSceneIDs`, `createdChapterIDs`), so the Mac can undo it;
   - reloads, with the caret at the split point.
   - `HistoryCapture::recordBarrier` gains the optional payload.
5. **Cross-scene Ctrl+X:** steps 3 and 4's copy, then the existing `deleteAcrossScenes`. No new structural code.
6. **Undo and redo of a structural node:** this replaces SP-178 Q2's stopgap with Apple's `applyStructuralInverse`.
   - A re-paste (undo of a cut, redo of a paste) calls `fragmentPaste`. An uncut (undo of a paste, redo of a cut) calls `fragmentUncutPaste`.
   - A per-project cache records what each re-paste created, so the next uncut removes those scenes.
   - Then reload. A failed replay says so and leaves history where the core put it, as Apple's does. Q2's notice text is removed.
7. **Buffers:**
   - A cross-scene Ctrl+N or Alt+N stores `{plainText, fragment}` (`buffersLoad` already takes `fragmentJson`).
   - Ctrl+Shift+N on a structured slot runs step 4's paste.
   - `BufferService` keeps the fragment, not only the `structured` flag.
   - SP-179's Q1 notice and its once-per-project flag are removed.
8. **Tests.** These are offscreen smokes with keys sent through `QTest` into the real widget and the result read FROM DISK. A new `structured_paste_smoke` covers:
   - copy across scenes and across a chapter, then paste mid-scene: scene count, scene and chapter titles, each scene's text, and the caret;
   - the [I-0270] cut, then paste;
   - Ctrl+Z (the created scenes are gone and the target is restored), then Ctrl+Shift+Z, then Ctrl+Z again;
   - an expired fragment (another copy since), which pastes flat;
   - a structured slot, and a structured slot written by the Mac (a hand-built `buffers.json`);
   - a `structuredPaste` and a `structuredCut` node written the Mac's way, undone and redone on Linux;
   - a Linux `structuredPaste` node read back through `scrivi_history_*` with Apple's payload keys;
   - balancing (copying out of bold, pasting into bold).
   - Mutation-checked. ctest on macOS and on Linux (Docker, non-root). Run [SP-178]'s `undo_smoke` and [SP-179]'s `buffers_smoke` again.
   - ⚠️ **The reload is timed** on the 1,186-scene fixture, `[SCRIVI-STRUCT]`-style. Every structured paste and every structural undo reloads. The split already pays this cost, but this Sprint makes it a clipboard action.
9. **Rig live pass** (user). The steps go in the chat reply, and the build is checked first (`--version`). It can share a session with [SP-178]'s and [SP-179]'s passes.

## Questions — ✅ all ruled 2026-10-13

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where is the held fragment kept, and when does it expire?** Apple keeps it per window and drops it once the pasteboard's change count moves ([I-0279]). T2=A rules that the system clipboard carries only flat text, with no structured paste across projects in v1. | ✅ **In memory, in the editor (per project window, as Apple's is per coordinator), tied to ITS clipboard write:** Ctrl+C stamps a copy token into the mime data next to the existing `application/x-scrivi-manuscript-source`. Ctrl+V uses the fragment only when the clipboard still carries that token, which is Qt's equivalent of the change count. Another copy, in Scrivi or in any other app, expires it. A copy in project A pasted in project B pastes flat, as on the Mac. ⛔ The fragment JSON is never put on the system clipboard (T2=A) | ✅ **As recommended** (user, 2026-10-13) |
| **Q2** | **Structured paste with a selection (finding 3).** On the Mac, by reading, it inserts before the selection and leaves the selection in place; a flat paste replaces it. | ✅ **Replace the selection, on both platforms.** Remove the selected text first (a delete or grouped delete, as [I-0270]), then paste the fragment at its start. That is **two** undo steps, the removal and then the paste. **Prove it on the Mac first** with a failing interop test, then file an `[Apple]` Issue and fix it here as step 0 ([I-0289]'s pattern). Alternative: mirror the Mac as it is, and file the Issue for later | ✅ **As recommended** (user, 2026-10-13): replace the selection on both platforms, proven on the Mac first |
| **Q3** | **Finding 1, the [I-0287]-kind gap after a structured paste.** Read, not proven. | ✅ **Prove it through `scrivi_*` first (plan step 0).** If it fails, file a `[ScriviCore]` Issue and fix it in this Sprint for both platforms, since Linux would ship the defect on day one otherwise. Design after the proof: the likely shape is that a structural node also carries the texts its op left, and the undo and redo replays account for them. ⚠️ `scrivi.history.v1` would gain an optional field again, which is backward-compatible and surfaced here rather than slipped in | ✅ **PROVEN 2026-10-13** (user: *"not sure this is an issue. Prove it."*) → **[I-0293]**, High. ✅ **Fixed here, step 0, both platforms** (user: *"yes, as step 0 for both platforms."*) |
| **Q4** | **Linux's flat cross-scene text (finding 2).** It holds the chapter heading and the scene gaps as they are. | ✅ **Apple's: the balanced pieces joined by a blank line, no heading**, for other apps (presented) and for Scrivi's own flat fallback (source). Changes visible Linux behaviour: a pasted cross-chapter copy no longer drops "Chapter 2" into the prose | ✅ **As recommended** (user, 2026-10-13) |

⛔ **NOT in this Sprint:**
- the fork popover, History card and Undo History settings (S3);
- Replace / Replace All ([EP-048] S4b);
- the collapsing cut (§4.3/§4.4, replaced by [I-0270]): Linux only replays old `structuredCut` nodes, never records one;
- structured paste across projects or apps (T2=A);
- Windows;
- any change to `scrivi.fragment.v1` or `scrivi.buffers.v1`;
- any `[Apple]` or `[ScriviCore]` change beyond what Q2 and Q3 rule in.

## Acceptance Criteria

- [x] AC0 — Q3 / [I-0293]: proven, then fixed on both platforms, each red before. Core: `CaptureCApiTests` `[I-0293]` ×2, 4 mutations caught. Apple: `structuredPasteThenUndo`. Linux: `structured_paste_smoke` §2 (and its `no-scene-texts` mutation caught)
- [x] AC0b — Q2 / [I-0295]: a structured paste replaces the selection, the removal its own step. Mac: `structuredPasteReplacesTheSelection`, red first. Linux: §3
- [x] AC0c — [I-0294]: a structured paste first saves every scene with unsaved typing. Mac: `structuredPasteKeepsUnsavedTyping`, red first. Linux: §2 (`no-save-before-paste` caught)
- [x] AC0d — [I-0296]: the next open takes no pasted, uncut or cut scene for an outside edit. Core fix: `CaptureCApiTests` `[I-0296]`, red first, 3 mutations caught. Mac `structuredPasteThenReopen` (red, then green with no Mac change). Linux §7
- [x] AC1 — Ctrl+C across scenes or chapters holds a balanced fragment; the clipboard's text has no heading (Q4) — §1, §2 (`no-cross-copy`, `flat-clipboard-text` caught)
- [x] AC2 — Ctrl+V rebuilds every carried scene and chapter at the caret, the chapter with its title, balanced against the caret — §1, §2, §4b (`no-paste-balance` caught); on disk
- [x] AC3 — ONE undo step: Ctrl+Z removes what the paste created and restores the target, and Ctrl+Shift+Z re-applies it, repeatably — §1 (`no-structural-replay`, `no-redo-cache` caught)
- [x] AC4 — Ctrl+X across scenes = AC1's copy + [I-0270]'s delete — `cutSelection` → `deleteAcrossScenes` → `copy()` → `crossSceneCopy` (the buffer cut, §5, runs the same path through Alt+3)
- [x] AC5 — An expired fragment pastes flat: another app's copy, and another Scrivi window's copy (its own token) — §4 (`no-token-check` caught)
- [x] AC6 — Buffers: Ctrl+1 / Alt+3 across scenes store the fragment; Ctrl+Shift+N rebuilds it; a slot the Mac wrote rebuilds; [SP-179] Q1's notice gone — §5 (`flat-slot-copy`, `flat-slot-paste` caught)
- [x] AC7 — A `structuredPaste` and an old collapsing `structuredCut` recorded on the Mac undo and redo on Linux; a Linux node carries Apple's payload, key for key; [SP-178] Q2's stopgap gone — §6, §7
- [x] AC8 — Smokes through the real key dispatch, 11 of 11 Linux mutations caught; `bridge_parity_smoke` covers the 2 new calls; on 1,186 scenes a paste + reload 436 ms, its undo + reload 393 ms (Docker, Release; §8, opt-in); ctest macOS 721/721, Linux 724/724; 32/32 Linux smokes; interop 288/288; `undo_smoke` and `buffers_smoke` green
- [ ] AC9 — ✅ **Rig live pass (user)**

---

## Progress log

### 🔵 2026-10-13 — CREATED in Planning

Read Apple's current shape (not the 2026-07 design's collapsing cut, which [I-0270] replaced), Linux's clipboard, cut and
undo paths, and the core's structural-node handling. Four questions are open. Nothing is implemented.

### 🔵 2026-10-13 — Q1, Q2, Q4 ruled; Q3 PROVEN → [I-0293]

User: *"Q1: The recommended option. Q2: recommended. Q3: not sure this is an issue. Prove it. Q4: recommended."*

**Q3 proof:** a scratch program, `scrivi_*` only, on a real project created through the ABI, linked against the current
`libScriviCore.a` (no ScriviCore source newer than it). It runs Apple's call order (`capture_flush` → `fragment_paste` →
`capture_record_barrier` + `structuredPaste` payload).
- **Target edited earlier in the session:** the next edit's undo returned `"Alpha. Bravo."` instead of `"Alpha. one"`, so
  "Bravo." is now also in the created `"threeBravo."`. The structural undo was then REFUSED by `fragment_uncut_paste`, and
  the two created scenes stayed.
- **Control (not edited before the paste):** both undos were correct.

Filed as [I-0293] (`[ScriviCore]`, High). The program is not in the repo; step 0 turns it into a `CaptureCApiTests` case that
fails before the fix.

### 🟡 2026-10-13 — ACTIVATED; step 0 ([I-0293]) — core and Apple done

User: *"yes, as step 0 for both platforms."*

- **Core:** a structural node carries `sceneTexts` (after the op) and `sceneTextsBefore` (each existing scene before it).
  - Record and redo apply the first; undo applies the second; every head rebuild reads the first.
  - A scene entering history there takes its before-text as its floor.
  - Both fields are persisted in the node's record and written only when present.
  - ABI: optional `sceneTextsBefore` on both barrier endpoints.
- **Tests:** 2 new `[I-0293]` cases in `CaptureCApiTests`, red first; 4 mutations each caught.
  - ⚠️ **First mutation round invalid.** Its revert-and-restore left stale objects, so the tests crashed rather than failed.
  - ⚠️ **Make missed a restored file.** It landed in the same second as the mutated object's compile, so make skipped it.
  - ✅ Re-run on a forced rebuild, each mutation failing on an assertion.
  - ⚠️ **Crashed runs left folders behind.** They left `scrivi-capture-capi-*` temp folders that a later run reopened, because the path is counter + stack address. The test helper now clears the path before opening it.
- **Apple:** `pasteStructuredFragment` sends the texts (read back after the paste) and the target's before-text. New interop test `structuredPasteThenUndo`: red without the texts. Interop 285/285.
- **Linux:** nothing to wire until step 4, its first structured paste.
- **Results:** ctest macOS 720/720; Linux (Docker, clean Release, uid 1000) 723/723; 31/31 smokes; no warnings.
- ⚠️ **Found: [I-0294]** `[Apple]`. Writing the interop test showed that a structured paste builds on the scene as SAVED: `" Pre"` typed just before the paste was dropped by the reload. The test saves first, as the autosave would, so it isolates [I-0293]. ⏳ Awaits a ruling. Linux's step 4 must save first regardless, as its split does.

### 🟡 2026-10-13 — [I-0294] fixed (Apple)

User: *"yes I-0294 in SP-180."*

- **Red first:** the new interop test types into two scenes, saves neither, and pastes. Both scenes' typing was lost, the other scene's too, because the reload replaces every scene with disk.
- **Fix:** `pasteStructuredFragment` saves every dirty scene before `fragmentPaste`.
- **Result:** interop 286/286.

### 🟡 2026-10-13 — Linux steps 1–8 built; [I-0295] fixed (Apple); [I-0296] found

User: *"yes"* (proceed with steps 1–8).

- **Apple, Q2 first ([I-0295]):** `structuredPasteReplacesTheSelection` was red (the paste landed before the selection and
  left it). Fixed: the selection is removed as its own step, then the fragment lands. Interop 287/287.
- **Linux, built:**
  - **Bridge:** `markdownStyleAtCaret` and `markdownBalancePastePieces`.
  - **`ManuscriptEditor`:** builds the spans and balances each piece (`crossSceneCopy`). A cross-scene copy holds the
    fragment, with a token on the clipboard (Q1); the clipboard's text has no headings (Q4). `pasteClipboard` sends Ctrl+V
    and Edit ▸ Paste to a structured paste while the token is still on the clipboard; drops stay flat.
    `removeSelectionForPaste` handles Q2.
  - **`EditorShell`:** `pasteFragment` removes the selection, commits pending history, saves dirty scenes ([I-0294]),
    balances against the caret, pastes, records a structural node with Apple's payload and both texts ([I-0293]), then
    reloads with the caret at the split. `applyStructuralInverse` replaces [SP-178] Q2's stopgap.
  - **Buffers:** slots hold fragments, and [SP-179] Q1's notice is gone (`buffers_smoke` §8 retired; `structured_paste_smoke`
    §5 covers it).
  - **`HistoryCapture::recordBarrier`:** takes the payload and the before-texts.
  - **Tests and CI:** the new `structured_paste_smoke` (§1–§7) is in CMake and the Linux CI.
- **Docker (clean Release, uid 1000):** ctest 723/723; 31 of 32 smokes. Paste plus reload: 12 ms on 5 scenes.
  - ⚠️ The first run reported 29.5 s. That was my test's wait: the reload finished inside the key helper's own wait, so the
    loop ran to its timeout. Fixed.
- ⛔ **`structured_paste_smoke` §7 fails: [I-0296].** After a structured paste, a reopen takes the target scene for an
  outside edit, because the core wrote it and nobody reported the bytes. Proven on the Mac as well
  (`structuredPasteThenReopen`, red). ⏳ Awaits a ruling. The recommendation is the core fix.
- ⏳ **Still to do:** the 1,186-scene reload timing; mutation checks; and the fix for [I-0296].

### 🟠 2026-10-13 — [I-0296] fixed in the core; Linux complete; ⏳ AC9 rig live pass

User: *"yes fix it in SP-180."*

- **[I-0296], the core fix:** `scrivi_fragment_paste`, `_uncut_paste` and `_cut` record the bytes they write while the
  project's history is open (`noteScenesWritten`). New `CaptureCApiTests` `[I-0296]` case: red first, then each of the three
  calls removed in turn is caught. Mac `structuredPasteThenReopen` passes with no Mac change; Linux §7 passes.
  - `_cut` was found by §7's simulated old Mac cut; it is the same unreported write, no longer called by either platform.
- **Mutations: 11 run on Linux, 9 caught at first.** The 2 survivors were gaps in the smoke, not the code:
  - Another window's token: §4 now covers a Scrivi copy from another window.
  - Pasting inside bold: new §4b.
  - Both are caught now: 11 of 11.
- **Timing, 1,186 scenes (§8, opt-in `SCRIVI_MEASURE_LARGE`):** built in 1.7 s; first open 349 ms; paste + reload 436 ms;
  structural undo + reload 393 ms (Docker, Release, on the Mac). ⚠️ The rig will be slower; its live pass measures it.
- **Final runs:** ctest macOS 721/721, Linux 724/724 (uid 1000); 32/32 Linux smokes, no warnings; interop 288/288.
- ⏳ **Next: AC9, the rig live pass.** It can share a session with [SP-178]'s and [SP-179]'s passes. Bump the build stamp
  before that push.

### 🟡 2026-10-13 — Rig live pass (build 73): steps 24–34 and 38 PASS; 36, 37 and 39 FAIL → [I-0297]–[I-0300]

The user ran SP-178, SP-179 and SP-180 together.
- **Steps 24–34 pass** (`Live Pass 73b`, local disk): user, *"steps 24 to 34 passed"*. These cover the paste, the
  cross-chapter paste with its title, the I-0293/I-0294/I-0295 checks, an expired copy, buffers, the cut, and the reopen.
- **One observation in step 24:** after Ctrl+Shift+Return the caret stayed in Chapter 1 → [I-0300].
- **Steps 35–39** (dumas-prose-timelines over the share, Mac ↔ Linux):
  - Opening took about 4–5 s.
  - ⛔ The undo of the Mac's paste froze the UI about 90 s; Ctrl+Shift+4 froze about 2 min and its undo about 1.5 min → [I-0297].
  - ⛔ No "previous session" warning on that undo → [I-0298].
  - ✅ The Mac's buffer pasted with its scene break.
  - ✅ Step 38's paste happened (the log, 21:17:29 UTC).
  - ⛔ Step 39: the Mac's reopen flagged scenes Linux's core had written, and two the Mac itself had → [I-0299].
- The extra history sessions in the log are the user's quits (user: *"I quit Scrivi more often than I should have"*), not a
  defect.
- ✅ **Ruled 2026-10-13** (user: *"We'll take your recommendations for 36, 37, 39 and 24. But do the new Issue now as well."*):
  all four are fixed in this Sprint; AC9 is re-run after them.

### 🟡 2026-10-13 — [I-0300], [I-0298] and [I-0297] fixed; [I-0299] probed

- **[I-0300]:** `reloadThen` places the caret, and runs the new chapter's renumber, once the asynchronous reload has landed.
  This covers the new chapter, the split and both merges. `undo_smoke` §7's step-24 case is red before the fix.
- **[I-0298]:** both platforms warn after a structural step that crosses into an earlier session.
  - Mac: `structuralUndoAcrossSessionsWarns`, red first.
  - Linux: §7, red first; the mutation is caught.
  - ⚠️ Both first tests passed the wrong way: an undo landing on the history's root, which belongs to no session, never warns.
    The tests now keep earlier work underneath, as on the rig.
- **[I-0297]:** measured with `strace` in Docker on 1,186 scenes. The paste's UI thread went from 7,683 file calls to 16, the
  undo's from 4,966 to 16.
  - ScriviCore returns what it wrote (`sceneTexts`), so neither the core nor the platforms look the scenes up again.
  - Linux runs the ScriviCore call off the UI thread (`runStructural`) behind the progress row, with the editor read-only
    until the reload lands.
  - About 4,200 of the 6,642 sidecar reads had come from SP-180's own code (the [I-0296] lookups and the read-back).
- **[I-0299]:** a Mac cross-scene delete with a clean close and reopen flags nothing.
  - Leading hypothesis: sessions that ended without closing their history (a force-quit, or Xcode stopping the app), which
    lose every recorded byte.
  - ⏳ Asked the user.
- **Runs:** ctest macOS 721/721, Linux 724/724 (uid 1000); 32/32 Linux smokes, no warnings; interop 290/290.

### 🟡 2026-10-13 (evening) — Rig re-run, build 74

- ✅ **[I-0300] passes** (user: step 5, *"yes the caret landed correctly"*).
- **[I-0297]:** the undo of the Mac's paste over the share completes, but *"takes about 40 seconds"* (the log: 03:12:59 →
  03:13:36 UTC). Down from about 90 s. ⏳ Whether that is acceptable is the user's call.
- **[I-0299]:** every quit was clean, and no new false flag appeared. The "changed outside Scrivi" stops were yesterday's
  barriers.
- **The resume position:** Linux reopens where Linux last was, not where the Mac was. ✅ By design: `workspace-state.json`
  lives outside the project, per device (Architecture v0.3, "confirmed correct"). Not a defect.
- **Step 11** was recorded as a plain (flat) paste. Either the copy did not cross a scene break, or the structure had expired.
  ⏳ Asked the user.
- **New:** [I-0301] `[Apple]`, scroll jitter at the manuscript's start (user report). ⏳ Which Sprint is to be ruled.

### 🟡 2026-10-14 — Rulings; [I-0299] proven; [I-0301] instrumented

User: *"1. yes, the window kept redrawing … We can accept it for now, but I reserve the right to revisit it … 2. Ok, you may
conduct your test. 3. Yes … we should fix it right now."*

- **[I-0297]:** about 40 s over the share, accepted for now. The user reserves the right to revisit it.
- **[I-0299] proven:** three processes through `scrivi_*`. A session that ends without closing its history loses the bytes it
  recorded, and the next open flags them; the control with a clean close does not. The fix (record each save's bytes in the
  log as it happens) is a `scrivi.history.v1` change. ⏳ It needs the user's ruling.
- **[I-0301] ruled into SP-180.** Reading did not single out a cause, and [I-0204]'s three wrong code-read diagnoses argue
  against guessing. `[SCRIVI-SCROLL]` diagnostics were added (marked to remove with the fix): the scroll wheel, every
  `scrollRangeToVisible` with its callers, the three explicit clip scrolls, the restore re-centre, and document-height changes
  or jumps. ⏳ The user reproduces it and pastes the console.

### 🟡 2026-10-14 — [I-0299] fixed; [I-0301] not reproduced

User: *"It didn't do it this time. Scrolled smoothly. I guess we'll need to keep an eye on it. yes, lets fix I-0299 in
SP-180."*

- **[I-0299]:** a `rec:"persisted"` log record per reported write (and per repair), replayed at open over `state.json`.
  - The new fork-based test was red first; 3 mutations are caught.
  - The I-0104 loop test now strips the records, so it still models a pre-fix log.
  - Design doc A.1.d added.
- **[I-0301]:** stays open. The `[SCRIVI-SCROLL]` diagnostics are now Debug-only, so the next occurrence is caught with
  evidence.
- **Runs:** ctest macOS 722/722, Linux 725/725 (uid 1000); 32/32 Linux smokes; interop 290/290.
  - Linux's only warnings are the pre-existing ones in the test mock `DeterministicUUIDProvider.hpp` (unchanged since
    `7d2bd5d`).

### 🟡 2026-10-14 — [I-0299] second half; [I-0301] → backlog; [I-0302] filed

User: *"detach I-0301 from SP-180 and move it to the backlog."*
- **[I-0301]** moved to `Issue-backlog.md`, unassigned.
- **The user's Mac check, first part:** the first ⌘Z only flashed. The history's newest step is a 14:56 `externalChange`
  barrier, from a session killed by an Xcode relaunch while it ran the pre-fix build. The Mac disables Undo at a barrier, so
  its notice never shows → **[I-0302]** `[Apple]`, filed; ⏳ ruling.
- **The user's Mac check, second part, plus a probe:** the first half of [I-0299] had opened a data-loss path. Pending
  typing at a crash was saved but never recorded, and undoing the next edit removed it.
  - Fixed per the user's ruling (*"record the session close as a historical event"*): `ctl:close` plus
    `closesRecorded`, and the saved-but-unrecorded difference recorded as a step after an unclean end.
  - Red first; 2 mutations caught; design doc A.1.c updated.
- **Runs:** ctest macOS 723/723, Linux 726/726; 32/32 Linux smokes; interop 290/290.

### 🟡 2026-10-15 — [I-0302] fixed

User: *"Ok fix it in SP-180."*

- ScriviCore's capture results carry `atBarrier`. The Mac's Edit ▸ Undo and Linux's menu item stay available there, so the
  undo shows the barrier's notice.
- Red first, three times: the core test, the Mac interop test through `validateUserInterfaceItem`, and Linux `undo_smoke`
  with the Linux change removed.
- **Runs:** ctest macOS 724/724, Linux 727/727; 32/32 Linux smokes; interop 291/291.

### 🟡 2026-10-15 — [I-0302] Verified (Mac)

User: *"I-0302 passes, the dialog appeared"* (⌘Z at dumas-prose-timelines' 14:56 barrier). Archived to
`Issues/Verified/Issue-verified-0301-0310.md` (new batch 31). Its Linux menu-item half rests on `undo_smoke`; not checked live.

### 🟡 2026-10-15 — [I-0293]–[I-0296], [I-0299] Verified (Mac); [I-0298] Mac half passed

User: *"I-0293 through I-0299 passes"* (the Mac steps). Five archived to `Issues/Verified/Issue-verified-0291-0300.md`.
- [I-0297] is Linux-only (its check is over the share on the rig), so it stays Resolved - Not Verified.
- [I-0298] covers both platforms. The Mac half passed; its Linux half (Ctrl+Z) waits for the rig.
- ⏳ AC9 on the rig: [I-0297], [I-0298] Linux, [I-0300], and the Linux side of [I-0293]–[I-0295].

### 🔵 2026-10-15 — Rig live pass (combined run sheet), steps 9–11; stopped for the day

- **Step 9** ⛔ → **[I-0307]** `[Linux]`: selecting across a scene break, Ctrl+C and Ctrl+V each froze the window 10–15 s on
  dumas over the share.
- **Step 10** ⛔ → **[I-0297] reopened.** User: *"I never approved a 40 seconds wait for anything. Ever. … Cut Copy and
  Paste must be as fast as possible at all times. I'll accept long load times, but once loaded the app must do everything
  instantly. Writers will demand it!"* (The 2026-10-14 record reads *"We can accept it for now, but I reserve the right to
  revisit it"*; it is revisited.)
- **Step 11:** ✅ Ctrl+Alt+F opens the bar with the Replace row (GNOME does not take it: [SP-181] AC4). ⛔ → **[I-0308]**
  `[Linux]`: the app crashed while typing in the Find bar.
- ⚠️ The binary reported build 74: the stamp bump never reached a commit. The rig was on `8ebb438`, the Mac's HEAD, built
  after the pull, so the code under test was current.
