# Verified Issues: I-0131 – I-0140

Archived Issues, ✅ **Resolved - Verified** by the user. Batched in decades of ten per
`Issue-GUIDELINES.md`.

| ID | Title | Severity | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| I-0131 | `[Apple]` Resume scene only persisted as a side effect of saving a **dirty** scene | Medium | SP-102 | 2026-08-18 |
| I-0132 | `[Apple]` Navigator never revealed the selected row, and a click left focus stranded in the list | Medium | SP-102 | 2026-08-18 |
| I-0133 | `[Apple]` `restoredScrollFraction` was written, cleared, and **never read** — dead state | Low | SP-102 | 2026-08-19 |

> ⚠️ **I-0132 was archived here prematurely once, then returned, then re-verified.** The first archive
> (2026-08-18) claimed both halves verified when only the **reveal half** was: the user's evidence
> covered focus *on app launch*, which he stated explicitly, and I credited it to click-to-focus as
> well. It did not work on click. The Issue went back to `Issue-active.md` and took **four** attempts
> to fix properly — the first three treated it as a first-responder race, which it was not.
> **Both halves are now ✅ Verified 2026-08-18** on an extended live click-through by the user. The
> record below documents every wrong turn, because the wrong turns are the reusable part.

---

## I-0131 — The resume scene was only persisted when the scene was dirty

**Severity:** Medium · **Sprint:** SP-102 (adopted) · ✅ **Verified 2026-08-18**

**Reported by the user 2026-08-17**, who isolated it precisely by accident: he clicked *"Six Bells
Above the Clouds"* in the navigator and quit → reopened on *"The House on Chicken Wheels"*; then
**repeated it while editing some text → the new scene stuck.** That difference was the whole
diagnosis.

⚠️ **Pre-existing EP-019 / I-0058 defect, unrelated to SP-102's world work** — surfaced here only
because world testing involved many quit/restart cycles.

### First fix — aimed at the wrong cause, disproved by the user's re-test

The original diagnosis was **two guards that both decline**: on quit, `saveAllDirtyBlocking` calls
(a) `saveSceneBlocking(currentIndex)`, guarded by `segments[index].isDirty`
(`ViewportSceneLoader.swift:206`) — which declines for a clean scene; then
(b) `stampWritingSurfaceBlocking`, guarded by `vpID != cursorSceneID` (`:273-275`).

The user's re-test disproved it: *"When I went to Scene 15 it came back in scene 10. When I went to
22 it came back in scene 14."* A pure persistence failure would resume on the **old** scene, not on a
**nearby-but-wrong** one.

⚠️ **A comment written during this fix claiming "navigator clicks move the caret (SP-063)" was
WRONG** — `navigateToScene` deliberately does not move the caret. Corrected in place.

⚠️ **The first fix was RETAINED on its own merits:** removing the `vpID != cursorSceneID` guard
closes a genuine hole where a clean viewport scene was never stamped at all. It simply was not this
bug.

### Actual root cause — a race in `ManuscriptTextView.navigateToScene`

`navigateToScene` sets `viewportSceneID` to the clicked scene, but `scrollRangeToVisible` then emits
scroll notifications, and the scroll handler recomputes the viewport from the **centre of the visible
rect** after a **120 ms debounce**, overwriting it.

Cancelling `scrollTask` there is useless — it cancels the *old* task; the scroll being caused
schedules a **new** one that lands afterwards and wins.

It also landed **short**, because `scrollRangeToVisible` scrolls **minimally**: the target sat at the
viewport edge while the centre still showed earlier scenes — which is exactly why 15→10 and 22→14,
the gap being however many scenes fit on screen.

**Fix:** `navigationLockUntil` (0.5 s) makes an explicit navigation authoritative, so the scroll it
causes cannot retarget it; hand-scrolling after the window is unchanged. Restore additionally
**centres** the scene (`centerStorageOffset`) rather than merely revealing it, so the viewport the
writer sees and the scene restored are the same thing (§1, Current Scene Model).

**Evidence at verification:** interop **93/93 macOS arm64**, `ctest` **520/520 macOS arm64**,
**BUILD SUCCEEDED**. User-verified live 2026-08-18: click a distant scene, quit **without typing**,
reopen → resumes there.

---

## I-0132 — The navigator did not finish the job of going to a scene

**Severity:** Medium · **Sprint:** SP-102 (adopted) · ✅ **Verified 2026-08-18**

Two halves, kept as one Issue because both are *"the navigator highlights a row and stops there"*,
and §1/§3 of the Current Scene Model govern both.

### (a) Focus stayed in the navigator on click

**Reported 2026-08-17** while verifying I-0131: *"When I click a scene in Scene Navigator the focus
stays on the navigator... Arrow keys only change the selected scene and typing text updates what is
selected. I have to click in the Manuscript View to get focus there... Tab has no effect, although
perhaps it should advance the focus to the Manuscript View."*

Navigating by click therefore required a **second, manual click** before the writer could type, and
typing while focus remained in the list was interpreted as type-select rather than as text.

`ViewportSceneLoader.takeFocus()` already existed and was wired (`onTakeFocus:`), so the capability
was present and only the trigger was missing — the same shape as I-0117/I-0123.

⚠️ **A wrong premise carried through the first three attempts:** that `navigateToScene` should
deliberately *not* move the caret ("a writer surveying her manuscript should not have her insertion
point dragged around"). **`Scrivi_Current_Scene_Model_v0_1.md` §3 rules the opposite** — a navigator
click moves the caret to the scene's **first character**, scrolls it into view, **and** transfers
focus, all three. §3 was ruled 2026-08-17, before any of this code was written, and I cited that
document in this Issue's own record without reading what it said about the caret. The defect surfaced
the moment focus started transferring: the stale caret became visible, and the user reported it —
*"the caret never changes position… it stays where it was last time I clicked."*

### ⚠️ Four attempts — the first three misdiagnosed it as a first-responder race

**Attempt 1 — synchronous `makeFirstResponder`.** `navigate(to:)` called `loader.takeFocus()`, which
called `window.makeFirstResponder(textView)` synchronously. **Archived as verified in error**; the
user reported it still broken: *"When I click a Scene in Scene Navigator it does not correctly change
keyboard focus to the ManuscriptView. The Issues I verified all specifically stated that focus changes
'when the app loads' not 'when a scene is clicked'."*

**Attempt 2 — defer one runloop pass.** `DispatchQueue.main.async` around the responder change.
Worked *sometimes*. The user: *"intermittent and appears to be related to timing… twice the focus did
not change"* — diagnosed by a detail I had not thought to use: **the navigator's selection highlight
dims when it loses focus and stays bright when it keeps it.**

**Attempt 3 — `simultaneousGesture` + claim/verify/re-claim.** Also introduced a second report:
*"sometimes when I click, the scene is not changed (although the selected scene in the Scene Navigator
is always correct)."* Each attempt made the failure rarer, which read as progress and was not.

> ⚠️ **All three were wrong, and the user stopped the fourth before it was written.** I was about to
> add an `NSEvent` monitor — reaching further *below* SwiftUI to win a fight created by reaching below
> it in the first place. His redirection: *"SwiftUI owns a lot of the interaction here… You may be
> relying on a lower level interaction when a higher-level SwiftUI interaction will be correct. We
> need to be certain we have the correct source of truth for the state we want to be in and that that
> state is propagated properly through the View hierarchy."*

### Actual cause — a one-shot trigger, not a responder race

`navigateToSceneID` is a **one-shot binding** set on click and cleared asynchronously after use
(`ManuscriptTextView.updateNSView`). Two failure modes fall out, both timing-dependent:

- **Re-selecting the same scene** wrote an unchanged value → SwiftUI coalesced the update away → no
  `updateNSView` → **no navigation and no focus transfer**.
- **A fast second click** could be clobbered by the previous click's still-pending `nil` write.

**The tell was in the user's own report and I missed it twice:** the navigator selection was *always*
correct. The table maintained selection reliably; only the parallel one-shot trigger beside it was
unreliable. Nothing was racing for first responder at all.

### Fix — selection as the single source of truth (the shape iOS already had)

macOS was the **only** platform without a `selection` binding, passing navigation through a
tap-gesture callback into the one-shot trigger — parallel state kept in step by hand. It was also the
only platform with these bugs. `EditorView` now binds `selectedSceneID` into the navigator's `List`
on macOS exactly as on iOS, and the manuscript follows from `onChange`.

Consequences, each of which was a latent bug of its own:
- **Tap gesture removed entirely** — no parallel path left to disagree with the table's selection.
- **`takeFocus` simplified** back to a single deferred call; the retry machinery was compensating for
  unreliable navigation, not for responder arbitration.
- **Delete paths rerouted** through `navigate(to:)` — they called `onNavigate` directly, which on
  macOS is now a no-op default, so they would have silently stopped working.
- **Launch double-navigation avoided** by seeding the selection to the already-restored scene.

**The caret moves to the scene's first character** (§3) — see I-0131's entry for why that had to be
added at the same time.

### Loop audit — echo suppression (user-prompted)

The user then asked for a cycle audit: *"one View sets a state, which triggers a function that resets
the same state… in some cases the real fix is to temporarily suspend notifications."*

One real cycle exists by design: manuscript scrolls → `setViewportScene` → navigator mirrors into
`selection` → `EditorView.onChange` → navigate → scroll. It terminated, but **only by value
equality** — a guard that silently assumed the scroll handler wrote `viewportSceneID` *before* the
navigator's mirror write arrived. **Correct only until that order changed.**

Replaced with the suspended-notification approach: `setViewportScene` raises
`isMirroringViewportToSelection` **before** the observable write (the navigator's `onChange` runs
synchronously off it) and lowers it once the update drains. An echo is now *known*, not inferred. The
equality check is retained as a backstop, not as the mechanism.

**Two regression tests added** (`ViewportSelectionEchoTests`), and verified non-vacuous: inverting the
flag ordering made both fail; restoring it made both pass. ⚠️ **They pin the flag's contract, not the
live loop** — they will catch someone breaking the mechanism, but not someone adding a new path that
writes `selection` without going through `setViewportScene`.

⚠️ **Accepted trade, user-ruled 2026-08-18:** taking focus on click ends arrow-key browsing of the
list after the first click. The user ruled this acceptable — *"Arrow browsing isn't strictly
necessary. Mouse Wheel and Trackpad Scrolling are still available when the keyboard focus is
elsewhere."* **Tab-as-focus-advance was consequently never needed** for this Issue and remains
unimplemented.

### (b) The navigator never revealed the selected row (widened 2026-08-18)

*"Only I had to scroll the Scene Navigator to be able to see the selected entry."* Distinct trigger
from the click case — the selection had been set by **restore**, with no click, focus or caret
involved. `SceneNavigatorView` had no `ScrollViewReader` at all (grep: zero matches), so a selection
outside the visible slice was invisible and the writer had to hunt for it.

**Fix:** the List is wrapped in a `ScrollViewReader`, scene rows tagged `.id(row.rowID)`, and
`scrollTo(anchor: .center)` fires **`onAppear` only** (`SceneNavigatorView.swift:110`) — launch being
the one moment reveal is needed, since restore sets the selection before the view exists.

> ⚠️ **The first attempt also revealed on `onChange(of: viewportSceneID)`, and the user's re-test
> disproved it:** *"every time I click a scene it scrolls the Scene Navigator 'a little bit up or
> down'."*
>
> **The comment claiming `scrollTo` "no-ops when the row is already visible" was WRONG.** `scrollTo`
> **re-anchors** an already-visible row to `.center`, so every click nudged the list by however far
> that row sat from centre.
>
> **The `onChange` reveal was removed outright, not tuned.** In practice the writer scrolls the list
> by hand and clicks a row she can already see, so it had **no case where it helped** and one where
> it actively fought her. Scene-to-scene reveal is the manuscript's job (`navigateToScene` centres
> the text), not the list's.

**Evidence at verification:** **BUILD SUCCEEDED**, interop **95/95 macOS arm64** (93 + the two new
echo tests). ✅ **User-verified 2026-08-18 on an extended live click-through** — *"I clicked about a
lot and saw no missed clicks or focus changes"* — after which the loop audit above was completed.

---

## I-0133 — `restoredScrollFraction` was written, cleared, and never read

**Severity:** Low · **Sprint:** SP-102 (adopted) · ✅ **Verified 2026-08-19**

Found 2026-08-18 while instrumenting [[I-0131]]. `ViewportSceneLoader.restoredScrollFraction` was
assigned in `loadAll` from the backend's `restored.scroll`, cleared in `restoreWritingSurface`, and
**read nowhere.**

⚠️ **ScriviCore's half was never broken.** The fraction is faithfully persisted by `saveScene` on
every quit and returned by `scrivi_open_project` — so I-0058's backend work functions correctly.
**The app simply dropped the value on the floor.**

### Why the impact is low *by design*, not by luck

Under the **Current Scene Model**, restore **centres** the restored scene ([[I-0131]]'s fix). That is
a strictly better outcome than reapplying a document-wide scroll fraction, and ⚠️ **the two approaches
contradict each other** — a centred scene and a restored fraction cannot both determine the viewport.
The dead property was therefore not a missing feature; it was a **superseded** one that no longer had
a caller.

### Ruling and fix (user, 2026-08-18)

> **Delete the Apple-side dead state; leave Linux alone.**

Removed the property, its `loadAll` parameter, the write, the clear, and the `ProjectSession`
plumbing. ⚠️ **Each removal site carries a comment recording that the omission is *by design*,** so a
future reader does not file it as a bug and "restore" it.

⚠️ **The schema field is NOT dead and was deliberately left in place** — `scrivi.h`, the open envelope
and `saveScene` are untouched, because **Linux consumes it.**

### What investigating this turned up

⚠️ Linux applies the fraction **after** `centerCursor()`, deliberately overriding the centring
(`EditorShell.cpp:364`). At the time this was filed as a parity defect ([[I-0134]]); **that framing
was wrong and I-0134 is closed as a non-issue** — parity is directional and the Apple implementation
is authoritative. See [`Issue-closed-0134.md`](../Closed/Issue-closed-0134.md).

**Related:** [[I-0018]] — *"the manuscript does not scroll to the restored scene on load"*, ✅ Verified
and archived 2026-08-19 → [`Issue-verified-0011-0020.md`](Issue-verified-0011-0020.md). It is the same
*"what does it mean to be at a scene?"* cluster as this Issue, [[I-0131]] and [[I-0132]] — across **load,
click and quit** — and all four were settled by the Current Scene Model's centring rule.

**Evidence at verification:** **BUILD SUCCEEDED**, interop **93/93 macOS arm64**; `grep` for
`restoredScrollFraction` returns **zero** references.
✅ **User-verified 2026-08-19:** *"I verify that the code was removed. Therefore it is verified."*
⚠️ **Nothing user-visible changes** — that *is* the verification: restore still lands centred on the
correct scene.

---

*Archived 2026-08-18 (I-0131, I-0132); I-0133 archived 2026-08-19 under audit ruling **R-01**.*
