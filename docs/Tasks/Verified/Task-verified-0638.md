# Verified Task — T-0638

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A5–A12, iPhone B2–B6, iPhone Duo simulator C1–C4 and the toolbar re-pass on Mac, iPad and iPhone (user, 2026-11-03))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0638 — `[Apple]` EP-055 S2: the Mac's toolbar buttons on iOS, at the system's placement

**Epic:** [EP-055] (AC6) · **Status:** ✅ **Verified 2026-11-03** (iPad A5–A12, iPhone B2–B6, iPhone Duo simulator C1–C4 and the toolbar re-pass on Mac, iPad and iPhone (user, 2026-11-03)) (2026-10-31) — ⏳ the user judges look and position on the simulators · **Sprint:** [SP-199] (🟡 Active) → [`Sprint-SP-199.md`](../../Sprints/Closed/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** the Mac's toolbar content (`ProjectWindowManager.swift`: the Scene and Chapter groups, Worlds, Project Settings) shared
with iOS, at the default placement: the top on iPad and iPhone, the right side on the Duo. ⛔ No Show group (G4) and no
placement code ([SP-198] R3). A button whose action is not wired on iOS yet is disabled, as on the Mac.
**Tests:** builds on both platforms; the user judges look and position on the simulators.

*Last Updated: 2026-10-30 (**T-0632–T-0635 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-30 (**T-0635 filed → active** in SP-198; T-0634 rescoped by [SP-198] R3.)*

*Last Updated: 2026-10-29 (**T-0632, T-0633, T-0634 filed → active** with SP-198's activation (EP-055 S1).)*

*Last Updated: 2026-10-27 (**T-0629, T-0630, T-0631 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-27 (**T-0629, T-0630, T-0631 → active** with SP-197's activation (EP-052 S3).)*

*Last Updated: 2026-10-27 (**T-0627 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-27 (**T-0627 → active** with SP-196's activation (EP-052 S2b).)*

*Last Updated: 2026-10-26 (**T-0626 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-26 (**T-0625 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-26 (**T-0625, T-0626 → active** with SP-195's activation (EP-052 S2a).)*

*Last Updated: 2026-10-26 (**T-0623, T-0624 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-26 (**T-0623, T-0624 → active** with SP-194's activation (EP-052 S1).)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-24 (**T-0622 → active** with SP-193's activation (EP-054 S0).)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-24 (**T-0620 → active** with SP-192's activation.)*


*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-22 (**T-0616 → active** with SP-188's activation (EP-053 S3).)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-21 (**T-0619 → active** with SP-191's activation (EP-053 S2b).)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → active** with SP-187's activation (EP-053 S2a).)*

*Last Updated: 2026-10-20 (**T-0614 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0614 filed → active** with SP-186 (EP-053 S1).)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0613 filed** in [SP-185] (planned, awaiting go-ahead).)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0612 filed → active** with SP-185's activation.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-15 (**T-0609 → active** with SP-182's activation.)*

*Last Updated: 2026-10-15 (**T-0608 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-15 (**T-0608 → active** with SP-181's activation.)*

*Last Updated: 2026-10-13 (**T-0607 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-13 (**T-0607 → active** with SP-180's activation.)*

*Last Updated: 2026-10-12 (**T-0606 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-12 (**T-0606 → active** with SP-179's activation.)*
**Done (2026-10-31):** new `Scrivi/Views/ProjectToolbar.swift` holds the Mac's toolbar as a `ToolbarContent`, moved unchanged from
`ProjectWindowManager.swift`. The Show group is `#if os(macOS)` (G4). The Mac window uses it at `ProjectWindowManager.swift:353`,
and the iOS editor at `EditorView.swift:217`, beside the ⋯ menu. ⚠️ On iOS the Scene and Chapter buttons are **disabled**,
because the `UITextView` host does not set `session.*Action` yet (that is structural edits, EP-055 AC5). Worlds and Project
Settings work. Mac interop 327/327, iOS 12/12.
