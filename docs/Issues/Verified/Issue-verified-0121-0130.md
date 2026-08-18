# Verified Issues: I-0121 – I-0130

Archived Issues, ✅ **Resolved - Verified** by the user. Batched in decades of ten per
`Issue-GUIDELINES.md`.

| ID | Title | Severity | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| I-0121 | `[ScriviCore]` `rebalancedKeys(1)` divides by zero — CI red since 2026-07-30 | High | SP-106 | 2026-08-17 |
| I-0122 | `[ScriviCore]` `stack-use-after-scope` — test iterates a destroyed temporary `Result` | Medium | SP-106 | 2026-08-17 |
| I-0123 | `[Apple]` A world that reappears while Scrivi is running is never re-activated — blind until relaunch | High | SP-102 | 2026-08-17 |
| I-0124 | `[Apple]` While a world is unavailable, EVERY pending object appears on EVERY world-scoped card | High | SP-102 | 2026-08-17 |
| I-0125 | `[Apple]`/`[ScriviCore]` Creating any object but character/location fails at the EDGE step — object orphaned | High | SP-102 | 2026-08-17 |
| I-0126 | `[Apple]` A card's error message persists after the draft is dismissed, until the scene changes | Low | SP-102 | 2026-08-17 |
| I-0127 | `[Apple]` The "Add \<Kind>…" picker popover is too short to use | Medium | SP-102 | 2026-08-17 |
| I-0128 | `[Apple]` Scene Inspector cards did not refresh when a world reconnected | Medium | SP-102 | 2026-08-17 |
| I-0129 | `[Apple]` World availability refreshed on app FOCUS, not on the drive actually mounting | Medium | SP-102 | 2026-08-17 |
| I-0130 | `[Apple]` "Locate…" repaired the world but the project-wide warning stayed stale | Medium | SP-102 | 2026-08-17 |

> ⚠️ **This table listed only I-0121, I-0122 and I-0130 until 2026-08-18** — the seven Issues between
> them had full write-ups below and **no index rows**, so they were invisible to anyone reading the
> summary. That is how the SP-102 exit-criteria table came to be read as "AC23/AC24 still owed" when
> **I-0123–I-0130 are the record of the live eject/reattach runs that established them.** Detail
> correct, index lying — the same failure class as the 2026-08-18 Task-backlog audit.

---

## I-0121 — ⚠️ `rebalancedKeys(1)` divides by zero; ScriviCore CI was red on every commit since 2026-07-30

**Severity:** High · **Sprint:** SP-106 · **Epic:** EP-031
**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**

**Symptom.** ScriviCore CI failed on **every commit from 2026-07-30 (1c42838) to 2026-08-16** — 17 days,
7 commits — while every local `ctest` run and every `macos-latest` CI leg reported green.

**Defect.** The `n == 0` ternary guard in `rebalancedKeys` is unreachable while the divisor is `n - 1`, so
`rebalancedKeys(1)` divides by zero.

⚠️ **Why it survived 17 days.** Integer division by zero is **UB, and UB is free to differ by target**:
arm64 `sdiv` quietly yields 0; x86-64 `idiv` raises `#DE` → SIGFPE. The developer's Mac and the
`macos-latest` runner were green; `ubuntu-latest` crashed. **"All tests pass" had meant "all tests pass on
arm64" since July**, and six sprints closed on that evidence.

**Fix.** Guard the single-key case explicitly. Per user ruling **R1**, a single key takes the **midpoint**,
not `lo`: `rebalancedKeys(1)` → `"H"` (`(lo + hi) / 2 == 17`, the generator alphabet being 0-indexed), with
`keyBefore("H") == "8"` and `keyAfter("H") == "Q"` — room on both sides.

**Verification.** Per ruling **R2**, a green arm64 run was explicitly **not** accepted as evidence. CI run
[`31975883684`](https://github.com/justgus/Scrivi/actions/runs/31975883684): test #172 passes on
**`ubuntu-latest` 523/523** and **`macos-latest` 516/516** — the first green ScriviCore CI since 2026-07-30.

**Files:** `ScriviCore/src/util/OrderKey.cpp`.

⚠️ **Consequence recorded for SP-100.** SP-093, SP-095, SP-096, SP-097, SP-098 and SP-099 all closed while
ScriviCore CI was failing; their "ctest N/N green" figures are **local arm64 results**. This does not
invalidate the work — the CI log showed **522/523 passing**, a single crashing test, not a rotten suite — but
re-running those suites on both architectures is the only way to know nothing else was hiding behind the
crash.

---

## I-0122 — ⚠️ `stack-use-after-scope`: a test iterated a destroyed temporary `Result`

**Severity:** Medium · **Sprint:** SP-106 · **Epic:** EP-031
**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**

**How it was found.** ⚠️ **By SP-106's own sanitizer leg (T-0413), on its very first CI run** — hours after
that leg was added, in the same sprint. The test had been passing green for weeks while reading freed stack
memory.

**Defect.** `Result::value()` returns a reference **into** the `Result`. Iterating
`listScenesByOrder(...).value()` directly leaves the loop walking a destroyed temporary: C++ lifetime
extension covers the temporary bound *directly* to the range variable, **not** the `Result` that owns the
vector behind it.

⚠️ **Same architecture-dependent invisibility as I-0121, one layer up.** The test passes on arm64 **both
before and after** the fix — the freed bytes happen to survive there. Only the **x86-64 sanitized** leg could
distinguish fixed from broken. Two defects in two days whose visibility depended on the host.

**Fix.** Bind the `Result` to a named variable before iterating its `value()`.

**Scope checked, not assumed.** A brace-matched scan of every range-for in `ScriviCore/src` and
`ScriviCore/tests` found **exactly one** iteration over a *temporary* `Result` — this one. The ~20 other
`for (... : xR.value())` sites bind a **named** `Result` and are correct. **Test code only; no shipping code
affected.**

**Verification.** ASan+UBSan **519/519** clean on macOS arm64; sanitized CI legs green.

**Files:** `ScriviCore/tests/` — `SceneSplitRepro.cpp`.

---

## I-0130

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)** — the user re-tested the Locate… repair
live and confirms the main warning now clears without a scene change.
**Severity:** Medium
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **"Locate…" repaired the world but the project-wide warning kept saying it was missing until the scene changed.** Reported by the user 2026-08-17 during the live `missing`-branch test: *"I click 'Locate' and point it at the new file name. The Scene Inspector warnings go away, but the main warning does not until I change the scene."* **Root cause — the exact INVERSE of [[I-0128]].** `WorldsView.relinkWorld` ended with `load()`, which refreshes **only the Worlds sheet's own list**; it never touched `session.worldWarning`. The inspector cards recovered because I-0128's `bumpWorldRevision()` fires on sheet *dismissal*, but the warning strip is refreshed only by `AppEnvironment.reconnectWorlds()`, which relink never called. ⚠️ **All four mutating actions in the sheet had the same gap** — `createWorld`, `bindExistingWorld`, `relinkWorld`, `removeWorld` — each refreshed the panel and nothing else, so any of them could leave the app-wide surfaces stale. **Fix:** the injected closure is renamed `onReconnect` → **`onWorldsChanged`** (it is not only about reconnecting) and is now called by **all four** actions plus the Reconnect button, routing every world mutation through `reconnectWorlds()`, which re-acquires access, reloads the warning, **and** bumps the card revision. Patching the four call sites individually was rejected as the restated-list trap in another costume.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

*I-0121/I-0122 archived 2026-08-17 at the SP-106 close; **I-0130 archived the same day at verification**,
from the active SP-102. Sprint records:
[`../../Sprints/Closed/Sprint-SP-106.md`](../../Sprints/Closed/Sprint-SP-106.md) and
[`../../Sprints/Sprint-active.md`](../../Sprints/Sprint-active.md).*

## I-0123

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** **High**
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **⚠️ A world that reappears while Scrivi is running is never re-activated, so the app stays blind to it until relaunch.** Reported by the user 2026-08-17 during T-0415 verification, with the correct diagnosis: *"I think the first finding is related to the fact that the world had never been loaded into the project during app launch."* **Confirmed.** `activateWorlds` (`AppEnvironment.swift:319-325`) is called from **`loadProject` only** — i.e. once, at project open. It re-acquires the security-scoped grant for every bound world via `WorldBookmarkStore.activateAll`. If the volume is absent at open, no grant is held; when the drive returns, **nothing calls `activateAll` again**, so the package stays unreadable to the sandbox even though it is physically present. ⚠️ **The bookmark itself is fine** — `WorldBookmarkStore.activate` is idempotent and self-healing (it even refreshes a stale bookmark in place, `WorldBookmarkStore.swift:86-89`). The capability to recover exists and is simply never invoked. **This is the same shape as I-0117 and SP-099's R4 finding: the capability shipped, the trigger did not.** **Consequences observed, all one cause:** links stayed pending after reattach; "Add Character" refused with no world available; "New Character" saw the world but errored on Create; a relaunch fixed everything. **Fix (user-proposed, accepted):** a **"Refresh"/"Reconnect Worlds" action in the Worlds menu** as the minimal intervention, plus an automatic re-activation attempt on app-foreground — the same trigger `WorldWarningView` already uses (T-0389), so a reappearing drive heals without the writer knowing the mechanism.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

## I-0124

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** **High**
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **⚠️ While a world is unavailable, EVERY pending object appears on EVERY world-scoped card — locations and chronicles listed under Characters.** Reported by the user 2026-08-17: *"Each card (Characters, Locations, and Chronicles) showed every object assigned to that Scene (i.e. it was not filtered correctly)."* **Root cause — `ObjectCard.swift:161-164` (SP-099/T-0386, predates SP-102):** <br>`let known = ofKind.contains(edge.otherID)` <br>`guard known \|\| (edge.otherPending && cardKind.isWorldScoped) else { return nil }` <br>The kind filter is `ofKind`, built from `listObjects(kind:)` — but an object in an **unavailable** world is absent from that listing, so `known` is false for all of them. The pending fallback then admits the edge whenever **the card** is world-scoped. ⚠️ **`cardKind.isWorldScoped` is a property of the CARD, not of the OBJECT** — there is no kind comparison on that branch at all, so every pending edge lands on all ten cards. The fallback is correct in intent (§7.2 forbids hiding pending objects) and simply omits the kind test. **The data is not wrong — only the presentation is**, which matters because to a writer it looks exactly like her world has been scrambled. **Root cause of the root cause:** ⚠️ **the C ABI does not serialize the far endpoint's kind.** `ResolvedEndpoint` carries `kind` and the core uses it for relation-type constraints (`RelationshipStore.cpp:214`), and the pending path fills it from the binding cache (`EndpointResolver.cpp:69`) — but `scrivi_list_edges_for` emits `otherID`/`otherDisplayName`/`otherPending`/`otherWorldID`/`otherWorldStatus` and **not `otherKind`** (`scrivi_c_api.cpp:886-894`). Swift therefore *cannot* filter pending edges by kind with the payload it is given. **This is the I-0113 shape for the seventh time: the boundary drops what the core already knows.** **Fix:** emit `otherKind` from `scrivi_list_edges_for` (additive), decode it in `EdgeView`, and make the pending branch compare it to `cardKind.kind`.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

## I-0125

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** **High**
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]`/`[ScriviCore]` **⚠️ Creating any object other than a character or location fails at the EDGE step — the object is written to disk and indexed, then orphaned, and the writer is told creation failed.** Reported by the user 2026-08-17: *"The Chronicle Create errored out (ScriviError 1), however, the file was created on disk this time."* **Confirmed on the user's world: all three chronicles exist in `chronicles/` AND in `index.json`, with no edge to any scene.** **Root cause:** `ObjectCardModel.createAndRelate` (`ObjectCard.swift:239-252`) performs **two non-atomic calls** — `createObject` then `createEdge`. The object write succeeds; the edge is rejected by the relation-type kind constraint and throws, so the `catch` reports failure for an operation that **half-succeeded**. The constraint is real: `appears-in` is declared `sourceKind: "character"` → `targetKind: "scene"` (`relation-types.json`), and `RelationshipStore.cpp:207-214` enforces it. ⚠️ **But `ObjectCardKind.all` gives `preferredRelationType: "appears-in"` to EIGHT of the ten kinds** (`ObjectCard.swift:56-90`) — chronicle, building, vehicle, item, map, artifact, faction, rule — so **only `character` (appears-in) and `location` (located-at) can ever be created from a card.** The other eight are broken for in-card creation and always have been; it surfaced now only because the user exercised Chronicles. **Two defects, and both need fixing:** **(a)** the relation vocabulary does not cover eight kinds — either widen `appears-in`'s constraints or seed per-kind types (Doc 1 §5.1 decision, needs a ruling); **(b)** ⚠️ **the half-success must not be reported as a failure** — a created-then-unlinked object silently becomes an orphan while the writer believes nothing happened, then reappears in the picker, which is exactly what the user saw. Roll back the object on edge failure, or report it honestly as "created but not linked".

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

## I-0126

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** Low
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **A card's error message persists after the writer dismisses the draft, and survives until the scene changes.** Reported by the user 2026-08-17: *"the warning message in the card persisted after I clicked 'Discard', subsequently clicking 'Add Chronicle' allowed me to select the new entry while the warning message persisted."* `commitError` (and `ObjectCardModel.loadError`) are never cleared on discard or on a subsequent successful action, so a stale failure notice sits on the card contradicting what the writer can plainly see working. Cosmetic relative to [[I-0125]], but it is the surface that made that defect confusing to diagnose: the writer could not tell which message described the current state. **Fix:** clear `commitError` on discard, on a successful commit, and on reload.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

## I-0127

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** Medium
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **The "Add \<Kind>…" picker popover is too short to use — it shows a fraction of one row.** Reported by the user 2026-08-17: *"The Popup that displays when I click 'Add Chronicle' (or 'Add *' to add any existing type) is too short. It only displays part of one row from all the objects of that kind to choose from."* **Root cause:** `ObjectPickerView` sized its list with `.frame(maxHeight: 240)` — a **maximum only**. A `ScrollView` has no intrinsic height, and inside a `.popover` (which sizes to its content rather than being handed a height) it collapses to near-zero instead of expanding, so the writer sees a sliver and must scroll blind through a list she cannot survey. ⚠️ **This defeats the point of AC17's deliberately unfiltered picker** — the writer is shown *everything* precisely so she can choose, and she could not see it. **Fix:** `.frame(minHeight: 180, maxHeight: 320)` — the **minimum** is what reserves the space; the maximum still caps a large world. The empty/no-match branch gets the same minimum so the popover does not resize on every keystroke, and the popover widens 280 → 320 pt so world-grouped names are not truncated.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

## I-0128

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** Medium
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **Scene Inspector cards did not refresh when a world reconnected — the writer had to change scenes.** Reported by the user 2026-08-17 in the T-0415 re-run: *"The main warning recovered and the objects re-linked, but the Scene Inspector Cards did not refresh until the Scene was changed."* **Root cause:** `ObjectCardBody` reloaded on `.task(id: context.sceneID)` — scene identity **only**. Reconnecting a drive changes world availability, not the sceneID, so nothing invalidated the card and it kept rendering the pending entries it had loaded while the world was away. ⚠️ **This is [[I-0105]] one layer over**, and the same shape exactly: there a history commit left the card stale because the mutation happened inside `HistoryCapture`, which the card could not observe; here it is world availability. **The project-level warning strip recovered correctly** (it reloads on foreground), which is what made the stale cards look like a data problem rather than a refresh problem. **Fix:** a monotonic `worldRevision` on `ProjectSession`, bumped by `AppEnvironment.reconnectWorlds()` and on dismissal of the Worlds sheet (which covers add/remove/relink/create), threaded through `SceneInspectorView` → `InspectorCardStackView` → `CardContext`, with `ObjectCardBody` keying `.task(id:)` on `"sceneID#worldRevision"` — mirroring `historyRevision` rather than inventing a second mechanism.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---

## I-0129

**Status:** ✅ **Resolved - Verified (2026-08-17, user-approved)**
**Severity:** Medium
**Sprint:** **SP-102**

**Description / Resolution:**
`[Apple]` **World availability was refreshed on app FOCUS, not on the drive actually mounting — so plugging a drive in while Scrivi was already frontmost left the warning up.** Reported by the user 2026-08-17, who found it by reversing the usual order: *"I returned focus to scrivi, then I plugged in the drive. Focus is already in Scrivi. The drive becomes available, the Warning does not go away."* **Root cause:** `reconnectWorlds()` was driven **only** by `NSApplication.didBecomeActiveNotification` (`EditorView.swift`). That masked the defect completely, because ejecting a drive normally *requires* leaving the app — so focus always changed and the refresh always happened to fire. It is the wrong event: ⚠️ **focus is a proxy for "something may have changed", not the change itself.** The user's diagnosis was exact — *"I think the warning is being refreshed from the function that gets called when you return to an app… but it isn't refreshing when the drive is actually restored"* — and he proposed a periodic timer as the likely recourse. **A timer is not needed:** macOS reports the event directly. **Fix:** observe `NSWorkspace.didMountNotification` and `didUnmountNotification` alongside the existing focus trigger; both call `reconnectWorlds()`, which re-acquires the sandbox grant, reloads the warning, and bumps `worldRevision` (so the cards refresh too, per [[I-0128]]). ✅ **Notification behavior PROVEN, not assumed** — a self-contained probe attached and detached a `hdiutil` image while observing, and captured `MOUNT: /Volumes/…` then `UNMOUNT: /Volumes/…`. **Strictly better than polling:** immediate, exact, and zero idle wakeups.

> *Archived from the `Issue-active.md` table row at verification (2026-08-17).*

---
