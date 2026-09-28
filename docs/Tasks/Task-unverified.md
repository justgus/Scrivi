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

## 🟠 T-0554 — `[Apple]` ✅ **Divider visibility ([I-0252])** — **Implemented 2026-09-28, NOT VERIFIED**

⚠️ **NO SPRINT** — ✅ **a standalone fix taken while [EP-043]/[SP-145] awaits rig verification and the
manuscript-rendering trade study awaits rulings.** ⛔ **Deliberately NOT folded into the renderer work:
the writer is impeded in Dark Mode today.**

✅ **`Scrivi/Views/ManuscriptTextView.swift` — `DividerTextAttachment.image(for:)`.** ⚠️ **TWO measured
defects, ⛔ neither the one first theorised:**
1. ⚠️ **Colour:** `separatorColor` → `secondaryLabelColor`. ⛔ **`1.34 : 1` was a CHROME hairline used as
   a content mark.**
2. ⚠️ **Pixel straddling:** a 1 pt line on an integral `y` antialiased across TWO rows at half alpha
   (`0.275 / 0.275` vs `0.549`). ✅ **`rect.midY.rounded() + 0.5`.**

✅ **MEASURED RESULT: Dark `1.34 → 5.89 : 1` (4.4x); Light `1.25 → 3.95 : 1` (3.2x).**
✅ **`xcodebuild -scheme ScriviApp build` → BUILD SUCCEEDED.**
⛔ **NOT VERIFIED: no visual confirmation in the running app.** ⚠️ **Scrivi was RUNNING during this work,
so `xcodebuild test` was NOT run** (`feedback_check_scrivi_running_before_tests` / [I-0150]: the test
runner LAUNCHES the app and once rewrote a real project). ✅ **The user is in Dark Mode and can confirm
by eye.**

---

## 🟠 [SP-145] — T-0551 · T-0552 · T-0553 — ✅ **Implemented 2026-09-27, NOT VERIFIED**

✅ **Sprint record (the detail lives there, not here):** →
[`../Sprints/Sprint-SP-145.md`](../Sprints/Sprint-SP-145.md). ✅ **[EP-043] S1.**

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0551** | ✅ **Introduce `ProjectSession`** — the seven per-project state members out of `EditorShell`; ⚠️ **behaviour-preserving** | ✅ **Implemented - Not Verified** |
| **T-0552** | ✅ **Introduce `OpenProjectRegistry`** — `projectID` → live session ([EP-043] [R-Q2]) | ✅ **Implemented - Not Verified** |
| **T-0553** | ⚠️ **[I-0251]** — pane visibility becomes per-project state; inspector visibility persists THROUGH THE CORE ([R-Q4]) | ✅ **Implemented - Not Verified** |

✅ **RUN, not asserted:** ⚠️ **`ctest` 641/641 as NON-ROOT** · ✅ **24/24 Linux smokes** · ✅ **boundary
guard GREEN** · ✅ **Apple `BUILD SUCCEEDED`** · ⚠️ **AC2 proven mechanically: 81 insertions, ⛔ 0
deletions in `tests/`.**
⚠️ **THE GUARD WAS PROVEN BY BREAKING IT** — ✅ **the T-0553 fix was removed, the smoke went RED with the
two right failures, then green on restore.**
⛔ **WHAT BLOCKS VERIFICATION: no live pass on the rig.** ⚠️ **T-0553 is a *"does it survive a quit?"*
fix, and `feedback_live_pass_finds_what_suites_cannot` says plainly that a green suite cannot answer
that.** ✅ **[SP-148] owns the Epic's live pass.**

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
