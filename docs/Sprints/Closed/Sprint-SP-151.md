---
sprint: SP-151
epic: none
status: Closed
platform: Apple
created: 2026-09-30
activated: 2026-09-30
closed: 2026-10-02
---

# SP-151 — `[Apple]` **Apple issue batch + long-manuscript navigation**

**Status:** ✅ **CLOSED 2026-10-02 (user-approved):** *"close SP-151 and carry the Issues into the next Sprint. We'll close AC2 without a resolution and if it happens reliably, will address it then."* (was 🟡 ACTIVE from 2026-09-30.) ⚠️ Runs **alongside [SP-147]**, which is blocked on the Linux
rig being down (its remaining ACs need a live pass). ✅ **No shared files:** SP-147 is `platforms/linux/`
only.
**Epic:** none — a batch of independent Apple Issues plus two writer-requested features.
**Size:** ✅ **MEDIUM**

---

## Items — FINAL status at close

| ID | Title | Final status |
| -- | ----- | ------------ |
| **[I-0216]** | Identity re-minted every launch → C++ `KeychainSecureStore` | ✅ Verified 2026-09-30 |
| **[I-0217]** | World picker greys out `.scrivworld` | ✅ Verified 2026-09-30 |
| **[I-0218]** | Asset picker unordered → sorted `AssetStore::list` | ✅ Verified 2026-09-30 |
| **[I-0202]** | `MainActor.assumeIsolated` / uncaught-exception handler | ⚪ **AC2 closed WITHOUT a resolution** — returned to the Issue backlog (fix in code, never verified) |
| **[I-0258]–[I-0263]** | Found during the 2026-09-30 passes | ✅ Verified 2026-09-30 |
| **[T-0568]** | Go to Manuscript Start / End | ✅ Verified 2026-09-30 |
| **[T-0570]** | Project drive gone → red top banner | ✅ Verified 2026-09-30 |
| **[T-0569]** | Scene navigator search (full scan, per platform) | ✅ Verified 2026-10-01 |
| **[T-0571]** | Search → caret at the first match | ✅ Verified 2026-10-01 |
| **[I-0266]** | Manuscript Start landed in front of Chapter 1's heading | ✅ Verified 2026-10-01 |
| **[T-0572]** | Caret skips the gap between scenes | ✅ Verified 2026-10-02 |
| **[I-0269]** | ⌫ on a selection starting a scene did nothing | ✅ Verified 2026-10-02 |
| **[I-0270]** | ⌫ across a scene break CORRUPTED the project → delete text, keep scenes; grouped undo (core) | ✅ Verified 2026-10-02 (Apple) — ⚠️ **Linux live pass OUTSTANDING**, user-ruled non-blocking |
| **[I-0271]** | Blank band after create/merge | ✅ Verified 2026-10-02 |
| **[T-0573]** | Caret keeps its viewport height across create/merge | ✅ Verified 2026-10-02 |
| **[I-0272]** | Chapter create/merge rebuilt the manuscript twice | ✅ Verified 2026-10-02 |
| **[T-0574]** | ⇧⌘0 / ⇧⌘1 shortcuts | ✅ Verified 2026-10-02 |
| **[T-0575]** | Chapter-ending divider tint | ✅ Verified 2026-10-02 |
| **[I-0273]** | Position not restored on load (UI refactor timing) | ✅ Verified 2026-10-02 |
| **[I-0267]** | Keystrokes 60–95 ms after open+close of another window | ⚪ **CLOSED 2026-10-02 after this close — a DUPLICATE of [I-0206]'s 2026-09-25 ruling** (was briefly carried to SP-152) |
| **[I-0268]** | History fails to open on tintagael (`unknown node`) | ➡️ **CARRIED to [SP-152]** (not investigated) |

<details><summary>Planning-time item table (as activated 2026-09-30)</summary>


| ID | Title | Ruling |
| -- | ----- | ------ |
| **[I-0217]** | World picker greys out `.scrivworld` packages | ✅ No ruling needed — drop `canChooseFiles = false` at both sites, KEEP the type filter |
| **[I-0202]** | `MainActor.assumeIsolated` in the app delegate | ⚠️ **Re-aimed 2026-09-30:** `NSApplicationDelegate` is `@MainActor`, so the two named sites cannot trap; ⛔ the **uncaught-exception handler** can (it runs on the throwing thread). To be proven by compile check, then fixed at the real site |
| **[I-0218]** | Asset picker unordered | ✅ **RULED 2026-09-30 — BOTH:** `AssetStore::list` returns entries sorted case-insensitively by displayed title (fallback filename); ⚠️ the picker stays FREE to re-sort by other keys later |
| **[I-0216]** | Local identity re-minted every launch on macOS | ✅ **RULED 2026-09-30 — KEYCHAIN**, as a **C++ `SecureStore` inside ScriviCore** using Security.framework's C API (no Swift). ⛔ `EncryptedFileSecureStore` REJECTED for Apple: it needs OpenSSL (not shipped on Apple), reads `/etc/machine-id` (absent on macOS), and its machine-bound key fails HARD after Migration Assistant / Time Machine restore |
| **[T-0568]** | Go to Manuscript Start / End | ✅ **RULED 2026-09-30 — Project menu + toolbar.** ✅ Also reveals the first/last row in the navigator via the I-0161 `revealRequest` path |
| **[T-0569]** | Scene navigator search | ✅ **RULED 2026-09-30 — filter the APP's live in-memory text** (unsaved edits are found). ⚠️ Index vs brute force: see §Search below |

---


</details>

## Acceptance Criteria

- [x] **AC1** — [I-0217] ✅ **VERIFIED 2026-09-30** (ticked 2026-10-02; the item was archived Verified but the box was never checked) — ✅ a `.scrivworld` is selectable by a SINGLE click in both `Add Existing World…` and
      the relink panel; ⚠️ the panel still does not descend into the package ([I-0185]).
- [ ] ⚪ **AC2 CLOSED WITHOUT A RESOLUTION 2026-10-02 (user ruling)** — [I-0202] returned to `Issue-backlog.md`; addressed only if the crash reproduces reliably — [I-0202] ✅ no `MainActor.assumeIsolated` can run off the main thread; ⚠️ the Issue record
      states which sites were real.
- [x] **AC3** — [I-0218] ✅ **VERIFIED 2026-09-30** (ticked 2026-10-02; the item was archived Verified but the box was never checked) — ✅ `AssetStore::list` returns a stable, case-insensitive title order, ⚠️ PROVEN through
      `scrivi_*` (`feedback_boundary_tests_not_facade`), and the picker shows that order.
- [x] **AC4** — [I-0216] ✅ **VERIFIED 2026-09-30** (ticked 2026-10-02; the item was archived Verified but the box was never checked) — ✅ the identity survives a relaunch: the second launch logs `new: false` with the
      SAME `identityID`. ⚠️ Tested through the C ABI.
- [x] **AC5** — [T-0568] ✅ **VERIFIED 2026-09-30** (ticked 2026-10-02; the item was archived Verified but the box was never checked) — ✅ `Project ▸ Go to Manuscript Start / End` + toolbar buttons move the caret to the
      first/last character of the manuscript, centre it, AND reveal the first/last navigator row.
- [x] **AC6** — [T-0569] ✅ **VERIFIED 2026-10-01** — ✅ an always-visible search field at the BOTTOM of the navigator filters it as the
      writer types, matching scene body text, derived scene titles and chapter titles.
- [x] **AC7** — [T-0571] ✅ **VERIFIED 2026-10-01** — ✅ a navigator click made while searching puts the caret at the first match in that
      scene (scene start for a chapter-title-only match).
- [x] **AC8** — [I-0266] ✅ **VERIFIED 2026-10-01** — ✅ `Go to Manuscript Start` and ⌘↑ land on the first scene's first character with
      chapter titles shown, and typing cannot reach heading text.
- [x] **AC9** — [T-0572] ✅ **VERIFIED 2026-10-02** — ✅ arrows and clicks never leave the caret in the gap between scenes (after a divider, or
      on a chapter heading); the divider's own position (previous scene's end) remains a stop.
- [x] **AC10** — [I-0269] ✅ **VERIFIED 2026-10-02** — ✅ ⌫ / ⌦ on a selection that starts at a scene's first character deletes the selection.
- [x] **AC11** — [I-0270] ✅ **VERIFIED 2026-10-02 (Apple; Linux live pass outstanding, user-ruled non-blocking)** — ✅ ⌫ / ⌦ / ⌘X / ⌥N (Linux: Backspace / Delete / Ctrl+X / Edit ▸ Cut) on a cross-scene
      selection remove the selected text from EACH scene and keep every scene; one ⌘Z restores them all;
      no ordinary edit can remove a divider.
- [x] **AC12** — [I-0271] ✅ **VERIFIED 2026-10-02** — no blank band after create/merge (predates SP-151; A/B on `dbc158f`).
- [x] **AC13** — [T-0573] ✅ **PASSED 2026-10-02 (scene + chapter)** — ✅ after create/merge of a scene or chapter the caret stays at the same height in the viewport.
- [x] **AC14** — [I-0272] ✅ **PASSED 2026-10-02** — ✅ a chapter create/merge rebuilds the manuscript ONCE; T-0573 + I-0271 hold for chapters too.
- [x] **AC15** — [T-0574] ✅ **VERIFIED 2026-10-02** — ✅ ⇧⌘0 / ⇧⌘1 go to Manuscript Start / End.
- [x] **AC16** — [T-0575] ✅ **VERIFIED 2026-10-02** — ✅ the chapter-ending divider is visibly stronger than a scene divider.
- [x] **AC17** — [I-0273] ✅ **VERIFIED 2026-10-02** — ✅ reopening a project restores the writer's position.
- [x] **AC-build** — ✅ 2026-10-02: macOS + iOS builds; `ctest` macOS **644/644**, Linux **648/648** (no core change since) — ✅ `xcodebuild … build` succeeds; ✅ `ctest` green; ✅ Linux still builds (core changes).

---

## §Search — the scale question

⚠️ **MEASURED 2026-09-30 by the user** (`bench.swift`: 4,272 KB in 1,200 scenes, ≈ Bible-sized,
case-insensitive substring over every scene, best of 5):

| Query | Swift `String.range` | `NSString.range` |
| ----- | -------------------- | ---------------- |
| `"a"` | 0.99 ms | 0.22 ms |
| `"and"` | 10.17 ms | 1.15 ms |
| `"and so"` | 167.50 ms | 15.40 ms |
| `"and so it goes forth"` | 257.80 ms | 21.37 ms |
| no match | 252.13 ms | 19.12 ms |

✅ **Brute force with `NSString` is ~20 ms worst case**; ⛔ Swift `String.range` is ~12× slower and unusable
per keystroke.

✅ **RULED 2026-10-01 — FULL SCAN, NO WORD INDEX; implemented PER PLATFORM.** User: *"we're working with
different platforms and integrating with different UI libraries so we will naturally have to build the
mechanism separately for each platform. Also each platform has different search capabilities that we will
want to deal with/take advantage of."* ✅ So navigator search is UI-layer matching over each platform's own
in-memory live text — ⚠️ **not** backend logic reimplemented in Swift, and **not** a core endpoint.

Why not an index: a word index cannot answer substring queries (`"nd s"`) without n-grams; the live-text
ruling would force it to update on every manuscript edit; and the scan is already fast enough.

⚠️ **Re-measured 2026-10-01 against the APP's representation** (Swift `String`, 4,174 KB / 1,200 scenes,
best of 5) — the earlier bench used pre-built `NSString`s, which the app does not hold:

| Query | pre-built `NSString` | bridged `String as NSString` | ✅ `localizedStandardContains` | `String.range` |
| ----- | -------------------- | ---------------------------- | ------------------------------ | -------------- |
| `"and so"` | 10.9 ms | 16.8 ms | 18.3 ms | 176.1 ms |
| `"and so it goes forth"` | 14.2 ms | 23.9 ms | 23.6 ms | 262.0 ms |
| no match | 13.4 ms | 22.1 ms | 22.6 ms | 256.5 ms |

✅ Bridging costs ~1.6× over pre-built — ⛔ not worth a second, stale-able copy of the text.
✅ `localizedStandardContains` costs the same as the bridge and is case- AND diacritic-insensitive and
locale-aware (Apple's user-facing search semantics), so it is the matcher. Revisit an index only if a
live pass on the largest real project shows lag, or search grows beyond the navigator.

---

## Retrospective (2026-10-02)

✅ **What went well**
- ✅ **Evidence before fixes paid off repeatedly:** the TextKit 2 harness (estimated geometry), the `dbc158f` A/B
  ([I-0271] predates the sprint), and the `viewH=0` diagnostic ([I-0273]) each turned a guess into a cause.
- ✅ **[I-0270] was caught as data corruption, not a display glitch,** and confirmed ON DISK in the fixture before
  the fix — then fixed on BOTH platforms (Linux's Edit ▸ Cut bypass found in the same work).
- ✅ New core behaviour (edit groups) shipped with C-ABI tests **proven red**.

⚠️ **What went wrong**
- ⛔ **Three fixes needed a second or third attempt** (T-0573, [I-0273] ×3): each first attempt trusted absolute
  TextKit 2 geometry or a fixed timing window. ✅ Lesson: after a full rebuild, scroll RELATIVE to what AppKit
  resolved; end corrections on the writer's input, not a timer.
- ⛔ **I misstated a tradeoff before the user chose** ("Swift-only" undo — the core reseed was not on the ABI);
  corrected before building.
- ⛔ **IDs T-0571–T-0575 and I-0266–I-0273 were issued WITHOUT `next-id.py`**; ✅ no collision (they matched the
  registry's next values) and the registry was advanced at close.
- ⚠️ A stale binary produced a false red once (`ctest` after restoring a disabled loop) — re-run on a forced rebuild.

➡️ **Carried / owed — and WHO closes the loop**
- **[I-0268]** → **[SP-152]** (🔵 Planning). Owner: Claude when SP-152 is activated.
- ⛔ **[I-0267] should never have been filed:** [I-0206] had accepted the same keystroke cost on 2026-09-25 and I did not search `Closed/`. ✅ Closed as a duplicate, 2026-10-02.
- **[I-0202]** → Issue backlog; ⚠️ re-opened only if the user reports the crash reproducing.
- **[I-0270] Linux live pass** → the user, at the next rig session (non-blocking by ruling).
- ⚠️ **Task index gap T-0551–T-0567** (no rows in `Task-Documentation.md`) — noticed at close, not backfilled.

## Progress log

### ⛔ 2026-10-02 — I-0270: deleting across a scene break CORRUPTED the fixture on disk; I-0269 fixed alongside

✅ **T-0572 VERIFIED** (*"These all pass."*) and ARCHIVED. ⛔ **[I-0270] confirmed ON DISK** in
`dumas-prose-timelines` (`chapter-Y.Y.Y.Y.Y.Y.Y.Y.Y.W`): the same tail (`kfjlkjfkle … ## The King one does not
choose`) is now in BOTH `001-scene.md` and `A-scene.md`. ✅ Read-only inspection; nothing modified.
⚠️ **The fixture should be restored from its source before further passes.**

✅ **Rulings 2026-10-02:** ⌫ across scenes = **delete text, keep scenes** (cut too); undo = **one ⌘Z restores all**,
built as **grouped events** in the core. ✅ **[I-0269] + [I-0270] IMPLEMENTED on BOTH platforms** (detail in
`Issue-active.md`). ⛔ Linux finding along the way: **Edit ▸ Cut bypassed the edit guard** (`QPlainTextEdit::cut()`
is not virtual) — fixed in the same work.

| Check | Result |
| ----- | ------ |
| `ctest` macOS (`build-tests`, FORCED rebuild) | ✅ **644/644** — ⚠️ one false red first: a stale binary still held the disabled loops |
| New `(I-0270)` C-ABI tests | ✅ 3/3 — ✅ **proven RED** (3/3 fail) with the group loops disabled |
| macOS `xcodebuild build` | ✅ **SUCCEEDED** |
| iOS Simulator `xcodebuild build` | ✅ **SUCCEEDED** |
| Linux Docker image (app + smokes) | ✅ builds |
| Linux `editor_map_smoke.sh` (extended) | ✅ **PASS** — ⚠️ first run FAILED on MY expectation (seg0 was `XXAAAA` after the earlier insert check), corrected |
| Linux `ctest` (tests ON, non-root) | ✅ **648/648** |
| `xcodebuild test` (Swift interop) | ➖ **NOT RUN** — it launches the app ([I-0150]); the boundary is covered by the C-ABI tests |

### ✅ 2026-10-01 — T-0571 + I-0266 IMPLEMENTED — ✅ BOTH VERIFIED same day (*"both items behaved correctly"*), ARCHIVED; I-0267 + I-0268 FILED from the console

| Item | What happened |
| ---- | ------------- |
| **[T-0571]** | ✅ Caret to the first match on a searched navigation (see `Task-active.md`) |
| **[I-0266]** | ✅ Manuscript Start / ⌘↑ → first scene's first character; heading guard now refuses insertion IN FRONT OF a heading |
| **[I-0267]** | ⛔ **Keystrokes 60–95 ms on dumas vs 1.7–8.6 ms on 2026-09-14.** ✅ Second run: arrows `0.5–5.0 ms` on a build CONTAINING T-0569 → **T-0569 ruled OUT.** ⚠️ Lead: the slow run opened + CLOSED tintagael's window before dumas in the same process |
| **[I-0268]** | ⛔ **`historyOpen failed: unknown node` on tintagael — same message as [I-0110]; NOT investigated** |

⚠️ **ALSO IN THE CONSOLE, NOT FILED:** (a) `navigateToScene` runs **2–3 times per click** (pre-existing; T-0571 is built around it); (b) near the END of dumas a click costs `setSel≈60–72 ms` + `center≈95–108 ms`; (c) the second open's `WALL CLOCK 71.83 s` is an instrumentation artefact — the phase table ACCUMULATES across both opens (`calls 2`), so the wall clock spans the idle time between them.

| Check | Result |
| ----- | ------ |
| macOS `xcodebuild build` | ✅ **SUCCEEDED** |
| iOS Simulator `xcodebuild build` | ✅ **SUCCEEDED** |

### ✅ 2026-10-01 — T-0569 IMPLEMENTED — ✅ VERIFIED same day by live pass on `dumas-prose-timelines` (*"The live test worked great."*), ARCHIVED

| What | Where |
| ---- | ----- |
| ✅ Search field at the navigator's BOTTOM as a `.safeAreaBar(edge: .bottom)` (the T-0545 pattern), magnifier + clear button | `Scrivi/Views/SceneNavigatorView.swift` (`searchField`) |
| ✅ Body scan OFF the main actor (`@concurrent`), cancelled by the next keystroke via `.task(id:)`; result tagged with its query so a stale one is never applied | `SceneNavigatorView.swift` (`.task(id: SearchKey…)`, `scenesMatching`) |
| ✅ Filter: a chapter whose TITLE matches keeps all its scenes; otherwise a scene stays when its displayed (derived) title or body matches, under its chapter header | `SceneNavigatorView.swift` (`flatRows`) |
| ⚠️ Drag-reorder is DISABLED while filtering — `performMove` reads the drop's predecessor from the visible rows, which would be the wrong neighbour | `SceneNavigatorView.swift` (`.onMove(perform:)`) |

⚠️ **Two behaviours chosen, not ruled — flagged for the live pass:**
1. Results do NOT re-filter while she types in the MANUSCRIPT; they re-run when the query or the scene
   count changes. Observing every edit would re-render the navigator per keystroke, and the scene she is
   typing in could vanish from the list mid-sentence.
2. Matching is case- AND diacritic-insensitive (`cafe` finds `café`).

| Check | Result |
| ----- | ------ |
| macOS `xcodebuild build` | ✅ **SUCCEEDED** |
| iOS Simulator `xcodebuild build` (`ScriviApp-iOS`) | ✅ **SUCCEEDED** |
| Core | ➖ untouched — no `ctest` / Linux impact |

### ✅ 2026-09-30 — I-0217 · I-0202 · I-0218 · I-0216 · T-0568 IMPLEMENTED

| Item | What happened |
| ---- | ------------- |
| **[I-0217]** | ⚠️ **ALREADY FIXED in `0a9e410` (SP-130) and never recorded** — both world panels already set `canChooseFiles = true` and keep the type filter. ✅ Record corrected; ⚠️ needs a click-test |
| **[I-0202]** | ⚠️ **Re-aimed, PROVEN by compile check both ways:** the two named delegate sites are main-actor isolated (they compile touching `@MainActor` statics; a `nonisolated` negative control does NOT). ⛔ **The real hazard was the uncaught-exception handler** — fixed to freeze only on the main thread |
| **[I-0218]** | ✅ `AssetStore::list` sorted by displayed name, case-insensitive, total order. ✅ C-ABI test, **proven red** without the sort |
| **[I-0216]** | ✅ C++ `KeychainSecureStore` (Security.framework C API), `if(APPLE)`. ✅ 5 hidden `[.keychain]` tests on the real login keychain under a TEST service; **persistence proof proven red** with an in-memory store. ✅ iOS targets link `Security`. ⚠️ **Real-app launch LEFT FOR THE USER** — its first launch mints her permanent identity |
| **[T-0568]** | ✅ `Project ▸ Go to Manuscript Start / End` + a `Manuscript` toolbar group (`arrow.up/down.to.line`, verified to exist). ✅ Reveals the first/last navigator row through the I-0161 `revealRequest`; ⛔ selection deliberately NOT set (on macOS it would drive the caret to the scene's FIRST character, undoing "End"). ✅ **Navigator fix:** a reveal for the ALREADY-current scene now scrolls at once — the deferred path waited for a `viewportSceneID` change that never comes. ✅ End offset in UTF-16 (`NSString.length`), not `String.count` |

| Check | Result |
| ----- | ------ |
| `ctest` (macOS, `build-tests`) | ✅ **638/638** |
| `[keychain]` (hidden, run explicitly) | ✅ **5/5, 17 assertions** |
| macOS `xcodebuild build` | ✅ **SUCCEEDED**, no warnings in touched files |
| iOS Simulator `xcodebuild build` | ✅ **SUCCEEDED** |

⚠️ **ONE FALSE RED OF MY OWN, caught before it misled:** after restoring the Keychain test from its
break, the binary was STALE (the restore did not trigger a recompile) and still failed. ✅ A `touch` +
rebuild went green — ⛔ so every "restored and green" claim here was re-run on a forced rebuild.

### ✅ 2026-09-30 — the user's first pass, and what it changed

| Item | Result |
| ---- | ------ |
| **[I-0216]** | ✅ **PASSED on the real app:** launch 1 `identity_01a0f2ee-… (new: true)`, launch 2 the SAME id `(new: false)` |
| **[I-0218]** | ✅ **PASSED:** *"asset picker sorted assets properly"* |
| **[T-0568]** | ✅ Start/End work. ⛔ **Toolbar group REMOVED** (user ruling: it read as Scene Start/End); **menu only**. ✅ ⌘↑/⌘↓ confirmed to reach the manuscript's ends |
| **[I-0258]** (NEW) | ⛔ **The navigator does not keep the current scene visible** — including after `Manuscript End`. ✅ Fixed: the list FOLLOWS `viewportSceneID` with `anchor: nil`; my `revealInNavigator` wiring removed. ⚠️ Reverses [I-0132] in part, by ruling |
| **Manage Worlds** | ✅ The remove-world button is now RED (`role: .destructive` does not tint a borderless image button on macOS) |
| **[I-0217]** | ⚠️ The user could not find the pickers — `Worlds ▸ Manage Worlds… ▸ Add Existing World…`, and `Locate…` on an UNAVAILABLE world's row |

### ⚠️ 2026-09-30 — the user's DRIVE-PULL pass (project + world on the pulled drive)

✅ **[I-0217] PASSED** in both places (single click selects a `.scrivworld`).
⛔ **FOUR findings, one of them DATA LOSS:**

| Item | Result |
| ---- | ------ |
| **[I-0259]** ⛔ High | ⛔ **A failed scene save was treated as a success** — scene marked clean, history told it was written, no retry. ✅ Fixed (stays dirty on failure; also `stampWritingSurface` and both split paths) |
| **[I-0261]** ⛔ High | ⛔ Core: a FAILED read of `worlds/` returned **"no worlds"** as a success; an unreadable binding was DROPPED. ✅ Fixed; 2 tests **proven red**; blast radius checked |
| **[I-0262]** | ⛔ UI: *"uses no worlds yet"* / *"No characters in this scene yet"* shown after a FAILED read. ✅ Fixed |
| **[I-0260]** | ⚠️ Raw *"Operation not permitted"* — the app never recognises its PROJECT is unreachable. 🔵 **Open — wording + behaviour to be ruled with [T-0570]** |

✅ `ctest` **640/640**; macOS build ✅. ⚠️ **None of the four was introduced by SP-151** (blame: 2026-08-12/15).

✅ **[T-0570] + [I-0260] IMPLEMENTED** after the user ruled (*"Project File Not Available"*; keep typing
under a red banner; warn before quitting). ⚠️ **Additions of mine, stated:** the SAME guard on closing the
project WINDOW (it loses the same edits), and an automatic save of every kept-dirty scene the moment the
drive is reachable again. ✅ macOS + iOS builds. ⚠️ **Nothing here is reproducible outside the sandboxed
app** — the whole path needs the user's drive-pull pass.

⚠️ **2026-09-30 — THE RED BANNER WAS PULLED ON A WRONG DIAGNOSIS (corrected the same day).**
✅ **User report:** *"all three top panels are showing that they are sized larger than the window… the
timeline… is not [visible]"* — on startup and after Open Project. ⛔ **I blamed the banner's
`.safeAreaBar(edge: .top)` and removed it; the fault persisted.** ⛔ I then blamed the navigator-follow;
a diagnostic build disproved that too.
✅ **FOUND BY BISECT WITH THE USER IN XCODE:** HEAD perfect → SP-151 Swift + HEAD core perfect → the
cause was in the CORE: **my first [I-0261] fix listed an empty world directory (the normal leftover of
Remove Reference) as an unavailable world**, and that phantom's world-warning bar broke the layout —
✅ corrected, regression test proven red.
⚠️ **AND THE BAR ITSELF BREAKS THE LAYOUT — PRE-EXISTING → [I-0263]:** the committed HEAD breaks
identically when a REAL world's drive is pulled.
⚠️ **The red banner was therefore NEVER shown to be at fault.** It stays out for now: it used the
same `safeAreaBar` mechanism [I-0263] implicates, so it waits for that finding. ⛔ I also skipped
`scripts/check-layout-convergence.sh` when adding it; it flagged the banner's full-width frame.
✅ Everything else in [T-0570] stays: the failed-save detection, auto-save on reconnect, the ruled
wording, and the quit / close-window guards.

### ✅ 2026-09-30 — [I-0263] fixed as an OVERLAY; red banner restored — USER PASS

✅ **[I-0263]:** the world warning is an `.overlay(alignment: .bottom)` on the manuscript (user ruling:
*"translucent… ZStack it"*); ⛔ its amber bleed into the Timeline fixed with `ignoresSafeAreaEdges: []`.
✅ User: *"banner appeared, only over ManuscriptView. timeline no change… banner went away."*
✅ **[T-0570] red banner** restored as a TOP overlay. ✅ User: project drive pulled → red banner; cards show
the drive-unavailable message; drive restored → banner and warnings disappear.
✅ **THEN EXERCISED — ALL PASS:** typing while the drive is out (every scene kept dirty and counted), ✅ the
kept edits **saved automatically on reconnect** ([I-0259]'s payoff), ✅ the Quit AND close-window guards
(Cancel aborts; Quit Anyway discards), ✅ the ruled wording confirmed.

✅ **2026-09-30 — USER-DIRECTED: [I-0216], [I-0217], [I-0218], [I-0263] VERIFIED and ARCHIVED**
(`Verified/Issue-verified-0211-0220.md`, new `Verified/Issue-verified-0261-0270.md`).
✅ **ALSO VERIFIED (user-directed, same day): [I-0261], [I-0262], [T-0568]** — archived
(`Verified/Issue-verified-0261-0270.md`, `../Tasks/Verified/Task-verified-0568.md`).
✅ **AND VERIFIED (user-reported passes): [I-0258], [I-0259], [I-0260], [T-0570]** — archived.
⚠️ **Only [I-0202] remains unverified** (the off-main exception path cannot be triggered on demand), and
**[T-0569] (search) is not started.**
⚠️ `Task-verified-0545.md` annotated: its *"did not deform the UI"* is SUPERSEDED by [I-0263].

✅ **[T-0569] (search) IMPLEMENTED 2026-10-01 — Not Verified** (full scan, ruled 2026-10-01).

⚠️ **2026-09-30 — Sprint created and ACTIVATED.**
