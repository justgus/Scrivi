# Active Tasks

## 🟡 T-0632 — `[Apple]` EP-055 S1: the manuscript host gap audit — what is AppKit, what is shared, the iOS shape

**Epic:** [EP-055] (AC1) · **Status:** 🟡 **Active** · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-29 (user: *"we should focus on the Manuscript View first"*)

**What:** every behaviour in `ManuscriptTextView.swift` (the representable `:37`, the `Coordinator` `:287`, `ManuscriptNSTextView`
`:2614`, `SceneBoundaryTable` `:3537`) and in `ManuscriptCommands` / `ManuscriptFind` / `ManuscriptAccessibility`, each given one
disposition: **shared as is**, **shared after a font/colour swap**, **moved out of the host into shared code**, or
**re-implemented on UIKit** (and why). Ordered by what S1, S2 and S3 need. Answers [SP-198] Q1 for the user to rule.
**Output:** `docs/Scrivi_iOS_Manuscript_Design_v0_1.md`.

---

## 🟡 T-0633 — `[Apple]` EP-055 S1: the presentation layer compiles for iOS, the Mac unchanged

**Epic:** [EP-055] (AC2) · **Status:** 🟡 **Active** · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-29

**What:** `ManuscriptPresenter`, `ManuscriptPresentedMap` and `ManuscriptTypography` (and whatever else [T-0632] rules shared)
built for both platforms, with UIKit's font and colour types on iOS. No behaviour change on the Mac.
**Tests:** the Mac's interop tests (`scripts/run-interop-tests.sh`), `check-textkit2.sh`, `check-manuscript-type-source.sh`; the
iOS target builds.

---

## 🟡 T-0634 — `[Apple]` EP-055 S1: the `UITextView` manuscript on iPad and iPhone

**Epic:** [EP-055] (AC2, AC3) · **Status:** 🟡 **Active** · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-29 (user: *"It should work with the keyboard overlay and a hardware keyboard on both the iPad, iPhone, and iPhone Duo"*)

**What:** the iOS branch of `ManuscriptTextView` replaced by a TextKit 2 `UITextView` host on [T-0633]'s presenter: the viewport
scenes load, read and scroll; typing autosaves through the same path as the Mac; selection; dividers and chapter titles.
⛔ Nothing device- or keyboard-specific: the system's default keyboard and layout behaviour ([SP-198] R3).
**Tests:** [T-0635]'s target (a scene typed on the simulator reaches its `.md` file on disk); live passes on the user's iPad
and iPhone.

---

## 🟡 T-0635 — `[Apple]` EP-055 S1: an iOS test target

**Epic:** [EP-055] (AC4) · **Status:** 🟡 **Active** · **Sprint:** [SP-198] (🟡 Active) → [`../Sprints/Sprint-SP-198.md`](../Sprints/Sprint-SP-198.md) · **Date Requested:** 2026-10-30 (user: *"We will need to create a test target for iOS."*)

**What:** a test target hosted by `ScriviApp-iOS`, at 27.1, run on the iOS Simulator by `xcodebuild test`, with the
manuscript's tests in it. Whether the Mac's interop tests that are not AppKit-bound also run there is [T-0632]'s to say.
**Tests:** it runs, and reports more than zero tests.

---

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
