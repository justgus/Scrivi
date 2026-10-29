# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| **I-0322** | `[Linux]` ⚠️ **Linux has no "Project File Not Available" handling at all ([T-0570] was Apple-only): no banner when the project's drive goes away, and no "Some changes can't be saved" question on close or quit.** Found 2026-10-24 by reading, during [SP-193] ([I-0306]). → [details](#i-0322) | High | Not Assigned |
| **I-0323** | `[ScriviCore]` ⚠️ **An older Scrivi opening a project from a newer one shows a raw parser error** (`[json.exception.parse_error.101] … attempting to parse an empty input`), not a plain message. Found 2026-10-24 in [SP-193]'s Mac live pass (an old `/Applications` build). → [details](#i-0323) | Medium | Not Assigned |
| **I-0223** | `[ScriviCore]` ⚠️ **A WORLD CAN BIND TO A DIFFERENT *VERSION* OF ITSELF, SILENTLY — the core detects world IDENTITY but has no notion of world STATE.** ⛔ **REWRITTEN 2026-09-25 BY USER RULING. ⚠️ THE ORIGINAL RECORD WAS SUBSTANTIALLY WRONG AND ITS TEXT IS SUPERSEDED — see 'WHAT THE ORIGINAL CLAIMED' below, kept because the error is instructive.** ---- ✅ **THE USER'S FRAMING, WHICH IS THE SPECIFICATION:** ⚠️ ***"The World's filename and internal ID must match the Project's expectation or else the World Load will be rejected. That will account for every world except a copy of the world or an earlier version of the world. A copy of the world is assumed to be identical... If the mounted file is a mis-matched version (older or newer) of the previous linked world, then an honest reconciliation should take place."*** ---- ✅ **THE IDENTITY CHECK IS REAL, AND IT RUNS ON EVERY CANDIDATE INCLUDING THE RELATIVE ONE.** ✅ **`WorldStore.cpp:428-436`:** ⚠️ ***"IDENTITY CHECK. A package whose worldID differs is NOT this world — resolution stops rather than silently substituting a same-named package. A world's name is a label; its worldID is its identity."*** — ✅ **a mismatch returns `WorldStatus::missing`, NOT a bind.** ✅ **`relinkWorld` enforces the same at `:492-498` with a `worldIDMismatch` detail.** ---- ⚠️ **SO THE SPACE PARTITIONS THREE WAYS, AND ONLY THE THIRD IS A DEFECT:** ✅ **(1) A DIFFERENT WORLD → REJECTED on `worldID`. ⛔ Not a defect; already correct.** ✅ **(2) A COPY of the world → identical `worldID`, identical content, binds and resolves every object reference. ⛔ NOT A DEFECT — a copy is assumed identical and linking to it is FINE (user ruling).** ⛔ **(3) A MIS-MATCHED VERSION of the SAME world (older or newer) → `worldID` MATCHES, so it BINDS SILENTLY, and the core CANNOT NOTICE it is not the version the project last saw. ⚠️ THAT IS THIS ISSUE, ENTIRELY.** ---- ⚠️ **WHY THE CORE IS STRUCTURALLY BLIND TO (3): `worldID` IS IDENTITY, NOT STATE.** ✅ **It answers *"is this the same world?"* and CANNOT answer *"is this the same version of it?"* — ⚠️ **two different questions, and only the first has a mechanism.** ---- ✅ **THE MATERIAL GAP, CONFIRMED IN THE SCHEMAS 2026-09-25:** ⚠️ **`world.json` ALREADY CARRIES `modifiedAt` AND `formatVersion`** (`WorldJson.cpp:25-26`, `WorldRecord` in `worlds/WorldTypes.hpp`) — ⛔ **BUT `WorldBindingRecord` RECORDS NEITHER.** ✅ **It stores only `worldID`, `displayName`, `epochOffsetMs`, `reference` and `cachedIndex`** (`WorldJson.cpp:82-98`). ⚠️ **So the binding has NO RECORD OF WHICH VERSION IT BOUND, and divergence is undetectable by construction — ⛔ not merely unchecked.** ---- ✅ **WHAT THE WORK IS (two capabilities, and the second NEEDS the first):** ✅ **(a) CHANGE-CONTROL DETECTION IN THE WORLD ITSELF** — ⚠️ **something that distinguishes VERSIONS of one `worldID`: a monotonic revision counter or a content hash, written into `world.json` AND recorded in the binding at bind time.** ⛔ **Without this the core cannot detect the case at all, no matter what resolution does.** ⚠️ **`modifiedAt` ALONE IS NOT SUFFICIENT EVIDENCE — a timestamp can go backwards, be preserved by a copy, or be rewritten by sync; ✅ whether it is a usable input is part of the work, not an assumption.** ✅ **(b) HONEST RECONCILIATION** — ⚠️ **when the bound revision and the found revision differ, REPORT IT and say WHICH DIRECTION (older / newer / divergent); ⛔ do not silently pick.** ✅ **This is exactly the two standing rules this code already follows: *"core reports, app decides"* ([SP-141] ruling) and §6a.0's *absence is never deletion*.** ---- ⚠️ **SCOPE WARNING: (a) IS A SCHEMA CHANGE to `world.json` and to the binding, so it needs a MIGRATION story for worlds predating the field** — ✅ **and `WorldRecord::kSupportedFormatVersion` (currently `1`) is the existing mechanism for exactly that.** ⚠️ **THAT is what makes this larger than a resolution patch and why it wants real design, not an incidental fix.** ---- ⚠️ **THE UNMOUNTED-DEVICE VARIANT FOLDS INTO (3), IT IS NOT SEPARATE:** ⛔ **the hazard is NOT resolving to a wrong world (identity blocks that) — ✅ it is resolving to a STALE COPY of the RIGHT world on some other reachable volume while the current one is unmounted.** ⚠️ **Identity passes; nothing flags the divergence.** ---- ⛔ **WHAT THE ORIGINAL RECORD CLAIMED, AND WHY IT WAS WRONG** (⚠️ **kept deliberately**): ⚠️ **it claimed a cross-volume relative path could make the core *"bind the WRONG WORLD and report success"*, severity framed as the [I-0181] class (a wrong answer asserted confidently).** ⛔ **THAT CANNOT HAPPEN — the identity check at `:428` rejects it.** ⚠️ **THE METHOD FAILURE: the 2026-09-22 re-confirmation (*"CONFIRMED STILL LIVE"*) read the CANDIDATE ORDERING at `WorldStore.cpp:305-315` and STOPPED THERE — ✅ it never read the 120 lines further down where the check lives.** ⚠️ **A partial code read produced a confident wrong severity that this record then carried for three days.** ✅ **THE RELATIVE-FIRST ORDERING IS CORRECT AND STAYS** (⚠️ *it is what protects a project and its worlds moved TOGETHER*). ---- ⛔ **THE ORIGINAL ACs ARE VOID:** ⚠️ **old AC2 (*"verify the resolved package IS the bound world"*) IS ALREADY IMPLEMENTED at `:428`; ⚠️ old AC1 (*"skip a meaningless relative candidate"*) addresses a hazard the identity check already closes.** ✅ **[EP-044] MUST BE RE-SPECIFIED around (a) and (b) above.** ---- ⚠️ **SEVERITY RE-REASONED, NOT INHERITED: *"silently binds a STALE VERSION, which is RECONCILABLE"* is a lesser and different thing than *"silently binds the WRONG WORLD"*.** ✅ **It is also arguably NO LONGER the [I-0181] family at all** (⚠️ *that family is about asserting a wrong answer from absent evidence; this is about having no evidence to assert from*). | **Low** — ⚠️ **DROPPED FROM MEDIUM 2026-09-25 (user ruling), re-reasoned NOT inherited:** ⛔ **the Medium came from the FALSE wrong-world claim;** ✅ **"silently binds a STALE VERSION, which is RECONCILABLE" is lesser and different.** ⚠️ **No data is lost and no wrong world is bound — ✅ the writer sees a world that is genuinely theirs, at a state they did not choose.** | [EP-044] — no Sprint (user, 2026-10-24: kept in the backlog, *"I don't think they pose a major threat to data"*) |
| **I-0192** | `[ScriviCore]` ⚠️ **A world on a PHYSICALLY REMOVED volume resolved `available`, and a full object read SUCCEEDED through it, until a scene change forced a re-resolve.** ⚠️ **Found by the USER on the REAL RIG during T-0477's S3 physical yank, 2026-09-07.** ⚠️ **RE-SCOPED 2026-09-07 from `[Linux]` to `[ScriviCore]` after reading the code — ⚠️ MY ORIGINAL DIAGNOSIS WAS WRONG.** ⚠️ **I filed this as an app-layer 'cached status with no invalidation' and as a placeholder dialog reusing a name it already held. ✅ **The code says otherwise, and the truth is WORSE:** `EditorShell::onOpenObjectRequested` (`EditorShell.cpp:~1790`) calls `bridge_->openObject(...)`, checks `lastCallFailed()`, and ⚠️ **parses the displayed name out of the `objectJson` THAT CALL RETURNED** — its own comment reads *"the object is genuinely read here."* ⚠️ **There is no app-side status cache to go stale.** ✅ **The read is guarded end-to-end**: `ObjectStore::open` → `findByID` → `kindDirFor` (`ObjectStore.cpp:~250`), which calls `WorldStore::resolve()` and ⚠️ **refuses unless status is `available`.** ✅ **And `resolve` caches NO verdict** — it sets `available` only after actually reading and parsing `world.json` from the candidate path (`WorldStore.cpp:337-342`). ⚠️ **SO THE DEFECT IS NOT STALENESS ANYWHERE — the filesystem itself answered successfully for a volume that was physically gone**, and every layer above correctly trusted a correct answer. ⚠️ **The likely mechanism is the PAGE CACHE / unreaped dentries**, which ✅ **the 2b container pass ALREADY measured in a stronger form**: a held FD survived `umount -l` + `losetup -D` entirely, reading and writing fine while the PATH broke instantly. ⚠️ **THIS IS I-0181's SIBLING, NOT ITS OPPOSITE**: I-0181 is `resolve` inferring absence it cannot prove; ⚠️ **I-0192 is `resolve` inferring PRESENCE it cannot prove** — ✅ **and `read succeeded` is no more proof of a mounted volume than `directory exists` is proof of a deleted one.** ⚠️ **T-0498's `st_dev` primitive is plausibly the SAME fix for both** — ⚠️ **but that must be MEASURED, not assumed.** ⚠️ **PARTIALLY MEASURED 2026-09-07 (over SSH, drive already out): the STEADY STATE is CORRECT** — `scrivi_get_world_status` returns ✅ **`unavailable`** (not `missing`), with `packagePath` empty and `lastKnownPackagePath` preserved, ⚠️ **stable across repeated calls**, and ✅ **`binding.json` stores no status key at all** — ⚠️ **so there is no persisted verdict to go stale.** ⚠️ **Therefore I-0192 is a TRANSIENT WINDOW, not a stuck verdict** — ⚠️ **severity lowered accordingly.** ⚠️ **The decay-vs-persist question across the yank itself is STILL OPEN and needs the sampled run.** | **Low** | [EP-044] — no Sprint (user, 2026-10-24: kept in the backlog, *"I don't think they pose a major threat to data"*) |
| **I-0317** | `[Linux]` ⚠️ **Drag-selecting toward the top or bottom (autoscroll) freezes the window for up to 25.6 s over the share: every autoscroll step that enters a new scene moves the Inspector to it on the GUI thread (50–290 ms each, back to back).** Found 2026-10-23 in the rig log during [SP-188]'s live pass (not that Sprint's code). → [details](#i-0317) | High | Not Assigned |
| **I-0318** | `[Linux]` ⚠️ **After a large cross-scene cut lands, the caret is restored to a position past the end of the shortened document** (`QTextCursor::setPosition: Position '1846043' out of range`). Nothing visible broke. Found 2026-10-23 in the rig log ([SP-188] live pass). → [details](#i-0318) | Low | Not Assigned |
| **I-0308** | `[Linux]` ⛔ **The app CRASHED while the writer typed in the Find bar** (dumas-prose-timelines over the share, after a cross-scene paste, a quit, a reopen and an undo of the paste). Found on the rig 2026-10-15. ⚪ **Not reproducible** (user, 2026-10-22): not reproduced in Docker under ASan, did not recur on the rig (build 75). → [details](#i-0308) | **High** | Not Assigned (removed from [SP-185] 2026-10-22) |
| **I-0301** | `[Apple]` ⚠️ **At the beginning of the manuscript, scrolling down jitters and the view does not move — as if something keeps putting it back.** Reported by the user 2026-10-13 during [SP-180]'s live pass. ✅ Caught in the diagnostics 2026-10-17 (after a Show chapter titles toggle): the view snaps back about 35 pt mid-gesture, with no Scrivi scroll call logged. Cause not yet named. → [details](#i-0301) | **Medium** | — (detached from [SP-180] 2026-10-14) |
| **I-0286** | `[Linux]` ⚠️ **A large aggregate dot cannot show all its members: the fan-out is painted inside the timeline panel and clipped at its edges.** Reported by the user 2026-10-10 (rig, dumas, during [SP-173]; not related to it). → [details](#i-0286) | Medium | Not Assigned |
| **I-0284** | `[Apple]` ⚠️ **A paragraph containing a HARD BREAK renders NO emphasis — its `**`, `*`, `_` markers show raw.** ✅ **MEASURED 2026-10-08** (found by [SP-171]'s AC1 corpus test; `MarkdownBlocks.analyze`): `"A line\nthen _under_ and **b**."` → 4 markers; `"A hard break\\\nthen _under_ and **b**."` (the Option-Return form, [T-0584]) → **0 markers**. The presenter and the presented map agree (both show the markers), so it is the ANALYZER's result, not the drawing. ⚠️ Not yet checked: whether the whole block is affected or only lines after the break; whether headings/lists in such a block are affected; Linux (md4c) behaviour. ✅ Repro: type a bold word, then Option-Return, then another bold word in the same paragraph. ⚠️ Not in SP-171's scope (filed to the backlog first). | Medium | Not Assigned |
| **I-0275** | `[Apple]` ⚠️ **Keystroke and caret-move cost grows LINEARLY with position in the manuscript — ~20–43 ms at the start, ~89–122 ms at the end of 1.85 MB.** ✅ **MEASURED 2026-10-04** ([SP-157] AC10c, user's run, `dumas-prose-timelines`, 1,855,917 chars): typing 20.0–42.6 ms (scene 0) · 37.1–66.2 ms (scene 291, ~460k) · 89.1–121.7 ms (scenes 1182–1184); Return and ⌫ the same as typing at each place; ⚠️ **arrow keys 0.6–1.7 ms at the start, 89–117 ms at the end — and arrows run NONE of Scrivi's per-keystroke work**, so the scaling is AppKit's (TextKit 2). Scrivi's own work (`[SCRIVI-KEY]`) is flat at 2.3–4.3 ms (→ [T-0583]). ---- ⚠️ **USER'S STANDING (quoted, not inflated):** *"I'm a little worried that we're scanning the whole document at each keystroke"* · *"I didn't say it bites. I said I was kind of concerned."* → *"Let's backlog the Issue for the real cost as well."* ⛔ **NOT ruled as biting.** ---- ✅ **Relation:** [I-0206] (closed not-a-defect) measured `setSel` linear in offset and typing "constant" — ⚠️ this shows typing and arrows are linear too ([I-0206] measured typing at one place). ⚠️ Consistent with [I-0267]'s unexplained fast/slow runs being a caret-POSITION difference — unproven. ---- ✅ **Next step: an Instruments time profile of ONE keystroke and ONE arrow at the end of the manuscript**, to name the AppKit call that grows with position. ⚠️ Only then a design decision; the structural option (not holding the whole manuscript in one text view, with a scene-position map) is Epic-sized. ---- ✅ **BACKGROUND — sync vs async (user asked 2026-10-04; added at the user's request):** ⚠️ **everything per keystroke is SYNCHRONOUS on the main thread, inside `keyDown`**: (1) AppKit's key dispatch → `insertText`; (2) our pre-edit guards in `shouldChangeText` (heading / divider refusal, escape-pair widening); (3) escaping in `insertText`; (4) the storage edit + `EscapeHidingStyler.restyle` (must finish before drawing, or a hidden backslash flashes); (5) **AppKit's own layout / selection / display**; (6) our `textDidChange` bookkeeping (boundary scan, scene lookup, text extract, history `noteEdit`, and on `.`/`!`/`?`/Return a C++ `historyRecordEvent`). ✅ **Already async (debounced `Task`s):** autosave 1 s, Navigator title 300 ms, viewport-scene-after-scroll 120 ms. ✅ **Split at the end of 1.85 MB:** `keyDown` 89–122 ms; Scrivi's (6) 2.3–2.8 ms (≈2.3 ms of it the scan → [T-0583]); (2)–(4) each < 0.5 ms; ⛔ **AppKit ≈ 85–115 ms.** ✅ Arrow keys run none of (2)–(6) and still cost ~90 ms at the end. ✅ **Can it be made async?** ⛔ AppKit's share: NO — `NSTextView`/TextKit are main-thread-only, inside the event; the lever is to find and avoid the offset-linear call, or give the view less text. ⛔ (2)–(4): NO — they decide or change what is stored or drawn. ⚠️ (6): possible, ✅ but after [T-0583] it is ~0.2 ms, and moving the history record off-main risks the EP-019 *"history commits before disk writes"* ordering (§4.d) — not worth it. | Low | Not Assigned |
| **I-0147** | `[ScriviCore]` ⚠️ **KNOWN LIMITATION (user-ruled 2026-08-21, ACCEPTED — not to be fixed in SP-116).** ⚠️ **For up to `kStaleSeconds` (60 s) after an interrupted world write, the world is ENTIRELY UNWRITABLE and its abandoned `.partial` is unreclaimable.** When a volume vanishes mid-import the writer dies holding the lock, leaving `.lock` on disk with a **fresh** heartbeat. Reattach the drive quickly — the natural thing to do — and the next write is refused `worldLocked`; ⚠️ **T-0433's sweep runs only AFTER a successful acquire**, so the orphan survives until the lock ages out. **Observed on the real rig 2026-08-21**: drive pulled mid-import, reattached within ~60 s, next import refused and a **2.9 GB** `.partial` remained. ✅ **Both halves verified**: staged fresh lock + orphan → `worldLocked`, orphan stays; waited past 60 s → **acquired and swept**. ⚠️ **This is arguably CORRECT, which is why it is accepted:** `kStaleSeconds` exists precisely because the core cannot distinguish *"writer died"* from *"writer is briefly stalled"*, and guessing wrong means two processes writing a shared world at once. It **self-heals** within a minute and loses no data. **The stronger evidence available — the package's own VOLUME was unmounted, which is far better proof of a dead holder than a quiet heartbeat — is not currently used.** ⚠️ **Deferred to the network-worlds design**, which must revisit *"exactly one winner"* regardless; ruling that inside an asset sprint is how a locking model gets set by accident (the lesson of I-0144). ⚠️ **The eventual UI must not present the 60 s wait as an error** — it is a retryable state. | Low | ⚠️ **Deferred — network-worlds design.** 🟡 Accepted limitation (2026-08-21); moved to the backlog 2026-10-03 (user). 🟡 **Accepted limitation (2026-08-21)** — ⚠️ **found by the LIVE RIG PASS**; ⚠️ **my own earlier staged-orphan test PASSED because it created the orphan WITHOUT a matching fresh lock** — not the state a real crash leaves |

## I-0322

**Title:** Linux has no project-drive-lost handling ([T-0570]'s Linux half was never built)
**Status:** 🔵 **Open**
**Platform:** `[Linux]`
**Component:** `ScriviWindow::closeEvent` (`platforms/linux/src/ScriviWindow.cpp:690`), `EditorShell` save path
**Severity:** High. The same loss [I-0306] closed on the Mac: with the project's drive pulled, typed edits can be lost on
close or quit with no question asked.
**Sprint:** Not Assigned
**Date Identified:** 2026-10-24, by reading, while fixing [I-0306] in [SP-193]. Filed by user request (2026-10-25:
*"yes, file both Issues"*).

**What is missing:** [T-0570] (verified 2026-09-30, Apple) gives the Mac three things: the red non-dismissable
*"Project File Not Available"* banner, dirty scenes kept and saved automatically on reconnect, and a Cancel / Quit Anyway
question on close and quit. ⚠️ Linux has none of them. `closeEvent` calls `flushEditor()` and closes; nothing checks
whether the writes landed. ⚠️ What a failed flush does on Linux (logged, silent, retried) is not yet measured.

**Fix direction (not ruled):** port [T-0570]'s shape from Apple (`feedback_linux_adopts_apple_shape`), including the
close guard through every close path (the X, Close Project, Quit; [I-0306]'s lesson). Measure on the rig with the
drive pulled first.

---

## I-0323

**Title:** An older build opening a newer project shows a raw JSON parser error
**Status:** 🔵 **Open**
**Platform:** `[ScriviCore]` (the message), shown by both apps
**Component:** `util::parseJson` (`ScriviCore/src/util/Json.cpp:174`) returns `e.what()` as the error message; the
caller that read an empty file is not yet named.
**Severity:** Medium. Nothing is lost, but alpha testers on builds that fall behind each other will meet it, and the
message tells them nothing.
**Sprint:** Not Assigned
**Date Identified:** 2026-10-24, [SP-193] Mac live pass step 4: a Finder double-click launched `/Applications/Scrivi.app`
(a 2026-09-16 build, process 88249) on a current project. Filed by user request (2026-10-25).

**What happened:** the open failed with *"[json.exception.parse_error.101] parse error at line 1, column 1: attempting to
parse an empty input…"*.

⚠️ **Cause not named.** "Empty input" means the old build read a file that was empty or absent, not a newer schema it
could not understand. Which file is not known: reproduce with that build (or any build before the change) and log the
path before fixing.

**Fix direction (not ruled):** two parts. (1) The parse error names the file and reads as a plain sentence. (2) A
project written by a newer format says so: *"This project was saved by a newer version of Scrivi"*, which needs the
open path to check a format version before reading anything else.

---

## I-0317

**Title:** Autoscroll during a drag-selection freezes the window over the share
**Status:** 🔵 **Open**
**Platform:** `[Linux]`
**Component:** `EditorShell::onScrolled` → `promoteActiveScene` → `SceneInspector::setScene`
(`platforms/linux/src/EditorShell.cpp:1514–1572`)
**Severity:** High. The window stops responding for many seconds during an ordinary gesture (selecting a long passage).
**Sprint:** Not Assigned
**Date Identified:** 2026-10-23, from the rig log (`~/.local/state/scrivi/scrivi.log`), [SP-188] live pass, build 79,
`dumas-prose-timelines` (1,180 scenes) over the share. Filed by user request (2026-10-24: *"file both Issues"*).

**What happened:** between 23:52:55 and 23:54:05 the user drag-selected toward the top / bottom of the manuscript and the
editor autoscrolled. The window stopped answering for 25.6 s, 14.5 s, 8.1 s and 6.9 s.

**Cause (from the log and the code):** each autoscroll step that brings a new scene into view promotes it to the active
scene, and `SceneInspector::setScene` reads that scene's Inspector data synchronously on the GUI thread: 50–290 ms per scene
over the share. A long autoscroll crosses scene after scene, so the event loop never idles.

**Mac (2026-10-24, user):** the same drag-selection on the Mac, on a LOCAL disk, did not freeze. ⚠️ Not yet compared over
the share, and the Mac Inspector's follow-the-scroll code not yet read.

**Fix direction (not ruled):** the Inspector follows the scene only once scrolling settles, and its reads move off the GUI
thread. Read how the Mac Inspector follows `viewportSceneID` first — Linux adopts Apple's shape.

---

## I-0318

**Title:** Caret restored out of range after a large cross-scene cut
**Status:** 🔵 **Open**
**Platform:** `[Linux]`
**Severity:** Low. Qt rejects the position and logs a warning; nothing visible broke.
**Sprint:** Not Assigned
**Date Identified:** 2026-10-23, from the rig log, [SP-188] live pass step 9 (the 49 s cut over most of
`dumas-prose-timelines`). Filed by user request (2026-10-24).

**What happened:** right after the cut's delta was applied, the log shows
`QTextCursor::setPosition: Position '1846043' out of range`. Something restored the caret (or a selection end) to its
pre-cut position, which no longer exists in the shortened document.

**User (2026-10-24):** *"I-0318 may have been me. I pasted the big text near or at the end of the document after the cut.
It warrants another test."* ✅ Added: `structural_baseline_smoke` cuts most of the document (selected forward, then
backward), pastes at its end, undoes both, and fails on any Qt `out of range` warning. ⚠️ It passes (2026-10-24, Docker, Qt 6.4): the warning does NOT reproduce on the smoke's 4-scene project, so the
cause is not yet caught. Next: the same steps on a large generated project.

**Next:** find the caller. ⏳ Not found yet: the reopen restore (`EditorShell.cpp:883–892`) is clamped; the unclamped
`setPosition` calls in `ManuscriptEditor.cpp` and `EditorShell.cpp:3064` are the candidates. Reproduce with a cut over most
of a large document in `structural_baseline_smoke`, with Qt warnings made fatal.

---

## I-0308

**Title:** Crash while typing in the Find bar
**Status:** ⚪ **Not reproducible** (user, 2026-10-22) — kept open in the backlog
**Platform:** `[Linux]`
**Severity:** High. The app closed with no warning; the writer's unsaved typing (if any) would be lost.
**Sprint:** Not Assigned (removed from [SP-185] 2026-10-22, user)
**Date Identified:** 2026-10-15, user, rig live pass (build 74 by its stamp; the code is commit `8ebb438`, SP-182 included)

**What happened (user):** *"Find opens with Replace row on Ctrl-Alt-F. App crashed while typing in the Find bar."*
- Before it, in the same session: a cross-scene Ctrl+C / Ctrl+V in dumas-prose-timelines (step 9), File ▸ Quit, reopen,
  and Ctrl+Z of the paste (step 10), so the manuscript had been rebuilt by two reloads.
- ⛔ No trace: `coredumpctl` is not installed on the rig and `/var/crash` is empty. The user: *"The app simply 'went away'."*
- The user, 2026-10-17: likely intermittent, *"it only occurs under certain circumstances that we haven't fully identified
  yet."*

**Next:** reproduce. Either the same steps on the rig under `gdb` (or with `coredumpctl` installed), or the same sequence in
Docker on a large generated project. Suspects, by reading only: the find bar's presented-text build after a reload, and
SP-182's presenter changes (chapter-title lines). ⏳ Not yet proven either way.

- **2026-10-17 — the Docker reproduction: NOT reproduced.** The real dumas-prose manuscript (1,153 scenes), Crimson Pro 18 pt,
  the rig pass's steps 7–11 through a real `EditorShell` under AddressSanitizer + UBSan (Qt 6.4.2, arm64, offscreen): the
  cross-scene copy and paste, a quit and reopen, the undo from the previous session, Ctrl+Alt+F, then 9 searches typed key by
  key (accented names, `e`, `the`), backspacing, and typing in the manuscript with the bar open. No memory error; the app
  survived.
- Ruled out in that setting: a use-after-free or overflow in the find bar's presented text or in SP-182's chapter-title
  drawing on this path. ⚠️ NOT ruled out: what Docker does not have — Qt 6.10.2 (the rig's), x86-64, Wayland and its primary
  selection, the network share's timing, the real dumas-prose-timelines (with timelines).
- Found on the way: [I-0309] and [I-0310], both crashes, both fixed. The diagnostic log ([T-0612]) now records a crash's
  backtrace on the rig.
- **2026-10-21 — did not recur; kept Open (user).** The rig pass on build 75 ran the same steps 9–11 without a crash. The
  user: *"keep I-0308 open. It is quite likely that the Find bar crash is related to background read collisions and will
  be taken care of in an upcoming Epic."* ⏳ No work now: the user opened a discussion of background task queuing and
  priorities (repeated gestures while an operation is pending; a Ctrl+Z queued behind the operation it undoes) and said
  *"let's not 'do' anything yet."*
- **2026-10-22 — not reproducible; removed from [SP-185] (user).** Not reproduced in Docker (2026-10-17) and not recurred on
  the rig (build 75, 2026-10-21). Moved to the backlog with no Sprint. The user's 2026-10-21 lead stands: background read
  collisions, for an upcoming Epic.

---


## I-0301

**Title:** At the start of the manuscript, scrolling down jitters and the view does not move
**Status:** 🔵 **Open — not reproduced** (2026-10-14). Ruled into [SP-180] (user: *"Regardless we should fix it right now."*); the instrumented re-run scrolled smoothly (user: *"It didn't do it this time … we'll need to keep an eye on it."*)
**Platform:** `[Apple]`
**Severity:** Medium — the writer cannot scroll away from the start of the manuscript while it happens
**Sprint:** — (found during [SP-180]'s rig live pass; detached and moved to the backlog 2026-10-14, user: *"detach I-0301 from SP-180 and move it to the backlog."*)
**Date Identified:** 2026-10-13 (user: *"Also on Mac, When at the beginning of the document. Scrolling down creates some jitter, and
the viewport does not move, as if the program is not letting the scroll happen over and over."*)

**Context:** dumas-prose-timelines (1,186 scenes, 1.8 MB), opened from local disk on the Mac. The caret was moved to the
beginning of the manuscript, edits were made, then the writer scrolled down.

**Lead, NOT a diagnosis:** [I-0273]'s restore re-centring (`pendingRestoreCentre`, `ManuscriptTextView.swift`). It re-centres
on any clip or document frame change while its target is off screen, and ends at the writer's first key, click or scroll, or
after 5 s. A scroll that does not end it, or another programmatic scroll answering every frame change, would look exactly like
this. ⚠️ [I-0204]'s history: three code-read diagnoses were wrong before `sample` found the cause. Reproduce and instrument
before naming one.

**2026-10-14:** `[SCRIVI-SCROLL]` diagnostics were added, in Debug builds only (`scrollDiag`). They log the scroll wheel, every
`scrollRangeToVisible` with its callers, the three explicit clip scrolls, the restore re-centre, and any document-height
change or jump of more than 150 pt.
- The user's re-run (Go ▸ Manuscript Start, ⇧⌘0; edits; scroll down) scrolled smoothly.
- The diagnostics stay, so the next occurrence brings evidence: paste the console lines filtered on `SCRIVI-SCROLL`.

**2026-10-17: caught (user, Mac, dumas-prose-timelines, local disk).** The user: *"I ran into the scrolling issue again …
It seemed to be related to showing the Chapter names … I toggled the switch, the Chapter headings appears and the scroll
issue happened. I was unable to reproduce it after that though."*
What the `[SCRIVI-SCROLL]` lines show:
- **The snap-back.** Near the top of the manuscript, scrolling down moves the view about 35 pt, and then it is put straight back,
  over and over: clip y 72 → 108, back to 72 (more than ten times); later 120 → 196 back to 120, and 49 → 80 back to 50. Each
  snap lands mid-gesture, between two wheel events moving the other way.
- **Not Scrivi's explicit scrolls.** No `scrollRangeToVisible`, no explicit clip scroll and no restore re-centre is logged at any
  snap; all of them are instrumented. The restore re-centre had ended at the first wheel event, as designed.
- **The toggle was off and back on** (user). No mismatch with the setting: the view was near the end of the manuscript, where no
  chapter starts were on screen, so it showed no titles. Two rebuilds, which the log also shows.
- **The document-height estimate swings at the toggle:** 1,064,023 → 1,293,578 → 926,156 → 589,348 → 851,609 pt within a
  moment, with the view jumping between about 1,290,000 and 588,000. The user then went to the start (`moveToBoundary`, loc 10).
- **Lead, NOT proven:** something outside the instrumented calls moves the clip origin: TextKit 2's own viewport adjustment as it
  re-measures the first lines (the first chapter title, Heading 1, is about one 35 pt line), or an app path that is not logged.
- ✅ **2026-10-17: that logging is added** (user: *"Add the logging, and leave I-0301 in the backlog for now. We are still trying
  to determine the root cause and it occurs intermittently."*). A clip move of more than 4 pt in the same direction as the last
  wheel delta, within 0.3 s of it, logs `[SCRIVI-SCROLL] AGAINST WHEEL …` with 18 frames of call stack. The bounds-change
  notification is posted from inside the origin change, so the stack names the caller (`Coordinator.scrollDidChange`,
  `ManuscriptNSTextView.scrollWheel`; Debug builds only). Mac build ✅, interop 292/292 (1 on-demand test skipped).
  ⏳ Next occurrence: paste the `AGAINST WHEEL` lines.

---

## I-0286

**Title:** On Linux, a large aggregate dot's fan-out is clipped by the timeline panel's edges, so not all its members can be seen
**Status:** 🔴 Open
**Platform:** `[Linux]`
**Component:** `TimelinePanel` (aggregate fan-out)
**Severity:** Medium
**Sprint:** Not Assigned
**Date Identified:** 2026-10-10 (user, on the rig with dumas; found during [SP-173], unrelated to it)

**Description (user):** *"the aggregated popup is clipped by the timeline boundaries, meaning that if the aggregate dot [is]
large, it cannot show all its member dots."*

**Expected:** every member of an aggregate can be seen and reached, however many there are — as on Apple, where the members
open in a popover outside the timeline strip, with its own size bound ([I-0208]).

**Actual:** the members are drawn as a fan of dots INSIDE the timeline widget, around the aggregate
(`TimelinePanel::paintFanOut`, `TimelinePanel.cpp:954`). Its radius grows with the member count (`fanRadius`, `:406`;
positions from `fanOutMemberPos`, `:382`), so a large aggregate's fan runs past the panel's top and bottom (the strip is short)
and past its ends, and Qt clips whatever falls outside the widget.

**Root cause (read from the code, not yet measured):** the expansion is painted in the panel, so it can never be larger than the
panel. ⚠️ Apple's shape is a separate surface (a popover) bounded independently (`Scrivi_Timeline_Clustering_Design_v0_1.md`:
*"an INDEPENDENT popover size bound"*); Linux adopts Apple's shape (`feedback_linux_adopts_apple_shape`), so the fix direction is
a pop-up window, not a bigger fan. ⛔ Not ruled.

**Related (not duplicates):** [I-0208], [I-0210], [I-0212] — the Apple aggregate popover (size, first/last dot, tooltips), all
verified; none covers Linux.

✅ **[I-0221] and [I-0222] were filed, fixed AND user-verified 2026-09-17/18**, then archived to
[`Verified/Issue-verified-0221-0230.md`](Verified/Issue-verified-0221-0230.md) **in the same step**
(`feedback_archive_on_close`). ⚠️ **Both came from ONE live pass on a FAT32 volume, and neither was
reachable by the test suite as it stood.**

✅ **I-0191 moved to `Issue-active.md` 2026-09-07** — fixed the same day it was filed; ✅ **user-verified 2026-10-03** → `Verified/Issue-verified-0191-0200.md`.

---

*Last Updated: 2026-10-24 — **I-0223** and **I-0192** moved here from `Issue-active.md` (user: kept in the backlog; not a major threat to data). Their records are in [EP-044]'s file.*

*Last Updated: 2026-10-24 — **I-0313** and **I-0202** assigned to [SP-193] (fix before the alpha, user) and moved to `Issue-active.md`.*

*Last Updated: 2026-10-24 — **I-0312 CLOSED — OBE** (user): no project uses the legacy undotted keys → `Closed/Issue-closed-0312.md`.*

*Last Updated: 2026-10-24 — **I-0317** and **I-0318** `[Linux]` filed (user) from [SP-188]'s rig live pass log: the autoscroll freeze and the out-of-range caret.*

*Last Updated: 2026-10-21 — **I-0315** moved to `Issue-active.md` (assigned to SP-191, user).*

*Last Updated: 2026-10-21 — **I-0315** `[Linux]` filed: a scene switch rewrites the previous scene's outline and to-do (found tracing [SP-191]'s GUI-thread calls).*

*Last Updated: 2026-10-21 — **I-0314** moved to `Issue-active.md` (assigned to SP-191).*

*Last Updated: 2026-10-21 — **I-0314** `[Apple]` filed: a Mac chapter split writes the scenes out of order on disk (found planning [SP-191]).*

*Last Updated: 2026-10-20 — **I-0313** `[Apple]` filed: typing into an empty scene before another empty one is credited to the next (found by [SP-187]'s test).*

*Last Updated: 2026-10-20 — **I-0312** `[ScriviCore]` filed: `keyBetween` mis-places a key next to an undotted legacy key (found testing [SP-186]).*

*Last Updated: 2026-10-12 — **I-0288 → active** (user: *"fix it inside SP-178"*; it left this file).*

*Last Updated: 2026-10-12 — **I-0288 filed** ([SP-178], measured through `scrivi_*`): after an outside edit, undo in a later session restores a mix of old and new text. Ruling owed.*

*Last Updated: 2026-10-11 — **I-0287 → active** (ruled: fix in [SP-178]; resolved, not verified — it left this file).*

*Last Updated: 2026-10-11 — **I-0287 filed** ([SP-178] `undo_smoke`): after a merge or split, undoing the next edit restores the pre-structural text (proven at the ABI).*

*Last Updated: 2026-10-08 — **I-0284 filed** ([SP-171] AC1 corpus test): a hard break in a paragraph switches off its emphasis rendering.*

*Last Updated: 2026-10-08 — **I-0277 → active** ([SP-171] activated; it left this file).*

*Last Updated: 2026-10-08 — **I-0283 → active** (ruled (a), fixed now; it left this file).*

*Last Updated: 2026-10-08 — **I-0283 filed** (user, reproduced): macOS Smart Quotes rewrite stored text outside the escape layer; ruling owed.*

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
