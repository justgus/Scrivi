# Active Sprint

## SP-102 — `[Apple]` Pending presentation + warning view + `sources` card

**Status:** 🟡 **Planned — ready to implement (planning completed 2026-08-17).**
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Codebases:** `[Apple]` only — ⚠️ **no `scrivi.h` change, no ScriviCore change, no pbxproj-invisible work**
**Activated:** 2026-08-17 · **Runs before** SP-100 (which runs last and closes the Epic).

---

## Why this sprint exists

SP-099 was **split at planning** (R2, user-approved) because the staged sprint mixed engine plumbing, CRUD
UI, and failure-surface work into a single verification pass. SP-099 took the engine wrappers, object cards
and picker (**AC16/17/21/22**, plus AC18/19/20 which landed early at R4). **SP-102 takes what is left of
EP-031 AC9: the failure surface** — how the app behaves when a world is *not there*.

⚠️ **This is the presentation half of the Epic's highest-risk property.** AC7 — *absence is never deletion* —
is already met and Verified in ScriviCore (SP-098/T-0380): the graph holds edges **pending** toward an
unavailable world and never prunes them. **A writer must be able to see that guarantee**, because the
alternative is the failure mode I-0115 already produced once in real use: a writer told her world was
*missing* when it was perfectly fine, invited toward a destructive remedy.

---

## ⚠️ Planning audit (2026-08-17) — the scope was materially wrong

The staged scope dates from 2026-08-13. **Three unplanned sprints have landed since** (SP-104, SP-105,
SP-106), two of them directly on the world surface. A code audit against the current tree found **most of
T-0389's stated scope already shipped**, and the sprint is much smaller than its own description.

### Already built — struck from this sprint

| Staged as SP-102 work | Actually shipped in | Evidence |
| --------------------- | ------------------- | -------- |
| **Worlds menu** (list / create / bind) | **SP-099/T-0408** (added at R4) | `Scrivi/Views/WorldsView.swift` |
| Worlds-menu **remove + "Locate…"** repair | **SP-104/I-0117** | `WorldsView.swift`, `ScriviEngineGraph.swift:220,241` |
| Pending rows **shown, not hidden** | SP-099/T-0386 | `ObjectCard.swift:152-171` — pending entries survive the index filter deliberately |
| Cached entries **named, never bare IDs** | SP-099/T-0386 | `ObjectCard.swift:165` uses `edge.otherDisplayName` |
| **⚠ badge** on a pending row | SP-099/T-0386 | `ObjectCard.swift:684-691` |
| Remove **disabled and explained**, not absent | SP-099/T-0386 | `ObjectCard.swift:719-723` + `pendingHelp` |
| Frozen-graph refusal read as protection | SP-099/T-0386 | `ObjectCard.swift:290-295` — catches `worldPending:<status>` |
| `WorldStatus` typed decode + honest fallback | SP-099/T-0407 | `ScriviError.swift:56-58,73-97`; unknown status → `.unavailable` |
| Edit suppressed on a pending row | SP-099/T-0386 | `ObjectCard.swift:703-706` |

> ⚠️ **AC18/19/20 also already landed** (T-0388, SP-099 R4) — in-stack create/edit, edit-state visuals, and
> complete-or-discard at the scene boundary, including the I-0119 origin-scene fix. The Epic file's older
> notes still assign them here; they are superseded.

**Consequence: AC23 is close to already met in code, but it has never been verified — and verification is the
criterion.** AC23 is not "pending rows render"; it is *"reattaching restores the card **with no writer
intervention**"*, which only a live ejectable-volume run can demonstrate (R3).

### Genuinely unbuilt — this sprint's real work

| # | Item | State today |
| - | ---- | ----------- |
| 1 | **Card-level pending footer** — the §7.2 sentence *"World «Midgard» is offline. These links are held pending."* | ❌ Absent. Per-row `.help()` tooltips exist; **the card names no world and shows no status sentence.** A tooltip is not a surface a writer discovers. |
| 2 | **Warning view** — project-wide list of unavailable worlds + pending counts | ❌ Absent. `listPendingEdges` is **wrapped in Swift and has zero call sites** — the same "capability shipped, surface did not" shape as the R4 and I-0117 findings. |
| 3 | **`offline`/`unmounted` refinement (AC24)** | ❌ Absent. `WorldTypes.hpp:67-68` still comments them "platform-layer refinement"; **the core emits only `missing`/`unavailable`.** |
| 4 | **Aggregate `sources` card (T-0365)** | ❌ Absent. Not registered in `registerBuiltIns()` (`InspectorCard.swift:213-220`); no citation popup anywhere. |

---

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0389 | Pending **footer** + warning view + the `offline`/`unmounted` refinement (**AC24**) | 🟠 **Implemented - Not Verified (2026-08-17)** |
| T-0365 | Aggregate `sources` card + citation popup (final third) | 🔵 Planned |
| T-0415 | ⚠️ **AC23 live verification on the real USB world rig** — both branches (**no fixture needed**) | 🔵 Planned — **unblocked by T-0389** |

### T-0389 delivery (2026-08-17) — 🟠 Implemented, awaiting live verification

**1. Card-level §7.2 footer** — `PendingWorldFooter` in `ObjectCard.swift`. Groups pending entries **by
world**, names each world, states its status, and says the links are **held pending**. `Entry` gained
`pendingWorldID` so the card can name the world rather than warn anonymously. Before this, the world and its
status appeared **only in a hover tooltip** — undiscoverable, and absent entirely on iPad.

**2. Warning view** — new `Scrivi/Views/WorldWarningView.swift` (`WorldWarningModel` + `WorldWarningView`),
wired into `EditorView` **below the manuscript and above the Timeline**, with `worldWarningVisible` on
`ProjectSession` and a **"Show World Warnings"** toggle at all four View-menu sites plus the iOS toolbar menu.
Reloads on appear **and on app-foreground** — ⚠️ a drive can be ejected *while the project is open*, which is
the entire AC23 scenario. Reports **every unavailable bound world**, not only those with pending edges: a
world that is away with nothing yet linked still blocks creation, and silence would read as "all is well".
✅ **No destructive affordance** — it routes to Manage Worlds and performs no repair.

**3. AC24 refinement** — new `Scrivi/App/WorldVolumeStatus.swift`. Applied at **`WorldEntry.worldStatus`**,
the single accessor all five call sites already read, so no site restates the rule (the kind-list defect in
another costume). ⚠️ **The natural implementation was proven wrong and the test proves it** — see below.

**Evidence**

| Suite | Result |
| ----- | ------ |
| macOS interop | **91/91 macOS arm64** (from 86 — **5 new AC24 tests**) |
| ScriviCore `ctest` | **516/516 macOS arm64** — untouched, as expected (no core change) |
| App build | **BUILD SUCCEEDED** |

> ⚠️ **Proven RED before green, and it caught the real defect.** Mutating `WorldVolumeStatus` to the natural
> rule — key `unmounted` off `volumeIsRemovableKey`/`volumeIsEjectableKey` — fails with
> `Expectation failed: (status → .missing) == .unmounted`. **That is I-0115 recurring**: an unmounted drive
> reported as `missing`, the status that tells a writer to restore from backup. A `hdiutil` fixture reports
> `ejectable == true` and would have **passed** the broken rule.

> ✅ **`listPendingEdges` probed through `scrivi_*` before use** (the zero-call-sites risk). It returns
> `{"ok":true,"result":null}` when nothing is pending — the T-0407 null-result shape, already handled by
> `decodeC`. No boundary gap.

> ✅ **The user's data is provably untouched:** `relationships.jsonl` still checksums to the pre-work baseline
> `fad2f8fe…`, and the world package diffs byte-identical against its backup.

**Not yet established:** AC23 and AC24 are **implemented, not verified** — that is T-0415's job, and it needs
the drive ejected. **No AC ticks on this work alone.**

### T-0415 first live run (user, 2026-08-17) — ⚠️ AC23 FAILS on the reattach branch

The user ran the real rig: added objects to Eskandar (now **15 links**), quit, relaunched, verified, quit,
**ejected**, relaunched, then **reattached**.

**What PASSED — T-0389's own deliverables both worked:**

| Behaviour | Result |
| --------- | ------ |
| Eject → links **held pending**, not pruned | ✅ **15 links held**, world named *Eskandar* |
| Warning surface reports the unavailable world | ✅ *"1 world … not available"* |
| **Eject while running** → warning appears, links go pending | ✅ (second pass, app left running) |
| **Reattach while running** → warning clears automatically | ✅ — the `didBecomeActive` reload works |

**What FAILED:** ⚠️ **reattach after launching WITHOUT the volume never recovers** — four reported symptoms,
**one cause**, correctly diagnosed by the user: *"the world had never been loaded into the project during app
launch."* → **[[I-0123]]**.

⚠️ **The distinction that makes this precise, and that the sprint plan did not anticipate:** AC23 holds when
the world is present **at open** and goes away later (verified above), and fails when the world is absent
**at open** and returns later. The plan's step 2→3 assumed one eject/reattach cycle within a running app; the
user tested the harder ordering, which is the one a writer actually hits — she plugs the drive in *after*
noticing the warning.

**Four Issues opened from this run — all four now 🟠 Resolved - Not Verified (2026-08-17):**

| ID | Severity | Summary | Fix |
| -- | -------- | ------- | --- |
| [[I-0123]] | **High** | A world reappearing while Scrivi runs is never re-activated — `activateWorlds` is called from `loadProject` only. **This was the AC23 blocker.** | `AppEnvironment.reconnectWorlds()` on **app-foreground** + a **"Reconnect Worlds"** button in the Worlds manager (the writer's own proposed remedy) |
| [[I-0124]] | **High** | While pending, **every** object shows on **every** world-scoped card — the pending branch tested `cardKind.isWorldScoped` (a property of the CARD), and ⚠️ the C ABI never emitted `otherKind` (the I-0113 shape, 7th occurrence) | `otherKind` added to `EdgeView`, serialized, decoded, and compared to `cardKind.kind`. Probed through `scrivi_*` |
| [[I-0125]] | **High** | ⚠️ **Object created on disk, then the edge fails, and the writer is told it failed** — `appears-in` was `character`→`scene`, but **8 of 10 cards request it**. Only `character` and `location` could be created from a card *at all*. Pre-existing, unrelated to SP-102. | **R5:** `appears-in` unconstrained on source; inverse label → **"features"**. ⚠️ **(b) deliberately NOT changed per R6** — the object is kept, never rolled back |
| [[I-0126]] | Low | Card error text persists after Discard until the scene changes | `commitError` cleared on both discard paths and on success |

**Suites after the fixes** (all re-run 2026-08-17 once the app was closed):

| Suite | Result |
| ----- | ------ |
| ScriviCore `ctest` | **520/520 macOS arm64** (from 519 — one new I-0125 regression test covering six previously-unlinkable kinds) |
| macOS interop | **91/91 macOS arm64** |
| App build | **BUILD SUCCEEDED** |

⚠️ **Four tests had to be amended because they PINNED the I-0125 defect** — two in `ctest`
(`RelationshipTests.cpp`) and two in interop (`ScriviInteropTests.swift`), all asserting either the
`character`-only constraint or the old `"has characters"` inverse label. The constraint machinery is still
covered: the amended `ctest` section now asserts `sibling-of` (character↔character) still refuses a location,
so the section is not left asserting nothing.

✅ **User data verified additive-only** against the pre-work backup: **nothing lost**, 8 objects added — the
user's own verification work (4 characters, 3 chronicles, 1 location).

### T-0415 second live run (user, 2026-08-17) — ⚠️ **AC23 nearly passes; one refresh gap**

Re-run of the exact sequence that failed before, after the I-0123–I-0126 fixes:

| # | Step | Result |
| - | ---- | ------ |
| 1 | Add objects, quit, relaunch, verify | ✅ **Done** |
| 2 | Eject → links held pending, world named, status `unmounted` | ✅ **Passed** |
| 3 | **Reattach → recovery with no relaunch** | ⚠️ **PARTLY passed** — warning cleared and objects relinked, **but the inspector cards did not refresh until the scene was changed** → [[I-0128]] |
| 4 | Each card shows only its own kind while pending | ✅ **Passed** — I-0124 confirmed fixed |
| 5 | Create a Chronicle | ✅ **Passed** — I-0125 confirmed fixed |

✅ **I-0123 is substantially confirmed:** the reconnect path that previously required a relaunch now recovers
the world and relinks the graph. **The residue is presentation only** — the cards held their stale pending
view because they key on `sceneID`, which a reconnect does not change.

⚠️ **[[I-0128]] is [[I-0105]] recurring one layer over.** Same cause both times: state changed somewhere the
card's `.task(id:)` could not observe (there a history commit, here world availability). Fixed the same way —
a monotonic revision folded into the reload key — rather than inventing a second mechanism.

**Second finding, unrelated to this sprint:** [[I-0127]] — the "Add \<Kind>…" picker popover showed **a
fraction of one row**. `.frame(maxHeight:)` is a *maximum*; a `ScrollView` in a popover has no intrinsic
height and collapsed instead of expanding. ⚠️ **It defeats AC17's deliberately unfiltered picker** — the
writer is shown everything precisely so she can choose, and she could not see it.

> **Both fixed 2026-08-17**, 🟠 Implemented - Not Verified. App **BUILD SUCCEEDED**, interop **91/91 macOS
> arm64**, `ctest` **520/520 macOS arm64**. ⚠️ **Step 3 needs one more live pass** to confirm the cards now
> refresh on reconnect without a scene change.

### T-0415 third live run (user, 2026-08-17) — picker approved; one trigger defect found

✅ **[[I-0127]] approved by the user** — the picker popover is now usable.

⚠️ **[[I-0129]] — the user isolated a defect by reversing the usual order.** Every prior test had ejected the
drive, which *requires leaving Scrivi*, so focus always changed and the refresh always fired. Returning focus
**first** and then plugging the drive in exposed that `reconnectWorlds()` was driven **only** by
`NSApplication.didBecomeActiveNotification`:

| Order | Result |
| ----- | ------ |
| Eject/plug **while Scrivi is in the background**, then return focus | ✅ warning updates both ways (this is what every earlier run did) |
| Return focus **first**, then plug the drive in | ❌ warning stays up; changing scenes fixed the cards but **not** the warning |

⚠️ **Focus is a proxy for "something may have changed", not the change itself.** The user proposed a periodic
timer as the likely recourse; **none is needed** — `NSWorkspace` reports mount/unmount directly, which is
immediate, exact, and costs nothing when idle.

✅ **Proven, not assumed:** a self-contained probe attached and detached a `hdiutil` image while observing and
captured `MOUNT: /Volumes/ScriviMountProbe` then `UNMOUNT: …` — 2 events. (The probe image was removed
afterwards.)

**Suites:** interop **93/93 macOS arm64** (+2 pinning the notification contract), `ctest` **520/520 macOS
arm64**, **BUILD SUCCEEDED**.

#### ✅ Live re-test PASSED (user, 2026-08-17) — both directions, focus never leaving Scrivi

| Step | Result |
| ---- | ------ |
| Scrivi focused → **unplug the drive** (surprise removal, not an eject) | ✅ warning **and** Scene Inspector updated **instantly** |
| Still focused → **plug the drive back in** | ✅ both updated correctly |

⚠️ **The user tested a harder case than the fix was designed against.** He did not eject — he physically
pulled the drive, which macOS reported as *"Drive not ejected properly"*. A surprise removal is a different
path from a graceful unmount and could have delivered a different notification, or none. It confirms
`didUnmountNotification` fires on surprise removal too — **and that is the case a writer actually hits by
accident**, so it matters more than the clean one.

This closes the focus-coupling gap: **world availability no longer depends on app focus in either
direction**, and [[I-0128]]'s card refresh rides the same trigger.

> ⚠️ **MIGRATION GAP on I-0125 — the code fix alone does NOT help an existing project.**
> `RelationTypeStore::load` re-seeds **only** when `relation-types.json` is missing or unusable
> (`RelationTypes.cpp:150-176`); a valid file is loaded verbatim. The user's project therefore still carries
> the **old constrained `appears-in`** and will keep failing chronicle/faction/artifact creation until its
> file is refreshed. **This needs a ruling** — see *Open question* below.

> ✅ **The user's read that "the Chronicles problem is not related to the rest of the Sprint" is correct.**
> I-0125 is a SP-099 defect in the relation vocabulary; it was merely invisible until someone created a kind
> other than character or location. ⚠️ **It also means eight of the ten object cards have never been able to
> create anything**, which no test caught because the interop suite exercises `character`.

> ⚠️ **All four were found by USE, not by the suites** — 91/91 interop and 516/516 ctest were green
> throughout. That is now **five consecutive sprints** where the defects that mattered came from live running
> (SP-099, SP-103, SP-104, SP-105, and now SP-102), and it is the single most important input to **SP-100's**
> evidence standard.

> **T-0415 is new, added at this planning session.** It was previously an exit criterion inside T-0389.
> ⚠️ **It is separated because it is the only thing that can establish AC23**, and burying a live-verification
> requirement inside an implementation task is how it gets reported as "done" on a fixture. **SP-106's whole
> lesson is that evidence which cannot distinguish pass from fail is not evidence.** It also has a real
> prerequisite (a mountable `.dmg`) that belongs on the board, not in a checklist line.

## Assigned Issues

*None at activation.* ⚠️ **I-0114–I-0117 (SP-104) remain 🟠 `Resolved - Not Verified`** and live on the same
world surface this sprint touches. They belong to the closed SP-104 and are **not** re-assigned here — but
this sprint's live run will exercise them, so **offer them for verification when it happens** rather than
letting a second sprint pass over the same ground.

---

## Acceptance criteria this sprint delivers

Both belong to **EP-031 AC9**, which ticks when they land (AC16/17/21/22 and AC18/19/20 are done).

| AC | Requirement | Source |
| -- | ----------- | ------ |
| **AC23** | Pending links **survive** an unavailable world and are **restored on reattach with no writer intervention** | Doc 2 §7.2, Doc 3 §4.6 |
| **AC24** | The unavailability **status is reported specifically** (offline / unmounted / missing), falling back to a generic *unavailable* rather than a guess | Doc 2 §7.2.1 |

---

## Rulings taken during T-0415 verification (2026-08-17)

**R5 — `appears-in` is WIDENED to any kind → scene** (user-ruled), resolving [[I-0125]](a).

Drop `appears-in`'s `sourceKind: "character"` constraint so a null source means **any kind**, as `cites`
already does on both ends (Doc 1 §3.4). One type covers all ten kinds appearing in a scene; no new vocabulary
to seed, document, or migrate. ⚠️ **The inverse label must be relabelled** — `"has characters"` is wrong the
moment a chronicle or faction uses the type; it becomes kind-neutral (e.g. *"features"*).

**R6 — ⚠️ AN UNLINKED WORLD OBJECT IS A NORMAL STATE, NOT DAMAGE. Never roll one back** (user-ruled).

I proposed deleting the just-created object when its edge fails, and framed the user's three chronicles as
"orphans" needing cleanup. **Both premises were wrong, and the user corrected them:**

> *"You say I have 3 orphaned chronicles now, but that is not strictly true… I was able to find the
> chronicles and manually relate them to the scene by selecting 'Add Chronicle' after the edge creation
> failed. Also, orphaned world objects are perfectly acceptable. I can develop a world in project 1 and then
> bring the world into project 2 and all the objects in that world are technically 'orphaned' in project 2."*

⚠️ **This is a property of the world model, not an edge case.** A world is shareable across projects (Doc 3),
so **every object in a newly-bound world is unlinked in that project by definition**. A rollback rule would
therefore have deleted legitimate authored work in service of tidying a state the design treats as ordinary —
and the recovery path the user actually used (Add Chronicle → pick the existing object) already works.

**Ruled:** on edge failure, **keep the object and report honestly** — "created, but not linked to this
scene." The fix for [[I-0125]] is to stop the edge failing (R5); the half-success reporting is the honest
fallback if it ever fails again. This aligns with EP-031's existing stance that orphans are **retained, not
swept** (AC4, SP-098/T-0378 `scrivi_list_orphaned_objects`).

> ⚠️ **Recorded because I nearly built the opposite.** "Create failed, so undo the write" is the tidy-looking
> rule, and it is destructive here precisely because unlinked ≠ broken in this model.

---

## Rulings taken at planning (2026-08-17)

**R1 — the warning view gets its OWN toggle and is not anchored to the Timeline** (user-ruled).

Doc 3 §4.6 says *"a dedicated warning/log view beneath the Timeline view"*, but the Timeline is
**hidden by default** and toggled via View ▸ Show Timeline (`EditorView.swift:151,164,222`). Anchoring the
warning view strictly beneath it would make **the only project-wide pending report invisible exactly when a
writer is not using the Timeline** — i.e. most of the time.

**Ruled:** a strip below the manuscript with its **own View-menu toggle**, independent of the Timeline;
**auto-reveals when a bound world goes unavailable**, dismissible, and absent when nothing is pending.
This honours Doc 3's *intent* — a passive, non-blocking report the writer can ignore — without inheriting an
unrelated view's visibility. ✅ **Doc 3 §4.6 amended 2026-08-17, at planning and before implementation** —
the SP-103 discipline: *code must not run ahead of the spec.* The original "beneath the Timeline view"
wording is struck in place, with the reasoning retained.

**R2 — AC24 is refined via URL volume keys, both states, with an honest fallback** (user-ruled).

`offline` and `unmounted` are distinguished in the **Apple platform layer** feeding the neutral enum, against
the world's `packagePath`. Doc 3 §4.4.1 forbids a platform-specific *model*, and this respects that: the model
stays neutral, only the diagnostic improves.

> ⚠️ **PROBED ON THE REAL RIG (2026-08-17) — the obvious signal does NOT work, and would misreport the
> user's actual drive.** `/Volumes/Scrivi Worlds` is a **931 GB USB external volume** (`disk7s1`, APFS;
> `diskutil`: `Protocol: USB`, `Device Location: External`). Probing it with `URLResourceValues` returns:
>
> ```
> volumeIsRemovable : false      ← ⚠️ on a drive the user physically unplugs
> volumeIsEjectable : false      ← ⚠️ likewise
> volumeIsLocal     : true
> volumeIsInternal  : false
> ```
>
> **`diskutil` itself reports `Removable Media: Fixed` for this drive.** Keying `unmounted` off
> `volumeIsRemovableKey`/`volumeIsEjectableKey` — the natural reading of R2 — would classify this world's
> volume as *not removable* and report the **wrong status** for the one rig we actually verify against.
> ⚠️ **This is exactly the I-0115 failure shape**: a plausible inference that is confidently wrong.
>
> ✅ **Use volume-root presence instead, which was probed and does work.** Resolve the package's volume root
> and ask whether it is currently mounted (`FileManager.mountedVolumeURLs`, or the root's existence):
>
> | Condition | Status |
> | --------- | ------ |
> | Volume root **not mounted**, package path under it | **`unmounted`** |
> | Volume root **mounted**, package **positively absent** | **`missing`** |
> | Volume mounted, package present but **unreadable** (sandbox/permissions) | **`unavailable`** — never `missing` (I-0115) |
> | Network volume unreachable (`volumeIsLocal == false`, or NFS/SMB URL scheme) | **`offline`** |
> | Anything inconclusive | **`unavailable`** |
>
> Probe evidence: an unmounted volume yields `exists: false` on both the package **and its volume root**, and
> `resourceValues` fails outright — so the two cases are cleanly separable. `volumeIsLocal` remains the right
> signal for the **`offline`** branch specifically, which is the one it actually describes.
>
> ⚠️ **`binding.json` carries `"volumeLabel": ""` — empty on the real rig**, so the status logic must not
> depend on it. Derive the volume root from `lastKnownAbsolutePath` instead.

⚠️ **Whenever inspection is inconclusive, the status is `unavailable` — never a guess.** This is I-0115's
rule, and I-0115 is the proof it matters: a present-but-unreadable package was reported `missing`, the one
status reserved for positive proof of absence. **A wrong `missing` invites a destructive remedy.**

✅ **Buildable with no ABI change:** `WorldEntry.packagePath` is already exposed to Swift
(`ScriviEngineGraph.swift:416`), so the refinement needs no `scrivi.h` change and no core change.

**R3 (carried from SP-099 planning, 2026-08-13) — AC23 is verified on a real ejectable volume.** ⚠️ **A
fixture cannot demonstrate AC23**, because the criterion is that reattaching restores the card *with no writer
intervention*. Both branches need coverage since they report different statuses. → now **T-0415**.

> ✅ **SUPERSEDED IN ITS MECHANISM, not its intent (2026-08-17): the rig is real and already exists.** R3
> specified a `hdiutil` disk image because no real one was known. The user's actual setup is
> `~/Desktop/the-stairs-of-tintagael.scrivi` bound to `/Volumes/Scrivi Worlds/Eskandar.scrivworld` on a
> **931 GB USB external drive** — a live project, a real world with 7 cached entries and 9 edges, on hardware
> that physically unplugs. **No disk image is needed.**
>
> ⚠️ **The real drive is the harder test, which is why it matters.** A `.dmg` reports
> `volumeIsEjectable == true`; this drive reports **`false`** (see R2). The fixture would have passed a
> detection rule that fails on the user's own hardware.

**R4 — `source` stays out of the ten object cards.** It is the only project-scoped kind (Doc 1 §3.0) and it
surfaces through **one aggregate card** in the Writing stack, never a per-kind card in Worldbuilding
(`ObjectCard.swift:41-57` already encodes this). T-0365 adds a card to the **Writing** stack, not an eleventh
object card.

---

## Scope

### T-0389 — pending footer + warning view + status refinement

**1. Card-level pending footer (Doc 2 §7.2).** The card must render the *§7.2 block*, not just per-row
badges: a divider, then the world **named** and its **status sentence** —
*"World «Midgard» is offline. These links are held pending."*

- ⚠️ **The card currently names no world at all.** A writer sees an orange triangle and a greyed name; she is
  never told *which* world is away or *why*. The tooltip that carries it today
  (`ObjectCard.swift:728-731`) is hover-only — **not a surface a writer discovers**, and useless on iPad.
- Group pending entries by world when a card holds entries from more than one.
- ⚠️ **Nothing that would destroy pending links may appear here.** Any "clean up" affordance belongs solely
  to the Worlds menu (§7.2, §7.3) — never inline on the card, where it reads as routine tidying.

**2. Warning view (Doc 2 §7.3, ruled shape per R1).** A passive, non-blocking strip below the manuscript:

- Lists **unavailable worlds and their pending counts**, sourced from `listPendingEdges` + `listWorlds`.
- **Its own View-menu toggle**; auto-reveals when a bound world goes unavailable; dismissible.
- **No modal, no blocking dialog, at any point in this flow.**
- ⚠️ **"Remove All World References" does NOT go here.** It lives in the Worlds menu and must be **sought out
  deliberately** (§7.3). Putting a destructive global action in the surface that appears *automatically when
  something looks broken* is precisely the trap Doc 3 §4.6 warns about.

**3. The `offline`/`unmounted` refinement (AC24) — per R2.** Apple-layer volume inspection against
`packagePath`, feeding the neutral enum, degrading to `unavailable` when inconclusive.

### T-0365 — the aggregate `sources` card (final third)

Two thirds are ✅ Verified: the `cites`/`documented-by` relation type (SP-096, `RelationTypes.cpp:84-87`) and
the `source` object kind (SP-098/T-0406). **Only the card remains.**

- **ONE aggregate card** in the **Writing** stack, not a card per source (ruled 2026-08-12) — a per-source
  card would flood the stack, and decisively, one card can be shown/hidden as a unit in the picker.
- Renders the **indirect** path: `this scene ──edges──▶ objects ──cites──▶ sources`. Sources attach to
  **objects**, never directly to scenes (Doc 1 §3.4).
- **Scope is this scene's objects.** An empty card is **not an error state** — neither for a scene whose
  objects carry no citations, nor for a scene with no objects.
- Each entry **names the object(s) it came from**; a source reached through two objects appears **once**,
  listing both.
- **Click → citation popup.** The same popup is reachable from worldbuilding-object cards: **one popup
  implementation, two entry points** — build it as a shared view, not twice.
- ⏸ Sources do **not** appear in the manuscript — footnotes/pull quotes are **EP-032**. This card is a
  **read-only view onto the graph**; it writes no scene text and implies no source→scene edge.
- Register in `registerBuiltIns()` (`InspectorCard.swift:213-220`), Writing stack, **ships hidden** like
  every other card (Doc 2 AC7 — nothing appears without an explicit writer action).

### T-0415 — AC23 live verification (per R3, on the user's real rig)

✅ **The rig already exists and needs no fixture** (confirmed 2026-08-17):

| Piece | Location | State |
| ----- | -------- | ----- |
| Project | `~/Desktop/the-stairs-of-tintagael.scrivi` | Real, in use |
| World | `/Volumes/Scrivi Worlds/Eskandar.scrivworld` | `world_character_01a000fb-…` |
| Volume | `disk7s1`, **931 GB USB external**, APFS | **Physically unpluggable** |
| Binding | `worlds/world_character_01a000fb-…/binding.json` | **7 cached entries** — 3 characters (Myton, Veyra, Petch), 4 locations (Tintagael, The Edge District, The Underside, The Forest that Remembered) |
| Edges | `objects/relationships.jsonl` | **9 records** |

⚠️ **This is a better rig than the planned disk image, and R3's `hdiutil` fixture is no longer needed.** A
synthetic `.dmg` would have been *ejectable-flagged*; this drive is **not** (see R2's probe), so it exercises
the harder and more realistic case — the one that would have been misreported.

⚠️ **BACK UP THE PROJECT AND THE WORLD BEFORE STEP 1.** This is the user's real writing project, and AC23's
whole subject is data preservation. A backup already exists at
`~/Desktop/the-stairs-of-tintagael.scrivi.backup-20260729-162050`, but it predates the world binding — take a
fresh one, of **both** the project and the world package, before any run.

1. **Baseline:** open the project with the volume mounted; confirm the 7 world objects resolve on their cards.
   **Copy `relationships.jsonl` aside** for the byte-comparison in step 5.
2. **Eject the drive** (Finder eject, then physically unplug for a second pass) → cards show pending: named
   entries, ⚠ badge, and the new footer naming **Eskandar** with status **`unmounted`** — ⚠️ **not `missing`**;
   the warning view lists Eskandar with its pending count.
3. **Reattach** → ⚠️ **the cards restore with NO writer intervention.** No relink, no repair prompt, no
   restart. *This is the actual AC.* (Watch the `WorldBookmarkStore` grant from I-0116 survives the cycle.)
4. **Second branch — `missing`:** with the volume mounted, rename `Eskandar.scrivworld` aside → status must
   report **`missing`** (positive proof of absence); rename it back → cards return. Both branches are required
   because they report different statuses.
5. **Assert `relationships.jsonl` is byte-identical** to the step-1 copy across the whole run — AC-A2's
   "verbatim" property. ⚠️ **This is the one that matters most:** a tombstone written and later compacted out
   would satisfy a weaker reading and still have destroyed the writer's links.
6. **Opportunistic:** this run exercises I-0114–I-0117's surfaces — offer them for verification.

---

## Exit criteria

| # | Criterion | How it is established |
| - | --------- | --------------------- |
| 1 | A card whose world is unavailable renders the **§7.2 block**: named entries, ⚠ badge, **the world named**, and its status sentence — never hidden, never bare IDs | Live + interop |
| 2 | Add/remove toward an unavailable world stays **disabled and explained**; no inline affordance can destroy pending links | Live |
| 3 | ⚠️ **AC23**: eject → pending; reattach → **restored with no writer intervention**. Both `unmounted` and `missing` branches | **T-0415, live on the real USB rig — a fixture cannot establish this** |
| 4 | **AC24**: `offline`/`unmounted` produced by the Apple layer into the neutral enum, **degrading to `unavailable`, never guessing `missing`** — and **correct on the user's own drive**, whose removable/ejectable flags both read `false` | Unit + live |
| 5 | Warning view lists unavailable worlds + pending counts, non-blocking, own toggle, **no destructive action present** | Live |
| 6 | `sources` card lists sources via this scene's objects, names the originating object(s), **dedupes** across objects, opens the shared citation popup; empty is not an error | Live + interop |
| 7 | The edge log is **byte-identical** across an eject/reattach cycle | T-0415 step 5 |
| 8 | `ctest` + macOS interop green, **each figure naming its architecture** | SP-106 standing practice |

---

## Risks and watch-items

⚠️ **1. The kind-list rule is at risk again — seven occurrences, two in Swift.** CLAUDE.md's standing rule:
any list partitioning `ObjectKind` must **derive** from `kAllStorableKinds` + `objectKindIsWorldScoped()`.
`ObjectCard.swift:46` currently derives correctly (`kind != "source"`), which is the I-0114 fix. **T-0365
adds a card that reasons about `source` specifically** — the exact shape that produced occurrence 5. Derive;
do not restate.

⚠️ **2. `listPendingEdges` has zero call sites — verify the payload before designing against it.** The R4 and
I-0117 findings were both "the capability shipped, the surface did not", and I-0113 was "the boundary drops
what the core knows." **Probe it through `scrivi_*` first** (`feedback_boundary_tests_not_facade`), because a
facade test cannot see a boundary gap.

✅ **3. Doc 3 §4.6 amended for R1 at planning** (2026-08-17), before any code — not after.

⚠️ **4. Every defect that mattered in SP-099/103/104/105 was found by USE, not by the suites**, which were
green throughout. Budget for live findings; do not treat a green run as the finish line.

⚠️ **5. T-0415 runs against the user's REAL writing project and REAL world.** `the-stairs-of-tintagael.scrivi`
is live work, not a fixture, and the world holds 7 objects on an external drive. **Back up both before the
first run** — the existing Desktop backup is from 2026-07-29 and predates the world binding. The irony is
worth stating: AC23 is the criterion that *absence is never deletion*, and verifying it carelessly is the one
way to lose the very data it protects.

⚠️ **6. The volume flags lie on this hardware — do not trust `volumeIsRemovable`/`volumeIsEjectable`.** Probed
`false` on a USB drive the user unplugs by hand (R2). Use volume-root mount presence. A rule that passes on a
`.dmg` and fails on the user's drive is worse than no rule.

**7. Linux has no counterpart** — the Qt app has no inspector cards and no warning view. This is `[Apple]`
only, and the asymmetry is expected, not a defect.

---

*Last Updated: 2026-08-17 (**Planning completed.** ⚠️ **A code audit found most of T-0389's staged scope
already shipped in SP-099/SP-104** — pending rows, ⚠ badge, named entries, disabled-and-explained removal,
typed status decode, and the Worlds menu are all built and struck from this sprint. What remains is the
**card-level §7.2 footer** (the card names no world today), the **warning view** (`listPendingEdges` has zero
call sites), the **AC24 volume refinement**, and the **`sources` card**. Two rulings taken: **R1** the warning
view gets its own toggle rather than being anchored to the hidden-by-default Timeline (Doc 3 §4.6 to be
amended); **R2** AC24 refined via URL volume keys for both states, degrading to `unavailable` and never
guessing `missing` — buildable with **no ABI change**. **T-0415 added**: AC23 live verification split out of
T-0389, because a fixture cannot establish "restores with no writer intervention".)*

*2026-08-17, later same day (**T-0415's rig identified — it is REAL and already exists**, so R3's `hdiutil`
disk image is dropped: `~/Desktop/the-stairs-of-tintagael.scrivi` bound to
`/Volumes/Scrivi Worlds/Eskandar.scrivworld` on a **931 GB USB external drive**, with 7 cached world entries
and 9 edges. ⚠️ **Probing that drive falsified R2's natural implementation:** `volumeIsRemovable` and
`volumeIsEjectable` both read **`false`** on hardware the user physically unplugs (`diskutil` agrees —
`Removable Media: Fixed`), so the obvious detection rule would misreport the very world we verify against —
the I-0115 shape exactly. **Ruled to volume-root mount presence instead**, probed working, with
`volumeIsLocal` reserved for the `offline` branch it actually describes. Also recorded: `binding.json` carries
an **empty `volumeLabel`**, so nothing may depend on it. ⚠️ **T-0415 runs against real writing work — back up
project and world first.**)*
