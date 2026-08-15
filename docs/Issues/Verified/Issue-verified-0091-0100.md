# Verified Issues — I-0091 to I-0100


---

## I-0091

**Status:** ✅ **Verified (2026-07-24, VNC)** — user confirmed both dialogs open in the project's folder; container build green (211/211), `timeline_events_smoke` + app-launch PASS.
**Severity:** Low
**Sprint:** **SP-082**

**Description / Resolution:**
`[Linux]` **Import/Export Timeline file dialogs default to `/root` instead of the project's folder.** Found in the SP-082 VNC verify (2026-07-24, T-0343/T-0344 on `the-twisted-remains-of-myself`). Both `QFileDialog`s opened in `/root` (Qt's `QDir::homePath()`), not `/projects` where the project lives, so the writer had to navigate every time. **Root cause:** `EditorShell::onImportTimelineRequested`/`onExportTimelineRequested` seeded the dialog with `QDir::homePath()`. **Fix:** seed both with `QFileInfo(projectPath_).absolutePath()` — the `.scrivi` package's parent, i.e. the folder the project lives in (NOT hardcoded to `/projects`).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0092

**Status:** ✅ **Verified (2026-07-28)** — user confirmed a new project's folder is created with the `.scrivi` extension.
**Severity:** Low
**Sprint:** **SP-089**

**Description / Resolution:**
`[Apple]` **New Project on macOS creates the project folder without the `.scrivi` extension.** Reported 2026-07-28 (`the-stairs-of-tintagael` created without the suffix; the user renamed it manually). **Root cause:** `NewProjectSheet.choosePath()` (`NewProjectSheet.swift:113-124`) builds the `NSSavePanel` with **no `allowedContentTypes`**, so the panel does not append the `.scrivi` package extension — whatever is in the name field becomes the folder name verbatim, and `chosenPath = url.path` is taken as-is. The comment at `NewProjectSheet.swift:138` ("NSSavePanel already produced the full `<name>.scrivi` path") was false. **Fix:** set `panel.allowedContentTypes = [scriviType]` (the `com.caposoft.scrivi.project` package UTID, matching the Open panel) so the panel enforces the extension, plus a defensive `if url.pathExtension != "scrivi"` append in `choosePath()` for the case where a user still deletes it.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0093

**Status:** ✅ **Verified (2026-07-28)** — user confirmed the project opens showing its real `project.json` title.
**Severity:** Medium
**Sprint:** **SP-089**

**Description / Resolution:**
`[Apple]` **A project opens showing "Untitled" even though `project.json` has the real `title`.** Reported 2026-07-28 for `the-stairs-of-tintagael.scrivi` ("The Stairs of Tintagael" in `project.json` + `manuscript.meta.json`, but the window/editor/navigator all read "Untitled"). **Root cause:** the displayed title comes *only* from `ProjectPreferences.projectTitle` (`ProjectWindowManager.swift:102`, `EditorView.swift:184`, `SceneNavigatorView.swift:46`), which `ProjectPreferences.init` (`ProjectPreferences.swift:24-37`) loads **solely from UserDefaults**, defaulting to `""` when absent. Nothing ever connects `project.json`'s title to the UI: (a) the backend `OpenProjectResult` envelope never emitted the title (`scrivi_open_project`), so on open there was no source to read; (b) `AppEnvironment.createProject` wrote the title into `project.json` via the engine but never seeded `ProjectPreferences`. Net: the on-disk title (source of truth per CLAUDE.md) was fully decoupled from display → "Untitled". **Fix (read-from-schema, the architecturally correct path):** `scrivi_open_project` now emits `projectTitle` (`v.project.title`, already on `ProjectSummary`); `OpenProjectResult` (Swift) decodes it; `ProjectSession` seeds `ProjectPreferences.projectTitle` from it on open when UserDefaults has no stored value — so both existing and newly-created projects show the real title.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0094

**Status:** ✅ **Verified (2026-07-29)** — user ran the rebuilt app on `the-stairs-of-tintagael.scrivi`: cross-boundary ⌘X leaves only the text above/below the selection (2 scenes merge → 1), and ⌘V re-inserts the fragment **once** with the scene boundary preserved (1 scene splits → 2). Console confirmed `fragment held: 2 piece(s), opensWith=["none","scene"]` and `fragmentPaste OK: createdScenes=1 createdChapters=0`.
**Severity:** High
**Sprint:** **SP-089**

**Description / Resolution:**
`[Apple]` **Cross-boundary Cut/Paste silently fell back to a flat native cut/paste → "all text in one scene".** Found in the SP-089 EP-029 live-verify (2026-07-29) on `the-stairs-of-tintagael.scrivi`: a cross-boundary ⌘X then ⌘V dumped everything into one scene (and, pre-fix, duplicated the trailing scene). **Root cause — a JSON-boundary decode mismatch, NOT the paste logic.** The C API's `okEnvelope` **omits empty ID arrays** (`appendStringToArray` only writes a key when it has elements), so a cut that removes no chapters ships no `removedChapterIDs` key. But Swift's `FragmentCutResult.removedChapterIDs` was declared **non-optional `[String]`**, so `JSONDecoder` threw `keyNotFound('removedChapterIDs')`. That threw error made `structuredCutIfCrossBoundary()` return `false` → fell through to `super.cut()` (native AppKit cut: deletes locally, flat text to pasteboard, no structured fragment held). On ⌘V nothing structured was held, so the flat pasteboard text landed in one scene. `FragmentPasteResult` had the identical latent bug (`createdChapterIDs` omitted when a paste creates no chapters). The C++ core + C ABI round-trip were correct throughout (ctest 8/8 paste tests green). **Fix (Swift, tolerant decode):** custom `init(from:)` on both `FragmentCutResult` and `FragmentPasteResult` decodes absent ID arrays as `[]` via `decodeIfPresent(...) ?? []`. `ScriviEngine.swift` only; no `scrivi.h`/core change; no pbxproj change (edited file already tracked).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0095

**Status:** ✅ **Verified (2026-07-29)** — user confirmed a chapter rename in the navigator refreshes the manuscript heading immediately (no reload).
**Severity:** Medium
**Sprint:** **SP-089**

**Description / Resolution:**
`[Apple]` **Manuscript view doesn't refresh a chapter heading after an in-navigator rename** (heading keeps showing the old/positional name, e.g. "Chapter 12"). Found in the SP-089 EP-029 live-verify (2026-07-29). **Root cause:** `updateNSView` (`ManuscriptTextView.swift:104`) rebuilds the text storage — which is where chapter headings are composed from `loader.allScenes[].chapterTitle` (`rebuildStorage`, ~line 445-456) — only when the **segment ID list** or the **title toggle** changes. A rename leaves every segment ID identical (same scenes, new heading text), so the guard never fires and the cached heading persists. **Fix:** add a chapter-heading fingerprint (ordered `chapterID=title` join across `allScenes`, empty titles included) to the rebuild guard; a rename (or a title *cleared* to positional fallback) now shifts the fingerprint and forces the heading to refresh. `ManuscriptTextView.swift` only; no core/ABI/pbxproj change.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0096: [Apple]+[ScriviCore] Cmd-Enter at the start of a scene mislabels scenes (title shifts to the wrong scene)

**Status:** ✅ **Verified (2026-07-31)** — user confirmed ⌘↩ at the start of a scene inserts an unnamed Scene
before it; the original scene keeps its name; survives quit→reopen. Core `ctest` **361/361** (2 new I-0096 tests);
macOS app **BUILD SUCCEEDED**.

Reported 2026-07-30 during the SP-089 live session. With the caret at the very **beginning** of a scene (most
visibly a chapter's FIRST scene), ⌘↩ created the new Scene-1 but it **inherited the following scene's first-line
title** while the following scene became untitled — persisting across quit→reopen. **Root cause:**
`handleCreateScene` (`ManuscriptTextView.swift`) treated caret-offset-0 as a normal split → **empty head** saved
into the existing scene (clearing its first line = its title) and the **entire body** saved into the new scene (so
the new scene adopted the old first-line title). Scene "names" are derived from the first line of body
(`liveTitles`), so the empty-head/full-tail split moved the title. Correct behaviour (as in one monolithic buffer):
a newline at offset 0 inserts an empty scene **before** the caret's scene, leaving that scene's text + title
intact. There was **no core path** to insert before a chapter's first scene — `createScene`'s empty `afterSceneID`
**appends**, not prepends (its comment even claimed "front" wrongly). **Fix:** (a) `CreateSceneRequest` gains
`beforeSceneID`; `SceneCreator` computes the order key as `keyBetween(prev, beforeScene)` (open-bottom `lo` when
it's the chapter's first scene); threaded through the C ABI `scrivi_create_scene` (+`beforeSceneID` param),
`ScriviEngine.createScene`, and the Linux bridge/smoke (pass `""`). (b) `ViewportSceneLoader.insertScene(before:)`;
`handleCreateScene` routes caret-offset-0 to a before-insert (caret lands in the new empty scene). Core tests
`createScene - beforeSceneID inserts before a chapter's FIRST scene / before a non-first scene` (2 new, RED without
the fix). **Sprint:** SP-089. **Severity:** Medium.

---

---

## I-0097: [Apple] A project last quit in macOS Full Screen reopens NOT full screen (maximize state lost)

**Status:** ✅ **Verified (2026-07-31)** — user confirmed a project last quit in macOS Full Screen reopens and
**remains** full screen (brief windowed→full-screen animation on launch is expected). macOS app **BUILD SUCCEEDED**.

Reported 2026-07-30 (not a regression — window-restore code unchanged since SP-046). **Root cause (log-confirmed,
two bugs):** (1) `windowDidEndLiveResize` was **unguarded** — it fires repeatedly DURING a full-screen transition
with `styleMask` flipping mid-flight, so it persisted transient states (`fs=1` then `fs=0`), churning/clobbering
the saved flag (`windowDidResize` was already guarded by `isTransitioningFullScreen`; this sibling callback was
missed). (2) The restore `toggleFullScreen` fired DURING app launch **stalled mid-flight** —
`windowWillEnterFullScreen` arrived but `windowDidEnterFullScreen` **never** did, leaving the window windowed (the
visible "lines drawing across then it lands windowed"). Manual green-button full screen always worked because the
app is already active. **Fix:** (a) guard `windowDidEndLiveResize` with `isTransitioningFullScreen`, matching
`windowDidResize`; (b) defer the restore until `NSApplication.didBecomeActive` (or immediately if already active)
**plus a 0.35 s settle** so the launch storm clears before toggling — the enter transition then completes cleanly.
Confirming log on reopen: `read savedFullScreen=1` → `windowWillEnterFullScreen` → **`windowDidEnterFullScreen`** →
`save fullScreen=1`, staying full screen through `windowWillClose`. `ProjectWindowManager.swift` /
`ProjectWindowFrameStore.swift` only; no core/ABI/pbxproj change. **Sprint:** SP-089. **Severity:** Low.

---

---

## I-0098

**Status:** ✅ **Verified (2026-08-03)** — user confirmed the fix; `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED** with all five Sendable-closure warnings gone.
**Severity:** Low
**Sprint:** **SP-089**

**Description / Resolution:**
`[Apple]` **Build warnings: Swift 6 Sendable-closure captures in the full-screen-restore observer.** `ProjectWindowController.showAndFocus()` deferred the I-0097 full-screen restore behind a `NSApplication.didBecomeActiveNotification` block observer whose closure is `@Sendable` — but it captured a mutable `var token` (mutated after capture, then read inside the closure) and two main-actor-isolated `() -> Void` closures (`enterFullScreen`, `scheduleAfterSettle`), producing five warnings at `ProjectWindowManager.swift:149/153/154`. **Fix:** (a) an `ObserverTokenBox` (`@unchecked Sendable`, only touched on `.main`) holds the token so the one-shot observer removes itself without a mutable-var capture; (b) the two local closures are replaced by a private `@MainActor` method `scheduleFullScreenAfterSettle()`, so the observer captures only `[weak self]` (hopped back via `MainActor.assumeIsolated` — sound because delivery is on `queue: .main`) and the Sendable box. Behavior unchanged (wait for `didBecomeActive` → 0.35 s settle → toggle, with the redundant-toggle guard). `ProjectWindowManager.swift` only; no core/ABI/pbxproj change (edited file already tracked; no new source file).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*
