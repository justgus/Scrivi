# T-0417 — `[Apple]` Scene/Chapter Boundary Navigation (SP-102)

**Status:** ✅ **Verified 2026-08-18** (user-approved — *"the 0417 scene chapter menu items all passed
and are verified"*)
**Epic:** EP-031 (adopted mid-sprint; ⚠️ **not EP-031 work** — see below)
**Sprint:** SP-102 · **Record of truth:** [`Sprint-active.md`](../../Sprints/Sprint-active.md)
**Codebase:** `[Apple]` only — no ScriviCore change.

---

## What shipped

Four menu items that move the caret to a scene or chapter boundary:

- **Scene menu** → *Go to Scene Start* · *Go to Scene End*
- **Chapter menu** → *Go to Chapter Start* · *Go to Chapter End*

**Implementation:** `Coordinator.moveToSceneBoundary(_:in:)` and `moveToChapterBoundary(_:in:)`
(`ManuscriptTextView.swift`), reached through four new `ProjectSession` actions
(`sceneStartAction` / `sceneEndAction` / `chapterStartAction` / `chapterEndAction`) following the
established `createSceneAction` pattern — **take focus first**, since a menu click leaves first
responder alone and moving the caret in an unfocused text view would hide the very thing the command
exists to show.

Chapter bounds are derived by finding the caret's scene and **widening across the contiguous run of
segments sharing its `chapterID`** — scene order in `segments` is manuscript order, the same
assumption the Linux chapter-reorder splice makes.

⚠️ Unlike `navigateToScene`, these **do** move the caret: they are editing moves, not survey gestures.

## ⚠️ Shipped deliberately incomplete — no key equivalents

The user asked whether Ctrl-Up/Ctrl-Down had been implemented for scene navigation, then established
**by direct trial** that no free combination exists on macOS:

| Combination | Already owned by | Shift-variant |
| ----------- | ---------------- | ------------- |
| ⌘↑ / ⌘↓ | Document start / end (`NSTextView`) | extends selection |
| ⌥↑ / ⌥↓ | Paragraph start / end (`NSTextView`) | extends selection |
| ⌃↑ / ⌃↓ | **macOS Mission Control** — not ours to take | — |

> *"So, we don't have any good candidates for Scene Begin and Scene End. But we can add the menu items
> to the Scene menu (and equivalent Chapter begin and Chapter end menu items to the Chapter menu),
> which will let us test the functions while we figure out the key press."*

Every candidate costs an existing editing behaviour, so the **functions** ship and the **binding stays
an open question**. ⚠️ **The key equivalent is still owed** — verifying these menu items verified the
*behaviour*, not a complete feature. If Scene/Chapter boundary motion is wanted from the keyboard, it
needs a new Task and a ruling on what to bind (or a modifier layer that takes nothing away).

## Why it is in SP-102 at all

⚠️ **Adopted mid-sprint and out of the Epic's scope.** T-0417 serves no EP-031 acceptance criterion;
it came out of the user's live navigator testing during SP-102 and was built at his direction — the
same pattern as I-0131's adoption. Recorded so the sprint's content matches its record.

## Evidence at verification

| Suite | Result |
| ----- | ------ |
| macOS interop | **99/99 macOS arm64** |
| ScriviCore `ctest` | **520/520 macOS arm64** |
| App build | **BUILD SUCCEEDED** |

User-verified live 2026-08-18: all four menu items exercised and passing.

⚠️ Per the SP-106 standing practice, **both figures name their architecture.** Neither suite covers
this Task directly — the menu items have no automated coverage — so the user's live pass is the whole
of the behavioural evidence, and the suites establish only that nothing else regressed.

---

*Archived 2026-08-18 on user verification.*
