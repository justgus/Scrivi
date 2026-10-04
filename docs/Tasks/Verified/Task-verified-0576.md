# Verified Task — T-0576

**Sprint:** none (user-directed 2026-10-03) · **Platform:** `[Linux]`
**Implemented:** 2026-10-03 · ✅ **USER-VERIFIED 2026-10-04 on the rig (build 54):** *"Both rig tests pass on build 54"*
**Archived:** 2026-10-04.

---

## ✅ T-0576 — `[Linux]` Per-row relationship labels in the Scene Inspector (revert, [I-0180]) — **Implemented 2026-10-03 · ✅ VERIFIED 2026-10-04 (user, rig build 54)**

⚠️ **NO SPRINT** — ✅ user-directed 2026-10-03: *"close I-0180, revert Linux to per-row labels."*

✅ **WHY:** [I-0180] was closed as NOT A DEFECT (→ `../Issues/Closed/Issue-closed-0180.md`). SP-126 build 8
had implemented that Issue's proposed fix on Linux only, hoisting the label into the group header —
⚠️ **so Linux differed from Apple, the reference shape** (`feedback_linux_adopts_apple_shape`).

✅ **THE CHANGE (`platforms/linux/src/SceneInspector.cpp`):** the group header is `"<kind> (<count>)"`
again; each row is `"<name> — <label>"` (the pre-SP-126 form, name leading); the tooltip carries the same
text, including on a pending row. The label is still the core's projection, never recomputed.

⚠️ **Shape note, not a defect:** Apple puts the label on a SECOND LINE under the name; a
`QTreeWidgetItem` row is one line, so Linux joins them with `—`, as it did before SP-126.

✅ **VERIFIED BY BUILDING:** `docker build --no-cache -f platforms/linux/docker/Dockerfile` succeeded 2026-10-03 (Qt app + ScriviCore, GCC). ✅ **VERIFIED LIVE on the rig, build 54 (2026-10-04):** *"Both rig tests pass on build 54"*
