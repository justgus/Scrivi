# Verified Task — T-0601

**Sprint:** [SP-174] · **Epic:** [EP-048] (S3a) · **Platform:** `[Cross]`
**Filed:** 2026-10-10 · **Implemented:** 2026-10-10 · ✅ **USER-VERIFIED 2026-10-10 by the Mac live pass** (*"Live Pass (Mac): 1-7 passes."*)
**Archived:** 2026-10-10, with the Sprint close (*"close SP-174"*).

---

## T-0601 — `[Cross]` EP-048 S3a: the Format-command and balanced-edit logic moves into ScriviCore; Apple re-pointed

**Status:** ✅ **Verified 2026-10-10** (user: *"Live Pass (Mac): 1-7 passes."*) — created 2026-10-10 at [SP-174]'s activation; archived with the Sprint close. **Epic:** [EP-048] · **Sprint:** ✅ [SP-174] →
[`../../Sprints/Closed/Sprint-SP-174.md`](../../Sprints/Closed/Sprint-SP-174.md)

**What:** the bold/italic style edit, heading/body/list commands, Return in a list, balanced cut/copy/replace and the pending-pair
case become pure text-transform functions in ScriviCore (built on the md4c analysis), exposed through the C ABI. Apple calls
them; its Swift implementation is deleted. No visible change on the Mac (EP-046 AC6/AC12 suites pass unchanged).
