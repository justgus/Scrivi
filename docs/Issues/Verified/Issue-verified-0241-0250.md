# Verified Issues: I-0241 – I-0250

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0231–I-0240 are in [`Issue-verified-0231-0240.md`](Issue-verified-0231-0240.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0241** | `[Linux]` ⚠️ **Linux reads historical-event tags STRAIGHT OFF DISK, scanning every file in `objects/historical-events/` to find one event.** ⚠️ **FOUND 2026-09-21 while designing [SP-149]'s guard — ⛔ not by a test, and it is in NO tracking layer.** ✅ **Site: `readHistoricalEventTagsFromDisk()`, `platforms/linux/src/EditorShell.cpp:2469`** (called by the historical-event Edit dialog to prefill tags). ⚠️ **IT IS THE [I-0197] BYPASS SHAPE, ON LINUX** — ⛔ **app-side code reading a project package directly, in plain violation of *"no backend logic is reimplemented"*.** ⚠️ **ITS OWN COMMENT ADMITS THE PROVENANCE:** *"the same read-the-file pattern Apple uses for imported-timeline events"* — ⛔ **and APPLE HAS SINCE RETIRED THAT PATTERN** ([SP-129] / [T-0502]; the tombstone is at `TimelineStripView.swift:567-577`). ✅ **So Linux is carrying a bypass Apple has already removed** — ⚠️ **exactly the drift `feedback_linux_adopts_apple_shape` exists to prevent.** ✅ **ROOT CAUSE IS THE SAME AS APPLE'S WAS, AND IT IS A REAL GAP: `scrivi_list_historical_events` DROPS `tags`**, ⚠️ **so the endpoint alone cannot feed the dialog.** ⛔ **DO NOT FIX THIS BY WRITING A BETTER QT PARSER.** ✅ **[SP-129] fixed Apple's by EXTENDING THE CORE**, ⚠️ **and that is the shape the fix must take here** — ✅ **Apple's own tombstone predicted this case: *"which is exactly why Linux reads the files too."*** ⚠️ **ALSO A COST DEFECT: it lists and parses EVERY file in the directory per lookup**, ⛔ **the read-amplification class `project_read_amplification_class` records.** ⚠️ **Severity Medium, not High: it is a PREFILL path (wrong/missing tags in an edit dialog), not data loss** — ✅ **but it is unbounded work on a directory that grows with the project.** ⚠️ **SCOPE WIDENED 2026-09-21 on inspection — THERE ARE TWO CALLERS, AND THE SECOND IS A LATENT DATA-LOSS PATH.** ✅ **`onEditHistoricalEventRequested` (`:2518`) is the PREFILL case originally filed.** ⛔ **BUT `onHistoricalEventDragged` (`:2495`) re-reads tags off disk ONLY to re-send them**, ⚠️ **because `updateHistoricalEvent` OVERWRITES ALL FIELDS and a partial update would blank them** — ✅ **the code's own comment says so.** ⛔ **SO IF THE DISK READ RETURNS EMPTY FOR ANY REASON — a renamed file, a permissions error, an unparseable sidecar — DRAGGING A DOT SILENTLY ERASES THAT EVENT'S TAGS.** ⚠️ **`readHistoricalEventTagsFromDisk` returns an empty list on EVERY failure and the caller cannot tell "no tags" from "could not read"** — ✅ **`project_envelope_empty_vs_failed`, exactly.** ✅ **THE FIX IS ONE LINE IN THE CORE: `ScriviCore.cpp:919-926` ALREADY reads and parses every event file and projects SEVEN fields — ⛔ it simply DROPS `tags`.** ⚠️ **`create`/`update` both ACCEPT `tagsJSON`, so tags can be WRITTEN and never READ BACK** (`project_capability_without_surface`). | **Medium** | ✅ **[SP-142] / T-0542** — ⚠️ **assigned 2026-09-21 by user ruling as its OWN Task, a sibling of T-0537 (NOT folded into it)** | ✅ **VERIFIED 2026-09-21 (user live pass, T-0542)** — ⚠️ **THE DEFECT WAS TWO BUGS, NOT ONE.** ✅ **(1) `listHistoricalEvents` now PROJECTS `tags` (`ScriviCore.cpp`).** ⛔ **(2) `create`/`update` NEVER STORED THEM EITHER** — ⚠️ **both called `getStringArray("tags")` on the payload, which reads an array UNDER a key while `scrivi.h` documents a ROOT array**, ✅ **so the documented shape could never work** (⚠️ **a dead `arraySize("tags")` block beside it shows the gap was seen and left**). ⚠️ **FIXING IT NAIVELY WOULD HAVE BROKEN LINUX: it sends `{"tags":[…]}` via `tagsToJson`, which the OLD BUGGY CODE happened to read** — ✅ **both shapes now accepted through ONE helper, each asserted, and `scrivi.h` corrected to document both.** ✅ **Added `JsonDoc::rootStringArray()`, the accessor whose absence caused bug 2.** ✅ **`readHistoricalEventTagsFromDisk` DELETED; both callers read the cache the projection fills; TOMBSTONE left at the site.** ✅ **EVIDENCE: `ctest` macOS 626/626, LINUX NON-ROOT 630/630; Linux smokes 24/24; 5 new ABI tests + 2 new drag assertions, ⚠️ ALL VERIFIED FAILING against the removed projection.** ✅ **LIVE PASS PASSED 2026-09-21 (user, real rig): an event with tags was DRAGGED and its tags SURVIVED.** ✅ **VERIFIED.** |
| **I-0243** | `[Apple]` ⚠️ **THE PROJECT TITLE IS RENDERED THREE TIMES IN ONE WINDOW, from THREE INDEPENDENT SOURCES.** ⚠️ **Found by the USER's live pass on [SP-134], 2026-09-22** — ⚠️ **the Sprint added a toolbar and the duplication became obvious.** ✅ **MEASURED:** ⚠️ **(1) the TAB HEADER lozenge, from `window.title`** (`ProjectWindowManager.swift:110`, AppKit) · ⚠️ **(2) the NAV BAR title, from `.navigationTitle(projectTitle)`** (`EditorView.swift:190`, `:194`, `:242`, SwiftUI) · ⚠️ **(3) a HAND-CODED header pinned above the scene list, from `Text(prefs.projectTitle)`** (`SceneNavigatorView.swift:64-73`, and it does NOT scroll so it is always visible). ⛔ **THREE MECHANISMS, ONE FACT — which is why no single deletion fixes it.** ⚠️ **THE TAB HEADER IS NOT A DESIGN DECISION.** ⛔ **`tabbingMode` is NEVER SET ANYWHERE in `Scrivi/`** — ✅ **so macOS's default `.automatic` applies**, ⚠️ **which tabs windows in FULL SCREEN and not outside it.** ⚠️ **Hence the behaviour the user observed: *Open Project* makes a NEW WINDOW when windowed and a NEW TAB when full-screen** — ⛔ **the same command doing two different things depending on a mode nobody chose.** ✅ **USER RULING 2026-09-22 — OPTION A, WINDOWS ONLY:** ⚠️ ***set `tabbingMode = .disallowed`***. ✅ **Full screen then behaves like windowed, and [EP-018]'s per-window model is the ONLY model.** ⚠️ **WHY NOT TABS:** ✅ **a tab bar teaches *"these are peers within one workspace"*, which is what a MULTI-document app looks like** — ⛔ **and the app-shape study ruled Scrivi a SINGLE-DOCUMENT editor** (⚠️ **one `NSTextView` holding all 1.8M characters; the scene is a structural unit of the document, not a document**). ✅ **One manuscript per window says that honestly.** ⚠️ **NOTE THE PHILOSOPHY IS NOT BEING REVISED: *one manuscript* and *one window* are DIFFERENT claims**, ✅ **and [EP-018] (closed, verified) deliberately delivered *"multiple distinct projects open at once, each in its own window"*.** ⛔ **Tabs were a THIRD way of showing multiple projects, arriving by ACCIDENT alongside the two that were designed.** ✅ **THE FIX IS SMALL, and the user sized it correctly:** ⚠️ **(a) one line — `window.tabbingMode = .disallowed` in `ProjectWindowController`, which removes source (1); ✅ (b) delete `projectHeader` and its single call site (`SceneNavigatorView.swift:58`, `:64-73`), which removes source (3).** ⛔ **VERIFIED: `projectHeader` has exactly ONE call site and nothing else references it; ⛔ nothing in `Scrivi/` depends on window tabs at all.** ✅ **`.navigationTitle` survives as the ONE title.** | **Low** | ✅ **[SP-135]** — ⚠️ **candidate; the bars/layout Sprint already owns the title area, and [EP-040] AC1 is not fully judged until this is resolved** | ✅ **VERIFIED 2026-09-22 (user live pass)** — ✅ **all three claims confirmed: the triplication is gone, the tab bar is gone, and *Open Project* in full screen now opens a NEW WINDOW.** ✅ **Fixed by [T-0544]: `window.tabbingMode = .disallowed` removed source (1); `projectHeader` DELETED removed source (3); `.navigationTitle` survives as the ONE title.** ⚠️ **`prefs` KEPT deliberately — it was `projectHeader`'s only reader, but removing it means changing the initialiser and BOTH platform call sites, which is churn beyond this Issue.** ✅ **[EP-040] AC1 is now FULLY MET.** |


---

## ⚠️ I-0241 — what it turned out to be

⚠️ **FOUND WHILE PLANNING A SPRINT, NOT WHILE RUNNING ONE.** ✅ **It surfaced during [SP-149]'s guard
design — the exercise of asking "what would a guard catch?" caught it before the guard existed.**
⛔ **No test, no review and no tracking layer had it in the months it lived.**

⚠️ **THE FILED ISSUE UNDERSTATED IT.** ✅ **It read "list drops tags"; there were TWO bugs** — the
projection dropped them AND `create`/`update` never stored them, because both parsed the payload with
an accessor that could only read a keyed array while the header documented a root one.
⛔ **So the DOCUMENTED shape had never worked.**

⚠️ **THE NAIVE FIX WOULD HAVE BROKEN THE ONLY LIVE PRODUCER.** ⛔ **Linux sends `{"tags":[…]}`, which
the buggy code happened to read** — ✅ **caught by the test, not by inspection.**

✅ **VERIFIED by the user's live pass 2026-09-21: an event WITH tags was dragged and its tags survived**
— ⚠️ **the data-loss half, checked separately from the Edit-dialog prefill**
(`feedback_verify_each_half_separately`).

✅ **Full record: [`../../Tasks/Verified/Task-verified-0537-0542.md`](../../Tasks/Verified/Task-verified-0537-0542.md).**

---

## ✅ I-0243 — verified 2026-09-22

⚠️ **FOUND BY A LIVE PASS ON THE SPRINT THAT CAUSED IT TO BE NOTICED.** ✅ **[SP-134] added a toolbar;
the third title became obvious the moment the window had real chrome.**

⛔ **THE TAB BAR WAS NEVER A DESIGN DECISION.** ✅ **`tabbingMode` was unset, so macOS's `.automatic`
default applied** — ⚠️ **tabbing in full screen only.** ⛔ **One command, two behaviours, depending on
a mode nobody chose.**

✅ **RULED OPTION A (windows only).** ⚠️ **Recorded because it reads like a philosophy change and is
not:** ✅ ***"one manuscript"* and *"one window"* are different claims** — ⚠️ **[EP-018] deliberately
delivered multiple projects in multiple windows, and tabs were a THIRD way of doing that, arriving by
accident.**

✅ **Full record: [`../../Tasks/Verified/Task-verified-0544.md`](../../Tasks/Verified/Task-verified-0544.md).**
