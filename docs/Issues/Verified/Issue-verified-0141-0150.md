# Verified Issues: I-0141 – I-0150

Archived Issues, ✅ **Resolved - Verified** by the user. Batched in decades of ten per
`Issue-GUIDELINES.md`.

⚠️ **I-0141 is NOT here** — it is 🔴 **Open**, filed by T-0424 for **SP-116**. This file starts its decade
with the first *verified* member.

| ID | Title | Severity | Sprint | Verified |
| -- | ----- | -------- | ------ | -------- |
| I-0142 | `[Cross]` ⚠️ **The object editor's World control never showed the object's own world — and RENAME of a world object was broken outright.** Found by the **user during SP-115 verification**: *"When the Edit Object popup surfaces, the world that the Object belongs in is not selected."* ⚠️ **Root cause spans THREE layers, all gating `worldID` on `pending`:** `EndpointResolver` set `out.worldID` only on its pending branch, returning without it for a reachable world object (`EndpointResolver.cpp:44-49`); `RelationshipStore` copied `otherWorldID` only inside `if (ep.pending())` (`:490-494`); and the C ABI emitted the key only when `otherPending` (`scrivi_c_api.cpp:895`). The app then hardcoded `worldID: ""` when opening the editor (`ObjectCard.swift:396`). ⚠️ **The second consequence is worse than the visible one:** `rename()` passes that `worldID` to `openObject` (`ObjectCard.swift:274-283`), which needs it to locate a world-scoped object's file — **so renaming any world object failed**, and the visible symptom was only the empty picker. ✅ **A world is a property of the OBJECT, not a symptom of its world being away** — that framing error is the whole bug. **Fixed:** `worldID` reported whenever the endpoint has one, at all three layers + the app. Boundary-tested (`[I-0142]`), and the test was confirmed to FAIL against the un-fixed core. | **High** | **SP-115** | ✅ **Resolved - Verified (2026-08-20)** — SP-115 / T-0425 |

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
