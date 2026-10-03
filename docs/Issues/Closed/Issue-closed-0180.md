# Closed Issue (Not Verified) — I-0180

## I-0180: `[Apple]` The object card repeats the relationship label on every row

**Status:** ⚪ **CLOSED — NOT A DEFECT (2026-10-03, user-approved)**
**Reason for Closure:** ⚠️ **The per-row label is correct by design.**
**Platform:** Apple (reference) · ⚠️ Linux had diverged and is reverted ([T-0576])
**Component:** `Scrivi/Views/Inspector/ObjectCard.swift` (`ObjectCardRow`, ~:1080)
**Severity at closure:** Low
**Date Identified:** 2026-08-30 · **Date Closed:** 2026-10-03

---

### ✅ THE RULING (user, 2026-10-03)

⚠️ ***"I don't think I-0180 is a real issue."*** → after review: ***"close I-0180, revert Linux to per-row labels."***

### ✅ WHY IT IS NOT A DEFECT — read in the code, 2026-10-03

1. ✅ **The label is projected for the endpoint that ASKED** (`RelationshipStore.cpp:497`,
   `v.label = isFrom ? t.forwardLabel : t.inverseLabel`). The card lives in the **Scene** Inspector and
   asks for the SCENE's edges, so it receives the scene's verb — *"features"*. ✅ In a scene panel that
   reads *"this scene features: Myton"*, which is consistent. ⛔ The Issue read it as a property of the
   OBJECT, which the panel's context does not support.
2. ✅ **It is not redundant in general.** A Locations card can hold BOTH *"features"* (`appears-in`)
   and *"takes place at"* (`located-at`) — the record conceded this — ✅ and writers can define their
   own relation types (`scrivi_upsert_relation_type`), at which point the per-row label is what tells
   rows apart. ⚠️ It looked repetitive only because `the-stairs-of-tintagael` uses the four seeded
   types and nothing else (checked read-only, 2026-10-03).

### ⚠️ CONSEQUENCE: Linux is reverted

SP-126 build 8 implemented this Issue's proposed fix on Linux (label hoisted to the group header,
*"Characters (2) (appears in)"*). ⚠️ That made Linux differ from Apple, the reference shape
(`feedback_linux_adopts_apple_shape`). ✅ Reverted to per-row labels in **[T-0576]**
(`platforms/linux/src/SceneInspector.cpp`).

---

### Original record (verbatim from `Issue-active.md`)

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0180** | `[Apple]` **The object card repeats the SAME relationship label on every row, and reads as a property of the object.** ⚠️ **Found by the user 2026-08-30 while reviewing the Linux mirror** — ⚠️ **it has been in the macOS app since EP-031 and was never noticed.** `ObjectCard.swift:1032-1036` renders `entry.label` beneath each `displayName`. **Two faults:** (1) ⚠️ **it reads WRONG** — a row showing *Myton at 23 / features* implies Myton features something; the stored edge is *Myton **appears in** scene*, and the core projects the inverse for the queried endpoint, so ⚠️ **the label describes what the SCENE does** and belongs nowhere near the object's name; (2) ⚠️ **it is REDUNDANT** — a scene relates to its objects the same way each time, so the identical word repeats down the whole card while distinguishing nothing. ✅ **Fix (user-ruled, implemented on Linux in SP-126 build 8): hoist the label to the CARD TITLE** — *"Characters (appears in)"* — and drop the per-row line. ⚠️ **Collect the labels from the rows rather than assuming one**: of the seeded vocabulary TWO types constrain to a scene (`appears-in` → *features*, `located-at` → *takes place at*), so a Locations card can legitimately hold both and all must be named. ✅ **Verified from the real project that `cites` never touches a scene** — it runs source→object and surfaces on the Sources card, so it is not a scene predicate at all. ⚠️ **EP-034 is CLOSED**, so this needs its own home rather than being smuggled into a `[Linux]` sprint. | Low | ⚠️ **Needs an Apple home** | 🔵 **Open** |
