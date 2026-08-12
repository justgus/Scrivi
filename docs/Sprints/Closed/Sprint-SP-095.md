## SP-095: [ScriviCore] Object Kinds + Fields + Object Index

**Status:** ✅ **Closed (Human-approved 2026-08-12).** All 4 tasks ✅ Verified; both suites green on
macOS and Linux; retrospective reviewed by the user.
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph (first of 6 sprints)
**Goal:** Lay the object-model foundation the relationship graph is built on: complete the `ObjectKind` set,
extend `WorldObjectFields` with `subtitle` / `image` / `worldID`, and replace the O(n) `findByID` directory
scan with `objects/index.json` — a derived, always-rebuildable cache. No edges, no worlds, no UI.
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` §3, §4.1, §4.2 (✅ Approved 2026-08-05);
`docs/Scrivi_World_Data_Separation_v0_1.md` §7 (✅ Approved 2026-08-05).
**Start Date:** 2026-08-12 | **End Date:** 2026-08-12 | **Capacity:** ~8–10 hours

---

### Sprint Goal

After SP-095, ScriviCore knows about every object kind EP-031 defines, every object file can carry a subtitle,
an image reference, and a world ID, and any object can be found by ID in one index lookup instead of a
directory scan with a JSON parse per file. That last item is the load-bearing one: **SP-096's edges store bare
`{id}` endpoints and cannot be rendered without the index** (Doc 1 §4.2, §5.2).

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0370 | `ObjectKind` additions (8 new) + `objectKindSubdir` + schema table; retire `timeline`; world-scoped kinds gated | High | ✅ **Verified (2026-08-12)** |
| T-0371 | `WorldObjectFields` extensions: `subtitle`, `image` (`assetID`/`thumbnailAssetID`), `worldID` | High | ✅ **Verified (2026-08-12)** |
| T-0372 | `objects/index.json` — build, atomic update on mutation, `findByID` over the index | High | ✅ **Verified (2026-08-12)** |
| T-0401 | Index rebuild + corruption coverage (delete / corrupt / hand-edit a slug) — Doc 1 AC2 | High | ✅ **Verified (2026-08-12)** |

### Assigned Issues

None. SP-095 opens no known defects.

---

### Scope rulings taken at planning (2026-08-12)

Three questions were surfaced before scope was fixed; all three are user-ruled.

#### R1 — World-scoped kinds are **declared but gated** (ruled: "declare all 8, implement 4")

Doc 1 §3 and Doc 3 §7.2 make `rule`, `artifact`, `chronicle`, and `faction` **world-scoped** — they live at
`worlds/<worldID>/…`, not `objects/…`. World packages do not exist until **SP-098** (T-0381/T-0382). Doc 3 §7
is explicit that Scrivi has not shipped, so **there is no migration**: world-scoped objects are "created in
world scope from the start," and no relocation pass will be written later.

SP-095 therefore:

- adds **all 8** kinds to `ObjectKind` + the schema table, so the enum is edited once and SP-096's index/graph
  can reason about the complete set;
- gives a working `objectKindSubdir` path to the **4 project-scoped** kinds only —
  `building`, `vehicle`, `map`, and (existing) `character` / `location` / `item`;
- makes create/open/save/delete of the **4 world-scoped** kinds fail with a clear, testable
  `invalidArgument` ("kind `<k>` is world-scoped; a world is required") until SP-098 supplies a world root.

**No object file lands in a location it will have to be moved out of.** That is the whole point of the gate.

> ⚠️ **`rule` is the exception that needs watching.** Unlike the other three, `rule` **already exists** as a
> project-scoped kind writing to `objects/rules/` (`ObjectTypes.hpp:22`, `ObjectStore.cpp`). It is the one kind
> where the design's target location and the shipped code disagree *today*. SP-095 does **not** relocate it —
> relocation is SP-098's, alongside the world package it moves into. SP-095 leaves `rule` working exactly as it
> does now and adds a `TODO(SP-098)` at the definition. Any developer fixture using `objects/rules/` is
> regenerated at SP-098, not migrated (Doc 3 §7).

#### R2 — `source` / T-0365 is **out of SP-095 entirely** (user ruling)

> *"Sources are not so much worldbuilding as they are a writing aid. Let's hold off on this entirely until
> later when we can explore this in more detail. It may be that sources will be better served in the Writing
> tab as opposed to the worldbuilding tab; however, all objects created in the worldbuilding tab should be
> able to have sources (more than one) associated with it. I'll want to review the language pertaining to
> sources already in the design documentation."*

`source` is **not** among SP-095's 8 kinds and no `objects/sources/` path is created. T-0365 stays deferred,
now with **no sprint assigned**, pending the user's review of the existing `source` language:

| Doc | Location | What it currently says |
| --- | --- | --- |
| Doc 1 | §3, l.98–100 | `source` is "a real `objects/` file (`objects/sources/<slug>.json`), **project**-scoped, because citations are reusable across scenes"; surfaced as a writing-tool card but a first-class object related to scenes by ordinary edges |
| Doc 1 | §11 Q2, l.676–679 | The same, recorded as a resolved open question (ruled 2026-08-05) |
| Doc 2 | l.28 | `sources` listed among **writing-tool cards** |
| Doc 2 | l.92 | Table: `sources` \| objects (`source`) \| "citations — reusable across scenes (ruled), so a real `objects/` file" |
| Doc 2 | l.302 | `sources` sits in the **Writing** tab (default), as a card stack |
| Doc 2 | l.585 | The resolved open question |

**The docs already agree with the user's instinct on placement** — Doc 2 §302 puts the `sources` card in the
**Writing** tab, not worldbuilding. What is **not** written anywhere is the second half of the ruling:

> **OQ-1 (new, unresolved):** every worldbuilding object should be able to carry **one or more** sources.
> Today the docs describe only `source → scene` edges. Object-to-source is a different edge shape (any object
> → many sources), which the SP-096 relationship graph supports natively, but **no relation type for it
> exists** and no doc states the requirement. This needs a design decision before T-0365 is scheduled —
> possibly a `cites` / `documented-by` relation type with `sourceKind: null` (any kind).

Deferring `source` costs SP-095 nothing: it was one entry in an enum SP-095 is already editing, and it can be
added later without disturbing the index or the schema table.

#### R3 — A fourth task for index-rebuild coverage (ruled)

Doc 1 §4.2 calls the rebuild path "**a correctness requirement, not a convenience**," and AC2 names three
distinct cases (delete the index, corrupt the index, hand-edit a slug). **T-0401** is opened so that coverage
is a deliverable rather than something folded into T-0372's implementation.

---

### Task detail

#### T-0370 — `ObjectKind` additions + `objectKindSubdir` + schema table; retire `timeline`

**Files:** `ScriviCore/include/scrivi/ObjectTypes.hpp`, `src/schemas/ObjectJson.{hpp,cpp}`,
`src/public_api/scrivi_c_api.cpp` (`objectKindFromStr`, l.345–350),
`src/public_api/ScriviCore.cpp` (`objectKindName` l.1041, the `collectObjects` kind list l.1149–1150).

1. **Add 8 kinds** to `ObjectKind`: `building`, `vehicle`, `map` (project-scoped, live);
   `artifact`, `chronicle`, `faction` (world-scoped, gated); plus `world` (container — see below).
   Each gets a `<Kind>Object` struct, a `WorldObject` variant alternative, a `scrivi.object.<kind>.v1` schema
   tag, and serialize/parse pairs, following the existing five exactly.
2. **`objectKindSubdir`** returns the plural directory for the project-scoped kinds. World-scoped kinds return
   their plural name too (`artifacts`, `chronicles`, `factions`) but are **never joined to `objects/`** — the
   gate in `ObjectStore` rejects them first. `rule` keeps `"rules"` unchanged (R1).
3. **Retire `ObjectKind::timeline`** (Doc 1 §3.2, T4=A). Remove the enum value, `TimelineObject`,
   `serializeTimeline`/`parseTimeline`, the `kTimelineSchema` tag, the `"timeline"` arm of
   `objectKindFromStr` (`scrivi_c_api.cpp:349`), `objectKindName` (`ScriviCore.cpp:1047`), the
   `collectObjects` kind list (`ScriviCore.cpp:1150`), the `ObjectStore` create switch
   (`ObjectStore.cpp:112`) and its `save` kind-inference fallback (`ObjectStore.cpp:169`), and
   `objectKindSubdir` (`ObjectTypes.hpp:33`). Update the three test sites (`JsonSchemaTests.cpp:728,740`,
   `ObjectCrudTests.cpp:324`, `SearchableContentTests.cpp:197`).

   > ⚠️ **Retire the KIND, not the DIRECTORY — verified at planning, and it is not what the design implies.**
   > Doc 1 §3.2 calls `ObjectKind::timeline` "a legacy overlap," which reads as though `objects/timelines/`
   > goes away with it. **It does not.** `ProjectCreator.cpp:240–252` creates `objects/timelines/` and writes
   > `timeline.meta.json` into it for **every new project**, and `ScriviCore.cpp:320` reads exactly that path
   > (`kTimelineMetaPath`) as the **project timeline's** home — live EP-016/EP-025 data. One directory, two
   > unrelated occupants: the dead kind's `<slug>.json` files and the Timeline Panel's `timeline.meta.json`.
   >
   > **Deleting the directory, or the `ProjectCreator` block that seeds it, breaks the Timeline Panel on every
   > newly created project.** T-0370 removes only the enum and its dispatch arms; `ProjectCreator.cpp:240–252`
   > and `kTimelineMetaPath` are **not touched**. `TimelineTests.cpp:91` asserts on that path and must stay green.
   >
   > Also note `ObjectStore.cpp:169`: the `save` kind-inference `if constexpr` chain uses `timeline` as its
   > **terminal `else`** — removing the alternative without restructuring that chain silently mis-infers the
   > kind of whatever alternative ends up last. Make the fallback explicit rather than positional.
4. **`objectKindFromStr` currently defaults unknown strings to `character`** (`scrivi_c_api.cpp:350`) — a silent
   mis-dispatch that would now be reachable with 12 kinds instead of 5. Change it to signal failure so an
   unknown kind returns an error envelope rather than quietly creating a character.
5. **`world` kind:** declared for schema completeness (`scrivi.object.world.v1`, Doc 1 §7.0) but **not**
   creatable through `scrivi_create_object` — worlds are created by `scrivi_create_world` in SP-098.

**Done when:** all 12 kinds compile through the variant; the 6 live kinds round-trip create → open → save →
delete; the 4 gated kinds return `invalidArgument` naming the world requirement; `world` is not creatable;
`timeline` is gone from every one of the 5 files above; existing character/location/item/rule files load
byte-identically.

#### T-0371 — `WorldObjectFields`: `subtitle`, `image`, `worldID`

**Files:** `ScriviCore/include/scrivi/ObjectTypes.hpp`, `src/schemas/ObjectJson.cpp`
(`serializeFields` l.16–55, `parseFields` l.57–96).

- `std::string subtitle` — one-line descriptor (Cumberland parity).
- `image` — **`assetID` + optional `thumbnailAssetID` only. Never inline bytes** (Doc 1 §4.1); images live in
  `assets/` via the existing `scrivi_import_asset`. Model as a small `ObjectImageRef` struct with an
  `empty()` predicate; serialize the sub-object only when non-empty.
- `std::string worldID` — optional; empty for project-scoped objects. SP-095 **writes and round-trips** it but
  attaches no meaning to it; resolution against a real world is SP-098's.

**All three are additive and must default empty.** An existing 5-kind object file with none of these keys must
parse unchanged — `parseFields` already returns `""` for absent string keys, so the guard is a test, not code.

**Done when:** all three fields round-trip; a legacy fixture file lacking all three parses with empty values
and re-serializes without spurious keys; `image` with only `assetID` set omits `thumbnailAssetID`.

#### T-0372 — `objects/index.json` + `findByID` over the index

**Files:** new `src/objects/ObjectIndex.{hpp,cpp}`; `src/objects/ObjectStore.cpp`
(`findByID` l.20–55, `create` l.61, `save` l.157, `remove` l.210); `src/schemas/` for the index schema.

Schema `scrivi.object-index.v1` per Doc 1 §4.2 — `entries[]` of
`{objectID, kind, slug, displayName, worldID}`.

- **`findByID` becomes an index lookup**, with the existing directory scan retained **only** as the rebuild
  path. Note the signature currently takes an `ObjectKind`; with the index, ID→kind is resolvable, so the
  kind parameter becomes optional/advisory. Keep the existing overload working so SP-095 changes no callers.
- **Written atomically via the existing `AtomicWrite`**, alongside every create / save / delete.
- **Derived cache, never authoritative** — `<slug>.json` files remain the truth (see T-0401).

> **Ordering note:** the index must be updated **after** the object write succeeds, never before. An index
> entry for a file that failed to write is a phantom; a missing entry for a file that did write is repaired
> by the next rebuild. The two failure modes are not symmetric — one is silently wrong, the other is
> self-healing.

**Done when:** `findByID` performs no directory listing on the happy path; index entries appear on create,
update on save (including a `displayName` change), and disappear on delete; the index survives a
create → close → reopen cycle.

#### T-0401 — Index rebuild + corruption coverage (Doc 1 AC2)

**Files:** `ScriviCore/tests/integration/ObjectIndexTests.cpp` (new),
registered in `ScriviCore/tests/CMakeLists.txt`.

Three explicit rebuild triggers, each its own test:

1. **Missing** — delete `objects/index.json`, reopen, assert it is rebuilt from a scan and `findByID` resolves.
2. **Corrupt** — write malformed JSON / a valid JSON document with the wrong schema tag; assert rebuild, not
   failure. **The project must open.**
3. **Stale** — hand-edit a `<slug>.json` (change `displayName`, and separately rename the file) so the index
   disagrees with disk; assert disk wins and the index is corrected.

Plus: a **rebuild is idempotent** (running it twice yields identical content), and an **unparseable object
file is skipped, not fatal** — matching `collectObjects`' existing best-effort posture (`ScriviCore.cpp:1053`).

**Done when:** all five tests pass, and no scenario leaves the project unopenable.

---

### Out of scope — explicitly

| Not in SP-095 | Where it lands |
| --- | --- |
| Any edge, relation type, or `relationships.jsonl` | SP-096 (T-0373–T-0376) |
| Cascade-prune, orphan queries, promotion | SP-097 (T-0377–T-0379) |
| ⚠️ Pending-vs-dangling — the Epic's highest-risk task | SP-097 (T-0380) |
| `.scrivworld` packages, bindings, locking, the epoch chain | SP-098 (T-0381–T-0385) |
| Relocating `rule` to world scope | SP-098 (with T-0381) |
| Any Apple/Linux UI, any card | SP-099 |
| `source` kind and `sources` card (T-0365) | Unscheduled — pending OQ-1 (R2) |

**No `scrivi.h` change is expected in SP-095.** Every new kind flows through the existing
`scrivi_create/open/save/delete_object` string dispatch (Doc 1 §6: "New object kinds need **no** new CRUD
endpoints"). If a signature change turns out to be needed, that is a scope deviation to surface, not to absorb.

**No `project.pbxproj` change is expected.** SP-095 is ScriviCore-only; the Apple app links the prebuilt
`libScriviCore.a` and does not compile ScriviCore sources (per the standing rule, pbxproj covers Swift app
sources only).

---

### Success criteria

SP-095 is done when:

1. All 12 `ObjectKind` values compile and the 6 live kinds round-trip create/open/save/delete.
2. The 4 world-scoped kinds fail with a clear, testable error naming the world requirement — not a crash, not
   a silent write to `objects/`.
3. `ObjectKind::timeline` is gone from all 5 files that reference it, with the Timeline Panel unaffected.
4. `subtitle` / `image` / `worldID` round-trip, and legacy files lacking them parse unchanged.
5. `findByID` resolves via `objects/index.json` with no directory scan on the happy path.
6. The index rebuilds from a scan when missing, corrupt, or stale — all three proven by test (T-0401).
7. **`ctest` green on macOS** at or above the 413/413 baseline, plus the new tests. ✅ **432/432**
8. **`ctest` green on Linux** in the Docker container (the Linux baseline runs ahead of macOS — it includes
   the `EncryptedFileSecureStore` tests). ✅ **439/439 under GCC 14**
9. No existing project fails to open; no object file changes location. ✅ (legacy-file parse test; `rule`
   untouched at `objects/rules/`; `objects/timelines/timeline.meta.json` asserted intact)

---

### Risks

| Risk | Mitigation |
| --- | --- |
| ⚠️ **Retiring `timeline` breaks the Timeline Panel.** `objects/timelines/` is **shared** — the dead kind *and* the live project timeline (`timeline.meta.json`, `ScriviCore.cpp:320`, seeded per-project by `ProjectCreator.cpp:240–252`) occupy the same directory. Doc 1 §3.2's "legacy overlap" phrasing invites deleting both. **Confirmed at planning, highest-likelihood defect in this sprint.** | T-0370 item 3: remove enum + dispatch arms only; `ProjectCreator` and `kTimelineMetaPath` untouched. `TimelineTests.cpp:91` must stay green — it asserts on that exact path. |
| **`ObjectStore.cpp:169` infers kind positionally** — `timeline` is the terminal `else` of an `if constexpr` chain, so removing it silently mis-infers whichever alternative lands last. | Make the fallback explicit, not positional (T-0370 item 3). A save-round-trip test per live kind catches a regression here. |
| **The index becomes a second source of truth.** The classic failure for a derived cache. | T-0401 exists precisely for this. Disk wins in every conflict test; the rebuild is the correctness path, not a fallback. |
| **`objectKindFromStr`'s silent `character` default** (`scrivi_c_api.cpp:350`) becomes reachable across 12 kinds. | Fixed in T-0370 item 4 — unknown kinds error rather than mis-dispatch. |
| **`rule`'s location disagrees with the design today.** | Ruled R1: untouched in SP-095, `TODO(SP-098)` at the definition, relocated with the world package that receives it. |
| Scope creep toward edges — the index makes them tempting. | The Out-of-scope table is explicit. SP-096 exists. |

---

### Notes

- **EP-031's ScriviCore sprints (SP-095–SP-098) have no dependency on EP-030**; only SP-099 needs the card
  framework. SP-095 could have run in parallel with the EP-030 sprints had capacity allowed.
- **Documentation debt found during planning, fixed in the same pass:** `Task-backlog.md` rows T-0373–T-0391
  still carried pre-renumbering sprint IDs (T-0373 read "SP-095" where `Epic-active.md` assigns SP-096). The
  2026-08-09 realignment updated the Epic but not the task backlog. Corrected 2026-08-12.
- **Package Structure v0.1 §11 still documents `objects/rules/`** and must be corrected to drop it (Doc 3 §7.2).
  That is a documentation fix, and it belongs with **SP-098**'s relocation, not here — noted so it is not lost.
- **Baseline confirmed at planning: `ctest --test-dir build` → 413/413, 0 failures** (macOS, 2026-08-12).
  That is the floor SP-095 must not drop below.
- **Two code-level findings surfaced during planning**, both folded into T-0370 rather than left to be
  discovered mid-implementation: (1) `objects/timelines/` is **shared** between the retiring kind and the live
  project timeline — the design's "legacy overlap" phrasing does not convey this, and deleting the directory
  would break every newly created project; (2) `ObjectStore.cpp:169` infers object kind from the **position**
  of `timeline` as a terminal `else`, which silently mis-infers once that alternative is removed. Neither is
  visible from the design docs alone.

---

## Implementation summary (2026-08-12)

**Files added:** `ScriviCore/src/objects/ObjectIndex.{hpp,cpp}`,
`ScriviCore/tests/integration/ObjectIndexTests.cpp`.
**Files changed:** `include/scrivi/ObjectTypes.hpp`, `src/schemas/ObjectJson.{hpp,cpp}`,
`src/objects/ObjectStore.{hpp,cpp}`, `src/public_api/scrivi_c_api.cpp`,
`src/public_api/ScriviCore.cpp`, both `CMakeLists.txt`, and three test files.
**`scrivi.h` untouched · no `project.pbxproj` change** — both as planned.

### T-0370 — kinds

`ObjectKind` is now 11 values: `character`, `location`, `item`, `building`, `vehicle`, `map` (project-scoped);
`rule` (project-scoped today, R1); `artifact`, `chronicle`, `faction` (world-scoped, gated); and `world`
(container). `objectKindName` moved into `ObjectTypes.hpp` as the single spelling — `ScriviCore.cpp` had a
second private copy of that switch, which is now deleted rather than extended.

- **The world-scope gate** is `ObjectStore::checkKindStorable`, applied on all four of create/open/save/delete.
  World-scoped kinds and `world` return `invalidArgument` with a message naming the reason. A test asserts the
  refusal leaves **no directory behind**.
- **`timeline` retired** across all 5 production sites. Per the planning finding, `ProjectCreator.cpp:240–252`
  and `kTimelineMetaPath` were **not touched**; a new test opens a fresh project and asserts
  `objects/timelines/timeline.meta.json` still exists and still carries the Timeline Panel's schema.
- **`ObjectStore.cpp:169`'s positional fallback is gone.** Kind inference moved to `worldObjectKind()`, an
  exhaustive `if constexpr` chain whose final `else` is a **`static_assert`** — adding a variant alternative
  later now fails to compile instead of silently mis-typing.
- **`objectKindFromStr` no longer defaults to `character`**; it returns `std::optional` and the four C ABI
  entry points emit an `unknown object kind` error envelope.

### T-0371 — fields

`subtitle`, `image` (`ObjectImageRef` — `assetID` + optional `thumbnailAssetID`, **by reference into
`assets/`, never inline bytes**), and `worldID` on `WorldObjectFields`. Optional blocks are written **only
when populated**, so an object carrying none of them serializes to the pre-SP-095 shape byte-for-byte. A test
parses a hand-written legacy character file and asserts empty defaults plus no spurious keys on rewrite.

### T-0372 — the object index

`objects/index.json` (`scrivi.object-index.v1`), written atomically **after** the object write succeeds, on
create / save / delete. `findByID` resolves through it and verifies the named file exists before trusting the
entry; a stale slug falls through to the retained scan.

> **The zero-scan claim is proven behaviorally, not asserted.** `ObjectIndexTests.cpp` installs a
> `ScanCountingFileSystem` decorator that counts `listDirectory` calls into `objects/`. Resolving through a
> healthy index counts **0**; deleting the index and resolving the same ID counts **> 0**, proving the scan
> survives as the repair path. Without this the suite would only have shown *that* resolution works, not that
> the index is what did the work.

### T-0401 — rebuild coverage

10 test cases / 85 assertions. Rebuild triggers: **missing**, **corrupt** (5 sections — malformed JSON, wrong
schema tag, unknown kind, empty `objectID`, truncated mid-document), and **stale** (2 sections — hand-edited
`displayName`, and a renamed file the index still points at). Plus rebuild **idempotence** (byte-identical
across two rebuilds, which required sorting entries by `objectID` — filesystem enumeration order is not
stable) and **one unparseable object file is skipped, not fatal**. In every case the project opens.

### Verification

| Check | Result |
| --- | --- |
| ScriviCore `ctest` (macOS) | ✅ **432/432**, 0 failures (baseline 413; +19 new) |
| ScriviCore `ctest` (**Linux / GCC 14, Docker**) | ✅ **439/439**, 0 failures (432 + 7 Linux-only `EncryptedFileSecureStore`) |
| Linux compile warnings | ✅ **none** — GCC 14 compiled the whole library clean |
| SP-095 tests under GCC | ✅ **22 test cases / 239 assertions** all pass |
| macOS app build | ✅ **BUILD SUCCEEDED** |
| macOS interop tests | ✅ **59 test cases passed, 0 failures** (baseline 56) |

**All nine success criteria are met.** Both criteria outstanding at first report (#7 interop, #8 Linux) were
resolved 2026-08-12 once Docker was started and the app was closed.

> **Reading the interop result — `Executed 0 tests` is a reporter artifact, not an empty run.** The suite is
> **Swift Testing**, which `xcodebuild`'s legacy XCTest reporter does not count; it prints
> `Executed 0 tests` alongside `TEST SUCCEEDED` while the Swift Testing reporter separately logs
> `Suite ScriviInteropTests passed`. Counting the actual per-case lines gives **59 passed / 0 failed**. The
> earlier `Could not launch "ScriviInteropTests"` failure was the app holding the runner — it cleared when the
> app was closed. Anyone re-running this should count test cases, not trust the `Executed N` line.

> **Linux was verified with two deviations from `platforms/linux/docker/Dockerfile`, both environmental and
> neither affecting what was under test** (GCC 14 compiling ScriviCore's C++23 + the full ctest suite):
> **(1)** Debian-based `gcc:14` instead of `ubuntu:24.04` — the Docker VM's apt fails GPG verification
> against every Ubuntu repo (*"At least one invalid signature was encountered"*); **(2)** FetchContent
> sources mounted from the host's populated `build/_deps` via `FETCHCONTENT_SOURCE_DIR_*` — GitHub returned
> **503** for the nlohmann/json tarball. Qt was omitted deliberately: SP-095 is pure ScriviCore.
> **Docker had also exhausted its disk** (91 GB used, 54 GB of it build cache) — `docker builder prune`
> reclaimed ~20 GB with all 76 images left intact. The `platforms/linux` Qt image still fails to build for
> reason (1) and should be re-checked when a `[Linux]` sprint next needs it.

---

### Retrospective (drafted 2026-08-12, for user review)

**Completed:** T-0370, T-0371, T-0372, T-0401 — all ✅ Verified (user-approved 2026-08-12).
**Returned to backlog:** none. All sprint items delivered.

**What went well.**
- **Reading the shipped code before writing the plan caught two defects that the design docs actively
  concealed.** Doc 1 §3.2 calls `ObjectKind::timeline` "a legacy overlap," which reads as though
  `objects/timelines/` retires with it — but `ProjectCreator` seeds `timeline.meta.json` into that same
  directory for every new project. Following the doc literally would have broken the Timeline Panel on every
  newly created project. The second, `ObjectStore.cpp:169`'s positional `else`, was invisible from the docs
  entirely. Both were folded into T-0370 at planning rather than discovered mid-implementation.
- **Turning the fallback into a `static_assert`** means the class of bug found at planning cannot recur: a
  future variant alternative fails to compile instead of silently mis-typing.
- **The scan-counting decorator turned an assumption into evidence.** The first-pass tests proved objects
  *resolve*, which passes identically whether or not the index is doing the work. Counting `listDirectory`
  calls proves the index replaced the scan (0 on the happy path) and that the scan survives as the repair
  path (>0 with the index deleted).

**What didn't.**
- **"TEST SUCCEEDED / Executed 0 tests" was nearly reported as a pass.** The interop suite is Swift Testing,
  which xcodebuild's legacy reporter does not count. Had that line been taken at face value, a green claim
  would have rested on zero executed tests. Counting per-case lines gave the real figure (59). **Lesson: for
  this project's interop suite, never trust `Executed N` — count test cases.** Recorded above the results
  table so the next sprint doesn't relearn it.
- **Three separate environmental failures** (Docker disk exhaustion, Ubuntu apt GPG, GitHub 503) stood between
  "code is done" and "Linux is verified." Worth knowing they are unrelated to the code, and that the
  `platforms/linux` Qt image is still broken by the apt issue.
- **Idempotence needed a fix the design didn't call for:** rebuild output had to be sorted by `objectID`,
  because filesystem enumeration order is not stable. Unsorted, the index would rewrite itself on every open.

**Adjust for SP-096.**
- The index is now load-bearing for bare-`{id}` edge endpoints. SP-096 should lean on `ObjectIndex::find`
  rather than re-deriving ID→kind.
- `checkKindStorable` is the single choke point for world-scope gating — SP-098 relaxes it in one place when
  world packages arrive, rather than hunting call sites.

---

*Last Updated: 2026-08-12 (**all 4 tasks ✅ Verified — user-approved.** SP-095 → 🟠 Review, awaiting close
approval. Suites: ScriviCore ctest **432/432 macOS** + **439/439 Linux (GCC 14)**, macOS interop **59 passed /
0 failed**, app **BUILD SUCCEEDED**. All nine success criteria met. Prior note follows.)*

*2026-08-12 (SP-095 planned. Scope fixed by three rulings: **R1** world-scoped kinds declared
but gated until SP-098 — no file lands where it must later be moved from; **R2** `source`/T-0365 held out
entirely pending the user's review of existing `source` language and a decision on **OQ-1** (worldbuilding
objects carrying multiple sources — an undocumented requirement no relation type covers); **R3** a fourth
task, T-0401, for index-rebuild coverage. Awaiting user confirmation before activation.)*
