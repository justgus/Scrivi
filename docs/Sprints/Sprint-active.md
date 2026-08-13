# Active Sprint

**No active sprint.** **SP-098** ✅ closed 2026-08-12 (Human-approved) — the relationship graph is now
self-consistent under deletion, and ⚠️ **absence is never deletion**. **SP-099** (Apple worldbuilding-object
cards, on EP-030's framework) is staged in `Sprint-backlog.md`, awaiting planning and activation. It is the
first `[Apple]` sprint of EP-031 and is unblocked — EP-030's card framework closed 2026-08-11.

---

## Recently closed (2026-08-12)

| Sprint | Title | Epic | Archive |
| ------ | ----- | ---- | ------- |
| SP-098 | `[ScriviCore]` Graph integrity — cascade-prune, orphans, promotion, ⚠️ pending-vs-dangling | EP-031 | `Closed/Sprint-SP-098.md` |
| SP-097 | `[ScriviCore]` World packages — `.scrivworld`, bindings, resolution, locking, epoch chain | EP-031 | `Closed/Sprint-SP-097.md` |
| SP-096 | `[ScriviCore]` Relationship graph — canonical edges, append-log, compaction | EP-031 | `Closed/Sprint-SP-096.md` |
| SP-095 | `[ScriviCore]` Object kinds + fields + object index | EP-031 | `Closed/Sprint-SP-095.md` |

**✅ SP-098 CLOSED** (Human-approved) — T-0405 / T-0380 / T-0377 / T-0378 / T-0379 / T-0406 all Verified, plus
**I-0113** Resolved-Verified. The graph now prunes itself on object, scene, **and** chapter delete with a
load-time repair pass behind it; orphaned objects are **retained and findable**; `item`↔`artifact` promotion
preserves `objectID` with the edge log **byte-identical**; and `source` landed, closing AC1.

⚠️ **The load-bearing outcome is T-0380.** An endpoint that will not resolve because its **world is away** is
*pending* — held, never pruned, never modified — and is now a distinct state from *dangling*, not something
callers infer from `found`. The graph is **frozen** toward an unavailable world in both directions
(`detail == "worldPending:<status>"`, never a silent drop), pending edges survive save verbatim, they return on
reattach with no repair pass, and they display **names** from the binding's cache rather than bare IDs. Both
branches of AC-A5 are tested explicitly. This is the failure Doc 3 §4.6 calls *silent and unrecoverable*; it is
the reason T-0380 was built and tested **before** any prune code existed (R2).

**Verification:** ctest **510/510 macOS** and **517/517 Linux (GCC 14.2, zero warnings)**; **33 new tests, all
through `scrivi_*`** rather than the C++ facade — the standing habit the I-0113 audit asked for, since a
facade-only test cannot see a boundary gap. macOS interop **56/56**. EP-027's shipped delete suites stayed
green, which was the gate for reaching into them.

**Two findings worth carrying forward:**

1. **R1's "the only in-tree callers are ScriviCore's own tests" was wrong** — `ScriviEngine.swift` wraps all
   three widened endpoints. They took a defaulted `worldID: String = ""`, so every existing Swift call site is
   source-compatible and the Apple layer gained world-scoped CRUD for free. *Treat "the only callers are X" as
   a hypothesis; the sweep is what caught this.*
2. ⚠️ **A duplicated kind list in `scrivi_c_api.cpp` rejected `source`** after the enum, schema, index, and
   search extractor had all accepted it. It now delegates to `objectKindFromName`. **This is the same defect
   shape as I-0113** — a boundary re-stating what the core already knows — found twice in one Epic.
   *Before adding to `ObjectKind`, grep for other dispatch lists first.*

**EP-031 now has AC1, AC4, and AC7 met — 8 of 10, with 4 of 6 sprints closed.** Remaining: AC9/AC10 (the Apple
cards, SP-099) and the Epic verification pass (SP-100).

---

*Last Updated: 2026-08-12 (SP-098 closed, Human-approved. Sprint archived to `Closed/Sprint-SP-098.md`;
this file reset for SP-099.)*
