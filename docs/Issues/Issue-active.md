# Active Issues

Issues awaiting **user verification**. An Issue leaves this file only when the user verifies it
(→ `Verified/Issue-verified-XXXX-YYYY.md`, batched in decades of ten) or approves its closure
(→ `Closed/`).

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0147** | `[ScriviCore]` ⚠️ **KNOWN LIMITATION (user-ruled 2026-08-21, ACCEPTED — not to be fixed in SP-116).** ⚠️ **For up to `kStaleSeconds` (60 s) after an interrupted world write, the world is ENTIRELY UNWRITABLE and its abandoned `.partial` is unreclaimable.** When a volume vanishes mid-import the writer dies holding the lock, leaving `.lock` on disk with a **fresh** heartbeat. Reattach the drive quickly — the natural thing to do — and the next write is refused `worldLocked`; ⚠️ **T-0433's sweep runs only AFTER a successful acquire**, so the orphan survives until the lock ages out. **Observed on the real rig 2026-08-21**: drive pulled mid-import, reattached within ~60 s, next import refused and a **2.9 GB** `.partial` remained. ✅ **Both halves verified**: staged fresh lock + orphan → `worldLocked`, orphan stays; waited past 60 s → **acquired and swept**. ⚠️ **This is arguably CORRECT, which is why it is accepted:** `kStaleSeconds` exists precisely because the core cannot distinguish *"writer died"* from *"writer is briefly stalled"*, and guessing wrong means two processes writing a shared world at once. It **self-heals** within a minute and loses no data. **The stronger evidence available — the package's own VOLUME was unmounted, which is far better proof of a dead holder than a quiet heartbeat — is not currently used.** ⚠️ **Deferred to the network-worlds design**, which must revisit *"exactly one winner"* regardless; ruling that inside an asset sprint is how a locking model gets set by accident (the lesson of I-0144). ⚠️ **The eventual UI must not present the 60 s wait as an error** — it is a retryable state. | Low | ⚠️ **Deferred — network-worlds design** | 🟡 **Accepted limitation (2026-08-21)** — ⚠️ **found by the LIVE RIG PASS**; ⚠️ **my own earlier staged-orphan test PASSED because it created the orphan WITHOUT a matching fresh lock** — not the state a real crash leaves |

## Currently: **one record — I-0147, an ACCEPTED limitation, not open work**

✅ **I-0148 was Verified 2026-08-21 (user-approved) and archived** →
[`Verified/Issue-verified-0141-0150.md`](Verified/Issue-verified-0141-0150.md).

✅ **SP-116 closed with ZERO open Issues.** Its six — **I-0140, I-0141, I-0143, I-0144, I-0145, I-0146** —
were ✅ **Verified 2026-08-21 (user-approved) and archived in the same step** →
[`Verified/Issue-verified-0141-0150.md`](Verified/Issue-verified-0141-0150.md).

**What the table cannot express:**

- ⚠️ **I-0147 is a KNOWN LIMITATION, not a defect awaiting a fix** (user ruling, option 1). For up to 60 s
  after an interrupted world write, the world is unwritable and its `.partial` unreclaimable, because the
  dead writer's lock is not yet stale and the sweep only runs after a successful acquire. It **self-heals**
  and loses no data. ⚠️ **A regression test ASSERTS this behaviour** — if someone later makes `acquire`
  break fresh locks, it fails and forces the locking-model conversation rather than letting it happen by
  accident (the lesson of I-0144).
- ⚠️ **The eventual UI must never present the 60 s wait as an error** — it is a retryable state.
- ⚠️ **I-0148 is the FIFTH defect in EP-034 found by use rather than by tests** (I-0137, I-0142, I-0146,
  I-0147, I-0148) — and the first the user reported **without recognising it as a defect**, folded into an
  otherwise positive report. ⚠️ **A satisfied user is not a green suite**: the observation mattered more
  than the verdict attached to it.
- ⚠️ **Three of SP-116's six were found by no suite at all**: I-0143 by reading the code D7 was about to
  modify, I-0144 by looking for a caller to mirror, and **I-0146 by physically pulling a USB drive**.

---

## ✅ SP-115 — all six Issues Verified 2026-08-20

| Issue | Sev | Task | Archive |
| ----- | --- | ---- | ------- |
| **I-0137** | **High** | T-0419 | [`Verified/Issue-verified-0131-0140.md`](Verified/Issue-verified-0131-0140.md) |
| I-0136 | Medium | T-0420 | same |
| I-0139 | Medium | T-0421 | same |
| I-0135 | Low | T-0422 | same |
| I-0138 | Low | T-0423 | same |
| **I-0142** | **High** | T-0425 | [`Verified/Issue-verified-0141-0150.md`](Verified/Issue-verified-0141-0150.md) |

⚠️ **I-0137 was verified on the REAL RIG** with the drive ejected — the check a passing suite genuinely
cannot substitute for.

⚠️ **I-0136 is Verified at the CORE ONLY.** Nothing in Scrivi surfaces `unsupportedWorldFormatVersion`, so
a writer opening a too-new world still sees *"unavailable"* with **no explanation**. The core refuses
correctly; **the writer-facing half does not exist** — `project_capability_without_surface` inside the very
sprint that fixed four other instances. **Owed a surface in a later sprint.**

⚠️ **I-0142 was found by the USER, not a suite** — and its unseen half (**renaming any world object
failed**) was worse than the reported symptom.

---

*Last Updated: 2026-08-21, twelfth pass (✅ **I-0148 VERIFIED (user-approved) and ARCHIVED in the same
step.** ⚠️ **It was found by the user's live click-through and reported as an OBSERVATION, not a
complaint** — the fifth defect in EP-034 found by use rather than by tests. **Open Issues: 0**; I-0147
remains an Accepted limitation. Next available Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, eleventh pass (⚠️ **I-0148 FILED AND RESOLVED — found by the user's LIVE
CLICK-THROUGH of SP-117**, and ⚠️ **reported as an observation, not a complaint**: `.disabled()` does not
make a `TextEditor` read-only, so Notes stayed editable beneath a "read only" banner. ✅ **Never a
write-safety bug** — Save is hidden when read-only — ⚠️ **but typing during an outage was silently
discarded on navigation**, since `load()` overwrites the draft. **User ruled: disable it**, for simplicity
and consistency over draft retention. Notes now renders as selectable text when read-only. Next available
Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, tenth pass (✅ **SP-116's SIX ISSUES VERIFIED (user-approved) and ARCHIVED in
the same step** → `Verified/Issue-verified-0141-0150.md` (`feedback_archive_on_close`). ⚠️ **I-0147 remains
here as an ACCEPTED limitation** — deferred to the network-worlds design, with a regression test asserting
it. **Open Issues: 0.** Next available Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, ninth pass (⚠️ **I-0147 FILED AND ACCEPTED as a known limitation** (user ruled
option 1): for up to 60 s after an interrupted world write the world is unwritable and its `.partial`
unreclaimable, because the dead writer's lock is not yet stale and **the sweep only runs after a successful
acquire**. ⚠️ **Found by the tidy end-to-end rig run** — drive pulled, reattached quickly, next write
refused `worldLocked`, **2.9 GB orphan retained**. ✅ **Both halves verified** (fresh lock → refused; past
60 s → acquired **and swept**). ⚠️ **My earlier staged-orphan test passed only because it omitted the
matching fresh lock** — a setup subtly easier than reality; **fourth defect this Epic found only by live
use**. **Deferred to the network-worlds design**, which must revisit "exactly one winner" anyway.
Open Issues: **0** (I-0147 is Accepted, not open). Next available Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, eighth pass (✅ **I-0146 ASSIGNED to SP-116 (T-0433) and RESOLVED** by user
ruling. `WorldLock::sweepAbandonedPartials()` reclaims abandoned `*.partial` files whenever the lock is
acquired. ⚠️ **Swept on EVERY successful acquire, not only after breaking a stale lock** — the rig showed
the lock file and the partial are orphaned TOGETHER, so the next writer acquires cleanly and never reaches
a break path; sweeping only on a break would have missed the exact case this Issue was filed for.
⚠️ **Verified on real hardware**: 459 MB orphan on the USB volume reclaimed by a normal import, 476 MiB →
12 MiB, real assets and `myton.json` untouched. Tests **551/551** (+4), ⚠️ **proven non-vacuous** —
disabling the sweep fails two. **Open Issues: 0.** Next available Issue: **I-0147**. Prior note follows.)*

*Last Updated: 2026-08-21, seventh pass (⚠️ **I-0146 FILED — found by the LIVE RIG PASS, not by a suite.**
Pulling a real USB drive mid-import left a **459 MB `.partial` orphan** inside the shared world: the
cleanup in `copyFileInBlocks` cannot run when the failure IS the volume vanishing. ⚠️ **`list_assets`
cannot see it, so nothing in Scrivi will ever reclaim it.** ✅ **The rest of the abort behaved correctly** —
heartbeat detected the loss, transfer aborted, no destination file, existing assets byte-identical, stale
lock breakable after 60 s. **Fix is the user's own stale-lock sweep**, which SP-116 did not implement.
Open Issues: **1** (I-0146). Next available Issue: **I-0147**. Prior note follows.)*

*Last Updated: 2026-08-21, sixth pass (**I-0144 🟢 Resolved - Not Verified** — every world-package write
path now takes the lock via `WorldWriteGuard`, ⚠️ **inert for project writes so there is no branch to
forget**. ⚠️ **One deliberate exception recorded**: `ObjectIndex::loadWorldIndex`'s rebuild stays unlocked
because `WorldLock` is NOT REENTRANT and `save`/`remove` reach it while holding the lock — a guard there
would fail against itself and skip the rebuild. It is idempotent; the real fix is a reentrant lock, which
belongs with the network-worlds design. **Open Issues: 0.** Next available Issue: **I-0146**. Prior note
follows.)*

*Last Updated: 2026-08-21, fifth pass (**I-0145 FILED — 🟢 Resolved - Not Verified.** ⚠️ **Pre-existing and
shipped**: `AssetStore::remove` deleted the sidecar first and discarded both results, so a half-failed
delete stranded **bytes with no sidecar — invisible to `list` and unfindable by any future `remove`**,
unreclaimable for the life of the package, with `deleted: true` returned regardless. ⚠️ **D6 raises its
severity**, since the junk now lands in a SHARED world. Found by **self-review**; ⚠️ **no test caught it**.
✅ **Fixed in T-0426** (binary deleted first, both failures reported). ⚠️ **A sibling defect was
deliberately NOT filed** — `ObjectKindScope`'s duplicate-key trap was written and fixed inside this sprint
and never shipped. Open Issues: **1** (I-0144). Next available Issue: **I-0146**. Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (✅ **I-0144 ASSIGNED to SP-116** by user ruling → **T-0431**;
⚠️ **it is a High-severity data-loss risk, not an asset defect** — every object write into a shared world
is unserialised. Open Issues: **1**, now assigned. Next available Issue: **I-0145**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**SP-116 IMPLEMENTED — I-0140, I-0141, I-0143 all 🟢 Resolved -
Not Verified.** ⚠️ **I-0140 and I-0143 were each proven non-vacuous by reverting the fix** and watching the
tests fail. ⚠️ **I-0144 FILED (High, unassigned)**: `WorldLock` has **no production caller** — world-package
object writes are unserialised and have been since they shipped, so two projects sharing a world can lose
each other's edits silently. Found while implementing T-0426, looking for a caller to mirror; **no test
would have caught it**, since a missing lock is invisible single-threaded. ⚠️ **Not fixed in SP-116** — it
touches every object write path, not assets. Open Issues: **1** (I-0144). Next available Issue: **I-0145**.
Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**SP-116 ACTIVATED** — all three open Issues are now assigned to
an **active** Sprint, not a planned one; Sprint fields marked 🟡. ⚠️ **None is Resolved** — activation is
not progress, and Claude may never mark an Issue Verified regardless
(`feedback_verification`). Next available Issue: **I-0144**. Prior note follows.)*

*Last Updated: 2026-08-21 (**I-0143 FILED at SP-116 planning** — ⚠️ `scrivi_list_assets` concatenates
its JSON with **no escaping** (`scrivi_c_api.cpp:1330-1341`), while every sibling envelope uses `JsonDoc`.
⚠️ **Found by reading the code D7 modifies, not by a test and not by the design doc** — and D7 is precisely
what makes it reachable, since **T-0427 puts a filesystem path into that array**. ✅ **User ruled: file it
AND fix it in SP-116** (T-0428), keeping T-0424's file-don't-fix-silently precedent while refusing to ship
a corruption path the same sprint could prevent. ⚠️ **The restating summary table below the main table was
REPLACED** with only what the table cannot express (P7). Open Issues 2 → **3**, all SP-116. Next available
Issue: **I-0144**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115's six Issues ✅ VERIFIED by the user and ARCHIVED in the same step** —
I-0135–I-0139 → `Verified/Issue-verified-0131-0140.md`, **I-0142 → a new decade file
`Issue-verified-0141-0150.md`.** Open Issues 8 → 2 (**I-0140, I-0141** — filed for SP-116, unfixed by
design). ⚠️ **I-0137 verified on the real rig, drive ejected.** ⚠️ **I-0136 verified at the CORE ONLY — its
writer-facing surface does not exist and is owed.** Suites: ctest **525/525** · interop **103/103** · app
**BUILD SUCCEEDED**. Next available Issue: **I-0143**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 implemented — all five Issues 🟢 Resolved - Not Verified**, and
⚠️ **I-0140 + I-0141 FILED by T-0424** (restated-kind-list class, occurrence eight → **SP-116**, cured by
D5). Suites: `ctest` **524/524** (was 520) · macOS interop **103/103 in 10 suites** (was 99) · app
**BUILD SUCCEEDED**. ⚠️ **I-0137 still needs the REAL-RIG check** — drive ejected — before it can be
Verified. Open Issues 5 → 7. Next available Issue: **I-0142**. Prior note follows.)*

*Last Updated: 2026-08-20 (**All five open Issues ASSIGNED to SP-115** 🟡 Active under **EP-034** — one
Task each, T-0419–T-0423. ✅ **Two carried rulings recorded**: **D9 = A** for I-0137
(`lastKnownPackagePath`, distinctly named; `packagePath` NOT widened) and **Q-b** for I-0139 (**patch the
control** — the Detail Sheet does **not** replace the inline editor, so it is a real fix). ⚠️ **I-0140 and
I-0141 to be FILED by T-0424.** Next available Issue: **I-0142** after that filing. Prior note follows.)*

*Last Updated: 2026-08-19 (**T-0390 + T-0418 filed five Issues — I-0135…I-0139.** The live pass on the
real USB rig **passed steps 3, 4 and 5**: ⚠️ **AC23's no-intervention clause HELD** — reattaching the drive
restored every card with no click, no menu, no relaunch. Step 1 confirmed **all ten world kinds
round-trip** (the four directories absent since before SP-104 were created on demand); ⚠️ **`source` could
not be created — no UI exists, the known EP-034 gap.** Step 2 was **blocked**: relating from an object card
opens an editor whose exit is labelled "Revert" (I-0139). Findings: **I-0137 (High)** — AC24's refinement
**cannot fire on real hardware**; **I-0138** — disabled-but-unexplained removal; **I-0139** — the editor
exit. Next available: **I-0140**. Prior note follows.)*

*2026-08-18, fourth pass (✅ **I-0132 VERIFIED (user-approved) and re-archived** — both
halves, on an extended live click-through: *"I clicked about a lot and saw no missed clicks or focus
changes."* It took **four** attempts; the first three misdiagnosed it as a first-responder race and
each made the failure rarer rather than fixing it. ⚠️ **The user stopped the fourth before it was
written** — I was about to add an `NSEvent` monitor, reaching further below SwiftUI to win a fight
created by reaching below it in the first place — and redirected to the actual question: *what is the
source of truth, and does it propagate through the View hierarchy?* **The real defect was a one-shot
`navigateToSceneID` trigger**, not responder arbitration: re-selecting the same scene wrote an
unchanged value and SwiftUI coalesced the update away. macOS now uses the selection-as-source-of-truth
shape iOS already had. A **user-prompted loop audit** then replaced a fragile value-equality guard
with explicit echo suppression, plus **two regression tests proven non-vacuous**. Interop **95/95
macOS arm64**. Active count: **2** (I-0133 Resolved-Not-Verified, I-0134 Open). Prior note follows.)*

*2026-08-18, third pass (⚠️ **I-0132 RETURNED FROM VERIFIED — I archived it on a claim
that was not true.** The user verified focus changing **on app launch**, and said so explicitly; I
recorded that as verifying **click-to-focus** as well. Clicking a scene still left focus in the
navigator. **Cause was a responder race, not a missing call:** `takeFocus` ran
`makeFirstResponder` synchronously from inside `onTapGesture`, and the `NSTableView` backing SwiftUI's
`List` reclaimed first responder while finishing its own mouse-down. Launch had no competing responder
change, which is exactly why the two cases diverged — **the evidence I verified against and the
failing case were different code paths.** Second fix defers the transfer one runloop pass. The
**reveal half stays verified** and remains archived. **Lesson recorded:** when a fix has two halves,
verify each half against its own trigger — a verification of one is not evidence for the other.
Active count: 2 → **3**. Prior note follows.)*

*2026-08-18, later same day (**I-0131 + I-0132 ✅ Verified (user-approved) and
archived** to the new `Verified/Issue-verified-0131-0140.md` decade file, and removed from this file
in the same step. **I-0133 ruled and resolved:** the user chose *delete Apple's dead state, leave
Linux alone* — the property, its `loadAll` parameter, the write, the clear and the `ProjectSession`
plumbing are gone, each site commented so the omission reads as deliberate; ⚠️ **the schema field
stays** because Linux consumes it. ⚠️ **Ruling I-0133 surfaced a finding the original report missed,
now filed as I-0134 (🔴 Open):** Linux applies the scroll fraction *after* `centerCursor()`,
deliberately overriding it — so **Apple and Linux now disagree about what "restore where I was"
means.** Deliberately **not** settled inside a dead-code cleanup: it changes shipped, VNC-verified
EP-022 behaviour and belongs to EP-026 parity. **BUILD SUCCEEDED**, interop **93/93 macOS arm64**.
⚠️ **Active count is now 2** — I-0133 (Resolved - Not Verified) and I-0134 (Open). The prior note's
"Active count: 10 → 13" was already stale before this pass: those Issues had been verified and
archived without this line being updated. Prior note follows.)*

*2026-08-18 (**I-0132 both halves now 🟠 Implemented - Not Verified.** ⚠️ **The
reveal-on-selection-change half was REMOVED, not tuned** — the user's re-test found it scrolled the
navigator "a little bit up or down" on **every** click, because `scrollTo` **re-anchors an
already-visible row** rather than no-opping as my comment had claimed. Reveal now fires **`onAppear`
only**, which is the one moment it is needed (restore sets the selection before the view exists).
The **focus half is implemented**: `navigate(to:)` calls `loader.takeFocus()`, so a click or Return
hands the keyboard to the manuscript and the caret is visible. ⚠️ **Accepted trade, user-ruled:**
this ends arrow-key list browsing after the first click — *"Arrow browsing isn't strictly necessary.
Mouse Wheel and Trackpad Scrolling are still available."* **Tab-as-focus-advance is no longer needed
for this Issue.** Also filed **T-0417** (Scene/Chapter boundary navigation) — adopted into SP-102,
shipping as menu items because ⚠️ **no free macOS key combination exists.** **BUILD SUCCEEDED**,
interop **93/93 macOS arm64**. Prior note follows.)*

*2026-08-17, later same day (**I-0114–I-0117 ✅ Verified (user-approved) and archived** to
`Verified/Issue-verified-0111-0120.md` in the same step — verified live during the SP-102 / T-0415
world-availability runs, which exercised those exact surfaces. ✅ **They are now usable as evidence for
SP-100's AC pass**, which the prior note said they were not. **Also filed and fixed the same day:
I-0123–I-0129**, all from the user's live SP-102 runs. Active count: 10 → **13**. Prior note follows.)*

*2026-08-17 (**SP-106 closed — I-0121 and I-0122 ✅ Verified and archived** to the new
`Verified/Issue-verified-0121-0130.md` decade file, and their full entries removed from `Issue-backlog.md` in
the same step. Neither was ever listed in this file — both were tracked in `Issue-backlog.md` and the SP-106
sprint record. **This file is unchanged otherwise: the same 10 `Resolved - Not Verified` Issues remain
active**, including I-0114–I-0117, which are **not** evidence for any EP-031 AC until verified. Prior note
follows.)*

*2026-08-15 (docs cleanup — 48 verified Issues archived to decade files, 4 closed Issues
archived; 6 stale full entries (I-0064, I-0067–I-0071) reconciled against their authoritative table rows.
10 `Resolved - Not Verified` Issues remain active.)*
