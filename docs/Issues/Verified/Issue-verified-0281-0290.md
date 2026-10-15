# Verified Issues: I-0281 – I-0290

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0271–I-0280 are in [`Issue-verified-0271-0280.md`](Issue-verified-0271-0280.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0281** | `[Apple]` ⚠️ **A block whose FIRST line is indented 1–3 spaces mis-renders emphasis on every FOLLOWING line — Apple's parser reports their source columns shifted by that indent.** ✅ **MEASURED 2026-10-07 by [SP-165]'s L2 agreement test** (CLI harness compiling `MarkdownBlocks.swift` against `libScriviCore.a`, reduced to a minimal block): `" She said⏎*no* twice."` → Apple hides the OPENING `*`, shows the CLOSING `*` in italic and leaves `n` upright (styles `…1 0 1 1`); `"   Three⏎lines *of* it⏎and *more* here."` → line 2's emphasis lost, line 3 shifted by 3. ✅ md4c (the core, Linux) reports the true positions. ⚠️ Reachable from imported or hand-written text with a leading space and soft line breaks. ⏳ Fix not designed: `MarkdownBlocks`' `offset(line, column)` would need the paragraph's first-line indent per continuation line — ✅ the L2 test is the oracle (its `knownDisagreements` entry flips when fixed). ➡️ **Linked forward to [EP-047]** (user, 2026-10-07). ✅ **FIX 2026-10-07 ([SP-169]):** spiked Apple's convention — a continuation line's column is reported after stripping its own leading whitespace PLUS the first line's indent; `MarkdownBlocks.trueColumn` corrects it (plain top-level paragraphs). L2: the `knownDisagreements` entry REMOVED; four I-0281 blocks must now agree (they do; with the fix undone 3/50 disagree). ✅ **VERIFIED 2026-10-08 (user, SP-169 live pass step 6):** *"1. passes. 2. passes. 3. passes. 4. passes. 5. passes. 6. passes."* | Medium | ✅ [SP-169] | ✅ **Verified 2026-10-08** — archived with the SP-169 close |
| **I-0282** | `[Apple]` ⚠️ **Changing the manuscript TYPEFACE or SIZE loses the writer's place — the caret and the reading position jump.** ✅ **REPORTED BY THE USER 2026-10-07** ([SP-168] live pass, step 4): *"when changing font or font size, the position of the caret is not maintained. This means that If I select a \"wider\" font my manuscript position will shift from say, 53 to say, 35. Same if the font size changes"* ✅ **CAUSE (read):** a typography change re-presents through `updateNSView` → plain `rebuildStorage`, which keeps neither the selection nor the place; the scroll keeps its PIXEL offset, and the same text is now taller or shorter, so the pixels land on different text. (Structural edits restore the caret through `revealCaretAfterRebuild`; this path never did.) ✅ **FIX:** `Coordinator.rebuildKeepingReadingPosition` — anchors on a CHARACTER (the caret's LINE when it is on screen, else the character at the top of the visible area), keeps the selection, and puts that line back at the same height, SETTLING over several passes (TextKit 2 gives unlaid paragraphs estimated heights; one pass was measured 170 pt off). Line positions come from TextKit 2 directly (`lineRect(forCharacterIndex:)`), not `firstRect`, which needs an on-screen window. ✅ 2 interop tests (caret on screen across Courier Prime 24 / Inter 11; scrolled away), mutation-checked (2/2); interop 226/226. ⛔ **LIVE RE-CHECK FAILED (user, 2026-10-07):** *"Using Literata-20 … caret at the blank line … in Chapter 51 … selected Courier Prime-16 … The viewport has moved to somewhere in the first three of four scenes of Chapter 25 (its slightly different each time). The caret is at the same spot. But it sometimes takes two arrow moves to get the display to show it."* ⚠️ **Not reproduced by the tests** — not at 1.8 MB, not with 1,200 scenes, dividers and titles, two changes: the harness gives the text view a FIXED frame. ✅ **CAUSE (read):** the fix computed a PIXEL target and clamped it to `tv.bounds.height`; right after a rebuild the app's text-view FRAME still holds TextKit 2's provisional height, so the clamp landed mid-document. ✅ **FIX 2:** the pattern already proven in the app (T-0573 `revealCaretAfterRebuild`) — `scrollRangeToVisible(anchor)` FIRST (AppKit sizes the document), then correct by the small delta; clamp against the scroll's document view. `[SCRIVI-TYPE] keep place: anchor=… wanted=… landed=…` is logged on every change so the next live run proves itself. 4 placement tests pass (incl. 1.8 MB and a 1,200-scene manuscript); interop 228/228. ⛔ **LIVE RE-CHECK 2 FAILED (user, 2026-10-07):** *"The view jumped again. a lot. Just as before. the caret is never off the viewport"* — log: `anchor?: caret=1846568 caretLineY=624533.0 clipY=591857.6 clipH=782.0` · `anchor = TOP OF VIEW (index 898922)` · `keep place: anchor=898922 wanted=-36.0 landed=-36.0`. ✅ **ROOT CAUSE (from the log):** the caret WAS on screen, but `lineRect` (via `ensureLayout`) put its line at ≈1.22 M pt — PAST the document's end (docH ≈1.18 M) — while the viewport drew Chapter 51 at ≈0.59 M: TextKit 2 positions the VIEWPORT's paragraphs on ESTIMATED heights above them, and `ensureLayout` returns TRUE positions; the two frames disagree. So the caret was judged off screen, and the top-of-view fallback (`characterIndexForInsertion`) mis-read the same way — index 898922, Chapter 25 — and FAITHFULLY kept Chapter 25 at the top. ✅ **FIX 3:** measure ONLY what the viewport has DRAWN — `lineRect` returns a line only inside `viewportRange` (no `ensureLayout`); the top of the view is the first drawn line (`firstVisibleLine`); after the rebuild, `scrollRangeToVisible` then lay out AGAIN until the anchor is drawn (measured: the first `layoutViewport` after the scroll has not yet drawn it), then correct the delta — converges in 2–3 passes in every test. ⚠️ **HONEST LIMIT:** no test reproduces the two-frame disagreement — a new 'read around a long manuscript' test passes even with the OLD measurement restored (checked). The fix rests on the user's log; the per-pass `[SCRIVI-TYPE]` lines stay until the live re-check passes. Interop 229/229. ⛔ **LIVE RE-CHECK 3 FAILED (user, 2026-10-07):** *"Still Jumps."* — log: `anchor?: caret=1846568 caretLineY=nan clipY=825947.2` · `anchor = TOP OF VIEW (index 833266)`. ✅ **THE ACTUAL ROOT CAUSE:** `updateNSView` set `presenter.typography` AND **`tv.font`** BEFORE calling the place-keeping rebuild — an `NSTextView` `font` assignment re-fonts the WHOLE document and invalidates every line position, so the writer's place was read from an invalid layout (NaN), and the fallback read garbage too. ⚠️ The tests called the rebuild DIRECTLY, never through `updateNSView`'s order — which is why three rounds of fixes passed every test and failed live. ✅ **FIX 4:** ONE coordinator method, `applyTypography(_:to:segments:)`, does the whole sequence — read the place FIRST, then change the type (presenter + `typingAttributes` only; ⛔ no `tv.font`: the rebuild writes every attribute, and `font` also overwrote the chapter titles' font), rebuild, restore; `updateNSView` calls it, and so do all five placement tests. ✅ **REPRODUCED:** with the old order restored, the tests log `caretLineY=nan` — the user's exact symptom — and fail (7 issues); with the fix they pass. (Fixes 2 and 3 stay: measuring only what is DRAWN is still right — the log's ≈1.22 M pt reading came from the same invalid layout.) Interop 229/229. ✅ **VERIFIED 2026-10-07 (user, live re-check 4):** *"works perfectly. Both for font changes and size changes."* The `[SCRIVI-TYPE]` diagnostics were removed after it (interop 229/229). | Medium | ✅ [SP-168] | ✅ **Verified 2026-10-07** — archived with the SP-168 close |
| **I-0283** | `[Apple]` ⚠️ **macOS SMART QUOTES rewrite the stored text OUTSIDE the escape layer — inconsistently: typed `'''` ends up `’\'’` (curly · straight · curly) and the caret jumps.** ✅ **REPORTED BY THE USER 2026-10-08** (during the I-0280 check): *"When I type ''' the displayed text shows <single-quote>'<single-quote><newline><single-quote> (where <single-quote> is a comma glyph displayed in the superscript position and ' is the \"tick\" glyph which are different in Literata). Also the caret moves to just after the second <single-quote>."* ✅ **REPRODUCED 2026-10-08** (interop harness, Smart Quotes on, three `'` keystrokes through the real `keyDown`): stored `Before \'\'\'` (each escaped, as designed), caret 13 → after AppKit's text-checking pass: stored **`Before ’\'’`**, caret **11** — AppKit replaced the OUTER two `\'` (backslash + quote) with `’`, DELETING their escape backslashes, and left the middle one straight and escaped. ⚠️ The `<newline>` the user saw was NOT reproduced (possibly a Return). ✅ **CAUSE (read):** Scrivi never sets `isAutomaticQuoteSubstitutionEnabled`, so the manuscript inherits macOS's Edit ▸ Substitutions ▸ Smart Quotes (normally ON); AppKit substitutes asynchronously over STORAGE ranges and does not know a backslash is part of an escape. ⏳ **RULING OWED:** (a) turn Smart Quotes OFF in the manuscript (a typed `'` is stored `\'` and shown straight); (b) Scrivi's OWN smart quotes — the typing path inserts `‘ ’ “ ”` by context (they are not Markdown punctuation, so need no escape) and AppKit's pass is turned off; (c) keep AppKit's and teach the escape layer to accept its replacements. ⚠️ Not EP-047's; the escape layer is EP-045/EP-049's, so Linux needs the same ruling (EP-048). ✅ **RULED 2026-10-08 (user): (a) — *"a for I-0283. Fix now."*** ✅ **FIX 2026-10-08** (outside a Sprint, at the user's instruction): `ManuscriptNSTextView` overrides `isAutomaticQuoteSubstitutionEnabled` to ALWAYS false (the Edit ▸ Substitutions toggle is ignored) and disables that menu item for the manuscript. ✅ **Measured on the way:** a typed `"hi"` was WORSE — it became `”hi\"”` (a quote ADDED, caret to 8); ✅ Smart DASHES need nothing (`--` is stored `\-\-`, which AppKit never sees as two hyphens). ✅ 2 interop tests through the real `keyDown` + AppKit's own checking pass, with Smart Quotes switched ON; with the override removed they fail with the user's exact text (`’\'’`, `”hi\"”`). Interop 245/245. ✅ **Linux:** already as ruled — Qt's `QPlainTextEdit` has no automatic quote substitution. ✅ **VERIFIED 2026-10-08 (user, live check):** *"Live check 1. passes."* (Step 2 moot: the Edit menu has no Substitutions item — *"I don't want one either"*; the validation still disables Smart Quotes in the text view's right-click Substitutions menu.) | Medium | — (fixed outside a Sprint) | ✅ **Verified 2026-10-08** — archived |
| **I-0285** | `[Linux]` ⚠️ **Opening dumas takes ~3 minutes and FREEZES the UI for ~55 s: the timeline is built TWICE on the main thread, each build asking for every scene's story time one call at a time.** ✅ **MEASURED 2026-10-09** on the rig (build 62, `SCRIVI_LOAD_LOG`), found in [SP-166]. Linked to [EP-048]. → [details](#i-0285) | High | [SP-173] | ✅ Verified 2026-10-10 |
| **I-0287** | `[ScriviCore]` ⛔ **After a scene MERGE or SPLIT, undoing the next edit in that scene restored its PRE-structural text — a merge's joined-in text DROPPED, a split's moved tail DUPLICATED.** Found 2026-10-11 by [SP-178]'s `undo_smoke`; proven through `scrivi_*` alone. ✅ Fixed: a barrier carries the scenes' new texts; both platforms record it after the op. → [details](#i-0287) | **High** | [SP-178] | ✅ **VERIFIED 2026-10-15** |
| **I-0288** | `[ScriviCore]` ⛔ **After an outside edit, undo in a LATER session restored a MIX of the old and new text — the `externalChange` repair re-floored the scene and the next open replayed the pre-change diffs onto it.** Measured 2026-10-12 through `scrivi_*`. ✅ Fixed: the repair's barrier carries the disk text; old logs' repair floors are read as that. → [details](#i-0288) | **High** | [SP-178] | ✅ **VERIFIED 2026-10-12** (user: *"mark I-0288 as verified"*) |
| **I-0289** | `[Apple]` ⛔ **Copy buffers 1–9 skipped the emphasis balancing ⌘C / ⌘X / ⌘V do: ⌘N / ⌥N stored raw storage (one scene and across scenes), and ⌃N pasted with the caret's style instead of its own.** Found 2026-10-12 planning [SP-179]; measured by 4 interop tests that failed. ✅ Fixed: the buffers reuse ⌘C's balanced copy and ⌘V's own-copy paste. → [details](#i-0289) | **Medium** | [SP-179] | ✅ **VERIFIED 2026-10-12** |
| **I-0290** | `[Apple]` ⛔ **Buffer paste ⌃1–9 collides with macOS Mission Control "Switch to Desktop N": ⌃1 never reaches Scrivi.** Found 2026-10-12 on the user's Mac during [I-0289]'s check. ✅ Fixed: paste moved to ⌃⌘1–9 (user: *"Option B"*). → [details](#i-0290) | **Medium** | [SP-179] | ✅ **VERIFIED 2026-10-12** |

---

## I-0290

**Title:** Buffer paste ⌃1–9 collides with macOS Mission Control "Switch to Desktop N"
**Status:** ✅ **VERIFIED 2026-10-12** (user, Mac: *"I-0290 passes."*). The palette's button still followed ⌃ alone → [I-0291].
**Platform:** `[Apple]`
**Component:** `ManuscriptNSTextView.keyDown` buffer chords; Edit ▸ Paste From Buffer label; `BuffersPalette` hint and help
**Severity:** Medium — paste from slot 1 by keyboard never works while the Mission Control shortcut is on; slots N whose desktop
exists are taken too (inferred from ⌃2 getting through, not measured). The palette and the Edit menu still paste.
**Sprint:** [SP-179] — ✅ ruled 2026-10-12 (user: *"Option B, fix it in SP-179."*)
**Date Identified:** 2026-10-12 — [I-0289]'s Mac check, step 2 (user: *"Ctrl-1 did not paste at all"*)

**Evidence:** `[SCRIVI-EDIT] keyDown` logged ONE "1" key (15:06:25, the ⌘1 copy) and none for ⌃1. On that Mac
`com.apple.symbolichotkeys` 118–126 (Switch to Desktop 1–9) are enabled on ⌃1–⌃9 (mask `0x40000`). The chord dates from SP-056;
no earlier record (`Issues/`, `Closed/`, `Verified/`, `Tasks/` searched).

**Fix (Option B):** paste is ⌃⌘1–9. Checked free first: no enabled macOS symbolic hotkey uses ⌃⌘ + digit on that Mac (⌃ alone =
desktops, ⌥⌘8 = Zoom, ⇧⌘3/4 screenshots), and no Scrivi menu item does (Scrivi uses ⌥⌘1–3/0 and ⇧⌘0/1).
- `ManuscriptTextView.swift` `keyDown`: the paste branch requires ⌃ AND ⌘. ⌃N alone now falls through to AppKit (measured: no text
  change).
- Labels: Edit ▸ Paste From Buffer header, palette hint and help text, comments in `BufferService.swift`, `ScriviApp.swift`.
- The palette's paste BUTTON still follows ⌃ held alone (a click, not a chord; Mission Control does not take a click).

**Tests:** `BufferPasteChordTests` (2): ⌃⌘2 pastes slot 2 and ⌃2 does not (mutation-checked: with the old ⌃-only branch the test
FAILS); the real main menu holds no ⌃⌘ + digit item (the walk is proven to see the ⌥⌘1–3 items). Interop 283/283.

**To verify (Mac):** select a word, ⌘1; click elsewhere, ⌃⌘1 → it pastes. ⌃1 alone does nothing in Scrivi (macOS may switch desktop).
Edit ▸ Paste From Buffer reads "⌃⌘1–9".

## I-0289

**Title:** Copy buffers 1–9 skip the emphasis balancing that ⌘C / ⌘X / ⌘V do
**Status:** ✅ **VERIFIED 2026-10-12** (user, on the Mac). First run: steps 2 and 4 *"failed"* — ⌃1 is taken by macOS (→ [I-0290]) and step 4's paste landed in a HEADING line, outside the bold (history log seq 6034). Re-run with slot 2 and body-text bold: user: *"the operator corrections "fixed" the problems with the paste in 2 and 4."*
**Platform:** `[Apple]`
**Component:** `ManuscriptTextView.Coordinator` — `copyIntoBuffer`, `cutIntoBuffer`; `ManuscriptNSTextView.insertTextForBuffer`
**Severity:** Medium — no text is lost and the scene itself stays balanced, but a buffer paste can drop or add bold/italic
**Sprint:** [SP-179] — ✅ ruled 2026-10-12 (user, Q2 Option A: *"I'll go with Option A. Fix it here."*)
**Date Identified:** 2026-10-12 (planning [SP-179], by reading; then measured)

**Description:** EP-046 AC12 made ⌘C copy BALANCED Markdown (a selection inside a bold span carries its own markers),
[T-0591] balanced each scene's piece of a cross-scene ⌘C, and [SP-163]'s live pass ruled that Scrivi's own paste keeps ITS
formatting where it lands. The copy buffers were built before all three (SP-056) and were never brought along.

**Expected:** a buffer behaves as ⌘C / ⌘X / ⌘V of Scrivi's own copy do.

**Actual (measured 2026-10-12, `BufferBalanceTests`, each test failed before the fix):**
- **(a)** ⌘N of `old te` inside `x **bold text** y` stored `old te` (expected `**old te**`), so its paste was not bold. ⌥N of
  `ld** y` stored raw text, not `**ld** y`. (The scene left by the cut WAS balanced — `shouldChangeText` balances the delete.)
- **(b)** ⌘N across scenes stored `fragmentExtract`'s raw pieces, not `["**ld** y", "next **tw**"]`.
- **(c)** ⌃N of plain `xy` pasted inside `**abcd**` turned bold; ⌘V of the same own copy keeps it plain.

**Root cause:** `copyIntoBuffer` / `cutIntoBuffer` sliced storage (`(tv.string as NSString).substring(with: sel)`) and stored
the extracted fragment as-is; `insertTextForBuffer` inserted verbatim without `pastingOwnCopy`, so `balancedEdit` used the
caret's style.

**Fix:** `ManuscriptTextView.swift` —
- `balancedSource(_:_:)` (new): the slot stores `presenter.balancedCopy(...).source`, used by `copyIntoBuffer` and `cutIntoBuffer`.
- `balancedFragment(_:_:)` (new): ⌘C's per-scene balancing, moved out of `structuredCopyIfCrossBoundary` so ⌘C and ⌘N share it.
- `insertTextForBuffer`: sets `pastingOwnCopy` around the insert (a slot is always Scrivi's own copy).
- `BufferService.swift`: the header comment described the REPLACED context-sensitive ⌘N / ⇧⌘N design; rewritten to ⌘N / ⌃N / ⌥N.

**Tests:** `Scrivi/Tests/ScriviInteropTests.swift` `BufferBalanceTests` (4) — driven through the coordinator's buffer actions,
the ones ⌘N / ⌥N / ⌃N and the palette call; share I-0287's harness. Interop 281/281.

**To verify (Mac):** in a scene with **bold text**, select from the middle of the bold word into the plain text after it, ⌘1;
put the caret in plain text and ⌃1 → the bold part stays bold. Then copy plain words with ⌘2, put the caret inside a bold word,
⌃2 → the pasted words stay plain.

## I-0288

**Title:** After a scene is edited outside Scrivi, undoing in a later session restores a mix of the old and new text
**Status:** ✅ **VERIFIED 2026-10-12** (user: *"mark I-0288 as verified"*), on the test evidence below — ⚠️ no rig or Mac live step, and `the-stairs-of-tintagael` not checked.
**Platform:** `[ScriviCore]` (both platforms reach it — the repair runs at every history open)
**Component:** `HistoryStore::validateSceneHead` (`HistoryStore.cpp:481`) → `HistoryService::reseedSceneFloor` (`HistoryService.cpp:243`)
→ `HistoryService::rebuildHeadCache` (`HistoryService.cpp:892`)
**Severity:** High — the undo is APPLIED and SAVED, so the mixed text reaches the scene file
**Sprint:** [SP-178] (user, 2026-10-12: *"fix it inside SP-178"*)
**Date Identified:** 2026-10-12 — measured by a test written for it ([SP-178]; [I-0287]'s record flagged the shape as *"seen, not
fixed … unmeasured"*)

**Measured through `scrivi_*` alone** (`CaptureCApiTests.cpp`, *"after an outside edit, the next edits undo to the OUTSIDE
text"*, tag `[.I-0288]`):
1. Session 1: `"Alpha"` → `"Alpha one."` → `"Alpha one. Alpha two."` (two steps); bytes reported persisted; close.
2. Outside Scrivi the file becomes `"Zulu."`. Session 2: `validate_scene` → `externalChange` ✅. Type → `"Zulu. Yankee."`;
   undo → `"Zulu."` ✅, undo → stops at the barrier ✅, redo; close.
3. Session 3: `validate_scene("Zulu. Yankee.")` → no external change ✅ — ⛔ **undo → `"Zulu. one. Alpha two."`** (expected
   `"Zulu."`), ⛔ **redo → `"Zulu. Yankee. one. Alpha two."`**.

**Root cause (read, and matches the measurement):** the repair calls `reseedSceneFloor(scene, diskText)` and persists it as a
`floor` record. At the next open `rebuildHeadCache` starts the scene from that floor and re-applies EVERY diff on the
root→current path (`HistoryService.cpp:908–915`) — including session 1's, which were computed against the OLD text. In session 2
the head was set directly, so the defect only appears after a reopen. ⚠️ The head-hash check stays silent because it compares
disk bytes to disk bytes ([I-0104]), so nothing warns the writer.

**Where it can bite already:** any scene that has raised *"This scene was changed outside Scrivi"* and was then edited and undone
in a later session. ⚠️ [I-0268]'s record says the real `the-stairs-of-tintagael` took this repair on `scene_019faeb3…`
(2026-10-03). Not checked on that project.

**Fix direction (as filed):** [I-0287]'s mechanism, which exists for exactly this: record the `externalChange` barrier WITH
`sceneTexts: {scene: diskText}` instead of re-flooring, so the reset sits AT the barrier and replay applies only the diffs after
it. Then drop `reseedSceneFloor` / the `externalChange` floor record. ⚠️ Logs already written carry the bad floor record. Whether
a fix must repair them (e.g. ignore an `externalChange` floor when replaying) is part of the ruling.

**Related:** [I-0287] (same shape, split/merge; its record named this risk), [I-0104] (why the hash check cannot see it),
[I-0268] (a real project took this repair).

**Resolution (2026-10-12, [SP-178], AC10) — the fix direction as filed:**
- **Repair:** `HistoryStore::validateSceneHead` records the `externalChange` barrier with `sceneTexts: {scene: diskText}` and
  writes NO floor record; `HistoryService::reseedSceneFloor` is removed (no other caller). The reset sits AT the barrier, so
  replay applies only the diffs after it.
- **Logs written before the fix:** while replaying, a `floor` record with `"reason": "externalChange"` that directly follows an
  `externalChange` barrier is attached to that barrier as its `sceneTexts` (`HistoryService::attachLoadedBarrierText`), not
  loaded as a floor. The old repair always wrote the two back to back. Any other `floor` record loads as before. Nothing is
  rewritten on disk; the log is read correctly at every open.
- **Docs:** `scrivi.h` (`scrivi_history_validate_scene`), design doc A.1 / §4 / §6.b.
- **Tests:** `CaptureCApiTests.cpp` `[I-0288]`, two sections: a log written by this build, and a log rewritten into the
  pre-fix shape. ✅ Green. ✅ **Mutation-checked:** legacy attach disabled → the legacy section red (`"Zulu. one. Alpha two."`);
  repair without the barrier text → both sections red. ✅ ctest macOS **718/718**; ✅ Linux (GCC, non-root) **721 test cases**, `[I-0288]` run on its own there too; ✅ interop **277**; ✅ **all 30 Linux smokes** (incl. `undo_smoke`).
- ⚠️ **The real project is not checked.** If `the-stairs-of-tintagael` holds such a record ([I-0268]), the legacy path is what
  opens it correctly; whether it does on that log is untested.

---

## I-0287

**Title:** After a structural rewrite of a scene (merge, split), undoing the next edit there restores the scene's text from BEFORE
the structural op
**Status:** ✅ **VERIFIED 2026-10-15** (user: *"verify I-0287"*), on the rig live pass of 2026-10-13 (build 73, [SP-178] steps 4–12: the split and merge notices with this Issue's undo). Resolved 2026-10-11.
**Platform:** `[ScriviCore]` — reached from Linux and Apple alike (both call the same `scrivi_capture_*`)
**Component:** `history/CaptureSession` + `HistoryService` per-scene cached head; every platform's split / merge path
**Severity:** High — the merge case loses text from disk: the merged scene's file was deleted, so the joined-in text then
survives only in redo
**Sprint:** [SP-178] — ✅ ruled 2026-10-11 (user: *"Fix I-0287 in SP-178."*)
**Date Identified:** 2026-10-11 ([SP-178], `platforms/linux/tests/undo_smoke.cpp`, while building Linux undo)

**Description:** the core diffs every edit against its OWN cached copy of the scene (the head of its history). A merge or a split
rewrites the scene on disk without telling history the new text, so the cache stays at the pre-structural text. The next edit is
recorded as a diff from that stale text; undoing it puts the stale text back.

**Expected:** after a merge or split (which already stops undo with a barrier), undoing the next edit in the affected scene
restores the scene as it was right after the structural op.

**Actual (measured):**
- **Merge** (Linux, `undo_smoke` §4b): scene 2 merged into scene 1; type in the survivor; Ctrl+Z → the survivor holds its
  PRE-MERGE text, the merged-in text is gone (and its scene file was deleted by the merge).
- **Split** (Linux, `undo_smoke` §5): split mid-scene; type in the head scene; Ctrl+Z → the head scene holds its PRE-SPLIT text,
  so the tail now exists twice (here and in the new scene).
- **Through `scrivi_*` alone** (a scratch program, 2026-10-11): `note_edit(S, "Alpha. One.", …, before "Alpha.")` →
  `record_barrier(sceneMerge, S)` → `note_edit(S, "Alpha. One.\n\nBravo. Two.", …, before "Alpha. One.\n\nBravo.")` →
  `capture_undo` returns `newText: "Alpha. One."` — "Bravo." is dropped. The platform passed the correct text before; the core
  uses it only to SEED a scene's first edit (`CaptureSession::seedIfNeeded`, `CaptureSession.cpp:90`), never again.

**Why Apple has it too (read, not run):** Apple's split writes both halves with `engine.saveScene` directly
(`ManuscriptTextView.swift:~1149–1160`) and its merge rewrites through `engine.mergeScene`; neither tells history the new text, and
the next `noteEdit` reaches the same core path. ⚠️ Not yet reproduced on the Mac by a live step — the ABI proof above is the
evidence.

**Root cause:** history's per-scene head is re-seedable only on first edit (`seedIfNeeded`) or by
`scrivi_history_validate_scene` on an external-change mismatch (which also records a misleading "changed outside Scrivi"
barrier). [I-0270]'s record already notes the gap: *"the core keeps a per-scene head text and its reseed is NOT on the ABI"*, and
flags the old collapsing cut as *"SUSPECTED, unverified"* for the same reason.

**Fix direction (as filed):** let a structural barrier carry the affected scenes' new texts (or a separate re-seed call after
the op), so the core resets each scene's head — no diff, no externalChange note — then call it from both platforms' split, merge,
chapter-split and chapter-merge paths. A ScriviCore + Apple + Linux change, tested through `scrivi_*`.

**Related:** [I-0270] (grouped edits; names the missing reseed), [I-0104] (head HASH vs disk — the hash half of the same idea,
fixed), [EP-019] AC7 (barriers).

**Resolution (2026-10-11, [SP-178]) — the fix direction's first option:**
- **Core:** a hard barrier now carries `sceneTexts` (sceneID → the text the op left in each scene it rewrote or created). From
  that node on, those scenes read that text: `HistoryService::recordBarrier` resets the cached head, and the same reset runs
  wherever the head is REBUILT — `rebuildHeadCache` (reopen, branch select), `pruneInconsistentNodes` (load validation), and both
  eviction paths (`evictToCapacity`, `applyLoadedEviction`: a barrier promoted to the root becomes the floor). Shared helper
  `applyBarrierTexts` (`HistoryService.cpp`). Persisted in the barrier record (`barrier.sceneTexts`, written only when present;
  older logs read as before). A scene the op CREATED (a split's tail) gets its floor at the barrier (`floorTexts_.try_emplace` +
  a `seed` floor record), so its head hash is still checked at the next open. `CaptureSession::afterBarrier` marks those scenes
  seeded and resets the caret scene's whitespace reference. ABI: optional `sceneTexts` on `scrivi_history_record_barrier` and
  `scrivi_capture_record_barrier` (`scrivi.h`). ⚠️ Not a re-seed of the FLOOR: the floor is the root's state, and re-flooring
  would replay pre-op diffs onto post-op text at the next open.
- **Both platforms record split/merge barriers AFTER the op**, with the texts (they were recorded BEFORE it, when a split's new
  scene ID did not exist yet). Linux `EditorShell.cpp` (`onCreateSceneRequested`, `onCreateChapterRequested`,
  `onMergeSceneRequested` — the survivor's text is read back with `readSceneTexts`, the loader's own call); Apple
  `ManuscriptTextView.swift` (`handleCreateScene`, `handleCreateChapter`, `handleMergeScene` — the loader's `joinText` / head /
  tail). Chapter merge only relocates files (IDs and texts unchanged), so its barrier carries no texts.
- **Tests:** `CaptureCApiTests.cpp` — merge (and after a reopen), split (head and new tail; the tail's outside edit is still
  detected), eviction past the barrier (in session and from the log) — **5 checks red with the core fix reverted, green with it**.
  Linux `undo_smoke` §4b/§5 green; **red again with the Linux texts removed**. ctest macOS 717/717, Linux (non-root) 721/721;
  interop 274 tests; all 30 Linux smokes.
- ✅ **Apple's half, tested 2026-10-12:** `UndoAfterStructuralOpTests` (`ScriviInteropTests.swift`): a real project opened by
  `loadAsync`, the REAL coordinator's `handleCreateScene` / `handleMergeScene`, typing through `insertText` → `textDidChange`,
  then `performUndo`, with the text checked in the loader AND on disk. Split (head, tail) and merge: 3/3 green. ✅ **Red with
  Apple's `sceneTexts` dropped** (split head + merge fail; the tail passes either way, as it should). ⚠️ The case needs the scene
  to have an edit BEFORE the op; a never-edited scene seeds correctly from "text before". (Not a live step on the Mac.)
- ⛔ **The `externalChange` repair DID corrupt at the next open** — measured 2026-10-12 as [I-0288], fixed the same day in [SP-178].


---

## I-0285

**Title:** Opening a large project on Linux takes ~3 minutes and freezes the UI for ~55 s — the timeline is built twice on the main thread
**Status:** ✅ **VERIFIED 2026-10-10** (user, SP-173 live pass on the rig and the Mac; launch to idle 172 s → 15.4 s). Archived with the SP-173 close (*"close SP-173"*).
**Platform:** `[Linux]`
**Component:** `EditorShell` (project load) · `ScriviBridge` (per-scene story-time calls)
**Severity:** High
**Epic:** [EP-048] (found there; not caused by it) · **Sprint:** 🟡 [SP-173] → [`../../Sprints/Closed/Sprint-SP-173.md`](../../Sprints/Closed/Sprint-SP-173.md)
**Date Identified:** 2026-10-09 (SP-166 live pass; user: *"note the time it takes Linux to load this"*)

**Description:** dumas-prose-timelines (1,186 scenes, 1.8 MB), opened on the rig from the network share, takes ~172 s from
launch to an idle editor. For the last ~59 s the UI is frozen: the main thread builds the timeline twice.

**Measured** (build 62, `SCRIVI_LOAD_LOG=1`; run A with the presenter, run B with `SCRIVI_NO_PRESENTER=1`):

| Phase | A | B | Thread |
| ----- | -: | -: | ------ |
| Core `openProject` | 55.6 s | 59.8 s | worker |
| 1,186 scene bodies (`openSceneForBulkLoad`, one call each, ~48 ms) | 57.4 s | 57.3 s | worker (progress bar) |
| `SceneDocument::build` + `setDocument` | 0.04 s | 0.05 s | main |
| `rebuildNavigator()` (its own work is cheap — see root cause) | 28.1 s | 27.9 s | main, UI frozen |
| `reloadTimeline()` at the end of the load | 27.4 s | 27.9 s | main, UI frozen |
| Event loop idle (presenter's deferred first highlight) | 3.4 s | 0.02 s | main |
| **Total** | **172 s** | **173 s** | |

**Root Cause Analysis:** `rebuildNavigator()` ends by calling `reloadTimeline()`. `reloadTimeline()` returns early while
`loading_` is set ("load() calls this once at the end"), but `applyLoadedProject` sets `loading_ = false` **before** calling
`rebuildNavigator()`, so the guard never fires and the timeline is built twice. Each build calls `getSceneStoryTime` once per
scene, and again per scene when a story structure is present (`reloadTimeline`, the dot loop and the band loop): up to
2 × 1,186 core calls per build, every one on the main thread, against the share.

**Expected:** the timeline is built once, its data is fetched off the main thread, and the UI never freezes while a project opens.

**Acceptance Criteria** (user, 2026-10-09):
- [ ] **AC1 — Built once:** the timeline is not rebuilt during the load.
- [ ] **AC2 — EP-048 must not make it worse:** every EP-048 Sprint that touches the Linux load records the `SCRIVI_LOAD_LOG`
  phases on dumas over the share, before and after. The presenter already adds ~3.4 s of main-thread work after the load
  (Qt defers a highlighter's first pass to the event loop); that figure is the baseline to beat, not to grow.
- [ ] **AC3 — Everything we can to minimise load time:** each phase measured and either reduced or ruled irreducible:
  the core open (56 s), the per-scene body reads (57 s, one ABI call per scene), the per-scene story-time calls, the
  navigator, the presenter's first highlight, layout.
- [ ] **AC4 — Off the main thread:** the timeline's data (`getTimeline`, per-scene story time, historical events, story
  structure) is fetched in the load's existing worker; the main thread only builds widgets. If any part cannot move, the
  reason is recorded.
- [ ] **AC5 — On the progress bar:** whatever moves into the worker is counted in the determinate progress bar, as scene
  bodies already are.
- [ ] Apple's shape is checked first (`feedback_linux_adopts_apple_shape`): how Apple loads timeline data at open.

**Related (not duplicates):** [I-0195] (the open froze the UI → moved to a worker), [I-0196] (`scrivi_open_scene` resolved the
whole manuscript per scene), [I-0231]/[I-0232] (slow open, tintagael). The load-phase log and `SCRIVI_NO_PRESENTER` are in
`EditorShell.cpp` (build 62); the log was ruled a permanent diagnostic (SP-173 Q2) — `Scrivi_Linux_Rig_Setup_v0_1.md` §5.1.
