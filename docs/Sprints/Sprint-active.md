# Active Sprint

**None.** SP-099, SP-103, SP-104 and SP-105 all closed 2026-08-15 (user-approved) →
[`Closed/`](Closed/Sprint-SP-099.md).

---

## Where EP-031 stands

| Sprint | State |
| ------ | ----- |
| SP-095–SP-098 | ✅ Closed — the entire `[ScriviCore]` half |
| SP-099 | ✅ **Closed 2026-08-15** — engine wrappers, object cards, picker, in-place create/edit, Worlds menu |
| SP-103 | ✅ **Closed 2026-08-15** — the scope ruling (T-0409) + test realignment (T-0411); T-0410 removed as OBE |
| SP-104 | ✅ **Closed 2026-08-15** — post-ruling fallout (I-0114–I-0117) |
| SP-105 | ✅ **Closed 2026-08-15** — world search indexing (I-0118) |
| SP-102 | 🔵 Planning — pending presentation, warning view, `sources` card |
| SP-100 | 🔵 Planning — EP-031 verification & Epic close (**runs last**) |

---

## ⚠️ Carried forward — NOT closed with the sprints above

**Nothing here is code.** One re-verification, one product decision, one platform with no counterpart.

**EP-031 AC1 must be RE-VERIFIED.** It was ticked against the pre-T-0409 scope table, so its evidence no
longer describes the shipped model. SP-100 owns this.

**EP-033 `[Cross]` — World Lifecycle Management** (Epic backlog, 🔵 Proposed). Opened from the I-0118 Q1
ruling. Nothing in Scrivi can delete a world or its search entries, and a world with no project bound is
unreachable — Scrivi opens *projects*. The Epic's first deliverable is the product fork: **a view inside
Scrivi, or a dedicated world-management application.**

**Linux** — untouched throughout SP-104/SP-105. The `[ScriviCore]` fixes are cross-platform and covered by
`ctest`; the `[Apple]` halves (Worlds menu, bookmarks, Spotlight donation) have no Qt counterpart, and the
Linux app donates to no search index at all.

---

## State at close (2026-08-15)

- `ctest` **516/516** · macOS interop **86/86** · app **BUILD SUCCEEDED**
- Issues verified across these three sprints: **I-0114–I-0119**

⚠️ **Every defect that mattered in these sprints was found by USE, not by the test suites**, which were
green throughout. Worth remembering when the next sprint's evidence is "all tests pass."

---

*Last Updated: 2026-08-15 (SP-099, SP-103, SP-104, SP-105 closed; no sprint active — next is SP-102).*
