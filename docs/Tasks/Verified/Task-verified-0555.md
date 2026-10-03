# Verified Task — T-0555

**Issue closed:** ✅ **[I-0253]** → [`../../Issues/Verified/Issue-verified-0251-0260.md`](../../Issues/Verified/Issue-verified-0251-0260.md)
**Platform:** `[Apple]` · **Sprint:** ⛔ none (standalone)
**Implemented:** 2026-09-28 · ✅ **USER-VERIFIED 2026-10-03:** *"I-0253 is verified fixed."* / *"verify T-0555."*
**Archived:** 2026-10-03.
✅ **Re-checked 2026-10-03 before verification:** `ScriviApp-visionOS` BUILD SUCCEEDED and
`scripts/check-engine-stub-parity.sh` clean, after SP-151/SP-152 had changed `ScriviEngine.swift`.

---

## ✅ T-0555 — `[Apple]` ✅ **Engine stub parity ([I-0253])** — Implemented 2026-09-28 · ✅ **VERIFIED 2026-10-03 (user)**

⚠️ **NO SPRINT** — ✅ **raised by the user from a symptom they had patched themselves.**

✅ **THE FIX (two parts, and the second matters more):**
1. ✅ **Four methods added to the visionOS stub** in `Scrivi/Engine/ScriviEngine.swift`:
   `closeProject` (a no-op — it is non-throwing and runs on teardown), `openSceneForBulkLoad`,
   `mergeScene`, `mergeChapter`.
2. ✅ **`scripts/check-engine-stub-parity.sh`** — ⚠️ **the guard, wired into `scrivi-apple-ci.yml`**
   (step + both path filters). ⛔ **Because the stub's own comment already predicted this recurrence
   and a comment cannot fail a build.**

⚠️ **THE GUARD WAS PROVEN BY BREAKING IT:** ✅ **`mergeScene` was removed from the stub, the guard went
RED naming it, and green on restore.**

⛔ **I GOT THE GUARD WRONG ONCE, AND IT BROKE THE BUILD — recorded because it is the instructive part.**
⚠️ **The first pattern anchored on whitespace-then-`public func`, so it MISSED every
`@discardableResult public func` in the stub and reported FOUR methods as absent that were ALREADY
THERE** (`:1536-1539`). ⛔ **Acting on that phantom added duplicates and broke the visionOS build — the
exact build the guard exists to protect.** ✅ **Fixed to skip leading attributes; the reverted additions
are gone.** ⚠️ **A check that reports a phantom is worse than no check: it invites a "fix" that breaks
something real.**

✅ **VERIFIED BY BUILDING: `ScriviApp` ✅ · `ScriviApp-iOS` ✅ · `ScriviApp-visionOS` ✅ — all BUILD
SUCCEEDED.** ✅ **All four guards green.**
⛔ **NOT VERIFIED: the app was not RUN on visionOS** — ⚠️ **it cannot usefully be: the stub throws by
design because ScriviCore is not linked for visionOS ([I-0053]).** ✅ **This Task restores COMPILATION,
which is what it claims.**
