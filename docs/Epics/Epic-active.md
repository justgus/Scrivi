# Active Epics

**No Epic is currently active.**

**EP-031** `[ScriviCore]` Worldbuilding Object Model & Relationship Graph was ✅ **CLOSED 2026-08-19
(user-approved)** → [`Closed/Epic-EP-031.md`](Closed/Epic-EP-031.md).

---

## What EP-031 delivered

**All 10 acceptance criteria verified across 11 sprints** (2026-08-12 → 2026-08-19): the object model and
its eleven kinds, the object index, the canonical relationship graph, `.scrivworld` packages with
identity-verified resolution and write locking, the epoch chain, cascade-prune integrity, and the
worldbuilding-object cards on EP-030's framework.

⚠️ **Its highest-risk property held: *absence is never deletion*.** An unavailable world holds its edges
**pending** — never pruned, never modified, surviving save byte-for-byte and restored on reattach. **That
was confirmed live on real hardware**, not on a fixture.

**Suites at close:** `ctest` **520/520 macOS arm64** · x86-64 + sanitizers ✅ (CI 2×2) · macOS interop
**99/99 in 10 suites** · app **BUILD SUCCEEDED**.

⚠️ **The Epic grew from 6 planned sprints to 11, and four of the five additions came from USE, not
planning** — SP-103 from a writer's question that exposed no path for cross-project character reuse;
SP-104 and SP-105 from that ruling's fallout; SP-106 from a CI error the user noticed, red for 17 days.

---

## ⚠️ Carried out of EP-031 — do not read as delivered

| Item | Owed to |
| ---- | ------- |
| **Source creation** — nothing in the app creates a `source` or attaches a `cites` edge | **EP-034** |
| §3.1.1's **second popup entry point** — object cards surfacing their own sources | **EP-034** |
| **I-0135 · I-0136 · I-0137 · I-0138 · I-0139** — filed by SP-100, **none fixed by design** | unassigned |
| **T-0416** — seeded relation types never reach existing projects | unscheduled |
| **Key equivalents** for Scene/Chapter Start-End | unassigned |

⚠️ **Two honest gaps in the AC evidence, stated rather than glossed:** AC1's eleventh kind (`source`) is
proven at the core and boundary but **not by use**, because no creation UI exists; and **AC3/AC9's
from-either-entrance clauses had no live evidence** from the final pass, because I-0139 blocked it. Both
rest on green suite coverage.

---

## The lesson this Epic paid for five times

**`project_capability_without_surface`** — a capability ships, its tests pass, and no writer can reach it.
⚠️ **I-0137 is its subtlest form yet:** the capability, its unit tests **and** its call site all exist —
only the data path is missing. An interop suite named *"World volume status refinement (EP-031 AC24)"*
**passes**, while the feature never reaches the writer.

> ⚠️ **A passing test suite named after an acceptance criterion is not evidence the criterion is met in
> the product.**

---

## Next

**No Epic is active.** Candidates in [`Epic-backlog.md`](Epic-backlog.md):

| Epic | Why it might be next |
| ---- | -------------------- |
| **EP-034** `[Cross]` Object Detail & Media | ⚠️ **Owes source creation** — EP-031's largest carried gap |
| **EP-032** `[Cross]` Inline Object References | Depends on EP-031 (now closed) + EP-029 |
| **EP-033** `[Cross]` World Lifecycle Management | Opened from the I-0118 Q1 ruling |
| **EP-026** `[Linux]` Undo/Redo, Menus, Settings & Parity | The last planned `[Linux]` Epic |

**Next available:** Epic **EP-035** · Sprint **SP-107** · Task **T-0419** · Issue **I-0140**.

---

*Last Updated: 2026-08-19 (EP-031 ✅ CLOSED, user-approved. Active Epics 1 → 0.)*
