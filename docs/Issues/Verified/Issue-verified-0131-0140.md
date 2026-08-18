# Verified Issues: I-0131 – I-0140

Archived Issues, ✅ **Resolved - Verified** by the user. Batched in decades of ten per
`Issue-GUIDELINES.md`.

| ID | Title | Severity | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| I-0131 | `[Apple]` Resume scene only persisted as a side effect of saving a **dirty** scene | Medium | SP-102 | 2026-08-18 |
| I-0132 | `[Apple]` Navigator never revealed the selected row, and a click left focus stranded in the list | Medium | SP-102 | ⚠️ **partial — see below** |

> ⚠️ **I-0132 WAS ARCHIVED HERE PREMATURELY (2026-08-18).** Its **reveal half (b)** is genuinely
> verified. Its **focus half (a) was not** — the user's verification covered focus *on app launch*,
> which he stated explicitly, and I recorded that as verifying click-to-focus as well. **It did not
> work on click.** The Issue has been **returned to `Issue-active.md`** with the focus half
> 🟠 *Resolved - Not Verified* pending a second fix. Half (b)'s record below stands; half (a)'s does
> not. **Do not cite I-0132 as verified evidence for anything focus-related.**

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

⚠️ **A deliberate design point colliding with an incomplete one:** `navigateToScene` intentionally
does not move the caret (scroll-without-caret is correct — a writer surveying her manuscript should
not have her insertion point dragged around), but **nothing offered her a way to then commit to that
scene.** `ViewportSceneLoader.takeFocus()` already existed and was wired (`onTakeFocus:`), so the
capability was present and only the trigger was missing — the same shape as I-0117/I-0123.

**First fix — did not work, and was archived as verified in error.** `navigate(to:)` calls
`loader.takeFocus()` (`SceneNavigatorView.swift:285`), which calls
`window.makeFirstResponder(textView)` **synchronously**.

⚠️ **The user reported it still broken 2026-08-18:** *"When I click a Scene in Scene Navigator it does
not correctly change keyboard focus to the ManuscriptView. The Issues I verified all specifically
stated that focus changes 'when the app loads' not 'when a scene is clicked'."*

**Cause:** SwiftUI's `List` is backed by an `NSTableView`, which claims first responder while
completing its own mouse-down handling. A `makeFirstResponder` issued from inside `onTapGesture` runs
**before** that, so the table takes focus straight back. **This is exactly why it appeared to work on
launch but not on click** — launch-time focus has no competing responder change, so the synchronous
call was never contested there. The verification evidence and the failing case were different code
paths, and I did not distinguish them.

**Second fix (2026-08-18, awaiting verification):** `takeFocus` hops to the next runloop pass via
`DispatchQueue.main.async` (`ManuscriptTextView.swift:54-70`) so the table finishes first and the
focus transfer is the last word. Safe for the other eight `takeFocus()` callers: every one sets its
selection *before* calling, and none reads focus state afterward.

**The caret is still not moved** — focus and caret position remain separate concerns.

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

**Evidence at verification:** **BUILD SUCCEEDED**, interop **93/93 macOS arm64**. User-verified live
2026-08-18.

---

*Archived 2026-08-18 on user verification of both Issues.*
