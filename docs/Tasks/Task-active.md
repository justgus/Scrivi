# Active Tasks

Tasks currently being worked by an **active Sprint**. Each row names the Sprint it belongs to; the
Sprint's own documentation lives in [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md) and is
**not** duplicated here.

A Task leaves this file when it is Verified (→ `Verified/Task-verified-XXXX.md`) or when its Sprint
closes. Tasks that are implemented but awaiting user verification move to
[`Task-unverified.md`](Task-unverified.md).

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

---

## Currently: **3 active Tasks** — SP-106 `[Cross]` Test integrity & CI trust

| ID | Title | Sprint | Priority | Status |
| -- | ----- | ------ | -------- | ------ |
| T-0412 | Confirm the I-0121 fix on **x86-64** — the platform that actually traps the defect | SP-106 | High | 🟠 **Implemented - Not Verified (2026-08-16)** |
| T-0413 | ⚠️ **Sanitizer CI leg** — `SCRIVI_ENABLE_SANITIZERS` + 2×2 CI matrix | SP-106 | High | 🟠 **Implemented - Not Verified (2026-08-16)** |
| T-0414 | **macOS platform coverage** — `platformDefault`'s Apple branch (Linux had 7 tests, macOS zero) | SP-106 | Medium | 🟠 **Implemented - Not Verified (2026-08-16)** |

**Evidence:** ScriviCore CI green on **x86-64** for the first time since 2026-07-30 (run `31975883684`:
`ubuntu-latest` 523/523, `macos-latest` 516/516). Local: **519/519** on macOS arm64, plain **and** ASan+UBSan.
Both T-0413 and T-0414 were **proven RED before green** — the sanitizer reports the reintroduced
divide-by-zero on arm64, and the new Apple tests fail when the Apple path rule is mutated.

Full sprint record: [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md).

---

## Cleanup note (2026-08-15)

This file previously carried **full sprint documentation for eight Sprints** — SP-090, SP-091, SP-092,
SP-093, SP-095, SP-096, SP-097 and SP-101 — including retrospectives, suite counts, design rulings and
planning narrative. That content belongs to the Sprint layer, not the Task layer.

**All eight Sprints were already closed and archived** to `../Sprints/Closed/`, and every Task and Issue
listed under them was already Verified. Nothing was active. The sections were removed rather than moved:
each Sprint's archive file is the authoritative record and already contains the same material.

| Sprint | Archive | Tasks (all Verified) |
| ------ | ------- | -------------------- |
| SP-090 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) | T-0359–T-0362 |
| SP-091 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) | T-0392, T-0393, T-0363, T-0364 |
| SP-092 | [`Sprint-SP-092.md`](../Sprints/Closed/Sprint-SP-092.md) | T-0394, T-0395, T-0366, T-0367, T-0368 |
| SP-093 | [`Sprint-SP-093.md`](../Sprints/Closed/Sprint-SP-093.md) | T-0396, T-0397, T-0398 (+ I-0104–I-0111) |
| SP-095 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) | T-0370, T-0371, T-0372, T-0401 |
| SP-096 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) | T-0402, T-0373, T-0374, T-0375, T-0376 |
| SP-097 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) | T-0403, T-0381–T-0385, T-0404 |
| SP-101 | [`Sprint-SP-101.md`](../Sprints/Closed/Sprint-SP-101.md) | T-0399 |

Two sections were **stale as well as misplaced**: SP-101 was shown 🟡 active and SP-092 🔵 planning, but
both closed 2026-08-11 (Human-approved). T-0399 is ✅ Verified; SP-092's T-0368 closed as delivered by
T-0399.

**Carried forward, not lost:** T-0365 (`sources` card) remains 🔵 Backlog, unblocked 2026-08-12, and is
scoped into SP-102.

⚠️ **Correction (2026-08-16):** this section previously said T-0217 "is still owed **before EP-019 closes**."
That was impossible on its face — **EP-019 closed 2026-08-11**, five days before. T-0217 was in fact
**completed in SP-094 and Verified 2026-08-11** (design §4.a/§4.a.1/§4.d/§12.2/§12.8/§14/§15,
package-structure §16a, repair-matrix §6.21); its record of truth is
[`Sprint-SP-094.md`](../Sprints/Closed/Sprint-SP-094.md). Nothing is owed. `Task-backlog.md` carried it as
🔵 Backlog under the superseded SP-057 and has been corrected to match.

---

*Last Updated: 2026-08-16 (SP-106 activated and implemented — T-0412/T-0413/T-0414 all 🟠 Implemented - Not
Verified. This file carries active Tasks again for the first time since 2026-08-15.)*
