# Verified Task — T-0687

**Sprint:** [SP-224] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-19** (user, 2026-11-19: *"all steps pass, close SP-224"*)
**Archived:** 2026-11-19.

---

## ✅ T-0687 — `[Apple]` EP-055 S4: projects and worlds survive a relaunch on iPad and iPhone

**Epic:** [EP-055] (AC7) · **Status:** ✅ **Verified 2026-11-19** (user) · **Sprint:** [SP-224] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-224.md`](../../Sprints/Closed/Sprint-SP-224.md) · **Date Requested:** 2026-11-19 (user: *"go, plan SP-224"*)

**What:** `ProjectBookmarkStore` and `WorldBookmarkStore` record and resolve on iOS (Apple's `.minimalBookmark` shape; the Mac keeps
`.withSecurityScope`), so the existing restore reopens a project picked in Files, iCloud Drive or an external drive, and an imported
world stays readable; a Files-picked project releases its scope on close; a project that cannot be restored says *"Open it again
from Files."* on iOS. Rulings and ACs: the Sprint.

**Where:** `ProjectBookmarkStore.swift:21–30` (the options), `WorldBookmarkStore.swift` (`record`, `activate`),
`AppEnvironment.swift` (`cannotReopenMessage`, `openProjectFromPickedURL`), `ScriviManuscriptTests-iOS.swift` (`IOSBookmarkTests`).

**Tested:** iOS 33/33 (2 new), Mac interop 352/352, macOS build. ✅ the iPad, iPhone, Duo and Mac live passes (user, 2026-11-19); with [I-0364], [I-0365].
