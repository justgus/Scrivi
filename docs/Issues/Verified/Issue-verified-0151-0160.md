# Verified Issues: I-0151 – I-0160

Archived Issues, verified by the user. Batched in decades of ten.

⚠️ **All eleven Issues in this file and I-0161 came from SP-118's LIVE CLICK-THROUGH.** None was found by
any suite — the suites were green throughout. ⚠️ **They assert edge creation, duplicate rejection,
both-endpoint visibility and pending presentation, every one of which held up.** What no test covered was
whether a writer could **reach** any of it.

> ⚠️ **I-0161 is filed here too**, out of decade, so SP-118's live-pass findings stay together. The
> 0161–0170 file starts at I-0162.

---

## I-0151 — ⚠️ Push-navigation to any world-scoped object failed outright

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **High**

*"The operation couldn't be completed. (ScriviError error -1)"* on every attempt to open an Eskandar object
from the related list.

`ObjectRelationsSection.navigate` passed `worldID: ""` under **a written comment asserting `openObject`
"accepts '' and resolves it"** — ⚠️ **an assumption never checked against the code, and false.**
`ObjectStore::kindDirFor` **fails** for a world-scoped kind with an empty worldID (*"a worldID is
required"*), and `findByID`'s index lookup cannot cover for it: world objects live in the **world's**
index, and the project index is empty in exactly the projects that have a world.

Navigating from a scene inspector worked, which made it look intermittent.

✅ **The data was already there.** I-0142 moved `otherWorldID` **out** of the pending branch precisely so a
healthy object reports its home world (`RelationshipStore.cpp:495`), and the C ABI emits it
unconditionally. **Fixed** by using `edge.otherWorldID`.

> ⚠️ **A confident comment is not evidence.** Writing one is how an unchecked assumption gets laundered
> into an apparent finding.

---

## I-0152 — ⚠️ A raw `scene_…` ID shown for an untitled scene

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

The related list showed `scene_019faeb3-3326-…` — ⚠️ **useless to the writer, who has no way to tell which
scene it is.**

The Scene Navigator, given the **same data at the same moment**, shows title → live first words →
**"Scene N"**, and never an ID. The Detail Sheet had **no scene-naming rule at all** and fell through to
`otherID`, because `EndpointResolver` copies `title` verbatim and an untitled scene sends `""`.

⚠️ **Two surfaces, one scene, two names — and the more useful one already existed.**

⚠️ **Claude first dismissed this as "not a display bug"** on the grounds that the empty title was genuine
(`the-stairs-of-tintagael` has exactly one, Ch1 Sc2). ⚠️ **The user rejected that, correctly:** *why* the
title is empty is a separate question from what the writer is shown.

**Fixed** by reusing the Navigator's rule, host-computed and passed as data to preserve S8. Last resort is
"Untitled scene" — ⚠️ **never an identifier.**

---

## I-0153 — ⚠️ Double-clicking a scene row did nothing at all

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

No navigation, no message, no cursor change. `navigate()` bailed on scene endpoints with a bare `return`
because a scene has no Detail Sheet.

⚠️ **A dead affordance that looks identical to a live one is worse than a disabled one:** the writer cannot
tell whether she missed the row or the app ignored her, and the same rows navigate fine for objects. It
also blocked click-through step 13 entirely.

**Fixed:** a scene row jumps the manuscript — a scene *has* a destination, it is just not a sheet.

---

## I-0154 — ⚠️ A right-clicked row was not highlighted

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Low**

With several similarly-named rows the writer could not tell which one "Show" would act on.

⚠️ **Reported in the live pass as a note rather than a defect** — the same shape as I-0148. ⚠️ **Superseded
in practice by I-0156**, because the first fix did not work.

---

## I-0155 — ⚠️ DATA LOSS: a Detail Sheet save silently reverted a rename

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **High**

⚠️ **The most serious defect of the sprint, and it was reported as a hedge:** *"It isn't necessarily a
defect. More like an unintended consequence… Maybe there is a defect here after all."*

Both surfaces edit the same fields and **can be open on the same object at once**.
`ObjectDetailSheet.save()` patched `detail.sourceJson` — **the snapshot read when the sheet OPENED** — so
any edit made elsewhere afterwards was overwritten with the stale value. **Saving a note wrote back the old
name**, and the sheet then displayed the reverted name as though nothing had happened.

⚠️ **`ObjectCardModel.rename` already did this correctly** (`ObjectCard.swift:280`): it re-opens the object
immediately before patching. This surface simply did not follow the same rule.

**Fixed, three parts:**
1. `save()` re-reads from disk and patches **that**, ⚠️ **with per-field conflict resolution so re-reading
   does not reverse the loss** — a field the writer did not touch takes the disk value, a field she edited
   takes hers.
2. Opening the sheet closes the Inspector's inline editor for the **same** object, ⚠️ **routing an unsaved
   draft through §4.6's complete-or-discard prompt rather than discarding it.**
3. An `objectRevision` token (same mechanism as `worldRevision`) refreshes the inspector after a save.

> ⚠️ **The uncertainty in a report is not a measure of its severity** — I-0148, I-0154 and I-0155, three
> times in this Epic.

---

## I-0156 — ⚠️ Related-list rows had no selection at all

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

Clicking a row did not highlight it, and neither did right-clicking, so with **"Tall Ash"** and **"Short
Ash"** adjacent the writer could not tell which row an action would apply to.

⚠️ **Supersedes I-0154's first fix, which did not work:** `TapGesture().modifiers(.control)` does not
observe a real right-click.

⚠️ **Root cause: I hand-rolled the list.** A `VStack`/`ForEach` meant reimplementing selection, the
highlight and right-click targeting — each attempt wrong in a new way — while **`SceneNavigatorView` was
already doing it correctly with `List(selection:)` in the same directory.**

**Fixed** with genuine row selection; `List` sequences right-click targeting for free.

---

## I-0157 — ⚠️ Scene navigation bypassed I-0132's selection-is-truth ruling

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

Navigating to a scene scrolled the manuscript but left the Scene Navigator's selection behind, caused by
writing the one-shot `navigateToSceneID` trigger directly.

⚠️ **I-0132 had ALREADY ruled `selectedSceneID` the source of truth on both platforms**, precisely so
navigator and manuscript cannot drift — ⚠️ **and I-0153's fix reintroduced the split three days later.**

**Fixed:** set selection; the existing `onChange` drives the manuscript.

---

## I-0158 — ⚠️ Hand-rolled list reimplemented what `List` provides

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

⚠️ **The user's question is the record:** *"A Swift standard List View handles all this automatically,
which makes me wonder why it is so hard for you."*

It was not hard. New machinery was built beside machinery that already worked. **Fixed** by adopting
`List(selection:)` with `Section` so direction grouping survives.

---

## I-0159 — ⚠️ The related list appeared to have LOST ROWS

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **High**

Myton showed **3 of his 8** relationships.

⚠️ **Nothing was lost.** I-0158's `List` sat inside the sheet's own `ScrollView`, so it got a capped 260pt
viewport of its own; the rows were all present and the inner list *could* be scrolled — but ⚠️ **nothing
on screen indicated a second scroll region existed**, so both the writer and Claude first read it as data
loss.

⚠️ **The user diagnosed it:** *"with no indication that the scroll was there I failed to check it."*

> ⚠️ **A scroll region with no affordance is a data-loss report waiting to happen** — the same lesson as
> I-0127, where a collapsed picker showed a fraction of a list the writer could not survey. ⚠️ **Whether
> rows are absent or merely unreachable-looking, the writer concludes her work is gone** (Doc 3: absence is
> never deletion).

**Fixed:** `LazyVStack` in the sheet's existing ScrollView, no height cap — one scroll region, sized to
the window.

---

## I-0160 — ⚠️ I-0155 was fixed in ONE DIRECTION ONLY

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

A Detail Sheet save refreshed the Scene Inspector; ⚠️ **an Inspector rename left the Detail Sheet showing
the old name.** Same two surfaces, same stale-view defect, same session.

⚠️ **I built and tested the half I had just written** — `feedback_verify_each_half_separately`, the exact
trap that memory exists for.

**Fixed:** `onObjectChanged` from the card through the context to the host, which bumps the same
`objectRevision` the sheet now watches. Both directions, one counter.

---

## I-0161 — ⚠️ Navigator did not reveal a scene navigated to from elsewhere

**Verified 2026-08-23 (user-approved).** SP-118 · `[Apple]` · **Medium**

⚠️ **Three attempts, and the first two failed for DIFFERENT reasons.**

**Attempt 1** scrolled at double-click time. Wrong: on macOS this list's highlight is driven **only** by
`loader.viewportSceneID`, which the manuscript publishes **after** it finishes scrolling — so it scrolled
to a row that was not current yet and the highlight landed elsewhere. ⚠️ **Two things moving on different
clocks**, which is why the manuscript "moved faithfully" and the list did not. ⚠️ **It is NOT a keyboard-
focus problem**, the first hypothesis on both sides.

**Attempt 2** fixed that correctly but ⚠️ **was never in the macOS build**: `EditorView` has **two**
`SceneNavigatorView` call sites — an `#if os(iOS)` one and a macOS one — and the edit reached only the iOS
site, so `revealRequest` kept its `nil` default on the platform actually running. ⚠️ **Claude then
diagnosed a "timing race" in code that never executed**, and the user's *"It didn't change anything"* was
the correct verdict on a build where the feature was absent.

**Attempt 3** — found **statically** by counting call sites, not by another theory.

✅ **Confirmed on the rig** by temporary `SCRIVI-DIAG` logging, which showed the true ordering:
`REQUEST` (viewport still on the last scene) → `setViewportScene` → gate matches → `SCROLLING`. **Logging
removed once it had done its job.**

⚠️ **Requests carry a token and expire after 2s**, so navigating twice to the same scene still fires and a
navigation that never lands cannot fire against unrelated later state.

> ⚠️ **THE RULE: prove new code is REACHED before explaining why it behaves oddly.** One log line, or one
> grep for call sites, would have replaced a whole round of theory. ⚠️ **"It didn't change anything" should
> first be read as "it isn't running", not "it ran and was wrong."**

---

*Last Updated: 2026-08-23 (**I-0151 – I-0161 ✅ Verified, user-approved, at SP-118 close.** ⚠️ **All eleven
came from the live click-through and none from any suite.** ⚠️ **Four were the same failure — an existing
correct pattern the new code did not follow** (I-0152, I-0155, I-0157, I-0158), each violating a rule
already written in this repo. ⚠️ **I-0155 was silent data loss reported as "maybe not a defect."**
I-0161 is filed here out of decade so SP-118's findings stay together; the next file starts at I-0162.)*
