# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

| ID | Title | Sprint | Epic | Implemented |
| -- | ----- | ------ | ---- | ----------- |
| **T-0508** | ⚠️ **Rule the 5 `fileExists` asset sites (Class C of [I-0197])** — ✅ **ruling written; ⛔ one site was a REAL defect and is fixed** | [SP-130] | [EP-040] | ✅ **2026-09-16** |

### ⚠️ What a verification pass should actually look at

⛔ **The build being green proves almost nothing here** — ✅ **four of the five sites did not change,
and the fifth changes only WHEN work happens, not WHAT is drawn.**

✅ **The ruling:** [`Scrivi_Asset_Presence_Check_Ruling_v0_1.md`](../Scrivi_Asset_Presence_Check_Ruling_v0_1.md).
✅ **The one code change:** `Scrivi/Views/Detail/ExistingAssetPicker.swift` — ⚠️ **`thumbnail(_:)`
became `AssetThumbnail`, loading off the main actor.**

⚠️ **THE USER-VISIBLE TEST, and it needs the REAL RIG** (`project_test_rig_tintagael_eskandar`):
✅ **open an object in the Detail Sheet, press *Choose Existing…*, and watch the popover.**
- ✅ **Expected: it opens IMMEDIATELY, rows drawn with the `photo` icon, pictures filling in after.**
- ⛔ **Before the fix: the popover did not appear until every visible thumbnail had been stat'd AND
  decoded** — ⚠️ **and on a sleeping or disconnected USB volume that is [I-0193]'s freeze, once per row.**
- ⚠️ **The adversarial case is the one worth running: pull the drive, THEN open the picker.**
  ✅ **The window must stay live.**

⚠️ **`xcodebuild` green on all three schemes (macOS / iOS / visionOS) and 127 interop tests passing
are recorded, ⛔ but NEITHER exercises this path** — ✅ **there is no test that opens the picker
against an unreachable volume, and none is claimed.**

⚠️ **SP-122's T-0466–T-0471 were ✅ Verified 2026-08-25** and archived to
[`Verified/Task-verified-0466-0471.md`](Verified/Task-verified-0466-0471.md) in the same step SP-122 closed.

⚠️ **SP-121's T-0460–T-0465 were ✅ Verified 2026-08-25** and archived to
[`Verified/Task-verified-0460-0465.md`](Verified/Task-verified-0460-0465.md) in the same step SP-121 closed.

> ⚠️ **T-0365 must not be Verified on the card alone.** It renders correctly and shows *"No sources cited
> by this scene's objects"* — which is indistinguishable from working, because there is no way to create
> a source to test it with. Its write half was ✅ **PAID by SP-120** under [EP-034](../Epics/Closed/Epic-EP-034.md), ✅ **now CLOSED 2026-08-25**, and §3.1.1's
> object-card entry point to `CitationPopover` is built but unwired.

---

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
