---
sprint: SP-151
epic: none
status: Active
platform: Apple
created: 2026-09-30
activated: 2026-09-30
---

# SP-151 — `[Apple]` **Apple issue batch + long-manuscript navigation**

**Status:** 🟡 **ACTIVE — 2026-09-30.** ⚠️ Runs **alongside [SP-147]**, which is blocked on the Linux
rig being down (its remaining ACs need a live pass). ✅ **No shared files:** SP-147 is `platforms/linux/`
only.
**Epic:** none — a batch of independent Apple Issues plus two writer-requested features.
**Size:** ✅ **MEDIUM**

---

## Items

| ID | Title | Ruling |
| -- | ----- | ------ |
| **[I-0217]** | World picker greys out `.scrivworld` packages | ✅ No ruling needed — drop `canChooseFiles = false` at both sites, KEEP the type filter |
| **[I-0202]** | `MainActor.assumeIsolated` in the app delegate | ⚠️ **Re-aimed 2026-09-30:** `NSApplicationDelegate` is `@MainActor`, so the two named sites cannot trap; ⛔ the **uncaught-exception handler** can (it runs on the throwing thread). To be proven by compile check, then fixed at the real site |
| **[I-0218]** | Asset picker unordered | ✅ **RULED 2026-09-30 — BOTH:** `AssetStore::list` returns entries sorted case-insensitively by displayed title (fallback filename); ⚠️ the picker stays FREE to re-sort by other keys later |
| **[I-0216]** | Local identity re-minted every launch on macOS | ✅ **RULED 2026-09-30 — KEYCHAIN**, as a **C++ `SecureStore` inside ScriviCore** using Security.framework's C API (no Swift). ⛔ `EncryptedFileSecureStore` REJECTED for Apple: it needs OpenSSL (not shipped on Apple), reads `/etc/machine-id` (absent on macOS), and its machine-bound key fails HARD after Migration Assistant / Time Machine restore |
| **[T-0568]** | Go to Manuscript Start / End | ✅ **RULED 2026-09-30 — Project menu + toolbar.** ✅ Also reveals the first/last row in the navigator via the I-0161 `revealRequest` path |
| **[T-0569]** | Scene navigator search | ✅ **RULED 2026-09-30 — filter the APP's live in-memory text** (unsaved edits are found). ⚠️ Index vs brute force: see §Search below |

---

## Acceptance Criteria

- [ ] **AC1** — [I-0217] ✅ a `.scrivworld` is selectable by a SINGLE click in both `Add Existing World…` and
      the relink panel; ⚠️ the panel still does not descend into the package ([I-0185]).
- [ ] **AC2** — [I-0202] ✅ no `MainActor.assumeIsolated` can run off the main thread; ⚠️ the Issue record
      states which sites were real.
- [ ] **AC3** — [I-0218] ✅ `AssetStore::list` returns a stable, case-insensitive title order, ⚠️ PROVEN through
      `scrivi_*` (`feedback_boundary_tests_not_facade`), and the picker shows that order.
- [ ] **AC4** — [I-0216] ✅ the identity survives a relaunch: the second launch logs `new: false` with the
      SAME `identityID`. ⚠️ Tested through the C ABI.
- [ ] **AC5** — [T-0568] ✅ `Project ▸ Go to Manuscript Start / End` + toolbar buttons move the caret to the
      first/last character of the manuscript, centre it, AND reveal the first/last navigator row.
- [ ] **AC6** — [T-0569] ✅ an always-visible search field at the BOTTOM of the navigator filters it as the
      writer types, matching scene body text, derived scene titles and chapter titles.
- [ ] **AC-build** — ✅ `xcodebuild … build` succeeds; ✅ `ctest` green; ✅ Linux still builds (core changes).

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
per keystroke. ⚠️ Index design and the ruling on it: pending (see the conversation of 2026-09-30).

---

## Progress log

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

⏳ **[T-0569] (search) NOT STARTED** — awaiting the index-vs-brute-force ruling.

⚠️ **2026-09-30 — Sprint created and ACTIVATED.**
