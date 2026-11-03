# Verified Task — T-0635

**Sprint:** [SP-198] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0635 — `[Apple]` EP-055 S1: an iOS test target

**Epic:** [EP-055] (AC4) · **Status:** ✅ **Verified 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*)) (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`Sprint-SP-198.md`](../../Sprints/Closed/Sprint-SP-198.md) · **Date Requested:** 2026-10-30 (user: *"We will need to create a test target for iOS."*)

**What:** a test target hosted by `ScriviApp-iOS`, at 27.1, run on the iOS Simulator by `xcodebuild test`, with the
manuscript's tests in it. Whether the Mac's interop tests that are not AppKit-bound also run there is [T-0632]'s to say.
**Tests:** it runs, and reports more than zero tests.
**Done (2026-10-30):** the `ScriviTests-iOS` target (hosted by `ScriviApp-iOS`, 27.1) in the `ScriviApp-iOS` scheme;
`Scrivi/Tests/ScriviManuscriptTests-iOS.swift`. **Evidence:** 10 tests in 2 suites pass on the iPhone 18 Pro simulator (iOS 27.2),
including a scene typed on iOS autosaved to disk, escaped. ⚠️ It needs a 27.1+ simulator runtime (the 27.0 devices are refused).
