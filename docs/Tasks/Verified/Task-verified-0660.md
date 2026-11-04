# Verified Task — T-0660

**Sprint:** none · **Epic:** [EP-050] / [EP-057] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-04** (user: "this is a pass")
**Archived:** 2026-11-04, with [SP-202] and [SP-203]'s close (user, 2026-11-04: *"this is a pass. We should be able to close SP-202 and SP-203."*).

---

## T-0660 — `[Linux]` Rig pass: Orca reads the manuscript as the page shows it

**Epic:** [EP-057] (for [EP-050] S4) · **Status:** ✅ **Verified 2026-11-04** (user: *"this is a pass"*) · **Sprint:** none ·
**Date Requested:** 2026-11-04 ([SP-202] Q3, ruled as recommended)

**What:** the Orca listen for [SP-203]: on scrivi-vm, Orca on, the arrow keys over a manuscript with escapes, emphasis, a
heading and a divider.

**Result (2026-11-04, scrivi-vm, Orca 50.2, build ≥ 1.0.328):** ⚠️ the VM's sound stuttered, so what Orca said was read from
its speech log (`orca -r --debug-file=/tmp/orca.log`, 148 utterances): *"Mr. Smith said bold and soft here."* (no "star",
no "backslash"); the heading without "number number"; *"1. not a list."* (no "backslash"); Right / Left through *bold* —
each letter once, no "star", the caret never stopping twice; the divider read as *"Scene break"* (the caret cannot rest on
it on Linux — it steps over — but Orca reads its line); typing echoed. ✅ User: *"this is a pass."*
