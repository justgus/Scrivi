# Verified Tasks: T-0426 – T-0433

**SP-116** — `[Cross]` World assets + `assetPath` + the kind-scope endpoint (EP-034).
✅ **All eight Verified 2026-08-21 (user-approved)** and archived in the same step.

| ID | Title | Codebase | Priority | Status |
| -- | ----- | -------- | -------- | ------ |
| **T-0426** | **D6** — `worldID` on the asset requests; resolve, refuse, lock, ⚠️ **heartbeat during copy** | SP-116 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0427** | **D7** — emit `assetPath` from `scrivi_list_assets` | SP-116 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0428** | ⚠️ **I-0143** — route the `list_assets` array through `JsonDoc` (no escaping today) | SP-116 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0429** | **D5** — `scrivi_list_object_kinds` **derived**; ⚠️ **Swift adopts it, `ObjectCard.swift:46` deleted** | SP-116 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0430** | ⚠️ **I-0141** — `scrivi.h` states the world-scope rule **by reference**, so it cannot rot again | SP-116 | Low | ✅ **Verified (2026-08-21)** |
| **T-0431** | ⚠️ **I-0144** — take `WorldLock` on **every** world-package write path, not just assets | SP-116 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0432** | ⚠️ **Streaming/block transfer** for asset bytes + per-block watchdog kick + **stale-lock cleanup of partials** | SP-116 | **High** | ✅ **Verified (2026-08-21)** |
| **T-0433** | ⚠️ **I-0146** — stale-lock **sweep** of abandoned `*.partial` files | SP-116 | **Medium** | ✅ **Verified (2026-08-21)** |

---

## What SP-116 delivered

**Three design trades (D5, D6, D7), four Issues fixed, and two more found while fixing them.**

| Task | Trade / Issue | Outcome |
| ---- | ------------- | ------- |
| T-0426 | **D6** | World-scoped assets live in the `.scrivworld` package and **travel with the world**. Mirrors `ObjectStore::kindDirFor` including the `worldUnavailable:<status>` detail, so both surfaces fail identically |
| T-0427 | **D7** | `assetPath` emitted from `list_assets`. ⚠️ The core had computed it all along and disclosed it **only at import time** — addressing, not capability |
| T-0428 | **I-0143** | The assets array routes through `JsonDoc`. ⚠️ It concatenated raw JSON and escaped nothing; **T-0427 was about to put a filesystem path into it** |
| T-0429 | **D5** | `scrivi_list_object_kinds`, **derived** from `kAllStorableKinds` + `objectKindIsWorldScoped()`, and **adopted in Swift** — `ObjectCard.swift:46`'s restatement deleted |
| T-0430 | **I-0141** | `scrivi.h` states the world-scope rule **by reference**. ⚠️ The list is gone, not corrected — a corrected list rots again |
| T-0431 | **I-0144** | `WorldWriteGuard` locks **every** world-package write path. ⚠️ Inert for project writes, so there is no `if (world)` branch to forget |
| T-0432 | user ruling | Block transfer + **per-block watchdog kick**. ⚠️ Lock liveness now tracks **progress**, not a guess about duration |
| T-0433 | **I-0146** | Stale-lock **sweep** of abandoned `*.partial` files — the cleanup a dead process cannot perform for itself |

### ⚠️ Three findings that were not in the plan

1. ⚠️ **`WorldLock` had NO production caller.** Complete, correct and unit-tested since SP-097 — and never
   acquired. **Every object write into a shared world was unserialised for three sprints.** → I-0144.
2. ⚠️ **D6's "heartbeat during the copy" was not achievable as ruled** — `FileSystem` had no streaming
   primitive to interleave with. The user's response reframed it: **replace the blocking write**, which
   also made the stale lock the cleanup trigger. → T-0432.
3. ⚠️ **A rollback I wrote could have destroyed a healthy asset** — re-importing a filename overwrites, so
   rolling back a failed sidecar write would delete the *earlier* asset's bytes. Caught in self-review.

### ⚠️ What only live use found

**Four defects in this Epic have now been found by use rather than by tests** — I-0137, I-0142, I-0146,
I-0147. Two of them in this sprint.

⚠️ **And three times in SP-116 my own test setup was easier than reality**: the staged orphan omitted the
matching fresh lock (twice), and the competing-writer rig used a `FixedClock` whose heartbeat read as stale.
**The narrow lesson: staging the AFTERMATH of a failure is not staging the FAILURE** — it silently omits
whatever else the failure leaves behind, which here was the very thing that blocked the fix.

### Verification

**Every fix proven NON-VACUOUS** by reverting it and watching tests fail: I-0143 (3 tests), I-0140 (5 tests
across 2 suites), I-0144 (one guard removed), I-0146 (2 tests).

**Suites:** `ctest` **552/552** macOS arm64 · x86-64 · ASan/UBSan (was **525** at sprint start) ·
**Linux 556 cases / 9300 assertions** under GCC 13 · interop **107/107** · app **BUILD SUCCEEDED**.

**On the real rig** (`tintagael` + `Eskandar`, USB): S1 routing · S2 ⚠️ **a world's image reaching a SECOND
project** · S3 honest refusal with the drive out · S4 ⚠️ **102 heartbeats across a 101-second write** ·
S14 orphan reclamation.

### ⚠️ Carried out of SP-116

| Item | Owner |
| ---- | ----- |
| ⚠️ **No UI ships in SP-116.** S1–S14 are core-and-boundary criteria; **AC3 and AC9 cannot close** without the Detail Sheet | **SP-117** |
| **T-0420's missing surface** (`unsupportedWorldFormatVersion` unexplained to the writer) — carried from SP-115, **still unowned** | ⚠️ **SP-117 is the first sprint that could take it** |
| **I-0147** — the 60 s post-crash lock window | **Deferred — network-worlds design** |
| ⚠️ **`ObjectIndex::loadWorldIndex` rebuild stays UNLOCKED** — `WorldLock` is not reentrant and `save`/`remove` reach it while holding the lock | **Network-worlds design** (needs a reentrant lock) |

---

*Last Updated: 2026-08-21 (**T-0426–T-0433 ✅ Verified (user-approved) and archived in the same step** at
SP-116's close.)*
