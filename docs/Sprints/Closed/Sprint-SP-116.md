# SP-116 — `[Cross]` World assets + `assetPath` + the kind-scope endpoint

✅ **CLOSED 2026-08-21 (user-approved).** EP-034's **second** sprint.
**All eight Tasks ✅ Verified** → [`../Tasks/Verified/Task-verified-0426-0433.md`](../Tasks/Verified/Task-verified-0426-0433.md).
**Six Issues ✅ Verified** → [`../Issues/Verified/Issue-verified-0141-0150.md`](../Issues/Verified/Issue-verified-0141-0150.md).
⚠️ **I-0147 is an ACCEPTED limitation**, deferred to the network-worlds design — not archived.

**Suites at close:** `ctest` **552/552** macOS arm64 · x86-64 · ASan/UBSan (**was 525**) ·
**Linux 556 cases / 9300 assertions** (GCC 13) · interop **107/107** · app **BUILD SUCCEEDED**.

⚠️ **Verified on the real rig**, not only in the lab — see the live-pass section below.

---

## SP-116: `[Cross]` World assets + `assetPath` + the kind-scope endpoint

**Status:** ✅ **CLOSED 2026-08-21 (user-approved)**
**Epic:** [EP-034: `[Cross]` Object Detail & Media](../Epics/Epic-active.md) — its **second** sprint
**Goal:** Close the two backend gaps the Detail Sheet needs before it can exist (**GAP-1** world-scoped
asset storage, **GAP-2** asset→pixels addressing) and **retire the restated-kind-list defect class at its
root** by giving the boundary a kind-scope endpoint Swift can derive from.
**Start Date:** 2026-08-21
**End Date:** 2026-08-21
**Trades implemented:** ✅ **D5** (kind-scope ABI addition) · ✅ **D6** (assets in the world package +
heartbeat) · ✅ **D7** (`assetPath` in `list_assets`, app-side thumbnails)

---

### ⚠️ Scope widened at planning — this Sprint is `[Cross]`, not `[ScriviCore]`

The Epic table lists SP-116 as `[ScriviCore]`. **Two planning rulings widened it, both deliberately:**

1. ⚠️ **D5 reaches into Swift.** Shipping `scrivi_list_object_kinds` and stopping would leave **I-0140
   open with its cure sitting unused on the shelf** — `project_capability_without_surface`, committed by
   the very sprint that cures occurrence *eight* of the sibling defect. **T-0429 therefore adopts the
   endpoint in Swift and deletes `ObjectCard.swift:46`.**
2. ⚠️ **A defect found during planning is fixed here, not deferred** — see I-0143 below.

**No UI work is in scope.** The Swift reach is a *derivation swap* at one property, not a surface.

---

### ⚠️ Found during planning — `scrivi_list_assets` cannot escape its own JSON

Not in the design doc; found reading the code D7 modifies.

```cpp
// scrivi_c_api.cpp:1330-1341 — the assets array is built by string concatenation
arr += "{\"assetID\":\"" + a.assetID + "\","
       "\"filename\":\"" + a.filename + "\","      // ⚠️ a quote in a filename breaks the envelope
       "\"title\":\"" + a.title + "\"}";           // ⚠️ so does one in a user-typed title
```

⚠️ **Every other envelope in the file routes through `JsonDoc`, which escapes.** This one does not, and
`JsonDoc::appendToArray` (`Json.hpp:52`) — the correct primitive — already exists.

> ⚠️ **Why it must be fixed HERE and not later: D7 puts a FILESYSTEM PATH into this array.** A path is the
> value most likely to carry a backslash or a quote. Shipping `assetPath` into an array that cannot escape
> means **shipping the corruption and then fixing it** — the bug becomes reachable in the same sprint that
> would have prevented it. **Filed as I-0143 and fixed by T-0428.**

**User ruling (planning, 2026-08-21):** *file the Issue **and** fix it in SP-116* — filed, per SP-115's
T-0424 precedent, rather than fixed silently.

---

### Assigned Tasks

| ID | Title | Codebase | Priority | Status |
| -- | ----- | -------- | -------- | ------ |
| **T-0426** | **D6** — `worldID` on the asset requests; resolve, refuse, lock, **heartbeat during copy** | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0427** | **D7** — emit `assetPath` from `scrivi_list_assets` | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0428** | ⚠️ **I-0143** — route the `list_assets` array through `JsonDoc` | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0429** | **D5** — `scrivi_list_object_kinds`, **derived** from `kAllStorableKinds` + `objectKindIsWorldScoped()`; Swift adopts it | `[Cross]` | **High** | ✅ **Verified** |
| **T-0430** | ⚠️ **I-0141** — restate `scrivi.h`'s world-scope comment **by reference**, so it cannot rot again | `[ScriviCore]` | Low | ✅ **Verified** |
| **T-0431** | ⚠️ **I-0144** — take `WorldLock` on **every** world-package write path | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0432** | ⚠️ **Streaming / block transfer** + per-block watchdog kick | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0433** | ⚠️ **I-0146** — stale-lock **sweep** of abandoned `*.partial` files | `[ScriviCore]` | **Medium** | ✅ **Verified** |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| **I-0140** | Swift **restates** the world-scope rule (`ObjectCard.swift:46`) — ⚠️ structural: nothing in the ABI exposes scope | **Medium** | 🔴 **Open** → **T-0429** 🟡 |
| **I-0141** | `scrivi.h:97-99` world-scope comment **stale since SP-103** — ⚠️ **occurrence eight** | Low | 🔴 **Open** → **T-0430** 🟡 |
| **I-0143** | ⚠️ `scrivi_list_assets` builds JSON by concatenation, **no escaping** — filed at planning | **Medium** | 🔴 **Open** → **T-0428** 🟡 |
| **I-0144** | ⚠️ **`WorldLock` has NO production caller** — world-package **object** writes are unserialised; a shared world can lose a writer's edits silently | **High** | 🔴 **Open** → **T-0431** 🟡 |
| **I-0146** | ⚠️ **A `.partial` orphan survives a vanished volume** — 459 MB unreclaimable inside a shared world | **Medium** | ✅ **Verified** → **T-0433** |
| **I-0147** | ⚠️ **60 s window where an interrupted world is unwritable and its orphan unreclaimable** | Low | 🟡 **Accepted limitation** — deferred to network-worlds design |
| **I-0145** | ⚠️ **`AssetStore::remove` could strand bytes with no sidecar** — invisible to `list`, unfindable by any later `remove`, unreclaimable | **Medium** | ✅ **Verified** → **T-0426** |

---

### Task detail

#### T-0426 — D6: world-scoped assets

⚠️ **This is parameter threading, not architecture.** `AssetStore::categoryDir` takes a root and joins
`assets/<category>` (`AssetStore.cpp:12-16`) — **it never reads a project manifest.** Pass a world root and
it already works. *(A first draft of the design called this "the single largest piece of backend work"; the
user caught it. **"No caller does X" is not evidence the core cannot do X.**)*

**The pattern to MIRROR, line for line** — `ObjectStore::kindDirFor` (`ObjectStore.cpp:36-69`):
require a `worldID` → `WorldStore::resolve` → refuse when `status != available` with
`detail = "worldUnavailable:" + worldStatusName(status)` → join. ⚠️ **Identical error detail**, so both
surfaces fail identically and the app needs one handler, not two.

- `worldID` onto `ImportAssetRequest` / `ListAssetsRequest` / `RemoveAssetRequest` (`Requests.hpp:131-147`)
  and onto the three `scrivi_*` entry points.
- ⚠️ **Take `WorldLock` for the write** — a world package write is locked like any other.
- ⚠️ **`heartbeat()` DURING the copy, not only at its start.** This is the **first world write whose
  duration depends on file size**; a large map from a slow USB volume can approach `kStaleSeconds = 60`,
  at which point another process is **entitled** to break the lock mid-copy (`WorldStore.hpp:157-174`).
- ⚠️ **`kStaleSeconds` is NOT to be raised** (D6 ruling) — that weakens crash recovery for *every* world
  write to solve a problem local to one.
- ⚠️ **Never leave a sidecar orphaned from its bytes**, and never half-write on an unavailable world.

**Empty `worldID` keeps today's meaning: the project root.** Existing callers are unaffected.

#### T-0427 — D7: `assetPath` from `list_assets`

⚠️ **The core already computes this and already returns it once** — `scrivi_import_asset` emits
`assetPath` (`scrivi_c_api.cpp:1310`). **The path is disclosed at import time and never again.** This is
addressing, not capability. ✅ **No schema change** — `thumbnailAssetID` already exists
(`ObjectTypes.hpp:194`).

⚠️ **No image codecs in ScriviCore** (D7 = D) — a real portability and security surface across four
platforms. Thumbnails are the app's job, in a later sprint.

⚠️ **`ListAssetsResult.assets` is typed `String` in Swift** (`ScriviEngine.swift:1692`) — the app re-parses
it. Adding a field is source-compatible, but **note it for SP-119**, which will want a real model.

#### T-0428 — I-0143: escape the array

Rebuild the loop on `JsonDoc::appendToArray` (`Json.hpp:52`). ⚠️ **Test with a title containing a quote and
a path containing a backslash** — a test that only uses tame ASCII proves nothing here.

#### T-0429 — D5: the kind-scope endpoint, and Swift deriving from it

`scrivi_list_object_kinds` returns each kind with its scope, ⚠️ **derived core-side by ITERATING
`kAllStorableKinds` and CALLING `objectKindIsWorldScoped()`** (`ObjectTypes.hpp:73-85, 166+`).
⚠️ **A hand-written list in the endpoint would be the defect it exists to cure** — occurrence nine, in the
fix for occurrence eight.

Then, in Swift: `ScriviEngine` wrapper → `ObjectCardKind.isWorldScoped` reads it → ⚠️ **delete
`ObjectCard.swift:46`.** Six call sites consume that property today (`ObjectCard.swift:600, 680, 796`;
`ObjectPickerView.swift:121`; and two in tests) — all keep working; only the *source* changes.

⚠️ **`ScriviURL.swift:23` and `ScriviEngineGraph.swift:387` are NOT this defect** — both read
`!worldID.isEmpty`, a property of *an object instance*, not a restatement of the kind partition. **Leave
them alone.**

#### T-0430 — I-0141: the comment that rotted

`scrivi.h:97-99` still names *"artifact / chronicle / faction / rule"* and says the rest are
project-scoped. **SP-103 moved six kinds to world scope on 2026-08-14**, so it names **4 of 10** and its
second sentence is actively wrong — a reader who trusts it passes `""` for a character and gets
`worldRequired`.

⚠️ **Do not correct the list. Delete it.** State the rule **by reference** to `objectKindIsWorldScoped()`
and to T-0429's new endpoint. ⚠️ **A corrected list is a list that will rot again** — that is the entire
lesson of *"a list rots without being edited."*

---

---

### ⚠️ Implementation status — 2026-08-21

**All five Tasks implemented. Suites green on every leg that can run here.**

| Leg | Result |
| --- | ------ |
| `ctest` macOS **arm64** | ✅ **552/552** (was 525 — **+27 new boundary tests**) |
| `ctest` macOS **x86-64** | ✅ **552/552** |
| `ctest` **ASan + UBSan** | ✅ **552/552** — no leaks or UB in the new lock/asset paths |
| macOS **interop** | ✅ **107/107 in 10 suites** (was 103 — **+4**) |
| macOS **app** | ✅ **BUILD SUCCEEDED** |
| **Linux — Qt app build** | ✅ **BUILD SUCCEEDED** — all 226 targets under **GCC 13 / Ubuntu 24.04** |
| **Linux — Qt smoke tests** | ✅ **16/16** (offscreen, via the `tests/*_smoke.sh` wrappers) |
| **Linux — `ctest` under GCC** | ✅ **556 cases / 9300 assertions** — ⚠️ **re-run AFTER the sweep landed**; SP-116's tag ✅ **26 cases / 406 assertions**, sweep ✅ **3 cases / 53 assertions** |

#### ⚠️ Both fixes were proven NON-VACUOUS by reverting them

A passing test is not evidence until it has been seen to fail.

- **I-0143** — restored the concatenating form: tests **367, 368, 369 FAILED**; restored the fix: all pass.
- **I-0140** — changed `isWorldScoped` to a different wrong restatement: **5 test failures across 2 suites**,
  including the new `cardScopeAgreesWithTheCore`; restored: 107/107.

#### ⚠️ Three findings from implementation — none of them was in the plan

1. ⚠️ **`WorldLock` has NO production caller.** It is fully implemented and tested, but `grep` finds it used
   only in `WorldTests.cpp` — **object writes into world packages take no lock today.** T-0426 makes asset
   import and remove the **first** real users. ⚠️ **This is a pre-existing gap far wider than SP-116**
   (every `scrivi_save_object` into a shared world is unserialised) and is **reported, not silently fixed**.
2. ⚠️ **D6's "heartbeat during the copy" is NOT FULLY ACHIEVABLE** as the ruling describes it. `FileSystem`
   exposes only whole-file `readTextFile`/`atomicWriteTextFile` (`Services.hpp:43-45`) — there is **no
   chunked or streaming primitive to interleave a heartbeat with**, so a single 500 MB write is one
   blocking call. Implemented as **heartbeat immediately before each of the two writes**, which bounds the
   exposure to one write rather than the whole operation. `kStaleSeconds` was **not** raised, per D6.
   ✅ **RULED 2026-08-21 (user): move to a streaming / block-transfer model — see below.**
3. ⚠️ **The rollback I first wrote could DESTROY a healthy asset.** `atomicWriteTextFile` is
   temp-write→rename, so re-importing a filename **overwrites**; rolling back a failed sidecar write would
   then delete bytes the *earlier* asset's sidecar still points at. Now guarded by `destExisted` — the
   rollback removes only what the failing call actually created.

#### ✅ RULED 2026-08-21 — streaming / block transfer, and what it unlocks

> **User:** *"move to a streaming or block transfer model for large assets. Then as each packet of data
> comes in we can 'kick the watchdog' to prevent lock timeout. On partial load, or dropped connection the
> timeout will fire and that can trigger the cleanup. This will also allow us to support worlds that are
> located on the network via URL."*

⚠️ **This is a larger and better idea than the finding that prompted it.** I reported a heartbeat that
could not reach inside a blocking write. The ruling inverts the framing: **the blocking whole-file write is
itself the thing to replace**, and three problems collapse into one design.

| What it solves | How |
| -------------- | --- |
| **The heartbeat gap** (finding #2) | The watchdog is kicked per block, so lock liveness tracks *progress*, not wall-clock guesses. A write of any size is safe. |
| ⚠️ **Cleanup of a half-written asset** | **The lock timeout becomes the cleanup trigger.** A dropped transfer stops kicking, the lock goes stale, and the next writer breaks it and sweeps the partial. ⚠️ **This is strictly better than my `destExisted` rollback**, which only covers failures the *writing process survives* — it cannot clean up after a crash or a yanked drive. |
| ⚠️ **Network-hosted worlds** | A block-transfer path has no reason to assume a local filesystem. ⚠️ **This is the part with the longest reach and it is NOT free** — see below. |

⚠️ **Sizing, checked against the code rather than estimated:**

- ✅ **One implementor** of `FileSystem` (`LocalFileSystem`), and **no FileSystem mocks at all** — tests run
  against the real one in temp dirs. A new virtual method costs one implementation, not N.
- ✅ **206 call sites** use `readTextFile`/`atomicWriteTextFile`. ⚠️ **The streaming API must therefore be
  ADDITIVE.** Changing the existing two primitives would touch all 206 for no benefit — every one of them
  writes a small JSON document, which is exactly the case whole-file writes suit.
- ⚠️ **Network worlds are a NEW CAPABILITY, not an extension.** `AbsolutePath` is a bare string and
  **nothing in ScriviCore has any notion of a URL** (`grep`: no scheme handling in `WorldStore`). Resolution,
  availability (`WorldStatus`), the lock's `createFileExclusive` exclusivity guarantee, and the sandbox's
  security-scoped bookmarks **all currently assume a mounted local path.** ⚠️ **`createFileExclusive` is
  the load-bearing one** — "exactly one winner" is an *OS filesystem* guarantee, and it does not survive a
  network transport unchanged.

##### ⚠️ The split, so SP-116 stays finishable

**In SP-116 (T-0432):** the additive streaming primitive on `FileSystem` + `LocalFileSystem`, asset import
rewritten to use it with a per-block heartbeat, and stale-lock cleanup of partial transfers. **This closes
finding #2 properly** and makes the D6 ruling true as written.

⚠️ **NOT in SP-116 — network worlds.** It needs its own design: a URL-bearing path type, what
`WorldStatus` means for an unreachable host, and above all **what replaces `createFileExclusive` when
"exactly one winner" can no longer come from the local OS.** ⚠️ **Answering that inside an asset sprint is
how a locking model gets decided by accident** — and I-0144 is already this Epic's evidence for what an
unexamined locking assumption costs. **Recorded for its own Epic/design pass.**

#### ⚠️ Second self-review pass — two more defects, both in code I had just written

**Continued review found two things the first pass and 536 green tests did not.**

4. ⚠️ **`AssetStore::remove` discarded BOTH `removeFile` results and deleted the sidecar FIRST**
   — ✅ **now filed as I-0145**, because ⚠️ **it was PRE-EXISTING AND SHIPPED** (confirmed at `HEAD`). If the
   sidecar delete succeeded and the binary delete failed, the result was **bytes with no sidecar** — which
   `list` cannot see (it scans for `*.meta.json`) and **no later remove can ever find again**: an
   unreclaimable file inside a shared world, and the exact condition D6 forbids. Now deletes the **binary
   first** (a failure between the two leaves a listable, removable sidecar — a state a writer can act on)
   and **reports** the failure instead of swallowing it. ⚠️ **Pre-existing, but T-0426 is what made it a
   world-package concern.**
5. ⚠️ **`ObjectKindScope` used `Dictionary(uniqueKeysWithValues:)`, which TRAPS on a duplicate key**
   — ⚠️ **deliberately NOT filed as an Issue**: that code was written in this sprint and fixed before
   commit, so it never existed in the product. Filing it would record a defect no build ever had. A
   core that ever emitted a kind twice would **crash the app** — an unacceptable failure mode for the one
   type whose entire purpose is safe degradation. Now folds duplicates toward world-scoped, the same
   conservative direction as its unknown-kind fallback. ⚠️ **The interop test deliberately KEEPS the
   trapping form**, where a trap is a legitimate assertion, and says so.

**Two tests added for cases nothing covered:** the **empty list** (where an array-building bug hides,
because `appendToArray` is never called) and **re-importing the same filename** (which guards finding #3's
`destExisted` fix directly).

> ⚠️ **The filing test, made explicit, since these two split on it:** an Issue records what **the product
> carried**. Defect #4 was in committed code and shipped — it gets I-0145 even though the same sprint fixes
> it, because the record must show it existed. Defect #5 was born and died inside this sprint's working
> tree. **The question is whether it shipped, not whether it was interesting or how it was found.**

#### ⚠️ T-0431 + T-0432 — implemented 2026-08-21, and what they turned up

**T-0431 (I-0144).** Introduced `worlds::WorldWriteGuard` — resolve, lock, release, all in one object, so a
write path gets locking **by construction** rather than by remembering. ⚠️ **An empty `worldID` makes it
inert**, so callers need no `if (world)` branch of their own — that branch is exactly what went missing for
three sprints. Wired into all four `ObjectStore` write paths (`create`, `save`, `remove`, promote/demote)
and the asset paths.

⚠️ **The promote/demote path can touch TWO worlds**, so guards are acquired in **deterministic
(lexicographic) order**. `acquire` fails fast rather than blocking (§4.5), so the worst case is a mutual
refusal rather than a deadlock — but ordering means one process wins outright instead of both losing.

⚠️ **`projectID` is passed empty** into the guard from `ObjectStore`. It is holder metadata for
diagnostics, not part of the exclusivity guarantee (that comes from `createFileExclusive`); threading it
would mean widening three request structs and their ABI entry points for a field no correctness property
reads. **Recorded rather than done, so the omission is deliberate and visible.**

##### ⚠️ One world-package write is STILL unlocked, on purpose

`ObjectIndex::loadWorldIndex` rebuilds and **writes** when `index.json` is missing or unparseable — during
what callers believe is a **read**. It is left unlocked because ⚠️ **`WorldLock` IS NOT REENTRANT**:
`save` and `remove` reach it through `findByID` *while already holding the lock*, so a guard there would
fail against **itself** and silently skip the rebuild in the very paths that most need a correct index.
Tolerable because the rebuild is a full scan and **idempotent** — racing processes write identical bytes,
through an atomic rename, so the worst case is a redundant write. ⚠️ **The real fix is a reentrant world
lock, which is a change to the locking MODEL** and belongs with the network-worlds design that must already
revisit "exactly one winner". **Not invented here.**

##### T-0432 — the streaming model, as ruled

`FileSystem::copyFileInBlocks(from, to, blockSize, onBlock)` — additive, one implementor, **no existing
call site touched**. Asset import now copies in 1 MiB blocks and kicks the watchdog after each, so ⚠️ **lock
liveness tracks PROGRESS rather than a guess about how long a write should take.** A transfer of any size
is now safe, and `kStaleSeconds` stays at 60.

⚠️ **Losing the lock aborts the transfer**: `heartbeat()` fails if another process broke and re-took the
lock, and returning that failure from the callback stops the copy rather than writing into a package this
process no longer owns. The copy runs to a `.partial` temporary and renames only on success — ⚠️ **so an
abort leaves NO destination and NO temporary**, which is the case the `destExisted` rollback could never
cover.

**Tests (S12/S13), all through `scrivi_*` or the real `LocalFileSystem`:** a locked package refuses
`create_object`, `delete_object` and `import_asset`; a **project** write is unaffected by a world lock; the
lock is released after every successful write and no `.lock` remains; a **5 MiB** asset round-trips
**byte-exact including NULs**; the watchdog is kicked **exactly once per block**; an aborted transfer leaves
neither destination nor `.partial`; and the destination never exists mid-flight.

##### ⚠️ Three lock tests FAILED first — and the PRODUCT was right

The competing-writer rig used `FixedClock`, so its lock's `heartbeatAt` read as **over a minute stale**
against the C ABI's real clock. `WorldLock` correctly judged it dead, broke it, and the write succeeded.
⚠️ **That looks exactly like a broken guard and was a broken RIG** — distinguishable only by reading the
envelope rather than the pass/fail. The rig now uses a real clock. **Then proven non-vacuous:** deleting
the `create` guard fails the test.

#### ✅ The Linux leg — run 2026-08-21, and two things it taught

**All three Linux checks pass**, so ⚠️ **S11 is complete**. GCC 13 is a different compiler from Apple
Clang, so this also exercises the portability of the new `copyFileInBlocks` and `WorldWriteGuard` code —
⚠️ **and 22 SP-116 test cases / 345 assertions ran there specifically**, not merely "the suite was green".

⚠️ **The shipping Dockerfile builds with `SCRIVI_BUILD_TESTS=OFF`** (`Dockerfile:50`), so it proves the Qt
app links and **never executes one ScriviCore test on Linux**. A second image was needed to actually run
`ctest` under GCC. **The standing "Linux container green" criterion is weaker than it sounds** — worth
knowing before another sprint reads a green container as a green suite.

##### ⚠️ Two failures that were NOT defects — both mine, in how I ran things

1. ⚠️ **`ctest` as root failed one WorldTests case** — *"a PRESENT but UNREADABLE package is never reported
   as 'missing'"*. The container runs as **uid 0**, and **root reads a `chmod 000` directory anyway**, so
   the test's premise never held and the world resolved as `available`. ✅ **Proven environmental**: the
   same binary in the same image **passes as `--user 1000:1000`**. ⚠️ **Not a Linux defect and not
   SP-116's** — but a real caveat for anyone running this suite in a container as root.
2. ⚠️ **13 "smoke test failures" that were an invocation error.** The harness binaries require a project
   directory argument; run bare they print `usage:` and exit non-zero. Driven properly through their
   `tests/*_smoke.sh` wrappers: **16/16 pass.** ⚠️ **I reported the number before diagnosing it** — a count
   of red is not a finding until you have read one.

#### ⚠️ THE LIVE RIG PASS — 2026-08-21, and the defect it found

**Run against the REAL `Eskandar.scrivworld` on the REAL USB volume, through the REAL C ABI** — not a
hand-placed file. Hand-copying a PNG would have tested the filesystem; only calling `scrivi_import_asset`
exercises routing, locking, the watchdog and the sidecar, and it is the same call SP-117's UI will make.

| Criterion | Result on hardware |
| --------- | ------------------ |
| **S1** | ✅ A 1024×1024 portrait landed in `Eskandar.scrivworld/assets/images/`, **not** the project. SHA-256 identical, still a valid PNG after the multi-block copy |
| **S2** | ✅ ⚠️ **The criterion D6 exists for.** A **separate project** on the same removable volume listed it with a resolving path — the image travelled with the world |
| **S3** | ✅ Drive **physically pulled** → refused `worldUnavailable:unavailable`, the honest generic status, **never** a guessed `missing` (§4.6), and named the world |
| **S4** (duration) | ✅ 3.8–3.9 GB in **99–102 s**, three times. ⚠️ **A lock watcher recorded 102 DISTINCT heartbeats, one per second** — without them the lock goes stale at 60 s, ~40 s before the write ends. **The watchdog is load-bearing, not decorative** |
| **S4** (abort) | ✅ Drive pulled at ~24 s → aborted, **no destination file**, **existing assets byte-identical**, stale lock breakable after 60 s |
| **S14** | ⚠️ **PARTIAL.** ✅ The sweep reclaims an orphan on lock acquire (verified on hardware, 476 MiB → 12 MiB, and again after a stale lock aged out). ⚠️ **But a real crash leaves a FRESH lock too**, so for up to 60 s no write can acquire and the orphan is retained — **I-0147, accepted**. |

⚠️ **The abort fired through the HEARTBEAT, not the byte copy** — `could not open temp file for writing …
/.lock.tmp`. The per-block kick proved to be both the liveness signal **and** the failure detector, a mode
the design comment had not anticipated.

##### ⚠️ I-0146 — the defect only real hardware produced

The pulled drive left **459 MB of `myton-huge.png.partial`** inside the shared world. `copyFileInBlocks`
removes its temporary on failure — ⚠️ **but that cleanup cannot run when the failure IS the volume going
away.** `list_assets` scans for `*.meta.json`, so nothing in Scrivi could ever see or reclaim it.

⚠️ **The lab test "an aborted transfer leaves NO destination and NO partial" PASSES and always did** — in a
lab the filesystem is still there to clean up with. ⚠️ **It could only ever test failures the writing
process survives.** Same blind spot as I-0145, and the **third** time this Epic produced a defect only live
use found (I-0137, I-0142, I-0146).

##### T-0433 — the sweep, as ruled

`WorldLock::sweepAbandonedPartials()` removes `*.partial` under `<package>/assets/<category>/`. Safe by
construction: a partial is only ever written by a lock **holder**, so if we hold the lock no live writer
owns one — an abandoned partial can never be resumed, only deleted.

⚠️ **Swept on EVERY successful acquire, not only after breaking a stale lock.** The ruling named the
timeout as the trigger, and that is the main path — but the rig showed **the lock file and the partial are
orphaned together**. When the lock is simply gone the next writer acquires cleanly through
`createFileExclusive` and never reaches a break path, so a break-only sweep would have missed the very case
the Issue was filed for.

⚠️ **Categories derive from `assetCategorySubdir`**, never written out, so a new category cannot go
silently unswept. ⚠️ **One test guards OVER-REACH specifically** — `partial-eclipse.png` must survive,
because a sweep deleting real images would be far worse than the junk it cleans.

#### ⚠️ THE TIDY END-TO-END RUN — and the limitation it exposed

**The single continuous pass (pull → orphan → reattach → sweep) FAILED at the sweep**, and it was right to.

Step 5 was refused **`worldLocked`**, so the sweep never ran and a **2.9 GB `.partial`** survived. Cause:
the vanished volume killed the writer **while it held the lock**, leaving `.lock` on disk with a **fresh**
heartbeat. The drive came back within ~60 s, so the lock was **not yet stale**, `acquire` correctly refused,
and ⚠️ **T-0433's sweep only runs AFTER a successful acquire.**

✅ **Both halves verified rather than reasoned:** a staged fresh lock + orphan reproduces `worldLocked` with
the orphan retained; waiting past 60 s and retrying **acquires and sweeps**. **The sweep is correct; the
window is the gap.**

> ✅ **RULED 2026-08-21 (user, option 1): ACCEPT as a known limitation → I-0147.**
> A 60 s lockout after abnormal termination is what `kStaleSeconds` is *for* — the core cannot distinguish
> "writer died" from "writer briefly stalled", and guessing wrong means two processes writing a shared
> world at once. It self-heals and loses nothing. ⚠️ **The stronger evidence — the package's own VOLUME was
> unmounted — is not currently used**, and using it is a change to the locking model, **deferred to the
> network-worlds design** that must revisit "exactly one winner" anyway. ⚠️ **The UI must never present the
> 60 s wait as an error**; it is retryable.

##### ⚠️ …and the regression test for I-0147 got it wrong AGAIN, the same way

Writing the test that ENCODES the accepted limitation, I staged the orphan **before** taking the lock — and
it failed, because ⚠️ **acquiring a lock now runs the sweep**, so the act of staging the lock reclaimed the
orphan. A real crash produces the opposite order naturally: hold the lock, write the partial, die.

⚠️ **Third time in one sprint that a setup was easier than reality**, and it surfaced something worth
knowing: **the sweep fires on EVERY acquire, including a competing writer's** — so an orphan is reclaimed by
whichever process next takes the lock, not only by the one that later succeeds.

##### ⚠️ My earlier staged-orphan test was weaker than I claimed it was

The 476 MiB → 12 MiB result **was real**, but it created the orphan **without a matching fresh lock** —
⚠️ **not the state a real crash leaves.** The tidy run produced the genuine state and broke immediately.

⚠️ **Fourth defect this Epic found only by live use** (I-0137, I-0142, I-0146, I-0147), and **the second
time my own setup was subtly easier than reality.** The lesson is narrower than "test on hardware": ⚠️ **a
test that stages the AFTERMATH of a failure is not the same as one that stages the FAILURE** — it silently
omits whatever else the failure leaves behind, which here was the very thing that blocked the fix.

#### Shape changes worth knowing

- **`ListAssetsResult` now holds `ListedAsset { meta, assetPath }`**, not bare `AssetMeta`. ⚠️ **`assetPath`
  is deliberately NOT on `AssetMeta`**: that struct is the on-disk `.meta.json` schema, and an absolute
  path baked into a sidecar is wrong the moment the package moves or is opened on another machine.
- **Swift `ListAssetsResult.assets` is `[ListedAsset]`**, no longer a `String` holding JSON. ⚠️ **No app
  code read it**, which is precisely why the escaping defect survived — the interop test asserted only
  `count`. It now looks inside.
- **New file `Scrivi/Engine/ObjectKindScope.swift`** — the derived scope table. ✅ **`project.pbxproj`
  updated in the same step** (3 build-file entries + file ref + group + 3 Sources phases).
  ⚠️ Its unknown-kind fallback returns **`true`** (world-scoped) on purpose: that fails loudly and
  recoverably, where guessing project-scope would silently pass an empty `worldID` and reproduce the
  SP-104 outage. **It names no kinds, so it is not a restatement.**

### Success criteria

Every one is checkable, and ⚠️ **each against its own trigger** (`feedback_verify_each_half_separately`).

| # | Criterion |
| - | --------- |
| **S1** | Importing an image for a **world-scoped** object writes it under `<world>/assets/<category>/`, **not** the project. |
| **S2** | ⚠️ Opening that world **from a different project** lists the same asset with a resolvable `assetPath`. **This is what D6 exists for** — S1 alone does not prove it. |
| **S3** | Importing to an **unavailable** world fails with `worldUnavailable:<status>` — ⚠️ **byte-identical to `ObjectStore`'s detail** — and writes **nothing**: no bytes, no orphaned sidecar. |
| **S4** | ⚠️ A copy **longer than `kStaleSeconds`** completes without its lock being broken. **A fast test does not exercise this** — needs an injected slow write or a large fixture. |
| **S5** | `scrivi_list_assets` returns `assetPath` for every asset, project- and world-scoped alike. |
| **S6** | ⚠️ An asset whose **title contains `"`** and whose **path contains `\`** round-trips through `list_assets` as parseable JSON (I-0143). |
| **S7** | `scrivi_list_object_kinds` returns **all eleven** kinds with correct scope, and ⚠️ **a test asserts the endpoint agrees with `objectKindIsWorldScoped()` for every kind** — so adding a kind cannot silently desync it. |
| **S8** | ⚠️ `grep -n "kind != \"source\"" Scrivi/` returns **nothing**; `ObjectCardKind.isWorldScoped` derives from the endpoint (I-0140). |
| **S9** | `scrivi.h`'s Object-CRUD comment **enumerates no kinds** (I-0141). |
| **S10** | ⚠️ **All new boundary tests live against `scrivi_*`, in the `*CApiTests.cpp` pattern — NOT the facade.** See below. |
| **S12** | ⚠️ **I-0144**: a world-package **object** write acquires the lock, and a second writer is refused with `worldLocked` — asserted through `scrivi_*`, on object writes, **not** assets. |
| **S13** | ⚠️ **Streaming**: an asset larger than one block imports byte-for-byte; the watchdog is kicked **per block**; and ⚠️ **a transfer abandoned mid-way is CLEANED UP by the stale-lock path** — no partial file, no orphaned sidecar. **This is the criterion the `destExisted` rollback CANNOT satisfy**, because a rollback cannot run after a crash. |
| **S11** | ✅ **COMPLETE** — arm64, x86-64, sanitizers, interop, the macOS app, **and Linux** (Qt build + 16/16 smokes + 551/551 ctest under GCC) all green. |

⚠️ **S1–S9 are core-and-boundary criteria. None of them proves a writer can DO anything** — no UI ships in
this sprint. That is by design here, but **it is exactly the shape that produced `capability_without_surface`
four times in EP-031**, so it is stated rather than assumed. The surfaces are SP-117–SP-120.

---

### ⚠️ Testing: against `scrivi_*`, never the facade

> **`feedback_boundary_tests_not_facade` — this is how I-0113 shipped green.**

⚠️ **`AssetTests.cpp` (`tests/integration/`) tests the FACADE.** It is the file that would pass while every
change in this sprint is unreachable through the ABI — **the I-0113 failure exactly.**

**The precedent to follow is `ObjectCApiTests.cpp`** — the file created *after* I-0113 for precisely this.
**T-0426, T-0427, T-0428 and T-0429 each need `scrivi_*`-level coverage**, in a new `AssetCApiTests.cpp`
(register in `tests/CMakeLists.txt` alongside line 33). Facade tests may be added; **they do not substitute.**

⚠️ **And per SP-115's closing lesson:** *"a passing test suite named after an acceptance criterion is not
evidence the criterion is met in the product."* **S2 and S4 in particular deserve a real-rig check** — the
`tintagael` + `Eskandar` USB rig — before this sprint is called done. ⚠️ **S2 across two projects on a
removable volume is exactly the shape that hid I-0137 from a green suite.**

---

### Sprint Notes

- ⚠️ **Depends on nothing outstanding.** SP-115 is closed; all its Tasks and Issues are Verified.
- ⚠️ **SP-117 depends on ALL of this.** The Detail Sheet cannot show an image (T-0427), cannot store one
  for a world object (T-0426), and cannot derive its field set (T-0429) until this lands.
- ⚠️ **Backward compatibility:** empty `worldID` = project root, so every existing asset caller and every
  asset already on disk is unaffected. **No migration.**
- ⚠️ **Carried from SP-115 and still unowned:** T-0420 shipped `unsupportedWorldFormatVersion` with **no
  writer-facing surface**. **Not assigned here** — SP-116 ships no UI — but it stays owed, and
  ⚠️ **SP-117 is the first sprint that could own it.**
- ⚠️ **T-0416 remains a possible live blocker for SP-118**, not this sprint. Worth confirming on the real
  rig while it is attached for S2/S4.
- **Pbxproj:** T-0429 adds no new Swift *file* if the wrapper goes in `ScriviEngine.swift`; **if a new
  `.swift` file is created, `project.pbxproj` MUST be updated in the same step** (CLAUDE.md, non-negotiable).
  ⚠️ **New ScriviCore `.cpp` files go in CMake, NOT pbxproj** (`project_pbxproj_scrivicore_scope`).

### Retrospective

*(Filled in at close.)*

---

## Previously: SP-115 ✅ CLOSED 2026-08-20 (user-approved)

EP-034's **first** sprint → [`Closed/Sprint-SP-115.md`](Closed/Sprint-SP-115.md). **All seven Tasks and all
six Issues Verified.** Suites at close: `ctest` **525/525** · macOS interop **103/103 in 10 suites** · app
**BUILD SUCCEEDED**.

### ⚠️ The lesson SP-115 proved twice — it governs how SP-116 is verified

> **User, at close:** *"ctest, and unit tests, and integration tests are designed to test specific things.
> But the true user experience can only be tested live, in app."*

1. ⚠️ **I-0137 — a suite named after the acceptance criterion PASSED while the feature could not fire.**
   `refine` was correct and unit-tested; the datum it needed never arrived. **Only ejecting a real drive
   proved it.**
2. ⚠️ **I-0142 — the user found in five minutes what 628 automated tests did not.**

> ⚠️ **A passing test suite named after an acceptance criterion is not evidence the criterion is met in
> the product.** ⚠️ **This is why S2 and S4 above name the real rig explicitly.**

### ⚠️ Carried out of SP-115 — still true, still owed

| Item | Owner |
| ---- | ----- |
| **T-0420 has no writer-facing surface** — a writer opening a too-new world sees *"unavailable"* with **no explanation**. ⚠️ `capability_without_surface`, shipped by the very sprint that fixed four instances of it | ⚠️ **STILL UNOWNED.** ⚠️ **Not SP-116** — this sprint ships no UI. **SP-117 is the first sprint that could take it.** |
| **I-0140 + I-0141** — filed by T-0424, fixed by neither, by design | ✅ **Now assigned — SP-116, above** |

---

*Last Updated: 2026-08-21, eighth pass (**I-0147 regression test added and ALL LEGS GREEN**: ctest
**552/552** (arm64 · x86-64 · ASan), **Linux 556 cases / 9300 assertions** under GCC, interop **107/107**,
app **BUILD SUCCEEDED**. ⚠️ **The new test asserts a LIMITATION, not a fix** — if someone later makes
`acquire` break fresh locks it fails and forces the locking-model conversation rather than letting it happen
by accident (the lesson of I-0144). ⚠️ **It failed on first write for the SAME reason as the hardware test**
— staged easier than reality — which surfaced that **the sweep fires on every acquire, including a
competing writer's**. Prior note follows.)*

*Last Updated: 2026-08-21, seventh pass (✅ **LINUX RE-RUN GREEN AFTER THE SWEEP** — 555 cases / 9284
assertions under GCC, SP-116's tag 26/406, sweep 3/53. ⚠️ **The TIDY end-to-end run FAILED at the sweep and
was right to**: a vanished volume leaves the dead writer's lock with a FRESH heartbeat, so a quick reattach
is refused `worldLocked` and the orphan is retained until the lock ages out. ✅ **User ruled option 1 —
ACCEPT** → **I-0147**, deferred to the network-worlds design. ⚠️ **S14 is therefore PARTIAL, not met.**
⚠️ **My earlier staged-orphan test was weaker than claimed** — it omitted the matching fresh lock, i.e.
staged the AFTERMATH rather than the FAILURE. **Fourth defect this Epic found only by live use.** Rig
restored: portrait only, `myton.json` unchanged, 2.9 GB reclaimed. Prior note follows.)*

*Last Updated: 2026-08-21, sixth pass (⚠️ **LIVE RIG PASS RUN — and it found a real defect.** **S1, S2,
S3 and S4 all pass on the real USB volume**, including **102 recorded heartbeats** proving the watchdog is
load-bearing, and an abort that fired through the heartbeat itself. ⚠️ **I-0146 FILED AND FIXED**: a pulled
drive left a **459 MB `.partial` orphan** no Scrivi operation could ever reclaim — the rollback cannot run
when the failure is the filesystem vanishing. **T-0433** adds the user-specified stale-lock sweep;
⚠️ **verified on hardware, 476 MiB → 12 MiB**. ⚠️ **The lab test asserting "no partial remains" passed
throughout** — it can only test failures the writing process survives. Tests **551/551** (arm64 · x86-64 ·
ASan), interop **107/107**, app **BUILD SUCCEEDED**. ⚠️ **Linux NEEDS A RE-RUN** — its green predates the
sweep; Docker has since stopped. Tasks 7 → **8**. Prior note follows.)*

*Last Updated: 2026-08-21, fifth pass (✅ **THE LINUX LEG RAN — S11 IS COMPLETE.** Qt app **BUILD
SUCCEEDED** under GCC 13/Ubuntu 24.04; **16/16** Qt smoke tests; **551/551** ctest under GCC (9223
assertions), of which SP-116's own tag is **22 cases / 345 assertions**. ⚠️ **The shipping Dockerfile builds
`SCRIVI_BUILD_TESTS=OFF`**, so "Linux container green" never meant the suite ran there — a second image was
needed. ⚠️ **Two red results were MINE, not defects**: ctest-as-root fails a permissions test because root
ignores `chmod 000` (passes as uid 1000, same binary), and 13 smoke "failures" were harnesses invoked
without their required argument. **Every leg of S11 is now green.** Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (**T-0431 + T-0432 IMPLEMENTED — all seven Tasks now 🟠 Not
Verified.** `ctest` **547/547** on arm64, x86-64 **and** ASan/UBSan (was 525 at sprint start); interop
**107/107**; app **BUILD SUCCEEDED**. ⚠️ **`WorldWriteGuard` makes locking structural** — inert for project
writes, so there is no branch to forget. ⚠️ **Block transfer lands the ruled model**: watchdog kicked per
block, abort on lost lock, `.partial` temp renamed only on success. ⚠️ **One world write stays unlocked
DELIBERATELY** — the index rebuild, because `WorldLock` is not reentrant and `save`/`remove` reach it while
holding the lock; the fix is a reentrant lock and belongs with the network-worlds design. ⚠️ **Three lock
tests failed on a bad RIG while the product was correct** — a FixedClock heartbeat read as stale. **S11
still incomplete: Linux container not run (Docker unavailable).** Prior note follows.)*

*Last Updated: 2026-08-21, third pass (✅ **I-0144 ASSIGNED here** (**T-0431**) and ⚠️ **the streaming
model RULED** (**T-0432**) — both by user. ⚠️ **The streaming ruling reframes my finding**: I reported a
heartbeat that could not reach inside a blocking write; the ruling replaces the blocking write, which also
makes **the stale lock the cleanup trigger for partial transfers** — strictly better than the `destExisted`
rollback, which cannot run after a crash. ⚠️ **Network-hosted worlds are explicitly OUT of SP-116**: they
need a "exactly one winner" guarantee that `createFileExclusive` cannot give over a network, and deciding
that inside an asset sprint is how a locking model gets set by accident. Two new success criteria **S12,
S13**. Tasks 5 → **7**. Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**ALL FIVE TASKS IMPLEMENTED — 🟠 Not Verified.** ctest **536/536**
on arm64, x86-64 **and** ASan/UBSan; interop **107/107**; app **BUILD SUCCEEDED**. ⚠️ **Both fixes proven
non-vacuous by reverting them.** ⚠️ **S11 INCOMPLETE — the Linux container leg could not run (Docker
daemon unavailable).** ⚠️ **Three findings reported rather than silently absorbed**: `WorldLock` has no
production caller at all, D6's in-copy heartbeat is not achievable without a streaming FileSystem
primitive, and the first rollback I wrote could have deleted a healthy asset's bytes on re-import.
⚠️ **Claude cannot mark anything Verified** — S1–S9 are core/boundary only and **no UI ships**, so a live
writer-facing check is neither possible nor claimed. Prior note follows.)*

*Last Updated: 2026-08-21 (**SP-116 🟡 ACTIVATED** — EP-034's second sprint; five Tasks **T-0426–T-0430**,
three Issues **I-0140, I-0141, I-0143**. Plan moved verbatim from `Sprint-backlog.md`, which left it in the
same step. ⚠️ **`[Cross]`, not `[ScriviCore]`** — D5's endpoint is adopted in Swift here so **I-0140 is
cured, not merely made curable**. ⚠️ **No UI ships**, so **T-0420's surface stays unowned and SP-117 is the
first sprint that could take it**. ⚠️ **New coverage belongs in `AssetCApiTests.cpp` against `scrivi_*`** —
`AssetTests.cpp` is a facade suite and would pass while every change here is unreachable through the ABI,
which is the I-0113 failure exactly. Next available: Sprint **SP-117** · Task **T-0431** · Issue
**I-0144**.)*
