# Active Issues

Issues awaiting **user verification**. An Issue leaves this file only when the user verifies it
(→ `Verified/Issue-verified-XXXX-YYYY.md`, batched in decades of ten) or approves its closure
(→ `Closed/`).

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| I-0135 | `[ScriviCore]` **A corrupt or unparseable `world.json` has no test coverage.** The `unavailable` fallback is covered generically (`WorldTests.cpp:234`) but **never for this cause**. ✅ The behaviour appears correct — `parseWorld` validates the schema tag and rejects an empty `worldID` (`WorldJson.cpp:31,43-46`), and resolution continues to the next candidate then degrades to `unavailable` (`WorldStore.cpp:291-292`) — ⚠️ **but nothing proves a corrupt world file is not auto-regenerated, deleted, or reported as `missing`.** §6a.0's *absence is never deletion* rule depends on that, and a corrupt file is **evidence the package EXISTS**, so reporting `missing` for it would be a guess of exactly the kind `WorldTests.cpp:277` was written to prevent. | Low | **unassigned** | 🔴 **Open (2026-08-19)** — found by **T-0390** while writing repair-matrix §6a.3; **filed, not fixed** (SP-100 ruling R3) |
| I-0136 | `[ScriviCore]` ⚠️ **`world.json`'s `formatVersion` is read but NEVER compared against a supported maximum.** `parseWorld` reads it into the record (`WorldJson.cpp:41`) and **no code anywhere validates it** — `grep -rn "formatVersion >" ScriviCore/src/` returns **nothing**. **A world package written by a FUTURE version of Scrivi is parsed as if it were current**, silently, with any fields the current reader does not understand dropped. §6.16 of the repair matrix handles precisely this for *project* files (*"unsupported newer schema version"*); the world path has no equivalent. ⚠️ **This is the one class of defect that cannot be retrofitted:** by the time a newer world file exists in the wild, the old readers that mis-parsed it have already shipped — and a world package is **shared between projects and carried across machines**, which is exactly where version skew occurs. | **Medium** | **unassigned** | 🔴 **Open (2026-08-19)** — found by **T-0390**; **filed, not fixed** (SP-100 ruling R3) |
| I-0137 | `[Apple]` ⚠️ **AC24's `unmounted`/`offline` refinement can NEVER FIRE — `packagePath` is empty for exactly the worlds it must diagnose.** Found by **T-0418** on the real USB rig: with the drive ejected, **every** warning surface said *"unavailable"*; `unmounted` appeared nowhere. **The refinement is not missing — it is unreachable.** `WorldVolumeStatus.refine` is correct, unit-tested (`ScriviInteropTests.swift:2377+`) and correctly wired at the single approved site (`ScriviEngineGraph.swift:445`). But it opens `guard !packagePath.isEmpty else { return coreStatus }`, and ⚠️ **`WorldStore::listWorlds` assigns `s.packagePath` ONLY when `status == available`** (`WorldStore.cpp`, listWorlds ll.17-18) — mirroring `resolve`, which sets `out.packagePath` only on its success branch. **So the one input the refinement needs is guaranteed absent in the one case it exists for.** Every consumer (`WorldsView`, object cards, `WorldWarningView`, `AppEnvironment`) reads the same unrefined value. ⚠️ **This is the "capability shipped, surface never built" pattern in a new costume** — the capability, its tests and its call site all exist, and the data path does not. ⚠️ **AC24 was marked Verified 2026-08-17 on evidence that could not have distinguished this**, because a fixture supplying a `packagePath` would pass while the real rig cannot. **Fix is likely one line in `listWorlds`** — carry the last-known candidate path regardless of status — but ⚠️ **`resolve` deliberately does not report a path it could not verify**, so the semantics need a ruling, not just an edit. ✅ **USER RULING 2026-08-19 — scope of blame settled:** *"The Task was verified. What was not is due to unimplemented software features."* ⚠️ **This Issue does NOT re-open T-0389 or unseat AC24/AC9.** The refinement T-0389 built is **correct and proven** — the interop suite's *"World volume status refinement (EP-031 AC24)"* passes. **The defect is in `WorldStore::listWorlds`, a component T-0389 does not own**, which starves it of `packagePath`. ⚠️ **The fix belongs to whoever owns the data path, not to the Apple layer that consumes it.** | **High** | **unassigned** | 🔴 **Open (2026-08-19)** — found by **T-0418**; **filed, not fixed** (R4) |
| I-0138 | `[Apple]` **"Remove from scene" is disabled for a pending object but NOT explained.** `ObjectCard.swift:829` sets a **static** `.help("Remove from scene")` on the remove button, and `:833` disables it when `entry.pending`. ⚠️ **The code comment two lines above claims the opposite** — *"the affordance is disabled and explained, never simply absent (§7.2)"*. It is disabled and **unexplained**: hovering a greyed-out button yields the same tooltip as a working one, so the writer is told what the button *would* do and never why she cannot use it. `pendingHelp` (`:838`) already composes the right sentence — *"Held pending — this object's world is …"* — and is applied to a **different** control at `:800`. **§7.2 requires disabled-and-explained.** | Low | **unassigned** | 🔴 **Open (2026-08-19)** — found by **T-0418**; **filed, not fixed** (R4) |
| I-0139 | `[Apple]` **Clicking an object's title opens the in-place editor with no evident way back to viewing.** Reported in T-0418 step 2: *"If I click the title of an object in a card it begins to edit the object. This panel cannot be dismissed."* ⚠️ **Partially a discoverability defect rather than a dead end** — the editor does carry a dismissal control: `ObjectDraftEditor` renders **Save/Create** (disabled until valid) and **Revert/Discard** (`ObjectCard.swift:674-679`), and `onDiscard` sets `draft = nil`, closing the panel (`:415`). **"Revert" is never disabled**, so the exit exists. The defect is that **a destructive-styled "Revert" does not read as "stop editing"**, and a single click on a title — with no explicit Edit affordance — puts the card into a mode whose exit is labelled as data loss. ⚠️ **Do not "fix" this by making the panel modal or adding a Cancel that bypasses the unfinished-work prompt** — §4.6 forbids the modal, and I-0119 shows how a commit route *around* that prompt filed an object into the wrong scene. | Medium | **unassigned** | 🔴 **Open (2026-08-19)** — found by **T-0418**; **filed, not fixed** (R4) |

**Currently: 5 Issues open** — **I-0135/I-0136** filed by **T-0390**, **I-0137/I-0138/I-0139** by
**T-0418**'s live pass. All awaiting **triage**, not verification.

> ⚠️ **None is a regression, and none blocks SP-100.** All five were found by exercising shipped behaviour
> rather than assuming it — which is exactly what rulings **R3** (document *and test*) and **R4** (the live
> pass is required evidence) were written to produce.
>
> ⚠️ **I-0137 is the most serious of the five and bears directly on the Epic's close.** AC24 was marked
> **Verified 2026-08-17**, and the live pass shows its refinement **cannot fire on real hardware**. That is
> a question for **T-0391's AC pass**, not something to settle here.

> **I-0133** ✅ Verified 2026-08-19 → [`Verified/Issue-verified-0131-0140.md`](Verified/Issue-verified-0131-0140.md) ·
> **I-0134** ⚪ Closed as a **non-issue** 2026-08-19 → [`Closed/Issue-closed-0134.md`](Closed/Issue-closed-0134.md)
> (erroneous parity premise — **Apple is authoritative; Linux conforms**). Both under audit ruling **R-01**.

---

## Full entries

> The five open Issues carry their detail in the table rows above, each citing `file:line`. ⚠️ **All five
> are 🔴 Open and awaiting TRIAGE — none is `Resolved - Not Verified`.** They were filed by SP-100's
> T-0390 and T-0418 under rulings R3/R4, which require findings to be **filed, not fixed**.

---

## Archive map

| Range | Location |
| ----- | -------- |
| I-0001–I-0050 | `Verified/Issue-verified-0001-0010.md` … `-0041-0050.md` |
| I-0051–I-0120 | `Verified/Issue-verified-0051-0060.md` … `-0111-0120.md` |
| I-0121–I-0130 | `Verified/Issue-verified-0121-0130.md` |
| I-0131–I-0133 | `Verified/Issue-verified-0131-0140.md` |
| I-0019 | `Closed/Issue-closed-0019.md` |
| I-0072, I-0073, I-0085, I-0103 | `Closed/Issue-closed-0072-0103.md` |
| **I-0134** | `Closed/Issue-closed-0134.md` — ⚪ non-issue |

---

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
