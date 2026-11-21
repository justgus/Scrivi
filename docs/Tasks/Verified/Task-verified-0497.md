# Verified Task — T-0497

**Sprint:** [SP-231] · **Epic:** [EP-059] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-21** (user, 2026-11-21: *"All the live tests pass."*)
**Archived:** 2026-11-21.

---

## ✅ T-0497 — `[Linux]` World creation

**Epic:** [EP-059] (D1; was EP-035) · **Status:** ✅ Verified (user, 2026-11-21: *"All the live tests pass."*) · **Sprint:** [SP-231] (✅ Closed) → [`../../Sprints/Closed/Sprint-SP-231.md`](../../Sprints/Closed/Sprint-SP-231.md) · **Date Requested:** 2026-09-01 (the T-0496 live pass)

**What (as filed):** ⚠️ **`[Linux]` World CREATION** — the one Worlds action SP-127 deliberately deferred. ⚠️ **Apple entangles it with `WorldBookmarkStore` (macOS sandbox grants, NO Linux equivalent)**, and ✅ **AC3 asks for added / status read / relinked, not created**. ⚠️ **PRIORITY ARGUMENT CHANGED BY THE T-0496 LIVE PASS (2026-09-01):** ⚠️ **the user hit this as a DEAD END while setting up a test** — `Project ▸ Manage Worlds…` offers **no create affordance**, so ⚠️ **a project with no world cannot proceed on Linux at all**: there is nothing to *add* or *relink*. ✅ **Apple detects the no-world case and PROMPTS the writer to create one** — ⚠️ **the deferral assumed add+relink were enough to be USABLE, and they are not.** ⚠️ **The step-7 test could not be completed on Linux alone; the world (`Alpha Centauri`) had to be created in the APPLE app** — ⚠️ **the sharpest evidence available that this is a gap, not a nicety.**

**Built:** Create… in the Worlds dialog — a name, then a folder (Q1) — `<folder>/<name>.inkwynworld`, bound to the project
(`WorldsDialog::createWorldIn`, `WorldsDialog.cpp`). The Mac's "prompt" when there is no world is its New Object form's line
(`ObjectCard.swift:836`), which comes to Linux with object creation (EP-059 A5).
