# Active Sprint

**None.** SP-099, SP-103, SP-104 and SP-105 all closed 2026-08-15 (user-approved) →
[`Closed/`](Closed/).

The next Sprint is **SP-106** (⚠️ `[Cross]` **test integrity & CI trust** — I-0121, the sanitizer CI leg,
macOS platform coverage), opened 🔵 Planning 2026-08-16. Then **SP-102** (`[Apple]` pending presentation +
warning view + `sources` card), and **SP-100** (verification & Epic close) last. All three are scoped in
[`Sprint-backlog.md`](Sprint-backlog.md).

⚠️ **SP-106 runs first deliberately.** ScriviCore CI has been red since 2026-07-30 on a divide-by-zero that
only traps on x86-64 (I-0121), so **six sprints closed on arm64-only evidence** — and SP-100's whole remit is
Epic verification. The code fix is already applied; the sprint's substance is making the *class* of defect
fail deterministically and closing the macOS platform-coverage gap.

---

## Recently closed

| Sprint | Title | Closed | Archive |
| ------ | ----- | ------ | ------- |
| SP-099 | `[Apple]` Engine wrappers, object cards, picker, in-place create/edit, Worlds menu | 2026-08-15 | [`Sprint-SP-099.md`](Closed/Sprint-SP-099.md) |
| SP-103 | `[Cross]` Scope ruling (T-0409) + test realignment (T-0411); T-0410 removed OBE | 2026-08-15 | [`Sprint-SP-103.md`](Closed/Sprint-SP-103.md) |
| SP-104 | `[Cross]` Post-ruling fallout — world reachability + the restated-kind class (I-0114–I-0117) | 2026-08-15 | [`Sprint-SP-104.md`](Closed/Sprint-SP-104.md) |
| SP-105 | `[Cross]` World search indexing (I-0118) | 2026-08-15 | [`Sprint-SP-105.md`](Closed/Sprint-SP-105.md) |

**State at close:** `ctest` **516/516** · macOS interop **86/86** · app **BUILD SUCCEEDED**.

Issues carried across these four sprints: **I-0114–I-0119**. ⚠️ **Not all are verified.** **I-0118 and
I-0119 are ✅ Verified** and archived; **I-0114, I-0115, I-0116 and I-0117 remain 🟠 `Resolved - Not
Verified`** in [`../Issues/Issue-active.md`](../Issues/Issue-active.md), awaiting user verification. Closing
a Sprint does not verify its Issues — only the user does.

⚠️ **Every defect that mattered in these sprints was found by USE, not by the test suites**, which were
green throughout. Worth remembering when the next sprint's evidence is "all tests pass."

---

*Last Updated: 2026-08-16 (consistency audit — the close summary claimed **"Issues verified across these four
sprints: I-0114–I-0119."** Only **I-0118/I-0119** are Verified; **I-0114–I-0117** are still `Resolved - Not
Verified` per `../Issues/Issue-active.md`. ⚠️ **A closed Sprint was being read as verifying its Issues** —
it does not. The same wrong claim was corrected in `../Epics/Epic-active.md` and
`Sprint-Documentation.md`. Prior note follows.)*

*2026-08-15 (docs cleanup — the "Where EP-031 stands" status block moved to EP-031 in
[`../Epics/Epic-active.md`](../Epics/Epic-active.md), where Epic status belongs. The AC1 re-verification
requirement, the Linux gap and the EP-033 spawn travelled with it. This file now carries Sprint status
only.)*
