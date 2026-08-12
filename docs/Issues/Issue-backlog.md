# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

---

## I-0058: App does not resume at the last-edited scene on launch — always opens at the first scene

**Status:** ✅ Resolved - Verified (2026-07-09, user-approved: full-fidelity fix confirmed — resumes at last-edited scene + cursor + scroll)
**Platform:** macOS (Apple platforms generally; the Swift open path is shared)
**Component:** `Scrivi/App/ProjectSession.swift` (`load(at:)`), `Scrivi/Views/ViewportSceneLoader.swift` (`loadAll()`), `Scrivi/Views/EditorView.swift` (`selectDefaultSceneIfNeeded()`); backend serialization gap in `ScriviCore/src/public_api/scrivi_c_api.cpp` (`scrivi_open_project`) and `scrivi_save_scene`
**Severity:** High (documented core behavior — "Resume Writing" — is silently non-functional; regressed against SP-049 R4 / T-0007 / T-0062 acceptance criteria)
**Regression:** ✅ Yes — regression against **SP-049 R4** ("On relaunch, all previously-open project windows are restored; per-window scene/cursor/scroll restored by the existing backend open flow"). The backend open flow does return the restored scene/selection/scroll, but the Swift open path was never wired to consume it, so the R4 "scene/cursor/scroll restored" acceptance was not actually met on the Apple UI even though the sprint was closed.
**Sprint:** Not Assigned
**Epic:** EP-018 (per-window/per-project model — R4 restore-within-project) / touches EP-019 area only incidentally
**Related:** `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` (Resume Writing Flow §, restores active scene / `restoredSelection` / `restoredScroll`), `Scrivi_PerWindow_Project_Model_Design_v0_1.md` R4, `Scrivi_Backend_Behavior_Spec_v0_2.md` step 12 ("Scrivi restores last scene and cursor"); Task-verified-0007, Task-verified-0062 (backend-side workspace-state restore, verified in isolation); I-0018 (Navigator shows no selection on load — same "nothing is selected on open" surface)
**Date Identified:** 2026-07-09
**Reported during:** SP-054 testing (T-0207/T-0208/T-0209 verified working; this is a separate, pre-existing regression noticed in passing)

**Description:**
Every launch opens the manuscript at the very first scene (chapter one), regardless of where the
writer was editing at the end of the previous session. The user left off in Chapter 15; the app
reopens at the beginning each time.

**Expected Behavior:**
On relaunch, within each restored project window, Scrivi returns the writer to the scene they last
edited (and, ideally, the cursor/scroll position within it) — the "Resume Writing Flow" specified in
`Scrivi_Project_Creation_and_Open_Flow_v0_2.md` and asserted by SP-049 R4.

**Actual Behavior:**
The editor always shows the first scene. The Navigator shows no active highlight until the user
scrolls.

**Steps to Reproduce:**
1. Open a project, edit a scene deep in the manuscript (e.g. Chapter 15), let it auto-save.
2. Quit the app.
3. Relaunch — the editor opens at the first scene, not Chapter 15.

**Root-Cause Analysis (no code changed):**

The backend *does* the right thing; the Swift layer discards it, and the boundary drops the fine-grained parts.

1. **Backend persists & restores the active scene correctly.**
   - On save, `SceneWriter.cpp` (~L92–110) writes `WorkspaceState.lastWritingSurface` with the
     saved `sceneID` (via `WorkspaceStateService`). So the last-edited **scene** is durably recorded
     each save.
   - On open, `ScriviCore.cpp` (~L99–145) loads that workspace state, computes `activeScene`,
     `restoredSelection`, and `restoredScroll`, and returns them in `OpenProjectResult`.

2. **The C ABI boundary drops selection/scroll (and never receives them on save).**
   - `scrivi_open_project` (scrivi_c_api.cpp ~L354–367) serializes only
     `activeScene.{sceneID,metadataPath,contentPath,markdown}` — it **omits `restoredSelection` and
     `restoredScroll`** that the C++ result carries. So cursor/scroll can never reach Swift even
     though the backend computed them.
   - `scrivi_save_scene` has **no selection/scroll parameters**, and Swift's `saveScene` does not
     send them. Thus `WorkspaceState.lastWritingSurface.selection/scroll` are always default. (The
     **sceneID** is still written correctly, so scene-level restore is fully available.)

3. **The Swift open path ignores the restored scene entirely — the decisive defect.**
   - `ProjectSession.load(at:)` (ProjectSession.swift ~L77–94) decodes `result.activeScene` into
     `OpenProjectResult` (it *is* decoded — ScriviEngine.swift ~L1093/1101) but passes **only
     `result.scenes`** to `ViewportSceneLoader` and calls `loadAll()`. `result.activeScene` is never
     used.
   - `ViewportSceneLoader.loadAll()` (ViewportSceneLoader.swift ~L87–99) hard-codes
     `currentIndex = 0`, sets `cursorSceneID = segments.first`, and **intentionally leaves
     `viewportSceneID = nil`**.
   - `EditorView.selectDefaultSceneIfNeeded()` (iOS, EditorView.swift ~L115–118) and its macOS
     analogue then read `loader.viewportSceneID ?? allScenes.first` → always falls back to the first
     scene. The in-code comments ("we already persist/restore it") are **stale/aspirational** — no
     Swift persistence or restore of `viewportSceneID` exists; there is no UserDefaults/disk key for
     it, and none was ever committed (git history shows no `viewportSceneID`/`scrivi.viewport`
     persistence).

**Net:** the backend hands Swift the correct last-active scene on every open; Swift throws it away and
defaults to scene 0.

**Candidate Fix Directions (for discussion — not yet chosen):**
- **Minimum viable (scene-level resume):** In `ProjectSession.load(at:)`, after `loadAll()`, seed the
  loader's `viewportSceneID` (and `currentIndex`/`cursorSceneID`) from `result.activeScene?.sceneID`
  when present. This alone fixes "returns to Chapter 15" with no ABI change, because the backend
  already returns the correct `activeScene`. Add a `ViewportSceneLoader` entry point (e.g.
  `seedActiveScene(_:)`) rather than overloading `loadAll()`.
- **Full fidelity (cursor + scroll):** Extend the C ABI to (a) serialize `restoredSelection` /
  `restoredScroll` in `scrivi_open_project`, and (b) accept selection/scroll on `scrivi_save_scene`
  so Swift can report them at save time. Then thread scroll/selection through `EditorView` /
  `ManuscriptTextView` to scroll-to and place the cursor on open.
- **Verify the write path first:** Confirm auto-save actually calls `saveScene` for the deep scene
  before quit (the last-active `sceneID` is only recorded on a real save). If auto-save on a scene the
  user merely scrolled to — but didn't edit — doesn't fire, the recorded active scene may lag; may
  need `openProject`/close to also stamp the current `viewportSceneID`.

**Recommended first step:** the minimum-viable scene-level seed (backend already supplies the data),
then decide whether cursor/scroll fidelity is worth the ABI change.

**Implementation — Full-fidelity fix (2026-07-09, user-approved direction: full fidelity):**

Scene + cursor + scroll all restored end-to-end. The backend already computed and persisted the data;
the fix widens the C ABI to carry it and wires the Swift layer to consume/produce it.

1. **C ABI — emit restored surface on open, accept it on save**
   (`ScriviCore/src/public_api/scrivi_c_api.cpp`, `ScriviCore/include/scrivi/scrivi.h`):
   - `scrivi_open_project` now serializes a `restored` sub-object `{anchor, focus, scroll}` from the
     C++ result's `restoredSelection`/`restoredScroll` (previously dropped at the boundary).
   - `scrivi_save_scene` gains `long long selectionAnchor, long long selectionFocus, double scroll`
     params, populating `SaveSceneRequest.selection`/`.scroll` (the C++ struct already had these
     fields; only the C ABI lacked them). Negative offsets are clamped to 0.
2. **Swift engine** (`Scrivi/Engine/ScriviEngine.swift`):
   - New `RestoredSurfaceResult {anchor, focus, scroll}`; `OpenProjectResult.restored` decodes it
     (all `decodeIfPresent`, defaults 0 — backward compatible).
   - `saveScene(...)` gains `selectionAnchor/selectionFocus/scroll` params (defaulted to 0, so
     existing call sites are unaffected) and forwards them to the widened C ABI. Unavailable-platform
     stub signature updated to match.
3. **Loader** (`Scrivi/Views/ViewportSceneLoader.swift`):
   - `loadAll(activeSceneID:restoredSelection:restoredScroll:)` — when the backend supplies an active
     scene that still exists, seeds `currentIndex`/`cursorSceneID`/`viewportSceneID` to it and stashes
     the one-shot `restoredSelectionOffset`/`restoredScrollFraction`; otherwise unchanged (scene 0, no
     pre-selection).
   - Tracks live `currentSceneCursorOffset` (scene-local) and `scrollFraction`; `saveScene(at:)` sends
     them **only for the current scene**. `saveAllDirty` now saves the current scene **last** so its
     writing-surface state is the one the backend records as `lastWritingSurface`.
4. **Session** (`Scrivi/App/ProjectSession.swift`): `load(at:)` passes
   `result.activeScene?.sceneID` + `result.restored?.anchor/scroll` into `loadAll(...)`.
5. **Editor** (`Scrivi/Views/ManuscriptTextView.swift`):
   - `textViewDidChangeSelection` records the scene-local cursor offset into the loader.
   - `scrollDidChange` records the document scroll fraction into the loader.
   - New `restoreWritingSurface(in:)` runs once (guarded by `didRestoreSurface`) on first
     `updateNSView`: places the cursor at the restored scene's storage offset + clamped scene-local
     offset and scrolls it into view, then consumes the one-shot state.
   - iOS resumes automatically: `EditorView.selectDefaultSceneIfNeeded()` already reads
     `loader.viewportSceneID`, which is now correctly seeded.

**Cursor rules honoured** (`Scrivi_Project_Creation_and_Open_Flow_v0_2.md` §9.3): the scene-local
offset is clamped to the restored scene's current text length, so an externally-shrunken scene places
the cursor safely rather than out of bounds.

**Files changed:** `scrivi_c_api.cpp`, `scrivi.h`, `ScriviEngine.swift`, `ViewportSceneLoader.swift`,
`ProjectSession.swift`, `ManuscriptTextView.swift`. No new files → no `project.pbxproj` change.

**Build/test:** ScriviCore builds clean; all **249** C++ tests pass (incl. `OpenProjectTests` /
`MvpLoopTests` that assert `restoredSelection`/`restoredScroll` round-trip). `xcodebuild -scheme
ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED**.

**Verification:** ✅ Confirmed on-device (2026-07-09, user-approved) — editing deep in the manuscript,
quitting, and relaunching returns to that scene with cursor/scroll restored.

**Follow-up — stamp the scrolled-to scene on flush (2026-07-09, user-approved):**

Addresses verification note (b): a scene the writer *scrolled to but never edited* is now also a valid
resume point. Previously the last-active `sceneID` was only stamped by a real `saveScene`, so a
scroll-only visit didn't move the resume point.

- `ViewportSceneLoader.stampWritingSurface(engine:ref:)` — forces one final `saveScene` of the
  **viewport** (scrolled-to) scene with the current scroll fraction and cursor offset 0, so the
  backend records it as `lastWritingSurface`. No-op when the viewport scene equals the cursor scene
  (already stamped) or isn't loaded. Cursor offset 0 follows §9.3 ("cursor not in this scene ⇒ restore
  scene, place cursor safely").
- `saveAllDirty(...)` calls `stampWritingSurface` after the current-scene save.
- Hook: this runs on the existing `willResignActive` flush (`AppEnvironment.onAppResign` →
  `session.saveAllDirty()`), which fires on quit and on background — the paths that matter for resume.
- **Deliberately not** wired into the synchronous window-close path (`closeProject` → `session.close()`
  is a synchronous NSWindowDelegate teardown; injecting an async save there risks the loader being
  nil'd mid-write). Narrow uncovered case: ⌘W one window while the app stays active and never resigns
  before quit — edits are already persisted by the per-scene autosave; only the scroll-only resume
  point could be slightly stale.

**Follow-up files changed:** `ViewportSceneLoader.swift` only. macOS app builds; 249 C++ tests pass.

---

## I-0017: Window maximized state not restored on app relaunch

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `WindowFrameAutosave.swift`
**Severity:** Medium
**Sprint:** Not Assigned
**Related:** I-0051 (multi-window per-project frame/position restore — Verified 2026-06-29, subsumes the position/size part of this); **I-0055** (multi-window maximize-restore defect — the same zoom-restore problem on the per-window model; fix the two together)

**Description:**
Window position, size, and maximized state are not fully restored between app launches. Frame and position restore correctly. Maximized state does not — the window always relaunches un-maximized regardless of saved zoom state.

**Expected Behavior:**
On relaunch, the window appears at the same size, position, and maximized state as when the user last quit. The Landing View and Editor share the same window — no resize occurs when transitioning between them.

**Actual Behavior:**
Frame and position restore correctly. Maximized state does not restore.

**Steps to Reproduce:**
1. Maximize the window.
2. Quit the app.
3. Relaunch — window opens un-maximized.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`window.zoom(nil)` fires too early — SwiftUI's `WindowGroup` continues async layout passes after the call and overrides it. Current approach uses `NSApplication.didFinishLaunchingNotification` as the trigger, but this has not resolved the issue. Requires deeper investigation.

**Resolution:** TBD

---

## I-0018: Scene Navigator shows no selection on app load

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `ViewportSceneLoader.swift`
**Severity:** Low
**Sprint:** Not Assigned

**Description:**
When the app loads a project, no scene is selected/highlighted in the Scene Navigator. The Navigator self-corrects on first scroll.

**Expected Behavior:**
On load, the Navigator highlights the scene visible at the top of the manuscript viewport.

**Actual Behavior:**
No scene is highlighted until the first scroll event.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`viewportSceneID` is intentionally left nil during `loadAll()`. The scroll observer sets it on first scroll, but this hasn't fired at load time.

**Resolution:**
TBD — needs a mechanism to determine the top-of-viewport scene after `NSTextView` completes initial layout without triggering a spurious scroll notification.

---

## I-0112: [Apple] Manuscript view becomes dark text on dark background when macOS switches to Dark Mode

**Status:** ✅ **Resolved - Verified (2026-08-11, user-confirmed)** — the user confirmed in a live run:
*"Manuscript View appears now in the live demo with light text on a dark background."* The reported
black-on-dark-gray defect is gone. ⚠️ **Verified in Dark Mode only** — see the verification scope note in
Resolution below.
**Platform:** macOS (Apple platforms generally — the iOS/visionOS manuscript view is still a stub, so the
defect is only *observable* on macOS today, but the requirement is all-platform)
**Component:** `Scrivi/Views/ManuscriptTextView.swift` (primary); appearance support across all Scrivi views
**Severity:** High — the manuscript is unreadable and effectively unusable whenever the Mac is in Dark Mode.
For a writer on macOS's automatic "Appearance: Auto" setting this happens every evening without any user action.
**Sprint:** Not Assigned
**Epic:** Not Assigned
**Date Identified:** 2026-08-11
**Reported by:** User, 2026-08-11 — *"My mac changes to dark mode when the sun goes down. When that happens the
manuscript view becomes dark text on dark background. Basically unusable."*

**Description:**
Scrivi has never been designed, implemented, or tested against macOS Dark Mode. With the system set to
**Appearance: Auto** (the macOS default, which flips to Dark at sunset), the manuscript editor renders
**dark body text on a dark background**, leaving the writer unable to read or edit their manuscript. The
failure arrives unannounced — the writer takes no action; the sun goes down and the app becomes unusable.

**Expected Behavior:**
Every view in Scrivi is legible and correctly contrasted in **both** Light and Dark appearance, and adapts
**live** when macOS switches appearance mid-session (no relaunch, no project reopen, no scene switch required).
This includes the manuscript editor, Scene Navigator, Inspector card stack, Timeline strip, Landing view,
sheets (New Project, Project Settings, About), popovers (Fork), and the Buffers palette.

**Actual Behavior:**
In Dark Mode the manuscript body text renders **black on a dark gray background** (user-confirmed wording,
2026-08-11). A live check by the user found the manuscript is the **only** affected view — every other
surface renders correctly in Dark Mode.

**Steps to Reproduce (cold launch — no appearance switch required):**
1. Set System Settings ▸ Appearance to **Dark**.
2. Launch Scrivi and open a project.
3. Observe the manuscript view — body text is black on a dark gray background, from the first frame.

*Also reproduces via a live switch (start in Light, switch to Dark with the project open), but the switch is
**not** required and is not the cause — see Root Cause Analysis.*

**Impact:**
- The core writing surface is unusable for roughly half of every day on a default macOS configuration.
- Affects the writer's primary workflow with no workaround inside the app (the only workaround is to force
  macOS itself back to Light Mode, i.e. changing an OS-wide setting to work around an app defect).
- Dark Mode is a baseline platform expectation on macOS 27; shipping without it is not viable for alpha testers.

**Root Cause Analysis (2026-08-11 — no code changed):**

> ⚠️ **A first hypothesis in this entry was wrong and has been replaced.** It attributed the defect to a
> *live appearance switch* leaving stale resolved colors, because nothing re-runs `rebuildStorage` when
> macOS flips. The user disproved it immediately: the Mac had **already been in Dark Mode for hours** and
> the app was launched **minutes** before observing the defect — storage was therefore built exactly once,
> under Dark, and still rendered wrong. A staleness theory predicts a correct render in that scenario. The
> confirmed cause below is a **static** defect that reproduces on a cold launch in Dark Mode, with no
> appearance switch involved. The original hypothesis is retained at the end of this section only so the
> reasoning is not repeated by a future reader.

**Confirmed root cause — body text has no color attribute, and the fallback is literal black (not adaptive):**

The manuscript's body runs are built with a **font-only attribute dictionary — no `.foregroundColor` key**, in
both places that write body text:

- `Coordinator.rebuildStorage` — `let attrs: [NSAttributedString.Key: Any] = [.font: font]`
  (`ManuscriptTextView.swift:517`), used for every scene segment (`:565`) and the inter-scene newline (`:530`).
- The history-apply path — the same font-only dictionary (`:296-298`), used by
  `storage.replaceCharacters(...)` (`:299`) when undo/redo rewrites a scene.

Neither sets a color, so both depend on the text view's `textColor` for the rendered color. **`textColor` is
never assigned anywhere in the file** — the only `grep` hit for it is an unrelated `CABasicAnimation(keyPath:
"backgroundColor")` in `flashRefuse` (`:1518`). When an `NSTextView`'s `textColor` is nil, a run carrying no
`.foregroundColor` renders as **`NSColor.black` — a literal, non-adaptive black**, not `NSColor.textColor`.

Meanwhile the text view's **background** is drawn from its own `backgroundColor`, which *does* follow the
effective appearance and becomes dark gray under Dark Mode. Hence **black text on a dark gray background**,
deterministically, from the first frame — matching the user's report exactly ("black text on a dark gray
background", cold launch, appearance switched hours earlier).

The in-code comment at `:293-295` states the attributes match "regular monospaced font, **default text color**".
That comment encodes the mistaken assumption at the heart of this defect: omitting `.foregroundColor` does not
yield the *adaptive* default — it yields black.

**Why the manuscript is the only affected view:** every other Scrivi surface is SwiftUI and colors text via
`.foregroundStyle(.primary/.secondary/…)`, which resolves against the current appearance correctly. The
manuscript is the sole AppKit text surface and the only place a text color is established by *omission*. This
is consistent with the user's observation that no other view misrenders.

**Superseded hypothesis (kept to prevent re-derivation):**

The cause is **not** a hardcoded light-mode palette. An audit of the Apple layer found the opposite —
essentially every color already in the code is semantic and would adapt correctly on its own:

- `ManuscriptTextView.swift` uses only `NSColor.secondaryLabelColor` (chapter headings, `:553`),
  `NSColor.separatorColor` (scene divider stroke, `:1870`), and `NSColor.systemRed`/`.clear` (refuse flash,
  `:1519-1520`). All are dynamic system colors.
- The SwiftUI views (`InspectorCardStackView`, `HistoryCard`, `ScenePropertiesView`, et al.) use
  `.foregroundStyle(.secondary/.tertiary/.primary)` throughout — no literal `Color(red:...)` anywhere.
- **Nothing forces a light appearance:** there is no `NSRequiresAquaSystemAppearance` in `Scrivi/App/Info.plist`,
  no appearance build setting in `project.pbxproj`, and no `preferredColorScheme` / `NSAppearance` assignment
  anywhere under `Scrivi/`.

The likely mechanism is therefore in the **manuscript text storage**, which is the one place a *non-adaptive*
color can enter — not by being written, but by being **omitted**:

1. `Coordinator.rebuildStorage` (`ManuscriptTextView.swift:516-517`) builds body-text attributes as
   `[.font: font]` — **no `.foregroundColor` key at all**. Body runs therefore inherit the text view's
   `textColor`, which is the correct adaptive default *at the time the storage is built*.
2. The storage is rebuilt only when the segment-ID list, the chapter-title toggle, or the chapter-heading
   fingerprint changes (`updateNSView`, `:104`; the fingerprint guard was added by I-0095). **An appearance
   change is not one of those triggers**, so nothing re-resolves the manuscript's colors when macOS flips
   at sunset. This matches the reported symptom precisely: the text keeps whatever resolved color it had
   under the previous appearance while the background follows the new one.

The earlier theory held that nothing re-resolves the manuscript's colors on a live appearance change, since
`updateNSView`'s rebuild triggers (`:104` — segment-ID list, title toggle, chapter-heading fingerprint) do not
include one. That observation **is still true**, but it is not what causes this defect and fixing it would not
have helped: with an adaptive color attribute in place, AppKit re-renders dynamic colors on an appearance
change without any storage rebuild. No new rebuild trigger is needed.

**Scope (why this is broader than one view):**
The user's report is specifically the manuscript view, and a live check confirms it is currently the **only**
misrendering surface. The requirement stated is Dark Mode support in **all** views, so the other surfaces still
need to be *exercised* in Dark Mode — but the audit plus the user's observation indicate that is verification
work, not rewriting. The one remaining code-level suspect is `DividerAttachmentCell.draw(withFrame:in:)`
(`:1864-1872`): `NSTextAttachmentCell` drawing does not reliably inherit the hosting view's appearance context,
so `NSColor.separatorColor` may resolve against the wrong appearance even though the color itself is semantic.

**Resolution (implemented 2026-08-11 — awaiting user verification):**

**Fix Date:** 2026-08-11
**Verification Date:** 2026-08-11 (user-confirmed, Dark Mode live run)

Three changes, all in `Scrivi/Views/ManuscriptTextView.swift`. No new source file, so **no `project.pbxproj`
change was required** (the edited file is already tracked). No ScriviCore/C-ABI change — this is purely an
Apple-presentation-layer defect.

1. **`rebuildStorage` body attributes now carry an adaptive color** (`:517-524`) — the font-only dictionary
   became `[.font: font, .foregroundColor: NSColor.textColor]`. This is the initial-build path that produced
   the reported black-on-dark-gray.

2. **The history-apply path carries the same color** (`:296-306`) — `.foregroundColor: NSColor.textColor`
   added to the dictionary used by `storage.replaceCharacters(...)` when undo/redo rewrites a scene. **Both
   sites had to change together:** fixing only `rebuildStorage` would have left an undo/redo re-blackening a
   scene's text under Dark Mode, producing a defect that reappears only after a specific action — harder to
   diagnose than the original. The misleading "default text color" comment (`:293-295`) that encoded the
   original wrong assumption was corrected to "adaptive text color".

3. **`textView.textColor = NSColor.textColor` in `makeNSView`** (`:39-45`) — the defensive backstop, so any
   run written in future without an explicit color inherits an adaptive value instead of falling through to
   AppKit's literal black. This alone would have fixed the reported symptom; items 1–2 are kept because
   relying on inheritance is what caused the defect.

4. **Divider attachment cell drawn in the host's appearance** (`DividerAttachmentCell.draw(withFrame:in:)`,
   `:1881-1899`) — the stroke is now wrapped in
   `controlView?.effectiveAppearance.performAsCurrentDrawingAppearance { … }`, falling back to a direct call
   when `controlView` is nil. `NSTextAttachmentCell` does not reliably inherit the hosting view's appearance
   context, so semantic `NSColor.separatorColor` could otherwise resolve against the wrong appearance.
   ⚠️ **This one is precautionary** — the divider was never *reported* as misrendering, and the pre-existing
   code was already semantic. If verification shows no difference it is harmless; it is called out here so it
   is not mistaken for a fix to an observed symptom.

**Why no new rebuild trigger was added:** the superseded hypothesis would have re-run `rebuildStorage` on
appearance change. That is unnecessary — AppKit re-renders dynamic colors on an appearance change without a
storage rebuild — and would have risked the caret-stomping regression class of I-0084.

**Verification status:**
- ✅ `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED**, no new warnings
  (the two emitted — `MACOSX_DEPLOYMENT_TARGET 27.0` and the CMake run-script phase — are both pre-existing).
- ✅ `xcodebuild -scheme ScriviApp -destination 'platform=macOS' test` → **TEST SUCCEEDED, 56/56** (2 suites).
- ⚠️ **No automated coverage proves the fix.** The interop suite exercises the ScriviCore boundary, not AppKit
  text rendering; 56/56 demonstrates only that nothing regressed. Appearance rendering is not observable from
  that suite, so **acceptance rests entirely on user verification.**

**User verification (2026-08-11) — what was actually exercised:**

1. ✅ **Dark Mode** — **CONFIRMED.** User, live run: *"Manuscript View appears now in the live demo with light
   text on a dark background."* This is the reported defect and it is fixed.
2. ⚠️ **Light Mode, cold launch** — **NOT exercised.** `NSColor.textColor` replaced a literal black in Light
   Mode as well; both render dark-on-light so no visible change is expected, but this has not been confirmed.
   The user noted they will see it in the morning (2026-08-12) when macOS returns to Light on the Auto schedule
   — which incidentally exercises both this and item 3.
3. ⚠️ **Live appearance switch** — **NOT exercised.** The verified run was a cold launch already in Dark. Text
   should follow a mid-session flip for free via dynamic-color re-rendering, but this is the exact scenario in
   which the *first* root-cause hypothesis for this issue proved wrong, so it is recorded as unconfirmed rather
   than assumed.
4. ⚠️ **Undo/redo in Dark Mode** — **NOT exercised.** Covers the history-apply path (change 2), the site that
   would re-blacken a scene's text if the fix were incomplete there.

**Verification scope ruling (user, 2026-08-11):** *"for now its fine… we can mark I-0112 as verified."*
Marked Verified on the strength of the reported symptom being gone. Items 2–4 are follow-on confidence checks
on paths that share the same one-line mechanism as the confirmed fix (an adaptive `.foregroundColor` in place
of an implicit black); they are **not** separate unfixed defects. If any of them misbehaves, re-open this entry
rather than filing a new Issue — the mechanism is identical.

**Remaining scope not covered by this fix:**
- **The other views were not re-audited in Dark Mode.** The user's live check found the manuscript to be the
  only misrendering surface, and the code audit found every other view uses adaptive `.foregroundStyle`, so
  no change was made elsewhere. A deliberate Dark Mode sweep of Landing, Scene Navigator, Inspector card
  stack (incl. History card), Timeline strip, New Project / Project Settings / About sheets, Fork popover and
  Buffers palette is **still outstanding** if the all-views requirement is to be closed rigorously.
- **A mechanical regression guard is not in place.** Asserting that the body-text attribute dictionaries carry
  a `.foregroundColor` key at all would have caught this defect and would catch its recurrence; it is not
  currently expressible in `ScriviInteropTests`, which does not reach into the AppKit layer.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — body-text `.foregroundColor` in `rebuildStorage` (`:517-524`) and
  the history-apply path (`:296-306`); `textView.textColor` backstop in `makeNSView` (`:39-45`); divider cell
  appearance-scoped drawing (`:1881-1899`). **Only file changed.**
- No other `Scrivi/Views/*.swift` changed — the Dark Mode audit found no defect elsewhere (see Remaining scope).

**Notes:**
- No `.scrivi` project data or ScriviCore behavior is involved — this is a pure Apple-presentation-layer defect.
- Per CLAUDE.md, if any new source file is added while fixing this, `Scrivi.xcodeproj/project.pbxproj` must be
  updated in the same step. The files listed above are already tracked.
- **Linux/Qt parity is out of scope for this Issue** — Qt has its own palette story and should be tracked
  separately if the writer wants dark theming there.

**Related Issues:**
- I-0095 / I-0084 (manuscript storage-rebuild guard and the caret-stomping redundant rebuild) — relevant only
  to the **superseded** hypothesis, which would have added a rebuild trigger. The confirmed fix touches
  attribute dictionaries, not the rebuild path, so neither issue constrains it.

---

*Last Updated: 2026-08-11 (I-0112 opened, then **root cause corrected**. Filed as a suspected live-appearance-switch
staleness bug; the user disproved that within minutes — Dark Mode had been active for hours and the app was
launched minutes before the defect was seen, so no switch was involved. Confirmed cause is static: body-text
attribute dictionaries omit `.foregroundColor` (`:517`, `:296-298`) and `textColor` is never set, so AppKit
renders body runs as literal `NSColor.black` against an adaptive dark background. Manuscript-only, as the sole
AppKit text surface. Sprintless/unassigned. I-0017/I-0018/I-0058 unchanged.)*
