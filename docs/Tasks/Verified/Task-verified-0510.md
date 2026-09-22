# Verified Task: T-0510

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0510** | ⚠️ **WITNESS [SP-149]'s guard across the retirement — RED before [SP-142], GREEN after — and CLOSE [I-0197]** | [SP-143] | [EP-041] | **2026-09-22** |

---

## ⚠️ Why a separate Task at all

⛔ **[SP-149]'s guard was written when the code was ALREADY CLEAN.** ⚠️ **It had only ever seen
GREEN** — ✅ **so "we built a guard" and "the guard works" were, until this Task, the same unproven
claim.** ⚠️ **The [SP-130] restructure kept them apart for exactly this reason:** *"a guard written by
the same change it polices has no independent witness."*

## ✅ D1 — the guard is RED against the pre-retirement code

⚠️ **TARGET CHOSEN FROM HISTORY, NOT RECONSTRUCTED BY HAND.** ✅ **`78a739f` is [SP-141]; so
`78a739f~1` is the last commit where BOTH app-side owners of `scrivi.inspector-layout.v1` existed.**
⛔ **`a25e106~1` would have been WRONG — Apple was already retired there by [SP-141].**

✅ **Confirmed both owners present at that commit: Apple 4 hits, Linux 5.**
✅ **Today's `check-package-boundary.sh`, run in a throwaway worktree at `78a739f~1`, EXITED 1 and
named them:**

```
Scrivi/App/InspectorLayoutStore.swift:192      let data = try Data(contentsOf: url)
Scrivi/App/InspectorLayoutStore.swift:369      try data.write(to: tmp, options: .atomic)
platforms/linux/src/InspectorLayoutStore.cpp:73    QFile f(path_);
platforms/linux/src/InspectorLayoutStore.cpp:137   QSaveFile f(path_);
```

⚠️ **AND IT CAUGHT SOMETHING NOBODY ASKED IT TO:** ✅ **`EditorShell.cpp:2469` — [I-0241]'s disk
walk** — ⛔ **a defect that lived for months and was found by a human reading code, not by any test.**
✅ **Proof the guard would have surfaced it at the commit that introduced it.**

## ✅ D2 — GREEN after, with NO exemption

✅ **On the retired state the guard exits 0.**
⛔ **`InspectorLayoutStore` appears NOWHERE in the allow-list** — ⚠️ **its one mention in the script is
the [I-0215] rationale comment, not an exemption.** ✅ **Checked, not assumed: passing because the code
is clean, NOT because it was excused.**

## ✅ D3 — no [I-0241] allow-list entry

✅ **Never needed one.** ⚠️ **[SP-149] AC7 planned to carry it as a tracked DEBT** — ⛔ **but
[SP-142]/T-0542 FIXED the site first, so there is nothing to exempt.** ✅ **The two `I-0241` strings in
the script are advisory text in a failure message.**

## ✅ D4 — [I-0197] is CLOSED

| Class | Closed by |
| ----- | --------- |
| **A**, **C** | ✅ [SP-129] / [SP-130] — T-0508's ruling (⚠️ **one site was a REAL defect, found BY writing the ruling**) |
| **B** | ✅ [SP-141] (Apple) + [SP-142] (Linux) — ⛔ **neither platform touches the package** |
| **the guard** | ✅ [SP-149] / T-0541, ⚠️ **witnessed HERE** |

✅ **Archived → [`../../Issues/Verified/Issue-verified-0191-0200.md`](../../Issues/Verified/Issue-verified-0191-0200.md).**

---

*Verified 2026-09-22. The guard was proven RED against the defect it exists to prevent.*
