# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| **I-0281** | `[Apple]` ⚠️ **A block whose FIRST line is indented 1–3 spaces mis-renders emphasis on every FOLLOWING line — Apple's parser reports their source columns shifted by that indent.** ✅ **MEASURED 2026-10-07 by [SP-165]'s L2 agreement test** (CLI harness compiling `MarkdownBlocks.swift` against `libScriviCore.a`, reduced to a minimal block): `" She said⏎*no* twice."` → Apple hides the OPENING `*`, shows the CLOSING `*` in italic and leaves `n` upright (styles `…1 0 1 1`); `"   Three⏎lines *of* it⏎and *more* here."` → line 2's emphasis lost, line 3 shifted by 3. ✅ md4c (the core, Linux) reports the true positions. ⚠️ Reachable from imported or hand-written text with a leading space and soft line breaks. ⏳ Fix not designed: `MarkdownBlocks`' `offset(line, column)` would need the paragraph's first-line indent per continuation line — ✅ the L2 test is the oracle (its `knownDisagreements` entry flips when fixed). ➡️ **Linked forward to [EP-047]** (user, 2026-10-07). | **Medium** | — (➡️ EP-047) |
| **I-0277** | `[Apple]` ⚠️ **VoiceOver reads the STORED manuscript, not what the writer sees: escape backslashes and hidden heading prefixes are spoken/exposed.** ✅ **MEASURED 2026-10-05** ([SP-161] step 1, TextKit 2 harness): `NSTextView.accessibilityValue()` = `Mr\\. Smith said \\*no\\*.⏎⏎## Heading here…`, and `accessibilityString(for:)` returns the same stored characters. ⚠️ **True since EP-045 E1** (escape backslashes, every typed punctuation mark), and **widened by EP-046**: the `##` prefix is hidden on screen (E2-S1) and `**`/`*` markers will be (E2-S2). ✅ **Expected:** assistive technology gets the PRESENTED text (what is on screen); a heading is exposed as a heading. ✅ **Not affected:** spell-check (no false flags from escapes — measured). ⚠️ **Not in EP-046's ACs** (user: *"file the voice over issue as an issue"*, 2026-10-05). ✅ **Seam for a fix:** `MarkdownEscapes.map` (source → presented) and `ManuscriptPresenter.hiddenTest` already know every hidden character; the AX overrides (`accessibilityValue`, `accessibilityString(for:)`, range ↔ offset methods) would map through them. ⚠️ Not measured: what VoiceOver actually SPEAKS for `\\` and `#` at default punctuation verbosity. ⚠️ Linux has the same question (Qt accessibility reads the document text). | Medium | Not Assigned |
| **I-0275** | `[Apple]` ⚠️ **Keystroke and caret-move cost grows LINEARLY with position in the manuscript — ~20–43 ms at the start, ~89–122 ms at the end of 1.85 MB.** ✅ **MEASURED 2026-10-04** ([SP-157] AC10c, user's run, `dumas-prose-timelines`, 1,855,917 chars): typing 20.0–42.6 ms (scene 0) · 37.1–66.2 ms (scene 291, ~460k) · 89.1–121.7 ms (scenes 1182–1184); Return and ⌫ the same as typing at each place; ⚠️ **arrow keys 0.6–1.7 ms at the start, 89–117 ms at the end — and arrows run NONE of Scrivi's per-keystroke work**, so the scaling is AppKit's (TextKit 2). Scrivi's own work (`[SCRIVI-KEY]`) is flat at 2.3–4.3 ms (→ [T-0583]). ---- ⚠️ **USER'S STANDING (quoted, not inflated):** *"I'm a little worried that we're scanning the whole document at each keystroke"* · *"I didn't say it bites. I said I was kind of concerned."* → *"Let's backlog the Issue for the real cost as well."* ⛔ **NOT ruled as biting.** ---- ✅ **Relation:** [I-0206] (closed not-a-defect) measured `setSel` linear in offset and typing "constant" — ⚠️ this shows typing and arrows are linear too ([I-0206] measured typing at one place). ⚠️ Consistent with [I-0267]'s unexplained fast/slow runs being a caret-POSITION difference — unproven. ---- ✅ **Next step: an Instruments time profile of ONE keystroke and ONE arrow at the end of the manuscript**, to name the AppKit call that grows with position. ⚠️ Only then a design decision; the structural option (not holding the whole manuscript in one text view, with a scene-position map) is Epic-sized. ---- ✅ **BACKGROUND — sync vs async (user asked 2026-10-04; added at the user's request):** ⚠️ **everything per keystroke is SYNCHRONOUS on the main thread, inside `keyDown`**: (1) AppKit's key dispatch → `insertText`; (2) our pre-edit guards in `shouldChangeText` (heading / divider refusal, escape-pair widening); (3) escaping in `insertText`; (4) the storage edit + `EscapeHidingStyler.restyle` (must finish before drawing, or a hidden backslash flashes); (5) **AppKit's own layout / selection / display**; (6) our `textDidChange` bookkeeping (boundary scan, scene lookup, text extract, history `noteEdit`, and on `.`/`!`/`?`/Return a C++ `historyRecordEvent`). ✅ **Already async (debounced `Task`s):** autosave 1 s, Navigator title 300 ms, viewport-scene-after-scroll 120 ms. ✅ **Split at the end of 1.85 MB:** `keyDown` 89–122 ms; Scrivi's (6) 2.3–2.8 ms (≈2.3 ms of it the scan → [T-0583]); (2)–(4) each < 0.5 ms; ⛔ **AppKit ≈ 85–115 ms.** ✅ Arrow keys run none of (2)–(6) and still cost ~90 ms at the end. ✅ **Can it be made async?** ⛔ AppKit's share: NO — `NSTextView`/TextKit are main-thread-only, inside the event; the lever is to find and avoid the offset-linear call, or give the view less text. ⛔ (2)–(4): NO — they decide or change what is stored or drawn. ⚠️ (6): possible, ✅ but after [T-0583] it is ~0.2 ms, and moving the history record off-main risks the EP-019 *"history commits before disk writes"* ordering (§4.d) — not worth it. | Low | Not Assigned |
| **I-0202** | `[Apple]` ⚠️ **`MainActor.assumeIsolated` IN THE APP DELEGATE ASSERTS AN ISOLATION APPKIT DOES NOT GUARANTEE — a latent launch/URL crash.** ⚠️ **Found 2026-09-12 while investigating [I-0201]** (⚠️ **NOT its cause — the app never got far enough to call this**). ⚠️ **`AppDelegate.application(_:open:)` (`ScriviApp.swift:23`) and `applicationWillTerminate` (`:29`) both wrap their bodies in `MainActor.assumeIsolated`.** ⚠️ **`assumeIsolated` ASSERTS a precondition; it does NOT establish isolation** — ✅ **if the caller is not actually on the main actor it TRAPS, killing the process.** ⚠️ **The comment claims AppKit *"called on the main thread"*, ✅ which is true for the THREAD but is NOT the same as Swift-6 main-ACTOR isolation**, ⚠️ **and it is asserted rather than checked.** ---- ⚠️ **WHY IT MATTERS: `application(_:open:)` is the FINDER DOUBLE-CLICK PATH** — ✅ **the app declares `CFBundleDocumentTypes`, so a `.scrivi` package double-click routes here** — ⚠️ **and it can arrive DURING LAUNCH, before the SwiftUI `App` has finished establishing its actor context.** ⚠️ **A trap there is indistinguishable from [I-0201]'s symptom: no window, no console.** ---- ✅ **FIX: `Task { @MainActor in … }`, which ESTABLISHES isolation instead of asserting it.** ⚠️ **`onOpenURLs` is already `@MainActor` and the work it does is async anyway, so nothing is lost by hopping.** ⚠️ **CAUTION on `applicationWillTerminate`: it must complete BEFORE the process dies** (it freezes the session manifest, R4/T-0195), ⚠️ **so a `Task` there would NOT be awaited and could silently drop the write** — ✅ **that one needs a different treatment and must be ruled, not blindly converted.** ⚠️ **SP-151 AC2 CLOSED WITHOUT A RESOLUTION 2026-10-02 (user ruling):** *"We'll close AC2 without a resolution and if it happens reliably, will address it then."* ✅ The uncaught-exception-handler fix from 2026-09-30 stays in the code; ⚠️ never verified, because the crash is latent and has not been reproduced. ---- **STATUS AT RETURN:** ✅ **Resolved - Not Verified** (2026-09-30) — ⚠️ **THE TWO NAMED SITES WERE NOT DEFECTS; A THIRD ONE WAS.** ✅ **PROVEN BY COMPILE CHECK, both ways:** with `assumeIsolated` removed, `application(_:open:)` and `applicationWillTerminate` still compile under Swift 6 touching the `@MainActor` statics — ⚠️ **so `NSApplicationDelegate` makes them main-actor isolated and they cannot trap**; ✅ **NEGATIVE CONTROL:** the same call from a `nonisolated` method FAILS (`main actor-isolated static property 'onWillTerminate' can not be referenced from a nonisolated context`). ⛔ **THE REAL SITE: the UNCAUGHT-EXCEPTION HANDLER** (`ScriviApp.swift`, `installUncaughtExceptionHandler`) — a C callback that runs on the THROWING thread, whose own comment said so, and which called `MainActor.assumeIsolated` unconditionally: ⚠️ an off-main exception TRAPPED inside the handler before its session freeze could run. ✅ **FIX:** freeze only `if Thread.isMainThread`, else log; ⛔ NOT `DispatchQueue.main.sync` (a crash-time deadlock would lose the log too). ⚠️ The delegate's two redundant `assumeIsolated` calls were LEFT — harmless, and out of scope. ⚠️ **Not reproducible on demand** (needs an off-main ObjC exception); verified by reading + build | **Medium** | Not Assigned (returned from SP-151 at its close) |
| **I-0147** | `[ScriviCore]` ⚠️ **KNOWN LIMITATION (user-ruled 2026-08-21, ACCEPTED — not to be fixed in SP-116).** ⚠️ **For up to `kStaleSeconds` (60 s) after an interrupted world write, the world is ENTIRELY UNWRITABLE and its abandoned `.partial` is unreclaimable.** When a volume vanishes mid-import the writer dies holding the lock, leaving `.lock` on disk with a **fresh** heartbeat. Reattach the drive quickly — the natural thing to do — and the next write is refused `worldLocked`; ⚠️ **T-0433's sweep runs only AFTER a successful acquire**, so the orphan survives until the lock ages out. **Observed on the real rig 2026-08-21**: drive pulled mid-import, reattached within ~60 s, next import refused and a **2.9 GB** `.partial` remained. ✅ **Both halves verified**: staged fresh lock + orphan → `worldLocked`, orphan stays; waited past 60 s → **acquired and swept**. ⚠️ **This is arguably CORRECT, which is why it is accepted:** `kStaleSeconds` exists precisely because the core cannot distinguish *"writer died"* from *"writer is briefly stalled"*, and guessing wrong means two processes writing a shared world at once. It **self-heals** within a minute and loses no data. **The stronger evidence available — the package's own VOLUME was unmounted, which is far better proof of a dead holder than a quiet heartbeat — is not currently used.** ⚠️ **Deferred to the network-worlds design**, which must revisit *"exactly one winner"* regardless; ruling that inside an asset sprint is how a locking model gets set by accident (the lesson of I-0144). ⚠️ **The eventual UI must not present the 60 s wait as an error** — it is a retryable state. | Low | ⚠️ **Deferred — network-worlds design.** 🟡 Accepted limitation (2026-08-21); moved to the backlog 2026-10-03 (user). 🟡 **Accepted limitation (2026-08-21)** — ⚠️ **found by the LIVE RIG PASS**; ⚠️ **my own earlier staged-orphan test PASSED because it created the orphan WITHOUT a matching fresh lock** — not the state a real crash leaves |

✅ **[I-0221] and [I-0222] were filed, fixed AND user-verified 2026-09-17/18**, then archived to
[`Verified/Issue-verified-0221-0230.md`](Verified/Issue-verified-0221-0230.md) **in the same step**
(`feedback_archive_on_close`). ⚠️ **Both came from ONE live pass on a FAT32 volume, and neither was
reachable by the test suite as it stood.**

✅ **I-0191 moved to `Issue-active.md` 2026-09-07** — fixed the same day it was filed; ✅ **user-verified 2026-10-03** → `Verified/Issue-verified-0191-0200.md`.


---

*Last Updated: 2026-10-07 — **I-0281 filed** ([SP-165] L2): a first line indented 1–3 spaces shifts Apple's emphasis positions on the following lines.*

*Last Updated: 2026-10-05 — **I-0278 filed** (user): Title, Subtitle, Show chapter titles live in `UserDefaults` and do not travel with the project.*

*Last Updated: 2026-10-05 — **I-0277 filed** (user): VoiceOver reads the stored text (escapes, hidden `##`), found in SP-161 step 1.*

*Last Updated: 2026-10-03 — **stale I-0223 row and section DELETED (user)**: they carried the framing from before the 2026-09-25 rewrite (Medium, "resolve to a wrong location"), which contradicted the live record in `Issue-active.md` (Low, rewritten, [EP-044]). ✅ **I-0147 moved here** (accepted limitation). Prior note follows.*

*Last Updated: 2026-09-18 — **[I-0221] and [I-0222] user-VERIFIED and archived** to
`Verified/Issue-verified-0221-0230.md`; ⚠️ **the backlog is empty again.** ✅ **The drive-pull test that
produced I-0222 is complete.** Prior note follows.*

*2026-09-17 — **two Issues filed from ONE live pass** on a FAT32 USB volume.
✅ **[I-0221]** filed AND fixed the same day: AppleDouble `._*` sidecars
aborted project open, ⚠️ **and the sandbox's `com.apple.quarantine` stamp REGENERATED them on every
write**, so the failure recurred after any manual clean. ✅ **Fixed at the `listDirectory` chokepoint**
after the audit found ~20 scan callers, not the 3 first identified. ✅ **[I-0222]** filed AND fixed the same day:
pulling the volume reported `ScriviError 1` instead of naming the world. ✅ **`ScriviError` now conforms
to `LocalizedError`** (~15 call sites had been showing Foundation's type-name fallback) ✅ **and the two
`worldPending:`/`worldUnavailable:` spellings merged to ONE derived constant on the user's ruling.**
⚠️ **The cards emptying was ruled ACCEPTED, not a defect** — an away world's objects genuinely are not
available. ⛔ **Its first diagnosis was WRONG and the correction is kept in the record**: measurement
showed neither call in the card's load path fails at all. ✅ **The underlying pending behaviour was
CORRECT throughout: the manuscript stayed usable with the volume gone and everything restored on
reattach.**
⚠️ **Both Issues were invisible to a green suite** — fixtures build on APFS and hand-construct their
`detail` strings. Prior note follows.*

*Last Updated: 2026-09-07 — **I-0191 opened** on the user's report of an unexplained folder in the
repo root. ⚠️ **Three garbage-named, EMPTY app-support trees** (dated 2026-08-17) were deleted; ⚠️ **the
mechanism that creates them was NOT fixed.** ✅ **Root cause found by reading the code and REPRODUCED**:
`bootstrapAppSupport` validates `appSupportRoot` in no way, `AbsolutePath` is a bare `std::string`, and
the C ABI's `S()` turns NULL into `""`. ✅ **Test-suite audit (user-requested) came back CLEAN** — every
fixture cleans up via RAII and roots at an absolute `temp_directory_path()`, so ⚠️ **the suite is not the
source**; its real gap is that it can only see its OWN temp dir (AC4). ⚠️ **The exact call site that made
these three folders is NOT identified — recorded as an open question, not guessed.** Prior note follows.*

*Last Updated: 2026-08-20 Removed  references to I-0118 which is verified and does not belong here.  

2026-08-17, later same day (*\*I-0017 ✅ Verified and archived; I-0018 partly fixed and
RESCOPED\*\* — both on the user's report while reviewing this backlog. I-0017 had been fixed and confirmed long
ago and was never filed. I-0018's original complaint (no selection shown on load) is **fixed**; the remaining
behaviour — **the manuscript not scrolling to that selection** — is different from what was reported, so the
Issue is retitled and rescoped rather than left implying the whole thing is broken. ⚠️ It is flagged to be
scoped **together with I-0131 and I-0132**, which are the same underlying question — \*what does it mean to
"be at" a scene?\* — across load, click, and quit. **Backlog is now 1.** Prior note follows.)\*

\*2026-08-17 (**I-0121 and I-0122 ✅ Verified and archived** to
`Verified/Issue-verified-0121-0130.md` at the SP-106 close — both full entries removed from this file in the
same step, which is the discipline the 2026-08-16 note below says was missed twice. **Backlog is now 2:**
I-0017 and I-0018, both 🔴 Open and unassigned. Prior note follows.)\*

*2026-08-16, later same day (*\*I-0058 and I-0112 archived to `Verified/`; I-0121's code fix
applied.\*\* Both Issues had been Verified — 2026-07-09 and 2026-08-11 — and left sitting in this backlog.
⚠️ **The consistency audit earlier the same day did not catch them because it never opened this file**: it
audited `Issue-active.md`, `Issue-Documentation.md` and the `Verified/` archives, and only *grepped*
`Issue-backlog.md`. **An audit scoped to the files that usually go stale will miss the file nobody looks at**
— which is exactly where a Verified Issue goes to be forgotten. I-0121's `rebalancedKeys` guard is now applied
and proven RED-then-GREEN under UBSan (516/516); its CI and coverage halves remain open. Prior note follows.)\*

\*2026-08-16 (**I-0121 opened — ScriviCore CI has been red on every commit since 2026-07-30.**
`rebalancedKeys(1)` divides by zero (`OrderKey.cpp:179`): the ternary guards `n == 0` — already unreachable —
while the divisor is `n - 1`. ⚠️ **Invisible locally by hardware, not by luck**: arm64 returns 0 for integer
division by zero, x86-64 raises SIGFPE, so the developer Mac and the `macos-latest` runner are green while
`ubuntu-latest` crashes. Introduced by **1c42838**, confirmed by `git log -S`, matching the user's report to
the commit. **Assigned to EP-031, to be scoped at the next sprint's planning** (user ruling) together with
adding `-fsanitize=undefined` to CI, which may change the test configuration. Prior note follows.)\*

\*2026-08-11 (I-0112 opened, then **root cause corrected**. Filed as a suspected live-appearance-switch
staleness bug; the user disproved that within minutes — Dark Mode had been active for hours and the app was
launched minutes before the defect was seen, so no switch was involved. Confirmed cause is static: body-text
attribute dictionaries omit `.foregroundColor` (`:517`, `:296-298`) and `textColor` is never set, so AppKit
renders body runs as literal `NSColor.black` against an adaptive dark background. Manuscript-only, as the sole
AppKit text surface. Sprintless/unassigned. I-0017/I-0018/I-0058 unchanged.)\*
