# Active Tasks

---

## 🟡 T-0601 — `[Cross]` EP-048 S3a: the Format-command and balanced-edit logic moves into ScriviCore; Apple re-pointed

**Status:** 🟡 **Active** — created 2026-10-10 at [SP-174]'s activation. **Epic:** [EP-048] · **Sprint:** 🟡 [SP-174] →
[`../Sprints/Sprint-SP-174.md`](../Sprints/Sprint-SP-174.md)

**What:** the bold/italic style edit, heading/body/list commands, Return in a list, balanced cut/copy/replace and the pending-pair
case become pure text-transform functions in ScriviCore (built on the md4c analysis), exposed through the C ABI. Apple calls
them; its Swift implementation is deleted. No visible change on the Mac (EP-046 AC6/AC12 suites pass unchanged).
