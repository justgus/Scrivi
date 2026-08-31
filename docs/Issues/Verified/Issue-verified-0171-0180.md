# Verified Issues — I-0171 … I-0180

⚠️ **New decade file.** The previous decade closed at **I-0170**
([`Issue-verified-0161-0170.md`](Issue-verified-0161-0170.md)).

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0172** | `[Apple]` ⚠️ **`ForkPopover` forced a nested AppKit layout pass to measure its content** | Low | ⚠️ **None** — unassigned, EP-019-adjacent | ✅ **Verified 2026-08-25** |

---

## I-0172 — `ForkPopover` forced a nested AppKit layout pass

**Filed:** 2026-08-25 (SP-122, from **T-0469's console log**) · **Verified:** 2026-08-25 (user-approved)

### The defect

`ForkPopover.show(fork:in:onSelect:onCancel:)` sized its popover by calling
`host.view.layoutSubtreeIfNeeded()` and then reading `fittingSize`. ⚠️ **`layoutSubtreeIfNeeded()` forces
a layout pass**, and when `show` is invoked from inside an existing pass AppKit logs:

> *"It's not legal to call -layoutSubtreeIfNeeded on a view which is already being laid out. If you are
> implementing the view's -layout method, you can call -[super layout] instead."*

⚠️ **Logged ONCE per session, with no user-visible symptom.** ✅ **Filed anyway because a forced nested
layout is the class of defect that becomes a real layout bug later** — not because anything was broken.

### The fix — `ForkPopover.swift:70-88`

Measure with **`NSHostingController.sizeThatFits(in:)`**, which asks SwiftUI to size the content directly
and provokes no nested layout pass on the host view.

⚠️ **Measured UNBOUNDED (`.greatestFiniteMagnitude`), deliberately.** A first attempt passed a bounded
`420×320` proposal — ⚠️ **that would have been a silent behaviour change**: content wider than 420
previously reported its full width and was then clamped, but under a bounded proposal it wraps and
reports a **taller** height instead. ✅ **The existing clamp remains the single place bounds are applied**,
so the resulting popover geometry is identical to before.

### ⚠️ How this was verified — and how it was NOT

✅ **Verified 2026-08-25 on user approval.**

⚠️ **The evidence is COMPILATION plus user approval, not exercise of the popover.** Recorded plainly:

- ⚠️ **The fork popover appears only when redoing into a branch point in the undo history**, which no test
  in SP-122 exercised.
- ⚠️ **The warning it removes was logged ONCE in an entire session**, so its absence from a future log
  would be weak evidence either way.
- ✅ **`BUILD SUCCEEDED`** under Xcode 27 (AppleClang 21), no new diagnostics.

⚠️ **The behavioural check that was never run:** hit a redo fork, confirm the popover still sizes
correctly and the warning is gone. ⚠️ **If a sizing regression ever appears in that popover, THIS is the
change to suspect first.**

### Why it carries no Epic

`ForkPopover` is the **undo/redo history fork picker**, so ⚠️ **EP-019 is where this file gets real
attention** — but this Issue is ⚠️ **NOT an EP-019 commitment** and was not scheduled into it.
⚠️ **It is explicitly NOT EP-035**, which is `[Linux]` Qt/QML and would never open this Swift file.

---

*Archived 2026-08-25 on user verification, in the same step it was marked Verified.*

---

## I-0173 — `[Linux]` Scene Inspector rows elided the relationship label

**Severity:** Low · **Sprint:** SP-125 (EP-035) · **Verified:** 2026-08-28

⚠️ **FOUND BY THE LIVE CLICK-THROUGH** (EP-035 AC9), not by any suite.

At the panel's default width (200px — the user-preferred default, narrower than Apple's 280) a row
rendered `character 1 — doc…`. ⚠️ **The elided text was "documented by"** — the `cites` type's **INVERSE**
label, projected by the core because the scene is the `to` endpoint.

✅ **The value was CORRECT and correctly read** — the panel does not recompute direction, which is
EP-037's known trap. ⚠️ **It simply could not be SEEN.**

### Why no suite could have caught it

⚠️ **All 571 ctests and all 23 `scene_inspector_smoke` checks were GREEN with this present.** The defect
is in what a *writer can read*, not in what the code computes. ✅ **This is the AC9 ruling paying for
itself on its first sprint.**

### The fix

Every row carries a tooltip with its full text; a pending row carries **both** its label and its pending
sentence rather than one replacing the other.

⚠️ **The tooltip is a SUPPLEMENT, not the only route** — the name stays the leading text and the panel is
resizable. ⚠️ **Apple's T-0389 exists precisely because a pending object's world once appeared in a
tooltip ALONE**, which is hover-only, undiscoverable, and absent entirely on iPad.

⚠️ **Carried caveat:** tooltip *rendering* was never visually confirmed — Xvfb plus synthetic pointer
motion does not reliably fire Qt's hover timer. ✅ **Verified by code and by the on-disk label**, and by
the user's live pass over the surface as a whole.

---

## I-0175 — `[Process]` A synthetic-input driver typed into a real manuscript

**Severity:** Medium · **Sprint:** SP-125 · **Verified:** 2026-08-28

⚠️ **THIS WAS MINE, NOT THE APP'S.**

Driving the Linux app with `xdotool` to switch projects, I sent `ctrl+q` to close
`the-lone-golem.scrivi` and then typed the next project's path. ⚠️ **The quit did NOT take**, so the path
went into the OPEN EDITOR — landing as `Myton stopped./projects/the-stairs-of-tintagael.scrivi` in
`manuscript/chapter-001/001-opening-scene.md`, plus a spurious blank line. ⚠️ **The app's idle-save then
persisted it to disk** — the T-0239 debounce doing exactly its job.

### Repair

✅ **Restored from the user's Desktop copy — the scene is byte-for-byte identical to the original.**
✅ **`characterCount` was restored from the original rather than recomputed**, so no different counting
rule could creep in. ⚠️ **Only `modifiedAt` and the container identity differ** — honest records that the
Linux app touched the file, not damage.

### Root cause and the rule adopted

⚠️ **Blind synthetic input goes to whatever holds focus, and I assumed a UI transition had happened
instead of CHECKING.** ⚠️ **Third occurrence of the I-0150 pattern** — an automated action writing to
real writing work.

✅ **Rules adopted:**
1. ⚠️ **NEVER drive synthetic input at a session holding real work** — use a throwaway copy.
2. ✅ **Screenshot and CONFIRM the expected surface has focus before typing.**
3. ⚠️ **Never assume a transition succeeded because the keystroke was sent.**

⚠️ **The deeper point: a click-through on real work should be done BY A HUMAN.** ✅ **That is exactly what
AC9 asks for, and automating it is what created this.**

---

*I-0173 and I-0175 archived 2026-08-28 on user verification, in the same step they were marked Verified
and in the same step SP-125's five Tasks were archived (`feedback_archive_on_close`).*

---

## I-0171 — `[Build]` Both `.dockerignore` files excluded only `build/`

**Severity:** Medium · **Sprint:** SP-122 (⚠️ **fixed in SP-125**) · **Verified:** 2026-08-29

⚠️ **This Issue was filed by SP-122, RECURRED in SP-125, and blocked the `ctest` run OUTRIGHT** before
being fixed — so it cost real time twice, and ⚠️ **both times it was found by RUNNING the leg, never by
reading the file.**

### The defect

Both ignore files matched only `build/`, while **five** build directories exist in the working tree:
`build/`, `build-tests/`, `build-linux-tests/`, `build-iphoneos/`, `build-iphonesimulator/`.

Any one of them entering the Docker context poisons the in-container configure:

> *"The current CMakeCache.txt directory /src/build-tests/CMakeCache.txt is different than the directory
> /Users/justgus/… where CMakeCache.txt was created."*

⚠️ **SP-121's original fix was incomplete because it matched the ONE directory it had seen** rather than
the pattern.

### The fix

✅ **Both files now match the FAMILY, not a name** — `build/`, `build-*/`, `**/build/`, `**/build-*/`:

- `platforms/linux/docker/Dockerfile.dockerignore`
- the root `.dockerignore`

### Evidence

✅ **`ctest` ran 571/571 NON-ROOT with tests ON** in a from-scratch `--no-cache` image immediately after —
⚠️ **the run that was impossible before the fix.**

### ⚠️ Carried caveat — NOT closed by this fix

⚠️ **The two-file relationship remains an unruled duplicate.** Docker applies `<dockerfile>.dockerignore`
**only** for that Dockerfile, so ⚠️ **a third Dockerfile would silently fall back to the weaker root
file** — which is exactly the shape of the original defect. **Worth a ruling before another Dockerfile
is added.**

---

*I-0171 archived 2026-08-29 on user verification, in the same step SP-125 was closed
(`feedback_archive_on_close`).*

---

## I-0179 — `[Linux]` The relationship label was wrong, and then redundant

**Severity:** Low · **Sprint:** SP-126 · **Verified:** 2026-08-30 (build 8)

⚠️ **FOUND BY THE USER in the live pass**, on real data. ⚠️ **The fix took THREE rounds, and only the
third addressed the actual defect** — worth recording, because the first two looked like fixes.

### Round 1 — the message quoted the wrong string

Double-clicking a pending character produced *«"Myton at 23 — features" can't be opened yet…»*.
⚠️ **The message recovered the name by string-surgery on the ROW TEXT**, which renders as
`"<name> — <label>"`, so the label came along. ✅ Fixed by carrying the display name in its own item
role. ⚠️ **A row's visible text is a PRESENTATION; parsing it back apart to recover data is the bug.**

### Round 2 — the row itself still read wrong

⚠️ **The user pointed out the display had the same fault**, not just the message. *"Myton at 23 —
features"* implies Myton features something. He does not: the stored edge is *Myton **appears in**
scene*, and the core projects the inverse for the queried endpoint — ⚠️ **so the label describes what
the SCENE does.** Stacking name over label (Apple's layout) fixed the grammar.

### ⚠️ Round 3 — the user found the REAL defect: it was redundant

⚠️ **Stacking kept the duplication.** A scene relates to its objects the same way every time, so the
identical word repeated down the whole list while distinguishing nothing.

✅ **User ruling: hoist the label to the GROUP HEADER** — `characters (2) (features)` — and drop the
per-row line entirely. ✅ **Says it once, compacts the list, loses nothing.**

⚠️ **My supporting analysis was ALSO wrong and the user corrected it.** I claimed labels varied per
kind, having counted `cites` edges — ⚠️ **but `cites` runs source→object and NEVER touches a scene**,
so it is not a scene predicate at all and surfaces on the Sources card. ✅ **Verified against the real
project.** Of the seeded vocabulary only **two** types constrain to a scene (`appears-in` → *features*,
`located-at` → *takes place at*), so labels are collected from the rows and all present are named.

### ⚠️ What it cost elsewhere

✅ **The same defect was found in the macOS app** (`ObjectCard.swift:1032-1036`), ⚠️ **shipping since
EP-031 and never noticed** → **I-0180**. ✅ **Building the surface a second time is what exposed it.**

---

*I-0179 archived 2026-08-30 on user verification. ⚠️ **This entry CLOSES the 0171–0180 decade**; the
next Issue is I-0181.*
