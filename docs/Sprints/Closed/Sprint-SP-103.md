# SP-103 — implementation progress (2026-08-14)

**Status:** ✅ **CLOSED 2026-08-15 (user-approved).** ctest **516/516**, macOS interop **86/86**.

---

## The change itself (T-0409)

`objectKindIsWorldScoped` now returns true for all ten worldbuilding kinds; `source` is the sole
project-scoped object kind. Every scope decision in `ObjectStore` routes through that one predicate, so the
core change is genuinely small — as the planning estimate assumed.

`scrivi.h` is **untouched**. The C ABI carries no scope logic of its own (the I-0113 lesson, applied), so
the boundary needed no change at all.

---

## ⚠️ Two real defects the scope change exposed

Neither is a test-harness problem. Both were found because the ruling moved every object into the world,
and both would have been latent data-loss bugs.

### F1 — a missing world index silently **destroyed** the rest of the world's objects

`ObjectIndex::loadWorldIndex` returned an **empty index** when the file was missing or corrupt, under a
comment claiming it would be *"rebuilt by the next write."* **It was not.** The next `upsertWorld` wrote an
index containing only the object it had just added, dropping every other object from the index — the files
stayed on disk, perfectly readable, and became **invisible**.

**Verified by probe before fixing, not inferred:**

```
3 characters created → world index deleted → 1 more character created
  → scrivi_list_objects returned ONLY the new one
```

Ada, Bram and Cleo were still on disk. To a writer, three characters had vanished.

> This was survivable while worlds held only artifacts/chronicles/factions. Under the new ruling it is where
> **an entire cast disappears** — which is why the scope change had to carry the fix, not defer it.

**Fixed:** the world index now gets the same scan-rebuild guarantee the project index has always had (AC2).
`scanDir(baseDir, worldScoped)` is shared by both rebuild paths, and `writeWorldIndex` is the single writer
so the rebuild and upsert paths cannot drift. Re-probed: all four characters survive.

### F2 — ⚠️ a **third** copy of the kind list, still naming the old scope

`ObjectIndex.cpp`'s `kScannedKinds` was a third independent restatement of the kind table (after the enum
and the C ABI's), and it still listed the pre-ruling project scope — so a rebuild scanned `objects/` for
characters that can no longer live there. It is now derived from `objectKindIsWorldScoped()` rather than
restated.

> **This is the same defect shape as I-0113 and the SP-098 `source` kind table — now found a THIRD time in
> this Epic.** The standing note says *"before adding to `ObjectKind`, grep for other dispatch lists first."*
> That was not enough here, because this list did not need a new *kind* to go wrong — it went wrong when a
> kind's *scope* changed. The rule should be broadened: **any list that partitions kinds must be derived,
> never restated.**

---

## Test realignment (T-0411, in progress)

68 → 11 failures. The pattern: fixtures create a world and route world-scoped kinds into it; read/write
requests carry the `worldID`; assertions that encoded the old partition were rewritten rather than patched.

**Rewritten, not merely fixed** — these asserted the old model directly:

- `world-scoped kinds are identified as such` — inverted for six kinds; now asserts all ten plus `source`.
- `world-scoped kinds are refused when NO world is supplied` — **widened from 4 kinds to all 10**, and now
  also asserts `detail == "worldRequired"`, which is what T-0410's prompt will read.
- **New:** `` `source` is the sole kind creatable with NO world`` — the other half of the ruling.
- `CrossPartitionGraph` — its premise (project character ↔ world artifact) no longer exists; the
  project-side endpoint is now `source`, which is what makes the pair genuinely cross-partition.
- The AC2 zero-scan counter matched `/objects/` only, so it would have counted **zero** scans for world
  objects and **passed vacuously** — worse than failing. Now counts either parent.
- Demotion no longer clears scope (`item` is world-scoped), so it needs a destination world like promotion.

## Remaining — 11 failures

| Test | Why |
| --- | --- |
| AC-A1/A2 pending edges survive open and save verbatim | pending fixtures still assume a project-side endpoint |
| deleting an object tombstones every edge referencing it | " |
| load-time repair drops a genuinely dangling edge | " |
| cascade-prune leaves the surviving endpoint as a retained orphan | " |
| demotion is the exact inverse through the same endpoint | asserts cleared scope |
| saving an object refreshes its index entry | save path needs world index assertions |
| a STALE index loses to disk | " |
| one unparseable object file does not cost the whole index | " |
| a cross-partition edge resolves like a same-partition one (AC10) | endpoint pair |
| extractSearchableText ×2 | scans `objects/` for world kinds |

None of these indicate a further defect in the change — each is an assertion still written against the old
partition. They are finished the same way as the 57 already done.

---

## Not started

- **T-0410** — the worldless-project prompt. The core half is in place: creation refuses with
  `detail == "worldRequired"`, which is the discriminator the app reads. The `[Apple]` half (offer to create
  a world in place, per Doc 2 §4.6's no-modal rule) is not built.
- **T-0411** — finishing the 11 above, then **re-verifying EP-031 AC1**, which was unticked because it was
  verified against the old scope table.
- **Linux** — not built or run this session.

*Last Updated: 2026-08-14.*

---

## Closure (2026-08-15, user-approved)

**T-0409** (scope change) and **T-0411** (test realignment, 11 → 0) are done and user-verified. The
fallout the change exposed was cleaned up in **SP-104** (I-0114–I-0117) and **SP-105** (I-0118), both
closed the same day.

⛔️ **T-0410 removed as OBE, not delivered.** Ruling (a) of 2026-08-14 said a worldless project should
"prompt to create a world on first object creation." **That was written the moment the scope ruling was
made, before the state existed to be used.** Exercising it, the user found the behavior he wanted was
already the behavior: a worldless project **operates silently**, and a world is mentioned only when an
operation genuinely requires one. The task traced to **no design section, no acceptance criterion and no
reported defect**.

> ⚠️ **Superseding ruling (2026-08-15):** *operate worldless silently until an operation requires a
> world.* A project without a world is a normal state, not a setup step to be completed.

**Carried out of this sprint:** EP-031 **AC1 must be re-verified** — it was ticked against the
pre-T-0409 scope table, so its evidence no longer describes the shipped model. **SP-100 owns this.**

*Last Updated: 2026-08-15.*