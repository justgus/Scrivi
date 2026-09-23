# Sprint SP-137 — ⚠️ **The Object Detail Sheet: real chrome, and a guard for the class that crashed us**

| Field | Value |
| ----- | ----- |
| **Sprint** | **SP-137** |
| **Epic** | **[EP-040]** `[Apple]` **The Editor Shell** |
| **Study step** | **S6** (app-shape §4.4) |
| **Serves** | **[EP-040] AC8** |
| **Planned** | **2026-09-23** |
| **Size** | ⚠️ **MEDIUM** |

**Status:** 🟢 **ACTIVE — ALL EIGHT ACs MET, ⚠️ AC2 and AC8 USER-VERIFIED 2026-09-23.**
⚠️ **T-0547 took FOUR implementations and produced THREE Issues** ([I-0248] · [I-0249] · [I-0250]),
✅ **all three resolved; [I-0250] user-VERIFIED.**
⛔ **T-0548's RUNTIME half was REMOVED by user ruling** — ✅ **its static half remains in CI.**
⚠️ **The Sprint is closable on the user's approval** — ⛔ **Claude may not close a Sprint.**
✅ **All FOUR questions RULED (Q1–Q4).** ⚠️ **AC1, AC3–AC7 met; ⛔ AC2 FAILED the live pass ([I-0248]) and is re-implemented, not re-verified.**
✅ **`xcodebuild test` 134/134 · `ctest` 626/626 · all THREE guards green.**

---

## ⚠️ Why this Sprint exists, and what changed under it this week

✅ **EP-040's thesis in one line: the app hand-builds what the platform already provides.**
⚠️ **[SP-134] fixed the toolbar · [SP-135] the bars · [SP-136] the Inspector.** ✅ **This surface is the
THIRD instance of the class** (app-shape §4.4) — ⛔ **and the LAST Sprint in the Epic.**

⚠️ **THE HAND-BUILT CHROME, read from `ObjectDetailSheet.swift`:**

```swift
private var toolbar: some View {
    HStack(spacing: 8) {
        Button { requestStep(back: true)  } label: { Image(systemName: "chevron.backward") }
        Button { requestStep(back: false) } label: { Image(systemName: "chevron.forward")  }
        Spacer()
        Button("Cancel") { revert() }
        Button("Save")   { save() }.keyboardShortcut("s", modifiers: .command)
        Button { requestClose() } label: { Image(systemName: "xmark") }
    }
    .buttonStyle(.borderless).padding(8)
}
```

⚠️ **THAT IS WINDOW CHROME INSIDE A CONTENT PANE:** ✅ **navigation history, a save affordance, and a
CLOSE BUTTON.**

### ⛔ §4.4's option (a) IS VOID, and the Sprint must not re-litigate it

⚠️ **§4.4 offered (a) keep D1-E and fix only the chrome · (b) a real auxiliary window · (c) a second
trailing column.** ⛔ **(a) DESCRIBES A SHAPE THAT NO LONGER EXISTS.**

✅ **[I-0245], during [SP-136]'s live pass:** ⚠️ **D1-E's non-modal beside-pane made the window's widths
UNSATISFIABLE — manuscript + pane + `.inspector` column could never all clear their minimums, and the
app CRASHED** (`NSGenericException: … more Update Constraints in Window passes than there are views in
the window`). ✅ **The fix made it a REAL modal `.sheet`, user-Verified 2026-09-23.**

⚠️ **§4.4 PREDICTED THIS COUPLING** — ✅ ***"(c) and S4 CONTEND FOR THE SAME TRAILING EDGE … S6 must
therefore be sequenced AFTER S4"*** — ⛔ **and it arrived as a crash rather than a design discussion.**
✅ **The sequencing rule was right; the cost of learning it this way is recorded in
[`Task-verified-0546.md`](../Tasks/Verified/Task-verified-0546.md).**

### ✅ THE HOSTING RULING (user, 2026-09-23) — **Q1: KEEP THE MODAL SHEET**

✅ **The modal `.sheet` is now the INTENDED shape, not an accident of a crash fix.**
⛔ **(b) auxiliary window: DECLINED** — ⚠️ **it restores D1-E's non-modal intent but costs the window
type [EP-018] documented, and is larger than this Sprint.**
⛔ **(c) second trailing column: DECLINED** — ⚠️ **[I-0245] measured why: the trailing edge does not have
room for two, and reconciling them means the Inspector must auto-close, which trades one surprise for
another.**

⚠️ **THIS CLOSES [I-0247]**, which was raised deliberately at [SP-136]'s close so the supersession was
RULED and not absorbed silently.

---

## ✅ Scope

### Q2 ruling (user): **ALL OF IT — a real sheet toolbar**

| Control | ⚠️ today | ✅ after |
| ------- | -------- | -------- |
| **Close (✕)** | ⚠️ `HStack` button | ✅ **toolbar `.cancellationAction` — LEADING** (⚠️ **keeps `.cancelAction`**) |
| **Undo / Save Changes** | ⚠️ `HStack` "Cancel"/"Save" | ✅ **toolbar `.principal` — CENTRED, RENAMED, Save NOT prominent** |
| **Back** | ⚠️ `HStack` button, always shown | ✅ **toolbar `.navigation` — TRAILING, HIDDEN when there is nowhere to go** |
| ⛔ **Forward** | ⚠️ `HStack` button | ⛔ **REMOVED FROM THE REQUIREMENTS** |

### ⚠️ Q2 was RE-RULED mid-implementation (user, 2026-09-23) — ✅ **both halves**

⛔ **[T-0547] first read Q2 as "Save/Cancel stay content-level"** — ⚠️ **on the reading that they are
document actions, not window chrome, and a sheet puts confirmation at its content's foot.**
✅ **THE USER RULED OTHERWISE:** ⚠️ ***"The toolbar should contain the Cancel Save buttons. Although,
since they do not dismiss the panel, they should be centered."*** ✅ **The premise was right — they do
not dismiss — ⛔ but the conclusion was PLACEMENT, and placement was the user's call.**

✅ **AND THE LABELS NOW SAY WHAT THEY DO:** ⚠️ ***"'Cancel' should instead read 'Undo Changes' and Save
should read 'Save Changes' and it should not be highlighted."*** ⛔ **"Cancel" reads as *dismiss*,
which this button has NEVER done; a prominent Save promises a dismissal it does not deliver.**

### ⛔ FORWARD IS RETIRED — ✅ **`NavigationStack` is the official model (user, 2026-09-23)**

✅ **THE USER'S RULING:** ⚠️ ***"Let's make it a NavStack officially and lose the forward behavior from
the requirements. Forward buttons are not required. We have navigate forward by double clicking the
object reference. That will also be much easier to implement on iOS and visionOS as it is a standard
UI metaphor."***

⚠️ **WHAT WAS CHECKED BEFORE RULING** (⛔ **not answered from memory**): ✅ **`NavigationPath`'s ENTIRE
API is `init` · `append` · `removeLast` · `count` · `isEmpty` · `codable`** — ⛔ **`removeLast`
DISCARDS.** ⚠️ **`NavigationStack`'s doc page mentions "forward" ZERO times, and no SwiftUI, AppKit or
UIKit component offers it** (`NavigationSplitView` is columns, `TabView` is siblings,
`UINavigationController` is the same stack scheme, `NSPathControl` is a filesystem path).
✅ **That is WHY browsers hand-roll a cursor — and why D2-B originally did.**
✅ **With forward gone, push/pop IS the whole requirement, and that is the standard metaphor.**

⚠️ **⛔ BUT THE MODEL CANNOT BE DELETED, and this is the load-bearing constraint:**
⛔ **`NavigationStack`'s own back chevron is NOT interceptable** — ⚠️ **it pops, and SwiftUI offers no
"about to pop" hook.** ✅ **Every exit from this sheet is deferred through `pendingExit` until the
writer answers the unsaved-changes prompt (T-0452).** ⛔ **A free chevron would discard her edits
SILENTLY — the [I-0155] / [I-0165b] class this Epic keeps closing.**
✅ **So Back stays an EXPLICIT toolbar item routed through the same guard, and
`ObjectDetailHistory` survives as a plain back-only STACK** (⛔ **no longer a cursor**).

### Q3 ruling (user): **a layout-convergence guard**

⚠️ **[I-0245] took FOUR attempts and `xcodebuild test` 133/133 + `ctest` 626/626 stayed GREEN through
every one.** ✅ **[SP-149] set the precedent: a regression guard is its own Task.**

---

## ✅ Tasks

| Task | Title | AC |
| ---- | ----- | -- |
| **T-0547** | ✅ **Real sheet chrome** — close (leading) · Undo/Save Changes (centred) · Back (trailing) as `.toolbar` items; ⛔ the hand-built `HStack` DELETED with a tombstone; ⛔ **forward RETIRED** | **AC1–AC5** |
| **T-0548** | ✅ **A layout-convergence guard** for the [I-0245] class — ⚠️ **TWO halves per Q4: a `scripts/` static guard (CI) + a debug Update-Constraints assertion** | **AC6** |

---

## ✅ Acceptance Criteria

- [x] **AC1** — ⛔ **THE HAND-BUILT `toolbar` `HStack` IS GONE**, ✅ **with a tombstone naming what
      replaced it.** ⚠️ **Back and close are REAL `.toolbar` items; Undo/Save Changes are centred `.principal` items.**
      ⛔ **Forward is GONE — retired from the requirements 2026-09-23.** ✅ **[EP-040] AC8.**
- [x] **AC2** — ✅ **VERIFIED BY THE USER 2026-09-23 (live pass).** ⚠️ ***"When changes are present in
      the view and the writer escapes, the confirmation dialog appears and works properly. Also
      unsaved changes on the previous view when the back navigation button is clicked also pops up the
      confirmation dialog 'You have unsaved changes on Colm'. Totally correct."***
      ✅ **BOTH GUARDED EXITS CONFIRMED — the three-way Esc prompt AND the two-way pop prompt** (⚠️ which
      also proves the off-by-one fix: the prompt names the DEPARTED object, not the arrival).
      ⛔ **It took THREE attempts** ([I-0248] a toolbar that did not render · [I-0250] a shortcut a
      toolbar never receives) — ✅ **the fix is `.onExitCommand` on the CONTENT.**
      ⚠️ **ESC STILL CLOSES, AND STILL THROUGH THE GUARD.** ✅ **[I-0245] ruled this: Esc
      must work (*"it is expected behavior and people would notice if it didn't"*), but SwiftUI's own
      dismissal BYPASSES `requestClose()` where the unsaved-changes prompt lives.** ⛔ **If the close
      control moves to a toolbar, `.keyboardShortcut(.cancelAction)` MUST move with it, and
      `.interactiveDismissDisabled()` MUST stay on the host.** ⚠️ **Verify with UNSAVED EDITS present.**
- [x] **AC3** — ✅ **Save and Cancel are UNCHANGED in BEHAVIOUR** (⚠️ **renamed and moved, but the
      actions are identical**): ⚠️ **Cancel REVERTS (T-0452), Save
      writes and KEEPS THE SHEET OPEN, and the [I-0246] status banner still appears and still clears
      on the next keystroke.** ⛔ **This Sprint restyles chrome; it does not re-rule document actions.**
- [x] **AC4** — ✅ **NAVIGATION STILL WORKS, AS RESCOPED**: ⚠️ **double-clicking a related object
      PUSHES, Back POPS, the unsaved-edits prompt still fires on both ([I-0168]), and the [I-0246]
      banner still clears when the sheet moves to a DIFFERENT object.** ⛔ **No forward step exists.**
      ⚠️ **Back HIDES when the stack has one entry — it does not grey.**
- [x] **AC5** — ⛔ **NOTHING ELSE MOVES.** ✅ **`EditorView`'s `.sheet` host, the Inspector column, the
      bars and `ManuscriptTextView` are untouched.**
- [x] **AC6** — ✅ **A GUARD FOR THE [I-0245] CLASS — ⚠️ NOW THE STATIC HALF ONLY.**
      ⛔ **Q4 ruled BOTH halves and both were built and proven red** — ⚠️ **but the user REMOVED the
      runtime assertion on 2026-09-23 after it fired twice on real work and MISDIAGNOSED both times,
      and turned a recoverable layout renegotiation into a CRASH** ([I-0249]).
      ✅ **`scripts/check-layout-convergence.sh` remains, wired into Apple CI.**
      ⚠️ **Q4's reasoning is superseded by evidence: a threshold `preconditionFailure` on a
      recoverable condition is an outage, not a guard.** ⚠️ **It must FAIL on the pre-fix shape** — ⛔ **a
      green test that cannot fail is not evidence** (`feedback_boundary_tests_not_facade`'s sibling
      rule). ✅ **Prove it by INJECTING the defect (restore the flexible `HStack` sibling) and watching
      it go red.**
- [x] **AC7** — ✅ **`xcodebuild` build + test clean on macOS · `ctest` clean.** ⚠️ **Run with the app
      QUIT** (⛔ **[I-0150]: `xcodebuild test` LAUNCHES the app and has rewritten a real project**).
- [x] **AC8** — ✅ **VERIFIED BY THE USER 2026-09-23.** ⚠️ ***"I tested all button choices on all the
      confirmation dialogs and they all work properly."*** ✅ **Back navigation · Colm displays · push
      checks for changes · Esc prompts · every dialog button exercised.**
      ⚠️ **A LIVE PASS.** ✅ **Open a Detail Sheet · use back/forward · Esc with unsaved edits
      (prompt) · Esc with none (closes) · Save (banner, sheet stays) · Cancel (reverts, banner) · type
      again (banner clears) · navigate to another object (banner clears).**

---

## ✅ Questions owed before implementation — ⛔ **NONE REMAIN**

✅ **Q1 (hosting), Q2 (chrome scope) and Q3 (guard-as-its-own-Task) are RULED above.**
✅ **Q4 is RULED below (user, 2026-09-23).**

### ✅ Q4 RULING (user, 2026-09-23): **(iii) + (i) — the static guard AND the debug assertion**
✅ **(i) A DEBUG ASSERTION in the app — ✅ ADOPTED** — ⚠️ **counts Update-Constraints passes per window and traps
past a sane bound.** ✅ **Catches the real thing, on any surface; ⛔ debug-only, so CI never sees it.**
⛔ **(ii) A HEADLESS LAYOUT TEST — ⛔ DECLINED** — ⚠️ **hosts the editor in an `NSWindow` off-screen and asserts the
pass count settles.** ✅ **Runs in CI; ⚠️ SwiftUI layout under test is fragile and may not reproduce.**
✅ **(iii) A STATIC GUARD — ✅ ADOPTED** — ⚠️ **a `scripts/` check that fails if a flexible-width view is added as an
`HStack` sibling in `manuscriptDetail`.** ✅ **Cheap, deterministic, CI-friendly; ⛔ narrow — it guards
the SHAPE, not the property.**
✅ **RULED: (iii) + (i), as recommended.** ✅ **The static guard is what actually runs on every push**
(the [SP-149] precedent), ⚠️ **and the debug assertion catches the general case during live passes.**
⛔ **(ii) is DECLINED: it promises the most and is the least likely to hold** — ⚠️ **SwiftUI layout
under test is fragile, and AC6 requires the guard be proven RED by injecting the defect. A headless
layout test is the option most likely to fail that bar.**

⚠️ **WHAT THIS MEANS FOR T-0548 — ⛔ BOTH halves are required, and AC6 applies to EACH:**
✅ **(iii) the static guard** — ⚠️ **a `scripts/` check that fails if a flexible-width view is added as
an `HStack` sibling in `manuscriptDetail`.** ✅ **It must go RED against the pre-fix shape.**
✅ **(i) the debug assertion** — ⚠️ **counts Update-Constraints passes per window and traps past a sane
bound.** ✅ **It must be proven to trap by restoring the [I-0245] shape in a debug build.**
⛔ **Neither alone satisfies AC6:** ⚠️ **(iii) guards the SHAPE and runs in CI; (i) catches the
PROPERTY and only runs locally.** ✅ **That split is the point of taking both.**

---

## ⚠️ What this Sprint must NOT do

⛔ **Re-litigate the hosting.** ✅ **Ruled in Q1; [I-0247] closes with it.**
⛔ **Change what Save and Cancel DO.** ✅ **[I-0245]/[I-0246] settled that, user-Verified.**
⛔ **Touch the Inspector column.** ✅ **[SP-136] closed it 2026-09-23.**
⛔ **Re-enable interactive dismissal to "simplify" Esc.** ⚠️ **That reintroduces silent data loss on
click-outside** ([I-0245]).

---

## ✅ On close, this Epic ends

⚠️ **[SP-137] is [EP-040]'s LAST planned Sprint.** ✅ **On its close, EP-040's remaining ACs should be
swept:** ⚠️ **AC8 (this Sprint) · ⛔ AC6 met by [SP-136] but its Epic box may be untidied · ⚠️ AC11
(carried from [EP-039]: [I-0206] and [I-0213]) is NOT this Sprint's and must be ruled at the Epic
close — ✅ addressed, or ACCEPTED AS LIMITATIONS with a measurement.**
⚠️ **An Audit Check is owed before the Epic closes** (⛔ **not an Audit — the lightweight mechanical
sweep**).

---

## ✅ Implementation record (2026-09-23) — ⚠️ **NOT VERIFIED**

### ✅ T-0547 — real sheet chrome

⚠️ **`ObjectDetailSheet.swift`:** ⛔ **the hand-built `toolbar` `HStack` is GONE**, ✅ **replaced by a
tombstone naming both halves of what succeeded it.**
✅ **`detailToolbar`, in the user's layout:** ⚠️ **close `.cancellationAction` (LEADING) · Undo/Save
Changes `.principal` (CENTRED) · Back `.navigation` (TRAILING).** ⚠️ **The close item carries
`.keyboardShortcut(.cancelAction)` — the AC2 trap, moved WITH the action.**
✅ **`.principal` VERIFIED against Apple's docs:** ⚠️ **"in macOS … the system places the principal
item in the center of the toolbar", and in iOS/iPadOS the center of the navigation bar** — ✅ **so the
layout carries to the platforms the forward ruling was made for.**
⛔ **`confirmationFooter(for:)` was added and REMOVED the same day** (tombstoned) — ✅ **the user ruled
the toolbar, not the content foot.**

✅ **BACK HIDES RATHER THAN GREYS** (user: *"with no second object navigated to, no buttons should
display"*) — ⚠️ **`if history.canGoBack`, not `.disabled`.** ✅ **Enable/disable now applies ONLY to
Undo/Save Changes, as ruled.**

⚠️ **A `NavigationStack` WAS REQUIRED and is load-bearing:** ⛔ **a `.toolbar` has nowhere to render
without a navigation container, and the sheet's content was a bare `VStack`.** ✅ **That absence is
part of WHY the chrome was hand-built in the first place.**

### ⛔ `ObjectDetailHistory` — forward REMOVED, cursor → stack

⛔ **DELETED: `canGoForward` · `forwardTarget` · `goForward()`.** ✅ **`goBack()` now POPS
(`entries.removeLast()`) rather than moving an index over a retained array** — ⚠️ **the popped entry is
DISCARDED, because nothing needs it any more.** ✅ **`visit()` is a plain append; the truncation branch
is gone.**

⚠️ **TWO TESTS WERE DELETED, NOT ADAPTED** — ⛔ **`backAndForward` and `visitTruncatesForward` asserted
a requirement that no longer exists.** ✅ **A test kept alive past its requirement reads as coverage
while proving nothing** (`feedback_fix_red_tests_dont_label_them`). ✅ **Replaced by `pushAndPop`,
`popDiscards` (asserts the entry is gone, not parked) and `popThenPush` (the old browser-rule test,
restated for a stack: A → B → back → C must leave the trail `[A, C]`).**
✅ **`xcodebuild test` is now 133/133** — ⚠️ **one MORE than before, not one fewer.**

### ✅ T-0548 — the guard, BOTH halves (Q4)

✅ **`scripts/check-layout-convergence.sh`** (static) — ⚠️ **wired into `scrivi-apple-ci.yml` THE SAME
DAY**, ⛔ **explicitly because `check-textkit2.sh` sat unrun for two Sprints.**
✅ **`Scrivi/App/LayoutConvergenceGuard.swift`** (debug assertion) — ⚠️ **counts Update-Constraints
passes per window per run-loop turn; traps past 64.**

⚠️ **AC6 — BOTH HALVES PROVEN BY INJECTION, not assumed:**
✅ **Static guard: RED.** ⚠️ **Restoring D1-E's `HStack` sibling (`.frame(minWidth: 420, idealWidth:
520)`) failed it, naming the line.** ✅ **Green again on removal.**
✅ **Assertion: TRAPPED.** ⚠️ **Budget temporarily set to 2; a normal launch trapped at 3 passes —
exit 133 (SIGTRAP), naming the window.** ✅ **Restored to 64.**

⚠️ **⛔ THE ASSERTION'S FIRST VERSION COULD NEVER HAVE FIRED, and only a probe found it.**
⛔ **It tested `window.contentView === self`.** ⚠️ **A probe proved the swizzle fired three times per
turn with `isContent` FALSE every time:** ✅ **AppKit drives the subtree pass from `NSThemeFrame`, the
content view's SUPERVIEW, so the test that identifies the root is `superview == nil`.**
⚠️ **It built clean, installed clean, logged nothing, and would have shipped as a guard that reports
success forever** (`feedback_prove_code_is_reached`).

⚠️ **⛔ A SECOND TRAP, worth recording: TWO `DerivedData` DIRECTORIES EXIST for this project.**
⛔ **The first probe run launched a binary that was an HOUR STALE** — ✅ **`xcodebuild` writes to
`Scrivi-hbxzf…`, and `find | head -1` picked `Scrivi-ftnof…`.**
⚠️ **The symptom was identical to "the code is not reached"** (`feedback_confirm_the_build_under_test`).

✅ **MEASURED: a healthy launch — Welcome window plus a real 1,174-scene project with Inspector and
Timeline shown — peaked at 8 passes per turn.** ✅ **The budget of 64 is 8× that headroom.**

### ✅ THE MODEL CHANGED: the STACK owns navigation (user rulings, 2026-09-23)

⚠️ **[T-0547] WAS REWRITTEN FOUR TIMES.** ✅ **The user's ruling ended it:** ⚠️ ***"Yes! The nav stack
owns the toolbar … When you push a View onto the stack, it automatically displays that, puts the title
up, and manages the back and confirm buttons. And yes, I want the whole model to change here."***

⛔ **THE FOUR ATTEMPTS, and why each was wrong:**
⛔ **(1) `.toolbar` on a bare sheet** — ⚠️ **the SHEET WINDOW has no `NSToolbar`** (`w.toolbar == nil`,
measured), ✅ **so every item was dropped SILENTLY** → **[I-0248]**, including `.cancelAction`.
⛔ **(2) A hand-built `VStack` bar** — ⚠️ **it rendered, but it hand-built chrome the platform
provides,** ⛔ **in the Epic that exists to STOP exactly that.** ✅ **The user caught it.**
⛔ **(3) A `NavigationStack` whose bar [T-0547] still populated entirely** — ⚠️ **the stack was
present and was never allowed to DO anything.** ⛔ **Back, title and transitions were all still ours.**
✅ **(4) THE STACK OWNS IT.** ⚠️ **`NavigationStack(path:)` + `navigationDestination` — it draws the
bar, the title and the back chevron; the sheet contributes Save · Undo · Close only.**

✅ **PLACEMENT IS THE PLATFORM'S** (user: *"put them where they are supposed to go"*) — ⚠️ **and the
macOS semantics are the REVERSE of the iOS ones I first assumed:** ✅ **`.cancellationAction` is
trailing (before confirmation), `.confirmationAction` trailing-most, `.navigation` leading.**
⚠️ **CLOSE IS `.confirmationAction`** (user ruling) — ✅ **because it is what DISMISSES.**
⛔ **That reading — placement describes what a button does TO THE MODAL, not what its label says — is
what put the controls on the wrong sides.**

### ✅ THE TWO-WAY PROMPT — ⚠️ **the user's ruling dissolved an impossible problem**

⛔ **[T-0547] spent three attempts trying to ARREST a pop so the prompt could offer Cancel.**
✅ **NO SUCH HOOK EXISTS** — ⚠️ **verified against the macOS 27 SDK's own
`SwiftUI.swiftinterface`, not doc pages: NO navigation or dismissal function takes a closure, and
`navigationTransition` takes a STYLE, not an action.**
✅ **THE USER'S RULING:** ⚠️ ***"This confirmation dialog shouldn't attempt to 'Cancel' the navigation
… rather it should require either a save or a revert."***
✅ **A settled pop needs no interception** — ⚠️ **only a decision about the orphaned drafts.**
✅ **CLOSE KEEPS ITS THREE-WAY prompt** (Save · Discard · Cancel) — ⚠️ **Cancel is meaningful there;
⛔ after a pop it would offer to undo something already done.**

✅ **MEASURED BEFORE BUILDING (2026-09-23):** ⚠️ **`onChange(of: path.count)` fires BEFORE the popped
view's `onDisappear`**, ✅ **so the prompt is raised while the drafts are still in hand** — and
⛔ **re-pushing the same value yields FRESH `@State`**, ⚠️ **so pushed-view state is DESTROYED on pop
and the drafts MUST live on the sheet, above the stack.**

⚠️ **AN OFF-BY-ONE THAT ONLY MEASUREMENT CAUGHT:** ⛔ **the first run of the prompt named "Myton" when
the writer had been editing "Brother Colm".** ✅ **The stack's ROOT is at `path.count == 0` but is
`trail[0]`, so the trail is always ONE LONGER than the path** — ⚠️ **indexing by the new count names
the ARRIVAL, not the departure.** ✅ **Pinned by a test that was PROVEN RED against the defect.**

---

### ⛔ AC8 FAILED ON FIRST ATTEMPT — ✅ **[I-0248]**, and what it costs to record honestly

⚠️ **THE USER'S LIVE PASS, 2026-09-23:** ⛔ ***"The save/cancel buttons are gone entirely. The close
button is still at the bottom. no navigation buttons appear when Colm is loaded. Esc cancels changes
silently."***

⛔ **FOUR SYMPTOMS, ONE CAUSE: a `.toolbar` inside a macOS `.sheet` RENDERS NOTHING.** ⚠️ **The sheet
is not a window with an `NSToolbar`, so SwiftUI drops the items at runtime with no diagnostic.**
⚠️ **EVERY SIGNAL WAS GREEN:** ✅ **BUILD SUCCEEDED · 133/133 Swift · 626/626 ctest · all three static
guards.** ⛔ **None of them can see whether a view renders.**

⚠️ **⛔ THE SEVERITY IS THE ESC HALF, AND IT IS THE TRAP AC2 EXISTS TO PREVENT.**
✅ **AC2 says: *"if the close control moves to a toolbar, `.keyboardShortcut(.cancelAction)` MUST move
with it."*** ⚠️ **The implementation honoured that LITERALLY — the shortcut did move with the button.**
⛔ **Both moved somewhere that does not render**, ✅ **so Esc fell through to SwiftUI's own dismissal,
which BYPASSES `requestClose()` — and the writer's edits went silently.**
⚠️ **AC2 was marked met on a reading of the code. ⛔ It was not met.**

⚠️ **⛔ THE LESSON WAS ALREADY IN HAND THIS SESSION.** ✅ **`feedback_prove_code_is_reached` was applied
to [T-0548]'s debug assertion — a probe caught a hook that could NEVER have fired.** ⛔ **The same
check was NOT applied to [T-0547]'s toolbar**, ⚠️ **and a 90-second empirical test (which is what
eventually diagnosed it) would have caught this before the user ever opened the sheet.**

✅ **THE FIX IS MEASURED, NOT REASONED:** ⚠️ **a minimal SwiftUI app presenting the old shape logs
`toolbar=nil` on the `SheetPresentationWindow`; the new shape logs THREE real `SwiftUIAppKitButton`s
in the sheet's view tree.**

✅ **THE BAR IS NOW CONTENT** — ⚠️ **a `ZStack` at the top of the sheet, keeping the user's layout
exactly: close LEADING · Undo/Save Changes CENTRED · Back TRAILING (hidden when there is nowhere to
go).** ⚠️ **THREE FIXED ZONES, not an `HStack` of six** — ⛔ **a plain row would shift Save sideways
the moment Back appeared.**

---

### ⚠️ What remains: AC8, a SECOND live pass — ⛔ **the user's**

⚠️ **Open a Detail Sheet · back/forward · Esc WITH unsaved edits (prompt) · Esc with none (closes) ·
Save (banner, sheet stays) · Cancel (reverts, banner) · type again (banner clears) · navigate to
another object (banner clears).**
⚠️ **AC2 is the one to press on** — ✅ **the close control moved to the toolbar, so Esc is the check
that matters most.**
