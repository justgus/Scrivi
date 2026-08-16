# Verified Issues — I-0111 to I-0120


---

## I-0111

**Status:** ✅ **Resolved - Verified (2026-08-11, user-approved)** — diagnostic-only fix, so there is no user-visible behaviour to exercise; the user approved verification on the test evidence. `lastSeq_ = maxSeq` is now adopted **before** the prune can write, and the later assignment is `std::max(lastSeq_, maxSeq)` so it cannot roll the counter backwards over records written in between. Regression test forces a real prune and asserts the resulting purge carries a seq above the replayed mark — **RED without the fix** (`1 > 9000`), with a `REQUIRE(sawPurge)` guard so it cannot pass vacuously (the first draft did exactly that and was rewritten). ctest **413/413**, interop **53/53**, macOS BUILD SUCCEEDED.
**Severity:** Low
**Sprint:** **SP-093**

**Description / Resolution:**
`[ScriviCore]` **Prune-driven `ctl:purge` records were written with `seq 1`, so log sequence numbers regressed.** Found while investigating [[I-0110]] (2026-08-11): the writer's log held **13 purge records all stamped seq 1**, causing **11 sequence regressions** in a 3,607-record log — including the one at the very end that made I-0110's failure so confusing to trace. **Root cause:** `openOrCreate` ran `pruneInconsistentNodes()` + `persistPurge()` at line ~190 while `lastSeq_` was still **0**; the replayed high-water mark (`lastSeq_ = maxSeq`) was not adopted until ~34 lines later. `persistPurge` stamps `++lastSeq_`, so every prune-driven purge got seq 1. **Impact is diagnostic, not functional** — replay is ordered by file position, not by `seq`, so nothing malfunctioned. But the numbers were meaningless and actively misled forensics. **Fix:** adopt `lastSeq_ = maxSeq` **before** the prune can write, and change the later assignment to `std::max(lastSeq_, maxSeq)` so it cannot roll the counter backwards over records written in between. **Regression test** forces a real prune (a diff-inconsistent node parented to the live tree) and asserts the resulting purge carries a seq above the replayed mark; **RED without the fix** (`1 > 9000` fails), and it asserts a purge was actually written so it cannot pass vacuously — the first draft did exactly that and was rewritten. ctest **413/413**.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0113

**Status:** ✅ **Resolved - Verified (2026-08-12, user-approved, T-0405)** — the three entry points now carry `worldID` (empty/NULL = project scope); `scrivi_save_object` stayed un-widened because the object JSON already holds it. **The regression test goes through `scrivi_*`, not the facade** — a world-scoped artifact round-trips create → open → delete through the ABI, and the audit probe now succeeds. ⚠️ **The blast radius was larger than the plan assumed:** `ScriviEngine.swift` wraps all three endpoints, so R1's "only in-tree callers are ScriviCore tests" was wrong; the Swift wrappers gained a defaulted `worldID: String = ""`, keeping every existing call site source-compatible. ctest **510/510 macOS** + **517/517 Linux (GCC 14, zero warnings)**, interop **56/56**.
**Severity:** High
**Sprint:** **SP-098**

**Description / Resolution:**
`[ScriviCore]` **World-scoped objects are unreachable through the C ABI — `scrivi_create_object` has no `worldID` parameter, so every `artifact`/`chronicle`/`faction`/`rule` create fails from Swift and Qt.** Found in the SP-098 planning audit (2026-08-12) by diffing the shipped ABI against the design docs, then **confirmed empirically** with a probe linked against `libScriviCore.a`: `scrivi_create_object(root, "artifact", …)` returns `{"ok":false,"message":"kind 'artifact' is world-scoped; a worldID is required"}` even with a world created and bound moments earlier. **Root cause:** SP-097 (T-0385) added `worldID` to `CreateObjectRequest`/`OpenObjectRequest`/`DeleteObjectRequest` and routed storage through `ObjectStore::kindDirFor`, but the four `scrivi_*_object` C entry points were never widened — `req.worldID` is left default-empty at the boundary, so `kindDirFor` correctly refuses. The capability exists in C++ and is covered by `WorldTests.cpp` (which calls the facade directly, **bypassing the ABI** — which is why the suite is green and the gap went unnoticed). **Impact:** no platform layer can create, open by kind, or delete a world-scoped object. AC1's world-scoped round-trip is met in C++ only; the Apple/Linux halves of EP-031 (SP-099) would hit this immediately. **Design status:** Doc 1 §6 asserts "New object kinds need **no** new CRUD endpoints" — true when it was written, but **no longer accurate** once kinds became scope-dependent; the doc is amended in the same step. **Fix:** add a `worldID` parameter to `scrivi_create_object`, `scrivi_open_object`, and `scrivi_delete_object` (empty string = project scope); `scrivi_save_object` needs none — the object JSON already carries `worldID`. ⚠️ **This is a breaking ABI change to shipped signatures**, the first in EP-031; every existing caller passes 3–7 args and must be updated.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0119

**Status:** ✅ **Resolved - VERIFIED (2026-08-15, user-confirmed by live re-test of the exact flow)** — **(a)** `ObjectDraft` now carries `originSceneID`, stamped at every one of its three construction sites, and `createAndRelate` takes `toSceneID` **explicitly** instead of reading the model's captured scene — so a draft completed after a scene change files against the scene it was started in. **(b)** The alert now OWNS the commit: it offers **"Create in Previous Scene"** / **"Save Changes"**, and the draft editor's own Save is **disabled while the prompt is up**, so a commit can no longer happen around the decision the alert is asking for. The prompt's copy now names the destination scene explicitly ("files it against the scene you were just in, not the one you're viewing") — §4.6.1 requires the consequence be concrete, and the writer cannot see the origin scene by then. **Regression test added at the boundary** (`ScriviInteropTests`, *"an object related to scene A stays on scene A, never on a later scene (I-0119)"*): builds two scenes, relates to A, asserts A has the edge **and B is empty**. ctest **516/516**, macOS interop **86/86** (was 85 — the new test ran by name and passed). ⚠️ **The regression test was verified to CATCH the bug, not merely to pass:** re-pointing the edge at scene B failed all three assertions (A missing it, B carrying it, B non-empty), then passed again on restore. A test that only ever passed would not show the invariant is real. ⚠️ **A THIRD defect, found by the user re-testing the fix (2026-08-14):** choosing **"Complete It"** reopened the draft *while the writer stood in the NEW scene*, and clicking Create then filed the object against the **previous** scene. **The data behavior was correct** — `originSceneID` held — **but nothing on screen said so**, so the writer reasonably read the visible scene as the destination. ⚠️ **The fix was a UI-honesty fix, not a data fix, and the distinction matters: the tempting "correction" (re-home the draft to the current scene) would have re-introduced the original bug.** User ruled **keep the origin, name it visibly**. Now: the draft editor's header shows **"for: Scene N"** (icon + words, never colour alone, §4.6.2), the commit button reads **"Create in Scene N"** instead of a bare "Create", and both appear **only when the origin differs** from the scene on screen, so the ordinary case stays quiet. `CardContext` gained `allSceneIDs` (manuscript order) to resolve the number; an origin that cannot be located says **"another scene"** rather than inventing one. **"Complete It" was renamed "Keep Editing"** — it never completed anything, it reopens the draft. **Verified live 2026-08-15** by the user re-running the originating flow: all three prompt options behave correctly, and the reopened draft now names its destination scene. ⚠️ **All three defects here were found by USE, not by tests** — and the third could not have been caught by any test written for the first two, because the write was always correct and only the explanation was missing. The regression test pins the data invariant (and was verified to catch the original bug by deliberately inverting it); the UI-honesty half is verified only by the live pass.
**Severity:** **High**
**Sprint:** **SP-099**

**Description / Resolution:**
`[Apple]` **⚠️ DATA CORRECTNESS — completing an unfinished object at a scene change relates it to the WRONG SCENE.** Reported by the user 2026-08-14 during the SP-099 AC20 check: *"When I clicked out of the scene I was creating the location in, it prompted me to save… When I clicked 'Save' it saved the Location to the scene I was currently in. I had to go back to the previous scene and add it manually to correct the error."* **Two distinct defects in one flow.** **(a) The draft outlives the model that owns its scene.** `ObjectCardBody.draft` is app-side `@State` (correct — §4.6 says nothing is written until commit), but `.task(id: context.sceneID)` **rebuilds `ObjectCardModel` with the NEW sceneID** as soon as the scene changes (`ObjectCard.swift:419`), and `ObjectCardModel.sceneID` is captured at `init` (`:130-137`). `commitDraft` → `model.createAndRelate` therefore writes the edge to `model.sceneID` — **the scene the writer just moved TO**, not the one she was working in. The object itself is created correctly; only the edge endpoint is wrong, which is why it looks like the object "moved". **(b) The alert has no "Save" button at all.** It offers **"Complete It"** and **"Discard New Location"** (`:436,:444`). The "Save" the user clicked is the **draft editor's own button** (`:547`), which is still on screen when the alert appears — so the prompt does not actually own the commit action, and a writer can commit *around* the prompt. That also means AC20's guarantee is unenforced: the decision the alert asks for can be bypassed by the control underneath it. **Impact:** silent mis-filing of a relationship, in the exact flow AC20 exists to protect. The writer must notice and repair it manually, as this user did. ⚠️ **AC20 must NOT be ticked** — the prompt fires at the right moment, but its outcome is wrong.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

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


> *Archived from `Issue-backlog.md` on 2026-08-16. It was Verified 2026-08-11 but never moved out of
> the backlog — the 2026-08-15 cleanup archived `Issue-active.md` only, and the 2026-08-16 consistency
> audit did not open `Issue-backlog.md` either. Content is unchanged from the backlog entry.*

---
