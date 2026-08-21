# Verified Issues: I-0141 – I-0150

Archived Issues, ✅ **Resolved - Verified** by the user. Batched in decades of ten per
`Issue-GUIDELINES.md`.

⚠️ **I-0143 lives in this file although its number belongs to the previous decade's range** — no: it is
**I-0143–I-0146** that are archived here alongside **I-0141**, all verified together at SP-116's close.
⚠️ **I-0147 is NOT here** — it is a 🟡 **Accepted limitation**, deferred to the network-worlds design, and
remains a live record in `Issue-active.md`.

| ID | Title | Severity | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| I-0142 | `[Cross]` ⚠️ **The object editor's World control never showed the object's own world — and RENAME of a world object was broken outright.** Found by the **user during SP-115 verification**: *"When the Edit Object popup surfaces, the world that the Object belongs in is not selected."* ⚠️ **Root cause spans THREE layers, all gating `worldID` on `pending`:** `EndpointResolver` set `out.worldID` only on its pending branch, returning without it for a reachable world object (`EndpointResolver.cpp:44-49`); `RelationshipStore` copied `otherWorldID` only inside `if (ep.pending())` (`:490-494`); and the C ABI emitted the key only when `otherPending` (`scrivi_c_api.cpp:895`). The app then hardcoded `worldID: ""` when opening the editor (`ObjectCard.swift:396`). ⚠️ **The second consequence is worse than the visible one:** `rename()` passes that `worldID` to `openObject` (`ObjectCard.swift:274-283`), which needs it to locate a world-scoped object's file — **so renaming any world object failed**, and the visible symptom was only the empty picker. ✅ **A world is a property of the OBJECT, not a symptom of its world being away** — that framing error is the whole bug. **Fixed:** `worldID` reported whenever the endpoint has one, at all three layers + the app. Boundary-tested (`[I-0142]`), and the test was confirmed to FAIL against the un-fixed core. | **High** | **SP-115** | ✅ **Resolved - Verified (2026-08-20)** — SP-115 / T-0425 |

| I-0140 | `[Apple]` ⚠️ **Swift RESTATES the world-scope rule instead of deriving it.** `ObjectCard.swift:46` reads `var isWorldScoped: Bool { kind != "source" }` — a hand-written partition of `ObjectKind` in Swift. CLAUDE.md's standing rule calls this **"a defect on sight, even when it is currently correct"**, and it is correct *only* because `source` happens to be the sole project-scoped kind today. ⚠️ **This is the SAME SITE CLASS as SP-104's occurrence, which blocked object creation in the app entirely.** ⚠️ **The cause is STRUCTURAL, not careless:** `grep` over `scrivi.h` shows **no endpoint exposes a kind's scope at all** — there is no `scrivi_list_object_kinds` and no scope field anywhere in the ABI, so **Swift cannot derive what the boundary never tells it.** ✅ **Fix is already RULED — design doc D5:** ScriviCore exposes each kind and whether it is world-scoped, derived from `kAllStorableKinds` + `objectKindIsWorldScoped()`; Swift then derives. ⚠️ **Test against `scrivi_*`, not the facade** (`feedback_boundary_tests_not_facade`) — that is how I-0113 shipped green. | **Medium** | **SP-116** 🟡 (D5) | ✅ **Resolved - Verified (2026-08-21)** — `ObjectCard.swift:46`'s restatement DELETED; scope now derives from `scrivi_list_object_kinds` via `ObjectKindScope`. ⚠️ **Proven non-vacuous**: breaking the derivation fails 5 tests in 2 suites |
| I-0141 | `[ScriviCore]` ⚠️ **`scrivi.h:97-99`'s world-scope list has been STALE since 2026-08-14.** The Object-CRUD header comment still reads *"`worldID` … names the world a WORLD-SCOPED object lives in — artifact / chronicle / faction / rule. Pass "" (or NULL) for the project-scoped kinds, which is every other kind."* ⚠️ **SP-103 moved `character`, `location`, `item`, `building`, `vehicle` and `map` to world scope**, so the comment names **4 of 10** world-scoped kinds and its second sentence is now actively wrong — a reader who trusts it will pass `""` for a character and get `worldRequired`. ⚠️ **This is the documented failure mode exactly** — *"a list rots without being edited"*: nobody touched this comment; a kind's scope changed underneath it. **Occurrence EIGHT** of the restated-kind-list class. **Fix:** state the rule by reference to `objectKindIsWorldScoped()` rather than enumerating kinds, so it cannot rot again. | Low | **SP-116** 🟡 | ✅ **Resolved - Verified (2026-08-21)** — the enumeration is GONE, not corrected; the header now points at `scrivi_list_object_kinds`, so it cannot rot again |
| **I-0143** | `[ScriviCore]` ⚠️ **`scrivi_list_assets` builds its JSON by string concatenation with NO ESCAPING.** `scrivi_c_api.cpp:1330-1341` assembles the `assets` array as `arr += "{\"assetID\":\"" + a.assetID + "\"," ...` for `filename` and `title` too — so **an asset whose title contains a `"` emits a malformed envelope**, and the app's decode fails or silently truncates. ⚠️ **Every other envelope in this file routes through `JsonDoc`, which escapes**; `JsonDoc::appendToArray` (`Json.hpp:52`) — the correct primitive — already exists and is used elsewhere. ⚠️ **Why it is urgent NOW rather than latent:** **T-0427 (D7) adds `assetPath` to this exact array** — a filesystem path is the value most likely to carry a backslash or a quote, so D7 would make the corruption **reachable through ordinary use**. **Fix:** rebuild the loop on `appendToArray`. ⚠️ **Test with a title containing `"` and a path containing `\`** — a test using tame ASCII proves nothing here. | **Medium** | **SP-116** 🟡 (T-0428) | ✅ **Resolved - Verified (2026-08-21)** — the array is built with `JsonDoc::appendToArray` now. ⚠️ **Proven non-vacuous**: restoring the concatenating form fails tests 367/368/369 |
| **I-0144** | `[ScriviCore]` ⚠️ **`WorldLock` HAS NO PRODUCTION CALLER — world package writes are UNSERIALISED.** The class is complete, correct and tested (`WorldStore.cpp:583-700`, `WorldTests.cpp:362-414`), but `grep` over `ScriviCore/src` finds it constructed **only in tests**. ⚠️ **`ObjectStore` never takes it**: `kindDirFor` resolves a world package and hands back a path, and every `scrivi_create_object` / `scrivi_save_object` / `scrivi_delete_object` into a shared world then writes **with no lock at all** — so two projects with the same world bound can interleave writes to the same object file, and the loser's edit is lost with no error. ⚠️ **The lock was built in SP-097 (T-0403) for exactly this and then never wired in** — `project_capability_without_surface` in its purest form: the capability shipped, tested and Verified, and nothing calls it. **SP-116 T-0426 makes asset import/remove the FIRST production users.** **Fix:** take the lock (and heartbeat where the write is size-dependent) on every world-package write path, not just assets. ⚠️ **Deliberately NOT fixed inside SP-116** — it changes every object write into a shared world, which is far wider than an asset sprint and needs its own verification. | **High** | **SP-116** (T-0431) | ✅ **Resolved - Verified (2026-08-21)** — `WorldWriteGuard` wired into all four `ObjectStore` write paths + assets; ⚠️ **proven non-vacuous** (removing the `create` guard fails the test). ⚠️ **ONE world write stays unlocked deliberately** — the index rebuild, because `WorldLock` is not reentrant and `save`/`remove` reach it while holding the lock; it is idempotent, and the real fix is a reentrant lock belonging to the network-worlds design |
| **I-0145** | `[ScriviCore]` ⚠️ **`AssetStore::remove` could strand bytes with NO SIDECAR — permanently unreclaimable.** It deleted the sidecar FIRST and **discarded both `removeFile` results** (`AssetStore.cpp`, pre-SP-116 form: `(void)fs.removeFile(sidecarPath); (void)fs.removeFile(assetPath);`). If the sidecar delete succeeded and the binary delete then failed — a permissions change, a read-only volume, a drive pulled between the two calls — the bytes remained with no `.meta.json`. ⚠️ **`list` scans for `*.meta.json`, so those bytes are INVISIBLE; and `remove` finds assets only by scanning sidecars, so NO LATER REMOVE CAN EVER FIND THEM AGAIN.** The file is unreclaimable through the API for the life of the package. ⚠️ **The caller was told nothing** — both results discarded, `deleted: true` returned regardless. ⚠️ **Severity rises with D6**: this now happens **inside a SHARED WORLD**, where the junk propagates to every project binding that world. **Fix (landed in SP-116 T-0426):** delete the **binary first** — a failure between the two then leaves a listable, removable sidecar, a state a writer can act on — and **report** both failures instead of swallowing them. | **Medium** | **SP-116** (T-0426) | ✅ **Resolved - Verified (2026-08-21)** — ⚠️ **PRE-EXISTING, shipped: confirmed present at `HEAD`** (`git show HEAD:…AssetStore.cpp`), not introduced by this sprint. Found by **self-review**, not by any test; **538 green tests had nothing to say about it** |
| **I-0146** | `[ScriviCore]` ⚠️ **A `.partial` temporary is ORPHANED when the volume disappears mid-import — 459 MB of unreachable junk inside a shared world.** `copyFileInBlocks` (T-0432) writes to `<dest>.partial` and renames on success; on any failure it calls `std::filesystem::remove(tmp)`. ⚠️ **That cleanup CANNOT RUN when the failure IS the volume going away** — there is no filesystem left to remove from. **Observed on the real rig 2026-08-21**: 3.8 GB import to Eskandar on USB, drive physically pulled at ~24 s → import correctly aborted (the heartbeat detected it: `could not open temp file for writing … /.lock.tmp`), **no destination file**, **existing assets intact** — but `myton-huge.png.partial` (**459 MB**) survived on the volume. ⚠️ **`list_assets` cannot see it** (it scans for `*.meta.json`), so nothing in Scrivi will ever surface or reclaim it; it is invisible junk that travels with a shared world and grows with every interrupted import. ⚠️ **Same defect CLASS as I-0145** — a cleanup path that only runs when the failing process survives. **Fix (user-specified at ruling time):** *"on partial load, or dropped connection the timeout will fire and that can trigger the cleanup"* — a **stale-lock sweep**: when a writer breaks a stale lock it also removes `*.partial` under that package, since a partial belonging to a dead lock-holder can never be resumed. ⚠️ **A crash leaves the same orphan**, so this is not USB-specific. | **Medium** | **SP-116** (T-0433) | ✅ **Resolved - Verified (2026-08-21)** — `WorldLock::sweepAbandonedPartials()` reclaims `*.partial` on lock acquisition. ⚠️ **VERIFIED ON THE REAL RIG**: a 459 MB orphan staged on the USB volume was reclaimed by an ordinary import (disk 476 MiB → 12 MiB), both real assets untouched. ⚠️ **FOUND BY THE LIVE PASS, not by any suite.** The lab test *"aborted transfer leaves NO destination and NO partial"* passes — because there the filesystem is still there to clean up with. ⚠️ **Only pulling a real drive produced it** |
---

## ⚠️ I-0142 — the one defect in SP-115 that no suite found

**The user found it during verification**, not a test:

> *"When the Edit Object popup surfaces, the world that the Object belongs in is not selected."*

**Root cause spanned three layers, all making the same framing error** — treating a world as *a symptom of
its being away* rather than *a property of the object*. `worldID` was populated only on the **pending**
branch in `EndpointResolver` (`:44-49`), copied only inside `if (ep.pending())` in `RelationshipStore`
(`:490-494`), and emitted only when `otherPending` at the C ABI (`scrivi_c_api.cpp:895`). The app then
hardcoded `worldID: ""` (`ObjectCard.swift:396`).

> ⚠️ **The visible symptom was the lesser half.** `rename()` passes that `worldID` to `openObject`, which
> needs it to locate a world-scoped object's file — **so renaming ANY world object was failing silently.**
> The empty picker was the tell; the broken rename was the cost.

**✅ User ruling, taken as scope-limiting:** an object **cannot be moved between worlds**. The control is a
**label**; the picker survives only for a genuinely unassigned object. The questions a move would raise —
migrate related objects? allow cross-world edges? delete them? remap to equivalents? — were
⚠️ **deliberately left unopened.**

⚠️ **The regression test was confirmed to FAIL against the un-fixed core** before being kept, so it pins
the defect rather than merely passing alongside it.

**Two lessons worth carrying:**

1. ⚠️ **The picker offered an operation the code never performed.** `rename` patches only `displayName` and
   preserves every other field, so no world change could ever have been saved. **A control that implies a
   capability the code lacks is a defect even when nothing breaks.**
2. **A user's five-minute pass caught what 628 automated tests did not** — the same argument that made
   T-0418's live pass mandatory in EP-031.

**Suites at verification:** `ctest` **525/525** · macOS interop **103/103 in 10 suites** · app
**BUILD SUCCEEDED**.

---

*Last Updated: 2026-08-20 (decade opened — **I-0142 ✅ Verified**, found by the user during SP-115
verification.)*

---

## SP-116's six — and what verified them

**Verified 2026-08-21 (user-approved), archived in the same step.**

⚠️ **Three of these were not found by any suite.**

| Issue | Found by |
| ----- | -------- |
| I-0140, I-0141 | Design review (T-0424 filed them; **occurrence eight** of the restated-kind-list class) |
| I-0143 | ⚠️ **Reading the code D7 was about to modify** — not the design doc, not a test |
| I-0144 | ⚠️ **Looking for a caller to mirror** while implementing T-0426. `WorldLock` had shipped complete and correct in SP-097 and **nothing ever called it**; world-package object writes were unserialised for three sprints |
| I-0145 | Self-review, second pass — **pre-existing and shipped** |
| I-0146 | ⚠️ **THE LIVE RIG PASS.** A USB drive physically pulled mid-import left a 459 MB `.partial` no Scrivi operation could reclaim. ⚠️ **The lab test asserting "no partial remains" passed throughout** — it can only test failures the writing process survives |

⚠️ **Every fix was proven NON-VACUOUS** by reverting it and watching the tests fail — I-0143 (3 tests),
I-0140 (5 tests across 2 suites), I-0144 (removing one guard), I-0146 (2 tests).

**Suites at verification:** `ctest` **552/552** on macOS arm64, x86-64 **and** ASan/UBSan (was 525 at
sprint start) · **Linux 556 cases / 9300 assertions** under GCC 13 · macOS interop **107/107** · app
**BUILD SUCCEEDED**.

**Verified on the real rig** (`tintagael` + `Eskandar` on USB): S1 asset routing, S2 ⚠️ **a world's image
reaching a SECOND project**, S3 honest refusal with the drive out, S4 ⚠️ **102 recorded heartbeats across a
101-second write**, S14 orphan reclamation.

---

*Last Updated: 2026-08-21 (**I-0140, I-0141, I-0143–I-0146 ✅ Verified (user-approved) and archived in the
same step** at SP-116's close. ⚠️ **I-0147 is NOT archived** — it is an Accepted limitation, deferred to the
network-worlds design.)*
