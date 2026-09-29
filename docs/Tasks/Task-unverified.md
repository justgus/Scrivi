# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0555 — `[Apple]` ✅ **Engine stub parity ([I-0253])** — **Implemented 2026-09-28, NOT VERIFIED**

⚠️ **NO SPRINT** — ✅ **raised by the user from a symptom they had patched themselves.**

✅ **THE FIX (two parts, and the second matters more):**
1. ✅ **Four methods added to the visionOS stub** in `Scrivi/Engine/ScriviEngine.swift`:
   `closeProject` (a no-op — it is non-throwing and runs on teardown), `openSceneForBulkLoad`,
   `mergeScene`, `mergeChapter`.
2. ✅ **`scripts/check-engine-stub-parity.sh`** — ⚠️ **the guard, wired into `scrivi-apple-ci.yml`**
   (step + both path filters). ⛔ **Because the stub's own comment already predicted this recurrence
   and a comment cannot fail a build.**

⚠️ **THE GUARD WAS PROVEN BY BREAKING IT:** ✅ **`mergeScene` was removed from the stub, the guard went
RED naming it, and green on restore.**

⛔ **I GOT THE GUARD WRONG ONCE, AND IT BROKE THE BUILD — recorded because it is the instructive part.**
⚠️ **The first pattern anchored on whitespace-then-`public func`, so it MISSED every
`@discardableResult public func` in the stub and reported FOUR methods as absent that were ALREADY
THERE** (`:1536-1539`). ⛔ **Acting on that phantom added duplicates and broke the visionOS build — the
exact build the guard exists to protect.** ✅ **Fixed to skip leading attributes; the reverted additions
are gone.** ⚠️ **A check that reports a phantom is worse than no check: it invites a "fix" that breaks
something real.**

✅ **VERIFIED BY BUILDING: `ScriviApp` ✅ · `ScriviApp-iOS` ✅ · `ScriviApp-visionOS` ✅ — all BUILD
SUCCEEDED.** ✅ **All four guards green.**
⛔ **NOT VERIFIED: the app was not RUN on visionOS** — ⚠️ **it cannot usefully be: the stub throws by
design because ScriviCore is not linked for visionOS ([I-0053]).** ✅ **This Task restores COMPILATION,
which is what it claims.**

---

## 🟠 T-0554 — `[Apple]` ✅ **Divider visibility ([I-0252])** — ✅ **FIXED 2026-09-29 (third attempt), NOT VERIFIED**

⛔ **THE CAUSE WAS FOUND BY MEASUREMENT, AND IT WAS NEITHER OF THE FIRST TWO DIAGNOSES.**
✅ **MEASURED by rendering a REAL `NSTextView` to a bitmap and sampling it** — ⚠️ **not a screenshot,
⛔ not documentation:**

| Probe | Result |
| ----- | ------ |
| engine | ✅ **TextKit 2** |
| `attachmentBounds` calls | ✅ **1 — ⚠️ the 24 pt gap IS reserved** |
| ⛔ **`image(for:)` calls** | ⛔ **ZERO** |
| ⛔ **an OPAQUE MAGENTA bar on the canvas** | ⛔ **0 pixels** |

⛔ **THE LINE WAS NEVER BEING DRAWN AT ALL** — ⚠️ **TK2 reserved its height and drew nothing, ✅ which
presents as a GAP between scenes, not a faint line.** ⛔ **So no colour could ever have fixed it.**

### ✅ THE FIX — one line

```swift
attachment.image = NSImage(size: NSSize(width: 1, height: 1))
```

⚠️ **A placeholder `image` PROPERTY makes TK2 take the image path; ✅ the `image(for:)` override then
supplies the real, correctly-width-ed art per layout pass** (⚠️ measured: `image(for:) calls = 2`).
⚠️ **1×1 is deliberate — ⛔ the override always replaces it, so the placeholder's size is never used
and must not be mistaken for the divider's geometry (`attachmentBounds` owns that).**

### ✅ PROVEN WITH THE APP'S REAL DRAWING CODE, ⛔ not the probe

⚠️ **Sampling a column PAST the text so glyphs cannot be counted:**

| Appearance | ⛔ without the fix | ✅ with the fix |
| ---------- | ------------------ | -------------- |
| **DARK** | ⛔ **0 drawn rows** | ✅ **2** (⚠️ 1 pt stroke + one antialiased row) |
| **LIGHT** | ⛔ **0 drawn rows** | ✅ **2** |

✅ **THE TWO EARLIER FIXES WERE KEPT AND ARE NOT WASTED** — ⚠️ **`secondaryLabelColor` (`5.89 : 1`
Dark) and the half-pixel alignment were correctly measured; ⛔ they were invisible behind a line that
never drew.** ✅ **Now that it draws, they are what make it READABLE.**

### ⚠️ Why this took three attempts — ✅ the lesson

⛔ **1:** [T-0526] dropping [I-0112]'s appearance guard. ✅ **Disproven by measurement.**
⛔ **2:** `separatorColor` at `1.34 : 1`. ✅ **Correct arithmetic, ⛔ wrong layer.**
✅ **3:** *"does it draw AT ALL?"* — ⚠️ **the question [EP-045]'s design doc said to ask FIRST.**
⚠️ **`feedback_prove_code_is_reached`: *"it didn't change anything" meant it wasn't running.*** ⛔ **The
same class, twice, on one Issue.**

✅ **VERIFIED:** ⚠️ **`xcodebuild -scheme ScriviApp` → BUILD SUCCEEDED** · ✅ **pixel measurement, both
appearances.** ⛔ **`xcodebuild test` NOT RUN** — ⚠️ **[I-0150]: the runner LAUNCHES the app and once
rewrote a real project.** ✅ **View-layer drawing, no ScriviCore involvement; ⚠️ the pixel measurement
is the stronger evidence anyway.**
⛔ **NOT user-Verified — ✅ the user is in Dark Mode and can confirm by eye.**
⚠️ **STILL SUPERSEDED BY the ruled CONFIGURABLE GLYPH** — ✅ **this restores VISIBILITY only.**
✅ **[EP-045] AC2's diagnostic obligation is DISCHARGED.**

---


## ✅ [SP-145] — T-0551 · T-0552 · T-0553 — ✅ **VERIFIED 2026-09-29, ARCHIVED**

✅ **All three USER-VERIFIED by live pass on the rig and ARCHIVED** →
[`Verified/Task-verified-0551-0553.md`](Verified/Task-verified-0551-0553.md) **in the same step
[SP-145] closed** (`feedback_archive_on_close`, `feedback_task_layer_discipline`).
✅ **[SP-145] CLOSED 2026-09-29 (user-approved)** →
[`../Sprints/Closed/Sprint-SP-145.md`](../Sprints/Closed/Sprint-SP-145.md).
✅ **[EP-043] S1 of 4 COMPLETE.**

⚠️ **ONE Task still awaits verification: ✅ [T-0555]** (`[Apple]` engine stub parity) — ⚠️ **above.**
✅ **[T-0554] WAS FIXED 2026-09-29 on the third attempt** (⚠️ **above**) — ⛔ **the cause was that the
divider was never DRAWN, not that it was the wrong colour.**

---


**Previously: none.** ✅ **T-0536 was user-Verified 2026-09-18** and archived to
[`Verified/Task-verified-0536.md`](Verified/Task-verified-0536.md) **in the same step**
(`feedback_archive_on_close`, `feedback_task_layer_discipline`).

✅ **T-0508 was user-Verified 2026-09-18** and archived to
[`Verified/Task-verified-0508.md`](Verified/Task-verified-0508.md) **in the same step**
(`feedback_archive_on_close`, `feedback_task_layer_discipline`).

⚠️ **SP-122's T-0466–T-0471 were ✅ Verified 2026-08-25** and archived to
[`Verified/Task-verified-0466-0471.md`](Verified/Task-verified-0466-0471.md) in the same step SP-122 closed.

⚠️ **SP-121's T-0460–T-0465 were ✅ Verified 2026-08-25** and archived to
[`Verified/Task-verified-0460-0465.md`](Verified/Task-verified-0460-0465.md) in the same step SP-121 closed.

> ⚠️ **T-0365 must not be Verified on the card alone.** It renders correctly and shows *"No sources cited
> by this scene's objects"* — which is indistinguishable from working, because there is no way to create
> a source to test it with. Its write half was ✅ **PAID by SP-120** under [EP-034](../Epics/Closed/Epic-EP-034.md), ✅ **now CLOSED 2026-08-25**, and §3.1.1's
> object-card entry point to `CitationPopover` is built but unwired.

---

*Last Updated: 2026-09-18 (**no Tasks awaiting verification** — ✅ **T-0508 Verified and archived the
same day**, together with the two Issues its drive-pull test produced, [I-0221] and [I-0222].
✅ **SP-130's only Task is now complete, so the Sprint is closable on the user's approval.**
Prior note follows.)*

*Last Updated: 2026-08-25, fourth pass (**no Tasks awaiting verification** — SP-122's six ✅ Verified and
archived the same day. Prior note follows.)*

*Last Updated: 2026-08-25, third pass (**six Tasks awaiting verification** — SP-122's **T-0466–T-0471**;
✅ **AC12 MET**. Prior note follows.)*

*Last Updated: 2026-08-25, second pass (**no Tasks awaiting verification** — SP-121's six were
✅ Verified and archived the same day. Prior note follows.)*

*Last Updated: 2026-08-25 (**six Tasks awaiting verification** — SP-121's **T-0460–T-0465**, moved here
from `Task-active.md` per `Task-Guidelines.md` §"When Marking Task as Implemented - Not Verified".
Prior note follows.)*

*Last Updated: 2026-08-20 (**no Tasks awaiting verification** — SP-115's seven were Verified and archived
the same day. Corrected a stale link: **EP-034 is now 🟡 Active**, not in the backlog. Prior note follows.)*

*Last Updated: 2026-08-19 (audit remediation — rulings R-16, R-17).*

---

## ✅ Cleared 2026-09-15 — SP-132's three, all user-ruled

✅ **[T-0521] Remove `rebuildStorage`'s O(N²) chapter lookup** → ✅ **VERIFIED.** ⚠️ **The `if !t.isEmpty { return t }` guard means the O(N) map rebuild never runs for a titled chapter.**
✅ **[T-0520] Make the `updateNSView` guard CHEAP** → ✅ **LANDED** as two `Hasher` digests (⚠️ a variation on the specified revision counter, not a gap).
✅ **[T-0519] Make the navigator cost O(changed), not O(N)** → ⛔ **CLOSED AS SUPERSEDED.** ⚠️ **Reverted (the build launched with no window); ✅ its target was then met by [EP-039] — navigator click `35–45 s` → `~0.3 s`.**

⛔ **NO mitigation Sprint** — ✅ **nothing from SP-132 is genuinely unfinished.** ✅ **Full record: [`Closed/Sprint-SP-132.md`](../Sprints/Closed/Sprint-SP-132.md).**
