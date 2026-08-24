# T-0365 — The Aggregate `sources` Card (EP-030 → EP-031, SP-091 → SP-096 → SP-098 → SP-102)

**Status:** ✅ **COMPLETE as of 2026-08-24** — ⚠️ **no longer partial.**
Verified 2026-08-18 as a **partial** delivery (read half); ✅ **the owed write half and the owed second
entry point BOTH shipped in EP-034 SP-120** (T-0454–T-0457, Verified 2026-08-24) →
[`Task-verified-0453-0458.md`](Task-verified-0453-0458.md).

> ⚠️ **Both debts this file recorded are now paid**, four sprints after it was written:
> 1. ✅ **Source creation + `cites` attachment** — from the documented object, per the user's ruling.
> 2. ✅ **§3.1.1's second entry point** — an object surfaces its own sources and clicking one opens **the
>    same `CitationPopover`**. ⚠️ **It was owned by no AC and no sprint** until SP-120 planning found it
>    by grepping: the popup had exactly one call site, the card that declares it.
>
> ⚠️ **The warning below — *"do not read this Task's ✅ as 'the sources feature works'"* — no longer
> applies.** It does now, verified by use on a real sequel project.
**Sprint:** SP-102 · **Design:** `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §3.1.1 (ruled
2026-08-12)

---

## The task was always three things, delivered across four sprints

| Third | What | Where | Status |
| ----- | ---- | ----- | ------ |
| 1 | `cites` / `documented-by` **relation type** — the first unconstrained on **both** ends | SP-096 / T-0373 | ✅ Verified 2026-08-12 |
| 2 | `source` **object kind** — project-scoped at `objects/sources/`; **closed EP-031 AC1** | SP-098 / T-0406 | ✅ Verified 2026-08-12 |
| 3 | The **aggregate card** + citation popup | SP-102 | ✅ Read half; ⚠️ **write half + 2nd entry point → SP-120, ✅ 2026-08-24** |

## What was delivered (verified)

`Scrivi/Views/Inspector/SourcesCard.swift`, registered in the Writing stack
(`InspectorCard.swift:235`), pbxproj'd across all three app targets (8 entries), 4 tests.

**The indirect traversal §3.1.1 requires**, since sources attach to **objects**, never to scenes
(Doc 1 §3.4):

```text
this scene ──edges──▶ worldbuilding objects ──cites──▶ sources
```

Both hops use `listEdgesFor`, which resolves the far end regardless of canonical edge direction, so
the card never reasons about direction itself.

Design points implemented deliberately:
- **ONE aggregate card**, never one per source — so it can be shown/hidden as a unit in the picker,
  and so a research-heavy project does not flood the Writing stack.
- **De-duplication across objects** — a source reached through two objects appears **once**, naming
  both. Two rows for one source reads as two sources.
- **Each entry names its citing object(s)** — required, not decorative: otherwise the writer cannot
  tell *why* a citation is surfacing on this scene.
- **Click → `CitationPopover`**, loaded lazily. The card row is a summary; the popup is the record.
- **Empty is a normal state**, worded as such — a scene with no objects, or objects with no citations,
  is not an error.
- **Pending objects are skipped** in the walk: their world is away so their `cites` edges are
  unreadable, and reporting "no sources" for them would be a silent lie about the writer's research.
  They are already badged on their own object card.

**Evidence:** interop **99/99 macOS arm64**, `ctest` **520/520 macOS arm64**, **BUILD SUCCEEDED**.

## ⚠️ What was NOT delivered in SP-102 — ✅ **both items shipped in SP-120 (2026-08-24)**

**1. The write half — nothing in the app can create a `source` or attach a `cites` edge.**

Found by the user on first use: *"I can show the sources card, but I can't create any sources. None of
the card interfaces allow it."* The card therefore **can only ever render its empty state**, which is
indistinguishable from working correctly.

The cause is broader than sources and is EP-031's signature defect: **object cards edit `displayName`
only** (one `TextField`, `ObjectCard.swift:647`) while `subtitle`, `notes` and `image` shipped in
**SP-095/T-0371** (✅ Verified 2026-08-12) and are read by **nothing** — *capability shipped, surface
never built*.

⚠️ **Source creation was deliberately NOT added to this card** (user ruling 2026-08-18): *"it would
muddy the fact that the source must be associated with an object in the world."* A citation documents
an **object**; creating one from a scene-scoped aggregate card inverts the model. It belongs on the
object's own detail surface — **[EP-034 `[Cross]` Object Detail & Media](../../Epics/Epic-backlog.md)**,
opened the same day. **EP-032 was checked and does not cover this** (it is manuscript reference syntax).

**2. §3.1.1's second entry point to the popup is unwired.**

> *"The same interaction appears on worldbuilding-object cards (ruled): an object card surfaces its own
> sources, and clicking one opens **the same citation popup**. One popup implementation, two entry
> points."*

`CitationPopover` is built and reused-ready; `ObjectCard.swift` does not reference it (grep: **0**).
Only the `sources` card opens it. ⚠️ **This is a design requirement, not a nicety** — and it is not
testable until sources can exist, so it travels with the write half to EP-034.

## Why it was accepted as partial

The blocker is another Epic's work, the delivered code is correct, and holding SP-102 open for a
dependency it does not own would make the record less honest rather than more. The card goes live the
moment EP-034 provides creation.

⚠️ **Do not read this Task's ✅ as "the sources feature works."** It means the read half is correct and
accepted. Anyone picking up EP-034 owes: source creation from the object detail surface, `cites`
attachment, and the object-card entry point to `CitationPopover`.

---

*Archived 2026-08-18 on user acceptance of the partial delivery.*

---

## ⚠️ Closing note added 2026-08-24 (EP-034 SP-120)

The card went live exactly as this file predicted: *"the card goes live the moment EP-034 provides
creation."* ⚠️ **It had rendered only its empty state for four sprints** — a state indistinguishable from
working correctly, which is why the partial acceptance was recorded so explicitly.

⚠️ **What this Task's history is now the best evidence for:** a capability shipped in the core and left
without a surface is **not** half-done — it is **invisible**, and it stays invisible until someone tries
to use it. `cites` shipped SP-096, the `source` kind SP-098, the card SP-102 — and no writer could create
a single source until SP-120.
