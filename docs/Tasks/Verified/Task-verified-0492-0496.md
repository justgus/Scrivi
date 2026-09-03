# Verified Tasks — T-0492 – T-0496 (SP-127, EP-035)

**Sprint:** SP-127 — `[Linux]` The Worlds surface (✅ **CLOSED 2026-09-02**)
**Epic:** [EP-035](../../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 3**
**Verified:** 2026-09-02 — **user-approved after a live pass on the REAL RIG (build 29)**
**Closes:** **AC3**

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| **T-0492** | **`WorldsDialog` — the list.** Name, path, status, empty state; ⚠️ **`WorldStatusText` HOISTED out of `SceneInspector` so the phrasing has ONE home** | **SP-127** | ✅ **VERIFIED 2026-09-02 (user-approved)** — the list was read on the rig; ⚠️ **its path and status took TWO further fixes to become readable (I-0184, I-0186)** |
| **T-0493** | ⚠️ **RELINK ("Locate…")** — ⚠️ **the sprint's REASON.** ✅ **PROVEN on the rig against a real unresolvable world**: wrong folder REFUSED, right package accepted, `unavailable` → `available` | **SP-127** | ✅ **VERIFIED 2026-09-02 (user-approved)** — ⚠️ **the sprint's REASON, proven end to end by the user**: wrong world rejected, correct world accepted, pending notices cleared |
| **T-0494** | **Project ▸ Manage Worlds…** + `editorOnlyActions_` gating; ⚠️ **the I-0130 refresh wired via `inspector_->reload()`** | **SP-127** | ✅ **VERIFIED 2026-09-02 (user-approved)** — menu gating and the I-0130 refresh both observed live |
| **T-0495** | **Add existing world** + **remove reference** with a confirmation stating ⚠️ **the package is NOT deleted** | **SP-127** | ✅ **VERIFIED 2026-09-02 (user-approved)** — add-existing and remove-reference exercised; ⚠️ **the add path also surfaced I-0185** |
| **T-0496** | ⚠️ **LIVE PASS on the REAL RIG** + `ctest` + smoke | **SP-127** | ✅ **VERIFIED 2026-09-02 (user-approved)** — all 8 steps pass; ⚠️ **step 7's UI half closed by the user driving the rejection through the dialog by hand.** Final state **build 29**: `ctest` **572/572** + **22/22** smokes. ⚠️ **The pass FOUND FOUR DEFECTS, ALL NOW VERIFIED**: **I-0183** (🔴 data loss), **I-0184**, **I-0185**, **I-0186**. ⚠️ **Three NEW smokes exist because this pass found what the suite could not**: `worlds_path_smoke`, `theme_contrast_smoke`, `package_folder_smoke` |

---

## What these five Tasks delivered

✅ **The Linux Worlds surface** — `Project ▸ Manage Worlds…`: bound worlds listed with name, path and
status; **Locate…** to repair a moved world; add-existing and remove-reference.

⚠️ **The reason it mattered:** before this, `addWorld`, `relinkWorld`, `getWorldStatus` and
`getWorldBinding` were bridged across the ABI with **ZERO callers** — ⚠️ **a capability with no
surface** — so **a project whose world had moved could not be repaired from the Linux app at all.**

---

## ⚠️ What the live pass found — and why it matters more than the delivery

⚠️ **The pass found FOUR defects. None was caught by any suite.** ✅ **All four are Verified**
([`Issue-verified-0181-0190.md`](../../Issues/Verified/Issue-verified-0181-0190.md)):

| Issue | What | ⚠️ How it was missed |
| ----- | ---- | ------------------- |
| ⚠️ **I-0183** | 🔴 **DATA LOSS** — an available-but-UNREADABLE world licensed a prune; **10 of 12 relationships destroyed** in a real project | ⚠️ **The sprint's OWN risk table believed it was mitigated** — *"the core verifies `worldID`"*. ⚠️ **It is the WRONG CHECK: a COPY shares its `worldID`** |
| **I-0184** | The row's path was **CLIPPED**, tail lost | ⚠️ **I first blamed a 360 px constant; real paths never reach it.** The **row layout** left 202 px for a 211 px path |
| **I-0185** | A package is an **ATOM**; the dialog descended into it | ⚠️ **My first fix made descent RECOVERABLE when the requirement was IMPOSSIBLE** — ⚠️ **and it passed every test I wrote for it** |
| **I-0186** | Secondary text at **1.07:1** contrast — invisible | ⚠️ **Qt's no-theme fallback made every offscreen check, AND a screenshot I offered as evidence, show a readable path no user ever saw** |

⚠️ **Two of the four needed the USER TO CORRECT MY DIAGNOSIS** before the real defect came into view.

✅ **Three NEW smokes exist because of this pass**, each verified to FAIL against the behaviour it
pins: `worlds_path_smoke`, `theme_contrast_smoke`, `package_folder_smoke`. ⚠️ **The last two exist
because the suite was structurally incapable of seeing the defect** — one ran headless (no theme), the
other tested pure functions instead of the interaction.

---

## ⚠️ Deferred, deliberately

| Item | Where |
| ---- | ----- |
| ⚠️ **World CREATION** | **T-0497** — ⚠️ **the live pass RAISED its priority**: the user hit a dead end, because a project with no world has nothing to *add* or *relink*. ⚠️ **The step-7 test could not be completed on Linux alone; the world had to be created in the APPLE app** |
| ⚠️ **`worldID`-equality weakness** | ⚠️ **Still real** — a copy shares `worldID` AND `modifiedAt`, so no field distinguishes revisions. ✅ **With I-0183 fixed, a wrongly-bound world can no longer DESTROY anything — only leave links pending** |
