# Scrivi — The Current Scene Model (v0.1)

**Status:** ✅ **Approved 2026-08-18 (user-ruled).**
**Scope:** `[Apple]` only. **No ScriviCore change** — this governs app-side caret, focus, selection and
scroll behaviour, and what gets persisted as the resume point.
**Supersedes:** the implicit and inconsistent behaviour behind **I-0018**, **I-0131** and **I-0132**.

---

## Why this document exists

Three open Issues turned out to be the same question asked in three places:

| Issue | Surface | Symptom |
| ----- | ------- | ------- |
| I-0018 | App load | Navigator highlights the restored scene; the manuscript does not scroll to it |
| I-0131 | Quit → relaunch | Resumes on the caret's scene, not the scene the writer navigated to |
| I-0132 | Navigator click | Caret does not move, focus stays in the navigator, no keyboard route out |

⚠️ **They were unfixable in isolation because the app had no single answer to "which scene am I in?"**
`viewportSceneID` (scroll-derived), `cursorSceneID` (caret-derived) and the navigator selection were three
notions that could disagree, and each surface picked a different one. Two attempted fixes to I-0131 failed
because they corrected *how* a value was written without settling *which value was right*.

---

## §1 One current scene (ruled)

**There is exactly ONE current scene.** The navigator highlight, the manuscript scroll position, the caret,
the quit-time resume point and the launch-time restore all refer to it.

> *"So, when we scroll up a new Scene and the current scene changes, the caret will also move to the new
> scene. This way you always know in what scene the caret should be, especially when restoring the current
> scene [on] app start."*

This replaces an earlier position in the same session that scrolling should leave the caret alone; the user
reconsidered and rejected it explicitly. **The two-notion model is withdrawn.**

## §2 The typing lock (ruled) — the crucial qualifier

⚠️ **While the writer is typing, the current scene is LOCKED to the scene being typed in, and scrolling does
not change it.**

> *"When the writer is typing, the Scene Navigator stays locked on the scene that the writer is typing in,
> regardless of what happens with regard to scrolling. If I'm typing at the end of a scene, and the view
> scrolls up revealing the next scene, the current scene does not change."*

**Why this matters:** typing at the end of a scene naturally scrolls the *next* scene into view. Without the
lock, §1 would move the current scene — and the caret — out from under a writer mid-sentence. The lock is
what makes §1 safe.

**The lock follows a structural move.** `Ctrl-Return` (new scene) changes the current scene and the lock
re-pins to the new one; the writer is still typing, just somewhere new.

**The lock releases when the writer stops typing and starts navigating.** Three actions mean *navigating*:

1. Scrolling the current scene out of view
2. Arrowing the caret out of the scene
3. Clicking a scene in the Scene Navigator

Once released, scrolling changes the current scene and moves the caret (§1).

## §3 Navigator click (ruled)

Clicking a scene in the Scene Navigator **selects** it:

1. The caret moves to the **first character** of that scene.
2. The manuscript scrolls the caret into view.
3. **Keyboard focus moves to the ManuscriptView** — the writer can type immediately.

> *"I am selecting the scene. The caret should move to the first character of the Scene… and focus should
> transfer to the [ManuscriptView] all ready for the writer to start writing."*

⚠️ **This reverses SP-063's scroll-without-caret ruling for navigator clicks specifically.** SP-063 governs
click-to-place *within* the manuscript, which is unchanged.

## §4 Scene-boundary caret motion (ruled — NEW functionality)

Because §3 always lands at the **start** of a scene, the writer needs a way to reach the end:

| Action | Effect |
| ------ | ------ |
| **Ctrl-Down Arrow** | Caret to the **last** character of the current scene |
| **Ctrl-Up Arrow** | Caret to the **first** character of the current scene |

Both get **menu items** as well as key bindings — a keystroke-only feature is undiscoverable.

> *"It kind of depends on if I'm reading or writing. I think that when a scene is selected we go to the
> beginning. [But] we provide a way, say Ctrl-Down Arrow, that moves the cursor to the last character in the
> Scene… And equivalent menu items."*

**App-only. No ScriviCore change.**

## §5 Scrolling with the manuscript focused

Scrolling **without** the typing lock held changes the current scene and moves the caret into it (§1). This is
the behaviour that also makes the quit-time resume point correct: the scene the writer is looking at *is* the
current scene, so there is nothing to reconcile at save time.

Existing behaviour that is **unchanged and correct**: typing or arrowing when the caret is off-screen scrolls
the caret back into view.

> *"If I start typing or arrow in some direction the ManuscriptView scrolls back to make the caret visible.
> It does this now, so no changes need to be made there."*

---

## §6 Consequences for the three Issues

| Issue | Resolved by |
| ----- | ----------- |
| **I-0018** | §1 — the restored scene is *the* current scene, so load scrolls to it and places the caret, rather than only highlighting it |
| **I-0131** | §1 + §2 — there is one scene to persist, so the caret's scene and the viewport scene can no longer disagree at quit time |
| **I-0132** | §3 + §4 — a click commits, focus moves, and Ctrl-Up/Down give caret control within the scene |

⚠️ **I-0131 must be re-diagnosed against this model, not patched again.** Two fixes have already failed:
removing the `stampWritingSurfaceBlocking` dirty-guard (aimed at a hole that was real but not this bug), and
`navigationLockUntil` (aimed at a scroll race). **Both were guesses at the mechanism while the model was still
ambiguous.** With one current scene there is a single value to write and a single value to read back, which is
what makes the fix verifiable rather than speculative.

---

*Created 2026-08-18 from the user's rulings, after two failed attempts at I-0131 established that the
underlying model — not the write path — was the defect.*
