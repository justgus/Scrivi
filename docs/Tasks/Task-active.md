# Active Tasks

Tasks currently being worked by an **active Sprint**. Each row names the Sprint it belongs to; the
Sprint's own documentation lives in [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md) and is
**not** duplicated here.

A Task leaves this file when it is Verified (→ `Verified/Task-verified-XXXX.md`) or when its Sprint
closes. Tasks that are implemented but awaiting user verification move to
[`Task-unverified.md`](Task-unverified.md).

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ **Active Tasks live here, never in `Task-backlog.md`.** A Sprint may be **deactivated without
deactivating its Tasks** — the Sprint's status changes on its own layer while its Tasks stay in this
file. The backlog is for unstarted, unassigned work only.

---

## Currently: **no active Tasks** — SP-102's four Tasks are all Verified

✅ **T-0389 + T-0415** (AC23/AC24) Verified 2026-08-17 → [`Task-verified-0389-0415.md`](Verified/Task-verified-0389-0415.md)
✅ **T-0417** (Scene/Chapter menu items) Verified 2026-08-18 → [`Task-verified-0417.md`](Verified/Task-verified-0417.md)
✅ **T-0365** (`sources` card) Verified 2026-08-18 **as a PARTIAL delivery** → [`Task-verified-0365.md`](Verified/Task-verified-0365.md)
⚠️ **The `sources` write half is owed to [EP-034](../Epics/Epic-backlog.md)** — nothing in the app
creates a `source`, so the card can only render its empty state.

| ID | Title | Sprint | Priority | Status |
| -- | ----- | ------ | -------- | ------ |

✅ **EP-031 AC23 + AC24 are DELIVERED and Verified (2026-08-17)** — the last two clauses of AC9.
**T-0389 and T-0415** were archived 2026-08-18 → [`Task-verified-0389-0415.md`](Verified/Task-verified-0389-0415.md).

> ⚠️ **Both were carried here as unverified for a day after they were verified.** SP-102's exit-criteria
> table was written at planning on 2026-08-17, **before** that day's live USB runs, and never updated;
> and `Issue-verified-0121-0130.md` listed only **3 of its 10** Issues, hiding I-0123–I-0129 — *the AC23
> evidence*. The user challenged it directly (*"I thought that was already verified. It shows."*) and was
> right. **The live eject/reattach cycle ran twice and produced eight Verified Issues**; I-0129 is the
> no-intervention clause itself (refresh keyed to app focus rather than the mount event).

> ⚠️ **T-0365 shipped its READ half and is not reachable (2026-08-18).** `SourcesCard.swift` is built,
> registered in the Writing stack, and covered by 4 tests — and it **can only ever show an empty list**,
> because **nothing in the app can create a `source` object or attach a `cites` edge.**
>
> **Found by the user on first use:** *"I can show the sources card, but I can't create any sources. None
> of the card interfaces allow it."* The cause is broader than sources: **object cards edit `displayName`
> only** (one `TextField` at `ObjectCard.swift:647`), while `subtitle`, `notes` and `image` have shipped
> in ScriviCore since **SP-095/T-0371** and are read by nothing. ⚠️ **Capability shipped, surface never
> built** — the same shape as the SP-099 R4 audit, I-0117, and `listPendingEdges`-with-zero-call-sites.
>
> ⚠️ **Source creation was deliberately NOT added to the `sources` card** (user ruling 2026-08-18):
> *"it would muddy the fact that the source must be associated with an object in the world."* A citation
> documents an **object**; creating one from a scene-scoped aggregate card inverts Doc 1 §3.4.
>
> **The write half is owed to [EP-034 `[Cross]` Object Detail & Media](../Epics/Epic-backlog.md)**, opened
> the same day. **EP-032 was checked and does not cover it** — that Epic is manuscript reference syntax.
> The card **stays registered** (user ruling) so it goes live the moment creation exists; until then it
> correctly reads *"No sources cited by this scene's objects."*
>
> **T-0365 must not be marked Verified on the card alone.** Its own design (§3.1.1) also owes the
> object-card entry point to the shared `CitationPopover`, which is built but unwired.

> **T-0417 is new (adopted 2026-08-18), and it is deliberately incomplete.** The user asked whether
> Ctrl-Up/Ctrl-Down had been implemented for scene navigation, then established by direct trial that
> **there is no free key combination for Scene/Chapter Start-End on macOS**:
>
> | Combination | Already owned by | Shift-variant |
> | ----------- | ---------------- | ------------- |
> | ⌘↑ / ⌘↓ | Document start / end (`NSTextView`) | extends selection |
> | ⌥↑ / ⌥↓ | Paragraph start / end (`NSTextView`) | extends selection |
> | ⌃↑ / ⌃↓ | **macOS Mission Control** — not ours to take | — |
>
> Every candidate costs an existing editing behaviour, so **the functions ship as menu items with no
> key equivalent** (Scene menu → *Go to Scene Start / End*; Chapter menu → *Go to Chapter Start /
> End*). ⚠️ **The binding remains an open question** — the menu items exist so the functions can be
> exercised and judged while it is decided. Do not treat this Task as finished when the menu items
> verify; it is the *behaviour* that is being tested, not the surface.
>
> Implementation: `Coordinator.moveToSceneBoundary(_:in:)` and `moveToChapterBoundary(_:in:)`
> (`ManuscriptTextView.swift`), reached through four new `ProjectSession` actions. Chapter bounds are
> derived by widening from the caret's scene across the contiguous run of segments sharing its
> `chapterID`. Unlike `navigateToScene` these **do** move the caret — they are editing moves, not
> survey gestures.

Full plan, rulings **R1/R2**, and the audit table: [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md).

---

## Closed: SP-106 `[Cross]` Test integrity & CI trust (2026-08-17)

T-0412, T-0413 and T-0414 were all ✅ **Verified 2026-08-17** and archived to
[`Verified/Task-verified-0412-0414.md`](Verified/Task-verified-0412-0414.md). Issues **I-0121** and **I-0122**
were Verified in the same step → [`../Issues/Verified/Issue-verified-0121-0130.md`](../Issues/Verified/Issue-verified-0121-0130.md).
Sprint record: [`../Sprints/Closed/Sprint-SP-106.md`](../Sprints/Closed/Sprint-SP-106.md).

**Standing practice adopted from its exit criterion 4:** ⚠️ **`ctest` figures must name their architecture.**
"516/516" without a platform is the habit that let a divide-by-zero run red on x86-64 CI for 17 days.

---

## Cleanup note (2026-08-15)

This file previously carried **full sprint documentation for eight Sprints** — SP-090, SP-091, SP-092,
SP-093, SP-095, SP-096, SP-097 and SP-101 — including retrospectives, suite counts, design rulings and
planning narrative. That content belongs to the Sprint layer, not the Task layer.

**All eight Sprints were already closed and archived** to `../Sprints/Closed/`, and every Task and Issue
listed under them was already Verified. Nothing was active. The sections were removed rather than moved:
each Sprint's archive file is the authoritative record and already contains the same material.

| Sprint | Archive | Tasks (all Verified) |
| ------ | ------- | -------------------- |
| SP-090 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) | T-0359–T-0362 |
| SP-091 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) | T-0392, T-0393, T-0363, T-0364 |
| SP-092 | [`Sprint-SP-092.md`](../Sprints/Closed/Sprint-SP-092.md) | T-0394, T-0395, T-0366, T-0367, T-0368 |
| SP-093 | [`Sprint-SP-093.md`](../Sprints/Closed/Sprint-SP-093.md) | T-0396, T-0397, T-0398 (+ I-0104–I-0111) |
| SP-095 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) | T-0370, T-0371, T-0372, T-0401 |
| SP-096 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) | T-0402, T-0373, T-0374, T-0375, T-0376 |
| SP-097 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) | T-0403, T-0381–T-0385, T-0404 |
| SP-101 | [`Sprint-SP-101.md`](../Sprints/Closed/Sprint-SP-101.md) | T-0399 |

Two sections were **stale as well as misplaced**: SP-101 was shown 🟡 active and SP-092 🔵 planning, but
both closed 2026-08-11 (Human-approved). T-0399 is ✅ Verified; SP-092's T-0368 closed as delivered by
T-0399.

**Carried forward, not lost:** T-0365 (`sources` card) remains 🔵 Backlog, unblocked 2026-08-12, and is
scoped into SP-102.

⚠️ **Correction (2026-08-16):** this section previously said T-0217 "is still owed **before EP-019 closes**."
That was impossible on its face — **EP-019 closed 2026-08-11**, five days before. T-0217 was in fact
**completed in SP-094 and Verified 2026-08-11** (design §4.a/§4.a.1/§4.d/§12.2/§12.8/§14/§15,
package-structure §16a, repair-matrix §6.21); its record of truth is
[`Sprint-SP-094.md`](../Sprints/Closed/Sprint-SP-094.md). Nothing is owed. `Task-backlog.md` carried it as
🔵 Backlog under the superseded SP-057 and has been corrected to match.

---

*Last Updated: 2026-08-18 (**Documentation audit.** ⚠️ **`Task-backlog.md` was carrying this file's
three active Tasks** (T-0389, T-0365, T-0415) as backlog rows, alongside ~60 Verified Tasks and 7
unverified ones. The backlog was rebuilt to hold unstarted work only; the layer rule is now stated at
the top of both files: **a Sprint may be deactivated without deactivating its Tasks.** Separately,
**T-0218–T-0222** were listed in the backlog as "🟢 Active → `Task-active.md`" — they have been
✅ Verified and archived since **2026-07-13**, and were never in this file. Prior note follows.)*

*2026-08-17, later same day (**SP-102 planning completed.** ⚠️ **A code audit found most of
T-0389's staged scope already shipped in SP-099/SP-104** and struck it; **T-0415 added** for AC23 live
verification. Two rulings: **R1** the warning view gets its own toggle rather than being anchored to the
hidden-by-default Timeline; **R2** AC24 refined via URL volume keys, degrading to `unavailable` and never
guessing `missing` — **no ABI change needed**. Prior note follows.)*

*2026-08-17 (**SP-106 ✅ closed (user-approved) — T-0412/T-0413/T-0414 all Verified and
archived**, together with Issues I-0121 + I-0122. **SP-102 activated**: T-0389 + T-0365, delivering EP-031
AC23 + AC24. Prior note follows.)*

*2026-08-16 (SP-106 activated and implemented — T-0412/T-0413/T-0414 all 🟠 Implemented - Not
Verified. This file carries active Tasks again for the first time since 2026-08-15.)*
