# Active Sprint

**No Sprint is currently active.**

SP-102 was ✅ **CLOSED 2026-08-18 (user-approved)** → [`Closed/Sprint-SP-102.md`](Closed/Sprint-SP-102.md).

---

## What SP-102 delivered

**EP-031 AC23 + AC24** — the last two clauses of **AC9**, the Epic's highest-risk property: *absence is
never deletion*, and the writer must be able to **see** that guarantee.

| Task | Outcome |
| ---- | ------- |
| **T-0389** | Card-level §7.2 pending footer + project-wide warning view + the `offline`/`unmounted` refinement | ✅ Verified 2026-08-17 |
| **T-0415** | AC23 live verification on the real USB world rig — ⚠️ **ran twice** | ✅ Verified 2026-08-17 |
| **T-0417** | `[Apple]` Scene/Chapter boundary navigation (adopted mid-sprint) | ✅ Verified 2026-08-18 |
| **T-0365** | Aggregate `sources` card | ✅ Verified 2026-08-18 **as a PARTIAL delivery** |

**Suites at close:** `ctest` **520/520 macOS arm64** · macOS interop **99/99 macOS arm64** ·
**BUILD SUCCEEDED**.

⚠️ **The live eject/reattach runs produced eight Issues — I-0123–I-0130 — all Verified 2026-08-17.**
They are the AC23/AC24 evidence, not a footnote to it. **I-0129 is the no-intervention clause itself**:
world availability refreshed on *app focus* rather than on the drive mounting, which no fixture would
have caught, because ejecting a drive normally forces a focus change that masked the defect entirely.

Also adopted mid-sprint and closed: **I-0131** and **I-0132** (navigator/manuscript current-scene
model), both ✅ Verified 2026-08-18, and **I-0133** (dead scroll-fraction state) 🟠 Resolved.

---

## ⚠️ Carried out of SP-102 — do not treat as delivered

| Item | Owed to |
| ---- | ------- |
| **Source creation** — nothing in the app creates a `source` object or attaches a `cites` edge, so the `sources` card can only render its empty state | **EP-034** |
| §3.1.1's **second popup entry point** — object cards must surface their own sources through the same `CitationPopover` (built; `ObjectCard.swift` references it **0** times) | **EP-034** |
| **Key equivalents** for Scene/Chapter Start-End — the macOS keyspace is exhausted, so T-0417 shipped as menu items only and the binding is still an open question | unassigned |

---

## Next

**SP-100** — EP-031 verification & Epic close — is 🔵 **Planning** in
[`Sprint-backlog.md`](Sprint-backlog.md) and **runs last**. It is the only Sprint remaining in EP-031's
eleven, and it ⚠️ **owns the AC1 re-verification**.

Its two Tasks (**T-0390** repair-matrix world conditions, **T-0391** Epic verification + close prep)
are in [`Task-backlog.md`](../Tasks/Task-backlog.md).

> ⚠️ **One question SP-100 inherits from SP-106:** the sanitizer matrix found an unknown defect
> (I-0122) on its first run, so *"what else is latent behind single-architecture evidence"* is an
> empirical question for that sprint, not a rhetorical one.

**Activating SP-100 is a planning decision and needs direct user approval.**

---

*Last Updated: 2026-08-18 (SP-102 closed; no Sprint active.)*
